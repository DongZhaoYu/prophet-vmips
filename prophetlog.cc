/* Main driver program for Prophet.
   Copyright2008 DongZhaoyu.

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

#include "prophetlog.h"
#include <string>
#include <time.h>
#include <iostream>
#include <vector>
#include <cxxabi.h>
#include <stdlib.h>
#include <fstream>

namespace Plog
{
	//base class for writing log
	class Recorder
	{
	public:
		Recorder() : m_WantsTime(false), m_Enabled(true) {}
		virtual ~Recorder() {}
		virtual void RecordMessage(Plog::PLOG_LEVEL, const std::string&) = 0;
		void Enable(bool work) { m_Enabled = work; }
		bool IsEnabled() { return m_Enabled; }
		void WithTime(bool time) { m_WantsTime = time; }
		bool WantsTime() { return m_WantsTime; }
	private:
		bool m_Enabled;
		bool m_WantsTime;
	};

	//write log to log file
	class FileRecorder : public Recorder
	{
	public:
		FileRecorder(const char *file) : Recorder()
		{
			m_File.open(file, std::ios_base::out | std::ios_base::app);
			if(!m_File)
				perror<<"can not set log file to "<<file<<pendl;
		}
		~FileRecorder()
		{
			m_File.close();
		}
		virtual void RecordMessage(Plog::PLOG_LEVEL level, const std::string& message)
		{
			if(!IsEnabled())
				return;
			m_File<<message<<std::endl;
		}
	private:
		std::ofstream m_File;
	};

	//write log to console
	class ConsoleRecorder : public Recorder
	{
	public:
		ConsoleRecorder() : Recorder() {}
		virtual void RecordMessage(Plog::PLOG_LEVEL level, const std::string& message)
		{
			if(!IsEnabled())
				return;
			std::cerr<<message<<std::endl;
		}
	};

	void DefaultActionOnError(const std::string &message)
	{
		//can not call perror here!
		pinfo << "exit on error message : " << message << pendl;
		exit(0);	//quit the process
	}

	//a vector containing all the recorders in the system
	typedef std::vector<Recorder*> Recorders_t;
	typedef void (*FatalFunction)(const std::string&);

	//here the settings are all default values, we will load the settings from 
	//configure file soon
	class Setting
	{
	public:
		PLOG_LEVEL m_LogLevel;
		std::string m_File;
		bool m_CTime;
		bool m_FTime;
		bool m_EnableConsole;
		bool m_EnableFile;
		bool m_PrintLocation;

		Recorders_t m_Recorders;
		FatalFunction m_CrashFunction;

		static Setting* Get() 
		{
			if(m_Instance == NULL)
				m_Instance = new Setting();
			return m_Instance;
		}
	private:
		static Setting *m_Instance;
		Setting() : m_LogLevel(PLOG_LEVEL_ALL), m_File("log.txt"), 
			m_CTime(false), m_FTime(true), m_EnableConsole(true), 
			m_EnableFile(true), m_PrintLocation(true),
			m_CrashFunction(DefaultActionOnError)
		{}
	};

	Setting *Setting::m_Instance = NULL;

	std::string GetTime()
	{
		time_t now = time(NULL);
		const size_t BUFF_SIZE = 100;
		char timebuff[BUFF_SIZE];	//ignore buffer overflow
		int len = strftime(timebuff, BUFF_SIZE, "%Y-%m-%d %H:%M:%S ", 
				localtime(&now));
		return len ? timebuff : "time error";
	}
	
	std::string ClassName(const std::type_info &ti)
	{
		static size_t namesize = 100;
		static char *namebuff = new char[namesize];
		int status;

		//note:__cxa_demangle can realloc the namebuff!
		char *name = abi::__cxa_demangle(ti.name(), namebuff, 
				&namesize, &status);
		return name ? name : ti.name();
	}
	
	std::string RemovePrefix(std::string &s, const std::string &p)
	{
		std::string::size_type where = s.find(p);
		if(where == std::string::npos)
			return s;
		return std::string(s, where + p.size());
	}

	void WriteLog(PLOG_LEVEL level, const std::string &message)
	{
		Setting *s = Setting::Get();
		Recorders_t::iterator it;
		std::string messagewithtime;
		for(it = s->m_Recorders.begin(); it != s->m_Recorders.end(); it++)
		{
			Recorder *r = *it;
			if(r->WantsTime())
			{
				messagewithtime = GetTime() + message;
				r->RecordMessage(level, messagewithtime);
			}else{
				r->RecordMessage(level, message);
			}
		}
	}

	CallSite::CallSite(PLOG_LEVEL level, const char *file, int line,
		const std::type_info &cinfo, const char *function)
		: m_Level(level), m_File(file), m_Line(line), m_ClassInfo(cinfo),
		m_Function(function), m_Cached(false), m_ShouldLog(false)
		{}

	bool Log::ShouldLog(CallSite &site)
	{
		site.m_Cached = true;
		site.m_ShouldLog = false;
		if(site.m_Level >= Setting::Get()->m_LogLevel)
			site.m_ShouldLog = true;
		return site.m_ShouldLog;
	}

	std::ostringstream* Log::Out()
	{
		return new std::ostringstream();	//this will be deleted in Flush
	}

	void Log::Flush(std::ostringstream *os, const CallSite &site)
	{
		std::string message = os->str();
		delete os;	//don't forget
		std::ostringstream prefix;
		switch(site.m_Level)
		{
		case PLOG_LEVEL_DEBUG : 	prefix << "DEBUG: "; break;
		case PLOG_LEVEL_INFO: 		prefix << "INFO: "; break;
		case PLOG_LEVEL_WARNING: 	prefix << "WARNING: "; break;
		case PLOG_LEVEL_ERROR: 		prefix << "ERROR: "; break;
		default: 			prefix << "XXX: "; break;
		}
		if(Setting::Get()->m_PrintLocation)
			prefix << site.m_File << "(" << site.m_Line << ") : ";
		if(site.m_ClassInfo != typeid(NoClassInfo))
			prefix << ClassName(site.m_ClassInfo) << "::";
		prefix << site.m_Function << ": " << message;
		message = prefix.str();
		WriteLog(site.m_Level, message);

		if(site.m_Level == PLOG_LEVEL_ERROR && Setting::Get()->m_CrashFunction)
			Setting::Get()->m_CrashFunction(message);
	}

	//initialize the log system
	void InitPlog(bool filelog, bool consolelog)
	{
		if(filelog)
		{
			FileRecorder *fr = new FileRecorder(Setting::Get()->m_File.c_str());
			fr->Enable(Setting::Get()->m_EnableFile);
			fr->WithTime(Setting::Get()->m_FTime);
			Setting::Get()->m_Recorders.push_back(fr);
		}

		if(consolelog)
		{
			ConsoleRecorder *cr = new ConsoleRecorder();
			cr->Enable(Setting::Get()->m_EnableConsole);
			cr->WithTime(Setting::Get()->m_CTime);
			Setting::Get()->m_Recorders.push_back(cr);
		}
	}
}

/*
class Test
{
	PLOG_CLASS(Test);
public:
	void show()
	{
		pdebug<<"this is a test!"<<pendl;
		pinfo<<"this is a test!"<<pendl;
		pwarn<<"this is a test!"<<pendl;
		perror<<"this is a test!"<<pendl;
	}
};

int main(int argc, char *argv[])
{
	Test t;
	Plog::InitPlog();
	t.show();
	return 0;
}
*/
