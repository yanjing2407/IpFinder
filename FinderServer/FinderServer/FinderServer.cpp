// FinderServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>


#include "FinderServer.h"

#define RECV_PORT 19999
#define RESP_PORT 19998

#define MAX_BUF_LEN 64

SOCKADDR_IN g_sin;
SOCKADDR_IN g_sin_from;

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	// 启动socket api
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		std::cout << "网络初始化失败!error code：" << err << "\n";
		system("PAUSE");
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		std::cout << "版本错误\n";
		system("PAUSE");
		return -1;
	}

	std::cout << "net work initialize success!\n";

	thread go(Work);

	go.join();

	WSACleanup();
}

void Work() 
{
	std::cout << "work start!\n";

	SOCKET connect_socket = InitSocket();

	if (connect_socket == -1)
	{
		std::cout << "socket creation failed!\n";
		return;
	}

	int rescode = BindSocket(connect_socket);

	if (rescode != 0) {
		std::cout << "bind socket failed!\n";
		return;
	}

	while (true) {
		string request = WaitRequest(connect_socket); // 阻塞等待广播请求ip

		if (request.compare("where are you")) {
			Response(); // 给请求方本机ip
		}

		//Sleep(2000);
	}

	closesocket(connect_socket);
}

string WaitRequest(SOCKET connect_socket)
{
	int nAddrLen = sizeof(SOCKADDR);
	char buff[MAX_BUF_LEN] = "";

	// 接收数据
	int nSendSize = recvfrom(connect_socket, buff, MAX_BUF_LEN, 0, (SOCKADDR*)&g_sin_from, &nAddrLen);
	if (SOCKET_ERROR == nSendSize)
	{
		int err = WSAGetLastError();
		printf("recvfrom error! error code is %d\n", err);
		return "";
	}
	int maxlen = nSendSize > MAX_BUF_LEN-1 ? MAX_BUF_LEN-1 : nSendSize;

	buff[maxlen] = '\0';

	printf("Recv: %s\n", buff);


	return string(buff);
}


SOCKET InitSocket()
{
	// 创建socket
	SOCKET connect_socket;
	connect_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connect_socket)
	{
		int err = WSAGetLastError();
		printf("socket error! error code is %d\n", err);
		return -1;
	}

	return connect_socket;
}

int BindSocket(SOCKET connect_socket)
{
	// 用来绑定套接字
	g_sin.sin_family = AF_INET;
	g_sin.sin_port = htons(RECV_PORT);
	g_sin.sin_addr.s_addr = 0;

	// 用来从网络上的广播地址接收数据
	g_sin_from.sin_family = AF_INET;
	g_sin_from.sin_port = htons(RECV_PORT);
	g_sin_from.sin_addr.s_addr = INADDR_BROADCAST;

	//设置该套接字为广播类型，
	bool bOpt = true;
	setsockopt(connect_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

	// 绑定套接字
	int err = bind(connect_socket, (SOCKADDR*)&g_sin, sizeof(SOCKADDR));
	if (SOCKET_ERROR == err)
	{
		err = WSAGetLastError();
		printf("bind error! error code is %d\n", err);
		return -1;
	}

	return  0;
}

void Response()
{
	SOCKET uSocket_client;
	uSocket_client = socket(AF_INET, SOCK_DGRAM, 0);
	if (uSocket_client == INVALID_SOCKET)
	{
		cout << "create response socket failed" << endl;
		exit(EXIT_FAILURE);
	}

	char serverip[20] = { '\0' };
	inet_ntop(AF_INET, (void*)&g_sin_from.sin_addr, serverip, sizeof(serverip));

	struct sockaddr_in server;
	int server_len = sizeof(server);
	server.sin_family = AF_INET;
	server.sin_port = htons(RESP_PORT); ///server的监听端口
	//server.sin_addr.s_addr = inet_addr("192.168.100.97"); ///serve的ip
	inet_pton(AF_INET, serverip, (void*)&server.sin_addr.s_addr);

	char sendbuf[10];
	int buflen = sprintf_s(sendbuf, sizeof(sendbuf), "i am here");
	int send_len = sendto(uSocket_client, sendbuf, buflen, 0, (struct sockaddr*)& server, sizeof(server));

	closesocket(uSocket_client);
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
