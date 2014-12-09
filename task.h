/* Declarations to support tasks.
   Copyright 2002 Paul Twohey.

This file is part of VMIPS.

VMIPS is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

VMIPS is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with VMIPS; if not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef _TASK_H_
#define _TASK_H_

/* A Task is the unit of deferred action used by Clock. See clock.h */
class Task
{
public:
	Task() throw() { }
	virtual ~Task() throw() { }

	/* The task to be performed. */
	virtual void task() = 0;
};

/* A CancelableTask is a task that may not be needed when it comes due. This
   is useful when a task refers to another object that may be delete'd when
   task() is called. */
class CancelableTask : public Task
{
public:
	CancelableTask() throw() : needed(true) { }
	virtual ~CancelableTask() throw() { }

	/* Mark this task is no longer needed. When task() is called it will
	   not call real_task() */
	virtual void cancel() throw() { needed = false; }
	
	/* Perform real_task() if it is still needed, otherwise return. */
	virtual void task() { if (needed) real_task(); }

protected:
	/* The real task to be performed. */
	virtual void real_task() = 0;

protected:
	bool	needed;		// true iff the task is needed
};

#endif /* _TASK_H_ */
