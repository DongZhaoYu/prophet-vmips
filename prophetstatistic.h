/* Main driver program for Prophet.
   Copyright2009 DongZhaoyu, GaoBing.

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

#ifndef _PROPHETSTATISTIC_H
#define _PROPHETSTATISTIC_H

#include "prophetxmlelement.h"
#include <list>
#include <map>
#include "types.h"

#define STAT_PATH SYSCONFDIR"/inst_cost.ini"

/*! \brief enumerate all the instruction condition for statistic
*/
typedef enum StatCondition{
	FAILED = 0,
	SUCCESS = 1
};

namespace ProphetStat
{
	/*! \brief a class to store the statistical variables
	*/
	class StatRecord
	{
	public:
		int m_TotalInst;	//the number of total instruction executed
		int m_TotalClock;	//the number of total clock
		int m_SpawnedThread;
		int m_CommitedThread;	//the number of thread successfuly executed

		//count the occurrence of squash for all reason
		int m_FailedThread;
		int m_FailOnDDV;
		int m_FailOnInstruction;
		int m_FailOnRegV;
		int m_FailOnMMV;
		int m_FailOnLowPriority;
		int m_FailOnControl;
		int m_FailOnReset;
		int m_FailOnHalt;
		int m_FailOnQuit;
		int m_FailOnException;
		int m_FailOnUnknown;

		int m_StartTime;
		int m_FinishTime;

		static int m_SeqTime;
		static int m_ParalTime;

		StatRecord();
		XmlElement* XmlNode();
	};

	/*! \brief class RecAdder wil be inherited by the classes needing statistic to add a StatRecord instance
	*/
	class RecAdder
	{
	public:
		RecAdder();
		virtual ~RecAdder();
		StatRecord* GetRec();

	protected:
		StatRecord *m_Record;
	};

	/*! \brief statistic action for a specific instruction
	*/
	typedef void (*STAT_ACTION)(StatRecord*, StatCondition, int, void*);

	/*! \brief instruction hash list item, its content is loaded from setting file
	*/
	typedef struct StatItem{
		int m_Cost;
		STAT_ACTION m_Action;
	}StatItem;

	typedef std::map<std::string, StatItem> StatMap;

	/*! \brief global variable to store the statistic settings
	*/
	extern StatMap gStatMap;

	/*! \brief indicating whether the statistic module is initialized or not
	*/
	extern bool gStatInited;

	void InitStat(const char*);
	void ReportStat();
}

/*! \brief macros for declareing the class needing statistic
*/
#define PROPHET_STAT_CLASS(name)	class name : public ProphetStat::RecAdder
#define WITH_PUBLIC_ROOT(name)		, public name
#define WITH_PRIVATE_ROOT(name)		, private name
#define WITH_PROTECTED_ROOT(name)	, protected name

/*! \brief this macros can only be used in a member function inherited from PROPHET_STAT_CLASS
* \param instname the name of the instruction
* \param condition the condition state of the execution of the instruction
* \param userdata the data needed to pass to the registered action function for \b instname	
*/
#define DO_STATISTIC(instname, condition, userdata...) \
	if(ProphetStat::gStatInited) \
	{ \
		ProphetStat::StatItem item = ProphetStat::gStatMap[instname]; \
		ProphetStat::STAT_ACTION action = item.m_Action; \
		action(RecAdder::m_Record, condition, item.m_Cost, ##userdata); \
	}NULL

/*! \brief user defined data passed to its associated actions
*/
typedef struct SpawnClientData
{
	ProphetStat::RecAdder* m_SubAdder;
}SpawnClientData;

//the valide value for the squash reason string can be "DDV", "INSTRUCTION", "V_REG_VOILATE"...
typedef struct SquashClientData
{
	char *m_Reason;
}SquashClientData;

typedef struct CpzeroClientData
{
	uint16 m_rs;
	uint16 m_funct;
}CpzeroClientData;

typedef struct CponeClientData
{
	uint16 m_rs;
	uint16 m_funct;
	uint16 m_ft;
}CponeClientData;

#endif	//_PROPHETSTATISTIC_H