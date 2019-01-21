#include "pch.h"
#include "Global.h"
#include "GBNRdtsender.h"
#include <cstring>

int GBNRdtsender::WindowSize = 4;
int GBNRdtsender::bits = 3;
int GBNRdtsender::MaxSeq = 8;

GBNRdtsender::GBNRdtsender()
{
	base = 0;
	nextSeqNum = 0;
	if ((base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;
	else
		full = false;

}
GBNRdtsender:: ~GBNRdtsender()
{

}
bool GBNRdtsender::send(Message &message)//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
{
	if (full) {//windows is full
		return false;
	}
	Packet tmp;
	tmp.acknum = -1;
	tmp.seqnum = nextSeqNum;
	tmp.checksum = 0;
	memcpy(tmp.payload, message.data, sizeof(message.data));
	tmp.checksum = pUtils->calculateCheckSum(tmp);
	packetData[nextSeqNum] = tmp;//入队
	printf("\n");
	pUtils->printPacket("发送方发送报文", tmp);
	printf("\n");
	if(base == nextSeqNum)//只有一个定时器
		pns->startTimer(SENDER, Configuration::TIME_OUT,0);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, tmp);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方

	nextSeqNum = (nextSeqNum + 1) % MaxSeq;
	if ((base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;

	return true;

}
void GBNRdtsender::receive(Packet &ackPkt)			//接受确认Ack，将被NetworkService调用	
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printf("期望收到的ack范围%d\t%d\n", base, (base + WindowSize) % 8);
	if (checkSum == ackPkt.checksum && ((ackPkt.acknum - base + 8) % 8 < WindowSize)) {//
		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号  累计确认
		printf("old     base:%d nextSeqNum:%d\n", base, nextSeqNum);
		base = (ackPkt.acknum + 1)% 8;
		
		printf("new     base:%d nextSeqNum:%d\n", base, nextSeqNum);
		full = false;
		printf("\n");
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		printf("\n");
		if (base == nextSeqNum) { //全部被接收
			pns->stopTimer(SENDER, 0);		//关闭定时器
		}
		else {
			pns->stopTimer(SENDER, 0);
			pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//重启定时器
		}
		
	}
}
void GBNRdtsender::timeoutHandler(int seqNum)	//Timeout handler，将被NetworkService调用
{
	printf("\n");
	int i = (nextSeqNum - base + 8) % 8;
	int tmp = base;
	pns->stopTimer(SENDER, 0);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//重新启动发送方定时器
	while (i--) {//全部重发
		pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", packetData[tmp]);
		pns->sendToNetworkLayer(RECEIVER, packetData[tmp]);
		printf("\n");
		tmp = (tmp + 1) % 8;
	}	
}
bool GBNRdtsender::getWaitingState()
{
	return full;
}