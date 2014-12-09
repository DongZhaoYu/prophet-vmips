/* DEC 5000/200 Error Address and Check/Syndrome Status registers emulation.
   Copyright 2003 Brian R. Gaeke.

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

/* Memory-mapped device representing the Error Address Status Register
 * and ECC Check/Syndrome Status Register in the DEC 5000/200 (KN02).
 */

#include "deviceexc.h"
#include "cpu.h"
#include "decstat.h"
#include "mapper.h"
#include "vmips.h"

DECStatDevice::DECStatDevice () throw ()
{
  extent = 0x100000;
}

#define ERRADR_VALID   0x80000000
#define ERRADR_CPU     0x40000000
#define ERRADR_WRITE   0x20000000
#define ERRADR_ECCERR  0x10000000
#define ERRADR_RSRVD   0x08000000
#define ERRADR_ADDRESS 0x07FFFFFF

uint32
DECStatDevice::fetch_word (uint32 offset, int mode, DeviceExc *client)
{
  uint32 rv;
  if (!(offset & 0x80000)) {
    fprintf (stderr, "CHKSYN reg read as 0x%x\n", chksyn_reg);
    rv = chksyn_reg;
  } else {
    Mapper::BusErrorInfo berr;
    machine->physmem->get_last_berr_info (berr);
    if (berr.valid) {
      erradr_reg = ERRADR_VALID;
      if (berr.mode == DATASTORE) erradr_reg |= ERRADR_WRITE;
      if (berr.client == dynamic_cast<DeviceExc *>(machine->cpu))
	erradr_reg |= ERRADR_CPU;
      // Simulate effects of pipelining - VMIPS's Mapper knows the
      // exact address, but the DECstation would have had to wait 5
      // cycles to get the address.
      berr.addr >>= 2;
      berr.addr = (berr.addr & ~0x0fffL) | ((berr.addr & 0x0fffL) + 5);
      berr.addr &= ERRADR_ADDRESS;
      erradr_reg |= berr.addr;
    } else {
      erradr_reg &= ~ERRADR_VALID;
    }
    fprintf (stderr, "ERRADR reg read as 0x%x\n", erradr_reg);
    rv = erradr_reg;
  }
  return rv;
}

void
DECStatDevice::store_word (uint32 offset, uint32 data, DeviceExc *client)
{
  if (!(offset & 0x80000)) {
    fprintf (stderr, "CHKSYN reg cleared\n");
    chksyn_reg = 0;
  } else {
    fprintf (stderr, "ERRADR reg cleared\n");
    if (interrupt)
      fprintf (stderr, "Interrupt cancelled\n");
    erradr_reg = 0;
  }
}
