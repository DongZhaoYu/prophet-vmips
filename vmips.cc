/* Main driver program for VMIPS.
   Copyright 2001, 2003 Brian R. Gaeke.
   Copyright 2002, 2003 Paul Twohey.

This file is part of VMIPS.

VMIPS is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

VMIPS is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with VMIPS; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "clock.h"
#include "clockdev.h"
#include "clockreg.h"
#include "cpzeroreg.h"
#include "debug.h"
#include "error.h"
#include "endiantest.h"
#include "haltreg.h"
#include "haltdev.h"
#include "intctrl.h"
#include "range.h"
#include "spimconsole.h"
#include "mapper.h"
#include "memorymodule.h"
#include "cpu.h"
#include "cpzero.h"
#include "spimconsreg.h"
#include "vmips.h"
#include "options.h"
#include "decrtc.h"
#include "decrtcreg.h"
#include "deccsr.h"
#include "deccsrreg.h"
#include "decstat.h"
#include "decserial.h"
#include "stub-dis.h"
#include "rommodule.h"
#include <fcntl.h>
#include <cerrno>
#include <cassert>
#include <csignal>
#include <cstdarg>
#include <string>
#include <exception>
#include <string.h>

vmips *machine;

void
vmips::refresh_options(void)
{
	/* Extract important flags and things. */
	opt_bootmsg = opt->option("bootmsg")->flag;
	opt_clockdevice = opt->option("clockdevice")->flag;
	opt_debug = opt->option("debug")->flag;
	opt_dumpcpu = opt->option("dumpcpu")->flag;
	opt_dumpcp0 = opt->option("dumpcp0")->flag;
	opt_haltdevice = opt->option("haltdevice")->flag;
	opt_haltdumpcpu = opt->option("haltdumpcpu")->flag;
	opt_haltdumpcp0 = opt->option("haltdumpcp0")->flag;
	opt_instcounts = opt->option("instcounts")->flag;
	opt_memdump = opt->option("memdump")->flag;
	opt_realtime = opt->option("realtime")->flag;
 
	opt_clockspeed = opt->option("clockspeed")->num;
	clock_nanos = 1000000000/opt_clockspeed;

	opt_clockintr = opt->option("clockintr")->num;
	opt_clockdeviceirq = opt->option("clockdeviceirq")->num;
	opt_loadaddr = opt->option("loadaddr")->num;
	opt_memsize = opt->option("memsize")->num;
	opt_timeratio = opt->option("timeratio")->num;
 
	opt_memdumpfile = opt->option("memdumpfile")->str;
	opt_image = opt->option("romfile")->str;
	opt_execname = opt->option("execname")->str;
	opt_ttydev = opt->option("ttydev")->str;
	opt_ttydev2 = opt->option("ttydev2")->str;
	opt_spimconsole = opt->option("spimconsole")->flag;

	opt_decrtc = opt->option("decrtc")->flag;
	opt_deccsr = opt->option("deccsr")->flag;
	opt_decstat = opt->option("decstat")->flag;
	opt_decserial = opt->option("decserial")->flag;
}

/* Set up some machine globals, and process command line arguments,
 * configuration files, etc.
 */
vmips::vmips(int argc, char *argv[])
	: opt(new Options), halted(false),
	  clock(0), clock_device(0), halt_device(0), spim_console(0),
	  num_instrs(0)
{
   	opt->process_options (argc, argv);
	refresh_options();
}

vmips::~vmips() throw()
{
	delete spim_console;
	delete halt_device;
	delete clock_device;
	delete clock;
}

void
vmips::setup_machine(void)
{
	/* Construct the various vmips components. */
	intc = new IntCtrl;
	physmem = new Mapper;
	cpu = new CPU (*physmem, *intc);

	/* Set up the debugger interface, if applicable. */
	if (opt_debug)
		dbgr = new Debug (*cpu, *physmem);

    /* Direct the libopcodes disassembler output to stderr. */
    disasm = new Disassembler (host_bigendian, stderr);
}

/* Connect the file or device named NAME to line number L of
 * console device C, or do nothing if NAME is "off".
 */
void vmips::setup_console_line(int l, char *name, TerminalController *c, const
char *c_name) throw()
{
	/* If they said to turn off the tty line, do nothing. */
	if (strcmp(name, "off") == 0)
		return;

	/* Open the file or device in question. */
	int ttyfd = open(name, O_RDWR | O_NONBLOCK);
	if (ttyfd == -1) {
		/* If we can't open it, warn and use stdout instead. */
		error("Opening %s (terminal %d): %s", name, l, strerror(errno));
		warning("using stdout, input disabled\n");
		ttyfd = fileno(stdout);
	}

	/* Connect it to the SPIM-compatible console device. */
	c->connect_terminal(ttyfd, l);
	boot_msg("Connected fd %d to %s line %d.\n", ttyfd, c_name, l);
}

bool vmips::setup_spimconsole() throw( std::bad_alloc )
{
	/* FIXME: It would be helpful to restore tty modes on a SIGINT or
	   other abortive exit or when vmips has been foregrounded after
	   being in the background. The restoration mechanism should use
	   TerminalController::reinitialze_terminals() */

	if (!opt_spimconsole)
		return true;
	
	spim_console = new SpimConsoleDevice( clock );
	physmem->map_at_physical_address( spim_console, SPIM_BASE );
	boot_msg("Mapping %s to physical address 0x%08x\n",
		  spim_console->descriptor_str(), SPIM_BASE);
	
	intc->connectLine(IRQ2, spim_console);
	intc->connectLine(IRQ3, spim_console);
	intc->connectLine(IRQ4, spim_console);
	intc->connectLine(IRQ5, spim_console);
	intc->connectLine(IRQ6, spim_console);
	boot_msg("Connected IRQ2-IRQ6 to %s\n",spim_console->descriptor_str());

	setup_console_line(0, opt_ttydev, spim_console,
      spim_console->descriptor_str ());
	setup_console_line(1, opt_ttydev2, spim_console,
      spim_console->descriptor_str ());
	return true;
}

bool vmips::setup_clockdevice() throw( std::bad_alloc )
{
	if( !opt_clockdevice )
		return true;

	uint32 clock_irq;
	if( !(clock_irq = DeviceInt::num2irq( opt_clockdeviceirq )) ) {
		error( "invalid clockdeviceirq (%u), irq numbers must be 2-7.",
		       opt_clockdeviceirq );
		return false;
	}	

	/* Microsecond Clock at base physaddr CLOCK_BASE */
	clock_device = new ClockDevice( clock, clock_irq, opt_clockintr );
	physmem->map_at_physical_address( clock_device, CLOCK_BASE );
	boot_msg( "Mapping %s to physical address 0x%08x\n",
		  clock_device->descriptor_str(), CLOCK_BASE );

	intc->connectLine( clock_irq, clock_device );
	boot_msg( "Connected %s to the %s\n", DeviceInt::strlineno(clock_irq),
		  clock_device->descriptor_str() );

	return true;
}

bool vmips::setup_decrtc() throw( std::bad_alloc )
{
	if (!opt_decrtc)
		return true;

	/* Always use IRQ3 ("hw interrupt level 1") for RTC. */
	uint32 decrtc_irq = DeviceInt::num2irq (3);

	/* DECstation 5000/200 DS1287-based RTC at base physaddr DECRTC_BASE */
	decrtc_device = new DECRTCDevice( clock, decrtc_irq );
	physmem->map_at_physical_address( decrtc_device, DECRTC_BASE );
	boot_msg( "Mapping %s to physical address 0x%08x\n",
		  decrtc_device->descriptor_str(), DECRTC_BASE );

	intc->connectLine( decrtc_irq, decrtc_device );
	boot_msg( "Connected %s to the %s\n", DeviceInt::strlineno(decrtc_irq),
		  decrtc_device->descriptor_str() );

	return true;
}

bool vmips::setup_deccsr() throw( std::bad_alloc )
{
	if (!opt_deccsr)
		return true;

	/* DECstation 5000/200 Control/Status Reg at base physaddr DECCSR_BASE */
    /* Connected to IRQ2 */
    static const uint32 DECCSR_MIPS_IRQ = DeviceInt::num2irq (2);
	deccsr_device = new DECCSRDevice (DECCSR_MIPS_IRQ);
	physmem->map_at_physical_address (deccsr_device, DECCSR_BASE);
	boot_msg ("Mapping %s to physical address 0x%08x\n",
		  deccsr_device->descriptor_str(), DECCSR_BASE);

	intc->connectLine (DECCSR_MIPS_IRQ, deccsr_device);
    boot_msg("Connected %s to the %s\n", DeviceInt::strlineno(DECCSR_MIPS_IRQ),
             deccsr_device->descriptor_str());

	return true;
}

bool vmips::setup_decstat() throw( std::bad_alloc )
{
	if (!opt_decstat)
		return true;

	/* DECstation 5000/200 CHKSYN + ERRADR at base physaddr DECSTAT_BASE */
	decstat_device = new DECStatDevice( );
	physmem->map_at_physical_address( decstat_device, DECSTAT_BASE );
	boot_msg( "Mapping %s to physical address 0x%08x\n",
		  decstat_device->descriptor_str(), DECSTAT_BASE );

	return true;
}

bool vmips::setup_decserial() throw( std::bad_alloc )
{
	if (!opt_decserial)
		return true;

	/* DECstation 5000/200 DZ11 serial at base physaddr DECSERIAL_BASE */
	/* Uses CSR interrupt SystemInterfaceCSRInt */
	decserial_device = new DECSerialDevice (clock, SystemInterfaceCSRInt);
	physmem->map_at_physical_address (decserial_device, DECSERIAL_BASE );
	boot_msg ("Mapping %s to physical address 0x%08x\n",
		  decserial_device->descriptor_str (), DECSERIAL_BASE );

	// Use printer line for console.
	setup_console_line (3, opt_ttydev, decserial_device,
      decserial_device->descriptor_str ());
	return true;
}

bool vmips::setup_haltdevice() throw( std::bad_alloc )
{
	if( !opt_haltdevice )
		return true;

	halt_device = new HaltDevice( this );
	physmem->map_at_physical_address( halt_device, HALT_BASE );
	boot_msg( "Mapping %s to physical address 0x%08x\n",
		  halt_device->descriptor_str(), HALT_BASE );

	return true;
}

void vmips::boot_msg( const char *msg, ... ) throw()
{
	if( !opt_bootmsg )
		return;

	va_list ap;
	va_start( ap, msg );
	vfprintf( stderr, msg, ap );
	va_end( ap );

	fflush( stderr );
}

int
vmips::host_endian_selftest(void)
{
  try {
    EndianSelfTester est;
    machine->host_bigendian = est.host_is_big_endian();
    if (!machine->host_bigendian) {
      boot_msg ("Little-Endian host processor detected.\n");
    } else {
      boot_msg ("Big-Endian host processor detected.\n");
    }
    return 0;
  } catch (std::string &err) {
    boot_msg (err.c_str ());
    return -1;
  }
}

void
vmips::halt(void) throw()
{
	halted = true;
}

uint32
vmips::get_file_size (FILE *fp) throw ()
{
	off_t here, there;

	assert (fp && "Null pointer passed to vmips::get_file_size ()");
	here = ftell (fp);
	fseek (fp, 0, SEEK_END);
	there = ftell (fp);
	fseek (fp, here, SEEK_SET);
	return there - here;
}

void
vmips::step(void)
{
	/* Process instructions. */
	cpu->step();

	/* Keep track of time passing. Each instruction either takes
	 * clock_nanos nanoseconds, or we use pass_realtime() to check the
	 * system clock.
     */
	if( !opt_realtime )
	   clock->increment_time(clock_nanos);
	else
	   clock->pass_realtime(opt_timeratio);

	/* If user requested it, dump registers from CPU and/or CP0. */
	if (opt_dumpcpu)
		cpu->dump_regs_and_stack(stderr);
	if (opt_dumpcp0)
		cpu->cpzero_dump_regs_and_tlb(stderr);
	num_instrs++;
}

long 
timediff(struct timeval *after, struct timeval *before)
{
    return (after->tv_sec * 1000000 + after->tv_usec) -
        (before->tv_sec * 1000000 + before->tv_usec);
}

bool
vmips::setup_rom ()
{
  // Open ROM image.
  FILE *rom = fopen (opt_image, "rb");
  if (!rom) {
    error ("Could not open ROM `%s': %s", opt_image, strerror (errno));
    return false;
  }
  // Translate loadaddr to physical address.
  opt_loadaddr -= KSEG1_CONST_TRANSLATION;
  ROMModule *rm;
  try {
    rm = new ROMModule (rom);
  } catch (int errcode) {
    error ("mmap failed for %s: %s", opt_image, strerror (errcode));
    return false;
  }
  // Map the ROM image to the virtual physical memory.
  physmem->map_at_physical_address (rm, opt_loadaddr);
  boot_msg ("Mapping ROM image (%s, %u words) to physical address 0x%08x\n",
            opt_image, rm->getExtent () / 4, rm->getBase ());
  // Point debugger at wherever the user thinks the ROM is.
  if (opt_debug)
    if (dbgr->setup (opt_loadaddr, rm->getExtent () / 4) < 0)
      return false; // Error in setting up debugger.
  return true;
}

bool
vmips::setup_ram () throw( std::bad_alloc )
{
  // Make a new RAM module and install it at base physical address 0.
  memmod = new MemoryModule(opt_memsize);
  physmem->map_at_physical_address(memmod, 0);
  boot_msg( "Mapping RAM module (host=%p, %uKB) to physical address 0x%x\n",
	    memmod->getAddress (), memmod->getExtent () / 1024, memmod->getBase ());

//init heap
//	....
  return true;
}

bool
vmips::setup_clock () throw( std::bad_alloc )
{
  /* Set up the clock with the current time. */
  timeval start;
  gettimeofday(&start, NULL);
  timespec start_ts;
  TIMEVAL_TO_TIMESPEC( &start, &start_ts );
  clock = new Clock( start_ts );
  return true;
}

static void halt_machine_by_signal(int i)
{
	machine->halt();
}

int
vmips::run()
{
	/* Check host processor endianness. */
	if (host_endian_selftest () != 0) {
		error( "Could not determine host processor endianness." );
		return 1;
	}

	/* Set up the rest of the machine components. */
	setup_machine();

	if (!setup_rom ()) 
	  return 1;

	if (!setup_ram ())
	  return 1;

	if (!setup_haltdevice ())
	  return 1;

	if (!setup_clock ())
	  return 1;

	if (!setup_clockdevice ())
	  return 1;

	if (!setup_decrtc ())
	  return 1;

	if (!setup_deccsr ())
	  return 1;

	if (!setup_decstat ())
	  return 1;

	if (!setup_decserial ())
	  return 1;

	if (!setup_spimconsole ())
	  return 1;

    signal (SIGQUIT, halt_machine_by_signal);

	/* Reset the CPU. */
	boot_msg( "\n*************RESET*************\n\n" );
	cpu->reset();

	if (!setup_exe ())
	  return 1;

	timeval start;
	if (opt_instcounts)
		gettimeofday(&start, NULL);

	if (opt_debug) {
		dbgr->serverloop();
	} else {
		while (! halted)
			step();
	}

	timeval end;
	if (opt_instcounts)
		gettimeofday(&end, NULL);

	/* Halt! */
	boot_msg( "\n*************HALT*************\n\n" );

    /* If we're tracing, dump the trace. */
    cpu->maybe_dump_trace ();

	/* If user requested it, dump registers from CPU and/or CP0. */
	if (opt_haltdumpcpu || opt_haltdumpcp0) {
		fprintf(stderr,"Dumping:\n");
		if (opt_haltdumpcpu)
			cpu->dump_regs_and_stack(stderr);
		if (opt_haltdumpcp0)
			cpu->cpzero_dump_regs_and_tlb(stderr);
	}

	if (opt_instcounts) {
		double elapsed = (double) timediff(&end, &start) / 1000000.0;
		fprintf(stderr, "%u instructions in %.5f seconds (%.3f "
			"instructions per second)\n", num_instrs, elapsed,
			((double) num_instrs) / elapsed);
	}

	if (opt_memdump) {
		fprintf(stderr,"Dumping RAM to %s...", opt_memdumpfile);
		if (FILE *ramdump = fopen (opt_memdumpfile, "wb")) {
			fwrite (memmod->getAddress (), memmod->getExtent (), 1, ramdump);
			fclose(ramdump);
			fprintf(stderr,"succeeded.\n");
		} else {
			error( "\nRAM dump failed: %s", strerror(errno) );
		}
	}

	/* We're done. */
	boot_msg( "Goodbye.\n" );
	return 0;
}

static void vmips_unexpected() {
  fatal_error ("unexpected exception");
}

static void vmips_terminate() {
  fatal_error ("uncaught exception");
}
/*
int
main(int argc, char **argv)
{
	try {
		std::set_unexpected(vmips_unexpected);
		std::set_terminate(vmips_terminate);

		machine = new vmips(argc, argv);
		int rc = machine->run();
		delete machine; /* No disassemble Number Five!! */
/*		return rc;
	}
	catch( std::bad_alloc &b ) {
		fatal_error( "unable to allocate memory" );
	}
}
*/