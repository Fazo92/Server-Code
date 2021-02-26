#include "stitching.h"

typedef float  Float32;
typedef long float  Float64;
using namespace cv::cuda;


void main() {
	Stitching st;
	server serv;

	cuda::setDevice(0);
	//thread tk(&Stitching::getFrameRS, &st);
	//thread tkk(&Stitching::getFeaturesRS, &st);

	//thread t1(&Stitching::serialTCP, &st, 54000, 53000, 52000, 80000);
	////thread t2(&Stitching::serialTCP, &st, 58000, 59000, 60000, 90000);



	thread t1Right(&Stitching::getFrameTCP, &st, 54000, 1, "Bild1");
	thread t2Right(&Stitching::getKeyPointsTCP, &st, 53000);
	thread t3Right(&Stitching::getDescriptorTCP, &st, 52000);
	//thread t2(&Stitching::getFeatures, &st, 53000, 52000, 80000);


	thread t1Left(&Stitching::getFrameTCP, &st, 58000, 2, "Bild2");

	thread t2Left(&Stitching::getKeyPointsTCP, &st, 59000);
	thread t3Left(&Stitching::getDescriptorTCP, &st,60000);


	thread t1Center(&Stitching::getFrameTCP, &st, 40000, 3, "Bild3");

	thread t2Center(&Stitching::getKeyPointsTCP, &st, 41000);
	thread t3Center(&Stitching::getDescriptorTCP, &st, 42000);

	thread t1Bottom(&Stitching::getFrameTCP, &st, 30000, 4, "Bild4");

	thread t2Bottom(&Stitching::getKeyPointsTCP, &st, 31000);
	thread t3Bottom(&Stitching::getDescriptorTCP, &st, 32000);


	thread t5(&Stitching::realTimeStitching, &st);

	//tk.join();
	////tkk.join();
	t1Right.join();
	t2Right.join();
	t3Right.join();
	t1Left.join();
	t2Left.join();
	t3Left.join();
	t1Center.join();
	t2Center.join();
	t3Center.join();
	t1Bottom.join();
	t2Bottom.join();
	t3Bottom.join();
	////t44.join();
	//////t4.join();
	t5.join();
}