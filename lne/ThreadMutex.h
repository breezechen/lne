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

#ifndef LNE_THREADMUTEX_H
#define LNE_THREADMUTEX_H

#include "TimeValue.h"
#include "BaseObject.h"

LNE_NAMESPACE_BEGIN

class LNE_Export ThreadMutex: public Available
{
public:
	ThreadMutex(void);
	~ThreadMutex(void);
	LNE_UINT TryAcquire(void);
	LNE_UINT Acquire(void);
	LNE_UINT Acquire(const TimeValue &tv);;
	LNE_UINT Release(void);

private:
	ThreadMutex(const ThreadMutex &);
	ThreadMutex &operator=(const ThreadMutex &);

#if defined(LNE_WIN32)
	HANDLE mutex_;
#else
	pthread_mutex_t mutex_;
#endif
};

LNE_INLINE LNE_UINT
ThreadMutex::TryAcquire(void)
{
	if(!IsAvailable())
		return LNERR_NOINIT;
#if defined(LNE_WIN32)
	return WaitForSingleObject(mutex_, 0) == WAIT_OBJECT_0 ? LNERR_OK : LNERR_TIMEOUT;
#else
	return pthread_mutex_trylock(&mutex_) == 0 ? LNERR_OK : LNERR_TIMEOUT;
#endif
}

LNE_INLINE LNE_UINT
ThreadMutex::Acquire(void)
{
	if(!IsAvailable())
		return LNERR_NOINIT;
#if defined(LNE_WIN32)
	return WaitForSingleObject(mutex_, INFINITE) == WAIT_OBJECT_0 ? LNERR_OK : LNERR_UNKNOW;
#else
	return pthread_mutex_lock(&mutex_) == 0 ? LNERR_OK : LNERR_UNKNOW;
#endif
}

LNE_INLINE LNE_UINT
ThreadMutex::Release(void)
{
	if(!IsAvailable())
		return LNERR_NOINIT;
#if defined(LNE_WIN32)
	return ReleaseMutex(mutex_) ? LNERR_OK : LNERR_UNKNOW;
#else
	return pthread_mutex_unlock(&mutex_) == 0 ? LNERR_OK : LNERR_UNKNOW;
#endif
}

LNE_NAMESPACE_END

#endif
