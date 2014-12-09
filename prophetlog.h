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

#ifndef _PROPHETLOG_H
#define _PROPHETLOG_H

#include <sstream>
#include <iostream>
#include <typeinfo>
#include "types.h"

/* Logging Facility
	Prophet	logs are divided into four levels:
	pdebug 	: 	debug messages that are normally suppressed
	pinfo 	:	informationally messages that are normally shown
	pwarn	:	warning messages that signal a problem
	perror	:	error messages that signal major and unvoeralble problems

	perror will automatically crash the process after it show the error message.

	usage: If you want to show some log message in your class member function
		(normal or static), just use the PLOG_CLASS macro, this will cause 
		you log message tagged with you class name and function name.
		example:
		class Foo
		{
			PLOG_CLASS(Foo);
		public:
			void show()
			{
				pinfo<<"Foo class"<<pendl;
			}
		}
		the output will be:
		PROPHET_INFO: Foo::show(): Foo class
		NOTE: pendl MUST be called at the end of the log printing statement.
			This version is not multithread safe.
*/

namespace Plog
{
	typedef enum PLOG_LEVEL{
		PLOG_LEVEL_ALL = 0,	//show all the logs
		PLOG_LEVEL_DEBUG = 0,
		PLOG_LEVEL_INFO = 1,
		PLOG_LEVEL_WARNING = 2,
		PLOG_LEVEL_ERROR = 3,
		PLOG_LEVEL_NON = 4	//show nothing
	};

	class CallSite;

	class Log
	{
	public:
		static bool ShouldLog(CallSite&);
		static std::ostringstream* Out();
		static void Flush(std::ostringstream*, const CallSite&);
	};

	class CallSite
	{
	public:
		CallSite(PLOG_LEVEL level, const char *file, int line, const std::type_info &cinfo, const char *function);
		bool ShouldLog()
		{ return m_Cached ? m_ShouldLog : Log::ShouldLog(*this); }

	private:
		PLOG_LEVEL m_Level;
		const char *const m_File;
		const uint32 m_Line;
		const std::type_info &m_ClassInfo;
		const char *const m_Function;
		bool m_Cached;
		bool m_ShouldLog;
		
		friend class Log;
	};

	void InitPlog(bool = true, bool = true);

	class End{};
	inline std::ostream& operator<<(std::ostream &out, const End&)
		{ return out; }

	class NoClassInfo{};
}

/*
	macro for logging information
*/

#define PLOG_CLASS(c)	typedef c _PROPHET_CLASS_TO_LOG

#define plog(level) \
	do{ \
		static Plog::CallSite _log_site(level, __FILE__, __LINE__, \
		typeid(_PROPHET_CLASS_TO_LOG), __FUNCTION__); \
		if(_log_site.ShouldLog()) \
		{ \
			std::ostringstream *_out_stream = Plog::Log::Out(); \
			(*_out_stream)

#define pendl \
			Plog::End(); \
			Plog::Log::Flush(_out_stream, _log_site); \
		} \
	}while(0)

#define pdebug plog(Plog::PLOG_LEVEL_DEBUG)
#define pinfo plog(Plog::PLOG_LEVEL_INFO)
#define pwarn plog(Plog::PLOG_LEVEL_WARNING)
#define perror plog(Plog::PLOG_LEVEL_ERROR)

typedef Plog::NoClassInfo _PROPHET_CLASS_TO_LOG;

#endif
