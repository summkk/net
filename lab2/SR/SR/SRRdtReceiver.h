#pragma once
#include "RdtReceiver.h"
#include <queue>

class SRRdtReceiver :public RdtReceiver
{
	/*int expectNextSeq;		// 期待收到的下一个报文序号
	Packet lastAckPkt;		//上一次发送的确认报文*/
	int rev_base;
	int accept[4];//记录已缓存的seqnum  vector?
	Packet buf[4];//缓存发过来的message
	int rev_buf[4];//缓存分组时候的rev_base，移动
	static int WindowSize;
public:
	SRRdtReceiver();
	~SRRdtReceiver();
public:
	void receive(Packet &packet); //接受报文，被NetworkService调用

};
