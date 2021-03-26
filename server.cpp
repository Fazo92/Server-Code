#include "server.h"
//server::server() {
//}

SOCKET server::createUDPSocket(int portNumber)
{
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int ws0k = WSAStartup(version, &data);
	if (ws0k != 0) {
		std::cout << "Can't start Winsock! " << ws0k;
	}
	SOCKET in = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (in < 0) perror("Can't create Socket");
	sockaddr_in serverHint;

	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	//serverHint.sin_addr.s_addr = INADDR_ANY;
	serverHint.sin_port = htons(portNumber);

	if (::bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == -1) {
		//cout << "Can't bind socket!" << WSAGetLastError() << endl;
		perror("bind failed");
	}
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(in, (sockaddr*)&client, &clientSize);
	char host[NI_MAXHOST];
	char service[NI_MAXHOST];

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXHOST);

	if (getnameinfo((sockaddr*)&serverHint, sizeof(serverHint), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		std::cout << host << "connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &serverHint.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(serverHint.sin_port) << std::endl;
	}
	//else {
	//	cout << "Erfolgreich Verbunden UDP" << endl;
	//}

	return clientSocket;

	//WSACleanup();
}

SOCKET server::createSocket(int port) {
	int bytes = 0;
	int bytes1 = 0;
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int ws0k = WSAStartup(ver, &wsData);
	if (ws0k != 0) {
		std::cerr << "Can't initialize winsock" << std::endl;
	}
	//Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket" << std::endl;

	}

	//Bind the op adress and port to a socket
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
	 //clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

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
	closesocket(listening);

	return clientSocket;
}


void server::createSocket2(int port, SOCKET& clientSocket) {
	int bytes = 0;
	int bytes1 = 0;
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int ws0k = WSAStartup(ver, &wsData);
	if (ws0k != 0) {
		std::cerr << "Can't initialize winsock" << std::endl;
	}
	//Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket" << std::endl;

	}

	//Bind the op adress and port to a socket
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
	//clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
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
	closesocket(listening);

}



void server::receive_dsc(SOCKET clientSocket, SOCKET clientSocket2, Mat &dsc) {
	int bytes = 0;
	int buf_dsc;
	while (true) {
		ZeroMemory((char*)&buf_dsc, sizeof(buf_dsc));

		recv(clientSocket2, (char*)&buf_dsc, sizeof(buf_dsc), 0);
		//const int dscSize = 1024*1024*sizeof(float);
		const int dscSize = buf_dsc;

		char* buf = new char[dscSize];
		ZeroMemory(buf, dscSize);
		for (int i = 0; i < dscSize; i += bytes)
			if ((bytes = recv(clientSocket, buf + i, dscSize - i, 0)) == -1) cout << "dsc recv failed" << endl;
		int dsc_cols = 64;
		int dsc_rows = bytes / (dsc_cols * 4);
		float* p = (float*)buf;
		dsc = Mat::zeros(dsc_rows, dsc_cols, CV_32FC1);
		for (int i = 0; i < dsc_rows; i++) {
			float* ptrdsc = dsc.ptr<float>(i);
			for (int j = 0; j < dsc_cols; j++) {
				ptrdsc[j] = *p;
				p++;
			}
		}
		cout << "dsc: " << dsc.size() << endl;
		this->m_dsc_C = dsc;
		//string s = "arrived";
		//send(clientSocket, s.c_str(), 10, 0);
	}
}
void server::send_dsc(SOCKET clientSocket,Mat dsc) {
	char buf[10];
	int dscSize = dsc.total() * dsc.elemSize();
	int sendDsc = send(clientSocket, (char*)dsc.ptr(0), dscSize, 0);
	ZeroMemory(buf, 10);
	recv(clientSocket, buf, 10, 0);
	cout << "answer: " << buf << endl;

}

void server::send_kpts(SOCKET clientSocket, vector<KeyPoint> kp) {
	char buf[10];
	int kpts_size = kp.size() * sizeof(KeyPoint);
	int send_kpts = send(clientSocket, (char*)&kp[0], kpts_size, 0);
	if (send_kpts == -1) cout << "send keypoints failed" << endl;
	recv(clientSocket, buf, 10, 0);
	cout << "answer: " << buf << endl;

}

void server::recv_kpts(SOCKET clientSocket,SOCKET clientSocket2,vector <KeyPoint> &kp) {
	int bytes = 0;
	
	int kp_size;
	while (true) {
		ZeroMemory((char*)&kp_size, sizeof(kp_size));

		recv(clientSocket2, (char*)&kp_size, sizeof(kp_size), 0);
		std::vector<KeyPoint> keyPoints;
		//const int kpSize = 1024 * 10 * sizeof(KeyPoint);
		const int kpSize = kp_size;

		char* buf = new char[kpSize];

		ZeroMemory(buf, kpSize);
		for (int i = 0; i < kp_size; i += bytes)
			if ((bytes = recv(clientSocket, buf + i, kpSize - i, 0)) == -1) cout << ("recv kp failed");

		KeyPoint* p;
		for (p = (KeyPoint*)&buf[0]; p <= (KeyPoint*)&buf[bytes - 1]; p++) {
			keyPoints.push_back(*p);
		}
		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
		}
		cout << " kpts: " << keyPoints.size() << endl;
		kp = keyPoints;
		this->m_kp_C = keyPoints;
		//string s = "arrived";
		//int sendKpts = send(clientSocket, s.c_str(), 10, 0);
	}

}


void server::recv_image(int port, int height, int width) {
	//const int imgSize = width * height * 3;
	SOCKET clientSocket = createSocket(port);
	//SOCKET clientSocket2 = createSocket(port+1);
	
	while (true) {

		//int bytes_dim = 0;
		//char* buf_dim = new char[4];
		//for (int i = 0; i < sizeof(buf_dim); i++) {
		//	if ((bytes_dim = recv(clientSocket, buf_dim + i, sizeof(buf_dim) - i, 0)) == -1) cout << "recv number on port: " << port << " failed " << endl;
		//}
		//int* pt = (int*)buf_dim;
		//int imgSize = *pt;
		//int bytes = 0;
		//char* buf = new char[imgSize];
		//cout << imgSize << endl;
		////ZeroMemory(buf, imgSize);
		//for (int i = 0; i < imgSize; i += bytes)
		//	if ((bytes = recv(clientSocket, buf + i, imgSize - i, 0)) == -1) cout << "recv img on port: " << port << " failed " << endl;
		//
		//Mat rawData = Mat(1, imgSize, CV_8UC1, buf);
		//Mat img = imdecode(rawData, IMREAD_COLOR);

		Mat img = Mat::zeros(480 * 2, 640 * 2, CV_8UC3);
		int imgSize = img.total() * img.elemSize();
		//char* buf = new char[imgSize];
		int bytes = 0;
		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(clientSocket,reinterpret_cast<char*>(img.data), imgSize , MSG_WAITALL)) == -1) cout << "recv img on port: " << port << " failed " << endl;


			//ZeroMemory(buf, imgSize);
			////Wait for client to send data
			//for (int i = 0; i < imgSize; i += bytes)
			//	if ((bytes = recv(clientSocket, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
			//int ptr = 0;
			//for (int i = 0; i < img.rows; i++) {
			//	for (int j = 0; j < img.cols; j++) {
			//		img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
			//		ptr = ptr + 3;
			//		//cout << buf[ptr + 0] << endl << buf[ptr + 1] << endl << buf[ptr + 2] << endl;

			//	}
			//}

		if (port == 30000) {
			muC.lock();
			m_img_C= img;
			muC.unlock();

			if (m_img_C.empty()) cout << "imgC empty" << endl;
			//imshow("imgC", img);
			//if (waitKey(1000 / 30) >= 0) break;
		}
		else if (port == 20000) {
			muR.lock();
			m_warp_R= img;
			muR.unlock();
			if (m_warp_R.empty()) cout << "m_warp_R empty" << endl;
			//imshow("imgR", img);
			//if (waitKey(1000 / 30) >= 0) break;
		}
		else if (port == 10000) {
			muL.lock();
			m_warp_L = img;
			muL.unlock();
			if (m_warp_L.empty()) cout << "m_warp_L empty" << endl;
			//imshow("imgL", img);
			//if (waitKey(1000 / 30) >= 0) break;

		}
		else if (port == 40000) {
			muB.lock();
			m_warp_B = img;
			muB.unlock();
			if (m_warp_B.empty()) cout << "m_warp_B empty" << endl;
			//imshow("imgB", img);
			//if (waitKey(1000 / 30) >= 0) break;
		}
	}
	
}

void server::recv_features(int port1, int port2, int port3) {
	SOCKET clientSocket2 = createSocket(port2);

	SOCKET clientSocket1 = createSocket(port1);
	//SOCKET clientSocket3 = createSocket(port3);

	while (true) {
		char* buf_dim = new char[4];
		int bytes_dim = 0;
		int bytes_dsc = 0;
		int bytes_kpts = 0;

		if ((bytes_dim = recv(clientSocket2, buf_dim, 4, MSG_WAITALL)) == -1) cout << "receiving kp dim failed" << endl;
		int* pt_dim = (int*)buf_dim;
		int kpSize = *pt_dim;
		int dscSize = ((kpSize / sizeof(KeyPoint) * 4 * 64));
		int dsc_cols = 64;
		int dsc_rows = (dscSize / (64 * sizeof(float)));
		Mat dsc = Mat::zeros(dsc_rows, dsc_cols, CV_32FC1);

		if ((bytes_dsc = recv(clientSocket1, reinterpret_cast<char*>(dsc.data), dscSize, MSG_WAITALL)) == -1) cout << "dsc recv failed" << endl;
		vector<KeyPoint> kpts;
		kpts.resize(kpSize / sizeof(KeyPoint));
		if ((bytes_kpts = recv(clientSocket1, reinterpret_cast<char*>(&kpts[0]), kpSize, MSG_WAITALL)) == -1) cout << "kpts recv failed" << endl;

		if (kpts.size() != dsc.rows) {
			//cout <<"current "<< keyPoints.size() << dsc.size() << endl;
			//cout << "prev " << m_kp_C.size() << m_dsc_C.size() << endl;

			cout << "dims don't not match" << endl;
			continue;
		}
		m_kp_C = kpts;
		m_dsc_C = dsc;
		delete[] buf_dim;
		kpts.clear();
		dsc.release();
	}
	closesocket(clientSocket1);
	closesocket(clientSocket2);
	//closesocket(clientSocket3);

}



void server::send_fts(int port1, int port2, int port3) {
	SOCKET clientSocket1 = createSocket(port1);
	//SOCKET clientSocket2 = createSocket(port2);
	SOCKET clientSocket3 = createSocket(port3);

	while (true) {
		if (m_dsc_C.empty() || m_kp_C.empty()) continue;
		Mat dsc = m_dsc_C;
		vector<KeyPoint>kp = m_kp_C;
		int dscSize = dsc.total() * dsc.elemSize();
		int kp_size = kp.size() * sizeof(KeyPoint);
		int sendNumber=send(clientSocket3, (char*)&kp_size, sizeof(kp_size), 0);
		int sendDsc = send(clientSocket1, (char*)dsc.data, dscSize, 0);
		int sendKp = send(clientSocket1, (char*)&kp[0], kp_size, 0);
		if (sendNumber == -1 || sendDsc == -1 || sendKp == -1) {
			cout << "send features failed" << endl;
			cout << "sendNumber: " << sendNumber << endl;
			cout << "sendDsc: " << sendDsc << endl;
			cout << "sendKp: " << sendKp << endl;

		}
	}
	closesocket(clientSocket1);
	//closesocket(clientSocket2);
	closesocket(clientSocket3);
}

void server::realTimeStitching() {
	Stitching st;
	while (true) {


		if (m_img_C.empty() || m_warp_R.empty()||m_warp_L.empty()|| m_warp_B.empty()) continue;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		cout << "h" << endl;
		Mat imgC = m_img_C;
		Mat warpR =m_warp_R;
		Mat warpL = m_warp_L;
		Mat warpB = m_warp_B;
		Mat pano = st.AlphaBlendingGPU(imgC, warpR, 1., 0);
		Mat pano2 = st.AlphaBlendingGPU(pano, warpL, 1., 1);
		Mat pano3 = st.AlphaBlendingGPU(pano2, warpB, 1., 2);

		cuda::GpuMat pano_GPU;
		pano_GPU.upload(pano3);
		//namedWindow("pano", WINDOW_OPENGL);
		//imshow("pano", pano_GPU);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
		//if (waitKey(10000/30) >= 0) break;

	}
}

void server::recv_obstacle(int port)
{
	SOCKET clientSocket = createSocket(port);
	while (true) {
		char* buf = new char[4];
		int bytes = 0;
		int bytes_size = recv(clientSocket, buf, 4, MSG_WAITALL);
		int* pt = (int*)buf;
		int rect_size = *pt;
		vector<Rect> rct;
		rct.resize(rect_size / sizeof(Rect));
		if (( bytes = recv(clientSocket, reinterpret_cast<char*>(&rct[0]), rect_size, MSG_WAITALL)) == -1) cout << "kpts recv failed" << endl;

		delete[] buf;
		if (rct.size() == 0) {
			cout << "Rct size is 0" << endl;
			continue;
		}
		std::sort(rct.begin(), rct.end(),
			[](Rect& x, Rect& y) {
				if (x.area() < y.area())
					return x.area() > y.area();
			});
		
		switch (port) {
			case 24000:
				m_rect_R = rct;
	

			case 14000:
				m_rect_L = rct;

			case 44000:
				m_rect_B = rct;

			case 34000:
				m_rect_C = rct;

		}

	}
	closesocket(clientSocket);
}

void server::find_biggst_rct() {
	while (true) {
		if (m_rect_R.empty() || m_rect_L.empty() || m_rect_B.empty() || m_rect_C.empty()) continue;

		int area = 0;
		if (m_rect_R[0].area() > area) {
			biggest_Rct = m_rect_R[0];
			area = m_rect_R[0].area();
		}
		if (m_rect_L[0].area() > area) {
			biggest_Rct = m_rect_L[0];
			area = m_rect_L[0].area();
		}
		if (m_rect_B[0].area() > area) {
			biggest_Rct = m_rect_B[0];
			area = m_rect_B[0].area();
		}
		if (m_rect_C[0].area() > area) {
			biggest_Rct = m_rect_C[0];
			area = m_rect_C[0].area();
		}
	}
}