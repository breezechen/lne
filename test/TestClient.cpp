#include "test.h"

#include <lne/SockWaves.h>
#include <lne/SockConnector.h>

LNE_NAMESPACE_USING

void TestClient()
{
	TimeValue tv(3);
	SockAddr addr("www.google.com:80");
	SockConnector *connector = SockConnector::NewInstance(addr, &tv);
	if(connector == NULL) {
		printf("connector cannot create\n");
		return;
	}
	SockPad skpad;
	if(connector->Connect(skpad) != LNERR_OK) {
		printf("connector cannot connect\n");
		connector->Release();
		return;
	}
	SockWaves *stream = SockWaves::NewInstance(skpad);
	const char *query = "GET / HTTP/1.1\r\n\r\n";
	DataBlock *block = DataBlock::NewInstance(1024 * 1024);
	strcpy(block->buffer(), query);
	block->set_size(static_cast<LNE_UINT>(strlen(block->buffer())));
	stream->Send(block);
	while(stream->Recv(block, tv) == LNERR_OK) {
		block->buffer()[block->size()] = '\0';
		puts(block->buffer());
	}
	block->Release();
	SockAddr addr_sock, addr_peer;
	stream->GetSockAddr(addr_sock);
	stream->GetPeerAddr(addr_peer);
	printf("connect %s => %s\n", addr_sock.addr_text(), addr_peer.addr_text());
	stream->Release();
	connector->Release();
}
