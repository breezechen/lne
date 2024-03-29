/*
 *  Copyright (C) 2011  Vietor Liu <vietor.liu@gmail.com>
 *
 *  This file is part of LNE.
 *  LNE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Lesser Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LNE is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with LNE.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LNE_THREAD_H
#define LNE_THREAD_H

#include "BaseObject.h"
#include "TimeValue.h"

LNE_NAMESPACE_BEGIN

class LNE_Export Runnable : public Abstract
{
public:
	virtual void Service(void) = 0;
	virtual void Terminate(void) = 0;
};

class LNE_Export Thread: public Available
{
public:
	static Thread *NewInstance(Runnable *run);
	void Release(void);
	LNE_UINT Active(void);
	LNE_UINT Wait(void);
	static void Sleep(LNE_UINT64 msec);
	static void Sleep(const TimeValue &tv);

private:
	Thread(Runnable *run);
	~Thread(void);

	Runnable *run_;
#if defined(LNE_WIN32)
	HANDLE handle_;
	static DWORD WINAPI ThreadRoutine(LPVOID parameter);
#else
	pthread_t thread_;
	static void *ThreadRoutine(void *parameter);
#endif
};

LNE_INLINE void
Thread::Sleep(LNE_UINT64 msec)
{
#if defined(LNE_WIN32)
	::Sleep((DWORD)msec);
#else
	::usleep(msec * 1000);
#endif
}

LNE_INLINE void
Thread::Sleep(const TimeValue &tv)
{
#if defined(LNE_WIN32)
	::Sleep((DWORD)tv.ToMillisecond());
#else
	::usleep(tv.ToMicroseconds());
#endif
}

LNE_NAMESPACE_END

#endif
