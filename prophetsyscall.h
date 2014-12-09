/* Main driver program for Prophet.
   Copyright2009 DongZhaoyu GaoBing.

This file is part of Prophet. A set of system-like service was
implemented in prophet syscall.

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

#ifndef _PROPHETSYSCALL_H
#define _PROPHETSYSCALL_H

#include <string>

/*Output service*/
#define PRINT_INT_SYSCALL 	1
#define PRINT_FLOAT_SYSCALL 	2
#define PRINT_DOUBLE_SYSCALL 	3
#define PRINT_STRING_SYSCALL	4
#define PRINT_CHAR_SYSCALL	5

/*input service*/
#define READ_INT_SYSCALL	6
#define READ_FLOAT_SYSCALL	7
#define READ_DOUBLE_SYSCALL	8
#define READ_STRING_SYSCALL	9
#define READ_CHAR_SYSCALL	10

/*these macros for syscall in myprintf*/
//this is the stack record size of myprintf function in mylib.s, if the size changed, remember to update this value!
/*
#define prophet_myprintf_stack_record_size	128
#define prophet_va_list		uint32
#define prophet_va_start(v)	v = (prophet_va_list)ReadLocalReg(29) + prophet_myprintf_stack_record_size - 4

#define prophet_va_arg(start, type, value) \
	{ \
		int size = sizeof(type); \
		static int argorder = 1; \
		std::string stype(#type); \
		if(stype.find("char") != -1) \
		{ \
			if(argorder <= 3) \
			{ \
				start -= 4; \
				value = (type)ReadLocalReg(argorder++ + 4); \
			} else { \
				uint8 byte; \
				start -= 1; \
				MMLoad(start, byte); \
				value = (type)byte; \
			} \
		} else if(stype.find("short") != -1) { \
			if(argorder <= 3) \
			{ \
				start -= 4; \
				value = (type)ReadLocalReg(argorder++ + 4); \
			} else { \
				uint16 dbyte; \
				start -= 2; \
				while(start % 2 != 0) start--; \
				MMLoad(start, dbyte); \
				value = (type)dbyte; \
			} \
		} else if(stype.find("int") != -1 || stype.fine("long") != -1) { \
			if(argorder <= 3) \
			{ \
				start -= 4; \
				value = (type)ReadLocalReg(argorder++ + 4); \
			} else { \
				uint32 qbyte; \
				start -= 4; \
				while(start % 4 != 0) start--; \
				MMLoad(start, qbyte); \
				value = (type)qbyte; \
			} \
		} else if(stype.find("float") != -1 || stype.find("double") != -1) { \
			if(argorder <= 3) \
			{ \
				start -= 8; \
				if(argorder % 2 != 0) argorder++; \
				value = (type)ReadLocalReg(argorder + 4); \
				argorder += 2; \
			} else { \
				uint32 byte4; \
				MMLoad(start, byte4); \
				value = (type)byte4; \
			} \
		} else if(stype.find("double") != -1) { \
			start -= 8; \
			while(start % 8 != 0) start--; \
			if() \
		} \
	}while(0) 
*/
//heap interface for application
#define MALLOC_SYSCALL	11
#define FREE_SYSCALL	12
#define CLOSE_VERIFY	13
#define	OPEN_VERIFY	14

#endif