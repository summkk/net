#pragma once
#include "RdtReceiver.h"
#include <queue>

using namespace std;
class GBNRdtReceiver:public RdtReceiver
{
	int expectNextSeq;		// �ڴ��յ�����һ���������
	Packet lastAckPkt;		//�ϴη��͵�ȷ�ϱ���
public:
	GBNRdtReceiver();
	~GBNRdtReceiver();
public:
	void receive(Packet &packet); //���ܱ��ģ���NetworkService����

};
