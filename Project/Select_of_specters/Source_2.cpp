#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#define  LOG_ENABLE
#include "log.h"

using namespace cv;
using namespace std;

enum Color {
	RED,
	BLUE
};

struct Spectr {
	int x0;
	int y0;
	int xCentr;
	int yCentr;
	int width;
	int height;
};


const static int BACKGROUND_DEPTH_X = 190;
const static int BACKGROUND_DEPTH_Y = 210;


void Paint(Mat src, int x, int y, Color color) {
	Mat pix = src.col(x).row(y);
	//log("pix", pix.at<uchar>(0, 0));
	switch (color)
	{
	case RED:
		pix -= Scalar(255, 255, 0);
		break;
	case BLUE:
		pix -= Scalar(0, 255, 255);
		break;
	default:
		break;
	}
}

void PaintRect(Mat src, Spectr sp) {
	for (int i = sp.x0; i < sp.x0 + sp.width; i++) {
		Mat pixUp = src.col(i).row(sp.y0);
		//log("PixUp", pixUp);
		pixUp -= Scalar(255, 255, 0);
		Mat pixDown = src.col(i).row(sp.height + sp.y0);
		//log("PixDown", pixDown);
		pixDown -= Scalar(255, 255, 0);
	}
	for (int i = sp.y0; i < sp.y0 + sp.height; i++) {
		Mat pixLeft = src.col(sp.x0).row(i);
		//log("PixLeft", pixLeft);
		pixLeft -= Scalar(255, 255, 0);
		Mat pixRight = src.col(sp.x0 + sp.width-1).row(i);
		//log("PixRight", pixRight);
		pixRight -= Scalar(255, 255, 0);
	}
}


// ���=����� ������� �������� �������� � �����
//double MiddleOfBlock(int x0, int y0, Mat& imgGrey) {
//	int pix = 0;
//	double result = 0;
//	for (int i = x0; i < x0 + 5; i++) {
//		for (int j = y0; j < y0 + 2; j++) {
//			pix += (int)imgGrey.at<uchar>(i, j);
//		}
//	}
//	result = pix / 10;
//	return result;
//}



//int SerchingLineX(int x0, int y0, Mat imgGrey, Mat&src) {
//	
//	while (x0 < (int)imgGrey.cols) {
//		Mat curPix = imgGrey.col(x0).row(y0);
//		double MVal =(int) curPix.at<uchar>(0, 0);
//		//log("MVal", MVal);
//		//BG
//		if (MVal >= BACKGROUND_DEPTH_X) {
//			Paint(src, x0, y0, RED);
//			x0++;
//		}
//		//SPECTR
//		else {
//			Paint(src, x0, y0, BLUE);
//			x0++;
//		}
//	}
//	
//	return x0;
//}

int BGX(int x0, int y0, Mat imgGrey, Mat&src) {

	while (x0 < (int)imgGrey.cols) {
		Mat curPix = imgGrey.col(x0).row(y0);
		double MVal = curPix.at<uchar>(0, 0);
		//log("MVal", MVal);
		//BG
		if (MVal >= BACKGROUND_DEPTH_X) {
			//Paint(src, x0, y0, RED);
			x0++;
		}
		//SPECTR Start
		else {
			return x0;
		}
	}
	return x0;
	
}


int SpectrX(int x0, int y0, Mat imgGrey, Mat&src) {
	while (x0 < (int)imgGrey.cols) {
		Mat curPix = imgGrey.col(x0).row(y0);
		double MVal = curPix.at<uchar>(0, 0);
		//log("MVal", MVal);
		//SPECTR

		if (MVal < BACKGROUND_DEPTH_X) {
			//Paint(src, x0, y0, BLUE);
			x0++;
		}
		//BG
		else {
			return x0;
		}
	}
	return x0;
}
//int SerchingLineY(int x0, int y0, Mat imgGrey, Mat&src) {
//	int y = y0;
//	while (y > 0) {
//		Mat curPix = imgGrey.col(x0).row(y);
//		double MVal = (int)curPix.at<uchar>(0, 0);
//		log("MVal", MVal);
//		//BG
//		if (MVal < BACKGROUND_DEPTH_Y) {
//			//Paint(src, x0, y, RED);
//			y--;
//		}
//		//SPECTR
//		else {
//			//Paint(src, x0, y, BLUE);
//			y--;
//		}
//	}
//	return y;
//}
int BGY(int x0, int y0, Mat imgGrey, Mat&src) {
	int y = y0;
	while (y > 0) {
		Mat curPix = imgGrey.col(x0).row(y);
		double MVal = curPix.at<uchar>(0, 0);
		//log("MVal", MVal);
		//BG
		if (MVal > BACKGROUND_DEPTH_Y) {
			//Paint(src, x0, y0, RED);
			y--;
		}
		//SPECTR Start
		else {
			return y;
		}
	}
	return y;

}


int SpectrY(int x0, int y0, Mat imgGrey, Mat&src) {
	int y = y0;
	while (y > 0) {
		Mat curPix = imgGrey.col(x0).row(y);
		double MVal = curPix.at<uchar>(0, 0);
		//log("MVal", MVal);

		if (MVal <= BACKGROUND_DEPTH_Y) {
			//Paint(src, x0, y0, BLUE);
			y--;
		}
		//SPECTR Start
		else {
			return y;
		}
	}
	return y;
}

void SpectrParametrs(Mat imgGrey, Mat& src) {
	//log("SpectrParametrs start");
	int x0 = 0, y0 = 0;
	int middleY = (int)(imgGrey.rows / 2) - 20;
	//log("middleY", middleY);
	//log("pixStart", imgGrey.at<uchar>(x0, middleY));
	vector<Spectr> spectrsExamples;
	
	while (x0 < (int)imgGrey.cols) {
		Spectr spec;
		int x = BGX(x0, middleY, imgGrey, src);
		int x1 = SpectrX(x, middleY, imgGrey, src);
		int centrSp = (x1 + x) / 2;
		//log("x1", x1);
		int yUp = SpectrY(centrSp, middleY, imgGrey, src);
		//log("->",yUp);
		//int y1Up = BGY(centrSp, middleY , imgGrey, src);
		int yDown = SpectrY(centrSp, middleY + (middleY - yUp), imgGrey, src);
		spec.x0 = x;
		spec.xCentr = centrSp;
		spec.y0 = yUp;
		spec.yCentr = middleY;
		spec.width = x1 - x;
		spec.height = (middleY - yUp) * 2;
		spectrsExamples.push_back(spec);
		PaintRect(src, spec);
		x0 = x1;
		y0 = yUp;

	}
	
	



	//log("First line");
	//int tmp = SerchingLineX(x0, middleY, imgGrey, src);
	//x0 = tmp;
	//
	//tmp = SerchingLineY(x0, middleY, imgGrey, src);
	//y0 = tmp;


}


int main() {
	Mat src = imread("SrcImg.png");
	Mat imgGrey;
	int pix, pixNext;
	int count = 0;
	cvtColor(src, imgGrey, CV_BGR2GRAY);  // convert to Grey

	SpectrParametrs(imgGrey, src);
	//cout << (int) imgGrey.at<uchar>(1, 1)  << "      ";
	//imshow("ResSRC", src);
	imshow("ResSRC", src);
	waitKey(0);
	system("pause");
	return 0;
}