#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib,"ws2_32.lib")

//using namespace std;

class server {
public:
	SOCKET createUDPSocket(int portNumber);
	SOCKET createSocket(int port);
};