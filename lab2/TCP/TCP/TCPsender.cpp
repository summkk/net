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
		count[i] = 0;//������
	if ((base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;
	else
		full = false;

}
TCPRdtSender:: ~TCPRdtSender()
{

}
bool TCPRdtSender::send(Message &message)//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
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
	packetData[nextSeqNum] = tmp;//���
	printf("\n");
	pUtils->printPacket("���ͷ����ͱ���", tmp);
	printf("\n");
	if (base == nextSeqNum)//ֻ��һ����ʱ��
		pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, tmp);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�

	nextSeqNum = (nextSeqNum + 1) % MaxSeq;
	if ((base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;

	return true;

}
void TCPRdtSender::receive(Packet &ackPkt)			//����ȷ��Ack������NetworkService����	
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printf("�����յ���ack��Χ%d\t%d\n", base, (base + WindowSize) % 8);
	if ((ackPkt.acknum - base + 8) % 8 < WindowSize) {//
		//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����  �ۼ�ȷ��
		if (checkSum == ackPkt.checksum) {
			printf("old     base:%d nextSeqNum:%d\n", base, nextSeqNum);
			for (int i = 0; i < WindowSize; i++) {
				count[(base - i + 8) % 8] = 0;//acknum֮ǰ�ۻ��Ĵ����ڵ�ackȫ������
			}
			base = (ackPkt.acknum + 1) % 8;//�Ѿ��յ�acknum��ǰ�����з���
			full = false;
			printf("\n");
			pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
			printf("\n");
			if (base == nextSeqNum) { //ȫ��������
				pns->stopTimer(SENDER, 0);		//�رն�ʱ��
			}
			else {
				pns->stopTimer(SENDER, 0);
				pns->startTimer(SENDER, Configuration::TIME_OUT, 0);//������ʱ��
			}			
			printf("new     base:%d nextSeqNum:%d\n", base, nextSeqNum);
		}
				
	}
	else {
		count[ackPkt.acknum]++;
			if (count[ackPkt.acknum] == 3) {
				printf("\n�����ش�choose\n");
				pns->sendToNetworkLayer(RECEIVER, packetData[(ackPkt.acknum + 1) % 8]);
				printf("\n");
			}
	}
}
void TCPRdtSender::timeoutHandler(int seqNum)	//Timeout handler������NetworkService����
{
	printf("\n");
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", packetData[base]);
	printf("\n");
	pns->stopTimer(SENDER, 0);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, packetData[base]);//���ش���Сδ��ȷ�ϵİ�
}
bool TCPRdtSender::getWaitingState()
{
	return full;
}