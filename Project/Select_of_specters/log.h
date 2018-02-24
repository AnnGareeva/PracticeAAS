#pragma once
#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>

using namespace cv;
using namespace std;

void log(const string& tag, const string& message) {
#ifdef LOG_ENABLE
	cout << tag << ": " << message << endl;
#endif
}

void log(const string& tag, const int value) {
#ifdef LOG_ENABLE
	cout << tag << ": " << value << endl;
#endif
}

void log(const string& tag, const double value) {
#ifdef LOG_ENABLE
	cout << tag << ": " << value << endl;
#endif
}


void log(const string& message) {
#ifdef LOG_ENABLE
	cout << message << endl;
#endif
}

void log(const string& tag, const Mat& value) {
#ifdef LOG_ENABLE
	cout << tag << ": " << value << endl;
#endif
}