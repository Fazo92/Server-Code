#include "stitching.h"

typedef float  Float32;
typedef long float  Float64;
using namespace cv::cuda;

void showImage(Mat img) {
	while (true) {
		if (img.empty()) continue;
		imshow("img", img);
		if (waitKey(1000 / 60) >= 0) break;
	}
}
void main() {
	Stitching st;
	server ser;	


	//cuda::setDevice(0);
	thread ft_C(&server::recv_features, &ser, 31000, 32000, 33000);
	thread ft_R(&server::send_fts, &ser, 21000, 22000, 23000);
	thread ft_L(&server::send_fts, &ser, 11000, 12000, 13000);
	thread ft_B(&server::send_fts, &ser, 41000, 42000, 43000);

	thread timg(&server::recv_image, &ser, 30000,960 , 1280 );		 
	thread timg2(&server::recv_image, &ser, 20000, 960, 1280);
	thread timgL(&server::recv_image, &ser, 10000, 960, 1280);
	thread timgB(&server::recv_image, &ser, 40000, 960 , 1280);

	thread t_rect_R(&server::recv_obstacle, &ser, 24000);
	thread t_rect_L(&server::recv_obstacle, &ser, 14000);
	thread t_rect_B(&server::recv_obstacle, &ser, 44000);
	thread t_rect_C(&server::recv_obstacle, &ser, 34000);

	//thread t_rect_max(&server::find_biggst_rct, &ser);
	
	while (true) {


		if (ser.m_img_C.empty() || ser.m_warp_R.empty() || ser.m_warp_L.empty() || ser.m_warp_B.empty()) continue;
		//std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		ser.muC.lock();
		Mat imgC = ser.m_img_C.clone();
		ser.muC.unlock();

		ser.muR.lock();
		Mat warpR = ser.m_warp_R.clone();
		ser.muR.unlock();

		ser.muL.lock();
		Mat warpL = ser.m_warp_L.clone();
		ser.muL.unlock();

		ser.muB.lock();
		Mat warpB = ser.m_warp_B.clone();
		ser.muB.unlock();

		Mat pano = st.AlphaBlendingGPU(imgC, warpR, 1., 0);
		Mat pano2 = st.AlphaBlendingGPU(pano, warpL, 1., 1);
		//Mat pano3 = st.AlphaBlendingGPU(pano2, warpB, 1., 2);

		//cuda::GpuMat pano_GPU;
		//pano_GPU.upload(pano2);
		//namedWindow("pano", WINDOW_OPENGL);
		//Rect rct_all= ser.biggest_Rct;
		//rectangle(pano2, ser.m_rect_R[0], cv::Scalar(0, 255, 0), 1, 8, 0);
		//rectangle(pano2, ser.m_rect_L[0], cv::Scalar(0, 255, 0), 1, 8, 0);
		//rectangle(pano2, ser.m_rect_C[0], cv::Scalar(0, 255, 0), 1, 8, 0);
		if(!st.findbiggestContour(imgC).contains(ser.m_rect_R[0].br()))
		rectangle(pano2, ser.m_rect_R[0], cv::Scalar(0, 255, 0), 1, 8, 0);

		imshow("pano", pano2);
		//std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		//std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
		if (waitKey(10000/30) >= 0) break;

	}
	//thread tS(&server::realTimeStitching, &ser);
	ft_C.join();
	ft_R.join();
	ft_L.join();
	ft_B.join();

	timg.join();
	timg2.join();
	timgL.join();
	timgB.join();
	t_rect_R.join();
	t_rect_L.join();
	t_rect_B.join();
	t_rect_C.join();
	//t_rect_max.join();
	//tS.join();
	
	return ;
}