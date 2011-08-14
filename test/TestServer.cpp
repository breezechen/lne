#include "test.h"

LNE_NAMESPACE_USING

class MyHander: public SockSprayHander
{
public:
	MyHander() {
		count_ = 0;
		first_ = false;
	}
	void HandleData(SockSpray *client, DataBlock *block) {
		if(!first_) {
			first_ = true;
			SockAddr addr_sock, addr_peer;
			client->GetSockAddr(addr_sock);
			client->GetPeerAddr(addr_peer);
			printf("accept %s <= %s\n", addr_sock.get_addr_text(), addr_peer.get_addr_text());
		}
		DataBlock *blocks[2];
		LNE_UINT64 index = reinterpret_cast<LNE_UINT64>(client->get_context());
		printf("[%llu] Recv Data: %u\n", index, block->get_size());
		blocks[0] = block;
		blocks[1] = block;
		client->Send(blocks, 2);
		block->Release();
		if(++count_ > 2)
			client->Shutdown();
	}

	void HandleShutdown(SockSpray *client) {
		LNE_UINT64 index = reinterpret_cast<LNE_UINT64>(client->get_context());
		printf("[%llu] Shutdown\n", index);
		delete this;
	}
private:
	bool first_;
	LNE_UINT count_;
};

void TestServer()
{
	SockAddr addr(8080);
	SockAcceptor *acceptor = SockAcceptor::NewInstance(addr);
	if(acceptor == NULL) {
		printf("acceptor cannot create\n");
		return;
	}
	SockPoller *poller = SockPoller::NewInstance(10);
	if(poller == NULL) {
		printf("poller cannot create\n");
		acceptor->Release();
		return;
	}
	LNE_UINT count = 0;
	SockPad sock;
	SockSpray *spray;
	DataBlockPool *pool = DataBlockPool::NewInstance();
	SockSprayFactory factory(pool);
	while(acceptor->Accept(sock) == LNERR_OK) {
		if(count > 0)
			break;
		spray = factory.Alloc(sock, new MyHander(), reinterpret_cast<void *>(++count));
		if(spray) {
			if(poller->Managed(spray) != LNERR_OK)
				spray->Release();
		}
	}
	pool->Release();
	poller->Release();
	acceptor->Release();
}
