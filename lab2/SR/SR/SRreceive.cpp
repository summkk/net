#include "pch.h"
#include "Global.h"
#include "SRRdtReceiver.h"
#include <cstring>
/*class SRRdtReceiver :public RdtReceiver
{
int rev_base;
int accept[4];//记录已缓存的seqnum  vector?
Packet buf[4];//缓存发过来的packet
public:
	SRRdtReceiver();
	~SRRdtReceiver();
public:
	void receive(Packet &packet); //接受报文，被NetworkService调用

}; */
int SRRdtReceiver::WindowSize = 4;
SRRdtReceiver::SRRdtReceiver()
{
	Packet tmp;
	tmp.acknum = -1;
	tmp.checksum = 0;
	tmp.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		tmp.payload[i] = '.';
	}
	rev_base = 0;
	for (int i = 0; i < 4; i++) {
		accept[i] = 0;
		buf[i] = tmp;
		rev_buf[i] = 0;
	}
		
	//memset(accept, 0, sizeof(accept));
}

SRRdtReceiver::~SRRdtReceiver()
{
}

void SRRdtReceiver::receive(Packet &packet) 
{
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	//如果校验和正确，同时收到报文的序号在接收方期待收到的报文序号范围内
	int nth = (packet.seqnum - rev_base + 8) % 8;
	if (checkSum == packet.checksum)
	{
		printf("\n");
		pUtils->printPacket("接收方正确收到发送方的报文", packet);
		printf("\n");

		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));//取数据

		Packet ackPkt;
		ackPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		ackPkt.seqnum = -1;
		memcpy(ackPkt.payload, msg.data, sizeof(msg));
		ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);

		if (nth < WindowSize) {
			if (accept[nth] == 0) {//缓存，若已接受，忽略
				printf("\n收到一个未接受过的报文\n");
				buf[nth] = ackPkt;
				rev_buf[nth] = rev_base;
				accept[nth] = 1;
			}
			else {

				printf("\n收到一个接收过的报文\n");
				printf("\n");
			}
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < sizeof(packet.payload); j++)
					printf("%c", buf[i].payload[j]);
				printf("\taccept:%d\trev_buf:\n",accept[i],rev_buf[i]);
			}
			for (int i = 0; i < 4; i++)
			{
				if (accept[i] && (rev_buf[i] != rev_base)) {
					int nth_new = (buf[i].seqnum - rev_base + 8) % 8;
					buf[nth_new] = buf[i];
				}
			}
			if (packet.seqnum == rev_base) {//收到rev_base
				for (int i = 0; i < WindowSize; i++) {
					if (accept[i]) {
						printf("\n发送缓存数据\n");
						Message m;
						memcpy(m.data, buf[i].payload, sizeof(buf[i].payload));
						pns->delivertoAppLayer(RECEIVER, m);
						accept[i] = 0;
						rev_base = (rev_base + 1) % 8;//发一个更新一次rev_base,不影响其他
						printf("rev_base:%d\n", rev_base);
					}
					else {
						printf("\n缓存发送完毕\n");
						break;
					}
				}
			}
		}
		else {
			printf("\nsend_base之前的重复报文\n");
		}	
		printf("\n");
		pUtils->printPacket("接收方发送确认报文", ackPkt);
		printf("\n");
		pns->sendToNetworkLayer(SENDER, ackPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
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
	}
}
