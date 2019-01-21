#pragma once
#include "RdtSender.h"
#include <vector>

class SRRdtSender :public RdtSender
{
private:
	static int MaxSeq;
	int send_base;//����δȷ�Ϸ�������
	int nextSeqNum;//��С��δʹ�����
	static int WindowSize;
	static int bits;
	bool full;   //��������
	int lastAck;//δ���յ���Сδȷ�Ϸ��飬���ڻ�������λ��
	bool flag[4];//��Ǵ�������Щ�Ѿ�������

	Packet packetData[8];   //����packet

public:

	bool send(Message &message);   //����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);				//����ȷ��Ack������NetworkService����	
	void timeoutHandler(int seqNum);			//Timeout handler������NetworkService����
	bool getWaitingState();          //����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
	SRRdtSender();
	virtual ~SRRdtSender();

};
