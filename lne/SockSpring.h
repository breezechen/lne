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

#ifndef LNE_SOCKSPRING_H
#define LNE_SOCKSPRING_H

#include "SockPad.h"
#include "SockEventer.h"

LNE_NAMESPACE_BEGIN

class SockSpring;
class SockSpringPool;

class LNE_Export SockSpringHandler: public Abstract
{
public:
	virtual void HandleClient(SockSpring *spring, SockPad client) = 0;
	virtual void HandleTerminate(SockSpring *spring) = 0;
};

class LNE_Export SockSpring: public SockEventer
{
	friend class SockSpringPool;
#if defined(LNE_WIN32)
	typedef struct : public IOCP_OVERLAPPED {
		SOCKET child;
		char address[sizeof(sockaddr_in6)];
	}	IOCP_OVERLAPPED_ACCEPT;
#endif
public:
	void Shutdown(void);

	// WARNING: only used for LNE
	SockSpringHandler *handler(void);

protected:
	bool IdleTimeout(void);
	void HandleRead(void);
	bool HandleBind(SockPoller *binder);
	void HandleTerminate(void);

private:
	SockSpring(SockEventerPool *pool);
	~SockSpring(void);
	void Clean(void);
	void __Shutdown(void);
	void __HandleRead(void);

	bool shutdown_already_;
	SockSpringHandler *handler_;
	SockPad skpad_;
#if defined(LNE_WIN32)
	IOCP_OVERLAPPED_ACCEPT iocp_data_;
#elif defined(LNE_LINUX)
	struct epoll_event epoll_data_;
#endif
};

class LNE_Export SockSpringPool : public SockEventerPool
{
	friend class SockSpring;
public:
	static SockSpringPool *NewInstance(LNE_UINT limit_cache = SockEventerPool::DEFAULT_LIMIT_CACHE);
	SockSpring *Alloc(SockPad skpad, SockSpringHandler *handler);

private:
	SockSpringPool(LNE_UINT limit_cache);
	~SockSpringPool(void);
};

LNE_INLINE SockSpringHandler *
SockSpring::handler(void)
{
	return handler_;
}

LNE_INLINE
SockSpringPool::SockSpringPool(LNE_UINT limit_cache)
	: SockEventerPool(limit_cache)
{
}


LNE_NAMESPACE_END

#endif
