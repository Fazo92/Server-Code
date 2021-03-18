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

	
	cuda::setDevice(0);
	thread ft_C(&server::recv_features, &ser, 31000, 32000, 33000);
	thread ft_R(&server::send_fts, &ser, 21000, 22000, 23000);
	thread ft_L(&server::send_fts, &ser, 11000, 12000, 13000);
	thread ft_B(&server::send_fts, &ser, 41000, 42000, 43000);

	thread timg(&server::recv_image, &ser, 30000,960 , 1280 );		 
	thread timg2(&server::recv_image, &ser, 20000, 960, 1280);
	thread timgL(&server::recv_image, &ser, 10000, 960, 1280);
	thread timgB(&server::recv_image, &ser, 40000, 960 , 1280);

	thread tS(&server::realTimeStitching, &ser);
	ft_C.join();
	ft_R.join();
	ft_L.join();
	ft_B.join();

	timg.join();
	timg2.join();
	timgL.join();
	timgB.join();
	tS.join();
	//tshow.join();
	//while (true) {
	//	Mat dsc_C = ser.receive_dsc(clientSocket_dsc_C);
	//	vector<KeyPoint> kp_C = ser.recv_kpts(clientSocket_kp_C);
	//	
	//	cout <<"dsc Size: "<< dsc_C.size() << endl;
	//	cout << "kp_ C Size: "<< kp_C.size() << endl;
	//	//thread tR_dsc(&server::send_dsc, &ser, clientSocket_dsc_R, dsc_C);
	//	//thread tL_dsc(&server::send_dsc, &ser, clientSocket_dsc_L, dsc_C);
	//	//thread tB_dsc(&server::send_dsc, &ser, clientSocket_dsc_B, dsc_C);
	//	//ser.send_dsc(clientSocket_dsc_R, dsc_C);
	//	//ser.send_kpts(clientSocket_kp_R, kp_C);
	//	//ser.send_dsc(clientSocket_dsc_L, dsc_C);
	//	//ser.send_kpts(clientSocket_kp_L, kp_C);
	//	//ser.send_dsc(clientSocket_dsc_B, dsc_C);
	//	//ser.send_kpts(clientSocket_kp_B, kp_C);
	//	
	//	//thread tR_kp(&server::send_kpts, &ser, clientSocket_kp_R, kp_C);
	//	//thread tL_kp(&server::send_kpts, &ser, clientSocket_kp_L, kp_C);
	//	//thread tB_kp(&server::send_kpts, &ser, clientSocket_kp_B, kp_C);

	//	//tR_dsc.join();
	//	//tL_dsc.join();
	//	//tB_dsc.join();
	//	//tR_kp.join();
	//	//tL_kp.join();
	//	//tB_kp.join();
	//	//Mat warp_R=ser.recv_image(clientSocket_img_R, 480 *2, 640*2);
	//	//Mat warp_L = ser.recv_image(clientSocket_img_L, 480 * 2, 640 * 2);
	//	//Mat warp_B = ser.recv_image(clientSocket_img_B, 480 * 2, 640 * 2);
	//	//Mat pano=st.AlphaBlendingGPU()
	//	//namedWindow("warp", WINDOW_OPENGL);
	//	//imshow("warp", warp_R);
	//	//if (waitKey(1000/30) >= 0) break;
	//}
	//thread tk(&Stitching::getFrameRS, &st);
	//thread tkk(&Stitching::getFeaturesRS, &st);

	//thread t1(&Stitching::serialTCP, &st, 54000, 53000, 52000, 80000);
	//thread t2(&Stitching::serialTCP, &st, 58000, 59000, 60000, 90000);
	//thread t3(&Stitching::serialTCP, &st, 40000, 41000, 42000, 63333);
	//thread t4(&Stitching::serialTCP, &st, 30000, 31000, 32000, 6333);
	//thread tH(&Stitching::getHomoGraphyTCP, &st, 8000);

	//thread t1Right(&Stitching::getFrameTCP, &st, 54000, 1, "Bild1");
	//thread t2Right(&Stitching::getKeyPointsTCP, &st, 53000);
	//thread t3Right(&Stitching::getDescriptorTCP, &st, 52000);
	////thread t2(&Stitching::getFeatures, &st, 53000, 52000, 80000);


	//thread t1Left(&Stitching::getFrameTCP, &st, 58000, 2, "Bild2");

	//thread t2Left(&Stitching::getKeyPointsTCP, &st, 59000);
	//thread t3Left(&Stitching::getDescriptorTCP, &st,60000);


	//thread t1Center(&Stitching::getFrameTCP, &st, 40000, 3, "Bild3");

	//thread t2Center(&Stitching::getKeyPointsTCP, &st, 41000);
	//thread t3Center(&Stitching::getDescriptorTCP, &st, 42000);

	//thread t1Bottom(&Stitching::getFrameTCP, &st, 30000, 4, "Bild4");

	//thread t2Bottom(&Stitching::getKeyPointsTCP, &st, 31000);
	//thread t3Bottom(&Stitching::getDescriptorTCP, &st, 32000);

	//thread t5(&Stitching::realTimeStitching, &st);
	//t1.join();
	//t2.join();
	//t3.join();
	//t4.join();
	//tH.join();
	////tk.join();
	//////tkk.join();
	//t1Right.join();
	//t2Right.join();
	//t3Right.join();
	//t1Left.join();
	//t2Left.join();
	//t3Left.join();
	//t1Center.join();
	//t2Center.join();
	//t3Center.join();
	//t1Bottom.join();
	//t2Bottom.join();
	//t3Bottom.join();
	//////t44.join();
	////////t4.join();
	//t5.join();
	return ;
}