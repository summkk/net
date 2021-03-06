// RDT.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <cstdlib>
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "SRRdtReceiver.h"
#include "SRRdtSender.h"

int main()
{
	//FILE *stream;
	//freopen_s(&stream, "G:\\junior\\net\\lab2\\AllSR.txt", "a+", stdout);
	RdtSender *ps = new SRRdtSender();
	RdtReceiver * pr = new SRRdtReceiver();
	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile("G:\\junior\\net\\rdt-win-student\\rdt-win-student\\input.txt");
	pns->setOutputFile("G:\\junior\\net\\rdt-win-student\\rdt-win-student\\outputSR.txt");
	pns->start();

	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
}

