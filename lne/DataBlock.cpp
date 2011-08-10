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

#include "DataBlock.h"
#include "DataBlockPool.h"

LNE_NAMESPACE_USING

DataBlock::DataBlock(void)
	: lock_(true), reference_count_(1)
{
	size_ = 0;
	buffer_ = NULL;
	capacity_ = 0;
	pool_ = NULL;
}

DataBlock::~DataBlock(void)
{
}

DataBlock *DataBlock::NewInstance(LNE_UINT capacity)
{
	LNE_ASSERT(capacity > 0, NULL);
	char *buffer = static_cast<char *>(malloc(sizeof(DataBlock) + capacity));
	if(buffer) {
		DataBlock *block = new(buffer)DataBlock();
		block->capacity_ = capacity;
		block->buffer_ = buffer + sizeof(DataBlock);
		return block;
	}
	return NULL;
}

DataBlock *DataBlock::AddRef(void)
{
	lock_.Lock();
	++reference_count_;
	lock_.Unlock();
	return this;
}

void DataBlock::Release(void)
{
	bool can_destroy = false;
	lock_.Lock();
	can_destroy = --reference_count_ < 1;
	lock_.Unlock();
	if(can_destroy) {
		if(pool_) {
			size_ = 0;
			reference_count_ = 1;
			pool_->Free(this);
		} else {
			this->~DataBlock();
			free(this);
		}
	}
}