#pragma once
#include "RdtReceiver.h"

class TCPRdtReceiver :public RdtReceiver
{
	int expectNextSeq;		// 期待收到的下一个报文序号
	Packet lastAckPkt;		//上次发送的确认报文
public:
	TCPRdtReceiver();
	~TCPRdtReceiver();
public:
	void receive(Packet &packet); //接受报文，被NetworkService调用

};