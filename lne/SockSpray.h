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

#ifndef LNE_SOCKSPRAY_H
#define LNE_SOCKSPRAY_H

#include "DataBlock.h"
#include "SockPad.h"
#include "SockStream.h"
#include "SockEventer.h"
#include "ObjectQueue_T.h"

LNE_NAMESPACE_BEGIN

class DataBlockPool;
class SockReactor;
class SockSpray;
class SockSprayPool;

class LNE_Export SockSprayHandler: public Abstract
{
public:
	virtual void HandleData(SockSpray *client, DataBlock *block) = 0;
	virtual void HandleShutdown(SockSpray *client) = 0;
	virtual void HandleTerminate(SockSpray *client) = 0;
};

class LNE_Export SockSpray: public SockEventer, public SockStream
{
	friend class SockSprayPool;
public:
	void Send(DataBlock *block);
	void Send(DataBlock *blocks[], LNE_UINT count);
	void Shutdown(void);

	// WARNING: only used for LNE
	SockSprayHandler *handler(void);

protected:
	bool IdleTimeout(void);
	void HandleRead(void);
	void HandleWrite(void);
	void HandleShutdown(void);
	bool HandleBind(SockPoller *binder);
	void HandleTerminate(void);
	void HandleIdleTimeout(void);

private:
	SockSpray(SockEventerPool *pool);
	~SockSpray(void);
	void Clean(void);
	void __Shutdown(void);
	void __HandleRead(void);
	void __HandleWrite(void);
	void __HandleShutdown(void);
	void EnterThreadSafe(void);
	void LeaveThreadSafe(void);

	bool enable_idle_check_;
	DataBlockPool *pool_;
	LNE_UINT limit_write_cache_;
	SockSprayHandler *handler_;
	ThreadLock thread_lock_;
	LNE_UINT thread_count_;
	// for send
	struct {
		bool ready;
		bool already;
		DataBlock *cache_buf;
		LNE_UINT cache_len;
	} send_state_;
	ObjectQueue<DataBlock *> send_blocks_;
	ThreadLock send_lock_;
	// for recv
	struct {
		bool ready;
		bool already;
	} recv_state_;
	ThreadLock recv_lock_;
	// for shutdown
	struct {
		bool query;
		bool invoke;
		bool already;
	} shutdown_state_;
	ThreadLock shutdown_lock_;
#if defined(LNE_WIN32)
	struct {
		LNE_INT count;
		WSABUF buffer;
		IOCP_OVERLAPPED overlap[IOCP_ARRAY_MAX];
		LPFN_DISCONNECTEX disconnectex;
	} iocp_data_;
	ThreadLock iocp_lock_;
#elif defined(LNE_LINUX)
	struct epoll_event epoll_data_;
#elif defined(LNE_FREEBSD)
	struct {
		LNE_UINT num_eof;
	} kevent_data_;
	ThreadLock kevent_lock_;
#endif
};

class LNE_Export SockSprayPool : public SockEventerPool
{
	friend class SockSpray;
	static const LNE_UINT DEFAULT_WRITE_CACHE = 128;
public:
	static SockSprayPool *NewInstance(DataBlockPool *data_pool, LNE_UINT limit_write_cache = DEFAULT_WRITE_CACHE, LNE_UINT limit_cache = SockEventerPool::DEFAULT_LIMIT_CACHE);
	SockSpray *Alloc(SockPad skpad, SockSprayHandler *handler);

private:
	SockSprayPool(LNE_UINT limit_cache);
	~SockSprayPool(void);

	DataBlockPool *data_pool_;
	LNE_UINT limit_write_cache_;
};

LNE_INLINE SockSprayHandler *
SockSpray::handler(void)
{
	return handler_;
}

LNE_INLINE
SockSprayPool::SockSprayPool(LNE_UINT limit_cache)
	: SockEventerPool(limit_cache)
{
	data_pool_ = NULL;
	limit_write_cache_ = 0;
}


LNE_NAMESPACE_END

#endif
