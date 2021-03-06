#include "pch.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>


#define bufsize 1000
#pragma comment(lib,"ws2_32.lib")

using namespace std;

WSADATA wsaData;
string root;
int port;
string addr;
bool flag = true;
struct Message
{
	string request;//请求报文
	string url;//url字段，路径
	string method;//请求方法 GET
	string type;//请求的对象的类型
	int id;
	SOCKET client_socket;//会话socket
	SOCKADDR_IN ConnAddrs;
	//void message_analysis();
};
void message_analysis(Message &me);
void respond(Message &me, SOCKET sConns);
void clientinfo(SOCKADDR_IN client);
DWORD WINAPI sonthread(LPVOID p);


int main()
{
	//初始化winsock
	int nRc = WSAStartup(0x0101, &wsaData);
	if (nRc) {
		return 0;//Winsock初始化错误
	}
	if (wsaData.wVersion != 0x0101) {
		//版本支持不够
		//报告错误给用户，清除Winsock，返回

		WSACleanup();
		return 0;
	}

	SOCKET tcpSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addrSrv;
	printf("请输入虚拟地址:\n");
	std::cin >> root;//    g:/junior/net
	//root = "g:/junior/net";
	printf("请输入监听端口:\n");
	std::cin >> port;
	printf("请输入监听地址:\n");
	std::cin >> addr;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // 将INADDR_ANY转换为网络字节序，调用 htonl(long型)或htons(整型)
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(5050);

	int nSockErr;

	bind(tcpSock, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)); // 第二参数要强制类型转换
	if (nRc == SOCKET_ERROR) {
		nSockErr = WSAGetLastError();//绑定出错处理
		printf("bind failed\n");
	}
	printf("bind success\n");

	nRc = listen(tcpSock, SOMAXCONN);//监听客户端请求链接
	if (nRc == SOCKET_ERROR) {
		nSockErr = WSAGetLastError();//出错处理
		printf("listen failed\n");
	}
	printf("listen success\n");

	// 客户端与用户端进行通信
	int len = sizeof(SOCKADDR);
	//char recvBuf[bufsize] = {};
	char Buf[bufsize] = {};

	DWORD messageid = 0;
	while (flag) {	// 不断等待客户端请求的到来

		SOCKADDR_IN ConnAddrs = {};//请求连接的客户端地址
		SOCKET sConns = accept(tcpSock, (SOCKADDR*)&ConnAddrs, &len);
		printf("accept\n");
		if (sConns == INVALID_SOCKET)
		{
			nSockErr = WSAGetLastError();
			continue;
		}
		else {
			Message *info = new Message;
			info->id = messageid++;
			info->client_socket = sConns;
			info->ConnAddrs = ConnAddrs;

			char recvBuf[bufsize];
			memset(recvBuf, 0, sizeof(recvBuf));

			if (recv(sConns, recvBuf, bufsize, 0) == SOCKET_ERROR) {
				printf("receive failed\n");
			}
			else if (!recvBuf[0]) {
				printf("client closed\n");
			}
			else {
				info->request = (string)recvBuf;
				DWORD threadid;
				HANDLE methread = CreateThread(NULL, 0, sonthread, (LPVOID)info, 0, &threadid);
				CloseHandle(methread);
			}
		}
	}
	closesocket(tcpSock);//关闭监听socket
	WSACleanup();
	return 0;
}

DWORD WINAPI sonthread(LPVOID p)
{
	Message &mes = *(Message *)p;
	DWORD newid = GetCurrentThreadId();
	printf("------------------thread ID:%lu message ID:%lu------------\n", newid, mes.id);
	clientinfo(mes.ConnAddrs);
	message_analysis(mes);
	respond(mes, mes.client_socket);
	closesocket(mes.client_socket);
	delete &mes;
	return 1;
}
void clientinfo(SOCKADDR_IN client)
{
	char *ip = (char*)&client.sin_addr;
	printf("client information:\tIP:");
	printf("%d", *(ip));
	for (int i = 1; i < 4; i++) {
		printf(".%d", *(ip + i));
	}
	printf("\tports:%d\n\n", htons(client.sin_port));
}
void message_analysis(Message &me)
{
	char method[5], obj[100], protocal[10];
	printf("RECIVE:\n%s\n", me.request.c_str());
	//得到方法，对象，协议类型
	sscanf(me.request.c_str(), "%s %s %s", method, obj, protocal);
	if (strcmp(obj, "/exit") == 0)
		flag = 0;
	me.method = method;
	if (me.method == "GET") {
		string tmp = string(obj);
		string type;
		me.url = root + tmp;
		int i = tmp.find(".") + 1;
		int len = tmp.length() - i;
		type = tmp.substr(i, len);

		if (type == "html") me.type = "text/html";
		else if (type == "png") me.type = "image/png";
		else if (type == "jpg") me.type = "image/jpg";
		else if (type == "ico") me.type = "x-ico";
	}
	return;
}

void respond(Message &me, SOCKET sConns)
{

	char buf[bufsize];
	string state;
	string error = "/error404.html";
	memset(buf, 0, sizeof(buf));
	ifstream file;
	if (flag == 0) {
		file.open("g:/junior/net/close.html", ios_base::in | ios_base::binary);
		state = "HTTP/1.1 200 OK\r\n";
	}
	else if (me.method == "GET") {
		file.open(me.url.c_str(), ios_base::in | ios_base::binary);
		printf("GET object file path:%s\n\n", me.url.c_str());
		if (!file.is_open()) {
			state = "HTTP/1.1 404 Not Found\r\n";
			printf("not find\n");
			file.close();
			file.open((root + error).c_str(), ios_base::in | ios_base::binary);
		}
		else
		{
			state = "HTTP/1.1 200 OK\r\n";
		}
	}

	file.seekg(0, ios_base::end);
	int size = file.tellg();

	strcpy(buf, state.c_str());
	sprintf(buf + strlen(buf), "Content-Type: %s\r\n", me.type.c_str());
	sprintf(buf + strlen(buf), "Content-Length: %d\r\n", size);
	sprintf(buf + strlen(buf), "Server: Apache/2.2.3\r\n");
	sprintf(buf + strlen(buf), "Cache-Control: no-cache\r\n");
	sprintf(buf + strlen(buf), "Connection: close\r\n\r\n");

	printf("RESPOND HEADER：\n%s", buf);

	if (send(sConns, buf, strlen(buf), 0) == SOCKET_ERROR) {
		printf("send failed\n");
	}

	file.clear();
	file.seekg(0, ios_base::beg);
	int remain = size;
	char databuf[bufsize];
	while (remain > 0) {
		int tosend = min(sizeof(databuf), remain);
		file.read(databuf, sizeof(databuf));
		if (send(sConns, databuf, tosend, 0) == SOCKET_ERROR) {
			printf("send failed\n");
		}
		remain -= sizeof(databuf);
	}
	file.close();

	return;
}
