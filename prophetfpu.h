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

#ifndef _PROPHETFPU_H
#define _PROPHETFPU_H

#include <cstdio>
#include "types.h"

class SpeculativeCPU;
class IntCtrl;

#define FPU_REG_NUMBER 32

class FPU
{
public:
	uint32 freg[32];
	SpeculativeCPU *speculativecpu;
	IntCtrl *intc;

	void interpret_float_value(uint32,float&);
	void interpret_double_value(uint64,double &d);

	uint32 uninterpret_float_value(float f);
	uint64 uninterpret_double_value(double d);

	void mfc1_emulate(uint32 instr, uint32 pc);
	void mtc1_emulate(uint32 instr, uint32 pc);
	void adds_emulate(uint32 instr, uint32 pc);
	void addd_emulate(uint32 instr, uint32 pc);
	void subs_emulate(uint32 instr, uint32 pc);
	void subd_emulate(uint32 instr, uint32 pc);
	void muls_emulate(uint32 instr, uint32 pc);
	void muld_emulate(uint32 instr, uint32 pc);
	void divs_emulate(uint32 instr, uint32 pc);
	void divd_emulate(uint32 instr, uint32 pc);
	void abss_emulate(uint32 instr, uint32 pc);
	void absd_emulate(uint32 instr, uint32 pc);
	void movs_emulate(uint32 instr, uint32 pc);
	void movd_emulate(uint32 instr, uint32 pc);
	void negs_emulate(uint32 instr, uint32 pc);
	void negd_emulate(uint32 instr, uint32 pc);
	void cvtsd_emulate(uint32 instr, uint32 pc);
	void cvtsw_emulate(uint32 instr, uint32 pc);
	void cvtds_emulate(uint32 instr, uint32 pc);
	void cvtdw_emulate(uint32 instr, uint32 pc);
	void cvtws_emulate(uint32 instr, uint32 pc);
	void cvtwd_emulate(uint32 instr, uint32 pc);
	void csfs_emulate(uint32 instr, uint32 pc);
	void csfd_emulate(uint32 instr, uint32 pc);
	void cseqs_emulate(uint32 instr, uint32 pc);
	void cseqd_emulate(uint32 instr, uint32 pc);
	void clts_emulate(uint32 instr, uint32 pc);
	void cltd_emulate(uint32 instr, uint32 pc);
	void cles_emulate(uint32 instr, uint32 pc);
	void cled_emulate(uint32 instr, uint32 pc);
	void bc1f_emulate(uint32 instr, uint32 pc);
	void bc1t_emulate(uint32 instr, uint32 pc);
	void cpone_emulate(uint32 instr, uint32 pc);

	FPU(SpeculativeCPU*m,IntCtrl *i):speculativecpu(m),intc(i){}

	void Clear_condition(uint32&);
	void Set_condition(uint32&);
	void reset();
	bool Float_is_NAN(uint32);
	bool Double_is_NAN(uint64);
	bool Fpcondition(uint32);

	uint16 ft(const uint32 instr) const;

private:
	uint16 fs(const uint32 instr) const;
	uint16 fd(const uint32 instr) const;
};

#endif /*_PROPHETFPU_H*/