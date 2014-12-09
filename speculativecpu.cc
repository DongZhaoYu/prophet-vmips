/* Main driver program for Prophet.
   Copyright2008 DongZhaoyu GaoBing.

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

#include "speculativecpu.h"
#include "speculativelogic.h"
#include <assert.h>
#include <limits.h>
#include "prophetlog.h"
#include "predefine.h"
#include "excnames.h"
#include "cpzero.h"
#include "mapper.h"
#include "vmips.h"
#include "stub-dis.h"
#include <stdio.h>
#include <signal.h>
#include <fstream>
#include "prophetconsole.h"
#include "prophetsyscall.h"

#define MAX_INT (INT_MAX - 2)
#define MY_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX_VERSION MAX_INT

#define obstack_chunk_free free
#define obstack_chunk_alloc malloc

static const int Sreg_zero = 0;  /* always zero */
static const int Sreg_sp = 29;   /* stack pointer */
static const int Sreg_ra = 31;   /* return address */

typedef void (SpeculativeCPU::*sp_fptr)(uint32, uint32);

#define faddr(instr) &SpeculativeCPU::instr##_emulate
#define sp_fcaller(fname, p1, p2) (this->*fname)(p1, p2)
#define CONSOLE_PATH SYSCONFDIR"/prophet_console.txt"

bool SpeculativeCPU::m_StackLocked = false;
uint32 SpeculativeCPU::m_LockID = 0xffffffff;
uint32 SpeculativeCPU::m_CPUNum = 0;
const uint32 SpeculativeCPU::m_MemCacheSize = 517;

SpeculativeCPU::SpeculativeCPU(Mapper &m, IntCtrl &i) : CPU(m, i), fpu(new FPU(this, &i))
{
	InitCache();
	m_State = IDLE;
	m_ID = m_CPUNum++;
	m_SwapMark = false;
	m_ExcMark = false;
	m_RegDumpMark = false;
	m_NeedVerify = true;
	m_PPC = 0;
}

SpeculativeCPU::~SpeculativeCPU()
{
	FreeCache();
	delete fpu;
	fpu = NULL;
}

void SpeculativeCPU::InitCache()	//we initialize the cache but we donot alloc memory for it
{
	//init register cache
	int i;
	for(i = 0; i < CPU_REG_NUMBER; i++)
	{
		m_RegCache[i].m_isValide = -1;
		m_RegCache[i].m_isWritten = false;
		m_RegCache[i].m_isRead = false;
		m_RegCache[i].m_Value = 0;
	}

	for(i = 0; i < FPU_REG_NUMBER; i++)
	{
		m_FregCache[i].m_Fvalide = -1;
		m_FregCache[i].m_Fwritten = false;
		m_FregCache[i].m_Fread = false;
		m_FregCache[i].m_Fvalue = 0;
	}

	//init memory L1Cache
	m_L1Cache.m_Size = m_MemCacheSize;
	m_L1Cache.m_MemoryInited = false;
	m_L1Cache.m_HaveInvalideEntry = false;

#ifdef CACH_STATISTIC
	m_Insertions = 0;
	m_Lookups = 0;
	m_Deletions = 0;
	m_Compares = 0;
#endif
}

//allocate memory for cache, if the memory has been allocated, this method will realloc and initialize it
void SpeculativeCPU::AllocCache()
{
	int cachesize = m_L1Cache.m_Size * sizeof(MemCacheEntry*), i = 0;

	FreeCache();
	obstack_init(&m_L1Cache.m_Memory);
	m_L1Cache.m_MemoryInited = true;

	m_L1Cache.m_Cache = (MemCacheEntry**)obstack_alloc(&m_L1Cache.m_Memory, cachesize);
	for(i = 0; i < m_L1Cache.m_Size; i++)
		m_L1Cache.m_Cache[i] = NULL;
}

void SpeculativeCPU::FreeCache()
{
	if(m_L1Cache.m_MemoryInited)
		obstack_free(&m_L1Cache.m_Memory, m_L1Cache.m_Cache);
	m_L1Cache.m_MemoryInited = false;
	m_L1Cache.m_Cache = NULL;
}

void SpeculativeCPU::InitCacheEntry(MemCacheEntry* entry)
{
	Valide0(entry) = Valide1(entry) = Valide2(entry) = Valide3(entry) = false;

	Version(entry) = -1;
	IsOld(entry) = true;

	RemoteLoaded0(entry) = RemoteLoaded1(entry) = RemoteLoaded2(entry) = RemoteLoaded3(entry) = false;
	Modified0(entry) = Modified1(entry) = Modified2(entry) = Modified3(entry) = false;

	Address(entry) = NULL;
	Value(entry) = 0;
}

void SpeculativeCPU::dump_regs(FILE *f)
{
	pinfo<<(*this)<<"hi = "<<std::hex<<hi<<", lo = "<<std::hex<<lo<<pendl;
	pinfo<<(*this)<<"delay state = "<<strdelaystate(delay_state)<<", delay pc = "<<std::hex<<delay_pc<<", next epc = "<<next_epc<<pendl;
	for(uint32 i = 0; i < CPU_REG_NUMBER; i++)
	{
		pinfo<<(*this)<<"register "<<i<<" is "<<std::hex<<reg[i]<<pendl;
	}
/*
	for(uint32 i = 0; i < CPU_REG_NUMBER; i++)
	{
		pinfo<<(*this)<<"register cache "<<i<<" is "<<m_RegCache[i].m_Value<<",Valide: "<<m_RegCache[i].m_isValide<<
		", Read: "<<m_RegCache[i].m_isRead<<", Write: "<<m_RegCache[i].m_isWritten<<pendl;
	}
*/
}

void SpeculativeCPU::InvalideButPslice()
{
	MemCacheEntry *head = NULL;

	//here we invalide all the cache entry with version greater than 0
	for(int i = 0; i < m_L1Cache.m_Size; i++)
	{
		if(m_L1Cache.m_Cache[i] == NULL)
			continue;
		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			if(Version(head) != 0)
			{
				Valide0(head) = Valide1(head) = Valide2(head) = Valide3(head) = false;
				Address(head) = 0;
				if(!m_L1Cache.m_HaveInvalideEntry)
					m_L1Cache.m_HaveInvalideEntry = true;
			}
		}
	}
}

uint32 SpeculativeCPU::code(uint32 instr)
{
	return (instr >> 6) & 0x0fffff;
}

uint32 SpeculativeCPU::code11(uint32 instr)
{
	return (instr >> 11) & 0x07fff;
}

SpeculativeCPU::MemCacheEntry* SpeculativeCPU::FindInvalide()
{
	MemCacheEntry *head = NULL, *prehead = NULL;

	if(!m_L1Cache.m_HaveInvalideEntry)
		return NULL;

	for(int i = 0; i < m_L1Cache.m_Size; i++)
	{
		if(m_L1Cache.m_Cache[i] == NULL)
			continue;
		prehead = NULL;
		for(head = m_L1Cache.m_Cache[i]; head; prehead = head, head = head->m_Next)
		{
			if(!IsValideSome(head))
			{
				//ok, delete it from hash link
				if(prehead == NULL)
					m_L1Cache.m_Cache[i] = head->m_Next;
				else
					prehead->m_Next = head->m_Next;
				return head;
			}
		}
	}

	m_L1Cache.m_HaveInvalideEntry = false;
	return NULL;
}

SpeculativeCPU::MemCacheEntry* SpeculativeCPU::NewCacheEntry()
{
	MemCacheEntry *entry = FindInvalide();
	if(entry == NULL)
		entry = (MemCacheEntry*)obstack_alloc(&(m_L1Cache.m_Memory), sizeof(MemCacheEntry));
	if(entry == NULL)
	{
		perror<<"can not alloc a new MemCacheEntry!"<<pendl;
	}else{
		InitCacheEntry(entry);
	}
	return entry;
}

uint32 SpeculativeCPU::HashMap(uint32 address)
{
	uint32 ra = address >> 2;
	return ra % m_L1Cache.m_Size;
}

void SpeculativeCPU::InitSubthread(SpeculativeCPU *subthread)
{
	subthread->SetRegularReg(reg);			//copy the register value to subthread
	subthread->SetFloatreg(fpu->freg);
	subthread->SetPC(CalSubAddress(instr));		//set the pc value of subthread
	//subthread->EnterPslice();			//make subthread to enter the pslice excution state???????????????????
	subthread->BeginSpRun();			//?????????????????????????????????????

	subthread->SetThreadVersion(m_ThreadVersion);	//pass current version to subthread and
	m_ThreadVersion++;				//current version plus one
}

void SpeculativeCPU::SetRegularReg(uint32 *preg)
{
	int i;
	for(i = 0; i < CPU_REG_NUMBER; i++)
	{
		reg[i] = preg[i];
	}
}

void SpeculativeCPU::SetFloatreg(uint32 *fpreg)
{
	int i;
	for(i = 0; i < FPU_REG_NUMBER; i++)
	{
		fpu->freg[i] = fpreg[i];
	}
}

void SpeculativeCPU::SetPC(uint32 ipc)
{
	pc = ipc;
}

void SpeculativeCPU::OnSpawn(SpeculativeCPU *parent)		//handle the spawn message
{
	//here we should initialize something that are speciale about the subthread
	//first we should do is to allocate the L1Cache memory
	for(int i = 0; i < CPU_REG_NUMBER; i++)
	{
		m_RegCache[i].m_isValide = -1;
		m_RegCache[i].m_isRead = false;
		m_RegCache[i].m_isWritten = false;
		m_RegCache[i].m_Value = 0;
	}

	for(int i = 0; i < FPU_REG_NUMBER; i++)
	{
		m_FregCache[i].m_Fvalide = -1;
		m_FregCache[i].m_Fwritten = false;
		m_FregCache[i].m_Fread = false;
		m_FregCache[i].m_Fvalue = 0;
	}

	AllocCache();

	//invalide the start p-slice pc
	//m_ValideStartPC = false;

	//initialize delay state
	delay_state = NORMAL;
	delay_pc = NULL;
	next_epc = NULL;
	last_epc = NULL;
	hi = lo = 0;
	exception_pending = false;

	//second initilize the machine state
	m_State = IDLE;

	//clere exception
	m_ExcMark = false;

	//set verify mark
	m_NeedVerify = true;

	//store the parent thread
	m_ParentThread = parent;

	//Accept Initialization;
	parent->InitSubthread(this);

	//here swap the subthread itself to speculative list
	SpeculativeLogic::GetInstance()->SwapToSpList(parent, this);
}

void SpeculativeCPU::EnterPslice()
{
	//transfer to pre_computation state
	m_State = PRE_COMPUTATION;
}

void SpeculativeCPU::BeginSpRun()
{
	//transfer to sp_execution
	m_State = SP_EXECUTION;
}

//Speculative loading algorithm template, T = uint32 or uint16 or uint8
template <typename T>
bool SpeculativeCPU::SpLoadTemplate(uint32 address, T &value, bool checklocal, MemCacheEntry** pentry)
{
	if((checklocal) && LocalLoad(address, value))
		return true;

	if(!checklocal && !pentry)
		pwarn<<"this may cause redundant data entry!"<<pendl;

	SpeculativeCPU *parent = NULL;
	int v = MAX_VERSION;

	//load the data from its parent thread according its state
	if(m_State == PRE_COMPUTATION)
	{	//in pre_computation state load data with version
		for(parent = m_ParentThread; parent; parent = SpeculativeLogic::GetInstance()->GetParent(parent))
		{
			v = (parent == m_ParentThread) ? m_ThreadVersion : MAX_VERSION;
			if(parent->LocalLoad(address, value, v))
				break;
		}
	}else if(m_State == SP_EXECUTION){	//in sp_execution just load the newest version
		for(parent = SpeculativeLogic::GetInstance()->GetParent(this); parent;
			parent = SpeculativeLogic::GetInstance()->GetParent(parent))
		{
			if(parent->LocalLoad(address, value, v))	//here v = MAX_VERSION
				break;
		}
	}else if(m_State == STABLE_EXECUTION){
		parent = NULL;
	}else if(m_State == WAIT){
		pwarn << (*this) << "load in wait state!" << pendl;
	}

	//load the data from memory
	if(!parent)
		MMLoad(address, value);

	//save the data to local cache, remeber to swap the world
	SpSave(address, value, pentry);

	return true;
}

bool SpeculativeCPU::SpStore(uint32 address, uint32 value)
{
	pinfo<<(*this)<<"SP STORE WORD: address = "<<std::hex<<address<<", value = "<<value<<pendl;
	return SpStore(address, value, 32);
}

bool SpeculativeCPU::SpStore(uint32 address, uint16 value)
{
	pinfo<<(*this)<<"SP STORE HWORD: address = "<<std::hex<<address<<", value = "<<value<<pendl;
	return SpStore(address, value, 16);
}

bool SpeculativeCPU::SpStore(uint32 address, uint8 value)
{
	pinfo<<(*this)<<"SP STORE BYTE: address = "<<std::hex<<address<<", value = "<<value<<pendl;
	return SpStore(address, value, 8);
}

bool SpeculativeCPU::SpStore(uint32 address, uint32 value, uint32 width)	//width = 32, 16 or 8
{
	assert(width == 32 || width == 16 || width == 8);

	MemCacheEntry *head = NULL;
	uint32 ra = Round4(address);
	uint32 i = ra % m_L1Cache.m_Size;
	SpeculativeCPU *sub = NULL;
	bool needinsertion = false;

	//find where to store
	if(m_State == PRE_COMPUTATION)
	{
		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			if(Address(head) == ra && Version(head) == 0)	//we do not test valide bit here
				break;
		}
	}else if(m_State == SP_EXECUTION || m_State == STABLE_EXECUTION){
		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			if(Address(head) == ra && !IsOld(head))
				break;
		}

		//test if we are generating a new version
		if(head && m_ThreadVersion > Version(head))
		{
			IsOld(head) = true;			//make current version old
			MemCacheEntry *temp = NewCacheEntry();	//generate new entry
			*temp = *head;				//copy the old value to new version
			head = temp;
			needinsertion = true;
		}

		//broadcast the store operation to subthreads
		sub = SpeculativeLogic::GetInstance()->GetSubthread(this);
		if(sub && m_NeedVerify)
			sub->OnStore(address, value, width);
	}else{
		perror<<(*this)<<"store in error state!"<<pendl;
	}

	//store the value
	if(head == NULL)
	{
		head = NewCacheEntry();
		needinsertion = true;
	}

	FillCacheEntry(head, address, value, width, STORE);
	if(needinsertion)
	{
		head->m_Next = m_L1Cache.m_Cache[i];
		m_L1Cache.m_Cache[i] = head;
	}

	return true;
}

/*! \brief
	save value to local cache, bitnum = 32, 16 or 8
	\p pentry is the exiting entry where the new data will be saved, if current version is
	greater than the version of \p pentry , a new cache line will be created by copying 
	\p pentry 's contents. This is because version field and old field in cache line is
	not extended. When SpSave returns, \p pentry points the newly created cache line.
*/
void SpeculativeCPU::SpSave(uint32 address, uint32 value, uint32 bitnum, MemCacheEntry** pentry)
{
	assert(address >= 0);
	assert(bitnum == 32 || bitnum == 16 || bitnum == 8);
	if(m_State == PRE_COMPUTATION)			//we do not need to memorize the data read during pre_computation
		return;

	uint32 ra = Round4(address);
	uint32 i = ra % m_L1Cache.m_Size;
	MemCacheEntry *entry = NULL;
	bool needinsertion = false;

	//no entry for the specific address or there exists an old version
	if(pentry == NULL || *pentry == NULL || Version(*pentry) < m_ThreadVersion)
	{
		entry = NewCacheEntry();
		if(pentry && *pentry)			//copy content of the old version
		{
			*entry = **pentry;
			*pentry = entry;
		}
		needinsertion = true;
	}else{
		entry = *pentry;
	}

	//fill the content of the entry
	FillCacheEntry(entry, address, value, bitnum, SAVE);

	if(needinsertion)
	{
		entry->m_Next = m_L1Cache.m_Cache[i];
		m_L1Cache.m_Cache[i] = entry;
	}
}

void SpeculativeCPU::SpSave(uint32 address, uint32 value, MemCacheEntry** pentry)
{
	uint32 v = TrySwapWord(value);
	SpSave(address, v, 32, pentry);
}

void SpeculativeCPU::SpSave(uint32 address, uint16 value, MemCacheEntry** pentry)
{
	uint16 v = TrySwapHalfWord(value);
	SpSave(address, v, 16, pentry);
}

void SpeculativeCPU::SpSave(uint32 address, uint8 value, MemCacheEntry** pentry)
{
	SpSave(address, value, 8, pentry);
}

#define SAVE_WORD(e, a, v) \
	do{ \
		Valide0(e) = Valide1(e) = Valide2(e) = Valide3(e) = true; \
		RemoteLoaded0(e) = RemoteLoaded1(e) = RemoteLoaded2(e) =  RemoteLoaded3(e) = true; \
		Modified0(e) = Modified1(e) = Modified2(e) = Modified3(e) = false; \
		IsOld(e) = m_State == PRE_COMPUTATION;\
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		Value(e) = v; \
	}while(0)

#define SAVE_LOWHALFWORD(e, a, v) \
	do{ \
		Valide0(e) = Valide1(e) = true; \
		RemoteLoaded0(e) = RemoteLoaded1(e) = true; \
		Modified0(e) = Modified1(e) = false; \
		IsOld(e) = m_State == PRE_COMPUTATION; \
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		LowhalfOfWord(Value(e)) = uint16(v); \
	}while(0)

#define SAVE_HIGHHALFWORD(e, a, v) \
	do{ \
		Valide2(e) = Valide3(e) = true; \
		RemoteLoaded2(e) = RemoteLoaded3(e) = true; \
		Modified2(e) = Modified3(e) = false; \
		IsOld(e) = m_State == PRE_COMPUTATION; \
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		HighhalfOfWord(Value(e)) = uint16(v); \
	}while(0)

#define SAVE_BYTE(e, a, v, offset) \
	do{ \
		Valide##offset(e) = true; \
		RemoteLoaded##offset(e) = true; \
		Modified##offset(e) = false; \
		IsOld(e) = m_State == PRE_COMPUTATION; \
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		Byte##offset##OfWord(Value(e)) = uint8(v); \
	}while(0)

#define STORE_WORD(e, a, v) \
	do{ \
		Valide0(e) = Valide1(e) = Valide2(e) = Valide3(e) = true; \
		Modified0(e) = Modified1(e) = Modified2(e) = Modified3(e) = true; \
		IsOld(e) = m_State == PRE_COMPUTATION; \
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		Value(e) = v; \
	}while(0)

#define STORE_LOWHALFWORD(e, a, v) \
	do{ \
		Valide0(e) = Valide1(e) = true; \
		Modified0(e) = Modified1(e) = true; \
		IsOld(e) = m_State == PRE_COMPUTATION; \
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		LowhalfOfWord(Value(e)) = uint16(v); \
	}while(0)

#define STORE_HIGHHALFWORD(e, a, v) \
	do{ \
		Valide2(e) = Valide3(e) = true; \
		Modified2(e) = Modified3(e) = true; \
		IsOld(e) = m_State == PRE_COMPUTATION; \
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		HighhalfOfWord(Value(e)) = uint16(v); \
	}while(0)

#define STORE_BYTE(e, a, v, offset) \
	do{ \
		Valide##offset(e) = true; \
		Modified##offset(e) = true; \
		IsOld(e) = m_State == PRE_COMPUTATION; \
		Version(e) = m_State == PRE_COMPUTATION ? 0 : m_ThreadVersion; \
		Address(e) = a; \
		Byte##offset##OfWord(Value(e)) = uint8(v); \
	}while(0)

void SpeculativeCPU::FillCacheEntry(MemCacheEntry *entry, uint32 address, uint32 value, uint32 bitnum, CacheEntryOperationType type)
{
	assert(address >= 0);
	assert(bitnum == 32 || bitnum == 16 || bitnum == 8);

	uint32 ra = Round4(address);
	uint32 offset = address - ra;
	MemCacheEntry *e = entry;
	assert(e != NULL);

	if(bitnum == 32)
	{
		if(offset != 0)
			perror<<(*this)<<"offset must be 0!"<<pendl;

		switch(type)
		{
		case SAVE:
			SAVE_WORD(e, ra, value);
			break;
		case STORE:
			STORE_WORD(e, ra, value);
			break;
		default: 
			perror<<(*this)<<"error type!"<<pendl;
			break;
		}
	}else if(bitnum == 16){
		if(offset == 0){
			switch(type)
			{
			case SAVE:
				SAVE_LOWHALFWORD(e, ra, value);
				break;
			case STORE:
				STORE_LOWHALFWORD(e, ra, value);
				break;
			default:
				perror<<(*this)<<"error type!"<<pendl;
				break;
			}
		}else if(offset == 2){
			switch(type)
			{
			case SAVE:
				SAVE_HIGHHALFWORD(e, ra, value);
				break;
			case STORE:
				STORE_HIGHHALFWORD(e, ra, value);
				break;
			default:
				perror<<(*this)<<"error type!"<<pendl;
				break;
			}
		}else{
		}
	}else if(bitnum == 8){
		if(offset == 0){
			switch(type)
			{
			case SAVE:
				SAVE_BYTE(e, ra, value, 0);
				break;
			case STORE:
				STORE_BYTE(e, ra, value, 0);
				break;
			default:
				perror<<(*this)<<"error type!"<<pendl;
				break;
			}
		}else if(offset == 1){
			switch(type)
			{
			case SAVE:
				SAVE_BYTE(e, ra, value, 1);
				break;
			case STORE:
				STORE_BYTE(e, ra, value, 1);
				break;
			default:
				perror<<(*this)<<"error type!"<<pendl;
				break;
			}
		}else if(offset == 2){
			switch(type)
			{
			case SAVE:
				SAVE_BYTE(e, ra, value, 2);
				break;
			case STORE:
				STORE_BYTE(e, ra, value, 2);
				break;
			default:
				perror<<(*this)<<"error type!"<<pendl;
				break;
			}
		}else if(offset == 3){
			switch(type)
			{
			case SAVE:
				SAVE_BYTE(e, ra, value, 3);
				break;
			case STORE:
				STORE_BYTE(e, ra, value, 3);
				break;
			default:
				perror<<(*this)<<"error type!"<<pendl;
				break;
			}
		}
	}else{
		perror<<(*this)<<"error width!"<<pendl;
	}
}

//implementation of localload, this version is called by the thread itself, bitnum = 32, 16 or 8
bool SpeculativeCPU::LocalLoadCore(uint32 address, uint32 &value, uint32 bitnum)
{
	assert(address >= 0);
	assert(bitnum == 32 || bitnum == 16 || bitnum == 8);

	uint32 ra = Round4(address);
	uint32 i = ra % m_L1Cache.m_Size;
	MemCacheEntry *head = NULL; // *newhead = NULL;
	uint32 offset = address - ra;

	//find the cache entry according its state, we do not test the valide field of the entry here
	if(m_State == PRE_COMPUTATION)
	{
		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			if(Address(head) == ra && Version(head) == 0)
				break;
		}
	}else if(m_State == SP_EXECUTION || m_State == STABLE_EXECUTION){
		MemCacheEntry *temp = NULL;
		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			if(Address(head) == ra && !IsOld(head))		//ok find the newest data
			{
				break;
			}else if(Address(head) == ra && Version(head) == 0){	//stable thread's version is 1
				//find the data in pslice, keep it
				temp = head;
			}
		}

		if(head == NULL && m_State == SP_EXECUTION)
			head = temp;
	}else{
		perror << (*this) << "load in error state!" << pendl;
	}

	//load data from the entry
	if(head)
	{
		return ReadDataFromEntry(head, bitnum, offset, value);
	}

	return false;
}

/*! \brief
	Implementation of LocalLoad, this is called by other thread, version can equal or LESS than
	MAX_VERSION, MAX_VERSION means load the newest version of the data, LESS than MAX_VERSION 
	means that load the data with newest version but LESS THAN \p version. \p bitnum = 32
	,16 or 8
*/
bool SpeculativeCPU::LocalLoadCore(uint32 address, uint32 &value, uint32 bitnum, int version)
{
	assert(address >= 0);
	assert(bitnum == 32 || bitnum == 16 || bitnum == 8);

	uint32 ra = Round4(address);
	uint32 offset = address - ra;
	uint32 i = ra % m_L1Cache.m_Size;
	MemCacheEntry *head = NULL, *presult = NULL;
	int minv = version;

	//the thread itself is in pre_computation state so all the data should not be read by its sub threads	
	if(m_State == PRE_COMPUTATION)
		return false;

	if(minv == MAX_VERSION)
	{
		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			if(Address(head) == ra && !IsOld(head))
			{
				presult = head;
				break;
			}
		}
	}else{
		head = m_L1Cache.m_Cache[i];
		while(head && (Address(head) != ra || Version(head) > minv))
			head = head->m_Next;
		if(!head)
			return false;
		presult = head;
		for(head = head->m_Next; head; head = head->m_Next)
		{
			if(Address(head) == ra && Version(head) <= minv && 
				Version(head) > Version(presult))
				presult = head;
		}
	}

	//if the data is generated during precomputation, subthread can not read it
	if(!presult || Version(presult) == 0)
		return false;

	//read data according its valide fields
	return ReadDataFromEntry(presult, bitnum, offset, value);
}

/*! \brief
	Load a byte from a specific cache entry
	entry: a pointer to a specific cache entry
	holder: a UINT8 variable to contain the byte value
	byteoffset: a number indicating the offset of the byte in the entry
*/
#define LoadByte(entry, offset, holder) \
	do{ \
		if(IsValide##offset(entry)) \
		{ \
			uint32 data = Value(entry); \
			holder = Byte##offset##OfWord(data); \
		}else{ \
			SpLoad(Address(entry) + offset, holder, false, &entry); \
		} \
	}while(0)

//perform the real data load operation
bool SpeculativeCPU::ReadDataFromEntry(MemCacheEntry *entry, uint32 width, uint32 offset, uint32 &value)
{
	MemCacheEntry *head = entry;
	bool error = false;
	if(!head)
		return false;

	uint8 bytes[BYTE_PER_WORD];
	uint32 data = Value(head);

	if(width == 32)
	{
		switch(offset)
		{
		case 0:
			if(IsValideWord(head))
			{				//OK! the whole word is valide, just read it
				value = data;
			}else{
				LoadByte(head, 0, bytes[0]);
				LoadByte(head, 1, bytes[1]);
				LoadByte(head, 2, bytes[2]);
				LoadByte(head, 3, bytes[3]);
				value = Word(bytes);
			}
			break;
		default:
			error = true;
			break;
		}
	}else if(width == 16){
		switch(offset)
		{
		case 0:
			if(IsValideLowhalfWord(head))
			{
				value = LowhalfOfWord(data);
			}else{
				LoadByte(head, 0, bytes[0]);
				LoadByte(head, 1, bytes[1]);
				value = HalfWord(bytes);
			}
			break;
		case 2:
			if(IsValideHighhalfWord(head))
			{
				value = HighhalfOfWord(data);
			}else{
				LoadByte(head, 2, bytes[0]);
				LoadByte(head, 3, bytes[1]);
				value = HalfWord(bytes);
			}
			break;
		default:
			error = true;
			break;
		}
	}else if(width == 8){
		switch(offset)
		{
		case 0:		LoadByte(head, 0, bytes[0]); 	break;
		case 1:		LoadByte(head, 1, bytes[0]); 	break;
		case 2:		LoadByte(head, 2, bytes[0]);	break;
		case 3:		LoadByte(head, 3, bytes[0]);	break;
		default:	error = true;			break;			
		}
		value = bytes[0];
	}else{
		perror << (*this) << "wrong width(" << width << ")" << pendl;
	}
	if(error)
		perror << (*this) << "wrong offset(" << offset << ") with width(" << width << ")" << pendl;
	return true;
}

uint32 SpeculativeCPU::CalSubAddress(uint32 instr)
{
	return (pc & 0xf0000000) | (jumptarg(instr) << 2);
}

//change word between big-endian and little-endian
uint32 SpeculativeCPU::TrySwapWord(uint32 w)
{
	if(m_SwapMark)	//need swap
		return SwapWord(w);
	return w;
}

//change half word between big-endian and little-endian
uint16 SpeculativeCPU::TrySwapHalfWord(uint16 hw)
{
	if(m_SwapMark)
		return SwapHalfWord(hw);
	return hw;
}

//helper macro for SpLoad function
#define SpLoadFunction(valuetype) \
	bool SpeculativeCPU::SpLoad(uint32 address, valuetype &value, bool checklocal, MemCacheEntry** pentry) \
	{ \
		return SpLoadTemplate(address, value, checklocal, pentry); \
	}

//helper macro for LocalLoad functiaddressons
#define LocalLoadCaller32(address, value, version...) \
	uint32 ret; \
	if(LocalLoadCore(address, ret, 32, ##version)) \
	{ \
		value = TrySwapWord(ret); \
		return true; \
	} \
	return false

#define LocalLoadCaller16(address, value, version...) \
	uint32 v; \
	uint16 ret; \
	if(LocalLoadCore(address, v, 16, ##version)) \
	{ \
		ret = uint16(v); \
		value = TrySwapHalfWord(ret); \
		return true; \
	} \
	return false

#define LocalLoadCaller8(address, value, version...) \
	uint32 v; \
	if(LocalLoadCore(address, v, 8, ##version)) \
	{ \
		value = uint8(v); \
		return true; \
	} \
	return false

#define LocalLoadFunction(loadtype, valuetype) \
	bool SpeculativeCPU::LocalLoad(uint32 address, valuetype &value) \
	{ \
		LocalLoadCaller##loadtype(address, value); \
	}

#define LocalLoadVFunction(loadtype, valuetype) \
	bool SpeculativeCPU::LocalLoad(uint32 address, valuetype &value, int version) \
	{ \
		LocalLoadCaller##loadtype(address, value, version); \
	}

//define SpLoad functions
SpLoadFunction(uint32)
SpLoadFunction(uint16)
SpLoadFunction(uint8)

//define LocalLoad functions
LocalLoadFunction(32, uint32)
LocalLoadFunction(16, uint16)
LocalLoadFunction(8, uint8)

//define LocalLoad functions with version
LocalLoadVFunction(32, uint32)
LocalLoadVFunction(16, uint16)
LocalLoadVFunction(8, uint8)

bool SpeculativeCPU::MMLoad(uint32 address, uint32 &value)
{
	uint32 physaddress;
	bool cacheable;

	if(address % 4 != 0)
	{
		pwarn<<"address of word should be at the bounder of 4!"<<pendl;
		exception(AdEL,DATALOAD);
		return false;
	}

	SpeculativeCPU* handler = SpeculativeLogic::GetInstance()->GetRunner();
	physaddress = cpzero->address_trans(address, DATALOAD, &cacheable, handler);
	if(physaddress == 0xffffffff)
		return false;
	value = mem->fetch_word(physaddress, DATALOAD, cacheable, handler);

	pinfo<<(*this)<<"MM LOAD WORD: address = "<<std::hex<<address<<", value = "<<value<<", pc = "<<pc<<pendl;
	return true;
}

bool SpeculativeCPU::MMLoad(uint32 address, uint16 &value)
{
	uint32 physaddress;
	bool cacheable;

	if(address % 2 != 0)
	{
		pwarn<<"address of short should be at the bounder of 2!"<<pendl;
		exception(AdEL,DATALOAD);
		return false;
	}

	SpeculativeCPU* handler = SpeculativeLogic::GetInstance()->GetRunner();
	physaddress = cpzero->address_trans(address, DATALOAD, &cacheable, handler);
	if(physaddress == 0xffffffff)
		return false;
	value = mem->fetch_halfword(physaddress, cacheable, handler);

	pinfo<<(*this)<<"MM LOAD HWORD: address = "<<std::hex<<address<<", value = "<<value<<", pc = "<<pc<<pendl;
	return true;
}

bool SpeculativeCPU::MMLoad(uint32 address, uint8 &value)
{
	uint32 physaddress;
	bool cacheable;

	SpeculativeCPU* handler = SpeculativeLogic::GetInstance()->GetRunner();
	physaddress = cpzero->address_trans(address, DATALOAD, &cacheable, handler);
	if(physaddress == 0xffffffff)
		return false;
	value = mem->fetch_byte(physaddress, cacheable, handler);

	pinfo<<(*this)<<"MM LOAD BYTE: address = "<<std::hex<<address<<", value = "<<value<<", pc = "<<pc<<pendl;
	return true;
}

bool SpeculativeCPU::MMStore(uint32 address, uint32 value)
{
	uint32 physaddress;
	bool cacheable;

	if (address % 4 != 0) 
	{
		pwarn<<"address of word should be at the bounder of 4!"<<pendl;
		exception(AdES,DATASTORE);
		return false;
	}
	physaddress = cpzero->address_trans(address, DATASTORE, &cacheable, this);
	if(physaddress == 0xffffffff)
		return false;
	mem->store_word(physaddress, value, cacheable, this);

	//pinfo<<(*this)<<"MM STORE WORD: address = "<<std::hex<<address<<", value = "<<value<<", pc = "<<pc<<pendl;
	return true;
}

bool SpeculativeCPU::MMStore(uint32 address, uint16 value)
{
	uint32 physaddress;
	bool cacheable;

	if (address % 2 != 0) 
	{
		exception(AdES,DATASTORE);
		return false;
	}
	physaddress = cpzero->address_trans(address, DATASTORE, &cacheable, this);
	if(physaddress == 0xffffffff)
		return false;
	mem->store_halfword(physaddress, value, cacheable, this);

	//pinfo<<(*this)<<"MM STORE HWORD: address = "<<std::hex<<address<<", value = "<<value<<", pc = "<<pc<<pendl;
	return true;	
}

bool SpeculativeCPU::MMStore(uint32 address, uint8 value)
{
	uint32 physaddress;
	bool cacheable;

	physaddress = cpzero->address_trans(address, DATASTORE, &cacheable, this);
	if(physaddress == 0xffffffff)
		return false;
	mem->store_byte(physaddress, value, cacheable, this);

	//pinfo<<(*this)<<"MM STORE BYTE: address = "<<std::hex<<address<<", value = "<<value<<", pc = "<<pc<<pendl;
	return true;
}

bool SpeculativeCPU::RcVerification(SpeculativeCPU *parent)
{
	assert(parent != NULL);

	return parent->Verify(this, m_PPC);
}

bool SpeculativeCPU::Verify(SpeculativeCPU *subthread , uint32 address)
{
	assert(subthread != NULL);

	MemCacheEntry *head = NULL;
	SpeculativeCPU *sub = subthread;

	if((address - pc) != 4)		//pc is current address(address of cqip)
	{
		SignalSquash(CONTROL_VOILATE);			//control dependency occurs, squash the subthread
		return false;
//		sub = SpeculativeLogic::GetInstance()->GetSubthread(sub);
//		if(sub)
//			return Verify(sub, sub->m_PPC);
	} else if(subthread->m_ExcMark){
		SignalSquash(EXCEPTION);			//control dependency occurs, squash the subthread
		return false;
	}else{

		//first we validate the values in the register cache
		for(int j = 0; j < CPU_REG_NUMBER; j++)
		{
//			if(!(sub->m_RegCache[i].m_isValide) || !(sub->m_RegCache[i].m_isRead))
//				continue;

			if(sub->m_RegCache[j].m_isValide == 0 &&	//the entry is valide and read first
				sub->m_RegCache[j].m_isRead && 		//the entry is read
				reg[j] != sub->m_RegCache[j].m_Value)	//the value in the entry is not equal
			{
				SignalSquash(V_REG_VOILATE);
				return false;
			}
		}
		for(int k = 0; k < FPU_REG_NUMBER; k++)
		{
			if(sub->m_FregCache[k].m_Fvalide == 0 &&
				sub->m_FregCache[k].m_Fread &&
				fpu->freg[k] != sub->m_FregCache[k].m_Fvalue)
			{
				SignalSquash(V_REG_VOILATE);
				return false;
			}
		}

		//then we validate the values generated during p-slice
		for(int i = 0; i < m_L1Cache.m_Size; i++)	//traverse the hashtable of subthread 
		{
			if(sub->m_L1Cache.m_Cache[i] == NULL)
				continue;

			for(head = sub->m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				//pre-exclude the entries that need no validation
				if((Version(head) > 0) || !IsModifiedSome(head) || !IsValideSome(head))
					continue;

				if(!VerifyEntry(head))		// validate the entry
				{
					SignalSquash(V_MM_VOILATE);
					return false;
				}
			}
		}
	}
	return true;
}

//check if the value in the entry equals the value in the specific memory address
bool SpeculativeCPU::VerifyEntry(MemCacheEntry *entry)
{
	//make sure the entry is not null and this entry is generated during p-slice
	assert(entry != NULL && Version(entry) == 0 && IsModifiedSome(entry) && IsValideSome(entry));

	uint32 word;
	uint16 halfword;
	uint8 byte;
	MemCacheEntry *pe = entry;
	bool done = true;

	if(IsValideWord(pe) && IsModifiedWord(pe))    					//verify whole word
	{
		done = MMLoad(Address(pe), word);
		return done && (word == Value(pe));
	}else {
		if(IsValideHighhalfWord(pe) && IsModifiedHighhalfWord(pe)){  		//verify high halfword
			done = MMLoad(Address(pe) + 2, halfword);
			done = done && (halfword == HighhalfOfWord(Value(pe)));
		}else if(IsValide2(pe) && IsModified2(pe)){				//verify byte 2
			done = MMLoad(Address(pe) + 2, byte);
			done = done && (byte == Byte2OfWord(Value(pe)));
		}else if(IsValide3(pe) && IsModified3(pe)){				//verify byte 3
			done = MMLoad(Address(pe) + 3, byte);
			done = done && (byte == Byte3OfWord(Value(pe)));
		}

		if(done && IsValideLowhalfWord(pe) && IsModifiedLowhalfWord(pe)){  		//verify low halfword
			done = MMLoad(Address(pe), halfword);
			return done && (halfword == LowhalfOfWord(Value(pe)));
		}else if(done && IsValide0(pe) && IsModified0(pe)){				//verify byte 0
			done = MMLoad(Address(pe), byte);
			return done && (byte == Byte0OfWord(Value(pe)));
		}else if(done && IsValide1(pe) && IsModified1(pe)){				//verify byte 1
			done = MMLoad(Address(pe) + 1, byte);
			return done && (byte == Byte1OfWord(Value(pe)));
		}
	}
	return false;	
}

//commit memory values and register values
void SpeculativeCPU::CommitValue()
{
	CommitMM();
	CommitRegisters();

	DO_STATISTIC("ecommit", SUCCESS, NULL);
}

void SpeculativeCPU::InvalideEntry(MemCacheEntry *entry)
{
	Address(entry) = 0;
	Valide0(entry) = Valide1(entry) = Valide2(entry) = Valide3(entry) = false;
	m_L1Cache.m_HaveInvalideEntry = true;
}

//commit value in L1cache to main memory
void SpeculativeCPU::CommitMM()
{
	MemCacheEntry *head = NULL;

	for(int i = 0; i < m_L1Cache.m_Size; i++)
	{
		if(m_L1Cache.m_Cache[i] == NULL)
			continue;

		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			CommitEntry(head);

			//invalide the entry
			InvalideEntry(head);
		}
	}
}

//commit register values to its immediate subthread
void SpeculativeCPU::CommitRegisters()
{
	SpeculativeCPU *subthread = SpeculativeLogic::GetInstance()->GetSubthread(this);
	if(!subthread)
		return;

	for(int i = 0; i < CPU_REG_NUMBER; i++)
	{
		if(subthread->m_RegCache[i].m_isValide == -1 && !(subthread->m_RegCache[i].m_isRead) &&
		 	!(subthread->m_RegCache[i].m_isWritten))
			{
				subthread->reg[i] = reg[i];	//commit value
				pinfo<<(*this)<<"commit cpu register: "<<i<<", value = "<<std::hex<<reg[i]<<pendl;
			}
	}

	for(int j = 0; j < FPU_REG_NUMBER; j++)
	{
		if(subthread->m_FregCache[j].m_Fvalide == -1 && !(subthread->m_FregCache[j].m_Fread) &&
			!(subthread->m_FregCache[j].m_Fwritten))
			{
				subthread->fpu->freg[j] = fpu->freg[j];
				pinfo<<(*this)<<"commit fpu register: "<<j<<", value = "<<std::hex<<fpu->freg[j]<<pendl;
			}
	}
}

//cmmit value in a specific entry to main memory
void SpeculativeCPU::CommitEntry(MemCacheEntry *pentry)
{
	assert(pentry != NULL);
	MemCacheEntry *entry = pentry;
	uint32 address = Address(pentry);

	//commit the values with newest version and generated by this thread
	if(Version(entry) > 0 && !IsOld(entry) && IsModifiedSome(entry) && IsValideSome(entry))
	{
		if(IsValideWord(entry) && IsModifiedWord(entry))
		{										//commit whole word
			pinfo<<(*this)<<"Commit WORD: address = "<<std::hex<<address<<", value = "<<Value(entry)<<", pc = "<<pc<<pendl;
			MMStore(address, Value(entry));
		}else{
			if(IsValideHighhalfWord(entry) && IsModifiedHighhalfWord(entry)){	//commit high halfword
				pinfo<<(*this)<<"Commit HFWORD: address = "<<std::hex<<address+2<<", value = "<<HighhalfOfWord(Value(entry))<<", pc = "<<pc<<pendl;
				MMStore(address + 2, HighhalfOfWord(Value(entry)));
			}else if(Valide2(entry) && Modified2(entry)) {				//commit byte 2
				pinfo<<(*this)<<"Commit BYTE: address = "<<std::hex<<address+2<<", value = "<<Byte2OfWord(Value(entry))<<", pc = "<<pc<<pendl;
				MMStore(Address(entry) + 2, Byte2OfWord(Value(entry)));
			}else if(Valide3(entry) && Modified3(entry)) {				//commit byte 3
				pinfo<<(*this)<<"Commit BYTE: address = "<<std::hex<<address+3<<", value = "<<Byte3OfWord(Value(entry))<<", pc = "<<pc<<pendl;
				MMStore(Address(entry) + 3, Byte3OfWord(Value(entry)));
			}
 
			if(IsValideLowhalfWord(entry) && IsModifiedLowhalfWord(entry)){		//commit low halfword
				pinfo<<(*this)<<"Commit LFWORD: address = "<<std::hex<<address<<", value = "<<LowhalfOfWord(Value(entry))<<", pc = "<<pc<<pendl;
				MMStore(address, LowhalfOfWord(Value(entry)));
			}else if(Valide0(entry) && Modified0(entry)){				//commit byte 0
				pinfo<<(*this)<<"Commit BYTE: address = "<<std::hex<<address<<", value = "<<Byte0OfWord(Value(entry))<<", pc = "<<pc<<pendl;
				MMStore(Address(entry), Byte0OfWord(Value(entry)));
			}else if(Valide1(entry) && Modified1(entry)){				//commit byte 1
				pinfo<<(*this)<<"Commit BYTE: address = "<<std::hex<<address+1<<", value = "<<Byte1OfWord(Value(entry))<<", pc = "<<pc<<pendl;
				MMStore(Address(entry) + 1, Byte1OfWord(Value(entry)));
			}
		}
	}
}

bool SpeculativeCPU::SpawnSubthread()
{
	return SpeculativeLogic::GetInstance()->NotifySpawn(this) != NULL;
}

void SpeculativeCPU::SaveRegValues()
{
	for(int i = 0; i < CPU_REG_NUMBER; i++)
	{
		m_RegCache[i].m_Value = reg[i];
		m_RegCache[i].m_isRead = 0;
		m_RegCache[i].m_isWritten = 0;
		m_RegCache[i].m_isValide = -1;
	}

	for(int j = 0; j < FPU_REG_NUMBER; j++)
	{
		m_FregCache[j].m_Fvalue = fpu->freg[j];
		m_FregCache[j].m_Fread = 0;
		m_FregCache[j].m_Fwritten = 0;
		m_FregCache[j].m_Fvalide = -1;
	}
}

void SpeculativeCPU::SignalSquash(SQUASH_REASON reason)
{
	SpeculativeLogic::GetInstance()->SquashSubthread(this, reason);
}

//called by OnStore to test if we need to pass the store address to its subthread or not
bool SpeculativeCPU::IsRefreshed(uint32 address, uint32 value, uint32 width)
{
	assert(width == 32 || width == 16 || width == 8);
	uint32 ra = Round4(address);
	uint32 offset = address - ra;
	uint32 i = ra % m_L1Cache.m_Size;
	MemCacheEntry *head = NULL;

	if(width == 32)
	{
		if(offset != 0)
			perror<<(*this)<<"offset must be 0!"<<pendl;

		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{	
			if((Address(head) == ra) && !IsOld(head) &&
				 !IsRemoteLoadedSome(head) && IsModifiedWord(head))
				return true;
		}
		return false;
	}else if(width == 16){
		if(offset == 0)
		{
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && IsValideLowhalfWord(head) && !IsOld(head) &&
				 !IsRemoteLoadedSomeLowhalf(head) && IsModifiedLowhalfWord(head))
				return true;
			}
			return false;
		}else if(offset == 2){
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && IsValideHighhalfWord(head) && !IsOld(head) &&
				 !IsRemoteLoadedSomeHighhalf(head) && IsModifiedHighhalfWord(head))
					return true;
			}
			return false;
		}else{
			perror<<(*this)<<"offset must be 0 or 2!"<<pendl;
		}
	}else{
		if(offset == 0)
		{
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && IsValide0(head) && !IsOld(head) &&
				 !RemoteLoaded0(head) && IsModified0(head))
					return true;
			}
			return false;
		}else if(offset == 1){
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && IsValide1(head) && !IsOld(head) &&
				 !RemoteLoaded1(head) && IsModified1(head))
					return true;
			}
			return false;
		}else if(offset == 2){
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if((Address(head) == ra) && IsValide2(head) && !IsOld(head) &&
				 !RemoteLoaded2(head) && IsModified2(head))
					return true;
			}
			return false;
		}else if(offset == 3){
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if((Address(head) == ra) && IsValide3(head) && !IsOld(head) &&
				 !RemoteLoaded3(head) && IsModified3(head))
					return true;
			}
			return false;
		}else{
			perror<<(*this)<<"offset error!"<<pendl;
		}
	}

	perror<<(*this)<<"width error!"<<pendl;
	return false;	
}

//test if the value with specific address cause voilation in this thread
bool SpeculativeCPU::TestDDV(uint32 address, uint32 value, uint32 width)
{
	assert(width == 32 || width == 16 || width == 8);
	uint32 ra = Round4(address);
	uint32 offset = address - ra;
	uint32 i = ra % m_L1Cache.m_Size;
	MemCacheEntry *head = NULL;

	if(width == 32)
	{						//test the whole word
		if(offset != 0)
			perror<<(*this)<<"offset must be 0!"<<pendl;

		for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
		{
			if(Address(head) == ra && IsValideSome(head) && 
				Version(head) > 0 && IsRemoteLoadedSome(head) &&
				(Value(head) != value))
				return true;
		}
		return false;
	}else if(width == 16){				//test the low halfword
		if(offset == 0)
		{
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && IsValideSomeLowhalf(head) &&
					Version(head) > 0 && IsRemoteLoadedSomeLowhalf(head) &&
					(LowhalfOfWord(Value(head)) != LowhalfOfWord(value)))
					return true;
			}
			return false;
		}else if(offset == 2){			//test the high halfword
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && IsValideSomeHighhalf(head) &&
				 Version(head) > 0 && IsRemoteLoadedSomeHighhalf(head) &&
					(HighhalfOfWord(Value(head)) != HighhalfOfWord(value)))
					return true;
			}
			return false;
		}else{
			perror<<(*this)<<"offset must be 0 or 2!"<<pendl;
		}
	}else{
		if(offset == 0)
		{					//test byte 0
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && Valide0(head) && 
				 Version(head) > 0 && RemoteLoaded0(head) &&
					(Byte0OfWord(Value(head)) != Byte0OfWord(value)))
					return true;
			}
			return false;
		}else if(offset == 1){			//test byte 1
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && Valide1(head) && 
				 Version(head) > 0 && RemoteLoaded1(head) &&
					(Byte1OfWord(Value(head)) != Byte1OfWord(value)))
					return true;
			}
			return false;
		}else if(offset == 2){			//test byte 2
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if(Address(head) == ra && Valide2(head) && 
				 Version(head) > 0 && RemoteLoaded2(head) &&
					(Byte2OfWord(Value(head)) != Byte2OfWord(value)))
					return true;
			}
			return false;
		}else if(offset == 3){			//test byte 3
			for(head = m_L1Cache.m_Cache[i]; head; head = head->m_Next)
			{
				if((Address(head) == ra) && Valide3(head) &&
				 Version(head) > 0 && RemoteLoaded3(head) &&
					(Byte3OfWord(Value(head)) != Byte3OfWord(value)))
					return true;
			}
			return false;
		}else{
			perror<<(*this)<<"error offset!"<<pendl;
		}
	}
	return false;
}

void SpeculativeCPU::OnStore(uint32 address, uint32 value, uint32 width)
{
	assert(width == 32 || width == 16 || width == 8);
	SpeculativeCPU *subthread = NULL;

	if(IsRefreshed(address,value,width))
	{	//have no need to pass the store message to its subthread
		return;
	}else if(TestDDV(address, value, width)){	//data dependence voilation occur
		SignalSquash(DDV);
		Restart();
		return;
	}else{						//pass the store message to its subthread
		subthread = SpeculativeLogic::GetInstance()->GetSubthread(this);
		if(subthread)
			subthread->OnStore(address,value,width);
		return;
	}
}

void SpeculativeCPU::OnSquash(SpeculativeCPU* sender, SQUASH_REASON reason)
{
	FreeCache();
	m_State = IDLE;
	SquashClientData data;

	//free stack
	FreeStack();

	std::string s("receive squash message! reason: ");
	switch(reason)
	{
	case DDV: 		data.m_Reason = "DDV";			break;
	case INSTRUCTION:	data.m_Reason = "INSTRUCTION";		break;
	case V_REG_VOILATE:	data.m_Reason = "V_REG_VOILATE";	break;
	case V_MM_VOILATE:	data.m_Reason = "V_MM_VOILATE";		break;
	case LOW_PRIORITY:	data.m_Reason = "LOW_PRIORITY";		break;
	case CONTROL_VOILATE:	data.m_Reason = "CONTROL_VOILATE";	break;
	case RESET:		data.m_Reason = "RESET";		break;
	case HALT:		data.m_Reason = "HALT";			break;
	case QUIT:		data.m_Reason = "QUIT";			break;
	case EXCEPTION:		data.m_Reason = "EXCEPTION";		break;
	default:		data.m_Reason = "XXX";			break;
	}
	s += data.m_Reason;
	if(sender != NULL)
		pinfo<<(*this)<<s<<" sender: "<<sender->m_ID<<pendl;
	else
		pinfo<<(*this)<<s<<" sender: "<<"UNKNOWN"<<pendl;

	SpeculativeLogic::GetInstance()->SwapToFreeList(this);

	DO_STATISTIC("esquash", SUCCESS, &data);
}

void SpeculativeCPU::HoldCQIP(uint32 instr)			//?????????????????????????????????????
{
	//here we should assert that the instruction that pc points is really a cqip instruction
	//?????????????????????????????????????
	if(opcode(instr) != 18)
		perror << "the instruction here is not CQIP! Something wrong happens!" << pendl;
	pc = pc - 4;
	m_State = WAIT;
}

void SpeculativeCPU::FreeStack()
{
	if(m_StackLocked && m_LockID == m_ID)
		m_StackLocked = false;
}

bool SpeculativeCPU::LockStack()
{
	if(!m_StackLocked)
	{
		m_StackLocked = true;
		m_LockID = m_ID;
		return true;
	}else if(m_LockID == m_ID){
		return true;
	}
	return false;
}

void SpeculativeCPU::OnCQIP(uint32 instr)			//??????????????????????????????????????
{
	if(m_State == STABLE_EXECUTION)
	{
		CommitValue();

		SpeculativeCPU *sub = SpeculativeLogic::GetInstance()->GetSubthread(this);

		//transfor to SUBTHREAD_VERIFICATION state
		m_State = SUBTHREAD_VERIFICATION;

		if(SpeculativeLogic::GetInstance()->VerifySubthread(this))
		{
			SpeculativeLogic::GetInstance()->PassStableToken(this);

			m_State = IDLE;
			FreeStack();
			SpeculativeLogic::GetInstance()->SwapToFreeList(this);
		}else{	//validation failed
			m_State = STABLE_EXECUTION;
			SkipPslice();
		}
	}
	if(m_State == SP_EXECUTION)
		HoldCQIP(instr);
}

void SpeculativeCPU::HoldBREAK(uint32 instr)
{
	if(opcode(instr) != 0 || funct(instr) != 13)
		perror << (*this) << "the instruction here is not break! Something wrong happens!" << pendl;
	pc = pc - 4;
	m_State = WAIT;
}

void SpeculativeCPU::RestoreRegisters()
{
	for(int i = 0; i < CPU_REG_NUMBER; i++)
	{
		reg[i] = m_RegCache[i].m_Value;
		m_RegCache[i].m_isRead = 0;
		m_RegCache[i].m_isWritten = 0;
		m_RegCache[i].m_isValide = -1;
	}

	for(int j = 0; j < FPU_REG_NUMBER; j++)
	{
		fpu->freg[j] = m_FregCache[j].m_Fvalue;
		m_FregCache[j].m_Fread = 0;
		m_FregCache[j].m_Fwritten = 0;
		m_FregCache[j].m_Fvalide = -1;
	}	
}

uint32 SpeculativeCPU::ReadLocalReg(uint32 num)
{
	assert(num >= 0 && num < CPU_REG_NUMBER);
	uint32 regvalue = reg[num];
	
	if(m_State == SP_EXECUTION)
	{
		m_RegCache[num].m_isRead = true;
		if(m_RegCache[num].m_isValide == -1)  // the reg hasn't been written or read
		{
			m_RegCache[num].m_isValide = 0;
		}
	}
	// the thread state is stable or pre_computation
	return regvalue;
}

void SpeculativeCPU::WriteLocalReg(uint32 num, uint32 value)
{
	assert(num >= 0 && num < CPU_REG_NUMBER);

	reg[num] = value;
	if(m_State == SP_EXECUTION)
	{
		m_RegCache[num].m_isWritten = true;
		if(m_RegCache[num].m_isValide == -1)
		{
			m_RegCache[num].m_isValide = 1;
		}
	}
}

uint32 SpeculativeCPU::ReadLocalFreg(uint32 fnum)
{
	assert(fnum >= 0 && fnum < FPU_REG_NUMBER);

	uint32 fregvalue = fpu->freg[fnum];
	if(m_State == SP_EXECUTION)
	{
		m_FregCache[fnum].m_Fread = true;
		if(m_FregCache[fnum].m_Fvalide == -1)
		{
			m_FregCache[fnum].m_Fvalide = 0;
		}
	}

	return fregvalue;
}

void SpeculativeCPU::WriteLocalFreg(uint32 fnum, uint32 value)
{
	assert(fnum >= 0 && fnum < FPU_REG_NUMBER);

	fpu->freg[fnum] = value;
	if(m_State == SP_EXECUTION)
	{
		m_FregCache[fnum].m_Fwritten = true;
		if(m_FregCache[fnum].m_Fvalide == -1)
		{
			m_FregCache[fnum].m_Fvalide = 1;
		}
	}
}

void SpeculativeCPU::OnStableToken(SpeculativeCPU *sender)
{
	if(m_State == PRE_COMPUTATION)
	{
		SkipPslice();
		pc += 4;
	}
	m_State = STABLE_EXECUTION;
}

void SpeculativeCPU::SkipPslice()		//?????????????????????????
{
	while(opcode(instr) != 21)		//find the struction pslice_exit
	{
		pc += 4;
		MMLoad(pc, instr);
	}
}

void SpeculativeCPU::Restart()
{
	if(m_ExcMark)			//if the thread has suffurred an exception, don't restart it any more
		return;

	InvalideButPslice();
	RestoreRegisters();		//we should restore the reg after pslice
	pc = m_PPC;
	SkipPslice();			//the instructions in pslice should be skipped
	pc += 4;
	m_State = SP_EXECUTION;		// now the thread state is sp_execution
	delay_state = NORMAL;
	delay_pc = NULL;
	next_epc = NULL;
	last_epc = NULL;
	hi = lo = 0;
	exception_pending = false;

	m_NeedVerify = true;

	FreeStack();

	pinfo<<(*this)<<"Restarted!"<<pendl;

	DO_STATISTIC("erestart", SUCCESS, NULL);
}

void SpeculativeCPU::exception(uint16 exeCode, int mode, int coprocno)
{
	pinfo<<(*this)<<"exception occur! pc = "<<std::hex<<pc<<pendl;

	if(m_State != STABLE_EXECUTION)
	{
		//exeception occuring in subthread_verification is not treated as real exeception
		if(m_State != SUBTHREAD_VERIFICATION)
		{
			SignalSquash(EXCEPTION);
			m_State = WAIT;
			m_ExcMark = true;
		}
	}else{
		raise(SIGQUIT);
	}
}

std::ostream& operator<<(std::ostream &out, const SpeculativeCPU &cpu)
{
	out<<"SP_CPU "<<cpu.m_ID<<" :";
	return out;
}

void SpeculativeCPU::reset()
{
	//reset the base stuff
	CPU::reset();

	m_State = STABLE_EXECUTION;
	m_ParentThread = NULL;
	m_ThreadVersion = STABLE_VERSION;

	InitCache();
	AllocCache();

	DO_STATISTIC("ebegin", SUCCESS, NULL);
}

void SpeculativeCPU::step()
{
	uint32 real_pc;
	bool cacheable;
	static char out[100];
	static const sp_fptr OpcodeJumpTable[] = {
		faddr(funct),		faddr(regimm),
		faddr(j),		faddr(jal),
		faddr(beq),		faddr(bne),
		faddr(blez),		faddr(bgtz),
		faddr(addi),		faddr(addiu),
		faddr(slti),		faddr(sltiu),
		faddr(andi),		faddr(ori),
		faddr(xori),		faddr(lui),
		faddr(cpzero),		faddr(cpone),
		faddr(cqip),		faddr(squash),
		faddr(pslice_entry),	faddr(pslice_exit),
		faddr(RI),		faddr(RI),
		faddr(RI),		faddr(RI),
		faddr(RI),		faddr(RI),
		faddr(RI),		faddr(RI),
		faddr(RI),		faddr(RI),
		faddr(lb),		faddr(lh),
		faddr(lwl),		faddr(lw),
		faddr(lbu),		faddr(lhu),
		faddr(lwr),		faddr(RI),
		faddr(sb),		faddr(sh),
		faddr(swl),		faddr(sw),
		faddr(spawn),		faddr(RI),
		faddr(swr),		faddr(RI),
		faddr(RI),		faddr(lwc1),
		faddr(lwc2),		faddr(lwc3),
		faddr(RI),		faddr(ldc1),
		faddr(RI),		faddr(RI),
		faddr(RI),		faddr(swc1),
		faddr(swc2),		faddr(swc3),
		faddr(RI),		faddr(sdc1),
		faddr(RI),		faddr(RI),
	};

	pinfo<<(*this)<<"pc = "<<std::hex<<pc<<pendl;
	if(m_State != PRE_COMPUTATION && m_State != STABLE_EXECUTION && m_State != SP_EXECUTION)
	{
		pinfo<<(*this)<<"not in a runnable state!"<<pendl;
		return;
	}

	/* Clear exception_pending flag if it was set by a
	 * prior instruction.
	 */
	exception_pending = false;

	/* decrement Random register */
	cpzero->adjust_random();

	if(opt_tracing && !tracing && pc == opt_tracestartpc)
		start_tracing();

	/* save address of instruction responsible for exceptions which may occur */
	if(delay_state != DELAYSLOT)
		next_epc = pc;

	/* get physical address of next instruction */
	real_pc = cpzero->address_trans(pc, INSTFETCH, &cacheable, this);
	if (exception_pending) 
	{
		if (opt_excmsg) 
			pwarn<<(*this)<<"** PC address translation caused the exception! **"<<pendl;
		goto out;
	}

	/* get next instruction */
	instr = mem->fetch_word(real_pc, INSTFETCH, cacheable, this);
	if(exception_pending)
	{
		if(opt_excmsg) 
			pwarn<<(*this)<<"** Instruction fetch caused the exception! **"<<pendl;
		goto out;
	}

	/* diagnostic output - display disassembly of instr */
	if(opt_instdump)
	{
		sprintf(out, "PC=0x%08x [%08x]\t%08x ", pc, real_pc, instr);
		pinfo<<(*this)<<out<<pendl;
		machine->disasm->disassemble(pc,instr);
	}

	/* first half of trace recording for this instruction */
	if(opt_tracing && tracing)
		write_trace_record_1(pc, instr);

	/* Check for a (hardware or software) interrupt */
	if(cpzero->interrupt_pending())
	{
		exception(Int);
		goto out;
	}

	/* Jump to the appropriate emulation function. */
	sp_fcaller(OpcodeJumpTable[opcode(instr)], instr, pc);

out:
	/* Register zero must always be zero; this instruction forces this. */
	WriteLocalReg(Sreg_zero,0);

	
	if(m_RegDumpMark)
	{
		dump_regs(NULL);
	}

	/* 2nd half of trace recording for this instruction */
	if(opt_tracing && tracing)
	{
		write_trace_record_2(pc, instr);
		if (pc == opt_traceendpc)
			stop_tracing();
	}

	/* If there is an exception pending, we return now, so that we don't
	 * clobber the exception vector.
	 */
	if(exception_pending)
	{
		/* Instruction at beginning of exception handler is NOT in
		 * delay slot, no matter what the last instruction was.
		 */
		delay_state = NORMAL;
		return;
	}

	/* increment PC */
	if(delay_state == DELAYING)
	{
		/* This instruction caused a branch to be taken.
		 * The next instruction is in the delay slot.
		 * The next instruction EPC will be PC - 4.
		 */
		delay_state = DELAYSLOT;
		pc = pc + 4;
	}else if(delay_state == DELAYSLOT){
		/* This instruction was executed in a delay slot.
		 * The next instruction is on the other end of the branch.
		 * The next instruction EPC will be PC.
		 */
		if(delay_pc == 0)
		{
			sprintf(out, "Jumped to zero (jump was at 0x%x)", pc - 4);
			pwarn<<(*this)<<out<<pendl;
		}
		delay_state = NORMAL;
		pc = delay_pc;
	}else if(delay_state == NORMAL){
		/* No branch; next instruction is next word.
		 * Next instruction EPC is PC.
		 */
		pc = pc + 4;
	}
}

//instruction emulation
void SpeculativeCPU::funct_emulate(uint32 instr, uint32 pc)
{
	static const sp_fptr FunctJumpTable[] = {
		faddr(szero),	faddr(RI),
		faddr(srl),	faddr(sra),
		faddr(sllv),	faddr(RI),
		faddr(srlv),	faddr(srav),
		faddr(jr),	faddr(jalr),
		faddr(RI),	faddr(RI),
		faddr(syscall),	faddr(break),
		faddr(RI),	faddr(RI),
		faddr(mfhi),	faddr(mthi),
		faddr(mflo),	faddr(mtlo),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(mult),	faddr(multu),
		faddr(div),	faddr(divu),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(add),	faddr(addu),
		faddr(sub),	faddr(subu),
		faddr(and),	faddr(or),
		faddr(xor),	faddr(nor),
		faddr(RI),	faddr(RI),
		faddr(slt),	faddr(sltu),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(RI),	faddr(RI),
		faddr(fst),	faddr(ust)
	};
	sp_fcaller(FunctJumpTable[funct(instr)], instr, pc);
}

void SpeculativeCPU::szero_emulate(uint32 instr, uint32 pc)
{
	if(instr == 0)
		nop_emulate(instr, pc);
	else
		sll_emulate(instr, pc);
}

void SpeculativeCPU::nop_emulate(uint32 instr, uint32 pc)
{
	pinfo<<(*this)<<"instr: nop"<<pendl;

	DO_STATISTIC("nop", SUCCESS, NULL);
}

void SpeculativeCPU::regimm_emulate(uint32 instr, uint32 pc)
{
	CPU::regimm_emulate(instr, pc);
}

void SpeculativeCPU::j_emulate(uint32 instr, uint32 pc)
{
	CPU::j_emulate(instr, pc);
	pinfo<<(*this)<<"instr: j "<<std::hex<<calc_jump_target(instr, pc)<<pendl;

	DO_STATISTIC("j", SUCCESS, NULL);
}

void SpeculativeCPU::jal_emulate(uint32 instr, uint32 pc)
{
	delay_state = DELAYING;
	delay_pc = calc_jump_target(instr, pc);
	WriteLocalReg(Sreg_ra, pc + 8);
	pinfo<<(*this)<<"instr: jal "<<std::hex<<delay_pc<<pendl;

	DO_STATISTIC("jal", SUCCESS, NULL);
}

void SpeculativeCPU::beq_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue,tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	if(svalue == tvalue)
		branch(instr, pc);
	pinfo<<(*this)<<"instr: beq r"<<rs(instr)<<" r"<<rt(instr)<<" "<<s_immed(instr)<<pendl;

	DO_STATISTIC("beq", SUCCESS, NULL);
}

void SpeculativeCPU::bne_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue,tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	if(svalue != tvalue)
		branch(instr, pc);
	pinfo<<(*this)<<"instr: bne r"<<rs(instr)<<" r"<<rt(instr)<<" "<<s_immed(instr)<<pendl;

	DO_STATISTIC("bne", SUCCESS, NULL);
}

void SpeculativeCPU::blez_emulate(uint32 instr, uint32 pc)
{
	uint32 value;

	if(rt(instr) != 0)
		exception(RI);
	value = ReadLocalReg(rs(instr));
	if(value == 0 || (value & 0x80000000))
		branch(instr, pc);
	pinfo<<(*this)<<"instr: blez r"<<rs(instr)<<" "<<s_immed(instr)<<pendl;

	DO_STATISTIC("blez", SUCCESS, NULL);
}

void SpeculativeCPU::bgtz_emulate(uint32 instr, uint32 pc)
{
	uint32 value;

	if(rt(instr) != 0)
		exception(RI);
	value = ReadLocalReg(rs(instr));
	if(value != 0 && (value & 0x80000000) == 0)
		branch(instr, pc);
	pinfo<<(*this)<<"instr: bgtz r"<<rs(instr)<<" "<<s_immed(instr)<<pendl;

	DO_STATISTIC("bgtz", SUCCESS, NULL);
}

void SpeculativeCPU::addi_emulate(uint32 instr, uint32 pc)
{
	int32 a, b, sum;
	uint32 svalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	a = (int32)svalue;
	b = s_immed(instr);
	sum = a + b;
	if ((a < 0 && b < 0 && !(sum < 0)) || (a >= 0 && b >= 0 && !(sum >= 0)))
	{
		exception(Ov);
	}else{
		tvalue = (uint32)sum;
		WriteLocalReg(rt(instr), tvalue);
	}
	pinfo<<(*this)<<"instr: addi r"<<rs(instr)<<" r"<<rt(instr)<<" "<<b<<pendl;

	DO_STATISTIC("addi", SUCCESS, NULL);
}

void SpeculativeCPU::addiu_emulate(uint32 instr, uint32 pc)
{
	int32 a, b, sum;
	uint32 svalue, tvalue;
	
	svalue = ReadLocalReg(rs(instr));
	a = (int32)svalue;
	b = s_immed(instr);
	sum = a + b;
	tvalue = (uint32)sum;
	WriteLocalReg(rt(instr), tvalue);
	pinfo<<(*this)<<"instr: addiu r"<<rs(instr)<<" r"<<rt(instr)<<" "<<b<<pendl;

	DO_STATISTIC("addiu", SUCCESS, NULL);
}

void SpeculativeCPU::slti_emulate(uint32 instr, uint32 pc)
{
	int s_rs = (int32)ReadLocalReg(rs(instr));
	if(s_rs < s_immed(instr))
		WriteLocalReg(rt(instr), 1);
	else
		WriteLocalReg(rt(instr), 0);
	pinfo<<(*this)<<"instr: slti r"<<rs(instr)<<" r"<<rt(instr)<<" "<<s_immed(instr)<<pendl;

	DO_STATISTIC("slti", SUCCESS, NULL);
}

void SpeculativeCPU::sltiu_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, value;

	svalue = ReadLocalReg(rs(instr));
	value = (uint32)(int32)s_immed(instr);
	if(svalue < value)
		WriteLocalReg(rt(instr), 1);
	else
		WriteLocalReg(rt(instr), 0);
	pinfo<<(*this)<<"instr: sltiu r"<<rs(instr)<<" r"<<rt(instr)<<" "<<value<<pendl;

	DO_STATISTIC("sltiu", SUCCESS, NULL);
}

void SpeculativeCPU::andi_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ((svalue & 0x0ffff) & immed(instr));
	WriteLocalReg(rt(instr), tvalue);
	pinfo<<(*this)<<"instr: andi r"<<rs(instr)<<" r"<<rt(instr)<<" "<<std::hex<<immed(instr)<<pendl;

	DO_STATISTIC("andi", SUCCESS, NULL);
}

void SpeculativeCPU::ori_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = svalue | immed(instr);
	WriteLocalReg(rt(instr), tvalue);
	pinfo<<(*this)<<"instr: ori r"<<rs(instr)<<" r"<<rt(instr)<<" "<<std::hex<<immed(instr)<<pendl;

	DO_STATISTIC("ori", SUCCESS, NULL);
}

void SpeculativeCPU::xori_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = svalue ^ immed(instr);
	WriteLocalReg(rt(instr), tvalue);
	pinfo<<(*this)<<"instr: xori r"<<rs(instr)<<" r"<<rt(instr)<<" "<<std::hex<<immed(instr)<<pendl;

	DO_STATISTIC("xori", SUCCESS, NULL);
}

void SpeculativeCPU::lui_emulate(uint32 instr, uint32 pc)
{
	uint32 tvalue = immed(instr) << 16;
	WriteLocalReg(rt(instr), tvalue);	//t register or d register? It is a problem????????????
	pinfo<<(*this)<<"instr: lui r"<<rt(instr)<<" "<<immed(instr)<<pendl;

	DO_STATISTIC("lui", SUCCESS, NULL);
}

void SpeculativeCPU::cpzero_emulate(uint32 instr, uint32 pc)
{
	CPU::cpzero_emulate(instr, pc);
	pinfo<<(*this)<<"instr: cpzero"<<pendl;

	CpzeroClientData data;
	data.m_rs = rs(instr);
	data.m_funct = funct(instr);
	DO_STATISTIC("cpzero", SUCCESS, &data);
}

void SpeculativeCPU::cpone_emulate(uint32 instr, uint32 pc)
{
	pinfo<<(*this)<<"instr:"<< std::hex<<instr<<pendl;
	fpu->cpone_emulate(instr, pc);
	pinfo<<(*this)<<"instr: cpone"<<pendl;

	CponeClientData data;
	data.m_rs = rs(instr);
	data.m_funct = funct(instr);
	data.m_ft = fpu->ft(instr);
	DO_STATISTIC("cpone", SUCCESS, &data);
}

void SpeculativeCPU::cptwo_emulate(uint32 instr, uint32 pc)
{
	CPU::cptwo_emulate(instr, pc);
	pinfo<<(*this)<<"instr: cptwo"<<pendl;
}

void SpeculativeCPU::cpthree_emulate(uint32 instr, uint32 pc)
{
	CPU::cpthree_emulate(instr, pc);
	pinfo<<(*this)<<"instr: cpthree"<<pendl;
}

void SpeculativeCPU::lb_emulate(uint32 instr, uint32 pc)
{
	uint32 base, address;
	uint8 byte;
	int32 offset;

	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	pinfo<<(*this)<<"load address = "<<std::hex<<address<<pendl;
	if(SpLoad(address, byte))
	{
		//sign extend
		int32 value = (int32)byte;
		WriteLocalReg(rt(instr), (uint32)value);
		pinfo<<(*this)<<"load value"<<std::hex<<byte<<pendl;
		pinfo<<(*this)<<"instr: lb r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
	}

	DO_STATISTIC("lb", SUCCESS, NULL);
}

void SpeculativeCPU::lh_emulate(uint32 instr, uint32 pc)
{
	uint32 base, address;
	uint16 halfword;
	int32 offset;

	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	if (address % 2 != 0)
	{
		exception(AdEL,DATALOAD);
		return;
	}

	if(SpLoad(address, halfword))
	{
		//sign extend
		int32 value = (int32)halfword;
		WriteLocalReg(rt(instr), (uint32)value);
		pinfo<<(*this)<<"instr: lh r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
	}

	DO_STATISTIC("lh", SUCCESS, NULL);
}

void SpeculativeCPU::lwl_emulate(uint32 instr, uint32 pc)
{
	pwarn<<(*this)<<"instr lwl was not supported now!"<<pendl;
	throw ProphetException("can not load in unaligned address!");

	DO_STATISTIC("lwl", SUCCESS, NULL);
}

void SpeculativeCPU::lw_emulate(uint32 instr, uint32 pc)
{
	uint32 base, address, word;
	int32 offset;

	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;	//the obsolute of offset can never be greater than base

	if (address % 4 != 0)
	{
		exception(AdEL, DATALOAD);
		return;
	}
	
	pinfo<<(*this)<<"load address = "<<std::hex<<address<<pendl;
	if(SpLoad(address, word))
	{
		//sign extend only need for 64-bit CPU
		pinfo<<(*this)<<"load word = "<<std::hex<<word<<pendl;
		WriteLocalReg(rt(instr), word);
		pinfo<<(*this)<<"instr: lw r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
	}

	DO_STATISTIC("lw", SUCCESS, NULL);
}

void SpeculativeCPU::lbu_emulate(uint32 instr, uint32 pc)
{
	uint32 base, address, word;
	int32 offset;
	uint8 byte;

	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	if(SpLoad(address, byte))
	{
		word = byte & 0x000000ff;
		WriteLocalReg(rt(instr), word);
		pinfo<<(*this)<<"instr: lbu r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
	}

	DO_STATISTIC("lbu", SUCCESS, NULL);
}

void SpeculativeCPU::lhu_emulate(uint32 instr, uint32 pc)
{
	uint32 base, address, word;
	int32 offset;
	uint16 halfword;

	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	if (address % 2 != 0)
	{
		exception(AdEL, DATALOAD);
		return;
	}

	if(SpLoad(address, halfword))
	{
		word = halfword & 0x0000ffff;
		WriteLocalReg(rt(instr), word);
		pinfo<<(*this)<<"instr: lhu r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
	}

	DO_STATISTIC("lhu", SUCCESS, NULL);
}

void SpeculativeCPU::lwr_emulate(uint32 instr, uint32 pc)
{
	pwarn<<(*this)<<"instr lwr was not supported now!"<<pendl;
	throw ProphetException("can not load data in unaligned address!");

	DO_STATISTIC("lwr", SUCCESS, NULL);
}

void SpeculativeCPU::sb_emulate(uint32 instr, uint32 pc)
{
	uint32 address, base;
	uint8 byte;
	int32 offset;

	byte = ReadLocalReg(rt(instr)) & 0x0ff;
	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	SpStore(address, byte);
	pinfo<<(*this)<<"instr: sb r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;

	DO_STATISTIC("sb", SUCCESS, NULL);
}

void SpeculativeCPU::sh_emulate(uint32 instr, uint32 pc)
{
	uint32 address, base;
	uint16 halfword;
	int32 offset;

	halfword = ReadLocalReg(rt(instr)) & 0x0ffff;
	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	if (address % 2 != 0)
	{
		exception(AdES, DATASTORE);
		return;
	}

	SpStore(address, halfword);
	pinfo<<(*this)<<"instr: sh r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;

	DO_STATISTIC("sh", SUCCESS, NULL);
}

void SpeculativeCPU::swl_emulate(uint32 instr, uint32 pc)
{
	pwarn<<"instr swl was not support now!"<<pendl;
	throw ProphetException("can not store data in an unaligned address!");

	DO_STATISTIC("swl", SUCCESS, NULL);
}

void SpeculativeCPU::sw_emulate(uint32 instr, uint32 pc)
{
	uint32 address, base, word;
	int32 offset;
	
	word = ReadLocalReg(rt(instr));
	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	if (address % 4 != 0) 
	{
		exception(AdES, DATASTORE);
		return;
	}
	
	SpStore(address, word);
	pinfo<<(*this)<<"instr: sw r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;

	DO_STATISTIC("sw", SUCCESS, NULL);
}

void SpeculativeCPU::swr_emulate(uint32 instr, uint32 pc)
{
	pwarn<<"instr swr was not supported now!"<<pendl;
	throw ProphetException("can not store data in an unaligned address!");

	DO_STATISTIC("swr", SUCCESS, NULL);
}

void SpeculativeCPU::lwc1_emulate(uint32 instr, uint32 pc)
{
	uint32 address, base, word;
	int offset;

	// Calculate virtual address. 
	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	// This virtual address must be word-aligned. 
	if (address % 4 != 0)
	{
		exception(AdEL,DATALOAD);
		return;
	}

	pinfo<<(*this)<<"load address = "<<std::hex<<address<<pendl;
	if(SpLoad(address, word))
	{
		//sign extend only need for 64-bit CPU
		pinfo<<(*this)<<"load word = "<<std::hex<<word<<pendl;
		WriteLocalFreg(rt(instr), word);
		pinfo<<(*this)<<"instr: lwc1 r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
	}

	DO_STATISTIC("lwc1", SUCCESS, NULL);
}

void SpeculativeCPU::lwc2_emulate(uint32 instr, uint32 pc)
{
	CPU::lwc2_emulate(instr, pc);

	DO_STATISTIC("lwc2", SUCCESS, NULL);
}

void SpeculativeCPU::lwc3_emulate(uint32 instr, uint32 pc)
{
	CPU::lwc3_emulate(instr, pc);

	DO_STATISTIC("lwc3", SUCCESS, NULL);
}

void SpeculativeCPU::swc1_emulate(uint32 instr, uint32 pc)
{
	uint32 address, base, word;
	int offset;
	// Load data from register.
	word = ReadLocalFreg(rt(instr));

	// Calculate virtual address. 
	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	// This virtual address must be word-aligned. 
	if (address % 4 != 0) {
		exception(AdES,DATASTORE);
		return;
	}

	SpStore(address, word);
	pinfo<<(*this)<<"instr: swc1 r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;

	DO_STATISTIC("swc1", SUCCESS, NULL);
}

void SpeculativeCPU::swc2_emulate(uint32 instr, uint32 pc)
{
	CPU::swc2_emulate(instr, pc);

	DO_STATISTIC("swc2", SUCCESS, NULL);
}

void SpeculativeCPU::swc3_emulate(uint32 instr, uint32 pc)
{
	CPU::swc3_emulate(instr, pc);

	DO_STATISTIC("swc3", SUCCESS, NULL);
}

void SpeculativeCPU::ldc1_emulate(uint32 instr, uint32 pc)
{
	uint32 address, base, wordh,wordl,temp;
	int offset;

	// Calculate virtual address. 
	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	// This virtual address must be word-aligned. 
	if (address % 8 != 0)
	{
		exception(AdEL,DATALOAD);
		return;
	}
	pinfo<<(*this)<<"load address = "<<std::hex<<address<<pendl;
		
	if(SpLoad(address+4, wordh) && SpLoad(address, wordl))
	{
		pinfo<<(*this)<<"load wordh = "<<std::hex<<wordh<<pendl;
		pinfo<<(*this)<<"load wordl = "<<std::hex<<wordl<<pendl;
		if(m_SwapMark)
		{
			temp = wordh;
			wordh = wordl;
			wordl = temp;
		}
		WriteLocalFreg(rt(instr)+1,wordh);
		WriteLocalFreg(rt(instr),wordl);
		pinfo<<(*this)<<"instr: ldc1 r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
	}
}

void SpeculativeCPU::sdc1_emulate(uint32 instr, uint32 pc)
{
	uint32 address,base,wordh,wordl,temp;
	int offset;

	wordh = ReadLocalFreg(rt(instr));
	wordl = ReadLocalFreg(rt(instr)+1);

	base = ReadLocalReg(rs(instr));
	offset = s_immed(instr);
	address = base + offset;

	if(address % 8 != 0)
	{
		exception(AdES,DATASTORE);
		return;
	}
	if(m_SwapMark)
	{
		temp = wordh;
		wordh = wordl;
		wordl = temp;
	}	

	SpStore(address,wordh);
	SpStore(address+4,wordl);
	pinfo<<(*this)<<"instr: sdc1 r"<<rt(instr)<<" "<<offset<<"(r"<<rs(instr)<<")"<<pendl;
}

void SpeculativeCPU::sll_emulate(uint32 instr, uint32 pc)
{
	uint32 tvalue, dvalue;
	
	tvalue = ReadLocalReg(rt(instr));
	dvalue = tvalue << shamt(instr);
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: sll r"<<rd(instr)<<" r"<<rt(instr)<<" "<<shamt(instr)<<pendl;

	DO_STATISTIC("sll", SUCCESS, NULL);
}

void SpeculativeCPU::srl_emulate(uint32 instr, uint32 pc)
{
	uint32 tvalue, dvalue;
	
	tvalue = ReadLocalReg(rt(instr));
	dvalue = srl(tvalue, shamt(instr));
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: srl r"<<rd(instr)<<" r"<<rt(instr)<<" "<<shamt(instr)<<pendl;

	DO_STATISTIC("srl", SUCCESS, NULL);
}

void SpeculativeCPU::sra_emulate(uint32 instr, uint32 pc)
{
	uint32 tvalue, dvalue;

	tvalue = ReadLocalReg(rt(instr));
	dvalue = sra(tvalue, shamt(instr));
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: sra r"<<rd(instr)<<" r"<<rt(instr)<<" "<<shamt(instr)<<pendl;

	DO_STATISTIC("sra", SUCCESS, NULL);
}

void SpeculativeCPU::sllv_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, dvalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	dvalue = tvalue << (svalue & 0x01f);
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: sllv r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("sllv", SUCCESS, NULL);
}

void SpeculativeCPU::srlv_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, dvalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	dvalue = srl(tvalue, svalue & 0x01f);
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: srlv r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("srlv", SUCCESS, NULL);
}

void SpeculativeCPU::srav_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, dvalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	dvalue = sra(tvalue, svalue & 0x01f);
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: srav r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("srav", SUCCESS, NULL);
}

void SpeculativeCPU::jr_emulate(uint32 instr, uint32 pc)
{
	uint32 dvalue,svalue;

	dvalue = ReadLocalReg(rd(instr));
	if (dvalue != 0)
	{
		exception(RI);
		return;
	}
	delay_state = DELAYING;
	svalue = ReadLocalReg(rs(instr));
	delay_pc = svalue;
	pinfo<<(*this)<<"instr: jr r"<<rd(instr)<<" r"<<rs(instr)<<pendl;

	DO_STATISTIC("jr", SUCCESS, NULL);
}

void SpeculativeCPU::jalr_emulate(uint32 instr, uint32 pc)
{
	delay_state = DELAYING;
	uint32 dvalue = ReadLocalReg(rd(instr));
	delay_pc = dvalue;
	WriteLocalReg(rs(instr), pc + 8);
	pinfo<<(*this)<<"instr: jalr r"<<rd(instr)<<" r"<<rs(instr)<<pendl;

	DO_STATISTIC("jalr", SUCCESS, NULL);
}

void SpeculativeCPU::syscall_emulate(uint32 instr, uint32 pc)
{
	uint32 phys, virt;
	int fnum, snum, tnum;
	uint8 byte;
	bool done;
	uint32 m, n;
	uint64 d;
	double a, b;
	std::ofstream file;
	static char buffer[1024];

	if(code(instr) == OPEN_VERIFY)
	{
		m_NeedVerify = true;
		return;
	}else if(code(instr) == CLOSE_VERIFY) {
		m_NeedVerify = false;
		return;
	}

	file.open("prophet_console.txt", std::ios_base::out | std::ios_base::app);
	if(!file)
		perror<<"can not open the console file!"<<pendl;

	int i = 0;
	char str[200];
	bool cacheable;
	virt = ReadLocalReg(4);
//	phys = cpzero->address_trans(virt, DATALOAD, &cacheable, this);
//	if (exception_pending)
//		return;

//	while((str[i] = (char)(mem->fetch_byte(phys, cacheable, this) & 0x0ff)) != '\0')
	do{
		done = SpLoad(virt + i, byte);
	}while((str[i++] = byte) != '\0' && done);

	if (exception_pending)
		return;
	if(code(instr) == 1) {
		file<<str;
		ProphetConsole::Write(str);
	}else if(code(instr) == 2){
		fnum = (int)ReadLocalReg(5);

		sprintf(buffer, str, fnum);
		file<<buffer;

		ProphetConsole::Write(str, fnum);
	}else if(code(instr) == 3){
		fnum = (int)ReadLocalReg(5);
		snum = (int)ReadLocalReg(6);

		sprintf(buffer, str, fnum, snum);
		file<<buffer;

		ProphetConsole::Write(str, fnum, snum);
	}else if(code(instr) == 4){
		fnum = (int)ReadLocalReg(5);
		snum = (int)ReadLocalReg(6);
		tnum = (int)ReadLocalReg(7);

		sprintf(buffer, str, fnum, snum, tnum);
		file<<buffer;

		ProphetConsole::Write(str, fnum, snum, tnum);
	}else if(code(instr) == 5) {
		m = ReadLocalFreg(12);
		n = ReadLocalFreg(13);
		d = ((uint64)n << 32) | m;
		fpu->interpret_double_value(d, a);

		sprintf(buffer, str, a);
		file<<buffer;

		ProphetConsole::Write(str, a);
	}else if(code(instr) == 6) {
		m = ReadLocalFreg(12);
		n = ReadLocalFreg(13);
		d = ((uint64)n<<32) | m;
		fpu->interpret_double_value(d,a);

		m = ReadLocalFreg(14);
		n = ReadLocalFreg(15);
		d = ((uint64)n<<32) | m;
		fpu->interpret_double_value(d,b);

		sprintf(buffer, str, a, b);
		file<<buffer;

		ProphetConsole::Write(str, a, b);
	}else if(code(instr) == 7) {			//print one float and one integer
		m = ReadLocalReg(6);
		n = ReadLocalReg(7);
		d = ((uint64)n<<32) | m;
		fpu->interpret_double_value(d, a);

		SpLoad(ReadLocalReg(29) + (uint32)108, m);

		sprintf(buffer, str, a, m);
		file<<buffer;

		ProphetConsole::Write(str, a, m);
	}
	file.close();

	pinfo<<(*this)<<"instr: syscall"<<pendl;

	DO_STATISTIC("syscall", SUCCESS, NULL);
}

void SpeculativeCPU::break_emulate(uint32 instr, uint32 pc)
{
	DO_STATISTIC("break", SUCCESS, NULL);

	//assert(SpeculativeLogic::GetInstance()->GetSubthread(this) == NULL);
	if(code11(instr) != 0)
	{
		if(m_State == STABLE_EXECUTION)
		{
			perror<<(*this)<<"programe error!"<<pendl;
		}else{
			Restart();
			SignalSquash(EXCEPTION);
		}
	}else{
		if(m_State == SP_EXECUTION || m_State == WAIT)
		{
			HoldBREAK(instr);
		}else if(m_State == STABLE_EXECUTION){	//now the thread state is stable
			CommitValue();
			FreeCache();
			m_State = IDLE;
			SpeculativeLogic::GetInstance()->SwapToFreeList(this);

			//quit running
			raise(SIGQUIT);
		}else{
			perror<<(*this)<<"can not process break instruction!"<<pendl;
		}
	}
	pinfo<<(*this)<<"instr: break"<<pendl;
}

void SpeculativeCPU::mfhi_emulate(uint32 instr, uint32 pc)
{
	WriteLocalReg(rd(instr), hi);
	pinfo<<(*this)<<"instr: mfhi r"<<rd(instr)<<pendl;

	DO_STATISTIC("mfhi", SUCCESS, NULL);
}

void SpeculativeCPU::mthi_emulate(uint32 instr, uint32 pc)
{
	if (rd(instr) != 0) 
	{
		exception(RI);
		return;
	}
	hi = ReadLocalReg(rs(instr));
	pinfo<<(*this)<<"instr: mthi r"<<rs(instr)<<pendl;

	DO_STATISTIC("mthi", SUCCESS, NULL);
}

void SpeculativeCPU::mflo_emulate(uint32 instr, uint32 pc)
{
	WriteLocalReg(rd(instr),lo);
	pinfo<<(*this)<<"instr: mflo r"<<rd(instr)<<pendl;

	DO_STATISTIC("mflo", SUCCESS, NULL);
}

void SpeculativeCPU::mtlo_emulate(uint32 instr, uint32 pc)
{
	if (rd(instr) != 0) 
	{
		exception(RI);
		return;
	}
	lo = ReadLocalReg(rs(instr));
	pinfo<<(*this)<<"instr: mtlo r"<<rs(instr)<<pendl;

	DO_STATISTIC("mtlo", SUCCESS, NULL);
}

void SpeculativeCPU::mult_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue;

	if (rd(instr) != 0)
	{
		exception(RI);
		return;
	}

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	mult64s(&hi, &lo, svalue, tvalue);
	pinfo<<(*this)<<"instr: mult r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("mult", SUCCESS, NULL);
}

void SpeculativeCPU::multu_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue;
	
	if (rd(instr) != 0)
	{
		exception(RI);
		return;
	}

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	mult64(&hi, &lo, svalue, tvalue);
	pinfo<<(*this)<<"instr: multu r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("multu", SUCCESS, NULL);
}

void SpeculativeCPU::div_emulate(uint32 instr, uint32 pc)
{
	int32 svalue, tvalue, result, mod;

	svalue = (int32)ReadLocalReg(rs(instr));
	tvalue = (int32)ReadLocalReg(rt(instr));
	result = svalue / tvalue;
	mod = svalue % tvalue;
	if(tvalue > 0 && mod < 0)
		mod += tvalue;

	lo = (uint32)result;
	hi = (uint32)mod;
	pinfo<<(*this)<<"instr: div r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("div", SUCCESS, NULL);
}

void SpeculativeCPU::divu_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	lo = svalue / tvalue;
	hi = svalue % tvalue;
	pinfo<<(*this)<<"instr: divu r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("divu", SUCCESS, NULL);
}

void SpeculativeCPU::add_emulate(uint32 instr, uint32 pc)
{
	int32 svalue, tvalue, sum;
	uint32 dvalue;

	svalue = (int32)ReadLocalReg(rs(instr));
	tvalue = (int32)ReadLocalReg(rt(instr));
	sum = svalue + tvalue;
	if ((svalue < 0 && tvalue < 0 && !(sum < 0)) || (svalue >= 0 && tvalue >= 0 && !(sum >= 0))) 
	{
		exception(Ov);
		return;
	}else{
		dvalue = (uint32)sum;
		WriteLocalReg(rd(instr), dvalue);
	}
	pinfo<<(*this)<<"instr: add r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("add", SUCCESS, NULL);
}

void SpeculativeCPU::addu_emulate(uint32 instr, uint32 pc)
{
	int32 svalue, tvalue, sum;
	uint32 dvalue;

	svalue = (int32)ReadLocalReg(rs(instr));
	tvalue = (int32)ReadLocalReg(rt(instr));
	sum = svalue + tvalue;
	dvalue = (uint32)sum;
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: addu r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("addu", SUCCESS, NULL);
}

void SpeculativeCPU::sub_emulate(uint32 instr, uint32 pc)
{
	int32 svalue, tvalue, diff;
	uint32 dvalue;

	svalue = (int32)ReadLocalReg(rs(instr));
	tvalue = (int32)ReadLocalReg(rt(instr));
	diff = svalue - tvalue;
	if ((svalue < 0 && !(tvalue < 0) && !(diff < 0)) || (!(svalue < 0) && tvalue < 0 && (diff < 0))) 
	{
		exception(Ov);
		return;
	}else{
		dvalue = (uint32)diff;
		WriteLocalReg(rd(instr), dvalue);
	}
	pinfo<<(*this)<<"instr: sub r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("sub", SUCCESS, NULL);
}

void SpeculativeCPU::subu_emulate(uint32 instr, uint32 pc)
{
	int32 svalue, tvalue, diff;
	uint32 dvalue;

	svalue = (int32)ReadLocalReg(rs(instr));
	tvalue = (int32)ReadLocalReg(rt(instr));
	diff = svalue - tvalue;
	dvalue = (uint32)diff;
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: subu r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("subu", SUCCESS, NULL);
}

void SpeculativeCPU::and_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue, dvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	dvalue = svalue & tvalue;
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: and r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("and", SUCCESS, NULL);
}

void SpeculativeCPU::or_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue, dvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	dvalue = svalue | tvalue;
	WriteLocalReg(rd(instr), dvalue);
	pinfo<<(*this)<<"instr: or r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("or", SUCCESS, NULL);
}

void SpeculativeCPU::xor_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue, dvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	dvalue = svalue ^ tvalue;
	WriteLocalReg(rd(instr),dvalue);
	pinfo<<(*this)<<"instr: xor r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("xor", SUCCESS, NULL);
}

void SpeculativeCPU::nor_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue, dvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	dvalue = ~(svalue | tvalue);
	WriteLocalReg(rd(instr),dvalue);
	pinfo<<(*this)<<"instr: nor r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("nor", SUCCESS, NULL);
}

void SpeculativeCPU::slt_emulate(uint32 instr, uint32 pc)
{
	int32 svalue, tvalue;
	svalue = (int32)ReadLocalReg(rs(instr));
	tvalue = (int32)ReadLocalReg(rt(instr));
	if(svalue < tvalue)
	{
		WriteLocalReg(rd(instr),1);
	}else{
		WriteLocalReg(rd(instr),0);
	}
	pinfo<<(*this)<<"instr: slt r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("slt", SUCCESS, NULL);
}

void SpeculativeCPU::sltu_emulate(uint32 instr, uint32 pc)
{
	uint32 svalue, tvalue;

	svalue = ReadLocalReg(rs(instr));
	tvalue = ReadLocalReg(rt(instr));
	if(svalue < tvalue)
	{
		WriteLocalReg(rd(instr),1);
	}else{
		WriteLocalReg(rd(instr),0);
	}
	pinfo<<(*this)<<"instr: sltu r"<<rd(instr)<<" r"<<rs(instr)<<" r"<<rt(instr)<<pendl;

	DO_STATISTIC("sltu", SUCCESS, NULL);
}

void SpeculativeCPU::bltz_emulate(uint32 instr, uint32 pc)
{
	int32 svalue;

	svalue = (int32)ReadLocalReg(rs(instr));
	if(svalue < 0)
	{
		branch(instr, pc);
	}
	pinfo<<(*this)<<"instr: bltz r"<<rs(instr)<<" LABEL"<<pendl;

	DO_STATISTIC("bltz", SUCCESS, NULL);
}

void SpeculativeCPU::bgez_emulate(uint32 instr, uint32 pc)
{
	int32 svalue;

	svalue = (int32)ReadLocalReg(rs(instr));
	if(svalue >= 0)
	{
		branch(instr, pc);
	}
	pinfo<<(*this)<<"instr: bgez r"<<rs(instr)<<" LABEL"<<pendl;

	DO_STATISTIC("bgez", SUCCESS, NULL);
}

void SpeculativeCPU::bltzal_emulate(uint32 instr, uint32 pc)
{
	int32 svalue;
	
	WriteLocalReg(Sreg_ra, pc + 8);
	svalue = (int32)ReadLocalReg(rs(instr));
	if(svalue < 0)
	{
		branch(instr, pc);
	}
	pinfo<<(*this)<<"instr: bltzal r"<<rs(instr)<<" LABEL"<<pendl;

	DO_STATISTIC("bltzal", SUCCESS, NULL);
}

void SpeculativeCPU::bgezal_emulate(uint32 instr, uint32 pc)
{
	int32 svalue;

	WriteLocalReg(Sreg_ra, pc + 8);
	svalue = (int32)ReadLocalReg(rs(instr));
	if(svalue >= 0)
	{
		branch(instr, pc);
	}
	pinfo<<(*this)<<"instr: bgezal r"<<rs(instr)<<" LABEL"<<pendl;

	DO_STATISTIC("bgezal", SUCCESS, NULL);
}

void SpeculativeCPU::RI_emulate(uint32 instr, uint32 pc)
{
	CPU::RI_emulate(instr, pc);
}

void SpeculativeCPU::spawn_emulate(uint32 instr, uint32 pc)
{
	SpeculativeCPU *sub = SpeculativeLogic::GetInstance()->NotifySpawn(this);
	if(sub)
		pinfo<<(*this)<<"spawn a sub thread on cpu "<<sub->m_ID<<pendl;
	else
		pinfo<<(*this)<<"can not spwan here!"<<pendl;
	pinfo<<(*this)<<"instr: spawn"<<pendl;

	SpawnClientData data;
	data.m_SubAdder = sub;
	DO_STATISTIC("spawn", sub ? SUCCESS : FAILED, &data);
}

void SpeculativeCPU::cqip_emulate(uint32 instr, uint32 pc)
{
	OnCQIP(instr);
	pinfo<<(*this)<<"instr: cqip"<<pendl;

	DO_STATISTIC("cqip", SUCCESS, NULL);
}

void SpeculativeCPU::squash_emulate(uint32 instr, uint32 pc)
{
	if(m_State == STABLE_EXECUTION)
		SignalSquash(INSTRUCTION);
	else
		pwarn<<(*this)<<"can only execute squash instruction in stable state!"<<pendl;
	pinfo<<(*this)<<"instr: squash"<<pendl;

	DO_STATISTIC("squash", SUCCESS, NULL);
}

void SpeculativeCPU::fst_emulate(uint32 instr, uint32 pc)
{
	assert(rs(instr) == SP_REG);

	//test if I am on the stack top
	if(LockStack())
	{
		ReadLocalReg(SP_REG);
		//reg[SP_REG] = SpeculativeLogic::GetInstance()->GetStackTop(this);
		WriteLocalReg(SP_REG, SpeculativeLogic::GetInstance()->GetStackTop(this));
	}else{
		this->pc -= 4;	//keep the pc pointing to the same address and wait for stack lock
	}
	pinfo<<(*this)<<"instr: fst, new stack top is "<<std::hex<<reg[29]<<pendl;

	DO_STATISTIC("fst", SUCCESS, NULL);
}

void SpeculativeCPU::ust_emulate(uint32 instr, uint32 pc)
{
	m_StackLocked = false;
	pinfo<<(*this)<<"instr: ust"<<pendl;

	DO_STATISTIC("ust", SUCCESS, NULL);
}

void SpeculativeCPU::pslice_entry_emulate(uint32 instr, uint32 pc)
{
	m_State = PRE_COMPUTATION;
	m_PPC = pc;
	pinfo<<(*this)<<"instr: pslice_entry"<<pendl;

	DO_STATISTIC("pslice_entry", SUCCESS, NULL);
}

void SpeculativeCPU::pslice_exit_emulate(uint32 instr, uint32 pc)
{
	SaveRegValues();
	m_State = SP_EXECUTION;
	pinfo<<(*this)<<"instr: pslice_exit"<<pendl;

	DO_STATISTIC("pslice_exit", SUCCESS, NULL);
}