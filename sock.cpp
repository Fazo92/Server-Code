#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "sock.h"

void sock::initUDP(int port,string clientIP) {
	WSADATA data;

	// To start WinSock, the required version must be passed to
	// WSAStartup(). This server is going to use WinSock version
	// 2 so I create a word that will store 2 and 2 in hex i.e.
	// 0x0202
	WORD version = MAKEWORD(2, 2);

	// Start WinSock
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		// Not ok! Get out quickly
		cout << "Can't start Winsock! " << wsOk;
		return;
	}
	SOCKET sockettmp= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in serverHint_send,serverHint_recv;
	//serverHint.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint_send.sin_family = AF_INET; // Address format is IPv4
	serverHint_send.sin_port = htons(port); // Convert from little to big endian

	serverHint_recv.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint_recv.sin_family = AF_INET; // Address format is IPv4
	serverHint_recv.sin_port = htons(port); // Convert from little to big endian

	inet_pton(AF_INET, clientIP.c_str(), &serverHint_send.sin_addr);

	//// Try and bind the socket to the IP and port
	int bin = ::bind(sockettmp, (sockaddr*)&serverHint_recv, sizeof(serverHint_recv));
	if (bin == -1)
	{
		std::cout << "Can't bind socket! " << WSAGetLastError() << endl;

	}
	cout << bin << endl;
	this->in = sockettmp;
	this->server= serverHint_send;
	this->serverLength = sizeof(server);
}
void sock::initTCP(int port)
{
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket" << std::endl;

	}

	sockaddr_in hint;
	//hint.sin_addr.s_addr = inet_addr("127.0.0.1");
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	::bind(listening, (sockaddr*)&hint, sizeof(hint));
	listen(listening, SOMAXCONN);

	//Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	//SOCKET clientSocket = accept(listening, NULL, NULL);

	char host[NI_MAXHOST];
	char service[NI_MAXHOST];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << "connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
	}
	in = clientSocket;
	closesocket(listening);
	tcp = true;
}

int sock::getClientLength() {
	return this->serverLength;

}

void sock::closeSock() {
	closesocket(this->in);
	WSACleanup();
}

sockaddr_in sock::getsockaddr_in() {
	return server;

}

void sock::rcv_img(Mat &img) 
{
	while (true) {
		int recvMsgSize; // Size of received message
		char buflen[sizeof(int)];
		if (tcp == true) {
			int bytesDim = 0;
			int bytes = 0;
			char buf2[4];
			int imgSize;
			ZeroMemory(buf2, sizeof(buf2));

			//Wait for client to send data
			//for (int i = 0; i < sizeof(buf2); i += bytesDim)
				if ((bytesDim = recv(in, buf2 , sizeof(buf2) , 0)) == -1) cout << "recv number failed" << endl;
			int* pt = (int*)buf2;
			imgSize = *pt;
			char* buf = new char[imgSize];
			ZeroMemory(buf, imgSize);

			for (int i = 0; i < imgSize; i += bytes)
				if ((bytes = recv(in, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
			Mat rawData = Mat(1, imgSize, CV_8UC1, buf);
			Mat frame = imdecode(rawData, IMREAD_COLOR);
			delete[] buf;
			imshow("frame", frame);
			waitKey(1);

		}
		else {
			
			char buffer[65540]; // Buffer for echo string

			int l = getClientLength();
			sockaddr_in cl = getsockaddr_in();
			sockaddr_in cl2;
			int l2 = sizeof(cl2);
			//ZeroMemory(&cl, sock1.getClientLength()); // Clear the client structure
			//ZeroMemory(buffer, 65540); // Clear the receive buffer
			do {

				recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2);
			} while (recvMsgSize > sizeof(int));
			long int total_pack = ((int*)buffer)[0];

			char* longbuf = new char[PACK_SIZE * total_pack];
			for (int i = 0; i < total_pack; i++) {
				recvMsgSize = recvfrom(in, buffer, 65540, 0, (sockaddr*)&cl2, &l2);
				if (recvMsgSize != PACK_SIZE) {
					cerr << "Received unexpected size pack:" << recvMsgSize << endl;
					continue;
				}
				memcpy(&longbuf[i * PACK_SIZE], buffer, PACK_SIZE);
			}


			Mat rawData = Mat(1, PACK_SIZE * total_pack, 0, longbuf);
			Mat frame = imdecode(rawData, IMREAD_COLOR);
			img = frame.clone();
			if (frame.size().width == 0) {
				cerr << "decode failure!" << endl;
				continue;
			}
			namedWindow("recv", WINDOW_FREERATIO);
			imshow("recv", frame);
			delete[] longbuf;

			waitKey(1);
		}
	}
}


void sock::send_img(cv::Mat img) {
	vector <uchar> imgVec;
	vector < int > compression_params;
	compression_params.push_back(IMWRITE_JPEG_QUALITY);
	compression_params.push_back(80);
	sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));

	imencode(".jpg", img, imgVec, compression_params);
	if (tcp == true) {
		int vecSize = imgVec.size();
		send(in, (char*)&vecSize, sizeof(int), 0);
		send(in, (char*)&imgVec[0], imgVec.size(), 0);

	}
	else
	{
		int total_pack = 1 + (imgVec.size() - 1) / PACK_SIZE;
		int ibuf[1];
		ibuf[0] = total_pack;
		int s=sendto(in, (char*)&ibuf, sizeof(int), 0, (sockaddr*)&server, sizeof(server));
		string st = "hello";
		//int s = sendto(in, st.c_str(), sizeof(st), 0, (sockaddr*)&server, sizeof(server));

		if (s == SOCKET_ERROR)
		{
			cout << "That didn't work! " << WSAGetLastError() << endl;
		}
		for (int i = 0; i < total_pack; i++)
			sendto(in, (char*)&imgVec[i * PACK_SIZE], PACK_SIZE, 0, (sockaddr*)&server, sizeof(server));
	}
}