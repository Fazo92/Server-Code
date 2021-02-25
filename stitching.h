#pragma once
#include <iostream>
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/core.hpp"
#include "opencv2/stitching.hpp"
#include <opencv2/cudaarithm.hpp>
#include <chrono>
#include <mutex>
#include <map> 
#include <opencv2/xfeatures2d/cuda.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include "opencv2/opencv_modules.hpp"
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <chrono>
#include "server.h"
#include <opencv2/videoio.hpp>
#include <librealsense2/rs.hpp>
#include <opencv2/cudawarping.hpp>
//#include <opencv2/flann.hpp>

#pragma comment (lib,"ws2_32.lib")



using namespace std;
using namespace cv;
using namespace xfeatures2d;

class Stitching {
	Mat img, dsctmp;
	vector<KeyPoint> kptmp, kptmpright,kptmpleft,kptmpcenter,kptmpbottom;
	Mat frametmpright,frametmpleft,frametmpcenter,frametmpbottom;
	bool features = false;
	static void warpThread(Mat dsc1, Mat dsc2, vector<KeyPoint> kp1, vector<KeyPoint> kp2, int& a, Mat img, Mat& warpImg);
	bool rs = false;
public:
	server serv;
	void getFeatures(int port1, int port2, int port3);
	void serialUDP(int port1, int port2, int port3, int port4);
	void serialTCP(int port1, int port2, int port3, int port4);
	cuda::GpuMat dscGPUnew;
	Mat dscnew1;
	Mat dscRight, dscCenter, dscLeft,dscBottom;
	void CamFeatures();
	Mat camFrame;

	void getFrameTCP(int port, int imgNumber, String windowname);
	void getKeyPointsTCP(int port);
	void getDescriptorTCP(int port);
	void getCudaDescriptorsTCP(int port);
	void realTimeStitching();
	int dscSize = 1000;
	Mat imgLeft, imgRight, imgCenter, imgBottom1, imgBottom2, hR, hL, hB1, hB2, warpLeft, warpRight, warpDown, warpDown2;
	cuda::GpuMat imgLeftGPU, imgRightGPU, imgCenterGPU, imgBottom1GPU, imgBottom2GPU, warpLeftGPU, warpRightGPU, warpDownGPU, warpDown2GPU;


	void CudaStitch(cuda::GpuMat& img1, cuda::GpuMat& img2, cuda::GpuMat& img3, cuda::GpuMat& img4);
	vector <KeyPoint> m_kpL, m_kpR, m_kpC, m_kpB1, m_kpB2, m_kptemp;
	vector <DMatch> mt1, mt2;
	//Stitching(String path1, String path2, String path3, String path4);
	Stitching();

	vector<DMatch>  getSiftmatches(Mat& img1, Mat& img2, float a);
	vector<DMatch> getSiftmatchesFlann(Mat img1, Mat img2, float ratio_thresh);
	vector<KeyPoint> getCurrentKeypoints(Mat img);
	void setKeypoints(Mat img, vector <KeyPoint> kp);
	Mat getHomography(vector <DMatch> match, vector<KeyPoint> kpObj, vector<KeyPoint> kpScn);

	Rect findbiggestContour(Mat img);
	Mat getAlpha(Mat dst1, Mat dst2);

	vector<DMatch> getSureMatches(Mat imgCPU1, Mat imgCPU2, double a);
	void VideoStream(String path, String windowname, double fps, Mat& frame);
	Mat AlphaBlending(Mat img, Mat addImg);
	void StitchAllImgs(cuda::GpuMat& warpBottom1, cuda::GpuMat& warpBottom1GPU, cuda::GpuMat& warpRight, cuda::GpuMat& warpLeft);
	Mat lastPano;
	void streamRealSense();
	Mat getBeta(Mat dst1, Mat dst2);
	void recenterDFT(Mat& source);
	void inverseDFT(Mat& source, Mat& destination);
	void takeDFT(Mat& source, Mat& destination);
	void showDFT(Mat& source);
	Mat makeNormalize(Mat img);

	Rect lastRct(Mat img, int a = 0);
	vector<Mat> takeCapture();
	void getFrameRS();
	void getFeaturesRS();
	vector<DMatch> getSurfMatches(Mat img1, Mat img2, double a, double b = 0, int minHessian = 400);
	void cropImg(Mat& img, Rect rct, int d);
	void removeBlackPoints(Mat& img);


};