/*	Implementation of VMIPS halt device.
	Copyright 2002 Paul Twohey.

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

#include "haltdev.h"
#include "vmips.h"

#include <cassert>

HaltDevice::HaltDevice( vmips *machine ) throw()
	: machine( machine )
{
	assert( machine );

	// XXX hack until we get ranges working properly
	extent = 4;
}

HaltDevice::~HaltDevice() throw()
{
}

uint32 HaltDevice::fetch_word(uint32 offset, int mode, DeviceExc *client)
{
	switch( offset / 4 ) {
	case 0:		// halt control
		return 0;
	default:
		assert( ! "reached" );
		return 0;
	}
}

void HaltDevice::store_word( uint32 offset, uint32 data, DeviceExc *client )
{
	switch( offset / 4 ) {
	case 0:
		if( data )
			machine->halt();
		break;
	default:
		assert( ! "reached" );
	}
}

char *HaltDevice::descriptor_str()
{
	return "Halt device";
}
