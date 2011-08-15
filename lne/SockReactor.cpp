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

#include "SockReactor.h"
#include "Thread.h" // for Sleep

LNE_NAMESPACE_USING

SockReactor::SockReactor(LNE_UINT workers, LNE_UINT idle_timeout)
	: idle_timeout_(idle_timeout), eventer_lock_(true)
{
	eventer_circle_ = NULL;
	threads_ = NULL;
	thread_workers_ = 0;
	exit_request_ = false;
	set_available(true);
	//create poller
#if defined(LNE_WIN32)
	poller_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
#elif defined(LNE_LINUX)
	poller_ = epoll_create(1);
#elif defined(LNE_FREEBSD)
	poller_ = kqueue();
#endif
	if(poller_ == INVALID_POLLER)
		set_available(false);
	//create thread pool
	if(IsAvailable()) {
#if defined(LNE_WIN32)
		threads_ = static_cast<HANDLE *>(malloc(sizeof(HANDLE) * workers + 1));
#else
		threads_ = static_cast<pthread_t *>(malloc(sizeof(pthread_t) * workers + 1));
#endif
		if(threads_ == NULL)
			set_available(false);
	}
	if(IsAvailable()) {
#if defined(LNE_WIN32)
		threads_[0] = CreateThread(NULL, 0, ThreadTimer, this, 0, NULL);
		if(threads_[0] != NULL)
#else
		if(pthread_create(&threads_[0], NULL, ThreadTimer, this) == 0)
#endif
			++thread_workers_;
		if(thread_workers_ > 0) {
			for(LNE_UINT i = 1; i <= workers; ++i) {
#if defined(LNE_WIN32)
				threads_[i] = CreateThread(NULL, 0, ThreadService, this, 0, NULL);
				if(threads_[i] == NULL)
#else
				if(pthread_create(&threads_[i], NULL, ThreadService, this) != 0)
#endif
					break;
				++thread_workers_;
			}
		}
		if(thread_workers_ <= workers)
			set_available(false);
	}
}

SockReactor::~SockReactor(void)
{
	exit_request_ = true;
	for(LNE_UINT i = 0; i < thread_workers_; ++i) {
#if defined(LNE_WIN32)
		WaitForSingleObject(threads_[i], INFINITE);
		CloseHandle(threads_[i]);
#else
		pthread_join(threads_[i], NULL);
#endif
	}
	if(poller_ != INVALID_POLLER)
		closepoller(poller_);
	if(threads_)
		free(threads_);
}

SockReactor *SockReactor::NewInstance(LNE_UINT workers, LNE_UINT idle_timeout)
{
	LNE_ASSERT_RETURN(workers > 0 && idle_timeout > 0, NULL);
	SockReactor *retval = NULL;
	try {
		if(idle_timeout < TIMER_INTERVAL)
			idle_timeout = TIMER_INTERVAL;
		retval = new SockReactor(workers, idle_timeout);
		if(retval) {
			if(!retval->IsAvailable()) {
				delete retval;
				retval = NULL;
			}
		}
	} catch(std::bad_alloc) {
	}
	return retval;
}

void SockReactor::ObjectDestroy(void)
{
	delete this;
}

POLLER SockReactor::Handle(void)
{
	return poller_;
}

void SockReactor::Bind(SockEventer *eventer)
{
	if(!eventer->HandleBind(this)) {
		eventer->HandleTerminate();
		return;
	}
	eventer_lock_.Lock();
	if(eventer_circle_ == NULL) {
		eventer->set_next(eventer);
		eventer->set_prev(eventer);
		eventer_circle_ = eventer;
	} else {
		eventer->set_next(eventer_circle_);
		eventer->set_prev(eventer_circle_->get_prev());
		eventer->get_prev()->set_next(eventer);
		eventer->get_next()->set_prev(eventer);
		eventer_circle_ = eventer;
	}
	eventer->set_active(time(NULL));
	eventer_lock_.Unlock();
}

void SockReactor::UnBind(SockEventer *eventer)
{
	LNE_ASSERT_RETURN_VOID(eventer->get_prev() != NULL && eventer->get_next() != NULL);
	eventer_lock_.Lock();
	if(eventer->get_prev() == eventer) {
		// circle empty
		LNE_ASSERT_IF(eventer == eventer_circle_) {
			eventer_circle_ = NULL;
		}
	} else {
		SockEventer *prev = eventer->get_prev();
		prev->set_next(eventer->get_next());
		prev->get_next()->set_prev(prev);
		if(eventer_circle_ == eventer)
			eventer_circle_ = prev;
		// clean save
		eventer->set_prev(NULL);
		eventer->set_next(NULL);
	}
	eventer_lock_.Unlock();
}

#if defined(LNE_WIN32)

DWORD WINAPI SockReactor::ThreadTimer(LPVOID parameter)
{
	((SockReactor *)parameter)->Timer();
	return 0;
}

DWORD WINAPI SockReactor::ThreadService(LPVOID parameter)
{
	((SockReactor *)parameter)->Service();
	return 0;
}

#else

void *SockReactor::ThreadTimer(void *parameter)
{
	((SockReactor *)parameter)->Timer();
	return (void *)0;
}

void *SockReactor::ThreadService(void *parameter)
{
	((SockReactor *)parameter)->Service();
	return (void *)0;
}

#endif

void SockReactor::Timer(void)
{
	SockEventer *next;
	time_t current, lasttime;
	time(&lasttime);
	do {
		if(time(&current) - lasttime > TIMER_INTERVAL) {
			eventer_lock_.Lock();
			if(eventer_circle_) {
				next = eventer_circle_;
				do {
					if(next->IdleTimeout() && time(&current) - next->get_active() > idle_timeout_) {
						next->set_active(current);
						next->HandleIdleTimeout();
					}
					next = next->get_next();
				} while(next != eventer_circle_);
			}
			eventer_lock_.Unlock();
			time(&lasttime);
		}
		if(!exit_request_)
			Thread::Sleep(TIMER_INTERVAL);
	} while(!exit_request_);
}

void SockReactor::Service(void)
{
#if defined(LNE_WIN32)
	DWORD bytes;
	ULONG_PTR key;
	SockEventer::IOCP_OVERLAPPED *overlap;
	do {
		if(GetQueuedCompletionStatus(poller_, &bytes, &key, reinterpret_cast<LPOVERLAPPED *>(&overlap), TIMER_INTERVAL)) {
			overlap->owner->set_active(time(NULL));
			if(overlap->type == SockEventer::IOCP_READ)
				overlap->owner->HandleRead();
			else if(overlap->type == SockEventer::IOCP_WRITE)
				overlap->owner->HandleWrite();
			else
				overlap->owner->HandleShutdown();
		}
	} while(!exit_request_);
#elif defined(LNE_LINUX)
	int rc;
	SockEventer *client;
	struct epoll_event event;
	do {
		rc = epoll_wait(poller_, &event, 1, TIMER_INTERVAL);
		if(rc > 0) {
			client = reinterpret_cast<SockEventer *>(event.data.ptr);
			client->set_active(time(NULL));
			if(event.events & EPOLLIN)
				client->HandleRead();
			if(event.events & EPOLLOUT)
				client->HandleWrite();
			if(event.events & (EPOLLERR | EPOLLHUP))
				client->HandleShutdown();
		}
	} while(!exit_request_);
#elif defined(LNE_FREEBSD)
	int rc;
	SockEventer *client;
	struct timespec timeout;
	struct kevent event, kev;
	timeout.tv_sec = 0;
	timeout.tv_nsec = TIMER_INTERVAL * 1000000;
	do {
		rc = kevent(poller_, NULL, 0, &event, 1, &timeout);
		if(rc > 0) {
			client = reinterpret_cast<SockEventer *>(event.udata);
			client->set_active(time(NULL));
			if(event.flags & (EV_EOF | EV_ERROR)) {
				EV_SET(&kev, event.ident, event.filter, EV_DELETE, 0, 0, NULL);
				kevent(poller_, &kev, 1, NULL, 0, NULL);
				client->HandleShutdown();
			} else {
				if(event.filter == EVFILT_READ)
					client->HandleRead();
				else if(event.filter == EVFILT_WRITE)
					client->HandleWrite();
			}
		}
	} while(!exit_request_);
#endif
}