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


void Stitching::serialTCP(int port1, int port2, int port3, int port4)
{
	server serv;
	SOCKET clientsocket1 = serv.createSocket(port1);
	SOCKET clientsocket2 = serv.createSocket(port2);
	SOCKET clientsocket3 = serv.createSocket(port3);


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
	int kpBufSize = 1024 * 1024 * 2;
	KeyPoint* p;

	int buffsize = 1024 * 1024 * 2;
	setsockopt(clientsocket3, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	int bufrow;
	int bufcol;
	char bufdsc[1024 * 1024 * 2];
	int dscbufSize = 1024 * 1024 * 2;
	int dscSize;
	Mat dsc;

	while (true)
	{
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		Mat img = Mat::zeros(height, width, CV_8UC3);
		int imgSize;
		imgSize = img.total() * img.elemSize();
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

		ZeroMemory(bufkp, kpBufSize);
		//for (int i = 0; i < len; i += bytes)
		if ((bytes1 = recv(clientsocket2, bufkp, kpBufSize, 0)) == -1) cout << ("recv3 failed");
		KeyPoint* p;

		for (p = (KeyPoint*)&bufkp[0]; p <= (KeyPoint*)&bufkp[bytes1 - 1]; p++) {
			veckp.push_back(*p);
		}
		p = nullptr;
		/////////////////////////////////////////////////////////////////////////////////////////

		ZeroMemory(bufdsc, dscbufSize);
		//Wait for client to send data
	


		if ((bytes3 = recv(clientsocket3, bufdsc, dscbufSize, 0)) == -1) cout << ("recv failed");
		float* pdsc = (float*)bufdsc;
		bufcol = 64;
		bufrow = bytes3 / (64 * 4);
		dsc = Mat::zeros(bufrow, bufcol, CV_32FC1);
		for (int i = 0; i < bufrow; i++) {
			for (int j = 0; j < bufcol; j++) {
				dsc.at<float>(i, j) = *pdsc;
				pdsc++;
			}
		}
		pdsc = nullptr;
		if (bytes == SOCKET_ERROR || bytes1 == SOCKET_ERROR || bytes3 == SOCKET_ERROR)
		{
			cerr << "Error in recv().Quitting" << endl;
			break;
		}
		if (bytes == 0 || bytes1 == 0 || bytes3 == 0)
		{
			cout << "Client disconnected" << endl;
			cout << "bytes: " << bytes << endl;
			cout << "bytes1: " << bytes1 << endl;
			cout << "bytes3: " << bytes3 << endl;

			break;
		}
		string s = "angekommen";
		int sendresult = send(clientsocket3, s.c_str(), sizeof(s), 0);

		if (port1 == 54000) {
			this->imgRight = img.clone();
			this->m_kpR = veckp;
			this->dscRight = dsc;
			/*cout << "vektor" <<veckp.size() << endl;
			cout << "dsc "<< dsc.size() << endl;
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[탎]" << std::endl;*/

		}
		else if (port1 == 58000) {
			this->imgLeft = img.clone();
			this->m_kpL = veckp;
			this->dscLeft = dsc;
		}
		else if (port1 == 40000) {
			this->imgCenter = img.clone();
			this->m_kpC = veckp;
			this->dscCenter = dsc;
		}
		else if (port1 == 30000) {
			this->imgBottom1 = img.clone();
			this->m_kpB1 = veckp;
			this->dscBottom = dsc;
		}

	}
	closesocket(clientsocket1);
	closesocket(clientsocket2);
	closesocket(clientsocket3);

	WSACleanup();

}



void Stitching::CamFeatures() {
	VideoCapture cap;
	// open the default camera, use something different from 0 otherwise;
	// Check VideoCapture documentation.
	Mat imgGray1, dsccpuc;
	//cuda::SURF_CUDA surf;
	//cuda::GpuMat img1, kpGPU, dscGPU;
	Ptr<SURF> detector = SURF::create(400);
	vector<KeyPoint> kp;
	Mat dsc;
	Mat outK, out, tmp;
	if (!cap.open(0))
		cout << "can't open Webcam" << endl;
	for (;;)
	{
		cap >> tmp;
		resize(tmp, tmp, Size(640, 480));
		this->camFrame = tmp;

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
	server serv;
	SOCKET clientSocket1 = serv.createSocket(port1);
	SOCKET clientSocket2 = serv.createSocket(port2);
	SOCKET clientSocket3 = serv.createSocket(port3);
	int bytes = 0;
	vector<KeyPoint> veckp;
	KeyPoint kp;

	char buf[1024 * 1024 * 2];
	//KeyPoint buffer[3000];
	float buffer;
	KeyPoint* p;
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
		KeyPoint* p;
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


		if ((bytes1 = recv(clientSocket2, bufdsc, bufcol * bufrow * 4, 0)) == -1) cout << ("recv failed");

		float* pd = (float*)bufdsc;
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

void Stitching::getHomoGraphyTCP(int port) {
	server serv;
	SOCKET clientSocket1=serv.createSocket(port);
	int port2 = port + 1500;
	SOCKET clientSocket2 = serv.createSocket(port2);
	char buf[1000];
	int bytes = 0;
	while (true) {
		cuda::GpuMat dsc1GPU, dsc2GPU;
		vector<KeyPoint> kp1 = this->m_kpC;
		if (this->dscCenter.empty() || this->dscRight.empty()||kp1.empty()) continue;

		dsc1GPU.upload(this->dscCenter);
		dsc2GPU.upload(this->dscRight);
		Ptr<cuda::DescriptorMatcher> matcherGPU;
		matcherGPU = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
		vector<vector<DMatch>> matches;
		matcherGPU->knnMatch(dsc1GPU, dsc2GPU, matches, 2);
		vector<DMatch> mt1;
		cout << "SA" << endl;
		if (!matches.empty()) {
			for (int i = 0; i < matches.size(); i++) {
				if (matches[i][0].distance < 0.8 * matches[i][1].distance) {
					mt1.push_back(matches[i][0]);
				}
			}
		}
		else cout << "matches empty" << endl;


		int sendMatches = send(clientSocket1, (char*)&mt1[0],sizeof(DMatch)*mt1.size(),0);
		bytes = recv(clientSocket1, buf, 1000, 0);
		int kpSize = kp1.size();
		int sendKp = send(clientSocket2, (char*)&kp1[0], kpSize, 0);
		cout << "kp size "<<kp1.size() << endl;
		cout << "bytes" << sendMatches << endl;

	}
	closesocket(clientSocket1);
	closesocket(clientSocket2);


}

void Stitching::warpThread(Mat dsc1,Mat dsc2, vector<KeyPoint> kp1, vector<KeyPoint> kp2,int &a,Mat img,Mat &warpImg,Mat &H) {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Stitching s;
	int tmp = a;
	cuda::GpuMat dsc1GPU, dsc2GPU;
	dsc1GPU.upload(dsc1);
	dsc2GPU.upload(dsc2);
	Ptr<cuda::DescriptorMatcher> matcherGPU;
	matcherGPU = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	Ptr<cv::DescriptorMatcher> matcher;
	vector<vector<DMatch>> matches;
	matcher = BFMatcher::create(NORM_L2, false);
	matcherGPU->knnMatch(dsc1GPU, dsc2GPU, matches, 2);
	vector<DMatch> mt1;
	//matcher->knnMatch(dsc1, dsc2, matches, 2);

	if (!matches.empty()) {
		for (int i = 0; i < matches.size(); i++) {
			if (matches[i][0].distance < 0.8 * matches[i][1].distance) {
				mt1.push_back(matches[i][0]);
			}
		}
	}
	else {
		cout << "matches are empty" << endl;
	}


	if (mt1.size()<10) {
		a = 0;
	}
	else {
		H = s.getHomography(mt1, kp2, kp1);
		std::chrono::steady_clock::time_point step5 = std::chrono::steady_clock::now();
		std::cout << "        Time difference homography on thread" << a << " = " << std::chrono::duration_cast<std::chrono::microseconds>(step5 - begin).count() << "[탎]" << std::endl;
	}



	if (H.empty()&&tmp!=10) { 
		cout << "Homopgrahy in thread " << tmp << "is empty" << endl;
		a = 0; 
	} 
	else {
		Mat Template = Mat::zeros(Size(img.size() * 2), img.type());

		Mat roi(Template, Rect(img.cols / 2, img.rows / 2, img.cols, img.rows));
		img.copyTo(roi);
		cuda::GpuMat TemplateGPU, warpImgGPU, HGPU;
		TemplateGPU.upload(Template);
		//warpImgGPU.upload(warpImg);
		
		//warpPerspective(Template, warpImg, H, Size(Template.size()), INTER_CUBIC);
		cuda::warpPerspective(TemplateGPU, warpImgGPU, H, Size(TemplateGPU.size()), INTER_CUBIC);
		warpImgGPU.download(warpImg);

	}

	

}

//void Stitching::realTimeStitching()
//{
//
//	//vector<KeyPoint> kpRight, kpLeft,kpCenter,kpBottom;
//	std::this_thread::sleep_for(1s);
//	//Mat dscR, dscL,dscC,dscB;
//	double data[9] = { 0.8541059976680163,-0.004509096363924728,192.1465655504763,-0.05264183124322289,0.9442585031052442,4156114063640476,-0.000101740153758875,-1.4001449861549e-05,1.028835123384315 };
//	Mat H_LR=Mat(3,3,CV_64FC1,data);
//	int i = 0;
//	while (true) {
//		if (this->m_kpR.empty() || this->m_kpL.empty() || this->dscLeft.empty() || this->dscRight.empty()||this->dscCenter.empty()||this->m_kpC.empty() || this->dscBottom.empty() || this->m_kpB1.empty()) {
//			continue;
//		}
//
//		//if (this->rs == false || this->features == false) continue;
//		vector<DMatch> mt1,mt2,mt3;
//		Mat L = this->imgLeft.clone();
//		Mat R = this->imgRight.clone();
//		Mat C = this->imgCenter.clone();
//		Mat B = this->imgBottom1.clone();
//		vector<KeyPoint> kpRight = this->m_kpR;
//		vector<KeyPoint> kpLeft = this->m_kpL;
//		Mat dscL = this->dscLeft;
//		Mat dscR = this->dscRight;
//		Mat dscB = this->dscBottom;
//		Mat dscC = this->dscCenter;
//		vector<KeyPoint> kpCenter = this->m_kpC;
//		vector<KeyPoint> kpBottom = this->m_kpB1;
//		//this->rs = false;
//		//this->features = false;
//		
//		if (kpLeft.size() != dscL.rows || kpRight.size() != dscR.rows || kpCenter.size() != dscC.rows || kpBottom.size() != dscB.rows) {
//			cout << "RealTime " << "kpLeft.size(): " << kpLeft.size() << "= ?" << " dscLCPU.rows: " << dscLeft.rows << endl;
//			cout << "RealTime " << "kpCenter.size(): " << kpCenter.size() << "= ?" << " dscCCPU.rows: " << dscC.rows << endl;
//			cout << "RealTime " << "kpRight.size(): " << kpRight.size() << "= ?" << " dscR.rows: " << dscR.rows << endl;
//			cout << "RealTime " << "kpBottom.size(): " << kpBottom.size() << "= ?" << " dscB.rows: " << dscB.rows << endl;
//
//			continue;
//		}
//
//		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//
//		Mat wLeft,wRight,wBottom;
//
//		Mat centerTemplate = Mat::zeros(Size(R.size() * 2), R.type());
//		Mat roi3(centerTemplate, Rect(R.cols / 2, R.rows / 2, R.cols, R.rows));
//		C.copyTo(roi3);
//
//		int a=10, b=20, c=30;
//		Mat HL, HB,HR;
//	
//		if (R.empty() || L.empty() || B.empty()) cout << "empty Image" << endl;
//		thread t1(warpThread, dscC, dscL, kpCenter, kpLeft, ref(b), L, ref(wLeft),ref(HL));
//		thread t2(warpThread, dscC, dscR, kpCenter, kpRight, ref(a), R, ref(wRight), ref(HR));
//		thread t3(warpThread, dscC, dscB, kpCenter, kpBottom, ref(c), B, ref(wBottom),ref(HB));
//		t1.join();
//		t2.join();
//		t3.join();
//		std::chrono::steady_clock::time_point step3 = std::chrono::steady_clock::now();
//		std::cout << "Time warpThreads = " << std::chrono::duration_cast<std::chrono::microseconds>(step3 - begin).count() << "[탎]" << std::endl;
//		//Mat RTemplate = Mat::zeros(Size(R.size() * 2), R.type());
//
//		//Mat roiR(RTemplate, Rect(R.cols / 2, R.rows / 2, R.cols, R.rows));
//		//R.copyTo(roiR);
//		//cuda::GpuMat TemplateGPU, warpImgGPU, HGPU;
//		//TemplateGPU.upload(RTemplate);
//		//cuda::warpPerspective(TemplateGPU, warpImgGPU, H_LR*(HL), Size(TemplateGPU.size()), INTER_CUBIC);
//		//warpImgGPU.download(wRight);
//		if (a == 0 || b == 0 || c == 0) continue;
//
//		Mat fstPano = AlphaBlendingGPU(centerTemplate, wBottom,1.,0);
//		Mat scndPano = AlphaBlendingGPU(fstPano, wRight,1.,1);
//		Mat thdPano = AlphaBlendingGPU(scndPano, wLeft,1.,2);
//		std::chrono::steady_clock::time_point step4 = std::chrono::steady_clock::now();
//		std::cout << "Time alpha blending = " << std::chrono::duration_cast<std::chrono::microseconds>(step4 - step3).count() << "[탎]" << std::endl;
//
//
//		namedWindow("fstPano", WINDOW_FREERATIO);
//		imshow("fstPano", thdPano);
//
//
//		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//		std::cout << "Time difference gesamt = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[탎]" << std::endl;
//		//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() << "[ns]" << std::endl;
//		if (waitKey(1000/60) == 27) break;
//
//
//	}
//
//}
void Stitching::realTimeStitching() {
	server serv;
	while (true) {
		if (serv.m_img_C.empty() || serv.m_warp_R.empty()) continue;
		Mat imgC = serv.m_img_C;
		Mat warpR = serv.m_warp_R;
		Mat pano=AlphaBlendingGPU(imgC, warpR,1.,1);
		imshow("pano", pano);
		if(waitKey(10000 / 10)>=0) break;

	}
}


void Stitching::getDescriptorTCP(int port)
{
	std::this_thread::sleep_for(1s);
	server serv;
	SOCKET clientSocket = serv.createSocket(port);
	//if (port == 52000) {
	//	clientSocket2 = serv.createSocket(80000);
	//}
	//else if (port == 60000) {
	//	clientSocket2 = serv.createSocket(90000);

	//}
	//else if (port == 42000) {
	//	clientSocket2 = serv.createSocket(43000);

	//}
	//else if (port == 32000) {
	//	clientSocket2 = serv.createSocket(33000);

	//}



	//SOCKET clientSocket = this->serv.createUDPSocket(52000);

	int bytes = 0;
	int bytes1 = 0;
	int bytes2 = 0;
	//sockaddr_in client;
	//int clientLength = sizeof(client);
	//ZeroMemory(&client, clientLength);
	int buffsize = 1024 * 1024 * 2;
	//setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	int bufrow;
	int bufcol;
	char buf [1024 * 1024*2 ];
	int dscSize;
	Mat dsc;
	while (true)
	{

		int bufsize = 1024 * 1024*2 ;
		ZeroMemory(buf, bufsize);
		//for (int i = 0; i < bufsize; i +=(4*bytes)) {
			if ((bytes = recv(clientSocket, buf, bufsize , 0)) == -1) cout << ("recv failed");
		//}
		float* p = (float*)buf;
		bufcol = 64;
		bufrow = (bytes / (64 * 4));
		dsc = Mat::zeros(bufrow, bufcol, CV_32FC1);

		int ptr = 0;
		typedef long float Float64;
		for (int i = 0; i < bufrow; i++) {
			float* ptrdsc = dsc.ptr<float>(i);

			for (int j = 0; j < bufcol; j++) {
				//dsc.at<float>(i, j) = *p;
				ptrdsc[j] = *p;
				p++;
				ptr = ptr + 1;

			}
		}
		//cout << "dsc: " << dsc.size() << endl;
		string s = "dsc fertig";
		int senddsctodsc = send(clientSocket, s.c_str(), sizeof(s), 0);
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
			this->m_kpL = this->kptmpleft;
			this->imgLeft = this->frametmpleft;
			//cout << "dscLeft Size: " << dsc.size() << endl;
			//cout << " " << endl;
		}
		else if (port == 52000) {

			this->dscRight = dsc;
			this->m_kpR = this->kptmpright;
			this->imgRight = this->frametmpright;
			//cout << "dscRight Size: " << dscRight.size() << endl;
			//cout << "m_kpR Size: " << m_kpR.size() << endl;

		}
		else if (port == 42000) {

			this->dscCenter = dsc;
			this->m_kpC = this->kptmpcenter;
			this->imgCenter = this->frametmpcenter;
			cout << "dscCenter Size: " << dsc.size() << endl;
			cout << "m_kpC Size: " << m_kpC.size() << endl;

		}
		else if (port == 32000) {

			this->dscBottom = dsc;
			this->m_kpB1 = this->kptmpbottom;
			this->imgBottom1 = this->frametmpbottom;
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

	WSACleanup();
}
void Stitching::getCudaDescriptorsTCP(int port)
{
	std::this_thread::sleep_for(2s);
	server serv;

	SOCKET clientSocket = serv.createSocket(port);
	SOCKET clientSocket2 = serv.createSocket(80000);

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

	char buf[1024 * 1024 * 2];
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
		if ((bytes = recv(clientSocket, buf, sizeof(buf), 0)) == -1) cout << ("recv failed dsc");
		float* p;

		//for (p = (float*)&buf[0]; p < (float*)&buf[bytes - 1]; p++) {
		//	descriptors.push_back(*p);
		//	//*p++;
		//}

		for (p = (float*)&buf[0]; p < (float*)&buf[bufcol * bufrow * 4]; p++) {
			descriptors.push_back(*p);
			//*p++;
		}
		int cnt = 0;

		dscCpu = Mat::zeros(bufrow, bufcol, CV_32FC1);
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
	server serv;

	SOCKET clientSocket = serv.createSocket(port);
	int bytes = 0;

	char buf[1024 * 1024 * 2];
	//KeyPoint buffer[3000];
	float buffer;
	Mat imgK;
	int len = 800000;
	while (true)
	{
		vector<KeyPoint> veckp;
		int buffsize = 1024 * 1024 * 2;

		ZeroMemory(buf, buffsize);
		//for (int i = 0; i < buffsize; i += (sizeof(KeyPoint) * bytes)) {
			if ((bytes = recv(clientSocket, buf , buffsize , 0)) == -1) cout << ("recv3 failed");
		//}

		KeyPoint* p;
		for (p = (KeyPoint*)&buf[0]; p <= (KeyPoint*)&buf[bytes - 1]; p++) {
			veckp.push_back(*p);
			//*p++;
		}
		p = nullptr;
		//cout << "veckp " << veckp.size() << endl;
		//string s = "kp fertig";
		//int sendkptokp = send(clientSocket, s.c_str(), sizeof(s), 0);
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
			this->kptmpright = veckp;
		}
		else if (port == 59000) {
			this->kptmpleft = veckp;
		}
		else if (port == 41000) {
			this->kptmpcenter = veckp;
		}
		else if (port == 31000) {
			this->kptmpbottom = veckp;
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
	server serv;
	SOCKET clientSocket = serv.createSocket(port);
	KeyPoint kp;
	char buf[921600];


	while (true)
	{
		Mat img = Mat::zeros(height, width, CV_8UC3);
		int imgSize;
		imgSize = img.total() * img.elemSize();
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
			this->frametmpright = img.clone();

		}
		else if (port == 58000) {
			this->frametmpleft = img.clone();



		}
		else if (port == 40000) {
			this->frametmpcenter = img.clone();

		}
		else if (port == 30000) {
			this->frametmpbottom = img.clone();

		}


	}

	closesocket(clientSocket);

	WSACleanup();
}

Mat Stitching::getAlpha(Mat dst1, Mat dst2) {
	typedef float  Float32;
	typedef long float  Float64;
	Mat alpha = Mat::zeros(Size(dst1.size()), dst1.type());


	//for (int r = 0; r < dst1.rows; r++) {
	//	for (int c = 0; c < dst2.cols; c++) {
	//		if (r < dst1.rows && c < dst1.cols)
	//		{
	//			alpha.at<float>(r, c) = Float64(dst1.at<float>(r, c) / (dst1.at<float>(r, c) + dst2.at<float>(r, c)) + 0.00000000001);

	//		}
	//		else {
	//			alpha.at<float>(r, c) = 0;
	//		}
	//		//cout << "dst2\t" << dst2.at<float>(r, c) << endl;
	//		//cout << "alpha\t" << alpha.at<float>(r, c) << endl;

	//	}
	//}
	for (int r = 0; r < dst1.rows; r++) {
		// We obtain a pointer to the beginning of row r
		float* ptrA = dst1.ptr<float>(r);
		float* ptrB = dst2.ptr<float>(r);
		float* ptralpha = alpha.ptr<float>(r);

		for (int c = 0; c < dst1.cols; c++) {
			ptralpha[c] = Float64(ptrA[c]/(ptrA[c]+ ptrB[c]));
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

void Stitching::takeDFT(Mat& source, Mat& destination)
{
	Mat originalComplex[2] = { source, Mat::zeros(source.size(),CV_32F) };
	Mat dftReady;
	merge(originalComplex, 2, dftReady);
	Mat dftOfOriginal;
	dft(dftReady, dftOfOriginal, DFT_COMPLEX_OUTPUT);
	destination = dftOfOriginal;

}

void Stitching::showDFT(Mat& source)
{
	Mat splitArray[2] = { Mat::zeros(source.size(),CV_32F),Mat::zeros(source.size(),CV_32F) };
	split(source, splitArray);
	Mat dftMagnitude;
	magnitude(splitArray[0], splitArray[1], dftMagnitude);
	dftMagnitude += Scalar::all(1);
	log(dftMagnitude, dftMagnitude);
	normalize(dftMagnitude, dftMagnitude, 0, 1, NORM_MINMAX);

}

void Stitching::recenterDFT(Mat& source) {
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

void Stitching::inverseDFT(Mat& source, Mat& destination) {
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
			//cout << i << " area  " << a << endl;
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
	cout << "			1" << endl;
	Mat grayImg, bin,dst;
	//if (img.type() != 0) {
	//	cvtColor(img, grayImg, COLOR_BGR2GRAY);
	//}
	//else {
	//	grayImg = img;
	//}
	cvtColor(img, grayImg, COLOR_RGB2GRAY);


	//removeBlackPoints(grayImg);

	cuda::GpuMat grayImgGPU, binGPU, dstGPU;
	//grayImgGPU.upload(grayImg);
	//cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);
	threshold(grayImg, bin, 0, 255, THRESH_BINARY);

	
	distanceTransform(bin, dst, DIST_L2, 3.0);

	return dst;
	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));
	//dilate(dst, dst, kernel,Point(-1,-1));

	//return dst;
}

vector<DMatch> Stitching::getSiftmatches(Mat& img1, Mat& img2, float a)
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


Mat Stitching::getHomography(vector <DMatch> matches, vector<KeyPoint> kpObj, vector<KeyPoint> kpScn)
{
	
	vector<Point2f> objtmp, scenetmp;
	//cout << "matches size: " << matches.size() << endl;
	for (int i = 0; i < matches.size(); i++) {
		scenetmp.push_back(kpScn[matches[i].queryIdx].pt);
		objtmp.push_back(kpObj[matches[i].trainIdx].pt);
	}

	Mat H = findHomography(objtmp, scenetmp, RANSAC);
	return H;
}


void Stitching::VideoStream(String path, String windowname, double fps, Mat& frame) {
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



Mat Stitching::AlphaBlending(Mat img, Mat addImg) {

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	
	Mat dst1=makeNormalize(img);
//	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
	Mat dst2= makeNormalize(addImg);

	//std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();
	//std::cout << "Time difference2 = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end2 - begin).count() << "[ns]" << std::endl;
	Mat alpha=getAlpha(dst1,dst2);

	//std::chrono::steady_clock::time_point end3 = std::chrono::steady_clock::now();
	//std::cout << "Time difference3 = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end3 - begin).count() << "[ns]" << std::endl;
	//Mat beta = getBeta(dst1, dst2);
	Mat Pano = Mat::zeros(Size(img.size()), img.type());
	//unsigned char* inputPano = (unsigned char*)(Pano.data);
	//unsigned char* inputImg = (unsigned char*)(img.data);
	//unsigned char* inputaddImg = (unsigned char*)(addImg.data);
	//unsigned char* inputalpha = (unsigned char*)(alpha.data);
	//Erstes Alpha Blending (Mittleres und linkes Bild)
	//for (int c = 0; c < dst1.cols; c++)
	//{
	//	for (int r = 0; r < dst1.rows; r++)
	//	{

	//		//firstpano.at<uchar>(r, c) = alpha1.at<float>(r, c) * (int)upgray.at<uchar>(r, c) + (1. - alpha1.at<float>(r, c)) * (int)lowgray.at<uchar>(r, c);
	//		//Pano.at<Vec3b>(r, c)[0] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[0] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[0];
	//		//Pano.at<Vec3b>(r, c)[1] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[1] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[1];
	//		//Pano.at<Vec3b>(r, c)[2] = alpha.at<float>(r, c) * img.at<Vec3b>(r, c)[2] + (1. - alpha.at<float>(r, c)) * addImg.at<Vec3b>(r, c)[2];
	//		inputPano[dst1.step * c + img.channels() * r] = inputalpha[dst1.step * c + r] * inputImg[dst1.step * c + img.channels()*r] + (1. - inputalpha[dst1.step * c + r]) * inputaddImg[dst1.step * c + img.channels() * r];
	//		inputPano[dst1.step * c + img.channels() * r+1] = inputalpha[dst1.step * c + r+1] * inputImg[dst1.step * c + img.channels() * r+1] + (1. - inputalpha[dst1.step * c + r+1]) * inputaddImg[dst1.step * c + img.channels() * r+1];
	//		inputPano[dst1.step * c + img.channels() * r+2] = inputalpha[dst1.step * c + r+2] * inputImg[dst1.step * c + img.channels() * r+2] + (1. - inputalpha[dst1.step * c + r+2]) * inputaddImg[dst1.step * c + img.channels() * r+2];

	//	}
	//}
	Mat beta = Mat::ones(alpha.size(), alpha.type());
	Mat betmp;
	absdiff(alpha, beta, betmp);
	blendLinear(img, addImg, alpha, betmp, Pano);
	//for (int r = 0; r < dst1.rows; r++) {
	//	// We obtain a pointer to the beginning of row r
	//	cv::Vec3b* ptr = Pano.ptr<cv::Vec3b>(r);
	//	cv::Vec3b* ptr1 = img.ptr<cv::Vec3b>(r);
	//	cv::Vec3b* ptr2 = addImg.ptr<cv::Vec3b>(r);
	//	float* ptra = alpha.ptr<float>(r);

	//	for (int c = 0; c < dst1.cols; c++) {
	//		ptr[c] = cv::Vec3b(ptr1[c][0]*ptra[c]+(1.-ptra[c])*ptr2[c][0],
	//			ptr1[c][1]*ptra[c] + (1. - ptra[c]) * ptr2[c][1], 
	//			ptr1[c][2]*ptra[c] + (1. - ptra[c]) * ptr2[c][2]);
	//	}
	//}
	//std::chrono::steady_clock::time_point end4 = std::chrono::steady_clock::now();
	//std::cout << "Time difference4 = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end4 - end3).count() << "[ns]" << std::endl;
	return Pano;
}
void Stitching::StitchAllImgs(cuda::GpuMat& warpBottom1GPU, cuda::GpuMat& warpBottom2GPU, cuda::GpuMat& warpRightGPU, cuda::GpuMat& warpLeftGPU)
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
	cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, 30);
	pipe.start(cfg);
	rs2::context ctx;        // Create librealsense context for managing devices
	std::vector<std::string> serials;
	rs2::rates_printer printer;
	rs2::colorizer color_map;

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	for (auto&& dev : ctx.query_devices()) {
		serials.push_back(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
		cout << dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) << endl;
	}
	rs2::frameset frames;

	for (int i = 0; i < 30; i++)
	{
		cout << "empty image" << endl;
		frames = pipe.wait_for_frames();
	}
	while (true) {
		//if (this->rs == true) continue;
		frames = pipe.wait_for_frames();
		rs2::frame color_frame = frames.get_color_frame();
		Mat color(sz, CV_8UC3, (void*)color_frame.get_data(), Mat::AUTO_STEP);
		//Mat imgSURF = color;
		//if (imgSURF.empty()) continue;
		//Ptr<SURF> detector = SURF::create(100);
		//std::vector<KeyPoint> kp;
		//Mat dsc, out;
		//detector->detectAndCompute(imgSURF, noArray(), kp, dsc);
		//this->kptmp = kp;
		//this->dsctmp = dsc;
		//this->img = color;
		//dsc.release();
		//kp.clear();
		Mat img = color.clone();
		imshow("Fr", img);
		if (waitKey(10) == 27) break;
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
		Mat dsc, out;
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

void Stitching::cropImg(Mat& img, Rect rct, int d)
{

	for (int r = 0; r < img.rows; r++) {
		for (int c = 0; c < img.cols; c++) {
			int dX = c - rct.x;
			int dY = r - rct.y;
			int dX2 = rct.width - c;
			int dY2 = rct.height - r;

			if (((dX > d && dY > d) && (dX2 < d && dY2 < d)) && rct.contains(Point(c, r))) {
				img.at<Vec3b>(r, c) = Vec3b(0, 0, 0);
			}
		}
	}
}

void Stitching::removeBlackPoints(Mat& img)
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

cuda::GpuMat Stitching::getAlphaGPU(cuda::GpuMat gpu_dst1, cuda::GpuMat gpu_dst2) {
	cuda::GpuMat gpu_alpha(gpu_dst1.size(), CV_32F);
	cuda::GpuMat added, divalpha, resAlpha;
	//gpu_dst1.upload(dst1);
	//gpu_dst2.upload(dst2);
	//gpu_alpha.upload(alpha);
	cuda::add(gpu_dst1, gpu_dst2, added);
	cuda::divide(gpu_dst1, added, resAlpha,2.);
	return resAlpha;


}

void Stitching::makeNormalizethread(Mat img, cuda::GpuMat &dst_gpu) {
	Mat grayImg, bin,dst;

	//if (img.type() != 0) {
	//	cvtColor(img, grayImg, COLOR_BGR2GRAY);
	//}
	//else {
	//	grayImg = img;
	//}
	
	cuda::GpuMat grayImgGPU, binGPU, dstGPU,img_GPU;

	img_GPU.upload(img);

	cuda::cvtColor(img_GPU, grayImgGPU, COLOR_BGR2GRAY);
	//grayImgGPU.upload(grayImg);

	cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);

	binGPU.download(bin);

	distanceTransform(bin, dst, DIST_L2, 3.0);

	dst_gpu.upload(dst);
}

Mat Stitching::AlphaBlendingGPU(Mat &img, Mat &addImg, float scalar, int a) {
	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	cuda::GpuMat gpu_alpha[3], gpu_beta[3], gpu_alphares, gpu_betares, alphtmp, betatmp;
	cuda::GpuMat alpha, beta;
	//if (this->counter % 50 < 3) {
	cuda::GpuMat dst1, dst2;

	thread t1(makeNormalizethread, img, ref(dst1));
	thread t2(makeNormalizethread, addImg, ref(dst2));
	t1.join();
	t2.join();

	//cout << "jetzt normalize upload" << endl;

	//cout << "normalized done" << endl;
	alpha = getAlphaGPU(dst1, dst2);
	beta = cuda::GpuMat(alpha.size(), alpha.type(), Scalar::all(1));
	//cout << "alpha and beta done" << endl;

	// switch (a) {
	//		case 0 :
	//			this->m_alpha1 = alpha;
	//			this->m_beta1 = beta;
	//		case 1:
	//			this->m_alpha2 = alpha;
	//			this->m_beta2 = beta;
	//		case 2:
	//			this->m_alpha3 = alpha;
	//			this->m_beta3 = beta;
	//	 }


	//	if (this->counter == 50) this->counter = 0;
	//}
	//else {
	//	switch (a) {
	//	case 0:
	//		alpha= this->m_alpha1;
	//		beta= this->m_beta1;
	//	case 1:
	//		alpha = this->m_alpha2;
	//		beta = this->m_beta2;
	//	case 2:
	//		alpha = this->m_alpha3;
	//		beta = this->m_beta3;
	//	}
	//}
	//this->counter++;
	vector<cuda::GpuMat> vecgpu1, vecgpu2;
	//alphtmp.upload(alpha);
	cuda::absdiff(alpha, beta, betatmp);


	Mat Pano;
	cuda::GpuMat pano2, panothresh, panodiff;
	//pano.convertTo(pano2, CV_8U, 255.);
	//pano.convertTo(panothresh, CV_8U);
	//cuda::absdiff(pano2, panothresh, panodiff);

	cuda::GpuMat gpu_img1, gpu_img2;
	gpu_img1.upload(img);

	gpu_img2.upload(addImg);

	cuda::blendLinear(gpu_img1, gpu_img2, alpha, betatmp, panodiff);

	panodiff.download(Pano);
	gpu_alpha->release();
	gpu_beta->release();
	/*std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference4 = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;*/

	return Pano;
}