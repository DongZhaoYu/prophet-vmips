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

#include "prophetstatistic.h"
#include "prophetxmldoc.h"
#include "prophetlog.h"
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

namespace ProphetStat
{
	bool gStatInited = false;
	StatMap gStatMap;
	
	StatRecord::StatRecord() : m_TotalInst(0), m_TotalClock(0),
				m_SpawnedThread(0), m_CommitedThread(0), m_FailedThread(0),
				m_StartTime(0), m_FinishTime(0), m_FailOnDDV(0), m_FailOnInstruction(0),
				m_FailOnRegV(0), m_FailOnMMV(0), m_FailOnLowPriority(0), m_FailOnControl(0),
				m_FailOnReset(0), m_FailOnHalt(0), m_FailOnQuit(0), m_FailOnException(0), m_FailOnUnknown(0)
	{}

	int StatRecord::m_SeqTime = 0;
	int StatRecord::m_ParalTime = 0;

	/*! \brief Statistic maintains all the statistic record for the cpus
	*/
	class Statistic
	{
	public:
		static Statistic* Get();
		void RegisterRec(StatRecord*);
		XmlElement* AddToXml(XmlElement*);
		XmlElement* AddStat(XmlElement*);

	private:
		Statistic();

		std::list<StatRecord*> m_RecList;
		static Statistic* m_Inst;
		int m_SpTime;		//the real time cost of the speculative execution
		int m_SeqTime;		//the real time cost of the sequential execution
	};

	Statistic* Statistic::m_Inst = NULL;

	Statistic* Statistic::Get()
	{
		if(m_Inst == NULL)
			m_Inst = new Statistic();
		return m_Inst;
	}

	Statistic::Statistic() : m_SpTime(0), m_SeqTime(0)
	{}

	void Statistic::RegisterRec(StatRecord* rec)
	{
		//here we do not pre-check the existance of the specified record, this may cause redundance
		m_RecList.push_back(rec);
	}

	XmlElement* Statistic::AddToXml(XmlElement* root)
	{
		std::list<StatRecord*>::iterator it;
		int i = 0;
		for(it = m_RecList.begin(); it != m_RecList.end(); it++)
		{
			char value[33];
			XmlElement* node = (*it)->XmlNode();
			sprintf(value, "%d", i++);
			node->addAttribute("cpu_id", std::string(value));
			root->addElement(node);
		}
		return root;
	}

	RecAdder::RecAdder()
	{
		m_Record = new StatRecord();
		Statistic::Get()->RegisterRec(m_Record);
	}

	RecAdder::~RecAdder()
	{}

	StatRecord* RecAdder::GetRec()
	{
		return m_Record;
	}

	typedef StatMap::value_type StatMapValue;

	/*! \brief setting file*/
	static char* file = STAT_PATH;
	static char program[100];
	std::string SettingVersion;

	XmlElement* StatRecord::XmlNode()
	{
		XmlElement *node = new XmlElement("statistic");
		char value[33];
		
		XmlElement *subnode = new XmlElement("total_inst");
		sprintf(value, "%d", m_TotalInst);
		subnode->setContent(value);
		node->addElement(subnode);

		subnode = new XmlElement("total_clock");
		sprintf(value, "%d", m_TotalClock);
		subnode->setContent(value);
		node->addElement(subnode);

		subnode = new XmlElement("spawned_thread");
		sprintf(value, "%d", m_SpawnedThread);
		subnode->setContent(value);
		node->addElement(subnode);

		subnode = new XmlElement("commited_thread");
		sprintf(value, "%d", m_CommitedThread);
		subnode->setContent(value);
		node->addElement(subnode);

		XmlElement *failnode = new XmlElement("failed_thread");
		sprintf(value, "%d", m_FailedThread);
		failnode->addAttribute("total", value);

		subnode = new XmlElement("OnDDV");
		sprintf(value, "%d", m_FailOnDDV);
		subnode->setContent(value);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnInstruction");
		sprintf(value, "%d", m_FailOnInstruction);
		subnode->setContent(value);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnRegisterVerification");
		sprintf(value, "%d", m_FailOnRegV);
		subnode->setContent(value);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnMemoryVerification");
		subnode->setContent(m_FailOnMMV);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnLowPriority");
		subnode->setContent(m_FailOnLowPriority);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnControlVoilate");
		subnode->setContent(m_FailOnControl);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnReset");
		subnode->setContent(m_FailOnReset);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnHalt");
		subnode->setContent(m_FailOnHalt);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnQuit");
		subnode->setContent(m_FailOnQuit);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnException");
		subnode->setContent(m_FailOnException);
		failnode->addElement(subnode);

		subnode = new XmlElement("OnUnknown");
		subnode->setContent(m_FailOnUnknown);
		failnode->addElement(subnode);

		node->addElement(failnode);
		return node;
	}

	typedef struct tstat
	{
		int m_TSpawnedThread;
		int m_TFailedThread;
		int m_TFailOnDDV;
		int m_TFailOnInstruction;
		int m_TFailOnRegV;
		int m_TFailOnMMV;
		int m_TFailOnLowPriority;
		int m_TFailOnControl;
		int m_TFailOnReset;
		int m_TFailOnHalt;
		int m_TFailOnQuit;
		int m_TFailOnException;
		int m_TFailOnUnknown;
	}tstats;

	XmlElement* Statistic::AddStat(XmlElement* node)
	{
		tstats t;
		std::memset(&t, 0, sizeof(t));
		char buffer[512];

		for(std::list<StatRecord*>::iterator it = m_RecList.begin(); it != m_RecList.end(); it++)
		{
			t.m_TSpawnedThread += (*it)->m_SpawnedThread;
			t.m_TFailedThread += (*it)->m_FailedThread;
			t.m_TFailOnDDV += (*it)->m_FailOnDDV;
			t.m_TFailOnInstruction += (*it)->m_FailOnInstruction;
			t.m_TFailOnRegV += (*it)->m_FailOnRegV;
			t.m_TFailOnMMV += (*it)->m_FailOnMMV;
			t.m_TFailOnLowPriority += (*it)->m_FailOnLowPriority;
			t.m_TFailOnControl += (*it)->m_FailOnControl;
			t.m_TFailOnReset += (*it)->m_FailOnReset;
			t.m_TFailOnHalt += (*it)->m_FailOnHalt;
			t.m_TFailOnQuit += (*it)->m_FailOnQuit;
			t.m_TFailOnException += (*it)->m_FailOnException;
			t.m_TFailOnUnknown += (*it)->m_FailOnUnknown;
		}

		XmlElement *stat = new XmlElement("total_spawned");
		stat->setContent(t.m_TSpawnedThread);
		node->addElement(stat);

		stat = new XmlElement("success_ratio");
		sprintf(buffer, "%f", double(t.m_TSpawnedThread - t.m_TFailedThread) / t.m_TSpawnedThread);
		stat->setContent(std::string(buffer));
		node->addElement(stat);

		XmlElement *fail = new XmlElement("fail_stat");
		fail->addAttribute("total_failed", t.m_TFailedThread);

		stat = new XmlElement("TOnDDV");
		stat->addAttribute("total", t.m_TFailOnDDV);
		sprintf(buffer, "%f", double(t.m_TFailOnDDV) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnInstruction");
		stat->addAttribute("total", t.m_TFailOnInstruction);
		sprintf(buffer, "%f", double(t.m_TFailOnInstruction) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnRegV");
		stat->addAttribute("total", t.m_TFailOnRegV);
		sprintf(buffer, "%f", double(t.m_TFailOnRegV) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnMMV");
		stat->addAttribute("total", t.m_TFailOnMMV);
		sprintf(buffer, "%f", double(t.m_TFailOnMMV) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnLowPriority");
		stat->addAttribute("total", t.m_TFailOnLowPriority);
		sprintf(buffer, "%f", double(t.m_TFailOnLowPriority) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnControl");
		stat->addAttribute("total", t.m_TFailOnControl);
		sprintf(buffer, "%f", double(t.m_TFailOnControl) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnReset");
		stat->addAttribute("total", t.m_TFailOnReset);
		sprintf(buffer, "%f", double(t.m_TFailOnReset) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnHalt");
		stat->addAttribute("total", t.m_TFailOnHalt);
		sprintf(buffer, "%f", double(t.m_TFailOnHalt) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnQuit");
		stat->addAttribute("total", t.m_TFailOnQuit);
		sprintf(buffer, "%f", double(t.m_TFailOnQuit) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnException");
		stat->addAttribute("total", t.m_TFailOnException);
		sprintf(buffer, "%f", double(t.m_TFailOnException) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		stat = new XmlElement("TOnUnknown");
		stat->addAttribute("total", t.m_TFailOnUnknown);
		sprintf(buffer, "%f", double(t.m_TFailOnUnknown) / t.m_TFailedThread);
		stat->setContent(std::string(buffer));
		fail->addElement(stat);

		node->addElement(fail);
		return node;
	}

	/*! \brief if a StatItem with a NULL statistic action, the default one will be invoked
	*/
	static void DefaultStatAction(StatRecord* rec, StatCondition condition, int cost, void* clientdata = NULL)
	{
		if(!gStatInited)
			return;
		rec->m_TotalInst++;
		rec->m_TotalClock += cost;
		rec->m_FinishTime += cost;
	}

	/*! \brief user defined action for specified instruction
	*/
	static void CpzeroAction(StatRecord* rec, StatCondition condition, int cost, void* clientdata = NULL)
	{
		if(!gStatInited)
			return;

		CpzeroClientData *data = (CpzeroClientData*)clientdata;
		StatItem item;
		item.m_Cost = 0;
		item.m_Action = NULL;
		if(data->m_rs > 15)
		{
			switch(data->m_funct)
			{
			case 1:		item = gStatMap["tlbr"];	break;
			case 2:		item = gStatMap["tlbwi"];	break;
			case 6:		item = gStatMap["tlbwr"];	break;
			case 8:		item = gStatMap["tlbp"];	break;
			case 16:	item = gStatMap["rfe"];		break;
			default:	NULL;				break;
			}
		} else {
			switch(data->m_rs)
			{
			case 0:		item = gStatMap["mfc0"];	break;
			case 4:		item = gStatMap["mtc0"];	break;
			case 8:		item = gStatMap["bc0x"];	break;
			default:	NULL;				break;
			}
		}

		STAT_ACTION action = item.m_Action;
		if(action)
			action(rec, condition, item.m_Cost, NULL);
	}

	static void CponeAction(StatRecord* rec, StatCondition condition, int cost, void* clientdata = NULL)
	{
		if(!gStatInited)
			return;

		CponeClientData *data = (CponeClientData*)clientdata;
		StatItem item;
		item.m_Action = NULL;
		item.m_Cost = 0;
		if(data->m_rs == 16)
		{
			switch(data->m_funct)
			{
			case 0:		item = gStatMap["adds"];	break;
			case 1:		item = gStatMap["subs"];	break;
			case 2:		item = gStatMap["muls"];	break;
			case 3:		item = gStatMap["divs"];	break;
			case 5:		item = gStatMap["abss"];	break;
			case 6:		item = gStatMap["movs"];	break;
			case 7:		item = gStatMap["negs"];	break;
			case 33:	item = gStatMap["cvtds"];	break;
			case 36:	item = gStatMap["cvtws"];	break;
			case 56:	item = gStatMap["csfs"];	break;
			case 58:	item = gStatMap["cseqs"];	break;
			case 60:	item = gStatMap["clts"];	break;
			case 62:	item = gStatMap["cles"];	break;
			default:	NULL;				break;
			}
		} else if(data->m_rs == 17) {
			switch(data->m_funct)
			{
			case 0:		item = gStatMap["addd"];	break;
			case 1:		item = gStatMap["subd"];	break;
			case 2: 	item = gStatMap["muld"];	break;
			case 3: 	item = gStatMap["divd"];	break;
			case 5: 	item = gStatMap["absd"];	break;
			case 6: 	item = gStatMap["movd"];	break;
			case 7:		item = gStatMap["negd"];	break;
			case 32:	item = gStatMap["cvtsd"];	break;
			case 36:	item = gStatMap["cvtwd"];	break;
			case 56:	item = gStatMap["csfd"];	break;
			case 58:	item = gStatMap["cseqd"];	break;
			case 60:	item = gStatMap["cltd"];	break;
			case 62: 	item = gStatMap["cled"];	break;
			default:	NULL;				break;
			}
		} else if(data->m_rs == 0) {
			item = gStatMap["mfc1"];
		} else if(data->m_rs == 4) {
			item = gStatMap["mtc1"];
		} else if(data->m_rs == 8) {
			switch(data->m_ft)
			{
			case 0:		item = gStatMap["bc1f"];	break;
			case 1:		item = gStatMap["bc1t"];	break;
			default:	NULL;				break;
			}
		} else if(data->m_rs == 20) {
			switch(data->m_funct)
			{
			case 32:	item = gStatMap["cvtsw"];	break;
			case 33:	item = gStatMap["cvtdw"];	break;
			default:	NULL;				break;
			}
		}

		STAT_ACTION action = item.m_Action;
		if(action)
			action(rec, condition, item.m_Cost, NULL);
	}

	/*! \brief user defined actions for specified event
	*/
	static void SpawnAction(StatRecord* rec, StatCondition condition, int cost, void* clientdata = NULL)
	{
		if(!gStatInited)
			return;
		rec->m_TotalInst++;
		rec->m_TotalClock += cost;

		if(condition == SUCCESS)
		{
			assert(clientdata != NULL);
			SpawnClientData* pdata = (SpawnClientData*)clientdata;
			RecAdder* sub = pdata->m_SubAdder;
			if(sub)
			{
				StatRecord* subrec = sub->GetRec();

				//here we initialize the subthread's statistic record
				subrec->m_SpawnedThread++;
				subrec->m_StartTime = rec->m_FinishTime;
				subrec->m_FinishTime = rec->m_FinishTime;
			}
		}

		rec->m_FinishTime += cost;
	}

	static void BeginAction(StatRecord* rec, StatCondition condition, int cost, void* clientdata = NULL)
	{
		if(!gStatInited)
			return;

		rec->m_SpawnedThread = 1;
	}

	static void CommitAction(StatRecord* rec, StatCondition condition, int cost, void* clientdata = NULL)
	{
		if(!gStatInited)
			return;

		rec->m_CommitedThread++;

		//update the parallel execution time
		if(rec->m_FinishTime > rec->m_ParalTime)
			rec->m_ParalTime = rec->m_FinishTime;

		//update the sequential execution time
		rec->m_SeqTime += (rec->m_FinishTime - rec->m_StartTime);

		/*! \brief updata finish time in case the subthread was squashed*/
		if(rec->m_ParalTime > rec->m_FinishTime)
			rec->m_FinishTime = rec->m_ParalTime;

		/*! updata the \b start time*/
		rec->m_StartTime = rec->m_FinishTime;

		//pinfo<<"current sequential time = "<<rec->m_SeqTime<<"\n current parallel time = "<<rec->m_ParalTime<<pendl;
	}

	static void SquashAction(StatRecord* rec, StatCondition condition, int cost, void* data = NULL)
	{
		if(!gStatInited)
			return;

		SquashClientData *clientdata = (SquashClientData*)data;
		assert(data != NULL);

		if(std::strcmp(clientdata->m_Reason, "DDV") ==0)
		{
			rec->m_FailOnDDV++;
		}else if(std::strcmp(clientdata->m_Reason, "INSTRUCTION") == 0) {
			rec->m_FailOnInstruction++;
		}else if(std::strcmp(clientdata->m_Reason, "V_REG_VOILATE") == 0) {
			rec->m_FailOnRegV++;
		}else if(std::strcmp(clientdata->m_Reason, "V_MM_VOILATE") == 0) {
			rec->m_FailOnMMV++;
		}else if(std::strcmp(clientdata->m_Reason, "LOW_PRIORITY") == 0) {
			rec->m_FailOnLowPriority++;
		}else if(std::strcmp(clientdata->m_Reason, "CONTROL_VOILATE") == 0) {
			rec->m_FailOnControl++;
		}else if(std::strcmp(clientdata->m_Reason, "RESET") ==  0) {
			rec->m_FailOnReset++;
		}else if(std::strcmp(clientdata->m_Reason, "HALT") == 0) {
			rec->m_FailOnHalt++;
		}else if(std::strcmp(clientdata->m_Reason, "QUIT") == 0) {
			rec->m_FailOnQuit++;
		}else if(std::strcmp(clientdata->m_Reason, "EXCEPTION") == 0) {
			rec->m_FailOnException++;
		}else if(std::strcmp(clientdata->m_Reason, "XXX") == 0) {
			rec->m_FailOnUnknown++;
		}
		rec->m_FailedThread++;
	}

	static void RestartAction(StatRecord* rec, StatCondition condition, int cost, void* clientdata = NULL)
	{
		if(!gStatInited)
			return;

		rec->m_StartTime = rec->m_FinishTime;
	}

	/*! \brief this is a hook function, the user can register its own actions here
	*/
	void RegisterActions()
	{
		gStatMap["spawn"].m_Action = SpawnAction;
		gStatMap["ebegin"].m_Action = BeginAction;
		gStatMap["ecommit"].m_Action = CommitAction;
		gStatMap["esquash"].m_Action = SquashAction;
		gStatMap["erestart"].m_Action = RestartAction;
		gStatMap["cpone"].m_Action = CponeAction;
		gStatMap["cpzero"].m_Action = CpzeroAction;
	}

	/*! \brief initialize the Statistic module
	*/
	void InitStat(const char* prgname)
	{
		std::fstream infile;
		infile.open(file);

		if(!file)
		{
			std::cout<<"can not open setting file!"<<std::endl;
			exit(0);
		}

		std::string name;
		int value;

		infile>>name;
		infile>>SettingVersion;
		if(name != "version")
		{
			std::cout<<"this seems not a setting file!"<<std::endl;
			exit(0);
		}

		while(!infile.eof())
		{
			infile>>name;

			if(name.empty())
				continue;

			if(name.substr(0, 2) == "//")	//comment
			{
				char comment[2048];
				infile.getline(comment, 2048);
				continue;
			}

			infile>>value;
			StatItem item;
			item.m_Cost = value;
			item.m_Action = DefaultStatAction;
			gStatMap[name] = item;
		}

		infile.close();
		RegisterActions();
		std::strcpy(program, prgname);
		gStatInited = true;
/*		
		StatMap::iterator it = gStatMap.begin();
		while(it != gStatMap.end())
		{
			std::cout << it->first << "\t" << it->second.m_Cost << std::endl;
			it++;
		}
*/
	}

	void ReportStat()
	{
		XmlElement* root = new XmlElement("statistic_report");
		time_t now = time(NULL);
		char timebuf[100];
		strftime(timebuf, 100, "%Y-%m-%d %H:%M:%S ", localtime(&now));
		root->addAttribute("report_time", std::string(timebuf));

		XmlElement* set = new XmlElement("settings");
		XmlElement* sitem = new XmlElement("setting_file");
		sitem->setContent(std::string(file));
		set->addElement(sitem);

		sitem = new XmlElement("version");
		sitem->setContent(SettingVersion);
		set->addElement(sitem);

		sitem = new XmlElement("program");
		sitem->setContent(std::string(program));
		set->addElement(sitem);
		root->addElement(set);

		Statistic::Get()->AddToXml(root);

		XmlElement* ana = new XmlElement("analysis");
		XmlElement* subana = new XmlElement("sequential_time");
		char buffer[65];
		sprintf(buffer, "%d", StatRecord::m_SeqTime);
		subana->setContent(std::string(buffer));
		ana->addElement(subana);

		subana = new XmlElement("parallel_time");
		sprintf(buffer, "%d", StatRecord::m_ParalTime);
		subana->setContent(std::string(buffer));
		ana->addElement(subana);

		subana = new XmlElement("speed_up");
		sprintf(buffer, "%f", double(StatRecord::m_SeqTime) / StatRecord::m_ParalTime);
		subana->setContent(std::string(buffer));
		ana->addElement(subana);

		Statistic::Get()->AddStat(ana);
		root->addElement(ana);

		XmlDocument* doc = new XmlDocument();
		doc->setRootElement(root);

		std::string docname(program);
		docname += "_statistic.xml";
		remove(docname.c_str());
		std::fstream xmldoc;
		xmldoc.open(docname.c_str(), std::ios::out);
		if(!xmldoc)
		{
			std::cout<<"can not open report document!"<<std::endl;
			return;
		}

		xmldoc<<doc->toString();
		xmldoc.close();
	}
}