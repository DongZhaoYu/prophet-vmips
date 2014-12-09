/* Definitions to support the physical memory system.
   Copyright 2001, 2003 Brian R. Gaeke.

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

#ifndef _MAPPER_H_
#define _MAPPER_H_

#include "range.h"
#include <cstdio>
#include <vector>
class DeviceExc;

class Mapper {
public:
	/* We keep lists of ranges in a vector of pointers to range
	   objects. */
	typedef std::vector<Range *> Ranges;

	struct BusErrorInfo {
	  bool valid;
	  DeviceExc *client;
	  int32 mode;
	  uint32 addr;
	  int32 width;
	  uint32 data;
	};

	BusErrorInfo last_berr_info;

private:
	/* A pointer to the last mapping that was successfully returned by
	   find_mapping_range. */
	Range *last_used_mapping;

	/* A list of all currently mapped ranges. */
	Ranges ranges;

	bool opt_bigendian;
	bool byteswapped;
public:
	Mapper();
	~Mapper();

	/* Add range R to the mapping. R must not overlap with any existing
	 * ranges in the mapping. Return 0 if R added sucessfully or -1 if
	 * R overlapped with an existing range. 
	 */
	int add_range (Range *r);
	/* Add range R to the mapping, as with add_range(), but first set its
	 * base address to PA.
	 */
	int map_at_physical_address (Range *r, uint32 pa) {
		r->setBase (pa); return add_range (r);
	}
	uint32 swap_word(uint32 w);
	uint16 swap_halfword(uint16 h);
	uint32 mips_to_host_word(uint32 w);
	uint32 host_to_mips_word(uint32 w);
	uint16 mips_to_host_halfword(uint16 h);
	uint16 host_to_mips_halfword(uint16 h);
	void bus_error (DeviceExc *client, int32 mode, uint32 addr, int32 width,
		uint32 data = 0xffffffff);
	uint32 fetch_word(uint32 addr, int32 mode, bool cacheable,
		DeviceExc *client);
	uint16 fetch_halfword(uint32 addr, bool cacheable, DeviceExc *client);
	uint8 fetch_byte(uint32 addr, bool cacheable, DeviceExc *client);
	void store_word(uint32 addr, uint32 data, bool cacheable,
		DeviceExc *client);
	void store_halfword(uint32 addr, uint16 data, bool cacheable,
		DeviceExc *client);
	void store_byte(uint32 addr, uint8 data, bool cacheable,
		DeviceExc *client);
	Range *find_mapping_range(uint32 p) throw();
	void dump_stack(FILE *f, uint32 stackphys);
	void get_last_berr_info (BusErrorInfo &info) { info = last_berr_info; }
};

#endif /* _MAPPER_H_ */
