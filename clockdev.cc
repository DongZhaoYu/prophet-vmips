/* Implementation of VMIPS clock device.
   Copyright 2003 Paul Twohey.

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

#include "clockdev.h"
#include "devreg.h"

#include <cassert>

ClockDevice::ClockDevice( Clock *clock, uint32 irq, long frequency_ns )
	throw( std::bad_alloc )
	: DeviceMap(20), irq(irq), frequency_ns(frequency_ns),
	  clock(clock), clock_trigger(0), clock_state(UNREADY),
	  interrupt_enabled(false)
{
	assert (clock && "ClockDevice initialized with null Clock");
	assert (frequency_ns > 0
            && "ClockDevice initialized with non-positive frequency");
	assert (irq != IRQ0 && irq != IRQ1
            && "ClockDevice initialized with invalid IRQ");

	clock_trigger = new ClockTrigger(this);
	clock->add_deferred_task( clock_trigger, frequency_ns );
}

ClockDevice::~ClockDevice() throw()
{
	assert( clock_trigger );
	
	clock_trigger->cancel();
	clock_trigger = NULL;
}

void ClockDevice::ready_clock() throw( std::bad_alloc )
{
	clock_state = READY;
 	
	if( interrupt_enabled )
		assertInt( irq );
	
	clock_trigger = new ClockTrigger( this );
	clock->add_deferred_task( clock_trigger, frequency_ns );
}

void ClockDevice::unready_clock() throw()
{
	clock_state = UNREADY;
	deassertInt( irq );
}

uint32 ClockDevice::fetch_word( uint32 offset, int mode, DeviceExc *client )
{
	switch( offset / 4 ) {
	case 0:		// real time - seconds
		timeval real_time;
		gettimeofday( &real_time, NULL );
		return real_time.tv_sec;
		break;
	case 1:		// real time - microseconds
		gettimeofday( &real_time, NULL );
		return real_time.tv_usec;
		break;
	case 2:		// simulated time - seconds
		return clock->get_time().tv_sec;
		break;
	case 3:		// simulated time - microseconds
		return clock->get_time().tv_nsec / 1000;
		break;
	case 4:		// control word
	{
		uint32 word = interrupt_enabled ? CTL_RDY : 0;
		word |= clock_state;
		unready_clock();
		return word;
		break;
	}
	default:
		assert( ! "reached" );
		return 0;
	}
}

void ClockDevice::store_word( uint32 offset, uint32 data, DeviceExc *client )
{
	switch( offset / 4 ) {
	case 0:		// real time - seconds
	case 1:		// real time - micro seconds
		return;
	case 2:		// simulated time - seconds
	{
		if( (int32)data < 0 )
			return;

		timespec time = { data, clock->get_time().tv_nsec };
		clock->set_time( time );
		return;
	}
	case 3:		// simulated time - micro seconds
	{
		if( (int32)data < 0 )
			return;

		timespec time =  { clock->get_time().tv_sec, data };
		clock->set_time( time );
		return;
	}
	case 4:		// control word
		interrupt_enabled = data & CTL_IE;
		if( interrupt_enabled && clock_state == READY )
			assertInt( irq );
		return;
	default:
		assert( ! "reached" );
	}
}

const char *ClockDevice::descriptor_str() const
{
	return "Clock device";
}


ClockDevice::ClockTrigger::ClockTrigger( ClockDevice *clock_device ) throw()
	: clock_device( clock_device )
{
	assert( clock_device );
}

ClockDevice::ClockTrigger::~ClockTrigger() throw()
{
}

void ClockDevice::ClockTrigger::real_task()
{
	clock_device->ready_clock();
}
