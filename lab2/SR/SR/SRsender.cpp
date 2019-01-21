#include "pch.h"
#include "Global.h"
#include "SRRdtSender.h"
#include <cstring>

int SRRdtSender::WindowSize = 4;
int SRRdtSender::bits = 3;
int SRRdtSender::MaxSeq = 8;

SRRdtSender::SRRdtSender()
{
	send_base = 0;
	nextSeqNum = 0;
	lastAck = 0;
	for (int i = 0; i < 4; i++)
		flag[i] = 0;
	if ((send_base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;
	else
		full = false;
}
SRRdtSender:: ~SRRdtSender()
{

}
bool SRRdtSender::send(Message &message)//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
{
	if (full) {
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
	pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, tmp);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方

	printf("base:%d nextSeqNum:%d\n", send_base, nextSeqNum);
	nextSeqNum = (nextSeqNum + 1) % 8;
	if ((send_base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;

	return true;
}
void SRRdtSender::receive(Packet &ackPkt)			//接受确认Ack，将被NetworkService调用	
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printf("base:%d\t flag:", send_base);
	for (int i = 0; i < 4; i++)
		printf("%d\t", flag[i]);
	int nth = (ackPkt.acknum - send_base + 8) % 8;
	printf("nth_send:%d\n", nth);
	if (checkSum == ackPkt.checksum && (nth < WindowSize)) {
		//如果校验和正确，并且确认序号在范围内  累计确认

		pns->stopTimer(SENDER, ackPkt.acknum);//关闭它的定时器
		
		flag[nth] = 1;//标记为已接受
		int i = 0;
		for (; i < 4; i++) {
			if (flag[i] == 0) {
				break;
			}		
		}
		lastAck = (send_base + i) % 8;
		
		if (send_base == ackPkt.acknum) {
			send_base = lastAck;//窗口滑动
			printf("full:%d\n",full);
			full = false;
			int k = 0;
			for (int j = i; j < 4; j++) {
				flag[k] = flag[j];
				k++;
			}
			for (; k < 4; k++)
				flag[k] = 0;
		}
		printf("base:%d\t flag:", send_base);
		for (int i = 0; i < 4; i++)
			printf("%d\t", flag[i]);
		printf("nextSeqNum:%d\t lastack:%d\t", nextSeqNum, lastAck);
		printf("\n");
		pUtils->printPacket("发送方正确收到确认", ackPkt);
		printf("\n");
		
	}
	else {//否则可能还没有包已发送，重发了旧的包
		printf("\n");
		pUtils->printPacket("发送方没有正确收到确认", ackPkt);
		printf("\n");
		if (send_base != nextSeqNum) {
			pns->sendToNetworkLayer(RECEIVER, packetData[send_base]);//?
			pns->stopTimer(SENDER, send_base);
			pns->startTimer(SENDER, Configuration::TIME_OUT, send_base);//重启定时器
		}
		
	}
}
void SRRdtSender::timeoutHandler(int seqNum)	//Timeout handler，将被NetworkService调用
{
	printf("\n");
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", packetData[seqNum]);
	printf("\n");
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, packetData[seqNum]);
}
bool SRRdtSender::getWaitingState()
{
	return full;
}