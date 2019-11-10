// FinderClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>
#include "FinderClient.h"
#include <atomic>
#include<WS2tcpip.h>

#define RECV_PORT   19998
#define RQUEST_PORT 19999

using namespace std;

std::atomic<bool> success(false);


int main()
{
	if (initNet() != 0)
		return -1;

	thread recive(ReciveIp);
	thread request(GetIp);

	request.join();
	recive.join();

	WSACleanup();

	system("PAUSE");
}

void ReciveIp()
{
	SOCKET uSocket_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (uSocket_server == SOCKET_ERROR)
	{
		cout << "create socket failed" << endl;
		exit(EXIT_FAILURE);
	}
	string localip = GetLocalIp();
	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));  //每个字节都用0填充
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(RECV_PORT);
	//server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, localip.c_str(), (void*)&server_addr.sin_addr.S_un.S_addr);
	bind(uSocket_server, (sockaddr*)&server_addr, sizeof(sockaddr));

	while (!success)
	{
		char buff[64];
		memset(buff, 0, sizeof(buff));

		sockaddr_in clientAddr;
		memset(&clientAddr, 0, sizeof(sockaddr_in));
		int clientAddr_len = sizeof(sockaddr);
		memset(buff, 0, sizeof(buff));
		int recv_len = recvfrom(uSocket_server, buff, sizeof(buff), 0, (sockaddr*)&clientAddr, &clientAddr_len);
		if (recv_len <= 0)
			continue;
		//相应处理

		char ip[20] = { '\0' };
		inet_ntop(AF_INET, (void*)&clientAddr.sin_addr, ip, 16);

		printf("ip:%s\n", ip);
		success = true;
	}

	closesocket(uSocket_server);
}

void GetIp()
{
	// 创建socket
	SOCKET connect_socket;
	connect_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connect_socket)
	{
		int err = WSAGetLastError();
		printf("socket error! error code is %d\n", err);
		return ;
	}

	string localIp = GetLocalIp();
	SOCKADDR_IN addrLocal = { 0 };
	addrLocal.sin_family  = AF_INET;
	//addrLocal.sin_addr.s_addr = inet_addr(localIp.c_str());
	//addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	inet_pton(AF_INET, localIp.c_str(), (void*)&addrLocal.sin_addr.s_addr);
	addrLocal.sin_port = 0; /// 0 表示由系统自动分配端口号

	if (0 != bind(connect_socket, (sockaddr*)&addrLocal, sizeof(addrLocal)))
	{
		printf("bind failed.ip=[%s] errno=[%d]\n", localIp.c_str(), WSAGetLastError());
		return;
	}

	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(RQUEST_PORT);
	sin.sin_addr.s_addr = INADDR_BROADCAST;

	bool bOpt = true;
	//设置该套接字为广播类型
	setsockopt(connect_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

	int nAddrLen = sizeof(SOCKADDR);

	char buff[64] = {0};
	while (!success)
	{
		sprintf_s(buff,sizeof(buff), "where are you?");

		// 发送数据
		int nSendSize = sendto(connect_socket, buff, strlen(buff), 0, (SOCKADDR*)&sin, nAddrLen);
		if (SOCKET_ERROR == nSendSize)
		{
			int err = WSAGetLastError();
			printf("sendto error!, error code is %d\n", err);
			return ;
		}
		printf("Send: %s\n", buff);
		Sleep(1000);
	}

}

int initNet()
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

	return 0;
}

std::string GetLocalIp()
{
	string result;
	struct addrinfo hints;
	struct addrinfo* res, * cur;
	int ret = 0;
	struct sockaddr_in* addr;
	char m_ipaddr[16];

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;     /* Allow IPv4 */
	hints.ai_flags = AI_PASSIVE;/* For wildcard IP address */
	hints.ai_protocol = 0;         /* Any protocol */
	hints.ai_socktype = SOCK_STREAM;

	char hostname[64] = { 0 };
	gethostname(hostname, sizeof(hostname));
	ret = getaddrinfo(hostname, NULL, &hints, &res);

	if (ret == -1) {
		perror("getaddrinfo");
		exit(1);
	}
	for (cur = res; cur != NULL; cur = cur->ai_next) {
		addr = (struct sockaddr_in*)cur->ai_addr;
		sprintf_s(m_ipaddr,sizeof(m_ipaddr), "%d.%d.%d.%d",
			(*addr).sin_addr.S_un.S_un_b.s_b1,
			(*addr).sin_addr.S_un.S_un_b.s_b2,
			(*addr).sin_addr.S_un.S_un_b.s_b3,
			(*addr).sin_addr.S_un.S_un_b.s_b4);
		//printf("%s\n", m_ipaddr);

		if (strncmp(m_ipaddr, "192.168.0",9) == 0) {
			result = m_ipaddr;
			break;
		}
	}
	freeaddrinfo(res);

	return result;
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
