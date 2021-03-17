#include "stitching.h"
#include "C:/Users/fmosh/CUDA_RANSAC_Homography/CUDA_RANSAC_Homography.h"

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
	SOCKET clientsocket1 = this->serv.createSocket(port1);
	SOCKET clientsocket2 = this->serv.createSocket(port2);
	SOCKET clientsocket3 = this->serv.createSocket(port3);


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
	KeyPoint* p;

	int buffsize = 1024 * 1024 * 2;
	setsockopt(clientsocket3, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	int bufrow;
	int bufcol;
	char bufdsc[1024 * 1024 * 2];
	int dscSize;
	int kpsize;
	Mat dsc;

	
	while (true)
	{
		//Mat img = Mat::zeros(height, width, CV_8UC3);
		//int imgSize;
		//imgSize = img.total() * img.elemSize();
		//ZeroMemory(buf, imgSize);
		////Wait for client to send data
		//for (int i = 0; i < imgSize; i += bytes)
		//	if ((bytes = recv(clientsocket1, buf + i, imgSize - i, 0)) == -1) cout << ("recv failed");
		//int ptr = 0;
		//for (int i = 0; i < img.rows; i++) {
		//	for (int j = 0; j < img.cols; j++) {
		//		img.at<Vec3b>(i, j) = Vec3b(buf[ptr + 0], buf[ptr + 1], buf[ptr + 2]);
		//		ptr = ptr + 3;
		//	}
		//}
		ZeroMemory((char*)&dscSize, sizeof(dscSize));
		ZeroMemory((char*)&kpsize, sizeof(kpsize));

		recv(clientsocket1, (char*)&dscSize, sizeof(dscSize), 0);
		recv(clientsocket1, (char*)&kpsize, sizeof(kpsize), 0);

		/////////////////////////////////////////////////////////////////////////////////////////
		vector<KeyPoint> veckp;
		int KpSize = 1024 * 1024 * 2;
		KpSize = kpsize;
		ZeroMemory(bufkp, KpSize);
		for (int i = 0; i < KpSize; i += (bytes1))
		if ((bytes1 = recv(clientsocket2, bufkp+i, KpSize-i, 0)) == -1) cout << ("recv3 failed");
		KeyPoint* p;

		for (p = (KeyPoint*)&bufkp[0]; p <= (KeyPoint*)&bufkp[bytes1 - 1]; p++) {
			veckp.push_back(*p);
		}
		p = nullptr;
		/////////////////////////////////////////////////////////////////////////////////////////
		//int dscSize = 1024 * 1024 * 2;

		ZeroMemory(bufdsc, dscSize);
		//Wait for client to send data


		ZeroMemory((char*)&bufcol, dscSize);
		for (int i = 0; i < dscSize; i += (bytes3))
		if ((bytes3 = recv(clientsocket3, bufdsc+i, dscSize-i, 0)) == -1) cout << ("recv failed");
		float* pdsc = (float*)bufdsc;
		bufrow = bytes3 / (64 * 4);
		bufcol = 64;
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
		if ( bytes1 == 0 || bytes3 == 0)
		{
			cout << "Client disconnected" << endl;
			cout << "bytes: " << bytes << endl;
			cout << "bytes1: " << bytes1 << endl;
			cout << "bytes3: " << bytes3 << endl;
			cout << "bytes4: " << bytes4 << endl;

			break;
		}
		string s = "angekommen";
		int sendresult = send(clientsocket3, s.c_str(), sizeof(s), 0);
		cout << "dsc size: " << dsc.size() << endl;
		cout << "veckp size: " << veckp.size() << endl;
		if (port1 == 54000) {
			this->imgRight = img.clone();
			this->m_kpR = veckp;
			this->dscRight = dsc;

		}
		else if (port1 == 58000) {
			this->imgLeft = img.clone();
			this->m_kpL = veckp;
			this->dscLeft = dsc;
		}


	}
	//closesocket(clientsocket1);
	closesocket(clientsocket2);
	closesocket(clientsocket3);

	WSACleanup();

}



void Stitching::CamFeatures() {
	VideoCapture cap;
	Mat imgGray1, dsccpuc;
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

		detector->detectAndCompute(tmp, noArray(), kp, dsc);

		this->m_kpCam = kp;
		this->m_dscCam = dsc;
		this->camFrame = tmp;

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
void Stitching::tmp() {
	server serv;
	SOCKET clientsocket=serv.createSocket(1000);
	SOCKET clientsocket2 = serv.createSocket(1001);
	SOCKET clientsocket3 = serv.createSocket(1002);
	char buf[100];
	int bytesH = 0;
	while(true) {
		if (this->m_dscCam.empty() || this->dscRight.empty()) continue;
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		cuda::GpuMat dsc1GPU, dsc2GPU;
		dsc1GPU.upload(this->m_dscCam);
		dsc2GPU.upload(this->dscRight);
		Ptr<cuda::DescriptorMatcher> matcherGPU;
		matcherGPU = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
		Ptr<cv::DescriptorMatcher> matcher;
		vector<vector<DMatch>> matches;
		matcherGPU->knnMatch(dsc1GPU, dsc2GPU, matches, 2);
		vector<DMatch> mt1;

		if (!matches.empty()) {
		for (int i = 0; i < matches.size(); i++) {
			if (matches[i][0].distance < 1. * matches[i][1].distance) {
				mt1.push_back(matches[i][0]);
			}
		}

		}
		else {
			DMatch nullMatch;
			nullMatch.distance = 1;
			nullMatch.imgIdx = 1;
			nullMatch.queryIdx = 1;
			nullMatch.trainIdx = 1;
			mt1.push_back(nullMatch);
		}
		//cout <<"Match size "<< sizeof(DMatch) * matches.size() << endl;
		int sendrspo=send(clientsocket, (char*)&mt1[0], sizeof(DMatch) * mt1.size(), 0);
		vector<KeyPoint> kptmp=this->m_kpCam;
		int sendkp= send(clientsocket2, (char*)&kptmp[0], sizeof(KeyPoint) * kptmp.size(), 0);
		if (sendrspo == -1) cout << "Error sending matches" << endl;
		cout <<"Matches size:"<< mt1.size() << endl;
		cout <<"KeyPoint size:"<< kptmp.size() << endl;
		Mat H = Mat::zeros(3, 3, CV_64FC1);
		const int sizeHomo = H.total() * H.elemSize();
		ZeroMemory(buf, sizeof(buf));

		//for (int i = 0; i < sizeof(buf); i += bytesH)
		if ((bytesH=recv(clientsocket3, buf, sizeof(buf), 0)) == -1) cout<<"recv failed"<<endl;
		cout <<"Homo Matrix"<< bytesH << endl;
		//if (bytesH == 0) continue;
		double* Hp = (double*)buf;

		for (int i = 0; i < H.rows; i++) {
			for (int j = 0; j < H.cols; j++) {
				H.at<double>(i, j) = *Hp;
				Hp++;
			}
		}
		cout << "homo: " << H << endl;
		Mat img = this->imgRight.clone();
		Mat frameCam = this->camFrame.clone();

		if (img.empty()) continue;
		Mat warpR;
		warpPerspective(img, warpR, H, img.size() * 2, INTER_CUBIC);
		Mat pano=AlphaBlending(frameCam, img);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "elapsed Time:  = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;

		imshow("pano", pano);
		if (waitKey(1) >= 0) break;
		
	}
	
	
	//else {
	//	cout << "matches are empty" << endl;
	//}
	//Mat H;
	//if (mt1.size() < 10) {
	//	cout << "Matches size in thread " << tmp << " is too less" << endl;
	//}
	//vector<Point2f> objtmp, scenetmp;
	//vector<KeyPoint> kpScn = this->m_kpCam;
	//vector<KeyPoint> kpObj = this->m_kpR;
	//

	//for (int i = 0; i < mt1.size(); i++) {
	//	//if ((matches[i].queryIdx < kpObj.size()) && (matches[i].trainIdx < kpScn.size())) 
	//	scenetmp.push_back(kpScn[mt1[i].queryIdx].pt);
	//	objtmp.push_back(kpObj[mt1[i].trainIdx].pt);

	//}


}

void Stitching::getHomoGraphyTCP(int port){
	SOCKET clientSocket = serv.createSocket(port);
	int port2 = port + 1500;
	SOCKET clientSocket2 = serv.createSocket(port2);
	int bytes = 0;
	char buf[1000];
	while (true) {
		if (this->m_dscCam.empty() || this->dscRight.empty()) continue;
		Mat dsc1 = this->m_dscCam;
		Mat dsc2 = this->dscRight;
		cuda::GpuMat dsc1GPU, dsc2GPU;
		vector<KeyPoint> kp1 = this->m_kpCam;
		dsc1GPU.upload(dsc1);
		dsc2GPU.upload(dsc2);
		Ptr<cuda::DescriptorMatcher> matcherGPU;
		matcherGPU = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
		Ptr<cv::DescriptorMatcher> matcher;
		vector<vector<DMatch>> matches;
		matcher = BFMatcher::create(NORM_L2, false);
		matcherGPU->knnMatch(dsc1GPU, dsc2GPU, matches, 2);
		if (matches.empty()) continue;
		vector<DMatch> mt1;
			for (int i = 0; i < matches.size(); i++) {
				if (matches[i][0].distance < .7 * matches[i][1].distance) {
					mt1.push_back(matches[i][0]);
				}
			 		}
			int sendDsc = send(clientSocket, (char*)&mt1[0], sizeof(DMatch) * mt1.size(), 0);
		
			if (sendDsc == -1) cout << "Matches konnten nicht gesendet werden" << endl;

			int sendKeyPoints = send(clientSocket2, (char*)&kp1[0], sizeof(KeyPoint) * kp1.size(), 0);
			if (sendKeyPoints == -1)  cout << "KeyPoints konnten nicht gesendet werden" << endl;
			cout << "KeyPOint size"<<kp1.size() << endl;



			/////////////////////////////////////////////////////////////////
			Mat H = Mat::zeros(3, 3, CV_64FC1);
			const int sizeHomo = H.total() * H.elemSize();
			ZeroMemory(buf, sizeof(buf));

			//for (int i = 0; i < sizeof(buf); i += bytesH)
			if ((bytes = recv(clientSocket2, buf, sizeof(buf), 0)) == -1) cout << "recv failed" << endl;
			cout << "Homo Matrix" << bytes << endl;
			//if (bytesH == 0) continue;
			double* Hp = (double*)buf;

			for (int i = 0; i < H.rows; i++) {
				for (int j = 0; j < H.cols; j++) {
					H.at<double>(i, j) = *Hp;
					Hp++;
				}
			}
			cout << "homo: " << H << endl;
	}
	
	closesocket(clientSocket);
}


void Stitching::warpThread(Mat dsc1, Mat dsc2, vector<KeyPoint> kp1, vector<KeyPoint> kp2, int& a, Mat img, Mat& warpImg, Mat &H) {
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
			if (matches[i][0].distance < .7 * matches[i][1].distance) {
				mt1.push_back(matches[i][0]);
			}
		}
	}
	else {
		cout << "matches are empty" << endl;
	}


	//cout << "mt1 Size: " << mt1.size() << "mt2 Size: " << mt2.size() << "mt1 Size: " << mt2.size() << endl;

	if (mt1.size() < 10) {
		cout << "Matches size in thread " << tmp << " is too less" << endl;
		a = 0;
	}
	else {
		H = s.getHomography(mt1, kp2, kp1);
	}

	mt1.clear();

	if (H.empty()) {
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
		//cout << "Homo: " << H << endl;
		warpImgGPU.download(warpImg);

	}


}

void Stitching::realTimeStitching()
{

	vector<KeyPoint> kpRight, kpLeft, kpCenter, kpBottom;
	std::this_thread::sleep_for(3s);
	Mat dscR, dscL, dscC, dscB;
	bool homographybool=false;
	double data[9] = { 0.8541059976680163, -0.004509096363924728, 192.1465655504763,
-0.05264183124322289, 0.9442585031052442, 41.56114063640476,
-0.0001017401253758875, -1.4001449861549e-05, 1.028835123384315 };
	Mat Hstat(3, 3, CV_64F, data);
	while (true) {
		if (this->m_kpR.empty() || this->m_kpL.empty() || this->dscLeft.empty() || this->dscRight.empty() || this->dscCenter.empty() || this->m_kpC.empty() || this->dscBottom.empty() || this->m_kpB1.empty()) {
			//cout << this->m_kpR.size() << this->m_kpL.size() << this->dscLeft.size() << this->dscRight.size() << this->dscCenter.size() << this->m_kpC.size() << this->dscBottom.size() << this->m_kpB1.size() << endl;
			continue;
		}
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		//if (this->rs == false || this->features == false) continue;
		kpCenter.clear();
		kpLeft.clear();
		dscR.release();
		dscL.release();
		dscC.release();
		kpRight.clear();
		kpBottom.clear();
		dscB.release();
		vector<DMatch> mt1, mt2, mt3;
		Mat L = this->imgLeft.clone();
		Mat R = this->imgRight.clone();
		Mat C = this->imgCenter.clone();
		Mat B = this->imgBottom1.clone();
		kpRight = this->m_kpR;
		kpLeft = this->m_kpL;
		dscL = this->dscLeft;
		dscR = this->dscRight;
		dscB = this->dscBottom;
		dscC = this->dscCenter;
		kpCenter = this->m_kpC;
		kpBottom = this->m_kpB1;
		Mat HLC,HRL,HBC;
		//this->rs = false;
		//this->features = false;
		//cout << "RealTime " << "kpCenter.size(): " << kpCenter.size() << "= ?" << " dscCCPU.rows: " << dscCenter.rows << endl;
		//cout << "RealTime " << "kpRight.size(): " << kpRight.size() << "= ?" << " dscRight.rows: " << dscRight.rows << endl;
		//cout << "RealTime " << "kpLeft.size(): " << kpLeft.size() << "= ?" << " dscLeft.rows: " << dscLeft.rows << endl;

		if (kpLeft.size() != dscL.rows || kpRight.size() != dscR.rows || kpCenter.size() != dscC.rows  || kpBottom.size() != dscB.rows) {
			cout << "RealTime " << "kpLeft.size(): " << kpLeft.size() << "= ?" << " dscLCPU.rows: " << dscLeft.rows << endl;
			cout << "RealTime " << "kpCenter.size(): " << kpCenter.size() << "= ?" << " dscCCPU.rows: " << dscCenter.rows << endl;
			cout << "RealTime " << "kpRight.size(): " << kpRight.size() << "= ?" << " dscRight.rows: " << dscRight.rows << endl;

			continue;
		}


	
		Mat wLeft, wRight, wBottom;

		Mat centerTemplate = Mat::zeros(Size(R.size() * 2), R.type());
		Mat RightTemplate = Mat::zeros(Size(R.size() * 2), R.type());

		Mat roiC(centerTemplate, Rect(R.cols / 2, R.rows / 2, R.cols, R.rows));
		Mat roiR(RightTemplate, Rect(R.cols / 2, R.rows / 2, R.cols, R.rows));


		C.copyTo(roiC);
		R.copyTo(roiR);

			int a = 10, b = 20, c = 30;
		if (R.empty() || L.empty() || B.empty()) cout << "empty Image" << endl;
		thread t1(warpThread, dscC, dscL, kpCenter, kpLeft, ref(b), L, ref(wLeft),ref(HLC));
		//thread t2(warpThread, dscC, dscR, kpCenter, kpRight, ref(a), R, ref(wRight));
		thread t3(warpThread, dscC, dscB, kpCenter, kpBottom, ref(c), B, ref(wBottom), ref(HBC));
		t1.join();
		//t2.join();
		t3.join();
		std::chrono::steady_clock::time_point threadtime = std::chrono::steady_clock::now();
		std::cout << "Time difference thread = " << std::chrono::duration_cast<std::chrono::microseconds>(threadtime - begin).count() << "[탎]" << std::endl;

		if (a == 0 || b == 0 || c == 0) continue;
						cuda::GpuMat gpu_Right,gpu_wRight;
						gpu_Right.upload(RightTemplate);
						Mat Hnew = Hstat * (HLC.inv());
						cuda::warpPerspective(gpu_Right, gpu_wRight, Hnew, RightTemplate.size(), INTER_CUBIC);
						gpu_wRight.download(wRight);

		std::chrono::steady_clock::time_point warpright = std::chrono::steady_clock::now();
		std::cout << "Time difference warpRight = " << std::chrono::duration_cast<std::chrono::microseconds>(warpright - begin).count() << "[탎]" << std::endl;

		Mat fstPano = AlphaBlendingGPU(centerTemplate, wBottom,1.5,0);
		Mat scndPano = AlphaBlendingGPU(fstPano, wRight,1.,1);
		Mat thdPano = AlphaBlendingGPU(scndPano, wLeft,1.,2);
		std::chrono::steady_clock::time_point AlphaBlendingGPUtime = std::chrono::steady_clock::now();
		std::cout << "Time difference AlphaBlendingGPUtime = " << std::chrono::duration_cast<std::chrono::microseconds>(AlphaBlendingGPUtime - begin).count() << "[탎]" << std::endl;

		namedWindow("fstPano", WINDOW_FREERATIO);
		imshow("fstPano", thdPano);
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[탎]" << std::endl;
		//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() << "[ns]" << std::endl;
		if (waitKey(1000 / 60) == 27) break;


	}

}



void Stitching::getDescriptorTCP(int port)
{
	std::this_thread::sleep_for(1s);
	//SOCKET clientSocket2;

	SOCKET clientSocket = serv.createSocket(port);
	//if (port == 52000) {
	//	//clientSocket2 = serv.createSocket(80000);
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
	sockaddr_in client;
	int clientLength = sizeof(client);
	ZeroMemory(&client, clientLength);
	int buffsize = 1024 * 1024 * 2;
	setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&buffsize, sizeof(buffsize));
	int bufrow;
	int bufcol;
	char buf[1024 * 1024 ];
	int dscSize;
	Mat dsc;
	while (true)
	{

		vector<float> descriptors;
		ZeroMemory(buf, sizeof(buf));
		//Wait for client to send data
		//ZeroMemory((char*)&bufrow, sizeof(bufrow));

		//if ((bytes1 = recv(clientSocket2, (char*)&bufrow, sizeof(bufrow), 0)) == -1) cout << ("recv failed dsc");

		//ZeroMemory((char*)&bufcol, sizeof(bufcol));

		//if ((bytes2 = recv(clientSocket2, (char*)&bufcol, sizeof(bufcol), 0)) == -1) cout << ("recv failed dsc");

		for (int i = 0; i < sizeof(buf); i += (4*bytes) ) {
			if ((bytes = recv(clientSocket, buf+i , sizeof(buf)-i, 0)) == -1) cout << ("recv failed");

		}

		bufcol = 64;
		bufrow = (bytes / (64 * 4));
		float* p = (float*)buf;
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
			//this->m_kpL = this->kptmpleft;
			//this->imgLeft = this->frametmpleft;
			//cout << "dscLeft Size: " << dsc.size() << endl;
			//cout << " " << endl;
		}
		else if (port == 52000) {

			this->dscRight = dsc;
			//this->m_kpR = this->kptmpright;
			//this->imgRight = this->frametmpright;

			//cout << "dscCenter Size: " << dsc.size() << endl;
			//imshow("dsc", dsc);
			//if (waitKey(1) >= 0) break;
		}
		else if (port == 42000) {

			this->dscCenter = dsc;
			//this->m_kpC = this->kptmpcenter;
			//this->imgCenter = this->frametmpcenter;
			//cout << "dscCenter Size: " << dsc.size() << endl;
		}
		else if (port == 32000) {

			this->dscBottom = dsc;
			//this->m_kpB1 = this->kptmpbottom;
			//this->imgBottom1 = this->frametmpbottom;
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
	SOCKET clientSocket = this->serv.createSocket(port);
	int bytes = 0;
	vector<KeyPoint> veckp;
	KeyPoint kp;

	char buf[1024 * 1024 * 2];
	//KeyPoint buffer[3000];
	float buffer;
	KeyPoint* p;
	Mat imgK;
	int len = 800000;
	while (true)
	{
		vector<KeyPoint> veckp;


		ZeroMemory(buf, sizeof(buf));
		for (int i = 0; i < sizeof(buf); i += (sizeof(KeyPoint) * bytes)) {
			if ((bytes = recv(clientSocket, buf + i, sizeof(buf) - i, 0)) == -1) cout << ("recv3 failed");
			cout << bytes << endl;
		}
		
		KeyPoint* p;
		for (p = (KeyPoint*)&buf[0]; p <= (KeyPoint*)&buf[bytes - 1]; p++) {
			veckp.push_back(*p);
			//*p++;
		}
		p = nullptr;
		string s = "kp fertig";
		int sendkptokp = send(clientSocket, s.c_str(), sizeof(s), 0);
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
			//this->kptmpright = veckp;

			this->m_kpR = veckp;

		}
		else if (port == 59000) {
			//this->kptmpleft = veckp;

			this->m_kpL = veckp;

		}
		else if (port == 41000) {
			//this->kptmpcenter = veckp;

			this->m_kpC = veckp;

		}
		else if (port == 31000) {
			//this->kptmpbottom = veckp;

			this->m_kpB1 = veckp;
			cout << "in: " << veckp.size() << endl;
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
			//this->frametmpright = img.clone();
			this->imgRight = img.clone();


		}
		else if (port == 58000) {
			//this->frametmpleft = img.clone();
			this->imgLeft = img.clone();

		}
		else if (port == 40000) {
			//this->frametmpcenter = img.clone();
			this->imgCenter = img.clone();

		}
		else if (port == 30000) {
			//this->frametmpbottom = img.clone();
			this->imgBottom1 = img.clone();

		}


	}
	closesocket(clientSocket);

	WSACleanup();
}


cuda::GpuMat Stitching::getAlphaGPU(Mat dst1, Mat dst2) {
	cuda::GpuMat gpu_alpha(dst1.size(), CV_32F);
	cuda::GpuMat gpu_dst1, gpu_dst2, added, divalpha,resAlpha;
	gpu_dst1.upload(dst1);
	gpu_dst2.upload(dst2);
	//gpu_alpha.upload(alpha);
	cuda::add(gpu_dst1, gpu_dst2, added);
	cuda::divide(gpu_dst1, added, resAlpha);
	return resAlpha;

	
}

Mat Stitching::getAlpha(Mat dst1, Mat dst2) {
	typedef float  Float32;
	typedef long float  Float64;
	//dst1.convertTo(dst1, CV_64FC1);
	//dst2.convertTo(dst2, CV_64FC1);
	
	Mat alpha = Mat::zeros(Size(dst1.size()), CV_32FC1);


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
	cuda::GpuMat gpu_dst1, gpu_dst2, gpu_alpha,added,divalpha;
	gpu_dst1.upload(dst1);
	gpu_dst2.upload(dst2);
	//gpu_alpha.upload(alpha);
	gpu_alpha = cuda::GpuMat(dst1.size(), CV_32F, Scalar::all(0));
	cuda::add(gpu_dst1, gpu_dst2, added);
	cuda::divide(gpu_dst1, added, gpu_alpha);
	gpu_alpha.download(alpha);
	//for (int r = 0; r < dst1.rows; r++) {
	//	// We obtain a pointer to the beginning of row r
	//	float* ptrA = dst1.ptr<float>(r);
	//	float* ptrB = dst2.ptr<float>(r);
	//	float* ptralpha = alpha.ptr<float>(r);

	//	for (int c = 0; c < dst1.cols; c++) {
	//		ptralpha[c] = Float64(ptrA[c] / (ptrA[c] + ptrB[c]));
	//	}
	//}

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
	//cvtColor(img, img, COLOR_BGR2GRAY);
	//threshold(img, img, 0, 255, THRESH_BINARY);
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

void Stitching::makeNormalizeThread(Mat img, Mat& dst) {
	cuda::GpuMat grayImgGPU, binGPU, dstGPU, gpu_img;
	gpu_img.upload(img);
	Mat grayImg, bin;
	cuda::cvtColor(gpu_img, grayImgGPU, COLOR_BGR2GRAY);
	cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);
	binGPU.download(bin);
	distanceTransform(bin, dst, DIST_L2, 3.0);

}
Mat Stitching::makeNormalize(Mat img) {

	cuda::GpuMat grayImgGPU, binGPU, dstGPU,gpu_img;
	gpu_img.upload(img);
	Mat grayImg, bin, dst;
	cuda::cvtColor(gpu_img, grayImgGPU, COLOR_BGR2GRAY);
	
	cuda::threshold(grayImgGPU, binGPU, 0, 255, THRESH_BINARY);

	//threshold(grayImg, bin, 0, 255, THRESH_BINARY);
	binGPU.download(bin);
	//imshow("bin", bin);
	//Mat kernel = Mat::ones(Size(5, 5), dst.type());
	//dilate(bin, bin, kernel, Point(-1, -1), 2);

	distanceTransform(bin, dst, DIST_L2, 3.0);
	//normalize(dst, dst, 0.0, (1. - 0.0), NORM_MINMAX, -1);
	//erode(dst, dst, kernel,Point(2, 2), 3);
	//dstGPU.upload(dst);
	/*double alpha = 0.;
	cuda::normalize(dstGPU, dstGPU, alpha, (1.-alpha), NORM_MINMAX,-1);*/


	//dstGPU.download(dst);


	//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(-1, -1));
	//dilate(dst, dst, kernel,Point(-1,-1));

	return dst;
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
	vector<float> match_score;
	for (int i = 0; i < matches.size(); i++) {
	//if ((matches[i].queryIdx < kpObj.size()) && (matches[i].trainIdx < kpScn.size())) 
		objtmp.push_back(kpObj[matches[i].queryIdx].pt);
		scenetmp.push_back(kpScn[matches[i].trainIdx].pt);
		//match_score.push_back(matches[i].distance);
	
	}
	cout << "objtmp " << objtmp[3].x<< " kp x" << endl;
	cout << "scenetmp " << scenetmp[3].x << " kp x" << endl;
	Mat mask;
	Mat H = findHomography(objtmp, scenetmp, RANSAC,1.5,mask);
	//vector<Point2f> objtmp2, scenetmp2;
	//for (int i = 0; i < mask.rows; i++) {
	//	if (mask.at<uchar>(i, 0) == 1) {
	//		scenetmp2.push_back(kpScn[matches[i].queryIdx].pt);
	//		objtmp2.push_back(kpObj[matches[i].trainIdx].pt);

	//	}

	//}
	//

	//Mat H2 = findHomography(objtmp2, scenetmp2, RANSAC, 1.5);


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



void Stitching::CudaStitch(cuda::GpuMat& img1, cuda::GpuMat& img2, cuda::GpuMat& img3, cuda::GpuMat& img4) {
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
	Mat beta = Mat::ones(alpha.size(), alpha.type());
	absdiff(alpha, beta, beta);
	Mat Pano = Mat::zeros(Size(img.size()), img.type());
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	
	//for (int r = 0; r < dst1.rows; r++) {
	//	cv::Vec3b* ptr = Pano.ptr<cv::Vec3b>(r);
	//	cv::Vec3b* ptr1 = img.ptr<cv::Vec3b>(r);
	//	cv::Vec3b* ptr2 = addImg.ptr<cv::Vec3b>(r);
	//	float* ptra = alpha.ptr<float>(r);
	//	//if (ptr1[r] == Vec3b(0, 0, 0) && ptr2[r] == Vec3b(0, 0, 0)) continue;
	//	for (int c = 0; c < dst1.cols; c++) {
	//		if (ptr1[c] == Vec3b(0, 0, 0) && ptr2[c] == Vec3b(0, 0, 0)) continue;
	//		//if (ptra[c] == 0.) ptr[c] = ptr2[c]; continue;
	//		ptr[c] = cv::Vec3b(ptr1[c][0] * ptra[c] + (1. - ptra[c]) * ptr2[c][0],
	//			ptr1[c][1] * ptra[c] + (1. - ptra[c]) * ptr2[c][1],
	//			ptr1[c][2] * ptra[c] + (1. - ptra[c]) * ptr2[c][2]);
	//	}
	//}
	

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference4 = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
	return Pano;
}

Mat Stitching::AlphaBlendingGPU(Mat img, Mat addImg, float scalar, int a) {
	//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	cuda::GpuMat gpu_alpha[3], gpu_beta[3], gpu_alphares, gpu_betares, alphtmp, betatmp;
	cuda::GpuMat alpha,beta;
	//if (this->counter % 50 < 3) {
		Mat dst1 = makeNormalize(img);
		Mat dst2 = makeNormalize(addImg);

		 alpha = getAlphaGPU(dst1, dst2);
		 beta = cuda::GpuMat(alpha.size(), alpha.type(),Scalar::all(1));
	//	 switch (a) {
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
	//betatmp.upload(beta);
	gpu_alpha[0]= alpha.clone();
	gpu_alpha[1] = alpha.clone();
	gpu_alpha[2] = alpha.clone();
	gpu_beta[0] = beta.clone();
	gpu_beta[1] = beta.clone();
	gpu_beta[2] = beta.clone();
	vecgpu1.push_back(gpu_alpha[0]);
	vecgpu1.push_back(gpu_alpha[1]);
	vecgpu1.push_back(gpu_alpha[2]);
	vecgpu2.push_back(gpu_beta[0]);
	vecgpu2.push_back(gpu_beta[1]);
	vecgpu2.push_back(gpu_beta[2]);

	cuda::merge(vecgpu1, gpu_alphares);
	cuda::merge(vecgpu2, gpu_betares);

	cuda::GpuMat gpu_img1, gpu_img2,gpu1[4],gpu2[4],res1,res2,pano;
	vector<cuda::GpuMat> vec1, vec2;

	gpu_img1.upload(img);
	gpu_img2.upload(addImg);
	

	cuda::GpuMat result1, result2;
	cuda::GpuMat newgpuimg1, newgpuimg2,newalph,newbeta;
	gpu_img1.convertTo(newgpuimg1, CV_64F,(1./255.));
	gpu_alphares.convertTo(newalph, CV_64F);
	cuda::multiply(gpu_alphares, cv::Scalar::all(scalar),gpu_alphares);
	cuda::multiply(newgpuimg1, newalph,result1);

	cuda::absdiff(gpu_betares, gpu_alphares, gpu_betares);

	gpu_betares.convertTo(newbeta, CV_64F);

	gpu_img2.convertTo(newgpuimg2, CV_64F, (1. / 255.));

	cuda::multiply(newgpuimg2, newbeta, result2);

	cuda::add(result1, result2, pano);


	Mat Pano=Mat::zeros(img.size(),img.type());
	cuda::GpuMat pano2,panothresh,panodiff;
	pano.convertTo(pano2, CV_8U,255.);
	pano.convertTo(panothresh, CV_8U);
	cuda::absdiff(pano2, panothresh, panodiff);
	//cuda::blendLinear(gpu_img1, gpu_img2, alphtmp, betatmp, panos);
	panodiff.download(Pano);
	/*std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference4 = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;*/

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
	cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, 6);
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

	//for (int i = 0; i < 30; i++)
	//{
	//	frames = pipe.wait_for_frames();
	//}
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

vector<DMatch> Stitching::getCudaSurfMatches(Mat imgCPU1, Mat imgCPU2,vector<KeyPoint> &kp1,vector<KeyPoint> &kp2, double a,int hessian)
{
	cuda::GpuMat imgGray1, imgGray2,gpu_dsc1,gpu_dsc2;
	cuda::GpuMat img1, img2;
	img1.upload(imgCPU1);
	img2.upload(imgCPU2);
	cuda::cvtColor(img1, imgGray1, COLOR_BGR2GRAY);
	cuda::cvtColor(img2, imgGray2, COLOR_BGR2GRAY);
	Mat cpu_gray1, cpu_gray2;
	imgGray1.download(cpu_gray1);
	imgGray2.download(cpu_gray2);

	Ptr<SURF> detector = SURF::create(hessian);
	Mat surfdescriptor1, surfdescriptor2;


	detector->detectAndCompute(cpu_gray1, noArray(), kp1, surfdescriptor1);
	detector->detectAndCompute(cpu_gray2, noArray(), kp2, surfdescriptor2);
	vector<float> dsc1,dsc2;


	Ptr<cuda::DescriptorMatcher> matcherGPU;
	matcherGPU = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	vector<vector<DMatch>> matches;
	vector<DMatch> gpu_matches;
	gpu_dsc1.upload(surfdescriptor1);
	gpu_dsc2.upload(surfdescriptor2);
	matcherGPU->knnMatch(gpu_dsc1, gpu_dsc2, matches, 2);

	vector<DMatch> mt1;



	for (int i = 0; i < matches.size(); i++) {
		if (matches[i][0].distance < a * matches[i][1].distance) {
			mt1.push_back(matches[i][0]);
		}
	}
	//cout << mt1.size() << endl;
	return mt1;
}

vector<DMatch> Stitching::getSurfMatches(Mat img1, Mat img2, double a , vector<KeyPoint>& kp1, vector<KeyPoint>& kp2, int minHessian, double b)
{
	Mat imgGray1, imgGray2;
	cvtColor(img1, imgGray1, COLOR_BGR2GRAY);
	cvtColor(img2, imgGray2, COLOR_BGR2GRAY);
	Ptr<SURF> detector = SURF::create(minHessian);
	//std::vector<KeyPoint> kp1, kp2;
	Mat surfdescriptor1, surfdescriptor2;


	detector->detectAndCompute(imgGray1, noArray(), kp1, surfdescriptor1);
	detector->detectAndCompute(imgGray2, noArray(), kp2, surfdescriptor2);


	vector<DescriptorMatcher> dsc;
	Ptr<cv::DescriptorMatcher> matcher11;
	matcher11 = BFMatcher::create(NORM_L2, false);
	vector<vector<DMatch>> matchesknn;
	matcher11->knnMatch(surfdescriptor1, surfdescriptor2, matchesknn, 2);



	for (int i = 0; i < matchesknn.size(); i++) {
		if (matchesknn[i][0].distance < a * matchesknn[i][1].distance) {
			mt1.push_back(matchesknn[i][0]);
		}
	}

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

vector<DMatch> Stitching::getOrbMatches(Mat img1, Mat img2, vector<KeyPoint>& kp1, vector<KeyPoint>& kp2,int hessian) {
	Mat gray1, gray2,dsc1,dsc2;
	cvtColor(img1, gray1, COLOR_BGR2GRAY);
	cvtColor(img2, gray2, COLOR_BGR2GRAY);
	Ptr<ORB> orb = ORB::create(hessian);
	orb->detectAndCompute(gray1,Mat(), kp1, dsc1);
	orb->detectAndCompute(gray2, Mat(), kp2, dsc2);
	std::vector<DMatch> matches;
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	matcher->match(dsc1, dsc2, matches, Mat());
	std::sort(matches.begin(), matches.end());
	const int numGoodMatches = matches.size() * 0.1f;
	matches.erase(matches.begin() + numGoodMatches, matches.end());
	return matches;

}

void Stitching::getPointsOptFlow(Mat prev_img, Mat next_img, vector<Point2f>& prev_pts, vector<Point2f>& next_pts) {
	int win_size = 10;
	// The first thing we need to do is get the features
	// we want to track.
	//
	cuda::GpuMat gpu_prev_img, gpu_next_img, gpu_cornersA, gpu_cornersB, gpu_features_found, gpu_err;

	cvtColor(prev_img, prev_img, COLOR_BGR2GRAY);
	cvtColor(next_img, next_img, COLOR_BGR2GRAY);

	gpu_prev_img.upload(prev_img);
	gpu_next_img.upload(next_img);
	Mat err;
	vector< cv::Point2f > cornersA, cornersB;
	const int MAX_CORNERS = 500;
	Ptr<cuda::CornersDetector> detector = cuda::createGoodFeaturesToTrackDetector(gpu_prev_img.type(),1000, 0.01, 5);
	detector->detect(gpu_prev_img, gpu_cornersA);


	//cv::goodFeaturesToTrack(
	//	prev_img,                         // Image to track
	//	cornersA,                     // Vector of detected corners (output)
	//	MAX_CORNERS,                  // Keep up to this many corners
	//	0.01,                         // Quality level (percent of maximum)
	//	5,                            // Min distance between corners
	//	cv::noArray(),                // Mask
	//	3,                            // Block size
	//	true,                        // true: Harris, false: Shi-Tomasi
	//	0.04                          // method specific parameter
	//);

	//cv::cornerSubPix(
	//	prev_img,                           // Input image
	//	cornersA,                       // Vector of corners (input and output)
	//	cv::Size(win_size, win_size),   // Half side length of search window
	//	cv::Size(-1, -1),               // Half side length of dead zone (-1=none)
	//	cv::TermCriteria(
	//		cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
	//		20,                         // Maximum number of iterations
	//		0.03                        // Minimum change per iteration
	//	)
	//);

	// Call the Lucas Kanade algorithm
	//
	
	Ptr<cuda::SparsePyrLKOpticalFlow> d_pyrLK_sparse = cuda::SparsePyrLKOpticalFlow::create(
		Size(win_size * 2 + 1, win_size * 2 + 1),5, 20);
	vector<uchar> features_found;
	
	
	d_pyrLK_sparse->calc(gpu_prev_img, gpu_next_img, gpu_cornersA, gpu_cornersB, gpu_features_found, gpu_err);
	gpu_cornersA.download(cornersA);
	gpu_cornersB.download(cornersB);
	//cv::calcOpticalFlowPyrLK(
	//	prev_img,                         // Previous image
	//	next_img,                         // Next image
	//	cornersA,                     // Previous set of corners (from imgA)
	//	cornersB,                     // Next set of corners (from imgB)
	//	features_found,               // Output vector, each is 1 for tracked
	//	err,                // Output vector, lists errors (optional)
	//	cv::Size(win_size * 2 + 1, win_size * 2 + 1),  // Search window size
	//	5,                            // Maximum pyramid level to construct
	//	cv::TermCriteria(
	//		cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
	//		20,                         // Maximum number of iterations
	//		0.3                         // Minimum change per iteration
	//	)
	//);

	//for (int i = 0; i < cornersA.size(); i++)
	//{
	//	if (features_found[i] == 1&& err.at<float>(i,0)<45.)
	//	{
	//		prev_pts.push_back(cornersA[i]);
	//		next_pts.push_back(cornersB[i]);
	//	}
	//}
	prev_pts = cornersA;
	next_pts = cornersB;
}

vector<DMatch> Stitching::surfCuda(Mat img1, Mat img2, vector<KeyPoint> &kp1, vector<KeyPoint> &kp2,int thresh) {

	
	//cuda::SURF_CUDA surf;
	//surf = cv::cuda::SURF_CUDA(200);
	Ptr<cuda::SURF_CUDA> surf= cv::cuda::SURF_CUDA::create(thresh,4,2,false);
	// detecting keypoints & computing descriptors
	cuda::GpuMat keypoints1GPU, keypoints2GPU;
	cuda::GpuMat descriptors1GPU, descriptors2GPU;
	cuda::GpuMat gpu_img1, gpu_img2;
	gpu_img1.upload(img1);
	gpu_img2.upload(img2);
	cuda::cvtColor(gpu_img1, gpu_img1, COLOR_BGR2GRAY);
	cuda::cvtColor(gpu_img2, gpu_img2, COLOR_BGR2GRAY);


	vector<float> descriptors1, descriptors2;
	surf->detectWithDescriptors(gpu_img1, cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
	surf->detectWithDescriptors(gpu_img2, cuda::GpuMat(), keypoints2GPU, descriptors2GPU);
	Mat dsc;
	descriptors1GPU.download(dsc);
	//surf(gpu_img1, cuda::GpuMat(), keypoints1GPU, descriptors1GPU);
	//surf(gpu_img2, cuda::GpuMat(), keypoints2GPU, descriptors2GPU);
	/*
	Mat dsc,dsc_C; 
	descriptors1GPU.download(dsc);
	char* p = (char*)dsc.data;
	cout << p << endl;
	Mat imgx= imdecode(Mat(1, dsc.rows * dsc.cols, CV_32F, (void*)dsc.data), IMREAD_ANYDEPTH);*/

	//dsc.convertTo(dsc_C, CV_8U, 255.);

	//vector<int> compressionsparam;
	//vector<uchar> encoded;
	//compressionsparam.push_back(IMWRITE_EXR_TYPE);
	//compressionsparam.push_back(80);
	//imencode(".tiff", dsc_C, encoded);
	//char *buf=(char*)encoded.data();
	//cout << encoded.data() << endl;
	//char* buf = new char[10000000];
	//cout << encoded.size() << endl;
	//cout << dsc.elemSize()*dsc.total() << endl;
	//Mat matImg = cv::imdecode(Mat(1, dsc.elemSize() * dsc.total(), CV_8U, buf), IMREAD_COLOR);
	//Mat imgF;
	//matImg.convertTo(imgF, CV_32FC1,1/255.);
	//Mat rawData = Mat(1, dsc.rows * dsc.cols, CV_8UC1, (char*)&encoded);
	//Mat imgx= imdecode(rawData, IMREAD_ANYDEPTH);

	//cout << imgx.at<float>(44,44) << endl;
	//cout << dsc.at<float>(44,44) << endl;

	//imshow("img", imgF);
	//imshow("original", dsc);

	//imshow("img", imgx);
	//cout << imgF.at<float>(12,12) << endl;
	//cout << dsc.at<float>(12, 12) << endl;

	//waitKey(0);
	cout << "FOUND " << descriptors1GPU.size() << " descriptors1GPU size" << endl;
	cout << "FOUND " << descriptors2GPU.size() << " descriptors2GPU size" << endl;

	//cout << "descriptors1GPU " << descriptors1GPU.data << " descriptors1GPU size" << endl;
	// matching descriptors
	Ptr<cuda::DescriptorMatcher> matcherGPU;
	matcherGPU = cuda::DescriptorMatcher::createBFMatcher(NORM_L2);
	vector<vector<DMatch>> matches;
	vector<DMatch> gpu_matches;
	//sort(gpu_matches.begin(), gpu_matches.end());
	matcherGPU->knnMatch(descriptors1GPU, descriptors2GPU, matches, 2);
	vector<DMatch> mt1;
	surf->releaseMemory();

	for (int i = 0; i < matches.size(); i++) {
		if (matches[i][0].distance < 0.75f * matches[i][1].distance)	 {
			mt1.push_back(matches[i][0]);
		}
	}
	surf->downloadKeypoints(keypoints1GPU, kp1);
	surf->downloadKeypoints(keypoints2GPU, kp2);

	surf->downloadDescriptors(descriptors1GPU, descriptors1);
	surf->downloadDescriptors(descriptors2GPU, descriptors2);
	sort(mt1.begin(), mt1.end());
	return mt1;

}
