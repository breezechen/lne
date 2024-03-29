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

#ifndef LNE_SOCKACCEPTOR_H
#define LNE_SOCKACCEPTOR_H

#include "SockPad.h"
#include "SockAddr.h"
#include "TimeValue.h"

LNE_NAMESPACE_BEGIN

class LNE_Export SockAcceptor
{
public:
	static LNE_UINT NewInstance(SockPad &skpad, const SockAddr &addr, LNE_UINT backlog = 5);
	static SockAcceptor *NewInstance(const SockAddr &addr, LNE_UINT backlog = 5, const TimeValue *tv = NULL);
	void Release(void);

	LNE_UINT Accept(SockPad &skpad);
	LNE_UINT Accept(SockPad &skpad, const TimeValue &tv);
	LNE_UINT Accept(SockPad &skpad, const TimeValue *tv);

private:
	SockAcceptor(void);
	~SockAcceptor(void);

	SockPad skpad_;
	bool use_timeout_;
	TimeValue timeout_;
};

LNE_INLINE LNE_UINT
SockAcceptor::Accept(SockPad &skpad)
{
	return Accept(skpad, use_timeout_ ? &timeout_ : NULL);
}

LNE_INLINE LNE_UINT
SockAcceptor::Accept(SockPad &skpad, const TimeValue &tv)
{
	return Accept(skpad, &tv);
}

LNE_NAMESPACE_END

#endif
