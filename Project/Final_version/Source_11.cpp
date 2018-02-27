#include <opencv2\opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#define LOG_ENABLE

#include "log.h"

using namespace cv;
using namespace std;

// Цвета
struct Colors {
	Scalar RED = Scalar(0, 0, 255);
	Scalar GREEN = Scalar(0, 255, 0);
	Scalar BLUE = Scalar(255, 0, 0);
	Scalar PURPLE = Scalar(173, 66, 209);
	Scalar LIGHT_BLUE = Scalar(0, 208, 255);
}COLORS;


int BACKGROUND_DEPTH_X = 140;
int BACKGROUND_DEPTH_Y = 150;
int AVERAGE_SPECTRE_WIDTH = 55;
const static int ERR_NOT_FOUND = -1;

// Структура для хранения спектра
struct SpectreArea {
	int id;
	int x0;
	int y0;
	int width;
	int height;
	int middleX;
	int middleY;
};

// Отрисовать точку
void Paint(Mat &src, int x, int y, Scalar color) {
	Mat pix = src.col(x).row(y);
	/*Mat pix2 = src.col(x - 1).row(y);
	Mat pix3 = src.col(x + 1).row(y);*/
	pix = color;
	/*pix2 = color;
	pix3 = color;*/
}

// Отрисовать прямоугольник (для спектра)
void PaintSpectreArea(Mat& src, const SpectreArea& tube, Scalar color) {
	Rect area(tube.x0, tube.y0, tube.width, tube.height);
	rectangle(src, area, color, 2);
}

// Получить среднее значение пикселя с учотом области вокруг него
int GetAverageDepth(const Mat& imgGrey, int x0, int y0) {
	int result(0);

	int minX = ((x0 - 1) < 0) ? x0 : (x0 - 1);
	int maxX = ((x0 + 1) > imgGrey.cols) ? x0 : (x0 + 1);
	int minY = ((y0 - 1) < 0) ? y0 : (y0 - 1);
	int maxY = ((y0 + 1) > imgGrey.rows) ? y0 : (y0 + 1);
	int divider = (maxX - minX) * (maxY - minY);

	for (int i(minX); i < maxX; i++) {
		for (int j(minY); j < maxY; j++) {
			result += imgGrey.col(i).row(j).at<uchar>(0, 0);
		}
	}
	result /= divider;

	return result;
}

// Поиск левой границы спектра
int FindEnterInTubeX(int x0, int y0, Mat imgGrey, Mat& src) {
	while (x0 < (int)imgGrey.cols) {
		double MVal = GetAverageDepth(imgGrey, x0, y0);
		if (MVal >= BACKGROUND_DEPTH_X) {
			Paint(src, x0, y0, COLORS.RED);
		}
		else {
			return x0;
		}
		x0++;
	}
	return ERR_NOT_FOUND;
}

// Поиск правой границы спектра
int FindEndOfTubeX(int x0, int y0, Mat imgGrey, Mat& src) {
	while (x0 < (int)imgGrey.cols) {
		double MVal = GetAverageDepth(imgGrey, x0, y0);
		if (MVal < BACKGROUND_DEPTH_X) {
			Paint(src, x0, y0, COLORS.BLUE);
		}
		else {
			break;
		}
		x0++;
	}
	return x0;
}

// Поиск верхней границы спектра
int FindUpBoardOfTubeY(int x0, int y0, Mat imgGrey, Mat& src) {
	while (y0 >= 0) {
		double MVal = GetAverageDepth(imgGrey, x0, y0);
		if (MVal <= BACKGROUND_DEPTH_Y) {
			Paint(src, x0, y0, COLORS.GREEN);
		}
		else {
			break;
		}
		y0--;
	}
	return y0;
}

// Удаление некореткно найденных по ширине спектров
void CorrectByWidth(vector<SpectreArea>& spectrs) {
	vector<int> widths;
	int averWidth = 0;
	for (int i = 0; i < spectrs.size(); i++) {
		widths.push_back(spectrs[i].width);
	}
	sort(widths.begin(), widths.end());

	int idealWidth = widths[(int)widths.size() / 2];

	for (int i = 0; i < spectrs.size(); i++) {
		if (abs(spectrs[i].width - idealWidth) >(idealWidth / 2)) {
			spectrs.erase(spectrs.begin() + i);
			i--;
		}
	}
}


// Корректировка высоты
void CorrectHeight(vector<SpectreArea>& spectrs) {
	vector<int> height;
	vector<int> y0;
	for (int i = 0; i < spectrs.size(); i++) {
		height.push_back(spectrs[i].height);
		y0.push_back(spectrs[i].y0);
	}
	sort(height.begin(), height.end());
	sort(y0.begin(), y0.end());

	int idealHeight = height[(int)height.size() / 2];
	int idealY0 = y0[(int)y0.size() / 2];

	for (int i = 0; i < spectrs.size(); i++) {
		if (abs(spectrs[i].height - idealHeight) >(idealHeight * 0.3)) {
			spectrs[i].y0 = idealY0;
			spectrs[i].height = idealHeight;
		}
	}
}

// Получить глубину фона
int GetBackgroundDepth(Mat imgGrey) {
	int result;
	vector<int> bckgrdDepths(imgGrey.cols);

	for (int i(0); i < imgGrey.cols; i++) {
		bckgrdDepths[i] = GetAverageDepth(imgGrey, i, imgGrey.rows*0.01);
	}
	result = *(max_element(bckgrdDepths.begin(), bckgrdDepths.end()));
	log("result", result);

	return result;
}

// Выделение максимально широкого спектра (для поиска спектров)
SpectreArea FindMaxWidthOfSpectre(vector<SpectreArea>& spectres) {
	sort(spectres.begin(), spectres.end(), [](const SpectreArea& spectre1, const SpectreArea& spectre2) {
		return spectre1.width < spectre2.width;
	});
	return spectres[spectres.size() - 1];
}

// Поиск высоты спектра
void FindHeightOfSpectre(vector<SpectreArea>& spectres, int middleY, Mat imgGrey, Mat& src) {
	for (int i(0); i < spectres.size(); i++) {
		vector<int> vec;
		int minBoard = middleY - 30;
		int maxBoard = middleY;
		for (int k(minBoard); k <= (maxBoard); k++) {
			vector<int> upperBoardOfTubeY;
			for (int j(spectres[i].x0); j < (spectres[i].x0 + spectres[i].width); j += 1) {
				upperBoardOfTubeY.push_back((FindUpBoardOfTubeY(j, k, imgGrey, src)));
			}
			sort(upperBoardOfTubeY.begin(), upperBoardOfTubeY.end());
			int avgUpperBoardOfTubeY = upperBoardOfTubeY[upperBoardOfTubeY.size() / 2];
			vec.push_back(avgUpperBoardOfTubeY);
		}

		int idx = min_element(vec.begin(), vec.end()) - vec.begin();

		spectres[i].y0 = vec[idx];
		spectres[i].height = (middleY - spectres[i].y0) * 2;
	}
}

// Получить глубину фона на определенном участке (по высоте)
int GetAverageDepthOfTubeBackground(Mat imgGrey, const SpectreArea& tube, int yStart, int yEnd) {
	int result;
	int sumOfDepth = 0;
	vector<int> vecOfDepths;

	for (int i = yStart; i < yEnd; i++) {
		//sumOfDepth += imgGrey.col(tube.middleX).row(i).at<uchar>(0, 0);
		vecOfDepths.push_back(imgGrey.col(tube.middleX).row(i).at<uchar>(0, 0));
	}

	//result = sumOfDepth / (yEnd - yStart);
	result = *(max_element(vecOfDepths.begin(), vecOfDepths.end()));

	return result;
}


void  CompareThisFUN(Mat& imgGrey2, const vector<SpectreArea>& tubes) {
	vector<vector<int>> result;
	for (int i = 0; i < tubes.size(); i++) {
		int countLooksLike = 0;
		vector<int> id;
		cout << "i: " << i << endl;
		for (int j = i + 1; j < tubes.size(); j++) {
			int countLooksLike = 0;
			int x0I = tubes[i].x0;
			int x0J = tubes[j].x0;
			for (int k = 0; k < imgGrey2.rows; k++) {
				int curPixI = (int)imgGrey2.col(x0I).row(k).at<uchar>(0, 0);
				int curPixJ = (int)imgGrey2.col(x0J).row(k).at<uchar>(0, 0);
				if (curPixI == curPixJ) {
					countLooksLike++;
				}
			}
			double OnePers = tubes[i].height / 100;//one persent
			
			double pers = ((countLooksLike-(imgGrey2.rows- tubes[i].height))/OnePers);
			//cout  << "PERSENT " << pers << endl;
			if (pers > 90) {
				id.push_back(j);
				cout << "  ->   " << j << endl;
			}
		}
		result.push_back(id);
		cout << endl;
	}
}


void CleaningBG(Mat& imgGrey2) {
	for (int i = 0; i < imgGrey2.cols; i++) {
		for (int j = 0; j < imgGrey2.rows; j++) {
			int curPix = (int)imgGrey2.col(i).row(j).at<uchar>(0, 0);
			if ((curPix != 0) && (curPix != 255)) {
				imgGrey2.col(i).row(j).at<uchar>(0, 0) = 255;
			}
		}
	}
	imshow("CLEAN", imgGrey2);
}

void BWFUNNY(Mat& imgGrey2, const vector<SpectreArea>& tubes) {
	int bg = 255;
	int informPix = 0;
	
	for (int i = 0; i < tubes.size(); i++) {
		//log("width", (0.8*tubes[i].width));
		for (int j = tubes[i].y0; j < tubes[i].y0+tubes[i].height; j++) {
			int count = 0;
			for (int k = tubes[i].x0; k < tubes[i].width + tubes[i].x0; k++) {
				int curPix = (int)imgGrey2.col(k).row(j).at<uchar>(0, 0);
				if (curPix == informPix) {
					count++;
				}
			}
			log("count", count);
			if (count < (0.6*tubes[i].width)) {
				for (int h = tubes[i].x0; h < tubes[i].x0 + tubes[i].width; h++) {
					imgGrey2.col(h).row(j).at<uchar>(0, 0) = 255;
				}
			}
			else {
				for (int h = tubes[i].x0; h < tubes[i].x0 + tubes[i].width; h++) {
					imgGrey2.col(h).row(j).at<uchar>(0, 0) = 0;
				}
			}
		}
	}
		//imshow("BWFUNNN", imgGrey2);
		CleaningBG(imgGrey2);
		CompareThisFUN(imgGrey2, tubes);
	
}


// Поиск спектральных линий способ 1 (черно-белое)
void FindSpectreLines(Mat& imgGrey, Mat& src, const vector<SpectreArea>& tubes) {
	Mat imgGrey2 = imgGrey.clone();
	int tubeBackground;
	for (int i = 0; i < tubes.size(); i++) {
		int shiftY = (tubes[i].middleY - tubes[i].y0) / 8;
		for (int k = tubes[i].y0; k < tubes[i].middleY; k += shiftY) {
			tubeBackground = GetAverageDepthOfTubeBackground(imgGrey, tubes[i], k, (k + shiftY)) * 0.85;
			//log("tubeBackground", tubeBackground);
			for (int j = k; j < (k + shiftY); j++) {
				{
					for (int y = tubes[i].x0; y < (tubes[i].x0 + tubes[i].width); y++) {
						if (GetAverageDepth(imgGrey, y, j) < tubeBackground) {
							imgGrey2.col(y).row(j).at<uchar>(0, 0) = 0;
						}
						else
						{
							imgGrey2.col(y).row(j).at<uchar>(0, 0) = 255;
						}
					}
				}
			}
		}
	}
	imshow("BW", imgGrey2);
	BWFUNNY(imgGrey2,tubes);
	waitKey(0);
}

// Поиск спектральных линий способ 2
void FindSpectreLines2(Mat& imgGrey, Mat& src, const vector<SpectreArea>& tubes) {
	int currentBackgroundDepth = 0;
	bool isLine = false;
	int MAX_WIDTH = 6;
	int curWidth;
	
	for (int i = 0; i < tubes.size(); i++) {
		currentBackgroundDepth = GetAverageDepth(imgGrey, tubes[i].middleX, tubes[i].y0);
		//currentBackgroundDepth = imgGrey.col(tubes[i].middleX).row(tubes[i].y0).at<uchar>(0, 0);
		for (int j = (tubes[i].y0 + 1); j < tubes[i].middleY; j++) {
			int pixAverageDepth = GetAverageDepth(imgGrey, tubes[i].middleX, j); //imgGrey.col(tubes[i].middleX).row(j).at<uchar>(0, 0);
			if (!isLine) {
				if ((currentBackgroundDepth - pixAverageDepth) > 3) {
					isLine = true;
					curWidth = 0;
					Paint(src, tubes[i].middleX, j, COLORS.BLUE);
				}
				else {
					Paint(src, tubes[i].middleX, j, COLORS.RED);
				}
				
			}
			else {
				if ((currentBackgroundDepth - pixAverageDepth) < (-3)) {
					isLine = false;
					Paint(src, tubes[i].middleX, j, COLORS.RED);
				}
				else {
					if (curWidth > MAX_WIDTH) {
						isLine = false;
						Paint(src, tubes[i].middleX, j, COLORS.RED);
					}
					Paint(src, tubes[i].middleX, j, COLORS.BLUE);
					curWidth++;
				}
			}
			currentBackgroundDepth = pixAverageDepth;
		}
	}
}




// Функция поиска трубочек (основная)
void TubeDetect(Mat imgGrey, Mat& src, vector<SpectreArea>& tubes) {
	int x0 = 0;
	int y0 = 0;
	int middleY = (int)(imgGrey.rows / 2) - 20;

	int background_depth = GetBackgroundDepth(imgGrey);
	BACKGROUND_DEPTH_X = ((double)background_depth * 0.79);
	//log("BACKGROUND_DEPTH_X", BACKGROUND_DEPTH_X);
	BACKGROUND_DEPTH_Y = ((double)background_depth * 0.86);
	//log("BACKGROUND_DEPTH_Y", BACKGROUND_DEPTH_Y);

	AVERAGE_SPECTRE_WIDTH = src.cols*0.058;
	//log("AVERAGE_SPECTRE_WIDTH", AVERAGE_SPECTRE_WIDTH);
	vector<SpectreArea> spectres;
	int spectreID = 0;
	while (true) {
		vector<SpectreArea> tmpTube;
		int endOfTubeX;
		for (int i(middleY - 120); i <= (middleY + 60); i++) {
			int firstEnterX = FindEnterInTubeX(x0, i, imgGrey, src);
			if (firstEnterX == ERR_NOT_FOUND) continue;
			if (firstEnterX > (x0 + AVERAGE_SPECTRE_WIDTH)) continue;
			endOfTubeX = FindEndOfTubeX(firstEnterX, i, imgGrey, src);
			if ((endOfTubeX - firstEnterX) > (AVERAGE_SPECTRE_WIDTH)) continue;
			int midleOfTubeX = (firstEnterX + endOfTubeX) / 2;

			SpectreArea spectreArea;
			spectreArea.id = spectreID;
			spectreArea.x0 = firstEnterX;
			spectreArea.width = endOfTubeX - firstEnterX;
			spectreArea.middleX = midleOfTubeX;
			spectreArea.middleY = middleY;
			tmpTube.push_back(spectreArea);
		}

		if (!tmpTube.empty()) {
			spectres.push_back(FindMaxWidthOfSpectre(tmpTube));
			x0 = spectres[spectres.size() - 1].x0 + spectres[spectres.size() - 1].width;
			spectreID++;
		}
		else {
			break;
		}
	}

	FindHeightOfSpectre(spectres, middleY, imgGrey, src);
	

	//log("spectres.size()", (int)spectres.size());
	CorrectByWidth(spectres);
	//log("spectres.size()", (int)spectres.size());
	CorrectHeight(spectres);
	//log("spectres.size()", (int)spectres.size());

	FindSpectreLines(imgGrey, src, spectres);
	//FindSpectreLines2(imgGrey, src, spectres);



	for (int i(0); i < spectres.size(); i++) {
		PaintSpectreArea(src, spectres[i], COLORS.PURPLE);
	}


}

// Визуальное представление 
void VisualTesting(int startIndex, int endIndex = -1) {
	if (endIndex == -1) {
		endIndex = startIndex;
	}

	for (int i(startIndex); i <= endIndex; i++) {
		String imageName = "src_img_" + to_string(i) + ".png";
		Mat src = imread(imageName);
		imshow(imageName, src);
		waitKey(0);

		Mat imgGrey;
		cvtColor(src, imgGrey, CV_BGR2GRAY);

		vector<SpectreArea> tubes;

		TubeDetect(imgGrey, src, tubes);
		imshow(imageName, src);
		String resName = "res_img_" + to_string(i) + ".png";
		imwrite(resName, src);
		waitKey(0);
	}
}

int main() {

	VisualTesting(5, 5);

	system("pause");
	return 0;
}