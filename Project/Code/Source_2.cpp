#include <opencv2\opencv.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>

#define  LOG_ENABLE
#include "log.h"

using namespace cv;
using namespace std;

enum Color {
	RED,
	BLUE,
	GREEN
};


int BACKGROUND_DEPTH_X;
int BACKGROUND_DEPTH_Y;
int BACKGROUND_DEPTH_SPEC;
int RANGE_OF_PERMISSIBLE_SHIFT_OF_SPECTRAL_LINE;

struct SpLines {
	int xCentr;
	int yCentr;
	int height;

};

struct Spectr {
	int x0; 
	int y0;
	int xCentr;
	int yCentr;
	int width;
	int height;
	vector<SpLines> lines;
	Spectr(int x0, int y0, int xCentr, int yCentr, int width, int height) {
		this->x0 = x0;
		this->y0 = y0;
		this->xCentr = xCentr;
		this->yCentr = yCentr;
		this->width = width;
		this->height = height;
	}
};


// Paint Block of Functions
void Paint(Mat src, int x, int y, Color color) {
	Mat pix = src.col(x).row(y);
	switch (color)
	{
	case RED:
		pix -= Scalar(255, 255, 0);
		break;
	case BLUE:
		pix -= Scalar(0, 255, 255);
		break;
	case GREEN:
		pix -= Scalar(255, 0, 255);
	default:
		break;
	}
}

void PaintRect(Mat src, Spectr sp) {
	Rect area(sp.x0, sp.y0, sp.width, sp.height);
	rectangle(src, area, RED);
}

//_____________________________

//Correction of ERRORS

double MiddleOfBlock(int x0, int y0, Mat& imgGrey) {
	int minX = ((x0 - 1) < 0) ? x0 : (x0 - 1);
	int maxX = ((x0 + 1) > imgGrey.cols) ? x0 : (x0 + 1);
	int minY = ((y0 - 1) < 0) ? y0 : (y0 - 1);
	int maxY = ((y0 + 1) > imgGrey.rows) ? y0 : (y0 + 1);
	int pix = 0;
	double result = 0;
	for (int i = minX; i < maxX; i++) {
		for (int j = minY; j < maxY; j++) {
			Mat curPix = imgGrey.col(i).row(j);
			pix += curPix.at<uchar>(0, 0);
		}
	}
	result = pix / ((maxX - minX)*(maxY - minY));
	return result;
}

void CorrectByWidth(vector<Spectr>& spectrs) {
	vector<Spectr> result;
	vector<int> widths;
	int averWidth = 0;
	for (int i = 0; i < spectrs.size(); i++) {
		widths.push_back(spectrs[i].width);
	}
	sort(widths.begin(),widths.end());
	int idealWidth = widths[(int)widths.size() / 2];
	//log("Ideal Width", idealWidth);
	for (int i = 0; i < spectrs.size(); i++) {
		if (abs(spectrs[i].width - idealWidth) > (idealWidth / 2)) {
			spectrs.erase(spectrs.begin() + i);
			i--;
		}
		
	}
}
void CorrectByHeight(vector<Spectr>& spectrs) {
	vector<Spectr> result;
	vector<int> height;
	int averHeight = 0;
	for (int i = 0; i < spectrs.size(); i++) {
		height.push_back(spectrs[i].height);
	}
	sort(height.begin(), height.end());
	int idealHeight = height[(int)height.size() / 2];
	//log("Ideal Height", idealHeight);
	for (int i = 0; i < spectrs.size(); i++) {
		if (abs(spectrs[i].height - idealHeight) >(idealHeight / 2)) {
			spectrs.erase(spectrs.begin() + i);
			i--;
		}

	}
}
int BgColorImg(Mat imgGrey) {
	vector<double> bgCol;
	int y0 = 0.01*imgGrey.rows;
	int resBg = 255;
	for (int i = 0; i < imgGrey.cols; i++) {
		bgCol.push_back(MiddleOfBlock(i, y0, imgGrey));
	}
	resBg = *(max_element(bgCol.begin(), bgCol.end()));
	//log("resg", resBg);
	
	return resBg;
}


//______________________
int BGX(int x0, int y0, Mat imgGrey, Mat&src) {
	
	//log("BGX", BACKGROUND_DEPTH_X);
	while (x0 < (int)imgGrey.cols) {
		//Mat curPix = imgGrey.col(x0).row(y0);
		double MVal = MiddleOfBlock(x0, y0, imgGrey);   //curPix.at<uchar>(0, 0);
		//log("MVal", MVal);
		//BG
		if (MVal >= BACKGROUND_DEPTH_X) {
			Paint(src, x0, y0, RED);
			x0++;
			//log("X", x0);
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
		//Mat curPix = imgGrey.col(x0).row(y0);
		double MVal = MiddleOfBlock(x0, y0, imgGrey);   //curPix.at<uchar>(0, 0);
		//log("MVal", MVal);
		//SPECTR
		if (MVal < BACKGROUND_DEPTH_X) {
			Paint(src, x0, y0, BLUE);
			x0++;
		}
		//BG
		else {
			return x0;
		}
	}
	return x0;
}


int SpectrY(int x0, int y0, Mat imgGrey, Mat&src) {
	int y = y0;
	while (y > 0) {
		//Mat curPix = imgGrey.col(x0).row(y);
		double MVal = MiddleOfBlock(x0, y, imgGrey);   //curPix.at<uchar>(0, 0);

		if (MVal <= BACKGROUND_DEPTH_Y) {
			Paint(src, x0, y0, BLUE);
			y--;
		}
		else {
			return y;
		}
	}
	return y;
}

vector<Spectr> DetectionOfSpectrs(Mat imgGrey, Mat& src) {
	//log("SpectrParametrs start");
	int x0 = 0, y0 = 0;
	int middleY = (int)(imgGrey.rows / 2) - 20;
	vector<Spectr> spectrsExamples;
	BACKGROUND_DEPTH_X = BgColorImg(imgGrey)*0.82;
	BACKGROUND_DEPTH_Y = BgColorImg(imgGrey)*0.90;
	//log("BACKGROUND_DEPTH_X", BACKGROUND_DEPTH_X);
	//log("BACKGROUND_DEPTH_Y", BACKGROUND_DEPTH_Y);
	while (x0 < (int)imgGrey.cols) {
		int x = BGX(x0, middleY, imgGrey, src);
		//log("X", x);
		int x1 = SpectrX(x, middleY, imgGrey, src);
		//log("X1", x1);
		int centrSp = (x1 + x) / 2;
		int yUp = SpectrY(centrSp, middleY, imgGrey, src);
		//log("YUP", yUp);
		int yDown = SpectrY(centrSp, middleY + (middleY - yUp), imgGrey, src);
		//log("YDOWN", yDown);
		Spectr spec(x, yUp, centrSp, middleY, (x1 - x), ((middleY - yUp) * 2));
		//log("Spectr");
		spectrsExamples.push_back(spec);
		//log("Paint");
		x0 = x1;
		y0 = yUp;
	}

	CorrectByWidth(spectrsExamples);
	CorrectByHeight(spectrsExamples);
	for (int i = 0; i < spectrsExamples.size(); i++) {
		Spectr spec = spectrsExamples[i];
		//log("width", spectrsExamples[i].width);
		PaintRect(src, spec);
		//log("Paint");
	}
	return spectrsExamples;

}

// SpectrsLine
int BgColorSpectr(Mat imgGrey, Spectr spec) {
	vector<double> bgCol;
	int resBg = 255;
	for (int i = spec.y0 + spec.height; i > spec.y0; i--) {
		
		bgCol.push_back(MiddleOfBlock(spec.xCentr, i, imgGrey));
		
	}
	
	resBg = *(max_element(bgCol.begin(), bgCol.end()));
	return resBg;
}
bool CheckingOkDiapazonLines(int lastLine, int newLine, int heightOfSpectr) {
	RANGE_OF_PERMISSIBLE_SHIFT_OF_SPECTRAL_LINE = heightOfSpectr / 50;
	if (abs(lastLine - newLine) < RANGE_OF_PERMISSIBLE_SHIFT_OF_SPECTRAL_LINE)
		return true;

	return false;
}


int ChooseX(int side) {
	int sign;
	
		if (side == 0) {
			sign = -1;
		}
		else {
			if (side == 1) {
				sign = 0;
			}
			else {
				sign = 1;
			}
		}
		return sign;
}

void PaintLine (Mat& src, int x, int y,int heightY, Color color) {
	for (int i = y - (heightY / 2); i < y + (heightY / 2); i++) {
		Mat pix = src.col(x).row(i);
		switch (color)
		{
		case RED:
			pix -= Scalar(255, 255, 0);
			break;
		case BLUE:
			pix -= Scalar(0, 255, 255);
			break;
		case GREEN:
			pix -= Scalar(255, 0, 255);
		default:
			break;
		}
	}
	
}
void DetectionSpectrLines(vector<Spectr> spectrs, Mat imgGrey, Mat &src) {
	const int countOfBlocks = 16;
	for (int i = 0; i < spectrs.size(); i++) {
		int x0;
		SpLines line;
		vector<SpLines> lines;
		int delta = spectrs[i].width / 5;
		int y1 = spectrs[i].y0;
		double sizeOfBlock = (spectrs[i].height / countOfBlocks);
		//log("point 1");
		vector <int> arrLines;
		for (int k = countOfBlocks; k > 0; k--) {
			bool flag = false;
			for (int side = 0; side < 3; side++) {
				flag = false;
				x0 = spectrs[i].xCentr + ChooseX(side)*delta;
				Spectr block(spectrs[i].x0, y1, x0, y1 - (sizeOfBlock / 2), spectrs[i].width, sizeOfBlock);
				BACKGROUND_DEPTH_SPEC = BgColorSpectr(imgGrey, block)*0.82;
				//log("BACKGROUND_DEPTH_SPEC", BACKGROUND_DEPTH_SPEC)
				int yInput = 0, yOutput = 0;
				for (int j = y1; j < y1 + sizeOfBlock; j++) {
					Mat curPix = imgGrey.col(x0).row(j);  
					double MVal = curPix.at<uchar>(0, 0);
					if (MVal < BACKGROUND_DEPTH_SPEC) {
						if (!flag) {
							yInput = j;
							flag = true;
						}
					}
					else {
						if (flag){
							yOutput = j;
							arrLines.push_back((yOutput + yInput) / 2);
							flag = false;
							//log("in", yInput, "out", yOutput);
						}
						Paint(src, x0, j, RED);
					}
					if ((j == y1 + sizeOfBlock - 1)&&(flag)) {
						yOutput = j;
						arrLines.push_back((yOutput + yInput) / 2);
						flag = false;
						//log("in", yInput, "out", yOutput);
					}
					
				}

			}
			
			y1 += sizeOfBlock;
			Spectr sp(spectrs[i].x0, y1, spectrs[i].xCentr, y1 - (sizeOfBlock / 2), spectrs[i].width, sizeOfBlock);
			//PaintRect(src, sp);
		}
		//log("size", (int)arrLines.size());
		std::sort(arrLines.begin(), arrLines.end());
		//log("point 2");
		for (int l = 0; l < arrLines.size(); l++) {
			int av, tmp;
			int oneOf = arrLines[l];
			av = oneOf;
			//log("point 3");
			for (int h = l + 1; h < arrLines.size(); h++) {
				if (CheckingOkDiapazonLines(oneOf, arrLines[h], spectrs[i].height) == true) {
					av = (av + arrLines[h]) / 2;
					arrLines.erase(arrLines.begin() + (h));
					arrLines[l] = av;
					h--;
				}
				else {
					//arrLines.erase(arrLines.begin() + l);
					//l--;
					break;
				}
			}

		}
		//log("size", (int)arrLines.size());
		//log("point 4");
		for (int l = 0; l < arrLines.size(); l++) {
			//log("arrLinr[l]", arrLines[l]);		
			line.xCentr = spectrs[i].xCentr;
			line.yCentr = arrLines[l];
			line.height = RANGE_OF_PERMISSIBLE_SHIFT_OF_SPECTRAL_LINE/2;
			lines.push_back(line);
			PaintLine(src, line.xCentr, line.yCentr, line.height, BLUE);
			PaintLine(src, line.xCentr - 1, line.yCentr, line.height, BLUE);
			PaintLine(src, line.xCentr + 1, line.yCentr, line.height, BLUE);
			imshow("GREEN", src);

		}
		spectrs[i].lines = lines;
	}

}




int main() {
	Mat src = imread("2.jpg");
	Mat imgGrey;
	int pix, pixNext;
	int count = 0;
	cvtColor(src, imgGrey, CV_BGR2GRAY);  // convert to Grey
	vector<Spectr> detectSpectrs;
	detectSpectrs= DetectionOfSpectrs(imgGrey, src);
	DetectionSpectrLines(detectSpectrs,imgGrey,src);

	//imshow("ResSRC", src);
	waitKey(0);
	system("pause");
	return 0;
}