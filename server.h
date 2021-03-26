#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <string>
#include "stitching.h"

#pragma comment (lib,"ws2_32.lib")

//using namespace std;
using namespace cv;
class server {
public:
	SOCKET createUDPSocket(int portNumber);
	SOCKET createSocket(int port);
	void createSocket2(int port, SOCKET& clientSocket);

	void receive_dsc(SOCKET clientSocket, SOCKET clientSocket2,Mat &dsc);
	void send_dsc(SOCKET clientSocket,Mat dsc);
	void send_kpts(SOCKET clientSocket, std::vector<KeyPoint> kp);
	void send_fts(int port1, int port2, int port3);
	void recv_kpts(SOCKET clientSocket, SOCKET clientSocket2,std::vector<KeyPoint> &kp);
	void recv_image(int port,int width, int height);
	void recv_features(int port1, int port2, int port3);
	void recv_obstacle(int port);
	Mat m_img_C,m_warp_R, m_warp_L, m_warp_B;
	Mat m_dsc_C;
	void realTimeStitching();
	std::vector<KeyPoint> m_kp_C;
	std::mutex muC;
	std::mutex muL;
	std::mutex muR; 
	std::mutex muB;
	std::vector<Rect> m_rect_R, m_rect_L, m_rect_B, m_rect_C;
	Rect biggest_Rct;
	void find_biggst_rct();
};