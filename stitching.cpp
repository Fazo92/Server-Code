#include "stitching.h"


Stitching::Stitching() {
	Mat imgLeft;
	Mat imgRight;
	Mat imgCenter;
	Mat imgBottom1;
	Mat imgBottom2;

	this->imgLeft = imgLeft;
	this->imgRight = imgRight;
	this->imgCenter = imgCenter;
	this->imgBottom1 = imgBottom1;
	this->imgBottom2 = imgBottom2;

}

void Stitching::serialUDP(int port1, int port2, int port3, int port4) {
	SOCKET clientsocket1 = this->serv.createUDPSocket(port1);

	sockaddr_in client1,client2,client3,client4;
	int clientLength1 = sizeof(client1);


	
	int height = 480;
	int width = 640;
	int bytes = 0;
	char buf[921600];

	while (true)
	{
		Mat img = Mat::zeros(height, width, CV_8UC3);
		int imgSize;
		imgSize = img.total()*img.elemSize();
		ZeroMemory(&client1, clientLength1); 
		
		ZeroMemory(buf, imgSize);
		//Wait for client to send data

		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(clientsocket1, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		//buf[bytes] = '\n';
		
		int ptr = 0;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
				ptr = ptr + 3;
			}
		}

		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}

		if (port1 == 54000) {
			this->imgCenter = img.clone();

		}
		else if (port1 == 58000) {
			this->imgLeft = img.clone();
		}


	}
	closesocket(clientsocket1);

	WSACleanup();

}
void Stitching::serialTCP(int port1, int port2, int port3, int port4)
{
	SOCKET clientsocket1 = this->serv.createSocket(port1);
	SOCKET clientsocket2 = this->serv.createSocket(port2);
	SOCKET clientsocket3 = this->serv.createSocket(port3);
	SOCKET clientsocket4= this->serv.createSocket(port4);


	int height = 480;
	int width = 640;
	int bytes = 0;
	int bytes1 = 0;
	int bytes2 = 0;
	int bytes3 = 0;
	int bytes4 = 0;
	char buf[921600];
	KeyPoint kp;

	char bufkp[1024 * 1024 * 2];
	KeyPoint *p;

	int buffsize = 1024 * 1024 * 2;
	setsockopt(clientsocket3, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	int bufrow;
	int bufcol;
	char bufdsc[1024 * 1024 * 2];
	int dscSize;
	Mat dsc;

	while (true)
	{
		if (this->features == true) continue;
		Mat img = Mat::zeros(height, width, CV_8UC3);
		int imgSize;
		imgSize = img.total()*img.elemSize();
		ZeroMemory(buf, imgSize);
		//Wait for client to send data
		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(clientsocket1, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		int ptr = 0;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
				ptr = ptr + 3;
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////
		vector<KeyPoint> veckp;

		ZeroMemory(bufkp, sizeof(bufkp));
		//for (int i = 0; i < len; i += bytes)
		if ((bytes1 = recv(clientsocket2, bufkp, sizeof(bufkp), 0)) == -1) cout << ("recv3 failed");
		KeyPoint *p;

		for (p = (KeyPoint*)&bufkp[0]; p <= (KeyPoint*)&bufkp[bytes1 - 1]; p++) {
			veckp.push_back(*p);
		}
		p = nullptr;
		/////////////////////////////////////////////////////////////////////////////////////////

		ZeroMemory(bufdsc, sizeof(bufdsc));
		//Wait for client to send data
		ZeroMemory((char*)&bufrow, sizeof(bufrow));

		if ((bytes4 = recv(clientsocket4, (char*)&bufrow, sizeof(bufrow), 0)) == -1) cout << ("recv failed dsc");

		ZeroMemory((char*)&bufcol, sizeof(bufcol));

		if ((bytes4 = recv(clientsocket4, (char*)&bufcol, sizeof(bufcol), 0)) == -1) cout << ("recv failed dsc");

		if ((bytes3 = recv(clientsocket3, bufdsc, bufcol*bufrow * 4, 0)) == -1) cout << ("recv failed");
		float *pdsc = (float*)bufdsc;
		dsc = Mat::zeros(bufrow, bufcol, CV_32FC1);
		for (int i = 0; i < bufrow; i++) {
			for (int j = 0; j < bufcol; j++) {
				dsc.at<float>(i, j) = *pdsc;
				*pdsc++;
			}
		}
		pdsc = nullptr;
		if (bytes == SOCKET_ERROR|| bytes1 == SOCKET_ERROR|| bytes2 == SOCKET_ERROR|| bytes3 == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}
		if (bytes == 0|| bytes1 == 0|| bytes3 == 0 || bytes4 == 0)
		{
			cout << "Client disconnected" << endl;
			cout << "bytes: "<<bytes << endl;
			cout << "bytes1: " << bytes1 << endl;
			cout << "bytes3: " << bytes3 << endl;
			cout << "bytes4: " << bytes4 << endl;

			break;
		}
		string s = "angekommen";
		int sendresult = send(clientsocket3, s.c_str(), sizeof(s), 0);
		this->features = true;

		if (port1 == 54000) {
			this->imgCenter = img.clone();
			this->m_kpC = veckp;
			this->dscCenter = dsc;

		}
		else if (port1 == 58000) {
			this->imgLeft = img.clone();
			this->m_kpL = veckp;
			this->dscLeft = dsc;
		}


	}
	closesocket(clientsocket1);
	closesocket(clientsocket2);
	closesocket(clientsocket3);
	closesocket(clientsocket4);

	WSACleanup();

}



void Stitching::CamFeatures() {
	VideoCapture cap;
	// open the default camera, use something different from 0 otherwise;
	// Check VideoCapture documentation.
	Mat imgGray1,dsccpuc;
	//cuda::SURF_CUDA surf;
	//cuda::GpuMat img1, kpGPU, dscGPU;
	Ptr<SURF> detector = SURF::create(400);
	vector<KeyPoint> kp;
	Mat dsc;
	Mat outK, out,tmp;
	if (!cap.open(0))
		cout << "can't open Webcam" << endl;
	for (;;)
	{
		cap >> tmp;
		resize(tmp, tmp, Size(640, 480));
		this->camFrame=tmp;

		detector->detectAndCompute(tmp, noArray(), kp, dsc);

		//cvtColor(this->camFrame, imgGray1, COLOR_BGR2GRAY);
		//img1.upload(imgGray1);
		//surf(img1, cuda::GpuMat(), kpGPU, dscGPU);
		//surf.downloadDescriptors(dscGPU, dsc);
		//mudscLeft.lock();
		//this->dscGPUnew = dscGPU;
		//mudscLeft.unlock();

		//surf.downloadKeypoints(kpGPU, kp);
		this->m_kpR = kp;

		this->dscRight = dsc;

		//img1.download(out);

		this->imgRight = tmp;

		//drawKeypoints(this->camFrame, kp, outK);
		//if (this->camFrame.empty()) break; // end of video stream
		//imshow("this is you, smile! :)", outK);
		//if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
	}
}



void Stitching::getFeatures(int port1, int port2, int port3) {
	SOCKET clientSocket1 = this->serv.createSocket(port1);
	SOCKET clientSocket2 = this->serv.createSocket(port2);
	SOCKET clientSocket3 = this->serv.createSocket(port3);
	int bytes = 0;
	vector<KeyPoint> veckp;
	KeyPoint kp;

	char buf[1024 * 1024 * 2];
	//KeyPoint buffer[3000];
	float buffer;
	KeyPoint *p;
	Mat imgK;
	int bytes1 = 0;
	int bytes2 = 0;
	int bytes3 = 0;
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);
	int buffsize = 1024 * 1024 * 2;
	setsockopt(clientSocket2, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	int bufrow;
	int bufcol;
	char bufdsc[1024 * 1024 * 2];
	int dscSize;
	Mat dsc;
	while (true) {
		vector<KeyPoint> veckp;

		ZeroMemory(buf, sizeof(buf));
		if ((bytes = recv(clientSocket1, buf, sizeof(buf), 0)) == -1) cout << ("recv3 failed");
		KeyPoint *p;
		for (p = (KeyPoint*)&buf[0]; p <= (KeyPoint*)&buf[bytes - 1]; p++) {
			veckp.push_back(*p);
		}
		p = nullptr;

		vector<float> descriptors;
		ZeroMemory(bufdsc, sizeof(bufdsc));
		//Wait for client to send data
		ZeroMemory((char*)&bufrow, sizeof(bufrow));

		if ((bytes2 = recv(clientSocket3, (char*)&bufrow, sizeof(bufrow), 0)) == -1) cout << ("recv failed dsc");

		ZeroMemory((char*)&bufcol, sizeof(bufcol));
		if ((bytes3 = recv(clientSocket3, (char*)&bufcol, sizeof(bufcol), 0)) == -1) cout << ("recv failed dsc");


		if ((bytes1 = recv(clientSocket2, bufdsc, bufcol*bufrow * 4, 0)) == -1) cout << ("recv failed");

		float *pd = (float*)bufdsc;
		dsc = Mat::zeros(bufrow, bufcol, CV_32FC1);
		int ptr = 0;

		for (int i = 0; i < bufrow; i++) {
			for (int j = 0; j < bufcol; j++) {
				dsc.at<float>(i, j) = *pd;
				*pd++;

			}
		}
		pd = nullptr;
		string s = "angekommen";
		int sendresult = send(clientSocket2, s.c_str(), sizeof(s), 0);

		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}
		if (port1 == 53000) {
			this->m_kpC = veckp;
			this->dscCenter = dsc;
			this->features = true;
			//cout << "dscCenter Size: " << dsc.size() << endl;
			//cout << " " << endl;

		}
		else if (port1 == 59000) {
			this->m_kpL = veckp;
			this->dscLeft = dsc;
			//cout << "dscLeft Size: " << dsc.size() << endl;
			//cout << " " << endl;
		}

	}
	closesocket(clientSocket1);
	closesocket(clientSocket2);
	closesocket(clientSocket3);

	WSACleanup();
}


void Stitching::realTimeStitching()
{

	Mat matchesimg1,dscC1;
	//Mat dscCenter, dscRight;
	vector<KeyPoint> kpCenter, kpLeft;
	std::this_thread::sleep_for(2s);
	cuda::GpuMat dscC, dscR;
	Mat dscCCPU, dscLCPU;


	while (true) {
		if (this->m_kpC.empty() || this->kptmp.empty()|| this->dscCenter.empty()|| this->dsctmp.empty()) {
			continue;
		}
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		//if (this->rs == false || this->features == false) continue;
		kpCenter.clear();
		kpLeft.clear();
		dscCCPU.release();
		dscLCPU.release();
		vector<DMatch> mt1;
		Mat L = this->img.clone();
		Mat C = this->imgCenter.clone();
		kpCenter = this->m_kpC;
		kpLeft = this->kptmp;
		dscCCPU = this->dscCenter;
		dscLCPU = this->dsctmp;
		//this->rs = false;
		//this->features = false;
		//cout << "RealTime " << "kpCenter.size(): " << kpCenter.size() << "= ?" << " dscCCPU.rows: " << dscCCPU.rows << endl;
		if (kpLeft.size() != dscLCPU.rows || kpCenter.size() != dscCCPU.rows) {
			cout <<"RealTime "<< "kpLeft.size(): " << kpLeft.size() << "= ?" << " dscLCPU.rows: " << dscLCPU.rows << endl;
			cout <<"RealTime " << "kpCenter.size(): " << kpCenter.size() << "= ?" << " dscCCPU.rows: " << dscCCPU.rows << endl;
			continue;
		}


		//Ptr<cv::cuda::DescriptorMatcher> matcher11;
		Ptr<cv::DescriptorMatcher> matcher11;
		vector<vector<DMatch>> matchesknn;
		matcher11 = BFMatcher::create(NORM_L2, false);
		matcher11->knnMatch(dscCCPU, dscLCPU, matchesknn,2);
		if (!matchesknn.empty()) {
			for (int i = 0; i < matchesknn.size(); i++) {
				if (matchesknn[i][0].distance < .75 * matchesknn[i][1].distance) {
					mt1.push_back(matchesknn[i][0]);
				}
			}
		}
		else {
			continue;
		}
		vector<Point2f> obj, scene;
		for (int i = 0; i < mt1.size(); i++) {
			if ((mt1[i].queryIdx < kpCenter.size()) && (mt1[i].trainIdx < kpLeft.size())) {
				obj.push_back(kpCenter[mt1[i].queryIdx].pt);
				scene.push_back(kpLeft[mt1[i].trainIdx].pt);
			}
			//cout << "obj: " << obj[i] << endl;
			//cout << "scene: " << scene[i] << endl;
		}


		if (obj.empty() || scene.empty()) continue;
		Mat H = findHomography(scene, obj, RANSAC);
		if (H.empty()) continue;
		Mat wLeft;
		Mat centerTemplate = Mat::zeros(Size(C.size() * 2), C.type());
		Mat roi(centerTemplate, Rect(C.cols / 2, C.rows / 2, C.cols, C.rows));
		C.copyTo(roi);
		warpPerspective(L, wLeft, H, Size(C.size()*2), INTER_CUBIC);
	
			Mat fstPano = AlphaBlending(centerTemplate, wLeft);
			//namedWindow("fstPano", WINDOW_FREERATIO);
			imshow("fstPano", fstPano);
			//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
			//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() << "[ns]" << std::endl;
			if (waitKey(1000) == 27) break;


	}

}



void Stitching::getDescriptorTCP(int port)
{
	std::this_thread::sleep_for(1s);
	SOCKET clientSocket2;

	SOCKET clientSocket = serv.createSocket(port);
	this->sock3 = clientSocket;
	if (port == 52000) {
		clientSocket2 = serv.createSocket(80000);
	}
	else if (port == 60000) {
		clientSocket2 = serv.createSocket(90000);

	}
 
	

	//SOCKET clientSocket = this->serv.createUDPSocket(52000);

	int bytes = 0;
	int bytes1 = 0;
	int bytes2 = 0;
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);
	int buffsize = 1024 * 1024*2;
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	int bufrow;
	int bufcol;
	char buf[1024 * 1024*2];
	int dscSize;
	Mat dsc;
	while (true)
	{

		vector<float> descriptors;
		ZeroMemory(buf, sizeof(buf));
		//Wait for client to send data
		ZeroMemory((char*)&bufrow, sizeof(bufrow));

		if ((bytes1 = recv(clientSocket2, (char*)&bufrow, sizeof(bufrow), 0)) == -1) cout << ("recv failed dsc");

		ZeroMemory((char*)&bufcol, sizeof(bufcol));

		if ((bytes2 = recv(clientSocket2, (char*)&bufcol, sizeof(bufcol), 0)) == -1) cout << ("recv failed dsc");


		if ((bytes = recv(clientSocket, buf, bufcol*bufrow*4, 0)) == -1) cout << ("recv failed");
		float *p=(float*)buf;
		dsc = Mat::zeros(bufrow, bufcol, CV_32FC1);
		
		int ptr = 0;

		for (int i = 0; i < bufrow; i++) {
			for (int j = 0; j < bufcol; j++) {
				dsc.at<float>(i, j) = *p;
				*p++;
				ptr = ptr + 1;

			}
		}	
		string s = "dsc fertig";
		int senddsctodsc = send(this->sock3, s.c_str(), sizeof(s), 0);
		//dsc.data = (uchar*)&buf[0];

		//imshow("DSC", dsc);
		//if (waitKey(100) >= 0) break;
		 //dsc=imdecode(buf,0);
		//imshow("DSC: ", dsc);
		//if (waitKey(10) == 27) break;
		//for (p = (float*)&buffer[0]; p <= (float*)&buffer[bytes - 1]; p++) {
		//	descriptors.push_back(*p);
		//	//*p++;

		//}
		//*p = NULL;

		if (bytes == SOCKET_ERROR) {
			cout << "Error receiving from client" << WSAGetLastError() << endl;
			continue;
		}


		if (port == 60000)
		{

			this->dscLeft = dsc;
			//cout << "dscLeft Size: " << dsc.size() << endl;
			//cout << " " << endl;
		}
		else if (port == 52000) {

			this->dscCenter = dsc;
			this->m_kpC = this->kpCtmp;
			this->imgCenter = this->frametmp;
			//cout << "dscCenter Size: " << dsc.size() << endl;
		}



		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}

		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}




	}
	closesocket(clientSocket);
	closesocket(clientSocket2);

	WSACleanup();
}
void Stitching::getCudaDescriptorsTCP(int port)
{
	std::this_thread::sleep_for(2s);

	SOCKET clientSocket = this->serv.createSocket(port);
	SOCKET clientSocket2 = this->serv.createSocket(80000);

	int rows = 400, cols = 64;
	if (port == 52000) {

	}
	else if (port == 6000) {

	}

	int bytes = 0;
	int bytes1 = 0;
	int bytes2 = 0;
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);
	int buffsize = 1024 * 1024;
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));

	char buf[1024 * 1024*2];
	int bufrow;
	int bufcol;
	vector<float> descriptors;
	int size;
	Mat dscCpu;
	while (true)
	{
		bytes = 0;
		size = this->dscSize;
		descriptors.clear();
		ZeroMemory((char*)&bufrow, sizeof(bufrow));

		//for (int i = 0; i < size; i += bytes)
		//	if ((bytes = recv(clientSocket, (char*)&buf + i, size - i, 0)) == -1) cout << ("recv failed dsc");

		if ((bytes1 = recv(clientSocket2, (char*)&bufrow, sizeof(bufrow), 0)) == -1) cout << ("recv failed dsc");

		ZeroMemory((char*)&bufcol, sizeof(bufcol));

		if ((bytes2 = recv(clientSocket2, (char*)&bufcol, sizeof(bufcol), 0)) == -1) cout << ("recv failed dsc");


		ZeroMemory(buf, sizeof(buf));
		//for (int i = 0; i < size; i += bytes)
		if ((bytes = recv(clientSocket,buf, sizeof(buf), 0)) == -1) cout << ("recv failed dsc");
		float *p;

		//for (p = (float*)&buf[0]; p < (float*)&buf[bytes - 1]; p++) {
		//	descriptors.push_back(*p);
		//	//*p++;
		//}

		for (p = (float*)&buf[0]; p < (float*)&buf[bufcol*bufrow*4]; p++) {
			descriptors.push_back(*p);
			//*p++;
		}
		int cnt = 0;

		dscCpu=Mat::zeros(bufrow, bufcol, CV_32FC1);
		for (int i = 0; i < bufrow; i++) {
			for (int j = 0; j < bufcol; j++) {
				dscCpu.at<float>(i, j) = descriptors[cnt];
				cnt++;
			}
			//cnt+=bufcol;

		}

		this->dscnew1 = dscCpu;

		//imshow("DSC", dscCpu);
		//if (waitKey(10) == 27) break;
		/*int ptr = 0;
		float *p = buf;
		mu4.lock();
		for (int i = 0; i < size; i++) {

			descriptors.push_back(*p++);
		}
		mu4.unlock();*/

		//this->dscCenter = descriptors;
		if (bytes == SOCKET_ERROR) {
			cout << "Error receiving from client" << WSAGetLastError() << endl;
			continue;
		}

		if (port == 60000)
		{
		}
		else if (port == 52000) {
		}



		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}

		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}



	}
	closesocket(clientSocket);

	WSACleanup();
}



void Stitching::getKeyPointsTCP(int port) {
	std::this_thread::sleep_for(1s);
	SOCKET clientSocket = this->serv.createSocket(port);
	this->sock2 = clientSocket;
	int bytes = 0;
	vector<KeyPoint> veckp;
	KeyPoint kp;

	char buf[1024 * 1024*2];
	//KeyPoint buffer[3000];
	float buffer;
	KeyPoint *p;
	Mat imgK;
	int len = 800000;
	while (true)
	{
		vector<KeyPoint> veckp;
		

		ZeroMemory(buf, sizeof(buf));
		//for (int i = 0; i < len; i += bytes)
		if ((bytes = recv(clientSocket, buf, sizeof(buf), 0)) == -1) cout << ("recv3 failed");
		KeyPoint *p;
		for (p = (KeyPoint*)&buf[0]; p <= (KeyPoint*)&buf[bytes-1]; p++) {
			veckp.push_back(*p);
			//*p++;
		}
		p = nullptr;
		string s = "kp fertig";
		int sendkptokp = send(this->sock2, s.c_str(), sizeof(s), 0);
		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}

		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}
		if (port == 53000) {
			this->kpCtmp = veckp;
			//cout << "Kp Size: " << veckp.size() << endl;
			//if (this->imgCenter.empty()) continue;
			//imgK = this->imgCenter.clone();
			//drawKeypoints(imgK, veckp, imgK);
			//cv::imshow("Frame with Center-Keypoints", imgK);
			//if (waitKey(10) == 27)
			//{
			//	break;
			//}
		}
		else if (port == 59000) {

			this->m_kpL = veckp;
			//cout << "m_kpL Size: " << m_kpL.size() << endl;
			//cout << " " << endl;
			//mu2.lock();
			//drawKeypoints(this->imgLeft, this->m_kpL, this->imgLeft);
			//mu2.unlock();
			//cv::imshow("Frame with LeftKeypoints", this->imgLeft);
			//if (waitKey(1000 / 20) >= 0) {
			//	break;
			//}
		}
		/*Mat dsc;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		detector->compute(this->imgCenter, veckp, dsc);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/ 1000000.0 << "[Sekunden]" << std::endl;*/


	}
	closesocket(clientSocket);

	WSACleanup();
}

void Stitching::getFrameTCP(int port, int imgNumber, String windowname) {
	int height = 480;
	int width = 640;
	int bytes = 0;
	SOCKET clientSocket = this->serv.createSocket(port);
	this->sock1 = clientSocket;
	KeyPoint kp;
	char buf[921600];

	while (true)
	{
		Mat img = Mat::zeros(height, width, CV_8UC3);
		int imgSize;
		imgSize = img.total()*img.elemSize();
		ZeroMemory(buf, imgSize);
		//Wait for client to send data
		for (int i = 0; i < imgSize; i += bytes)
			if ((bytes = recv(clientSocket, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		int ptr = 0;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
				ptr = ptr + 3;
				//cout << buf[ptr + 0] << endl << buf[ptr + 1] << endl << buf[ptr + 2] << endl;

			}
		}
		//string s = "frame fertig";
		//int sendframetoframe = send(this->sock1, s.c_str(), sizeof(s), 0);

		if (bytes == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}
		if (bytes == 0)
		{
			cout << "Client disconnected" << endl;
			break;
		}

		if (port == 54000) {
			this->frametmp = img.clone();

		}
		else if (port == 58000) {
			this->imgLeft = img.clone();
		}
	}
	closesocket(clientSocket);

	WSACleanup();
}

Mat Stitching::getAlpha(Mat dst1, Mat dst2) {
	typedef float  Float32;
	typedef long float  Float64;
	Mat alpha = Mat::zeros(Size(dst1.size()), dst1.type());


	for (int r = 0; r < dst1.rows; r++) {
		for (int c = 0; c < dst2.cols; c++) {
			if (r < dst1.rows&&c < dst1.cols)
			{
				alpha.at<float>(r, c) = Float64(dst1.at<float>(r, c) / (dst1.at<float>(r, c) + dst2.at<float>(r, c)) + 0.00000000001);

			}
			else {
				alpha.at<float>(r, c) = 0;
			}
			//cout << "dst2\t" << dst2.at<float>(r, c) << endl;
			//cout << "alpha\t" << alpha.at<float>(r, c) << endl;

		}
	}
	return alpha;
}

Mat Stitching::getBeta(Mat dst1, Mat dst2) {
	typedef float  Float32;
	typedef long float  Float64;
	Mat beta = Mat::zeros(Size(dst1.size()), dst1.type());
	//cout << "dst1\t" << beta.at<float>(dst1.rows-1, dst1.cols-1) << endl;

	for (int r = 0; r < dst1.rows; r++) {
		for (int c = 0; c < dst1.cols; c++) {
			beta.at<float>(dst1.rows - 1 - r, dst1.cols - 1 - c) = Float64(dst1.at<float>(r, c) / (dst1.at<float>(r, c) + dst2.at<float>(r, c) + 0.0000000000000001));

			//cout << "dst1\t" << beta.at<float>(dst1.rows-r-1, dst1.cols-c-1) << endl;
			//cout << "dst2\t" << dst2.at<float>(r, c) << endl;
			//cout << "alpha\t" << alpha.at<float>(r, c) << endl;

		}
	}
	return beta;
}

void Stitching::takeDFT(Mat &source, Mat &destination)
{
	Mat originalComplex[2] = { source, Mat::zeros(source.size(),CV_32F) };
	Mat dftReady;
	merge(originalComplex, 2, dftReady);
	Mat dftOfOriginal;
	dft(dftReady, dftOfOriginal, DFT_COMPLEX_OUTPUT);
	destination = dftOfOriginal;

}

void Stitching::showDFT(Mat &source)
{
	Mat splitArray[2] = { Mat::zeros(source.size(),CV_32F),Mat::zeros(source.size(),CV_32F) };
	split(source, splitArray);
	Mat dftMagnitude;
	magnitude(splitArray[0], splitArray[1], dftMagnitude);
	dftMagnitude += Scalar::all(1);
	log(dftMagnitude, dftMagnitude);
	normalize(dftMagnitude, dftMagnitude, 0, 1, NORM_MINMAX);

}

void Stitching::recenterDFT(Mat &source) {
	int centerX = source.cols / 2;
	int centerY = source.rows / 2;

	Mat q1(source, Rect(0, 0, centerX, centerY));
	Mat q2(source, Rect(centerX, 0, centerX, centerY));
	Mat q3(source, Rect(0, centerY, centerX, centerY));
	Mat q4(source, Rect(centerX, centerY, centerX, centerY));

	Mat swapMap;
	q1.copyTo(swapMap);
	q4.copyTo(q1);
	swapMap.copyTo(q4);

	q2.copyTo(swapMap);
	q3.copyTo(q2);
	swapMap.copyTo(q3);
}

void Stitching::inverseDFT(Mat &source, Mat &destination) {
	Mat inverse;
	dft(source, inverse, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
	destination = inverse;
}

Rect Stitching::findbiggestContour(Mat img) {
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	cvtColor(img, img, COLOR_BGR2GRAY);
	threshold(img, img, 0, 255, THRESH_BINARY);
	findContours(img, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_NONE);
	int largest_area = 0;
	int largest_contour_index = 0;
	Rect rct;
	for (int i = 0; i < contours.size(); i++)
	{
		//  Find the area of contour
		double a = contourArea(contours[i], false);
		if (a > largest_area) {
			largest_area = a;
			cout << i << " area  " << a << endl;
			// Store the index of largest contour
			largest_contour_index = i;
			// Find the bounding rectangle for biggest contour
			rct = boundingRect(contours[largest_contour_index]);
		}
	}
	return rct;
}


vector <KeyPoint> Stitching::getCurrentKeypoints(Mat img) {
	vector <KeyPoint> kp;

	if (norm(img, this->imgLeft, NORM_L1) == false) {
		kp = this->m_kpL;

	}
	else if (norm(img, this->imgRight, NORM_L1) == false) {
		kp = this->m_kpR;
	}
	else if (norm(img, this->imgCenter, NORM_L1) == false) {
		kp = this->m_kpC;
	}
	else if (norm(img, this->imgBottom1, NORM_L1) == false) {
		kp = this->m_kpB1;
	}
	else if (norm(img, this->imgBottom2, NORM_L1) == false) {
		kp = this->m_kpB2;
	}
	else {
		kp = this->m_kptemp;
	}
	return kp;
}

void Stitching::setKeypoints(Mat img, vector <KeyPoint> kp) {

	if (norm(img, this->imgLeft, NORM_L1) == false) {
		this->m_kpL = kp;

	}
	else if (norm(img, this->imgRight, NORM_L1) == false) {
		this->m_kpR = kp;
	}
	else if (norm(img, this->imgCenter, NORM_L1) == false) {

		this->m_kpC = kp;
	}
	else if (norm(img, this->imgBottom1, NORM_L1) == false) {
		this->m_kpB1 = kp;
	}
	else if (norm(img, this->imgBottom2, NORM_L1) == false) {
		this->m_kpB2 = kp;
	}
	else
	{
		this->m_kptemp = kp;
	}

}

Mat Stitching::makeNormalize(Mat img) {

	Mat grayImg, bin, dst;
	if (img.type() != 0) {
		cvtColor(img, grayImg, COLOR_BGR2GRAY);
	}
	else {
		grayImg = img;
	}

	//removeBlackPoints(grayImg);

	cuda::GpuMat grayImgGPU, binGPU, dstGPU;
	grayImgGPU.upload(grayImg);
	cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);
	//threshold(grayImg, bin, 0, 255, THRESH_BINARY);
	binGPU.download(bin);
	//imshow("bin", bin);
	//Mat kernel = Mat::ones(Size(55, 55), dst.type());
	//dilate(bin, bin, kernel, Point(-1, -1), 2);

	distanceTransform(bin, dst, DIST_L2, 3.0);
	double min, max;
	cv::minMaxLoc(bin, &min, &max);
	//imshow("dst", dst);
	//waitKey(0);
	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));

	//erode(dst, dst, kernel, Point(-1, -1), 2);
	dstGPU.upload(dst);
	/*double alpha = 0.;
	cuda::normalize(dstGPU, dstGPU, alpha, (1.-alpha), NORM_MINMAX,-1);*/


	dstGPU.download(dst);


	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));
	//dilate(dst, dst, kernel,Point(-1,-1));

	return dst;
}

vector<DMatch> Stitching::getSiftmatches(Mat &img1, Mat &img2, float a)
{

	vector <KeyPoint> kp1, kp2;
	Mat siftdescriptor1, siftdescriptor2;
	Ptr<SIFT> sift = SIFT::create();

	sift->detectAndCompute(img1, noArray(), kp1, siftdescriptor1);
	sift->detectAndCompute(img2, noArray(), kp2, siftdescriptor2);

	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> matcher11;
	matcher11 = BFMatcher::create(NORM_L2, false);
	vector<vector<DMatch>> matchesknn, matchesknn2;
	matcher11->knnMatch(siftdescriptor1, siftdescriptor2, matchesknn, 2);

	vector<DMatch> mt1;

	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}


	setKeypoints(img1, kp1);
	setKeypoints(img2, kp2);
	return mt1;

}


Mat Stitching::getHomography(Mat img1, Mat img2, vector <DMatch> matches)
{
	vector<KeyPoint> kpObj, kpScn;
	kpObj = getCurrentKeypoints(img1);
	kpScn = getCurrentKeypoints(img2);
	vector<Point2f> obj, scene;
	for (int i = 0; i < matches.size(); i++) {
		if ((matches[i].queryIdx < kpObj.size()) && (matches[i].trainIdx < kpScn.size())) {
			obj.push_back(kpObj[matches[i].queryIdx].pt);
			scene.push_back(kpScn[matches[i].trainIdx].pt);
		}

	}
	Mat H = findHomography(scene, obj, RANSAC);
	return H;
}


void Stitching::VideoStream(String path, String windowname, double fps, Mat &frame) {
	VideoCapture cap(path);

	while (cap.isOpened()) {
		cap.read(frame);
		resize(frame, frame, Size(848, 480));
		namedWindow(windowname, WINDOW_FREERATIO);

		imshow(windowname, frame);

		if (waitKey(1000 / fps) >= 0) {
			break;
		}
	}
}



void Stitching::CudaStitch(cuda::GpuMat &img1, cuda::GpuMat &img2, cuda::GpuMat &img3, cuda::GpuMat &img4) {
	for (;;) {
		this_thread::sleep_for(5s);

		while (true) {
			//mu.lock();
			Mat img2CPU;
			img2.download(img2CPU);
			Mat centerTemplate = Mat::zeros(Size(img2CPU.size()), img2CPU.type());
			cuda::GpuMat centerTemplateGPU;
			centerTemplateGPU.upload(centerTemplate);
			cuda::GpuMat roiCenter(centerTemplateGPU, Rect(0, 0, img1.cols, img1.rows));
			img1.copyTo(roiCenter);
			Rect cropLast(imgCenterGPU.cols / 5, imgCenterGPU.rows / 1.4, imgCenterGPU.cols / 1.5, imgCenterGPU.rows - imgCenterGPU.rows / 1.35);

			centerTemplateGPU.download(centerTemplate);

			//for (int r = 0; r < centerTemplate.rows; r++) {
			//	for (int c = 0; c < centerTemplate.cols; c++) {
			//		if (cropLast.contains(Point(c, r)) == true) {
			//			centerTemplate.at<Vec3b>(r, c) = Vec3b(0, 0, 0);
			//		}
			//	}
			//}

			Mat dstcenter = makeNormalize(centerTemplate);
			Mat dstright = makeNormalize(img2CPU);

			Mat alpha1 = getAlpha(dstcenter, dstright);
			Mat firstpano = Mat::zeros(Size(centerTemplate.size()), centerTemplate.type());

			//Erstes Alpha Blending (Mittleres und linkes Bild)
			for (int r = 0; r < dstcenter.rows; r++) {
				for (int c = 0; c < dstcenter.cols; c++) {

					//firstpano.at<uchar>(r, c) = alpha1.at<float>(r, c) * (int)upgray.at<uchar>(r, c) + (1. - alpha1.at<float>(r, c)) * (int)lowgray.at<uchar>(r, c);
					firstpano.at<Vec3b>(r, c)[0] = alpha1.at<float>(r, c) * centerTemplate.at<Vec3b>(r, c)[0] + (1. - alpha1.at<float>(r, c)) * img2CPU.at<Vec3b>(r, c)[0];
					firstpano.at<Vec3b>(r, c)[1] = alpha1.at<float>(r, c) * centerTemplate.at<Vec3b>(r, c)[1] + (1. - alpha1.at<float>(r, c)) * img2CPU.at<Vec3b>(r, c)[1];
					firstpano.at<Vec3b>(r, c)[2] = alpha1.at<float>(r, c) * centerTemplate.at<Vec3b>(r, c)[2] + (1. - alpha1.at<float>(r, c)) * img2CPU.at<Vec3b>(r, c)[2];

				}
			}
			centerTemplateGPU.upload(centerTemplate);
			//cuda::GpuMat firstpanoGPU;
			//firstpanoGPU.upload(firstpano);
			Mat img3CPU;
			img3.download(img3CPU);
			Mat dstleft = makeNormalize(img3CPU);
			Mat fpano = makeNormalize(firstpano);
			Mat alpha2 = getAlpha(fpano, dstleft);
			Mat secondpano = Mat::zeros(Size(centerTemplate.size()), centerTemplate.type());
			for (int r = 0; r < dstright.rows; r++) {
				for (int c = 0; c < dstright.cols; c++) {

					//secondpano.at<uchar>(r, c) = alpha2.at<float>(r, c) * (int)firstpano.at<uchar>(r, c) + (1. - alpha2.at<float>(r, c)) * (int)graywarpRight.at<uchar>(r, c);
					secondpano.at<Vec3b>(r, c)[0] = alpha2.at<float>(r, c) * firstpano.at<Vec3b>(r, c)[0] + (1. - alpha2.at<float>(r, c)) * img3CPU.at<Vec3b>(r, c)[0];
					secondpano.at<Vec3b>(r, c)[1] = alpha2.at<float>(r, c) * firstpano.at<Vec3b>(r, c)[1] + (1. - alpha2.at<float>(r, c)) * img3CPU.at<Vec3b>(r, c)[1];
					secondpano.at<Vec3b>(r, c)[2] = alpha2.at<float>(r, c) * firstpano.at<Vec3b>(r, c)[2] + (1. - alpha2.at<float>(r, c)) * img3CPU.at<Vec3b>(r, c)[2];

				}
			}
			cuda::GpuMat secondpanoGPU;
			secondpanoGPU.upload(secondpano);
			Mat img4CPU;
			img3.download(img4CPU);
			Mat dstdown = makeNormalize(img4CPU);
			normalize(dstdown, dstdown, 0., 1., NORM_MINMAX);
			Mat fpano2 = makeNormalize(secondpano);
			Mat alpha3 = getAlpha(fpano2, dstdown);
			Mat thirdpano = Mat::zeros(Size(centerTemplate.size()), centerTemplate.type());
			for (int r = 0; r < dstright.rows; r++) {
				for (int c = 0; c < dstright.cols; c++) {

					//thirdpano.at<uchar>(r, c) = alpha3.at<float>(r, c) * (int)secondpano.at<uchar>(r, c) + (1. - alpha3.at<float>(r, c)) * (int)graywarpDown.at<uchar>(r, c);
					thirdpano.at<Vec3b>(r, c)[0] = alpha3.at<float>(r, c) * secondpano.at<Vec3b>(r, c)[0] + (1. - alpha3.at<float>(r, c)) * img4CPU.at<Vec3b>(r, c)[0];
					thirdpano.at<Vec3b>(r, c)[1] = alpha3.at<float>(r, c) * secondpano.at<Vec3b>(r, c)[1] + (1. - alpha3.at<float>(r, c)) * img4CPU.at<Vec3b>(r, c)[1];
					thirdpano.at<Vec3b>(r, c)[2] = alpha3.at<float>(r, c) * secondpano.at<Vec3b>(r, c)[2] + (1. - alpha3.at<float>(r, c)) * img4CPU.at<Vec3b>(r, c)[2];

				}
			}
			cuda::GpuMat thirdpanoGPU;
			thirdpanoGPU.upload(thirdpano);
			//mu.unlock();

			namedWindow("gestitched", WINDOW_OPENGL);
			imshow("gestitched", thirdpanoGPU);
			if (waitKey(1000 / 20) >= 0) {
				break;
			}
		}
	}
}

vector<DMatch> Stitching::getSiftmatchesFlann(Mat img1, Mat img2, float ratio_thresh)
{
	vector <KeyPoint> kp1, kp2;
	Mat siftdescriptor1, siftdescriptor2;
	Ptr<SIFT> sift = SIFT::create();
	sift->detectAndCompute(img1, noArray(), kp1, siftdescriptor1);
	sift->detectAndCompute(img2, noArray(), kp2, siftdescriptor2);
	int FLANN_INDEX_KDTREE = 0;

	Ptr<FlannBasedMatcher> flann = FlannBasedMatcher::create();

	//Ptr<cv::DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
	vector<vector<DMatch>> matchesknn;
	flann->knnMatch(siftdescriptor1, siftdescriptor2, matchesknn, 2);


	std::vector<DMatch> good_matches;
	for (size_t i = 0; i < matchesknn.size(); i++)
	{
		if (matchesknn[i][0].distance < ratio_thresh * matchesknn[i][1].distance)
		{
			good_matches.push_back(matchesknn[i][0]);
		}
	}

	this->m_kpL = kp1;
	this->m_kpR = kp2;
	setKeypoints(img1, kp1);
	setKeypoints(img2, kp2);
	return good_matches;

}

Mat Stitching::AlphaBlending(Mat img, Mat addImg) {

	Mat dst1 = makeNormalize(img);
	Mat dst2 = makeNormalize(addImg);
	Mat alpha = getAlpha(dst1, dst2);
	//Mat beta = getBeta(dst1, dst2);
	Mat Pano = Mat::zeros(Size(img.size()), img.type());

	//Erstes Alpha Blending (Mittleres und linkes Bild)
	for (int r = 0; r < dst1.rows; r++)
	{
		for (int c = 0; c < dst1.cols; c++)
		{

			//firstpano.at<uchar>(r, c) = alpha1.at<float>(r, c) * (int)upgray.at<uchar>(r, c) + (1. - alpha1.at<float>(r, c)) * (int)lowgray.at<uchar>(r, c);
			Pano.at<Vec3b>(r, c)[0] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[0] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[0];
			Pano.at<Vec3b>(r, c)[1] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[1] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[1];
			Pano.at<Vec3b>(r, c)[2] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[2] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[2];

		}
	}
	return Pano;
}
void Stitching::StitchAllImgs(cuda::GpuMat &warpBottom1GPU, cuda::GpuMat &warpBottom2GPU, cuda::GpuMat &warpRightGPU, cuda::GpuMat &warpLeftGPU)
{
	this_thread::sleep_for(3s);
	while (true)
	{
		/*	this->imgCenterGPU.download(this->imgCenter);
			Rect cropLast(this->imgCenter.cols / 5, this->imgCenter.rows / 1.4, this->imgCenter.cols / 1.5, this->imgCenter.rows - this->imgCenter.rows / 1.4);
			Mat centerTemplate = Mat::zeros(Size(this->imgCenter.size()*2), this->imgCenter.type());
			Mat roi(centerTemplate, Rect(0, 0, this->imgCenter.cols, this->imgCenter.rows));
			this->imgCenter.copyTo(roi);
	*/
	//for (int r = 0; r < centerTemplate.rows; r++) {
	//	for (int c = 0; c < centerTemplate.cols; c++) {
	//		if (cropLast.contains(Point(c, r)) == true) {
	//			centerTemplate.at<Vec3b>(r, c) = Vec3b(0, 0, 0);
	//		}
	//	}
	//}

	//#########################################################STITCHING###########################################################//

		Mat wBottom1, wBottom2, wRight, wLeft;
		warpBottom1GPU.download(wBottom1);
		warpBottom2GPU.download(wBottom2);
		warpRightGPU.download(wRight);
		warpLeftGPU.download(wLeft);
		Mat centerTemplate = this->imgCenter;
		Mat fstPano = AlphaBlending(centerTemplate, wBottom1);
		Mat scnPano = AlphaBlending(fstPano, wBottom2);
		Mat thdPano = AlphaBlending(scnPano, wRight);
		lastPano = AlphaBlending(thdPano, wLeft);
		cuda::GpuMat lastPanoGPU;
		lastPanoGPU.upload(lastPano);

		namedWindow("gestitched", WINDOW_OPENGL);
		imshow("gestitched", lastPanoGPU);
		if (waitKey(1000 / 60) >= 0) {
			break;
		}
		wBottom1.release();
		wBottom2.release();

		wRight.release();
		wLeft.release();

	}
}

void Stitching::streamRealSense()
{
	rs2::context ctx;        // Create librealsense context for managing devices

	std::map<std::string, rs2::colorizer> colorizers; // Declare map from device serial number to colorizer (utility class to convert depth data RGB colorspace)

	std::vector<rs2::pipeline>  pipelines;

	// Capture serial numbers before opening streaming
	std::vector<std::string>              serials;
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}


	rs2::pipeline pipe(ctx), pipe2(ctx), pipe3(ctx), pipe4(ctx), pipe5(ctx);
	rs2::config cfg, cfg2, cfg3, cfg4, cfg5;
	cfg.enable_device(serials[0]);
	cfg2.enable_device(serials[1]);
	cfg3.enable_device(serials[2]);
	cfg4.enable_device(serials[3]);
	cfg5.enable_device(serials[4]);

	cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg2.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg3.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg4.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);
	cfg5.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);

	pipe.start(cfg);
	pipe2.start(cfg2);
	pipe3.start(cfg3);
	pipe4.start(cfg4);
	pipe5.start(cfg5);

	pipelines.emplace_back(pipe);
	pipelines.emplace_back(pipe2);
	pipelines.emplace_back(pipe3);
	pipelines.emplace_back(pipe4);
	pipelines.emplace_back(pipe5);

	colorizers[serials[0]] = rs2::colorizer();
	colorizers[serials[1]] = rs2::colorizer();
	colorizers[serials[2]] = rs2::colorizer();
	colorizers[serials[3]] = rs2::colorizer();
	colorizers[serials[4]] = rs2::colorizer();

	std::map<int, rs2::frame> render_frames;
	rs2::frameset framesOCV;
	rs2::frame color_frame0;
	rs2::frame color_frame1;
	rs2::frame color_frame2;
	rs2::frame color_frame3;
	rs2::frame color_frame4;
	VideoWriter video("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam0.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video1("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam1.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video2("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam2.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video3("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam3.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));
	VideoWriter video4("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\cam4.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, Size(640, 480));

	while (true) {
		color_frame0 = pipelines[0].wait_for_frames();
		color_frame1 = pipelines[1].wait_for_frames();
		color_frame2 = pipelines[2].wait_for_frames();
		color_frame3 = pipelines[3].wait_for_frames();
		color_frame4 = pipelines[4].wait_for_frames();

		Mat color0(Size(640, 480), CV_8UC3, (void*)color_frame0.get_data(), Mat::AUTO_STEP);
		Mat color1(Size(640, 480), CV_8UC3, (void*)color_frame1.get_data(), Mat::AUTO_STEP);
		Mat color2(Size(640, 480), CV_8UC3, (void*)color_frame2.get_data(), Mat::AUTO_STEP);
		Mat color3(Size(640, 480), CV_8UC3, (void*)color_frame3.get_data(), Mat::AUTO_STEP);
		Mat color4(Size(640, 480), CV_8UC3, (void*)color_frame4.get_data(), Mat::AUTO_STEP);
		video.write(color0);
		video1.write(color1);
		video2.write(color2);
		video3.write(color3);
		video4.write(color4);

		//this->imgLeftGPU.upload(color0);
		//this->imgRightGPU.upload(color1);
		//this->imgRightGPU.upload(color2);
		//this->imgRightGPU.upload(color3);
		//this->imgRightGPU.upload(color4);

		namedWindow("cam0", WINDOW_FREERATIO);
		namedWindow("cam1", WINDOW_FREERATIO);
		namedWindow("cam2", WINDOW_FREERATIO);
		namedWindow("cam3", WINDOW_FREERATIO);
		namedWindow("cam4", WINDOW_FREERATIO);

		imshow("cam0", color0);
		imshow("cam1", color1);
		imshow("cam2", color2);
		imshow("cam3", color3);
		imshow("cam4", color4);

		if (waitKey(1000 / 20) >= 0) {
			break;
		}
	}
	
}


vector<Mat> Stitching::takeCapture()
{
	Size sz1 = Size(848, 480);
	Size sz2 = Size(640, 480);
	rs2::context ctx;        // Create librealsense context for managing devices
	std::vector<std::string> serials;
	rs2::rates_printer printer;
	rs2::colorizer color_map;

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}
	vector<rs2::config> cfg;
	for (int i = 0; i < serials.size(); i++) {
		rs2::config cfgtemp;
		cfgtemp.enable_device(serials[i]);
		cfgtemp.enable_stream(RS2_STREAM_COLOR, sz2.width, sz2.height, RS2_FORMAT_BGR8, 30);
		cfg.push_back(cfgtemp);
	}

	vector<Mat> imgs;
	rs2::pipeline pipe1, pipe2;
	std::vector<rs2::pipeline>  pipelines;
	for (int i = 0; i < cfg.size(); i++) {
		rs2::pipeline pipe;
		pipelines.push_back(pipe);
	}

	for (int i = 0; i < pipelines.size(); i++) {


		pipelines[i].start(cfg[i]);
	}
	////Instruct pipeline to start streaming with the requested configuration
	//pipe1.start(cfg1);
	//pipe2.start(cfg2);
	// Camera warmup - dropping several first frames to let auto-exposure stabilize
	rs2::frameset frames1, frames2, frames3, frames4, frames5;
	vector<rs2::frameset> frame;
	for (int i = 0; i < 30; i++)
	{
		frames1 = pipelines.at(0).wait_for_frames();
		frames2 = pipelines.at(1).wait_for_frames();
		frames3 = pipelines.at(2).wait_for_frames();
		frames4 = pipelines.at(3).wait_for_frames();
		frames5 = pipelines.at(4).wait_for_frames();

	}

	//Get each frame
	rs2::frame color_frame1 = frames1.get_color_frame();
	rs2::frame color_frame2 = frames2.get_color_frame();
	rs2::frame color_frame3 = frames3.get_color_frame();
	rs2::frame color_frame4 = frames4.get_color_frame();
	rs2::frame color_frame5 = frames5.get_color_frame();

	// Creating OpenCV Matrix from a color image
	Mat color1(sz2, CV_8UC3, (void*)color_frame1.get_data(), Mat::AUTO_STEP);
	Mat color2(sz2, CV_8UC3, (void*)color_frame2.get_data(), Mat::AUTO_STEP);
	Mat color3(sz2, CV_8UC3, (void*)color_frame3.get_data(), Mat::AUTO_STEP);
	Mat color4(sz2, CV_8UC3, (void*)color_frame4.get_data(), Mat::AUTO_STEP);
	Mat color5(sz2, CV_8UC3, (void*)color_frame5.get_data(), Mat::AUTO_STEP);

	// Display in a GUI
	//namedWindow("Display Image1", WINDOW_AUTOSIZE);
	//imshow("Display Image1", color1);
	//waitKey(0);
	//imshow("Display Image2", color2);
	//imshow("Display Image3", color3);
	//imshow("Display Image4", color4);
	//imshow("Display Image5", color5);

	//waitKey(0);
	imgs.push_back(color1); //Left
	imgs.push_back(color2); //Right
	imgs.push_back(color3);	//BottomLeft
	imgs.push_back(color4);	//Center
	imgs.push_back(color5);	//BottomRight
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\LeftUp4.png", color1);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\RightUp4.png", color2);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\centerUp4.png", color3);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\BottomLeftUp4.png", color4);
	imwrite("C:\\Users\\fmosh\\OneDrive\\Dokumente\\WiSe_2021\\Masterarbeit\\AutomatisierteAufnahmen\\BottomRightUp4.png", color5);
	cout << "FERTIG" << endl;
	return imgs;
}



void Stitching::getFrameRS()
{

	int width = 640;
	int height = 480;
	Size sz = Size(640, 480);
	rs2::pipeline pipe;
	rs2::config cfg;
	cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, 15);
	pipe.start(cfg);

	rs2::frameset frames;
	for (int i = 0; i < 30; i++)
	{
		frames = pipe.wait_for_frames();
	}
	while (true) {
		//if (this->rs == true) continue;
		frames = pipe.wait_for_frames();
		rs2::frame color_frame = frames.get_color_frame();
		Mat color(sz, CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
		Mat imgSURF = color;
		if (imgSURF.empty()) continue;
		Ptr<SURF> detector = SURF::create(100);
		std::vector<KeyPoint> kp;
		Mat dsc, out;
		detector->detectAndCompute(imgSURF, noArray(), kp, dsc);
		this->kptmp = kp;
		this->dsctmp = dsc;
		this->img = color;
		dsc.release();
		kp.clear();
		//this->rs = true;
		
	}


}

void Stitching::getFeaturesRS()
{
	while (true) {
		if (this->features == false) continue;
		Mat imgSURF = this->img;
		if (imgSURF.empty()) continue;
		Ptr<SURF> detector = SURF::create(100);
		std::vector<KeyPoint> kp;
		Mat dsc,out;
		detector->detectAndCompute(imgSURF, noArray(), kp, dsc);
		this->kptmp = kp;
		this->dsctmp = dsc;
		this->features = false;

	}
}

vector<DMatch> Stitching::getSureMatches(Mat imgCPU1, Mat imgCPU2, double a)
{
	Mat imgGray1, imgGray2;

	cvtColor(imgCPU1, imgGray1, COLOR_BGR2GRAY);
	cvtColor(imgCPU2, imgGray2, COLOR_BGR2GRAY);
	//Ptr<cuda::SURF_CUDA> surf;
	cuda::SURF_CUDA surf;
	// detecting keypoints & computing descriptors
	cuda::GpuMat img1, img2, keypoints1GPU, keypoints2GPU;
	cuda::GpuMat descriptors1GPU, descriptors2GPU;
	img1.upload(imgGray1);
	img2.upload(imgGray2);
	surf(img1, cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
	surf(img2, cuda::GpuMat(), keypoints2GPU, descriptors2GPU);
	vector<float> dsc;
	surf.downloadDescriptors(descriptors1GPU, dsc);
	
	//	float *p = dsc.data();
	//	for (int i = 0; i < dsc.size(); i++) {
	//	cout << *p++ << endl;
	//	if (i == 20) {
	//		break;
	//
	//	}
	//}
	//	cout << "Richtiger Vektor" << endl;
	//	for (int i = 0; i < dsc.size(); i++) {
	//		cout << dsc[i] << endl;
	//		if (i == 20) {
	//			break;
	//		}
	//	}

		//surf->detectWithDescriptors(img1, cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
		//surf->detectWithDescriptors(img2, cuda::GpuMat(), keypoints2GPU, descriptors2GPU);
	/*
		Ptr<cv::cuda::DescriptorMatcher> matcher = cuda::DescriptorMatcher::createBFMatcher(surf.defaultNorm());
		vector<DMatch> matches;
		matcher->match(descriptors1GPU, descriptors2GPU, matches);
		vector<KeyPoint> keypoints1, keypoints2;
		vector<float> descriptors1, descriptors2;
		surf.downloadKeypoints(keypoints1GPU, keypoints1);
		surf.downloadKeypoints(keypoints2GPU, keypoints2);
		surf.downloadDescriptors(descriptors1GPU, descriptors1);
		surf.downloadDescriptors(descriptors2GPU, descriptors2);
		Mat img_matches;
		vector<DMatch> mt1;
		vector<vector<DMatch>> matchesknn;
		matcher->knnMatch(descriptors1GPU, descriptors2GPU, matchesknn, 2);
		for (int i = 0; i < matchesknn.size(); i++) {
			if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
				mt1.push_back(matchesknn[i][0]);
			}
		}*/
		//drawMatches(Mat(img1), keypoints1, Mat(img2), keypoints2, mt1, img_matches);

		//imwrite("seaman_result.jpg", img_matches);

		//namedWindow("matches", 0);
		//imshow("matches", img_matches);
		//waitKey(0);

		//cuda::resetDevice();
		/*vector <KeyPoint> kp1, kp2;
		Mat suredescriptor1, suredescriptor2;
		Ptr<cuda::SURF_CUDA> surf = cuda::SURF_CUDA::create(200);
		vector<DescriptorMatcher> dsc;
		Ptr<cv::DescriptorMatcher> matcher11;
		matcher11 = BFMatcher::create(NORM_L2, false);
		vector<vector<DMatch>> matchesknn, matchesknn2;
		matcher11->knnMatch(siftdescriptor1, siftdescriptor2, matchesknn, 2);
		vector<DMatch> mt1;
		for (int i = 0; i < matchesknn.size(); i++) {
			if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
				mt1.push_back(matchesknn[i][0]);
			}
		}*/


		//setKeypoints(imgCPU1, keypoints1);
		//setKeypoints(imgCPU2, keypoints2);
	return mt1;
}

vector<DMatch> Stitching::getSurfMatches(Mat img1, Mat img2, double a, double b, int minHessian)
{
	Mat imgGray1, imgGray2;
	cvtColor(img1, imgGray1, COLOR_BGR2GRAY);
	cvtColor(img2, imgGray2, COLOR_BGR2GRAY);
	Ptr<SURF> detector = SURF::create(minHessian);

	std::vector<KeyPoint> kp1, kp2;
	Mat surfdescriptor1, surfdescriptor2;

	
	detector->detectAndCompute(imgGray1, noArray(), kp1, surfdescriptor1);
	detector->detectAndCompute(imgGray2, noArray(), kp2, surfdescriptor2);


	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> matcher11;
	matcher11 = BFMatcher::create(NORM_L2, false);
	vector<vector<DMatch>> matchesknn, matchesknn2;
	matcher11->knnMatch(surfdescriptor1, surfdescriptor2, matchesknn, 2);

	vector<DMatch> mt1;

	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}
	setKeypoints(img1, kp1);
	setKeypoints(img2, kp2);
	if (b != 0)
	{

		Mat matchesimg1, matachesimg2;
		drawMatches(img1, kp1, img2, kp2, mt1, matchesimg1);
		namedWindow("matches", WINDOW_FREERATIO);
		imshow("matches", matchesimg1);
		waitKey(0);
	}
	return mt1;

}



Rect Stitching::lastRct(Mat img, int a) {
	Mat blurCenter, centerGray, centerthresh, image;
	medianBlur(img, blurCenter, 185);
	cvtColor(blurCenter, centerGray, COLOR_BGR2GRAY);
	threshold(centerGray, centerthresh, 164, 255, THRESH_BINARY);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(centerthresh, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
	int largest_area = 0;
	int largest_contour_index = 0;
	for (int i = 0; i < contours.size(); i++) {
		if (hierarchy[i][3] == -1) {
			//drawContours(centerTemplate, contours, i, (255, 0, 0), 10);
		}
		double af = contourArea(contours[i], false);
		if (af > largest_area) {
			largest_area = af;
			cout << i << " area  " << af << endl;
			// Store the index of largest contour
			largest_contour_index = i;
		}
	}
	Rect rct12 = boundingRect(contours[largest_contour_index]);
	rectangle(img, rct12, Scalar(255, 255, 255), 10);
	if (a == 1) {
		imshow("Rechteck um Last", img);
		waitKey(0);
	}
	return rct12;
}

void Stitching::cropImg(Mat &img, Rect rct, int d)
{

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			int dX = c - rct.x;
			int dY = r - rct.y;
			int dX2 = rct.width - c;
			int dY2 = rct.height - r;

			if (((dX > d&&dY > d) && (dX2 < d&&dY2 < d)) && rct.contains(Point(c, r))) {
				img.at<Vec3b>(r, c) = Vec3b(0, 0, 0);
			}
		}
	}
}

void Stitching::removeBlackPoints(Mat &img)
{
	vector<Point> nonBlackList2;
	nonBlackList2.reserve((int)img.rows + (int)img.cols);

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			if ((int)img.at<uchar>(r, c) != 0) {

				nonBlackList2.push_back(Point(c, r));
			}
		}
	}
	Rect nonBlacklistRect = boundingRect(nonBlackList2);
	img = img(nonBlacklistRect);

}