#pragma once
#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#define PACK_SIZE 65500 //udp pack size; note that OSX limits < 8100 bytes
#define ENCODE_QUALITY 80
using namespace std;
using namespace cv;
class sock {
public:
	int serverLength;
	sockaddr_in server;
	SOCKET in;
	char buf[65540];

	//sock() {
	//	WSADATA data;

	//	WORD version = MAKEWORD(2, 2);

	//	// Start WinSock
	//	int wsOk = WSAStartup(version, &data);
	//	if (wsOk != 0)
	//	{
	//		std::cout << "Can't start Winsock! " << wsOk;
	//		return;
	//	}
	//}


	void initUDP(int port, string clientIP);
	void initTCP(int port);

	int getClientLength();
	sockaddr_in getsockaddr_in();
	void closeSock();
	void rcv_img(Mat &img);
	bool tcp = false;
	void send_img(cv::Mat img);
};