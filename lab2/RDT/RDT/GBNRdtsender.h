#pragma once
#include "RdtSender.h"
#include <vector>

class GBNRdtsender:public RdtSender
{
private:	
	static int MaxSeq;
	int base;//最早未确认分组的序号
	int nextSeqNum;//最小的未使用序号
	static int WindowSize;
	static int bits;
	bool full;   //窗口已满
	
	Packet packetData[8];   //数据packet
	//Packet packetData[8];

public:

	bool send(Message &message);   //发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(Packet &ackPkt);				//接受确认Ack，将被NetworkService调用	
	void timeoutHandler(int seqNum);			//Timeout handler，将被NetworkService调用
	bool getWaitingState();          //返回RdtSender是否处于等待状态，如果发送方正等待确认或者发送窗口已满，返回true
	GBNRdtsender();
	virtual ~GBNRdtsender();

};
