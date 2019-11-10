#pragma once

#include <thread>
#include <WinSock2.h>
#include<WS2tcpip.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

void Work();

string WaitRequest(SOCKET connect_socket);

void Response();

SOCKET InitSocket();

int BindSocket(SOCKET socket);

extern SOCKADDR_IN g_sin;
extern SOCKADDR_IN g_sin_from;

