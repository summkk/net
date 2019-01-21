#pragma once
#include "RdtReceiver.h"
#include <queue>

using namespace std;
class GBNRdtReceiver:public RdtReceiver
{
	int expectNextSeq;		// 期待收到的下一个报文序号
	Packet lastAckPkt;		//上次发送的确认报文
public:
	GBNRdtReceiver();
	~GBNRdtReceiver();
public:
	void receive(Packet &packet); //接受报文，被NetworkService调用

};
