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

#ifndef _SPECULATIVELOGIC_H
#define _SPECULATIVELOGIC_H

#include "speculativecpu.h"
#include <list>
#include "prophetlog.h"

class SpeculativeLogic
{
	PLOG_CLASS(SpeculativeLogic);
public:
	typedef std::list<SpeculativeCPU*> SpList;
	typedef std::list<SpeculativeCPU*>::iterator SpListIterator;
	typedef std::list<SpeculativeCPU*>::const_iterator SpListConstIterator;

	static SpeculativeLogic* GetInstance();

	void SwapToSpList(SpeculativeCPU*, SpeculativeCPU*);
	void SwapToFreeList(SpeculativeCPU*);

	SpeculativeCPU* GetParent(SpeculativeCPU*);
	SpeculativeCPU* GetSubthread(SpeculativeCPU*);

	SpeculativeCPU* NotifySpawn(SpeculativeCPU*);
	void SquashSubthread(SpeculativeCPU*, SpeculativeCPU::SQUASH_REASON);
	bool VerifySubthread(SpeculativeCPU*);
	bool PassStableToken(SpeculativeCPU*);
	uint32 GetStackTop(SpeculativeCPU*);
	SpeculativeCPU* GetRunner();

	void Step(void);
	void Reset(void);
	bool CheckStateOnQuit();		//check if the machine quit normally or not

private:
	SpeculativeLogic();

	static SpeculativeLogic* m_Instance;
	SpList m_FreeList;
	SpList m_SpList;

	SpeculativeCPU *m_CPU;			//the cpu is currently running
};

#endif