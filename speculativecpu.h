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

#ifndef _SPECULATIVECPU_H
#define _SPECULATIVECPU_H

#include "cpu.h"
#include "obstack.h"
#include "predefine.h"
#include "prophetlog.h"
#include "prophetstatistic.h"
#include "prophetfpu.h"
#include <iostream>

#define BYTE_PER_WORD 4
#define CACHE_STATISTIC
#define STABLE_VERSION 1

PROPHET_STAT_CLASS(SpeculativeCPU) WITH_PUBLIC_ROOT(CPU)
{
	PLOG_CLASS(SpeculativeCPU);

	friend class FPU;
public:
	typedef enum{BYTE, HALFWORD, WORD, DWORD} CPU_DTYPE;

	typedef enum{DDV,		//data violation
		INSTRUCTION, 		//squash instruction
		V_REG_VOILATE,		//register voilation in verification
		V_MM_VOILATE,		//main memory voilate in verification
		LOW_PRIORITY,		//a thread with low prority is spawned
		CONTROL_VOILATE,	//control voilation
		RESET,			//the machine was reset
		HALT,			//the machine was halted
		QUIT,			//receive quit signal
		EXCEPTION,		//squash on exception
	} SQUASH_REASON;

	typedef enum{
		UNDEFINE = 0,
		IDLE = 1,
		INITIALIZATON = 2,
		PRE_COMPUTATION = 3,
		SP_EXECUTION = 4,
		RESTART = 5,
		WAIT = 6,
		SQUASH = 7,
		STABLE_TOKEN = 8,
		STABLE_EXECUTION = 9,
		SUBTHREAD_VERIFICATION = 10,
		COMMIT = 11
	} SPECULATIVE_STATE;

	typedef struct RegCacheItem
	{
		int m_isValide;		// -1: invalide, 0: read first, 1: write first
		bool m_isRead;
		bool m_isWritten;
		uint32 m_Value;
	} RegCacheItem;

	typedef struct FregCacheItem	// this is used for float reg
	{
		int m_Fvalide;
		bool m_Fread;
		bool m_Fwritten;
		uint32 m_Fvalue;
	}FregCacheItem;

	typedef struct MemCacheItem
	{
		bool m_IsValide[BYTE_PER_WORD];
		int m_Version;
		bool m_IsOld;
		bool m_IsRemoteLoaded[BYTE_PER_WORD];
		bool m_IsModified[BYTE_PER_WORD];
		uint32 m_Address;
		uint32 m_Value;
	} MemCacheItem;

	typedef struct MemCacheEntry
	{
		MemCacheEntry *m_Next;
		MemCacheItem m_Entry;	//we use the address as the key of an entry
	} MemCacheEntry;

	typedef struct MemCache		//memcache is implemented with hash map
	{
		MemCacheEntry **m_Cache;
		uint32 m_Size;		//total number of slot in the hash table
		struct obstack m_Memory;
		bool m_MemoryInited;	//indicating whether the memory pool was allocated or not
		bool m_HaveInvalideEntry;

		#ifdef CACHE_STATISTIC
		uint32 m_Insertions;
		uint32 m_Lookups;
		uint32 m_Deletions;
		uint32 m_Compares;
		#endif
	} MemCache;

	SpeculativeCPU(Mapper &r, IntCtrl &i);
	virtual ~SpeculativeCPU();

	virtual void dump_regs(FILE *f);
	inline uint32 code(uint32 instr);
	inline uint32 code11(uint32 instr);
	void OnSquash(SpeculativeCPU*, SQUASH_REASON);
	void OnSpawn(SpeculativeCPU*);			//handle the spawn message
	bool RcVerification(SpeculativeCPU*);
	void OnStableToken(SpeculativeCPU*);

	SET_GET_ACCESSOR(uint32, ThreadVersion, version);
	SET_GET_ACCESSOR(bool, RegDumpMark, mark);
	GET_ACCESSOR(uint32, ID);
	GET_ACCESSOR(SPECULATIVE_STATE, State);
	bool NeedSwap() const { return m_SwapMark;}
	void SetSwapMark() { m_SwapMark = true; }

	//fetch and execute a instruction
	virtual void step();

	//handle exception
	virtual void exception(uint16 exeCode, int mode = ANY, int coprocno = -1);

	//initialize as a stable cpu
	void reset();

protected:
	typedef enum CacheEntryOperationType{
		SAVE = 0,
		STORE = 1,
	}CacheEntryOperationType;

	void InitSubthread(SpeculativeCPU*);		//callbacked by the subthread
	void EnterPslice();		//execute pslice instruction

	bool SpLoad(uint32, uint32&, bool = true, MemCacheEntry** = NULL);	//speculatively load a 32bit data from specific address
	bool SpLoad(uint32, uint16&, bool = true, MemCacheEntry** = NULL);	//speculatively load a 16bit data from specific address
	bool SpLoad(uint32, uint8&, bool = true, MemCacheEntry** = NULL);	//speculatively load a 8bit data from specific address

	bool SpStore(uint32, uint32);	//speculatively store a 32bit data to specific address
	bool SpStore(uint32, uint16);	//speculatively store a 16bit data to specific adress
	bool SpStore(uint32, uint8);	//speculatively store a 8bit data to specific address

	void SaveRegValues();		//save the registers' value to L1Cache when quit pre-computation state
	bool SpawnSubthread();		//spawn a subthread when execute a spawn instruction
	void SignalSquash(SQUASH_REASON);
	void SignalDDV(SpeculativeCPU*, uint32);
//	void OnDDV(SpeculativeCPU*, uint32);
	void OnCQIP(uint32);
	void OnTrap();
	bool Verify(SpeculativeCPU*, uint32);
	bool VerifyEntry(MemCacheEntry*);
	void CommitValue();
	void OnStore(uint32, uint32, uint32);
	void RestoreRegisters();
	void CommitMM();
	void CommitEntry(MemCacheEntry*);
	void CommitRegisters();
	void HoldCQIP(uint32);
	void HoldBREAK(uint32);
	void SkipPslice();
	void Restart();
	void BeginSpRun();

	//class field accessors
	void SetRegularReg(uint32*);		//set the values of regular registers
	void SetFloatreg(uint32*);		//set the values of float registers
	void SetRegCache(RegCacheItem*);
	inline void SetPC(uint32);

	//void AcceptInit(SpeculativeCPU*);	//subthread call this to initialize itself when it was spawned
	void InitCache();			//initialize L1cache and register cache
	void FreeCache();			//free cache memory
	void AllocCache();			//allocate memory for cache
	void InitCacheEntry(MemCacheEntry*);
	void InvalideButPslice();		//invalide all cache entry that are not generated in pslice
	MemCacheEntry* FindInvalide();		//find a entry that contains invalude value only
	MemCacheEntry* NewCacheEntry();		//allocate a entry and initialize it
	inline uint32 HashMap(uint32);		//map a address to its cache entry index
	void FreeStack();
	bool LockStack();

	inline uint32 ReadLocalFreg(uint32);
	inline void WriteLocalFreg(uint32,uint32);
	inline uint32 ReadLocalReg(uint32);
	inline void WriteLocalReg(uint32, uint32);

	//Possible LocalLoad functions called by SpLoadTemplate
	bool LocalLoad(uint32, uint32&);	//called by the thread self
	bool LocalLoad(uint32, uint32&, int);	//called by the subthread
	bool LocalLoad(uint32, uint16&);
	bool LocalLoad(uint32, uint16&, int);
	bool LocalLoad(uint32, uint8&);
	bool LocalLoad(uint32, uint8&, int);

	//real implementation of Localload
	bool LocalLoadCore(uint32, uint32&, uint32);
	bool LocalLoadCore(uint32, uint32&, uint32, int);	//local load with version
	bool ReadDataFromEntry(MemCacheEntry *entry, uint32 width, uint32 offset, uint32 &value);

	bool SpStore(uint32, uint32, uint32);		//implementation of the store operation

	//real implementation of SpLoad
	template <typename T>
	bool SpLoadTemplate(uint32, T&, bool, MemCacheEntry**);

	bool IsRefreshed(uint32, uint32, uint32);
	bool TestDDV(uint32, uint32, uint32);

	//undefined store
	void SpSave(uint32, uint32, MemCacheEntry**  = NULL);
	void SpSave(uint32, uint16, MemCacheEntry** = NULL);
	void SpSave(uint32, uint8, MemCacheEntry** = NULL);
	void SpSave(uint32, uint32, uint32, MemCacheEntry** = NULL);

	void FillCacheEntry(MemCacheEntry*, uint32 address, uint32 value, uint32 bitnum, CacheEntryOperationType type);
	void InvalideEntry(MemCacheEntry*);

	//read data from main memory
	bool MMLoad(uint32, uint32&);
	bool MMLoad(uint32, uint16&);
	bool MMLoad(uint32, uint8&);

	//write data to main memory
	bool MMStore(uint32, uint32);
	bool MMStore(uint32, uint16);
	bool MMStore(uint32, uint8);

	inline uint32 TrySwapWord(uint32);
	inline uint16 TrySwapHalfWord(uint16);
	inline uint32 CalSubAddress(uint32);

	//instruction emulation
	virtual void funct_emulate(uint32 instr, uint32 pc);
	virtual void regimm_emulate(uint32 instr, uint32 pc);
	virtual void j_emulate(uint32 instr, uint32 pc);
	virtual void jal_emulate(uint32 instr, uint32 pc);
	virtual void beq_emulate(uint32 instr, uint32 pc);
	virtual void bne_emulate(uint32 instr, uint32 pc);
	virtual void blez_emulate(uint32 instr, uint32 pc);
	virtual void bgtz_emulate(uint32 instr, uint32 pc);
	virtual void addi_emulate(uint32 instr, uint32 pc);
	virtual void addiu_emulate(uint32 instr, uint32 pc);
	virtual void slti_emulate(uint32 instr, uint32 pc);
	virtual void sltiu_emulate(uint32 instr, uint32 pc);
	virtual void andi_emulate(uint32 instr, uint32 pc);
	virtual void ori_emulate(uint32 instr, uint32 pc);
	virtual void xori_emulate(uint32 instr, uint32 pc);
	virtual void lui_emulate(uint32 instr, uint32 pc);
	virtual void cpzero_emulate(uint32 instr, uint32 pc);
	virtual void cpone_emulate(uint32 instr, uint32 pc);
	virtual void cptwo_emulate(uint32 instr, uint32 pc);
	virtual void cpthree_emulate(uint32 instr, uint32 pc);
	virtual void lb_emulate(uint32 instr, uint32 pc);
	virtual void lh_emulate(uint32 instr, uint32 pc);
	virtual void lwl_emulate(uint32 instr, uint32 pc);
	virtual void lw_emulate(uint32 instr, uint32 pc);
	virtual void lbu_emulate(uint32 instr, uint32 pc);
	virtual void lhu_emulate(uint32 instr, uint32 pc);
	virtual void lwr_emulate(uint32 instr, uint32 pc);
	virtual void sb_emulate(uint32 instr, uint32 pc);
	virtual void sh_emulate(uint32 instr, uint32 pc);
	virtual void swl_emulate(uint32 instr, uint32 pc);
	virtual void sw_emulate(uint32 instr, uint32 pc);
	virtual void swr_emulate(uint32 instr, uint32 pc);
	virtual void lwc1_emulate(uint32 instr, uint32 pc);
	virtual void lwc2_emulate(uint32 instr, uint32 pc);
	virtual void lwc3_emulate(uint32 instr, uint32 pc);
	virtual void swc1_emulate(uint32 instr, uint32 pc);
	virtual void swc2_emulate(uint32 instr, uint32 pc);
	virtual void swc3_emulate(uint32 instr, uint32 pc);
	virtual void sll_emulate(uint32 instr, uint32 pc);
	virtual void srl_emulate(uint32 instr, uint32 pc);
	virtual void sra_emulate(uint32 instr, uint32 pc);
	virtual void sllv_emulate(uint32 instr, uint32 pc);
	virtual void srlv_emulate(uint32 instr, uint32 pc);
	virtual void srav_emulate(uint32 instr, uint32 pc);
	virtual void jr_emulate(uint32 instr, uint32 pc);
	virtual void jalr_emulate(uint32 instr, uint32 pc);
	virtual void syscall_emulate(uint32 instr, uint32 pc);
	virtual void break_emulate(uint32 instr, uint32 pc);
	virtual void mfhi_emulate(uint32 instr, uint32 pc);
	virtual void mthi_emulate(uint32 instr, uint32 pc);
	virtual void mflo_emulate(uint32 instr, uint32 pc);
	virtual void mtlo_emulate(uint32 instr, uint32 pc);
	virtual void mult_emulate(uint32 instr, uint32 pc);
	virtual void multu_emulate(uint32 instr, uint32 pc);
	virtual void div_emulate(uint32 instr, uint32 pc);
	virtual void divu_emulate(uint32 instr, uint32 pc);
	virtual void add_emulate(uint32 instr, uint32 pc);
	virtual void addu_emulate(uint32 instr, uint32 pc);
	virtual void sub_emulate(uint32 instr, uint32 pc);
	virtual void subu_emulate(uint32 instr, uint32 pc);
	virtual void and_emulate(uint32 instr, uint32 pc);
	virtual void or_emulate(uint32 instr, uint32 pc);
	virtual void xor_emulate(uint32 instr, uint32 pc);
	virtual void nor_emulate(uint32 instr, uint32 pc);
	virtual void slt_emulate(uint32 instr, uint32 pc);
	virtual void sltu_emulate(uint32 instr, uint32 pc);
	virtual void bltz_emulate(uint32 instr, uint32 pc);
	virtual void bgez_emulate(uint32 instr, uint32 pc);
	virtual void bltzal_emulate(uint32 instr, uint32 pc);
	virtual void bgezal_emulate(uint32 instr, uint32 pc);
	virtual void spawn_emulate(uint32 instr, uint32 pc);
	virtual void squash_emulate(uint32 instr, uint32 pc);
	virtual void cqip_emulate(uint32 instr, uint32 pc);
	virtual void fst_emulate(uint32 instr, uint32 pc);
	virtual void ust_emulate(uint32 instr, uint32 pc);
	virtual void pslice_entry_emulate(uint32 instr, uint32 pc);
	virtual void pslice_exit_emulate(uint32 instr, uint32 pc);
	virtual void RI_emulate(uint32 instr, uint32 pc);
	virtual void nop_emulate(uint32 instr, uint32 pc);
	virtual void szero_emulate(uint32 instr, uint32 pc);
	virtual void ldc1_emulate(uint32 instr, uint32 pc);
	virtual void sdc1_emulate(uint32 instr, uint32 pc);

	friend std::ostream& operator<<(std::ostream&, const SpeculativeCPU&);

private:
	FPU *fpu;		//float processor unit for prophet simulator

	uint32 m_PPC;		//the start address of p-slice
	//uint32 m_EPC;		//the end address of p-slice
	//bool m_ValideEPC;

	RegCacheItem m_RegCache[CPU_REG_NUMBER];
	FregCacheItem m_FregCache[FPU_REG_NUMBER];
	MemCache m_L1Cache;

	SPECULATIVE_STATE m_State;
	SpeculativeCPU *m_ParentThread;
	uint32 m_ThreadVersion;

	bool m_SwapMark;
	bool m_ExcMark;
	bool m_RegDumpMark;
	bool m_NeedVerify;
	uint32 m_ID;
	static bool m_StackLocked;	//indicating the stack was locked or not
	static uint32 m_LockID;
	static uint32 m_CPUNum;

	static const uint32 m_MemCacheSize;
};

//help macros used to operate the L1cache entry, parameter p is a pointer to a specific entry
//in these macros high mean high address, low means low address
#define Valide0(p) ((p)->m_Entry.m_IsValide[0])
#define Valide1(p) ((p)->m_Entry.m_IsValide[1])
#define Valide2(p) ((p)->m_Entry.m_IsValide[2])
#define Valide3(p) ((p)->m_Entry.m_IsValide[3])
#define IsValide0(p) ((p) && Valide0(p))
#define IsValide1(p) ((p) && Valide1(p))
#define IsValide2(p) ((p) && Valide2(p))
#define IsValide3(p) ((p) && Valide3(p))
#define IsValideLowhalfWord(p) (IsValide0(p) && IsValide1(p))
#define IsValideHighhalfWord(p) (IsValide2(p) && IsValide3(p))
#define IsValideWord(p) (IsValideLowhalfWord(p) && IsValideHighhalfWord(p))
#define IsValideSomeLowhalf(p) (IsValide0(p) || IsValide1(p))
#define IsValideSomeHighhalf(p) (IsValide2(p) || IsValide3(p))
#define IsValideSome(p) (IsValideSomeLowhalf(p) || IsValideSomeHighhalf(p))

#define RemoteLoaded0(p) ((p)->m_Entry.m_IsRemoteLoaded[0])
#define RemoteLoaded1(p) ((p)->m_Entry.m_IsRemoteLoaded[1])
#define RemoteLoaded2(p) ((p)->m_Entry.m_IsRemoteLoaded[2])
#define RemoteLoaded3(p) ((p)->m_Entry.m_IsRemoteLoaded[3])
#define IsRemoteLoadedSomeHighhalf(p) (RemoteLoaded2(p) || RemoteLoaded3(p))
#define IsRemoteLoadedSomeLowhalf(p) (RemoteLoaded0(p) || RemoteLoaded1(p))
#define IsRemoteLoadedSome(p) (IsRemoteLoadedSomeLowhalf(p) || IsRemoteLoadedSomeHighhalf(p))

#define Modified0(p) ((p)->m_Entry.m_IsModified[0])
#define Modified1(p) ((p)->m_Entry.m_IsModified[1])
#define Modified2(p) ((p)->m_Entry.m_IsModified[2])
#define Modified3(p) ((p)->m_Entry.m_IsModified[3])
#define IsModified0(p) ((p) && Modified0(p))
#define IsModified1(p) ((p) && Modified1(p))
#define IsModified2(p) ((p) && Modified2(p))
#define IsModified3(p) ((p) && Modified3(p))
#define IsModifiedHighhalfWord(p) (IsModified2(p) && IsModified3(p))
#define IsModifiedLowhalfWord(p) (IsModified0(p) && IsModified1(p))
#define IsModifiedWord(p) (IsModifiedLowhalfWord(p) && IsModifiedHighhalfWord(p))
#define IsModifiedSome(p) (IsModified0(p) || IsModified1(p) || IsModified2(p) || IsModified3(p))

#define IsOld(p) ((p)->m_Entry.m_IsOld)
#define Version(p) ((p)->m_Entry.m_Version)
#define Address(p) ((p)->m_Entry.m_Address)
#define Value(p) ((p)->m_Entry.m_Value)

#define Round4(a) ((a) / 4 * 4)

#define LowhalfOfWord(value) (*((uint16*)((void*)(&(value)))))
#define HighhalfOfWord(value) (*((uint16*)((char*)(&(value)) + 2)))
#define Byte0OfWord(value) (*((uint8*)((char*)(&(value)))))
#define Byte1OfWord(value) (*((uint8*)((char*)(&(value)) + 1)))
#define Byte2OfWord(value) (*((uint8*)((char*)(&(value)) + 2)))
#define Byte3OfWord(value) (*((uint8*)((char*)(&(value)) + 3)))
#define Word(startbyte) (*((uint32*)(&(startbyte))))
#define HalfWord(startbyte) LowhalfOfWord(startbyte)

#define SwapHalfWord(value) (((((value) & 0x00ff) << 8)) | (((value) >> 8) & 0x00ff))
#define SwapWord(value) ((((value) << 24) & 0xff000000) | (((value) << 8) & 0x00ff0000) | \
			(((value) >> 8) & 0x0000ff00) | (((value) >> 24) & 0x000000ff))

#endif