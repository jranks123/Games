#include <iostream>
#include <opencv.hpp>
#include <highgui/highgui.hpp>
#include <math.h>
#include "util.h"
using namespace std;
using namespace cv;



struct harrisLine
{
	Point p1, p2;
	double distance, brightness;
};

struct colorArrayCell
{
  /* data */
};


class Colour {
public:
  int r, g, b;


  Colour (int r, int g, int b) : r(r), g(g), b(b) {}

  void print () {
    cout << "#(" << r << ", " << g << ", " << b << ")" << endl;
  }
};



 void HoughStuff(Mat image, Mat input, vector<Point> acceptablePoint){
     Mat sobelVertical = (Mat_<double>(3, 3) << 
                -1, 0, 1,
                -2, 0, 2,
                -1, 0, 1);

      Mat sobelHorizontal = (Mat_<double>(3, 3) << 
                -1, -2, -1,
                0, 0, 0,
                1, 2, 1);

      Mat dbydx = conv(input, sobelVertical);

      Mat dbydy = conv(input, sobelHorizontal);

      Mat magnitude = getMagnitude(dbydx, dbydy);
      magnitude = normalize(magnitude);
      threshold(magnitude, 15);
        Mat gradient = getGradient(dbydx, dbydy);
      //  imshow("thresh", magnitude);
        vector<Vec4i> lines;
        vector<Vec4i> lines2;
        vector<double> vertLines;
        vector<double> vertLinesClone;
        
      HoughLinesP(magnitude, lines, 1, CV_PI/180, 20, 50, 30 );
      
      
      //HoughLines(magnitude, lines, 1, CV_PI/180, 100, 10, 10 );
        for( size_t i = 0; i < lines.size(); i++ )
      {
          Vec4i l = lines[i];
        
          double x1 = l[0];double x2 = l[2]; double y1 = l[1]; double y2 = l[3];
          double angle =  atan2(y2 - y1, x2 - x1) * 180 / CV_PI;
          //double n1 = sqrt(x1*x1+y1*y1), n2 = sqrt(x2*x2+y2*y2);
        //double angle = acos((x1*x2+y1*y2)/(n1*n2)) * 180 / CV_PI;
          angle = abs(angle);
        //  cout << angle << endl;
          if(angle > 85 && angle < 95 || angle < 5){
          double distance = sqrt((x1-y1)*(x1-y1)+(x2-y2)*(x2-y2));
          // cout << distance << endl;
           if(distance < 100){
              line( image, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,0), 1, CV_AA);
           }
            vertLines.push_back(distance);
            Point p1;
            p1.x =x1;
            p1.y = y1;
            Point p2;
            p2.x = x2;
            p2.y = y2;
            acceptablePoint.push_back(p1);
          acceptablePoint.push_back(p2);
            
          } 
      }
      vertLinesClone = vertLines;
  }


  

//IDEA - DO CLUSTERING
vector<harrisLine> clusterLines(vector<harrisLine> lines){
  bool converged = true;
  do{
    int thresh = 20;
    converged = true;
    //cout << "here" << endl;
   // vector<harrisLine> newHarris;
    for(size_t i = 0; i < lines.size(); i++){
    bool merged = false;
      for(size_t k = i+1; k < lines.size(); k++){
        double p1x = abs(lines[i].p1.x - lines[k].p1.x);
        double p1y = abs(lines[i].p1.y - lines[k].p1.y);
        double p2x = abs(lines[i].p2.x - lines[k].p2.x);
        double p2y = abs(lines[i].p2.y - lines[k].p2.y);

        if((p1x < thresh) && (p1y < thresh) && (p2x< thresh) &&(p2y< thresh) && (p1x > 0) && (p1y > 0) && (p2x > 0) &&(p2y > 0)){
          lines[i].p1.x = (lines[i].p1.x + lines[k].p1.x)/2;
          lines[i].p1.y = (lines[i].p1.y + lines[k].p1.y)/2;
          lines[i].p2.x = (lines[i].p2.x + lines[k].p2.x)/2;
          lines[i].p2.y = (lines[i].p2.y + lines[k].p2.y)/2;
          lines[i].distance = (lines[i].distance+lines[k].distance)/2;
          lines[k].p1.x = (lines[i].p1.x + lines[k].p1.x)/2;
          lines[k].p1.y = (lines[i].p1.y + lines[k].p1.y)/2;
          lines[k].p2.x = (lines[i].p2.x + lines[k].p2.x)/2;
          lines[k].p2.y = (lines[i].p2.y + lines[k].p2.y)/2;
          lines[k].distance = (lines[i].distance+lines[k].distance)/2;
          converged = false;
          //cout << "made it" << endl;
        }
      }
    }
  }while(converged == false);
  return lines;
}

Mat conv (Mat input, Mat kernel) {
  Mat output = Mat(input.rows, input.cols, CV_32FC1);
  int kernelRadiusX = ( kernel.size[0] - 1 ) / 2;
  int kernelRadiusY = ( kernel.size[1] - 1 ) / 2;

  Mat paddedInput;
  copyMakeBorder(input, paddedInput, 
  kernelRadiusX, kernelRadiusX, kernelRadiusY, kernelRadiusY,
  BORDER_REPLICATE );

  for ( int i = kernelRadiusX; i < input.rows - kernelRadiusX; i++ ) {
    for( int j = kernelRadiusY; j < input.cols - kernelRadiusY; j++ ) {
      float sum = 0.0;
      for( int m = -kernelRadiusX; m <= kernelRadiusX; m++ ) {
        for( int n = -kernelRadiusY; n <= kernelRadiusY; n++ ) {
          int imagex = i + 1 + m;
          int imagey = j + 1 + n;
          int kernelx = m + kernelRadiusX;
          int kernely = n + kernelRadiusY;
          int imageval = ( int ) paddedInput.at<uchar>( imagex, imagey );
          double kernalval = kernel.at<double>( kernelx, kernely );
          sum += imageval * kernalval;
        }
      }
      output.at<float>(i, j) = (float) sum;
    }
  }

  return output;
}

Mat normalize (Mat input) {
  float min = input.at<float>(0, 0);
  float max = input.at<float>(0, 0);

  for (int r = 0; r < input.rows; r++) {
    float* rowPtr = input.ptr<float>(r);

    for (int c = 0; c < input.cols; c++) {
      if (rowPtr[c] > max) {
	max = rowPtr[c];
      }
      else if (rowPtr[c] < min) {
	min = rowPtr[c];
      }
    }
  }

  Mat output = Mat(input.rows, input.cols, CV_8UC1);
  
  for (int r = 0; r < input.rows; r++) {
    float* inputRowPtr = input.ptr<float>(r);
    uchar* outputRowPtr = output.ptr<uchar>(r);
    
    for (int c = 0; c < input.cols; c++) {
      outputRowPtr[c] = (uchar)(((inputRowPtr[c] - min) / (max - min)) * 255);
    }
  }

  return output;
}

Mat getMagnitude (Mat a, Mat b) {
  Mat output = Mat(a.rows, a.cols, CV_32FC1);
  
  for (int r = 0; r < a.rows; r++) {
    float* aRowPtr = a.ptr<float>(r);
    float* bRowPtr = b.ptr<float>(r);
    float* outputRowPtr = output.ptr<float>(r);

    
    for (int c = 0; c < a.cols; c++) {
      float p1 = aRowPtr[c];
      float p2 = bRowPtr[c];

      outputRowPtr[c] = sqrt(p1 * p1 + p2 * p2);
    }
  }

  return output;
}

Mat getGradient (Mat a, Mat b) {
  Mat output = Mat(a.rows, a.cols, CV_32FC1);
  
  for (int r = 0; r < a.rows; r++) {
    float* aRowPtr = a.ptr<float>(r);
    float* bRowPtr = b.ptr<float>(r);
    float* outputRowPtr = output.ptr<float>(r);

    
    for (int c = 0; c < a.cols; c++) {
      float p1 = aRowPtr[c];
      float p2 = bRowPtr[c];

      if (p1 != 0.0) {
	outputRowPtr[c] = atan(p2 / p1);
      }
      else {
	outputRowPtr[c] = M_PI / 2.0;
      }
    }
  }

  return output;
}

void threshold (Mat input, int val) {
  for (int r = 0; r < input.rows; r++) {
    uchar* rowPtr = input.ptr<uchar>(r);
    
    for (int c = 0; c < input.cols; c++) {
      rowPtr[c] = (rowPtr[c] < val ? 0 : 255);
    }
  }
}

Mat flatten (Mat hs) {
  int xSize = hs.size.p[0];
  int ySize = hs.size.p[1];
  int rSize = hs.size.p[2];
  
  Mat output = Mat::zeros(xSize, ySize, CV_32FC1);

  for (int x = 0; x < xSize; x++) {
    float* outputRowPtr = output.ptr<float>(x);
    
    for (int y = 0; y < ySize; y++) {
      for (int r = 0; r < rSize; r++) {
  	outputRowPtr[y] += hs.at<float>(x, y, r);
      }
    }
  }

  return output;
}

  
  void GaussianBlur(cv::Mat &input, int size, cv::Mat &blurredOutput)
{
    // intialise the output using the input
    blurredOutput.create(input.size(), input.type());

    // create the Gaussian kernel in 1D
    cv::Mat kX = cv::getGaussianKernel(size, -1);
    cv::Mat kY = cv::getGaussianKernel(size, -1);

    // make it 2D multiply one by the transpose of the other
    cv::Mat kernel = kX * kY.t();
    int kernelRadiusX = ( kernel.size[0] - 1 ) / 2;
    int kernelRadiusY = ( kernel.size[1] - 1 ) / 2;

    cv::Mat paddedInput;
    cv::copyMakeBorder( input, paddedInput,
                        kernelRadiusX, kernelRadiusX, kernelRadiusY, kernelRadiusY,
                        cv::BORDER_REPLICATE );

    // now we can do the convoltion
    for ( int i = 0; i < input.rows; i++ )
    {
        for ( int j = 0; j < input.cols; j++ )
        {
            double sum = 0.0;
            for ( int m = -kernelRadiusX; m <= kernelRadiusX; m++ )
            {
                for ( int n = -kernelRadiusY; n <= kernelRadiusY; n++ )
                {
                    // find the correct indices we are using
                    int imagex = i + 1 + m;
                    int imagey = j + 1 + n;
                    int kernelx = m + kernelRadiusX;
                    int kernely = n + kernelRadiusY;

                    // get the values from the padded image and the kernel
                    int imageval = ( int ) paddedInput.at<uchar>( imagex, imagey );
                    double kernalval = kernel.at<double>( kernelx, kernely );

                    // do the multiplication
                    sum += imageval * kernalval;
                }
            }
            // set the output value as the sum of the convolution
            blurredOutput.at<uchar>(i, j) = (uchar) sum;
        }
    }
}

void drawGrid2(harrisLine finalHLines, double total, double width, double yScale, Mat image){
   for(int i = -10; i < 10; i++){
      double red = 0.0, blue = 0.0, green = 0.0, x = finalHLines.p1.x + width*i, y = finalHLines.p1.y+yScale*total;

      if(x> 0 && y > 0 && x+width < image.cols && y+total < image.rows){
        Mat section  = image(Rect(x,y, width, total));
        for(size_t l = 0; l < section.cols; l++){
            for(size_t m = 0; m < section.rows; m++){
              red += section.at<Vec3b>(l,m)[0];
              blue += section.at<Vec3b>(l,m)[1];
              green += section.at<Vec3b>(l,m)[2];
            }
        }
        red = red/(section.cols*section.rows);
        blue = blue/(section.cols*section.rows);
        green = green/(section.cols*section.rows);
        Vec3b avColour(red,blue,green);
        int t = 50;
        //if((red < t || blue < t || green < t)){
             cout << avColour << endl;
          for(size_t f = y; f < y+total; f++){
              for(size_t g = x; g < x+width; g++){
                image.at<Vec3b>(f,g) = avColour;
              }
          }
       // }

      //CODE TO DRAW ON GRID IF REQUIRED
      /*
      Point p(finalHLines.p1.x + width*i, finalHLines.p1.y+yScale*total);
      Point q(finalHLines.p2.x + width*i, finalHLines.p2.y+yScale*total);
      Point r(finalHLines.p1.x + width+width*i, finalHLines.p1.y+yScale*total);
      Point s(finalHLines.p2.x + width+width*i, finalHLines.p2.y+yScale*total);
      line( image, s, r, Scalar(0,0,0), 3, CV_AA); 
      line( image, p, q, Scalar(0,0,0), 3, CV_AA);
      line( image, p, r, Scalar(0,0,0), 3, CV_AA);
      line( image, q, s, Scalar(0,0,0), 3, CV_AA);*/
    }
 }

}