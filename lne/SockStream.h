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

#ifndef LNE_SOCKSTREAM_H
#define LNE_SOCKSTREAM_H

#include "BaseObject.h"
#include "SockPad.h"
#include "SockAddr.h"

LNE_NAMESPACE_BEGIN

class LNE_Export SockStream: public Abstract
{
public:
	LNE_UINT GetSockAddr(SockAddr &addr);
	LNE_UINT GetPeerAddr(SockAddr &addr);

protected:
	SockPad skpad_;
};

LNE_NAMESPACE_END

#endif
