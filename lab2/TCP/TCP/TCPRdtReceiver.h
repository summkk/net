#pragma once
#include "RdtReceiver.h"

class TCPRdtReceiver :public RdtReceiver
{
	int expectNextSeq;		// �ڴ��յ�����һ���������
	Packet lastAckPkt;		//�ϴη��͵�ȷ�ϱ���
public:
	TCPRdtReceiver();
	~TCPRdtReceiver();
public:
	void receive(Packet &packet); //���ܱ��ģ���NetworkService����

};