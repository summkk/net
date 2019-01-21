#include "pch.h"
#include "Global.h"
#include "TCPRdtSender.h"
#include <cstring>
#include <cstdio>

int TCPRdtSender::WindowSize = 4;
int TCPRdtSender::bits = 3;
int TCPRdtSender::MaxSeq = 8;

TCPRdtSender::TCPRdtSender()
{
	base = 0;
	nextSeqNum = 0;
	for(int i = 0;i < 8;i++)
		count[i] = 0;//无冗余
	if ((base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;
	else
		full = false;

}
TCPRdtSender:: ~TCPRdtSender()
{

}
bool TCPRdtSender::send(Message &message)//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
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
	if (base == nextSeqNum)//只有一个定时器
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, tmp);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方

	nextSeqNum = (nextSeqNum + 1) % MaxSeq;
	if ((base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;

	return true;

}
void TCPRdtSender::receive(Packet &ackPkt)			//接受确认Ack，将被NetworkService调用	
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printf("期望收到的ack范围%d\t%d\n", base, (base + WindowSize) % 8);
	if ((ackPkt.acknum - base + 8) % 8 < WindowSize) {//
		//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号  累计确认
		if (checkSum == ackPkt.checksum) {
			printf("old     base:%d nextSeqNum:%d\n", base, nextSeqNum);
			for (int i = 0; i < WindowSize; i++) {
				count[(base - i + 8) % 8] = 0;//acknum之前累积的窗口内的ack全部清零
			}
			base = (ackPkt.acknum + 1) % 8;//已经收到acknum以前的所有分组
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
			printf("new     base:%d nextSeqNum:%d\n", base, nextSeqNum);
		}
				
	}
	else {
		count[ackPkt.acknum]++;
			if (count[ackPkt.acknum] == 3) {
				printf("\n快速重传choose\n");
				pns->sendToNetworkLayer(RECEIVER, packetData[(ackPkt.acknum + 1) % 8]);
				printf("\n");
			}
	}
}
void TCPRdtSender::timeoutHandler(int seqNum)	//Timeout handler，将被NetworkService调用
{
	printf("\n");
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", packetData[base]);
	printf("\n");
	pns->stopTimer(SENDER, 0);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, packetData[base]);//仅重传最小未被确认的包
}
bool TCPRdtSender::getWaitingState()
{
	return full;
}