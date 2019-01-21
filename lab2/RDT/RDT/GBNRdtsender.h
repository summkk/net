#pragma once
#include "RdtSender.h"
#include <vector>

class GBNRdtsender:public RdtSender
{
private:	
	static int MaxSeq;
	int base;//����δȷ�Ϸ�������
	int nextSeqNum;//��С��δʹ�����
	static int WindowSize;
	static int bits;
	bool full;   //��������
	
	Packet packetData[8];   //����packet
	//Packet packetData[8];

public:

	bool send(Message &message);   //����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);				//����ȷ��Ack������NetworkService����	
	void timeoutHandler(int seqNum);			//Timeout handler������NetworkService����
	bool getWaitingState();          //����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
	GBNRdtsender();
	virtual ~GBNRdtsender();

};
