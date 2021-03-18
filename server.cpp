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
		Mat img = Mat::zeros(height, width, CV_8UC3);

		const int imgSize = img.total() * img.elemSize();

		char* buf = new char[imgSize];

		int bytes = 0;

		ZeroMemory(buf, imgSize);
		/*for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(clientSocket, buf+i , imgSize-i , 0)) == -1) cout << ("recv failed");
		cout << "bytes image: " << bytes << endl;
			Mat rawData = Mat(1, height * width, CV_8U, buf);

			img = imdecode(rawData, IMREAD_COLOR);*/
			//if (img.empty()) {
			//	img = Mat::ones(height, width, CV_8UC3);
			//}

		for (int i = 0; i < imgSize; i += bytes) {
			if ((bytes = recv(clientSocket, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		}
		int ptr = 0;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
				ptr = ptr + 3;
			}
		}

//
//		ZeroMemory(buf_depth, depth_imgSize);
//
//		for (int i = 0; i < depth_imgSize; i += bytes_depth) {
//			if ((bytes_depth = recv(clientSocket2, buf_depth + i, depth_imgSize - i, 0)) == -1) cout << ("recv failed");
//		}
//		int ptr_d = 0;
//		for (int i = 0; i < depth_img.rows; i++) {
//			for (int j = 0; j < depth_img.cols; j++) {
//				depth_img.at<Vec3b>(i, j) = Vec3b(buf_depth[ptr_d + 0], buf_depth[ptr_d + 1], buf_depth[ptr_d + 2]);
//				ptr_d = ptr_d + 3;
//			}
//		}
//		imshow("img", depth_img);
//if (waitKey(1000 / 30) >= 0) break;

		if (port == 30000) {
			m_img_C= img;
			if (m_img_C.empty()) cout << "imgC empty" << endl;
			//imshow("img", img);
			//if (waitKey(1000 / 30) >= 0) break;
		}
		else if (port == 20000) {
			m_warp_R= img;
			if (m_warp_R.empty()) cout << "m_warp_R empty" << endl;

		}
		else if (port == 10000) {
			m_warp_L = img;
			if (m_warp_L.empty()) cout << "m_warp_L empty" << endl;

			//imshow("img", img);
			//if (waitKey(1000 / 30) >= 0) break;
		}
		else if (port == 40000) {
			m_warp_B = img;
			if (m_warp_B.empty()) cout << "m_warp_B empty" << endl;

		}

	}
	
}

void server::recv_features(int port1, int port2, int port3) {

	SOCKET clientSocket1 = createSocket(port1);
	//SOCKET clientSocket2 = createSocket(port2);
	SOCKET clientSocket3 = createSocket(port3);

	while (true) {
		int bytes = 0;
		int bytes_kp = 0;
		int bytes_bufdsc = 0;
		int bytes_kp_num = 0;
		int buf_dsc;
		int kp_size;
		ZeroMemory((char*)&buf_dsc, sizeof(buf_dsc));

		for(int i=0;i< sizeof(buf_dsc);i+=bytes_bufdsc)
			if ((bytes_bufdsc=recv(clientSocket3, (char*)&buf_dsc+i, sizeof(buf_dsc)-i, 0))==-1) cout << "dsc recv failed" << endl;

		ZeroMemory((char*)&kp_size, sizeof(kp_size));

		for (int i = 0; i < sizeof(kp_size); i += bytes_kp_num)
			if ((bytes_kp_num = recv(clientSocket3, (char*)&kp_size+i, sizeof(kp_size)-i, 0))==-1) cout << "dsc kp failed" << endl;


		//const int dscSize = 1024*1024*sizeof(float);
		const int dscSize = buf_dsc;

		char* buf = new char[dscSize];
		ZeroMemory(buf, dscSize);
		for (int i = 0; i < dscSize; i += bytes)
			if ((bytes = recv(clientSocket1, buf + i, dscSize - i, 0)) == -1) cout << "dsc recv failed" << endl;
		int dsc_cols = 64;
		int dsc_rows = dscSize / (dsc_cols * 4);
		float* p = (float*)buf;
		Mat dsc = Mat::zeros(dsc_rows, dsc_cols, CV_32FC1);
		for (int i = 0; i < dsc_rows; i++) {
			float* ptrdsc = dsc.ptr<float>(i);
			for (int j = 0; j < dsc_cols; j++) {
				ptrdsc[j] = *p;
				p++;
			}
		}

		///////////////////////////////////////////////////
		std::vector<KeyPoint> keyPoints;
		//const int kpSize = 1024 * 10 * sizeof(KeyPoint);
		const int kpSize = kp_size;

		char* buf_kp = new char[kpSize];

		ZeroMemory(buf_kp, kpSize);
		for (int i = 0; i < kp_size; i += bytes_kp)
			if ((bytes_kp = recv(clientSocket1, buf_kp + i, kpSize - i, 0)) == -1) cout << ("recv kp failed");

		KeyPoint* p_kp;
		for (p_kp = (KeyPoint*)&buf_kp[0]; p_kp <= (KeyPoint*)&buf_kp[bytes_kp - 1]; p_kp++) {
			keyPoints.push_back(*p_kp);
		}
		if (bytes_kp == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
		}
		//cout << "dsc: " << dsc.size() << endl;
		//cout << " kpts: " << keyPoints.size() << endl;
		if (keyPoints.size() != dsc.rows) {
			//cout <<"current "<< keyPoints.size() << dsc.size() << endl;
			//cout << "prev " << m_kp_C.size() << m_dsc_C.size() << endl;

			cout << "dims don't not match" << endl;
			continue;
		}
		m_kp_C = keyPoints;
		m_dsc_C = dsc;
	}
	closesocket(clientSocket1);
	//closesocket(clientSocket2);
	closesocket(clientSocket3);

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
		send(clientSocket3, (char*)&dscSize, sizeof(dscSize), 0);
		send(clientSocket3, (char*)&kp_size, sizeof(kp_size), 0);
		int sendDsc = send(clientSocket1, (char*)dsc.data, dscSize, 0);
		int sendKp = send(clientSocket1, (char*)&kp[0], kp_size, 0);
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
		namedWindow("pano", WINDOW_OPENGL);
		imshow("pano", pano_GPU);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
		if (waitKey(10000/30) >= 0) break;

	}
}