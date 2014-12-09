/* Declaration of the halt device.
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

#ifndef _HALTDEV_H_
#define _HALTDEV_H_

#include "devicemap.h"
class vmips;

class HaltDevice : public DeviceMap
{
public:
	HaltDevice( vmips *machine ) throw();
	virtual ~HaltDevice() throw();

	/* Ignore vaild reads. */
	virtual uint32 fetch_word(uint32 offset, int mode, DeviceExc *client);

	/* Halt the machine when a non zero word is written. */
	virtual void store_word(uint32 offset, uint32 data, DeviceExc *client);

	/* Return a string describing the device. */
	char *descriptor_str();

protected:
	vmips	*machine;
};

#endif /* _HALTDEV_H_ */
