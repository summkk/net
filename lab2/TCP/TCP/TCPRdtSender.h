#pragma once
#include "RdtSender.h"

class TCPRdtSender :public RdtSender
{
private:
	static int MaxSeq;
	int base;//����δȷ�Ϸ�������
	int nextSeqNum;//��С��δʹ�����
	static int WindowSize;
	static int bits;
	bool full;   //��������
	int count[8];//����ack����Ŀ

	Packet packetData[8];   //����packet

public:

	bool send(Message &message);   //����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void receive(Packet &ackPkt);				//����ȷ��Ack������NetworkService����	
	void timeoutHandler(int seqNum);			//Timeout handler������NetworkService����
	bool getWaitingState();          //����RdtSender�Ƿ��ڵȴ�״̬��������ͷ����ȴ�ȷ�ϻ��߷��ʹ�������������true
	TCPRdtSender();
	virtual ~TCPRdtSender();

};
