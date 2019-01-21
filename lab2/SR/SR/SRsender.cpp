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
bool SRRdtSender::send(Message &message)//����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
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
	packetData[nextSeqNum] = tmp;//���
	printf("\n");
	pUtils->printPacket("���ͷ����ͱ���", tmp);
	printf("\n");
	pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);			//�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, tmp);								//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�

	printf("base:%d nextSeqNum:%d\n", send_base, nextSeqNum);
	nextSeqNum = (nextSeqNum + 1) % 8;
	if ((send_base - nextSeqNum + 8) % 8 == WindowSize)
		full = true;

	return true;
}
void SRRdtSender::receive(Packet &ackPkt)			//����ȷ��Ack������NetworkService����	
{
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	printf("base:%d\t flag:", send_base);
	for (int i = 0; i < 4; i++)
		printf("%d\t", flag[i]);
	int nth = (ackPkt.acknum - send_base + 8) % 8;
	printf("nth_send:%d\n", nth);
	if (checkSum == ackPkt.checksum && (nth < WindowSize)) {
		//���У�����ȷ������ȷ������ڷ�Χ��  �ۼ�ȷ��

		pns->stopTimer(SENDER, ackPkt.acknum);//�ر����Ķ�ʱ��
		
		flag[nth] = 1;//���Ϊ�ѽ���
		int i = 0;
		for (; i < 4; i++) {
			if (flag[i] == 0) {
				break;
			}		
		}
		lastAck = (send_base + i) % 8;
		
		if (send_base == ackPkt.acknum) {
			send_base = lastAck;//���ڻ���
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
		pUtils->printPacket("���ͷ���ȷ�յ�ȷ��", ackPkt);
		printf("\n");
		
	}
	else {//������ܻ�û�а��ѷ��ͣ��ط��˾ɵİ�
		printf("\n");
		pUtils->printPacket("���ͷ�û����ȷ�յ�ȷ��", ackPkt);
		printf("\n");
		if (send_base != nextSeqNum) {
			pns->sendToNetworkLayer(RECEIVER, packetData[send_base]);//?
			pns->stopTimer(SENDER, send_base);
			pns->startTimer(SENDER, Configuration::TIME_OUT, send_base);//������ʱ��
		}
		
	}
}
void SRRdtSender::timeoutHandler(int seqNum)	//Timeout handler������NetworkService����
{
	printf("\n");
	pUtils->printPacket("���ͷ���ʱ��ʱ�䵽���ط��ϴη��͵ı���", packetData[seqNum]);
	printf("\n");
	pns->stopTimer(SENDER, seqNum);										//���ȹرն�ʱ��
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//�����������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, packetData[seqNum]);
}
bool SRRdtSender::getWaitingState()
{
	return full;
}