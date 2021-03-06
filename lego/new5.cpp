#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include "util.h"
#include <boost/multi_array.hpp>
#include <cassert>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
using namespace cv;
using namespace boost;



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
  int gridSize = 10;
   for(int i = -gridSize; i < gridSize; i++){
    for(int j = -gridSize; j < gridSize; j++){
          viewArray[i+gridSize][j+gridSize] = 0;
         // cout << viewArray[i][j] << endl;
    }
  }
   for(int i = -gridSize; i < gridSize; i++){
      for(int j = -gridSize; j < gridSize; j++){
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

      cout << "2" << endl;
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
        cout << "2" << endl;

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


      cout << "2" << endl;
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
      cout << "2" << endl;

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

      cout << "2" << endl;

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
 

void initialiseTestArray(int viewArrayFront[20][20], int viewArrayLeft[20][20], int viewArrayRight[20][20], int viewArrayBack[20][20]){
    viewArrayFront[4][4] = 1;viewArrayFront[5][4] = 1;
    viewArrayFront[4][5] = 2;viewArrayFront[5][5] = 2;
    viewArrayFront[4][6] = 3;viewArrayFront[5][6] = 3;viewArrayFront[3][6] = 3;

    viewArrayLeft[2][3] = 1;viewArrayLeft[3][3] = 1;viewArrayLeft[4][3] = 1;viewArrayLeft[5][3] = 1;
    viewArrayLeft[4][4] = 2;viewArrayLeft[5][4] = 2;
    viewArrayLeft[4][5] = 3;viewArrayLeft[5][5] = 3;

    viewArrayBack[2][6] = 1;viewArrayBack[3][6] = 1;
    viewArrayBack[2][7] = 2;viewArrayBack[3][7] = 2;
    viewArrayBack[2][8] = 3;viewArrayBack[3][8] = 3;viewArrayBack[4][8] = 3;

    viewArrayRight[5][6] = 1;viewArrayRight[6][6] = 1;viewArrayRight[7][6] = 1;viewArrayRight[8][6] = 1;
    viewArrayRight[5][7] = 2;viewArrayRight[6][7] = 2;
    viewArrayRight[5][8] = 3;viewArrayRight[6][8] = 3;
}

void findBoundingBox(int &top, int &right, int &bottom, int &left, int array[20][20]){
      top = 20;left = 20; bottom=0; right=0;
       for(int i = 0; i < 20; i++){
        for(int j = 0; j < 20; j++){
          if (array[i][j] != 0){
              if(i < left) left = i;
              if(i > right) right = i;
              if(j < top) top = j;
              if(j > bottom) bottom = j;
          }
        }
    }
    right = 20 - right -1;
    bottom = 20 - bottom - 1;
    //cout << "top = " << top << ", left = "<< left << ", right = " << right << ", bottom = " << bottom << endl;

}


multi_array<colorArrayCell, 3> create3dArray(int viewArrayFront[20][20], int viewArrayLeft[20][20], int viewArrayRight[20][20], int viewArrayBack[20][20]){
    int gSize = 20;
    typedef boost::multi_array<colorArrayCell, 3> array3d;
    typedef array3d::index index;
    array3d A(boost::extents[gSize][gSize][gSize]);

    int frontTop, frontRight, frontBottom, frontLeft;
    int backTop, backRight, backBottom, backLeft;
    int leftTop, leftRight, leftBottom, leftLeft;
    int rightTop, rightRight, rightBottom, rightLeft;

    findBoundingBox(frontTop, frontRight, frontBottom, frontLeft, viewArrayFront);
    findBoundingBox(backTop, backRight, backBottom, backLeft, viewArrayBack);
    findBoundingBox(leftTop, leftRight, leftBottom, leftLeft, viewArrayLeft);
    findBoundingBox(rightTop, rightRight, rightBottom, rightLeft, viewArrayRight);


    int topFrontBackDifference = frontTop - backTop;
    int leftFrontBackDifference = frontLeft - backLeft;
    int topFrontRightDifference = frontTop - rightTop;

    int topRightLeftDifference = rightTop - leftTop;

    for(int i = 0; i < gSize; i++){
      for(int j = 0; j < gSize; j++){
        if (viewArrayFront[i][j] != 0){
          for(int k = 0; k < gSize; k++){
            if(viewArrayFront[i][j] == 1){
                A[i][j][k].redCount++;
            }else if(viewArrayFront[i][j] == 2){
                A[i][j][k].greenCount++;
            }else if(viewArrayFront[i][j] == 3){
                A[i][j][k].blueCount++;
            }
          }
        }
      }
    }


    for(int i = 0; i < gSize; i++){
      for(int j = 0; j < gSize; j++){
        if (viewArrayBack[i][j] != 0){
          for(int k = 0; k < gSize; k++){
            int newIinitial = gSize-i-1;
            int newI = newIinitial + frontLeft - backRight;
            int newJ = j+topFrontBackDifference;
            if(viewArrayBack[i][j] == 1){
                A[newI][newJ][k].redCount++;
            }else if(viewArrayBack[i][j] == 2){
                A[newI][newJ][k].greenCount++;
            }else if(viewArrayBack[i][j] == 3){
                A[newI][newJ][k].blueCount++;
            }
          } 
        }
      }
    }


   for(int k = 0; k < gSize; k++){
      for(int j = 0; j < gSize; j++){
        if (viewArrayRight[k][j] != 0){
          for(int i = 0; i < gSize; i++){
            int newJ = j+topFrontRightDifference;
            if(viewArrayRight[k][j] == 1){
                A[i][newJ][k].redCount++;
            }else if(viewArrayRight[k][j] == 2){
                A[i][newJ][k].greenCount++;
            }else if(viewArrayRight[k][j] == 3){
                A[i][newJ][k].blueCount++;
            }
          } 
        }
      }
    }

    
    for(int k = 0; k < gSize; k++){
      for(int j = 0; j < gSize; j++){
        if (viewArrayLeft[k][j] != 0){
          for(int i = 0; i < gSize; i++){
            int newKinitial = gSize-k-1;
            int newK = newKinitial + rightLeft - leftRight ;
            int newJ = j+topRightLeftDifference+topFrontRightDifference;
            if(viewArrayLeft[k][j] == 1){
                A[i][newJ][newK].redCount++;
            }else if(viewArrayLeft[k][j] == 2){
                A[i][newJ][newK].greenCount++;
            }else if(viewArrayLeft[k][j] == 3){
                A[i][newJ][newK].blueCount++;
            }
          } 
        }
      }
    }
  
  






      int count2 = 0;

      vector<finalPoint> fPoints; 
      finalPoint f;
      for(index i = 0 ; i < gSize; i ++){
      for(index j = 0; j < gSize; j++){
        for(index k = 0; k < gSize; k++){
          if (A[i][j][k].redCount >= 3){
             f.x = i;
             f.y = j;
             f.z = k;
             f.colour = 1;
             fPoints.push_back(f);
          }else if (A[i][j][k].greenCount >= 3){
             f.x = i;
             f.y = j;
             f.z = k;
             f.colour = 2;
             fPoints.push_back(f);
          }else if (A[i][j][k].blueCount >= 3){
             f.x = i;
             f.y = j;
             f.z = k;
             f.colour = 3;
             fPoints.push_back(f);
          
        //    A[i][j][k].print();
            //cout << i << ", " << j << ", " << endl << k;
         }
        }
      }
    }


    for(int i = 0; i < fPoints.size(); i++){
    cout <<fPoints[i].x << ", "<< fPoints[i].y << ", " << fPoints[i].z << " - " << fPoints[i].colour << endl;
  }
    return A;

      
}




int main() {
    string filename1 = "pictures/perfect2.jpg";
    string filename2 = "pictures/perfect2.jpg";
    string filename3 = "pictures/perfect2.jpg";
    string filename4 = "pictures/perfect2.jpg";
    Mat input = imread(filename1, CV_LOAD_IMAGE_GRAYSCALE);
    Mat harris1 = imread(filename1, CV_LOAD_IMAGE_GRAYSCALE);
    Mat harris2 = imread(filename2, CV_LOAD_IMAGE_GRAYSCALE);
    Mat harris3 = imread(filename3, CV_LOAD_IMAGE_GRAYSCALE);
    Mat harris4 = imread(filename4, CV_LOAD_IMAGE_GRAYSCALE);
    //GaussianBlur(input, 3, input);                     //CURRENTLY DOESNT MAKE DIFFERENCE
    Mat image1 = imread(filename1, CV_LOAD_IMAGE_COLOR);
    Mat image2= imread(filename2, CV_LOAD_IMAGE_COLOR);
    Mat image3 = imread(filename3, CV_LOAD_IMAGE_COLOR);
    Mat image4 = imread(filename4, CV_LOAD_IMAGE_COLOR);
    /*VideoCapture cap;
   
    cap.open(CV_CAP_ANY);
   
    if (!cap.isOpened()) {
      printf("Error: could not load a camera or video.\n");
    }
   
    Mat frameIn;
   
    namedWindow("video", 1);
   
    cap >> frameIn;*/




    removeBackground (image1); 
    removeBackground (image2); 
    removeBackground (image3); 
    removeBackground (image4); 
    Mat linesImage(input.size(), image1.type());
    if (!input.data) {
      cerr << "Cannot open " << filename1 << endl;
      exit(0);
    }
  //UNUSED HOUGH CODE
  /**/

    vector<Point> acceptablePoint;
   // HoughStuff(image, input, acceptablePoint);
      int viewArrayFront[20][20] = {{0}};   int viewArrayLeft[20][20] = {{0}};   int viewArrayRight[20][20] = {{0}};   int viewArrayBack[20][20] = {{0}};
      cout << "1" << endl;
 // initialiseTestArray(viewArrayFront, viewArrayLeft, viewArrayBack, viewArrayRight);
  harris1 = cornerHarris(viewArrayFront, harris1, image1, acceptablePoint);
        cout << "1" << endl;
  harris2 = cornerHarris(viewArrayBack, harris2, image2, acceptablePoint);
        cout << "1" << endl;
  harris3 = cornerHarris(viewArrayRight, harris3, image3, acceptablePoint);
        cout << "1" << endl;
  harris4 = cornerHarris(viewArrayLeft, harris4, image4, acceptablePoint);
      cout << "1" << endl;

  multi_array<colorArrayCell, 3> array3d =create3dArray(viewArrayFront, viewArrayLeft, viewArrayBack, viewArrayRight);
      cout << "1" << endl;
  // int viewArray[20][20];



  imshow("houghlines", image1);
    imshow("houghlines2", image2);
      imshow("houghlines3", image3);
        imshow("houghlines4", image4);
   waitKey(0);
  return 0;
  }
  
