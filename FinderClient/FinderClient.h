#pragma once

#include <WinSock2.h>
#include <vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main();

void GetIp();

void ReciveIp();

int initNet();

string GetLocalIp();
