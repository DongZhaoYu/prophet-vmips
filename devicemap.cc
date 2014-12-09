/* Base class for devices that are memory-mapped into the CPU's
   address space.
   Copyright 2001, 2003 Brian R. Gaeke.
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

/* Routines implementing memory mappings for devices which support
 * memory-mapped I/O. */

#include "accesstypes.h"
#include "range.h"
#include "devicemap.h"

uint16
DeviceMap::fetch_halfword(uint32 offset, DeviceExc *client)
{
    const uint32 word_data = fetch_word(offset & ~0x03, DATALOAD, client);
    const uint32 halfword_offset_in_word = ((offset & 0x03) >> 1);
    const uint16 *halfwordptr = reinterpret_cast<const uint16 *>(&word_data);
    return halfwordptr[halfword_offset_in_word];
}

uint8
DeviceMap::fetch_byte(uint32 offset, DeviceExc *client)
{
    const uint32 word_data = fetch_word(offset & ~0x03, DATALOAD, client);
    const uint32 byte_offset_in_word = (offset & 0x03);
    const uint8 *byteptr = reinterpret_cast<const uint8 *>(&word_data);
    return byteptr[byte_offset_in_word];
}

void
DeviceMap::store_halfword(uint32 offset, uint16 data, DeviceExc *client)
{
    const uint32 word_offset = offset & 0xfffffffc;
    const uint32 halfword_offset_in_word = ((offset & 0x02) >> 1);
    uint32 word_data = 0;
    uint16 *halfwordptr = reinterpret_cast<uint16 *>(&word_data);
    halfwordptr[halfword_offset_in_word] = data;
    store_word(word_offset, word_data, client);
}

void
DeviceMap::store_byte(uint32 offset, uint8 data, DeviceExc *client)
{
    const uint32 word_offset = offset & 0xfffffffc;
    const uint32 byte_offset_in_word = (offset & 0x03);
    uint32 word_data = 0;
    uint8 *byteptr = reinterpret_cast<uint8 *>(&word_data);
    byteptr[byte_offset_in_word] = data;
    store_word(word_offset, word_data, client);
}

bool
DeviceMap::canRead(uint32 offset) throw()
{
    return true;
}

bool
DeviceMap::canWrite(uint32 offset) throw()
{
    return true;
}
