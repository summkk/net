#include "pch.h"
#include "Global.h"
#include "GBNRdtreceiver.h"

GBNRdtReceiver::GBNRdtReceiver()
{
	Packet tmp;
	tmp.acknum = -1;
	tmp.checksum = 0;
	tmp.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		tmp.payload[i] = '.';
	}
	tmp.checksum = pUtils->calculateCheckSum(tmp);
	lastAckPkt = tmp;
	this->expectNextSeq = 0;
}

GBNRdtReceiver::~GBNRdtReceiver()
{
}

void GBNRdtReceiver::receive(Packet &packet) {
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum && this->expectNextSeq == packet.seqnum) {
		printf("\n");
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		printf("\n");
		//ȡ��Message�����ϵݽ���Ӧ�ò�
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		memcpy(lastAckPkt.payload, msg.data, sizeof(msg));
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		
		printf("\n");
		pUtils->printPacket("���շ�����ȷ�ϱ���", lastAckPkt);
		printf("\n");
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�

		this->expectNextSeq = (expectNextSeq + 1) % 8; //������Ÿı�

	}
	else {
		if (checkSum != packet.checksum) {
			printf("\n");
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
			printf("\n");
		}
		else {
			printf("\n");
			pUtils->printPacket("���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			printf("\n");
		}
		printf("\n");
		pUtils->printPacket("���շ����·����ϴε�ȷ�ϱ���", lastAckPkt);
		printf("\n");
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
	}
}
