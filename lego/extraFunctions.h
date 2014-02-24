#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <opencv.hpp>
#include <highgui/highgui.hpp>
#include <math.h>
#include "util.h"
using namespace std;
using namespace cv;


void drawGrid2(harrisLine finalHLines, double total, double width, double yScale, Mat image);
void GaussianBlur(cv::Mat &input, int size, cv::Mat &blurredOutput);
Mat flatten (Mat hs);
void threshold (Mat input, int val);
Mat getGradient (Mat a, Mat b);
Mat getMagnitude (Mat a, Mat b);
Mat normalize (Mat input);
Mat conv (Mat input, Mat kernel);
vector<harrisLine> clusterLines(vector<harrisLine> lines);

 void HoughStuff(Mat image, Mat input, vector<Point> acceptablePoint);

#endif // UTIL_H