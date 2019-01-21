#include "pch.h"
#include "Global.h"
#include "SRRdtReceiver.h"
#include <cstring>
/*class SRRdtReceiver :public RdtReceiver
{
int rev_base;
int accept[4];//��¼�ѻ����seqnum  vector?
Packet buf[4];//���淢������packet
public:
	SRRdtReceiver();
	~SRRdtReceiver();
public:
	void receive(Packet &packet); //���ܱ��ģ���NetworkService����

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
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);
	//���У�����ȷ��ͬʱ�յ����ĵ�����ڽ��շ��ڴ��յ��ı�����ŷ�Χ��
	int nth = (packet.seqnum - rev_base + 8) % 8;
	if (checkSum == packet.checksum)
	{
		printf("\n");
		pUtils->printPacket("���շ���ȷ�յ����ͷ��ı���", packet);
		printf("\n");

		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));//ȡ����

		Packet ackPkt;
		ackPkt.acknum = packet.seqnum; //ȷ����ŵ����յ��ı������
		ackPkt.seqnum = -1;
		memcpy(ackPkt.payload, msg.data, sizeof(msg));
		ackPkt.checksum = pUtils->calculateCheckSum(ackPkt);

		if (nth < WindowSize) {
			if (accept[nth] == 0) {//���棬���ѽ��ܣ�����
				printf("\n�յ�һ��δ���ܹ��ı���\n");
				buf[nth] = ackPkt;
				rev_buf[nth] = rev_base;
				accept[nth] = 1;
			}
			else {

				printf("\n�յ�һ�����չ��ı���\n");
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
			if (packet.seqnum == rev_base) {//�յ�rev_base
				for (int i = 0; i < WindowSize; i++) {
					if (accept[i]) {
						printf("\n���ͻ�������\n");
						Message m;
						memcpy(m.data, buf[i].payload, sizeof(buf[i].payload));
						pns->delivertoAppLayer(RECEIVER, m);
						accept[i] = 0;
						rev_base = (rev_base + 1) % 8;//��һ������һ��rev_base,��Ӱ������
						printf("rev_base:%d\n", rev_base);
					}
					else {
						printf("\n���淢�����\n");
						break;
					}
				}
			}
		}
		else {
			printf("\nsend_base֮ǰ���ظ�����\n");
		}	
		printf("\n");
		pUtils->printPacket("���շ�����ȷ�ϱ���", ackPkt);
		printf("\n");
		pns->sendToNetworkLayer(SENDER, ackPkt);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
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
	}
}
