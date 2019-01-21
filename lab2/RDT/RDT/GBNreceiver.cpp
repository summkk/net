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
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->expectNextSeq == packet.seqnum) {
		printf("\n");
		pUtils->printPacket("接收方正确收到发送方的报文", packet);
		printf("\n");
		//取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		memcpy(lastAckPkt.payload, msg.data, sizeof(msg));
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		
		printf("\n");
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		printf("\n");
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		this->expectNextSeq = (expectNextSeq + 1) % 8; //接收序号改变

	}
	else {
		if (checkSum != packet.checksum) {
			printf("\n");
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
			printf("\n");
		}
		else {
			printf("\n");
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
			printf("\n");
		}
		printf("\n");
		pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
		printf("\n");
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
	}
}
