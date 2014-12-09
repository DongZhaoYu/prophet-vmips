/* Main driver program for Prophet.
   Copyright2009 DongZhaoyu, GaoBing.

This file is part of Prophet. Here we implement the unmapped IO interface.

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

#include "prophetconsole.h"
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xlib.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Text.h>
#include <X11/keysym.h>
#include <cstring>
#include <iostream>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define TEXTHEIGHT 20
#define TEXTWIDTH 10
#define IO_BUFFER 1024
#define PROPHET_CONSOLE	"Prophet Console"
#define CONSOLE_INPUT SIGUSR1

namespace ProphetConsole
{
	Widget toplevel, shell, pane, console;
	XtAppContext appcontext;
	Arg args[10];
	Cardinal n;
	XtInputId inputid;
	XtSignalId signalid;

	/*subthread and its pipe*/
	pid_t pid;
	int fd[2];
	int fd_in[2];

	bool inited = false;

	using namespace std;

	void output(char *buf, int start, int len)
	{
		XawTextBlock textblock;
		XawTextPosition ip = XawTextGetInsertionPoint(console);
		textblock.firstPos = start;
		textblock.length = len;
		textblock.ptr = buf;
		textblock.format = FMT8BIT;
		XawTextReplace(console, ip, ip, &textblock);
		XawTextSetInsertionPoint(console, XawTextGetInsertionPoint(console) + textblock.length);
		XtPopup(shell, XtGrabNone);

		while(XtAppPending(appcontext))	{
			XEvent event;
			XtAppNextEvent(appcontext, &event);
			XtDispatchEvent(&event);
		}
	}

	int input(char *buf, int len)
	{
		char buffer[11];
		KeySym key;
		XComposeStatus compose;
		XEvent event;
		char *ptr;

		ptr = buf;

		while (1 < len) {		/* Reserve space for null */
			XtAppNextEvent (appcontext, &event);
			if (event.type == KeyPress) {
				int chars = XLookupString (&event.xkey, buffer, 10, &key, &compose);
				if ((key == XK_Return) || (key == XK_KP_Enter))	{
					*ptr++ = '\n';

					output("\n", 0, 1);
					break;
				}
				else if (*buffer == 3) /* ^C */
					XtDispatchEvent (&event);
				else if (chars > 0) {
					/* Event is a character, not a modifier key */
					int n = (chars < len - 1 ? chars : len - 1);

					strncpy (ptr, buffer, n);
					ptr += n;
					len -= n;

					buffer[chars] = '\0';
					output(buffer, 0, chars);
				}
			}else
				XtDispatchEvent (&event);
		}

		if (0 < len)
			*ptr = '\0';
	}

	static void ReadCallBack(XtPointer clientdata, XtSignalId *psignalid)
	{
		char buffer[IO_BUFFER];

		if(*psignalid == signalid) {
			input(buffer, IO_BUFFER);
			write(fd_in[1], buffer, std::strlen(buffer));
		}
	}

	static void SignalCallBack(int sig)
	{
		if(sig == CONSOLE_INPUT) {
			XtNoticeSignal(signalid);
		}
	}

	static void InputCallBack(XtPointer clientdata, int *psource, XtInputId *pinputid)
	{
		int n;
		char buffer[IO_BUFFER];

		if(*pinputid == inputid && *psource == fd[0]) {
			n = read(fd[0], buffer, IO_BUFFER);
			buffer[n] = '\0';

			output(buffer, 0, n);
		}
	}

	int Init(int argc, char *argv[])
	{
		struct sigaction act, oact;
		char finish[10];
		
		std::strcpy(finish, "finished");
		if(pipe(fd) < 0) {
			cout<<"pipe error!"<<endl;
			return 1;
		}
		if(pipe(fd_in) < 0) {
			cout<<"pipe error 2!"<<endl;
			return 1;
		}

		if((pid = fork()) < 0) {
			cout<<"fork error!"<<endl;
			return 1;
		}

		if(pid == 0){
			/*create console*/
			toplevel = XtAppInitialize(&appcontext, PROPHET_CONSOLE, NULL, ZERO,
			     				&argc, argv, NULL, NULL, ZERO);
			/*add input*/
			inputid = XtAppAddInput(appcontext, fd[0], (XtPointer)XtInputReadMask, InputCallBack, NULL);
			/*add signal*/
			signalid = XtAppAddSignal(appcontext, ReadCallBack, NULL);

			shell = XtCreatePopupShell(PROPHET_CONSOLE, topLevelShellWidgetClass, toplevel,
							NULL, ZERO);
			pane = XtCreateManagedWidget(PROPHET_CONSOLE, panedWidgetClass, shell, NULL, ZERO);
			n = 0;
			XtSetArg(args[n], XtNeditType, XawtextAppend); n++;
			XtSetArg(args[n], XtNscrollVertical, XawtextScrollWhenNeeded); n++;
			XtSetArg(args[n], XtNpreferredPaneSize, TEXTHEIGHT * 24); n++;
			XtSetArg(args[n], XtNwidth, TEXTWIDTH * 80); n++;
			console = XtCreateManagedWidget(PROPHET_CONSOLE, asciiTextWidgetClass, pane, args, n);
			XawTextEnableRedisplay(console);
			XtPopup(shell, XtGrabNone);

			act.sa_handler = SignalCallBack;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;
			sigaction(CONSOLE_INPUT, &act, &oact);
			write(fd_in[1], finish, std::strlen(finish));

			XtAppMainLoop(appcontext);

			XtDestroyWidget(console);
			XtDestroyWidget(pane);
			XtDestroyWidget(shell);
			XtDestroyWidget(toplevel);
			XtDestroyApplicationContext(appcontext);
			exit(0);
		}
		read(fd_in[0], finish, 10);		//we read pipe just for synchronization
		inited = true;
		return 0;
	}

	void Write(const char* format, ...)
	{
		if(!inited) {
			cout<<"you must initialize the console first!"<<endl;
			return;
		}

		char buf[IO_BUFFER];
		va_list args;

		assert(format != NULL);
		va_start(args, format);
		vsprintf(buf, format, args);
		write(fd[1], buf, std::strlen(buf));
		write(fd[0], "", 0);				//make the pipe to flush, is right?
	}

	void Read(char* buffer, int buflen)
	{
		if(!inited) {
			cout<<"you must initialize the console first!"<<endl;
			return;
		}
		kill(pid, CONSOLE_INPUT);			//signal console to read input
		int n = read(fd_in[0], buffer, buflen - 1);	//read console input
		buffer[n] = '\0';
	}
}