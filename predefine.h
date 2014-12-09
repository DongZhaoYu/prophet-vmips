/* Main driver program for Prophet.
   Copyright2008 DongZhaoyu, GaoBing.

This file is part of Prophet.

Prophet is free software developed based on VMIPS, you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

Prophet is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with VMIPS; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _PREDEFINE_H
#define _PREDEFINE_H

#include <string>

#define GET_ACCESSOR(t, n) t Get##n() const { return m_##n; }
#define SET_ACCESSOR(t, n, v) void Set##n(t v) { m_##n = v; }
#define SET_GET_ACCESSOR(t, n, v) GET_ACCESSOR(t, n) SET_ACCESSOR(t, n, v)
#define GET_SET_ACCESSOR(t, n, v) SET_ACCESSOR(t, n) GET_ACCESSOR(t, n, v)

#define __PROPHET

class ProphetException
{
public:
	ProphetException() {}
	ProphetException(std::string s) : m_Reason(s) {}
	~ProphetException() {}

	GET_ACCESSOR(std::string, Reason);
private:
	std::string m_Reason;
};

#endif
