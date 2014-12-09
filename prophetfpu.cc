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

#include "prophetfpu.h"
#include "speculativecpu.h"
#include "deviceexc.h"
#include "intctrl.h"
#include "excnames.h"

#define FCSR 31

void FPU::Set_condition(uint32 &x)
{
	x = x | (1 << 22);
}

void FPU::Clear_condition(uint32 &x)
{
	x = x & ~(1 << 22);
}

void FPU::reset()
{
	int i;
	for(i = 0; i < FPU_REG_NUMBER;i++)
	{
		freg[i] = 0;
	}
}

bool FPU::Float_is_NAN(uint32 x)
{
	return (((x >> 23) & 0xff) == 0xff);
}

bool FPU::Double_is_NAN(uint64 x)
{
	return (((x >> 52) & 0x07ff) == 0x7ff);
}

bool FPU::Fpcondition(uint32 x)
{
	return (((x >> 22) & 0x01) == 1);
}

void FPU::interpret_float_value(uint32 x,float &f)   //change the uint32 to float
{
	int n_frac = 23, n_exp = 8;
	int i,  sign = 0, exponent;
	float fraction;

	if(Float_is_NAN(x))
	{
		speculativecpu->exception(Ov);
		return;
	}
	exponent = (x >> n_frac) & ((1 << n_exp) - 1);
	exponent -= (1 << (n_exp-1)) - 1;
	fraction = 0.0;
	sign = (x >> 31) & 1;
	fraction = 0.0;
		for (i=0; i<n_frac; i++) {
			int bit = (x >> i) & 1;
			fraction /= 2.0;
			if (bit)
				fraction += 1.0;
		}
		fraction = (fraction / 2.0) + 1.0;

	f = fraction;
	if (exponent > 0) {
		if (exponent == 128)
			exponent = 127;
		while (exponent-- > 0)
			f *= 2.0;
	} 
	else if (exponent < 0) 
	{
		while (exponent++ < 0)
			f /= 2.0;
	}
	if (sign)
		f = -f;
}

uint32 FPU::uninterpret_float_value(float nf)       //change the float to uint32
{
	int n_frac = 23, n_exp = 8, signofs=31;
	int i, exponent;
	uint32 r = 0;
	if (nf < 0.0) 
	{
		r |= ((uint32)1 << signofs);
		nf = -nf;
	}
	exponent = 0;
	while (nf < 1.0 && exponent > -127) 
	{
		nf *= 2.0;
		exponent --;
	}
		
	while (nf >= 2.0 && exponent < 127) 
	{
		nf /= 2.0;
		exponent ++;
	}
	nf -= 1.0;
	for (i=n_frac-1; i>=0; i--) 
	{
		nf *= 2.0;
		if (nf >= 1.0) 
		{
			r |= ((uint32)1 << i);
			nf -= 1.0;
		}
	}
	exponent += (((uint32)1 << (n_exp-1)) - 1);
	if (exponent < 0)
		exponent = 0;
	if (exponent >= ((uint32)1 << n_exp))
		exponent = ((uint32)1 << n_exp) - 1;
	r |= (uint32)exponent << n_frac;
	if (exponent == 0)
		r = 0;
	if(Float_is_NAN(r))
	{
		speculativecpu->exception(Ov);
		return 0;
	}
	return r;
}

void FPU::interpret_double_value(uint64 x,double &f)    //change the uint64 to double
{
	int n_frac = 52, n_exp = 11;
	int i,  sign = 0, exponent;
	double fraction;

	if(Double_is_NAN(x))
	{
		speculativecpu->exception(Ov);
		return;
	}
	exponent = (x >> n_frac) & ((1 << n_exp) - 1);
	exponent -= (1 << (n_exp-1)) - 1;
	fraction = 0.0;
	sign = (x >> 63) & 1;
	fraction = 0.0;
		for (i=0; i<n_frac; i++) {
			int bit = (x >> i) & 1;
			fraction /= 2.0;
			if (bit)
				fraction += 1.0;
		}
		fraction = (fraction / 2.0) + 1.0;

	/*  form the value:  */
	f = fraction;
	if (exponent > 0) {
		if (exponent == 1024)
			exponent = 1023;
		while (exponent-- > 0)
			f *= 2.0;
	} 
	else if (exponent < 0) 
	{
		while (exponent++ < 0)
			f /= 2.0;
	}
	if (sign)
		f = -f;
}

uint64 FPU::uninterpret_double_value(double nf)      //change the double to uint64
{
	int n_frac = 52, n_exp = 11, signofs=63;
	int i, exponent;
	uint64 r = 0;
	if (nf < 0.0) 
	{
		r |= ((uint64)1 << signofs);
		nf = -nf;
	}
	exponent = 0;
	while (nf < 1.0 && exponent > -1023) 
	{
		nf *= 2.0;
		exponent --;
	}
		
	while (nf >= 2.0 && exponent < 1023) 
	{
		nf /= 2.0;
		exponent ++;
	}
	nf -= 1.0;
	for (i=n_frac-1; i>=0; i--) 
	{
		nf *= 2.0;
		if (nf >= 1.0) 
		{
			r |= ((uint64)1 << i);
			nf -= 1.0;
		}
	}
	exponent += (((uint64)1 << (n_exp-1)) - 1);
	if (exponent < 0)
		exponent = 0;
	if (exponent >= ((uint64)1 << n_exp))
		exponent = ((uint64)1 << n_exp) - 1;
	r |= (uint64)exponent << n_frac;
	if (exponent == 0)
		r = 0;
	if(Double_is_NAN(r))
	{
		speculativecpu->exception(Ov);
		return 0;
	}
	return r;
}

uint16 FPU::ft(const uint32 i) const
{
	return (i>>16)&0x01f;
}

uint16 FPU::fs(const uint32 i) const
{
	return (i>>11)&0x01f;
}

uint16 FPU::fd(const uint32 i) const
{
	return (i>>6)&0x01f;
}

void FPU::mfc1_emulate(uint32 instr, uint32 pc)    //GPR[rt] = FPR[fs]
{
	uint32 a;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	speculativecpu->WriteLocalReg(speculativecpu->rt(instr),a);
}

void FPU::mtc1_emulate(uint32 instr, uint32 pc)    //FPR[fs] = GPR[rt]
{
	uint32 a;
	a = speculativecpu->ReadLocalReg(speculativecpu->rt(instr));
	speculativecpu->WriteLocalFreg(fs(instr),a);
}

void FPU::adds_emulate(uint32 instr, uint32 pc)    //FPR[fd] = FPR[fs] + FPR[ft]
{
	float a,b,c;
	int sign;
	uint32 d,e,f;
	d = speculativecpu->ReadLocalFreg(fs(instr));	
	e = speculativecpu->ReadLocalFreg(ft(instr));
	interpret_float_value(d,a);
	interpret_float_value(e,b);
	c = a + b;
	f = uninterpret_float_value(c);
	sign = (f>>31)&1;
	if((Float_is_NAN(f)) || (a<0 && b<0 && !sign) || (a>=0 && b>= 0 && sign))  // exception(call CPU add overflow)
	{
		speculativecpu->exception(Ov);
		return;
	}
	else
	{
		speculativecpu->WriteLocalFreg(fd(instr),f);
	}
}

void FPU::addd_emulate(uint32 instr, uint32 pc)
{
	double a,b,c;
	int sign;
	uint64 d,e,f;
	uint32 m,n;
	m = speculativecpu->ReadLocalFreg(fs(instr));
	n = speculativecpu->ReadLocalFreg(fs(instr)+1);
	d = ((uint64)n<<32) | m;
	m = speculativecpu->ReadLocalFreg(ft(instr));
	n = speculativecpu->ReadLocalFreg(ft(instr)+1);
	e = ((uint64)n<<32) | m;
	interpret_double_value(d,a);
	interpret_double_value(e,b);
	c = a + b;
	f = uninterpret_double_value(c);
	sign = (f>>63)&1;
	if(Double_is_NAN(f) || (a<0 && b<0 && !sign) || (a>=0 && b>= 0 && sign))
	{
		speculativecpu->exception(Ov);
		return;
	}
	else
	{
		m = f&0xffffffff;
		n = (f>>32)&0xffffffff;
		speculativecpu->WriteLocalFreg(fd(instr),m);
		speculativecpu->WriteLocalFreg(fd(instr)+1,n);
	}
}

void FPU::subs_emulate(uint32 instr, uint32 pc)   //FPR[fd] = FPR[fs] - FPR[ft]
{
	float a,b,c;
	int sign;
	uint32 d,e,f;
	d = speculativecpu->ReadLocalFreg(fs(instr));
	e = speculativecpu->ReadLocalFreg(ft(instr));
	interpret_float_value(d,a);
	interpret_float_value(e,b);
	c = a - b;
	f = uninterpret_float_value(c);
	sign = (f>>31)&1;
	if(Float_is_NAN(f) || (a<0 && !(b<0) && !sign) || (!(a<0) && b<0 && sign))
	{
		speculativecpu->exception(Ov);
		return;
	}
	else
	{
		speculativecpu->WriteLocalFreg(fd(instr),f);
	}
}

void FPU::subd_emulate(uint32 instr, uint32 pc)
{
	double a,b,c;
	int sign;
	uint64 d,e,f;
	uint32 m,n;
	m = speculativecpu->ReadLocalFreg(fs(instr));
	n = speculativecpu->ReadLocalFreg(fs(instr)+1);
	d = ((uint64)n<<32) | m;
	m = speculativecpu->ReadLocalFreg(ft(instr));
	n = speculativecpu->ReadLocalFreg(ft(instr)+1);
	e = ((uint64)n<<32) | m;
	interpret_double_value(d,a);
	interpret_double_value(e,b);
	c = a - b;
	f = uninterpret_double_value(c);
	sign = (f>>63)&1;	
	if(Double_is_NAN(f) || (a<0 && !(b<0) && !sign) || (!(a<0) && b<0 && sign))
	{
		speculativecpu->exception(Ov);
		return;
	}
	else
	{
		m = f&0xffffffff;
		n = (f>>32)&0xffffffff;
		speculativecpu->WriteLocalFreg(fd(instr),m);
		speculativecpu->WriteLocalFreg(fd(instr)+1,n);
	}
}

void FPU::muls_emulate(uint32 instr, uint32 pc)    //FPR[fd] = FPR[fs] * FPR[ft]
{
	float a,b,c;
	uint32 d,e,f;
	d = speculativecpu->ReadLocalFreg(fs(instr));
	e = speculativecpu->ReadLocalFreg(ft(instr));
	interpret_float_value(d,a);
	interpret_float_value(e,b);
	c = a * b;
	f = uninterpret_float_value(c);
	if(Float_is_NAN(f))
	{
		speculativecpu->exception(Ov);
		return;
	}
	speculativecpu->WriteLocalFreg(fd(instr),f);
}

void FPU::muld_emulate(uint32 instr, uint32 pc)
{
	double a,b,c;
	uint64 d,e,f;
	uint32 m,n;
	m = speculativecpu->ReadLocalFreg(fs(instr));
	n = speculativecpu->ReadLocalFreg(fs(instr)+1);
	d = ((uint64)n<<32) | m;
	m = speculativecpu->ReadLocalFreg(ft(instr));
	n = speculativecpu->ReadLocalFreg(ft(instr)+1);
	e = ((uint64)n<<32) | m;
	interpret_double_value(d,a);
	interpret_double_value(e,b);
	c = a * b;
	f = uninterpret_double_value(c);
	if(Double_is_NAN(f))
	{
		speculativecpu->exception(Ov);
		return;
	}
	m = f&0xffffffff;
	n = (f>>32)&0xffffffff;
	speculativecpu->WriteLocalFreg(fd(instr),m);
	speculativecpu->WriteLocalFreg(fd(instr)+1,n);
}

void FPU::divs_emulate(uint32 instr, uint32 pc)    //FPR[fd] = FPR[fs] / FPR[ft]
{
	float a,b,c;
	uint32 d,e,f;
	d = speculativecpu->ReadLocalFreg(fs(instr));
	e = speculativecpu->ReadLocalFreg(ft(instr));
	interpret_float_value(d,a);
	interpret_float_value(e,b);
	if(!(b>0) && !(b<0))
	{
		speculativecpu->exception(Ov);
		return;
	}
	else
	{
		c = a / b;
		f = uninterpret_float_value(c);
		if(Float_is_NAN(f))
		{
			speculativecpu->exception(Ov);
			return;
		}
		speculativecpu->WriteLocalFreg(fd(instr),f);
	}
}

void FPU::divd_emulate(uint32 instr, uint32 pc)
{
	double a,b,c;
	uint64 d,e,f;
	uint32 m,n;
	m = speculativecpu->ReadLocalFreg(fs(instr));
	n = speculativecpu->ReadLocalFreg(fs(instr)+1);
	d = ((uint64)n<<32) | m;
	m = speculativecpu->ReadLocalFreg(ft(instr));
	n = speculativecpu->ReadLocalFreg(ft(instr)+1);
	e = ((uint64)n<<32) | m;
	interpret_double_value(d,a);
	interpret_double_value(e,b);
	if(!(b>0) && !(b<0))
	{
		speculativecpu->exception(Ov);
		return;
	}
	else
	{
		c = a / b;
		f = uninterpret_double_value(c);
		if(Double_is_NAN(f))
		{
			speculativecpu->exception(Ov);
			return;
		}
		m = f&0xffffffff;
		n = (f>>32)&0xffffffff;
		speculativecpu->WriteLocalFreg(fd(instr),m);
		speculativecpu->WriteLocalFreg(fd(instr)+1,n);
	}
}

void FPU::abss_emulate(uint32 instr, uint32 pc)     //FPR[fd] = abs(FPR[fs])
{
	uint32 a,b;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = a & 0x7fffffff;
	speculativecpu->WriteLocalFreg(fd(instr),b);
}

void FPU::absd_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b,c;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	c = b & 0x7fffffff;
	speculativecpu->WriteLocalFreg(fd(instr),a);
	speculativecpu->WriteLocalFreg(fd(instr)+1,c);
}

void FPU::movs_emulate(uint32 instr, uint32 pc)    //FPR[fd] = FPR[fs]
{
	uint32 a;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	speculativecpu->WriteLocalFreg(fd(instr),a);
}

void FPU::movd_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	speculativecpu->WriteLocalFreg(fd(instr),a);
	speculativecpu->WriteLocalFreg(fd(instr)+1,b);
}

void FPU::negs_emulate(uint32 instr, uint32 pc)    //FPR[fd] = -FPR[fs]
{
	uint32 a;
	int sign;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	sign = (a>>31) & 1;
	if(sign)
	{
		a = a & 0x7fffffff;
	}
	else
	{
		a = a | (1<<31);
	}
	speculativecpu->WriteLocalFreg(fd(instr),a);
}

void FPU::negd_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	int32 sign;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	sign = (b>>31) & 1;
	if(sign)
	{
		b = b & 0x7fffffff;
	}
	else
	{
		b = b | (1<<31);
	}
	speculativecpu->WriteLocalFreg(fd(instr),a);
	speculativecpu->WriteLocalFreg(fd(instr)+1,b);
}

void FPU::cvtsd_emulate(uint32 instr, uint32 pc)   //FPR[fd] = convert_and_round(GPR[fs])
{
	uint32 a,b;
	uint64 c;
	float m;
	double n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	c = ((uint64)b<<32) | a;
	interpret_double_value(c,n);
	m = (float)n;
	a = uninterpret_float_value(m);
	speculativecpu->WriteLocalFreg(fd(instr),a);
}

void FPU::cvtsw_emulate(uint32 instr, uint32 pc)
{
	uint32 a;
	int b;
	float m;
	b = (int)speculativecpu->ReadLocalFreg(fs(instr));
	m = (float)b;
	a = uninterpret_float_value(m);
	speculativecpu->WriteLocalFreg(fd(instr),a);
}

void FPU::cvtdw_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	uint64 c;
	int m;
	double n;
	m = (int)speculativecpu->ReadLocalFreg(fs(instr));
	n = (double)m;
	c = uninterpret_double_value(n);
	b = c & 0xffffffff;
	a = (c>>32) & 0xffffffff;
	speculativecpu->WriteLocalFreg(fd(instr)+1,a);
	speculativecpu->WriteLocalFreg(fd(instr),b);
}

void FPU::cvtds_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	uint64 c;
	float m;
	double n;
	a = (int)speculativecpu->ReadLocalFreg(fs(instr));
	interpret_float_value(a,m);
	n = (double)m;
	c = uninterpret_double_value(n);
	b = c & 0xffffffff;
	a = (c>>32) & 0xffffffff;
	speculativecpu->WriteLocalFreg(fd(instr)+1,a);
	speculativecpu->WriteLocalFreg(fd(instr),b);
}

void FPU::cvtws_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	int m;
	float n;
	b = speculativecpu->ReadLocalFreg(fs(instr));
	interpret_float_value(b,n);	
	m = (int)n;
	a = (uint32)m;
	speculativecpu->WriteLocalFreg(fd(instr),a);
}

void FPU::cvtwd_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	uint64 c;
	int m;
	double n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	c = ((uint64)b<<32) | a;
	interpret_double_value(c,n);
	m = (int)n;
	a = (uint32)m;
	speculativecpu->WriteLocalFreg(fd(instr),a);
}

void FPU::csfs_emulate(uint32 instr, uint32 pc)
{
	Clear_condition(freg[FCSR]);
}

void FPU::csfd_emulate(uint32 instr, uint32 pc)
{
	Clear_condition(freg[FCSR]);
}

void FPU::cseqs_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	float m,n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(ft(instr));
	interpret_float_value(a,m);
	interpret_float_value(b,n);
	if(m == n)
		Set_condition(freg[FCSR]);
	else
		Clear_condition(freg[FCSR]);
}

void FPU::cseqd_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	uint64 c,d;
	double m,n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	c = ((uint64)b << 32) | a;
	a = speculativecpu->ReadLocalFreg(ft(instr));
	b = speculativecpu->ReadLocalFreg(ft(instr)+1);
	d = ((uint64)b << 32) | a;
	
	interpret_double_value(c,m);
	interpret_double_value(d,n);
	if(m == n)
		Set_condition(freg[FCSR]);
	else
		Clear_condition(freg[FCSR]);
}

void FPU::clts_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	float m,n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(ft(instr));
	interpret_float_value(a,m);
	interpret_float_value(b,n);
	if(m < n)
		Set_condition(freg[FCSR]);
	else
		Clear_condition(freg[FCSR]);
}

void FPU::cltd_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	uint64 c,d;
	double m,n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	c = ((uint64)b << 32) | a;
	a = speculativecpu->ReadLocalFreg(ft(instr));
	b = speculativecpu->ReadLocalFreg(ft(instr)+1);
	d = ((uint64)b << 32) | a;
	
	interpret_double_value(c,m);
	interpret_double_value(d,n);
	if(m < n)
		Set_condition(freg[FCSR]);
	else
		Clear_condition(freg[FCSR]);
}

void FPU::cles_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	float m,n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(ft(instr));
	interpret_float_value(a,m);
	interpret_float_value(b,n);
	if(m <= n)
		Set_condition(freg[FCSR]);
	else
		Clear_condition(freg[FCSR]);
}

void FPU::cled_emulate(uint32 instr, uint32 pc)
{
	uint32 a,b;
	uint64 c,d;
	double m,n;
	a = speculativecpu->ReadLocalFreg(fs(instr));
	b = speculativecpu->ReadLocalFreg(fs(instr)+1);
	c = ((uint64)b << 32) | a;
	a = speculativecpu->ReadLocalFreg(ft(instr));
	b = speculativecpu->ReadLocalFreg(ft(instr)+1);
	d = ((uint64)b << 32) | a;
	
	interpret_double_value(c,m);
	interpret_double_value(d,n);
	if(m <= n)
		Set_condition(freg[FCSR]);
	else
		Clear_condition(freg[FCSR]);
}

void FPU::bc1f_emulate(uint32 instr, uint32 pc)
{
	if(!(Fpcondition(freg[FCSR])))
		speculativecpu->branch(instr,pc);
}

void FPU::bc1t_emulate(uint32 instr, uint32 pc)
{
	if(Fpcondition(freg[FCSR]))
		speculativecpu->branch(instr,pc);
}

void FPU::cpone_emulate(uint32 instr, uint32 pc)
{
	if(speculativecpu->rs(instr) == 16)
	{
		switch(speculativecpu->funct(instr))
		{
		case 0:  adds_emulate(instr, pc);break;
		case 1:  subs_emulate(instr, pc);break;
		case 2:  muls_emulate(instr, pc);break;
		case 3:  divs_emulate(instr, pc);break;
		case 5:  abss_emulate(instr, pc);break;
		case 6:  movs_emulate(instr, pc);break;
		case 7:  negs_emulate(instr, pc);break;
		case 33: cvtds_emulate(instr, pc);break;
		case 36: cvtws_emulate(instr, pc);break;
		case 56: csfs_emulate(instr,pc);break;
		case 58: cseqs_emulate(instr,pc);break;
		case 60: clts_emulate(instr,pc);break;
		case 62: cles_emulate(instr,pc);break;
		default: speculativecpu->exception(RI); break;
		}
	}
	else if(speculativecpu->rs(instr) == 17)
	{
		switch(speculativecpu->funct(instr))
		{
		case 0:  addd_emulate(instr, pc);break;
		case 1:  subd_emulate(instr, pc);break;
		case 2:  muld_emulate(instr, pc);break;
		case 3:  divd_emulate(instr, pc);break;
		case 5:  absd_emulate(instr, pc);break;
		case 6:  movd_emulate(instr, pc);break;
		case 7:  negd_emulate(instr, pc);break;
		case 32: cvtsd_emulate(instr, pc);break;
		case 36: cvtwd_emulate(instr, pc);break;
		case 56: csfd_emulate(instr,pc);break;
		case 58: cseqd_emulate(instr,pc);break;
		case 60: cltd_emulate(instr,pc);break;
		case 62: cled_emulate(instr,pc);break;
		default: speculativecpu->exception(RI); break;
		}	
	}
	else if(speculativecpu->rs(instr) == 0)
	{
		mfc1_emulate(instr, pc);
	}
	else if(speculativecpu->rs(instr) == 4)
	{
		mtc1_emulate(instr, pc);
	}
	else if(speculativecpu->rs(instr) == 8)
	{
		switch(ft(instr))
		{
			case 0: bc1f_emulate(instr,pc);break;
			case 1: bc1t_emulate(instr,pc);break;
			default: speculativecpu->exception(RI); break;
		}
	}
	else if(speculativecpu->rs(instr) == 20)
	{
		if(speculativecpu->funct(instr) == 32)
		{
			cvtsw_emulate(instr, pc);
		}
		else if(speculativecpu->funct(instr) == 33)
		{
			cvtdw_emulate(instr, pc);
		}
		else
		{
			speculativecpu->exception(RI);
		}
	}
	else
	{
		speculativecpu->exception(RI);
	}
}
