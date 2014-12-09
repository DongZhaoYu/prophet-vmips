/* DS1287-based DEC 5000/200 Real-Time Clock emulation.
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

/* Dallas Semiconductor DS1287 real-time clock chip, as implemented
 * as a memory-mapped device in the DEC 5000/200 (KN02).
 */

#include "devicemap.h"
#include "clock.h"
#include "decrtcreg.h"
#include "decrtc.h"
#include <cstdio>
#include <cassert>
#include <time.h>

static const uint32 int_freqs [16] = {
  0, 3906250, 7812500, 122070, 244141, 488281, 976562, 1953125,
  3906250, 7812500, 15625000, 31250000, 62500000, 125000000,
  250000000, 500000000
};
static const char *reg_names [15] = {
  "RTC_SEC", "RTC_ALMS", "RTC_MIN", "RTC_ALMM", "RTC_HOUR",
  "RTC_ALMH", "RTC_DOW", "RTC_DAY", "RTC_MON", "RTC_YEAR",
  "RTC_REGA", "RTC_REGB", "RTC_REGC", "RTC_REGD", "RTC_RAM"
};

DECRTCDevice::DECRTCDevice (Clock *_clock, uint32 _irq) :
  clock(_clock), frequency_ns (0), irq (_irq), interrupt_enable (false)
{
  extent = 0x80000;
  // Initialize registers
  for (int i = 0; i < 64; ++i) {
    rtc_reg[i] = 0x00;
  }
  rtc_reg[RTC_REGA] = 0x20;  // Timebase divisor = 010 (32.768 kHz)
  // Set up write masks
  for (int i = 0; i < 64; ++i) {
    write_masks[i] = 0xff;
  }
  write_masks[RTC_REGA] = 0x7f;
  write_masks[RTC_REGB] = 0xd7;
  write_masks[RTC_REGC] = 0x00;
  write_masks[RTC_REGD] = 0x00;
}

const struct tm *
DECRTCDevice::get_host_time () const {
  time_t real_time = clock->get_time().tv_sec;
  return gmtime(&real_time);
}

uint32
DECRTCDevice::fetch_word(uint32 offset, int mode, DeviceExc *client)
{
  uint32 reg_no = (offset / 4);
  uint32 rv = 0;
  if (reg_no == RTC_REGA) {
    // Fake "update in progress" once a second
    time_t real_nanos = clock->get_time().tv_nsec;
    if ((real_nanos + ((244 + 1948)*1000)) > 1000000000) {
	  rtc_reg[RTC_REGA] |= REGA_UIP;
	} else {
	  rtc_reg[RTC_REGA] &= ~REGA_UIP;
	}
  } 
  if (reg_no < 10)
    update_host_time ();
  if (reg_no < 64)
    rv = rtc_reg[reg_no];
  if (reg_no == RTC_REGC)
    unready_clock ();
  return rv;
}

void
DECRTCDevice::update_host_time ()
{
  const struct tm *host_time = get_host_time ();
  rtc_reg[RTC_SEC]  = host_time->tm_sec; /* second 0..59 */
  rtc_reg[RTC_MIN]  = host_time->tm_min; /* minute 0..59 */
  rtc_reg[RTC_HOUR] = host_time->tm_hour; /* hour 0..23 */
  rtc_reg[RTC_DOW]  = 1 + host_time->tm_wday; /* day of week 1..7 */
  rtc_reg[RTC_MON]  = 1 + host_time->tm_mon; /* month 1..12 */
  rtc_reg[RTC_YEAR] = host_time->tm_year % 100; /* year 0..99 */
}

void
DECRTCDevice::store_word(uint32 offset, uint32 data, DeviceExc *client)
{
  bool old_interrupt_enable = rtc_reg[RTC_REGB] & REGB_PIE;
  uint32 reg_no = (offset / 4);
  fprintf (stderr, "RTC %s (%u) written with 0x%x\n",
	reg_names[(reg_no > 14) ? 14 : reg_no], reg_no, data);
  if (reg_no < 64)
    rtc_reg[reg_no] = rtc_reg[reg_no] | (((uint8) data) & write_masks[reg_no]);
  if (reg_no == RTC_REGA) {
    /* Maybe they changed the interrupt rate selector. */
    uint8 rate_selector = rtc_reg[RTC_REGA] & REGA_RSX;
    fprintf (stderr, "RTC rate_selector set to 0x%x (%u)\n", rate_selector,
	  int_freqs[rate_selector]);
    frequency_ns = int_freqs[rate_selector];
  }
  if (reg_no == RTC_REGB) {
    /* Maybe they enabled or disabled interrupts. */
    bool new_interrupt_enable = rtc_reg[RTC_REGB] & REGB_PIE;
    fprintf (stderr, "RTC interrupt_enable set to %d\n", new_interrupt_enable);
    if ((!old_interrupt_enable) && new_interrupt_enable) {
      fprintf (stderr, "RTC turning interrupts on\n");
      ready_clock ();
    } else if (old_interrupt_enable && (!new_interrupt_enable)) {
      fprintf (stderr, "RTC turning interrupts off\n");
    }
    interrupt_enable = new_interrupt_enable;
  }
}

void
DECRTCDevice::ready_clock ()
{
  static unsigned long counter = 0;
  if (interrupt_enable) {
    counter++;
    if ((counter % 50000) == 0)
	  fprintf (stderr, "RTC counted %lu interrupts\n", counter);
    assertInt (irq);
  }

  clock_trigger = new ClockTrigger (this);
  clock->add_deferred_task (clock_trigger, frequency_ns);
}

void
DECRTCDevice::unready_clock ()
{
  deassertInt (irq);
}

DECRTCDevice::~DECRTCDevice ()
{
  assert (clock_trigger);

  clock_trigger->cancel ();
  clock_trigger = NULL;
}

DECRTCDevice::ClockTrigger::ClockTrigger (DECRTCDevice *clock_device) throw ()
  : rtc (clock_device)
{
  assert (rtc);
}

DECRTCDevice::ClockTrigger::~ClockTrigger () throw ()
{
}

void
DECRTCDevice::ClockTrigger::real_task ()
{
  rtc->ready_clock ();
}
