#include "test.h"

LNE_NAMESPACE_USING

void TestObject()
{
	LNE_UINT i, v;
	ObjectList<LNE_UINT> list;
	for(int i = 0; i < 3; i++)
		list.PushFront(i + 1);
	while(list.PopFront(v) == LNERR_OK) {
		printf("%u\n", v);
	}
	DataBlock *block;
	ObjectQueue<DataBlock *> queue;
	DataBlockPool *pool = DataBlockPool::NewInstance();
	for(i = 0; i < 10000; ++i)
		queue.Append(pool->Alloc());
	i = 0;
	while(queue.Extract(block) == LNERR_OK) {
		++i;
		block->Release();
	}
	printf("data count: %u\n", i);
	pool->Release();
}