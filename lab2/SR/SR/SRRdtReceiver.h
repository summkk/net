#pragma once
#include "RdtReceiver.h"
#include <queue>

class SRRdtReceiver :public RdtReceiver
{
	/*int expectNextSeq;		// �ڴ��յ�����һ���������
	Packet lastAckPkt;		//��һ�η��͵�ȷ�ϱ���*/
	int rev_base;
	int accept[4];//��¼�ѻ����seqnum  vector?
	Packet buf[4];//���淢������message
	int rev_buf[4];//�������ʱ���rev_base���ƶ�
	static int WindowSize;
public:
	SRRdtReceiver();
	~SRRdtReceiver();
public:
	void receive(Packet &packet); //���ܱ��ģ���NetworkService����

};
