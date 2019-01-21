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
bool GBNRdtsender::send(Message &message)//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
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
	if(base == nextSeqNum)//ֻ��һ����ʱ��
		pns->startTimer(SENDER, Configuration::TIME_OUT,0);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, tmp);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�

	nextSeqNum = (nextSeqNum + 1) % MaxSeq;
	if ((base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;

	return true;

}
void GBNRdtsender::receive(Packet &ackPkt)			//����ȷ��Ack������NetworkService����	
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printf("�����յ���ack��Χ%d\t%d\n", base, (base + WindowSize) % 8);
	if (checkSum == ackPkt.checksum && ((ackPkt.acknum - base + 8) % 8 < WindowSize)) {//
		//���У�����ȷ������ȷ�����=���ͷ��ѷ��Ͳ��ȴ�ȷ�ϵ����ݰ����  �ۼ�ȷ��
		printf("old     base:%d nextSeqNum:%d\n", base, nextSeqNum);
		base = (ackPkt.acknum + 1)% 8;
		
		printf("new     base:%d nextSeqNum:%d\n", base, nextSeqNum);
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
		
	}
}
void GBNRdtsender::timeoutHandler(int seqNum)	//Timeout handler������NetworkService����
{
	printf("\n");
	int i = (nextSeqNum - base + 8) % 8;
	int tmp = base;
	pns->stopTimer(SENDER, 0);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);			//�����������ͷ���ʱ��
	while (i--) {//ȫ���ط�
		pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", packetData[tmp]);
		pns->sendToNetworkLayer(RECEIVER, packetData[tmp]);
		printf("\n");
		tmp = (tmp + 1) % 8;
	}	
}
bool GBNRdtsender::getWaitingState()
{
	return full;
}