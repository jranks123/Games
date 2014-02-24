#include <iostream>
#include <opencv.hpp>
#include <highgui/highgui.hpp>
#include <math.h>
#include "util.h"
using namespace std;
using namespace cv;


void removeBackground (Mat image) {
    Mat mask;
    cvtColor(image, mask, CV_RGB2HSV);

    imshow("hsv", mask);

    
    for (int r = 0; r < image.rows; r++) {
        uchar* rowPtr = image.ptr<uchar>(r);
        uchar* maskRowPtr = mask.ptr<uchar>(r);

        for (int c = 0; c < image.cols*3; c += 3) {
            int h = maskRowPtr[c];
            int s = maskRowPtr[c + 1];
            int v = maskRowPtr[c + 2];

            if (v > 90 && s < 230) {
                rowPtr[c] = 0;
                rowPtr[c+1] = 0;
                rowPtr[c+2] = 0;
            }
        }
    }
}

int getHue (Colour colour) {
  float r = colour.r/255.0f;
  float g = colour.g/255.0f;
  float b = colour.b/255.0f;

  float cMax = max(r, max(b, g));
  float cMin = min(r, min(b, g));
  float delta = cMax - cMin;
  
  if (cMax == r) {
    return (int)((g - b)/delta) % 6;
  }
  else if (cMax == g) {
    return (int)((b - r)/delta) + 2;
  }
  else if (cMax == b) {
    return (int)((r - g)/delta) + 4;
  }
}

Colour getClosestColour (Colour colour, vector<Colour> baseColours) {
  int hue = getHue(colour);

  int minI;
  int minDelta = 10000000;
  
  for (int i = 0; i < baseColours.size(); i++) {
    int baseHue = getHue(baseColours[i]);

    int delta = abs(hue - baseHue);

    if (delta < minDelta) {
      minDelta = delta;
      minI = i;
    }
  }

  return baseColours[minI];
}

Colour getColour (Vec3b pixelColour) {
  vector<Colour> baseColours;
  baseColours.push_back(Colour(255, 0, 0));
  baseColours.push_back(Colour(0, 255, 0));
  baseColours.push_back(Colour(0, 0, 255));
  baseColours.push_back(Colour(255, 255, 0));





  Colour darkRed(pixelColour[0], pixelColour[1], pixelColour[2]);
  Colour colour = getClosestColour(darkRed, baseColours);

  return colour;
  
}



//IDEA : DRAW FROM BRIGHTEST HOUGH POINT





//Determines whether a harris-detected corner is acceptable by checking whether it is close enough
//to an end of a hough line
bool pointIsAcceptable(double x, double y, vector<Point> pointVec){
	bool accept = false;
	for(size_t i = 0; i < pointVec.size(); i++){
		if (abs(x - pointVec[i].x) < 10 || abs(y - pointVec[i].y) < 10) accept = true;
	}
	return accept;
}


void drawGrid3(int viewArray[20][20], harrisLine finalHLines, double total, double width, Mat image){
   for(int i = -10; i < 10; i++){
    for(int j = -10; j < 10; j++){
          viewArray[i+10][j+10] = 0;
         // cout << viewArray[i][j] << endl;
    }
  }
   for(int i = -10; i < 10; i++){
      for(int j = -10; j < 0; j++){
        double red = 0.0, blue = 0.0, green = 0.0, x = finalHLines.p1.x + width*i, y = finalHLines.p1.y+j*total;
        if(x> 0 && y > 0 && x+width < image.cols && y+total < image.rows){
          Mat section  = image(Rect(x,y, width, total));
          bool black = false;
          for(size_t l = 0; l < section.cols; l++){
              for(size_t m = 0; m < section.rows; m++){
                    black = false;
                    Vec3b pixel = section.at<Vec3b>(l,m);
                    Colour c = getColour(pixel);
                    if(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0){
                       viewArray[i+10][j+10] = 0;
                    }else
                    if(c.r == 255){
                        red++;
                    }else if(c.g == 255){
                      green++;
                    }else if(c.b == 255){
                      blue++;
                    }
                    
              }
          }
          //cout << red << endl;
          Vec3b avColour(0,0,0);
          if(!black){
            if(red > (section.cols*section.rows)/3){
                avColour[0] = 255;
                viewArray[i+10][j+10] = 1;
            }else if(green > (section.cols*section.rows)/3){
                avColour[1] = 255;
                viewArray[i+10][j+10] = 2;
            }else if(blue > (section.cols*section.rows)/3){
                avColour[2] = 255;
                viewArray[i+10][j+10] = 3;
            }


              for(size_t f = y; f < y+total; f++){
                  for(size_t g = x; g < x+width; g++){
                      image.at<Vec3b>(f,g) = avColour;
                  }
              }
            }
         // }

        //CODE TO DRAW ON GRID IF REQUIRED
        Point p(finalHLines.p1.x + width*i, finalHLines.p1.y+j*total);
        Point q(finalHLines.p2.x + width*i, finalHLines.p2.y+j*total);
        Point r(finalHLines.p1.x + width+width*i, finalHLines.p1.y+j*total);
        Point s(finalHLines.p2.x + width+width*i, finalHLines.p2.y+j*total);
        line( image, s, r, Scalar(0,0,0), 3, CV_AA); 
        line( image, p, q, Scalar(0,0,0), 3, CV_AA);
        line( image, p, r, Scalar(0,0,0), 3, CV_AA);
        line( image, q, s, Scalar(0,0,0), 3, CV_AA);
      }
   }
}


}





double cornerHarris(int viewArray[20][20], Mat src, Mat image, vector<Point> acceptablePoint)
{
	
	int thresh = 10;
	Mat dst, dst_norm, dst_norm_scaled;
	dst = Mat::zeros( src.size(), CV_32FC1 );

	/// Detector parameters
	int blockSize = 3;
	int apertureSize = 31;
	double k = 0.001;

	/*int thresh = 100;// 100 or 150	
	int blockSize = 7;
	int apertureSize = 31;
	double k = 0.03;*/


	/// Detecting corners
	cornerHarris( src, dst, blockSize, apertureSize, k, BORDER_DEFAULT );

	/// Normalizing and initialising
	normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
	convertScaleAbs( dst_norm, dst_norm_scaled );
  vector<Point> harrisPoints;
  vector<double> distances;
  vector<double>initialDistances;
  vector<harrisLine> hLines;
  Point topLeft(1000, 1000);
  harrisLine bright;
  double brightest = 0;


  //calculate initial harris corners  
  int count = 0;
  for( int j = 0; j < dst_norm.rows ; j++ ){
    for( int i = 0; i < dst_norm.cols; i++ ){  
      int p = dst_norm.at<float>(j,i);
    	if( p > thresh ){
          if(p > brightest){
            bright.p1.x = i;
            bright.p1.y = j;
            bright.brightness = thresh;
          }
    	  harrisPoints.push_back(Point(i,j));
    	    count ++;
    	   circle( dst_norm_scaled, Point( i, j ), 5,  Scalar(0), 2, 8, 0 );
    	  }
    }
  }



   //Make pairs of corners that are vertically alligned and are not too close or too
   //far apart to be considered edges
	 for(size_t i = 0; i < harrisPoints.size(); i++){
	 	for(size_t k = i+1; k < harrisPoints.size(); k++){
	 		double x1 = harrisPoints[i].x, x2 = harrisPoints[k].x, y1 = harrisPoints[i].y, y2 = harrisPoints[k].y;
	 		if(abs(x1 - x2) <2 && abs(y1 -y2) > 50){
	 			if(true){//pointIsAcceptable(x1,y1,acceptablePoint)&& pointIsAcceptable(x2,y2, acceptablePoint)){    //Ended up not making a big enough difference
		 		    double distance = sqrt(((x1-x2)*(x1-x2))+((y1-y2)*(y1-y2)));
  		 			initialDistances.push_back(distance);
  		 			harrisLine h;
  		 			h.p1 = Point(x1, y1);
  		 			h.p2 = Point(x2,y2);
  		 			h.distance = distance;
  		 			hLines.push_back(h);
		 		}
	 		}
	 	}
	 }


	//Find the median distance and then rule out any lines that are not close enough
  //to this distance
  sort(initialDistances.begin(), initialDistances.end());
	vector<harrisLine> finalHLines;
	 double median = initialDistances[initialDistances.size()/2];
	 double range = median/5;
	 for(size_t i = 0; i <  hLines.size(); i++){
	 	if(abs(hLines[i].distance - median) < range){
	 		harrisLine h;
		 	h.p1 = Point(hLines[i].p1.x, hLines[i].p1.y);
		 	h.p2 = Point(hLines[i].p2.x, hLines[i].p2.y);
		 	h.distance = hLines[i].distance;
	 	 finalHLines.push_back(h);
	 	}
	 }

  // finalHLines = clusterLines(finalHLines);   //DECIDED THAT THIS TOOK TOO LONG FOR NOT ENOUGH IMPROVEMENT



   //calculate the average edge length
	 double total = 0; 
	 for(size_t i = 0; i < finalHLines.size(); i++){
        line( image, finalHLines[i].p1, finalHLines[i].p2, Scalar(0,0,255), 3, CV_AA);    //Draw on edge lines
        circle( image, finalHLines[i].p1, 5,  Scalar(0), 2, 8, 0 );
       circle( image, finalHLines[i].p2, 5,  Scalar(0), 2, 8, 0 );
        //cout << " Line distance = " << finalHLines[i].distance << endl;
	 	     total += finalHLines[i].distance;
	 }
	 total = total/finalHLines.size();
   bright.p2.x = bright.p1.x;
   bright.p2.y = bright.p1.y + total;
	 cout << "TOTAL = " << total << endl;
   double width = (total/6)*5;
      drawGrid3(viewArray, bright, total, width, image);
	/// Showing the harris corners
	imshow( "harris ", dst_norm_scaled );
	return total;
}
 
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


int main () {
	string filename = "pictures/perfect1.jpg";
  	Mat input = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
  	Mat harris = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
  	//GaussianBlur(input, 3, input);                     //CURRENTLY DOESNT MAKE DIFFERENCE
    Mat image = imread(filename, CV_LOAD_IMAGE_COLOR);
    //removeBackground (image); 
    Mat linesImage(input.size(), image.type());
  	if (!input.data) {
    	cerr << "Cannot open " << filename << endl;
    	exit(0);
  	}
	//UNUSED HOUGH CODE
	/**/
    vector<Point> acceptablePoint;
    HoughStuff(image, input, acceptablePoint);

	 int viewArray[20][20];
   harris = cornerHarris(viewArray, harris, image, acceptablePoint);


	imshow("houghlines", image);
   waitKey(0);
  return 0;
  }
  
