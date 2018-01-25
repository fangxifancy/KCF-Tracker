#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"

//#include <dirent.h>

using namespace std;
using namespace cv;


Mat org,img,tmp;
int num_obj = 0;
char temp[50];
static Point pre_pt = Point(-1,-1);//original coordinate
static Point cur_pt = Point(-1,-1);//real-time coordinate

vector<Rect> Track_rec(10);//create container

void on_mouse(int event,int x,int y,int flags,void *ustc)//event:code for mouse event，x,y:coordinate of mouse，flags:code for drag
{
	//static Point tmp_min = pre_pt;
	//static Point tmp_max = cur_pt;

    if (event == CV_EVENT_LBUTTONDOWN)//press on left,read original coordinate,circle on point
    {
        org.copyTo(img);
        sprintf(temp,"(%d,%d)",x,y);
        pre_pt = Point(x,y);
        putText(img,temp,pre_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255),1,8);//show coordinate on window
        circle(img,pre_pt,2,Scalar(255,0,0,0),CV_FILLED,CV_AA,0);
        imshow("img",img);
    }
    else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//no press on left
    {
        img.copyTo(tmp);
        sprintf(temp,"(%d,%d)",x,y);
        cur_pt = Point(x,y);
        putText(tmp,temp,cur_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));//show mouse coordinate real-time(no press)
        imshow("img",tmp);
    }
    else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//press on left,draw rectangle
    {
        img.copyTo(tmp);
        sprintf(temp,"(%d,%d)",x,y);
        cur_pt = Point(x,y);
        putText(tmp,temp,cur_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));
        rectangle(tmp,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);//show rectangle on tmp image real-time
        imshow("img",tmp);
    }
    else if (event == CV_EVENT_LBUTTONUP)//release left,draw rectangle
    {
        org.copyTo(img);
        sprintf(temp,"(%d,%d)",x,y);
        cur_pt = Point(x,y);
        putText(img,temp,cur_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));
        circle(img,pre_pt,2,Scalar(255,0,0,0),CV_FILLED,CV_AA,0);
        rectangle(img,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);//draw rectangle
        imshow("img",img);

	Point tmp_min = pre_pt;
	Point tmp_max = cur_pt;
        Track_rec[num_obj] = Rect(pre_pt,cur_pt);
	num_obj++;
	cout<<"Objects number is: "<<num_obj<<endl;
        waitKey(500);
    }
}


int main(int argc, char* argv[]){

	if (argc > 5) return -1;

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = true;
	bool LAB = false;

	for(int i = 0; i < argc; i++){
		if ( strcmp (argv[i], "hog") == 0 )
			HOG = true;
		if ( strcmp (argv[i], "fixed_window") == 0 )
			FIXEDWINDOW = true;
		if ( strcmp (argv[i], "singlescale") == 0 )
			MULTISCALE = false;
		if ( strcmp (argv[i], "show") == 0 )
			SILENT = false;
		if ( strcmp (argv[i], "lab") == 0 ){
			LAB = true;
			HOG = true;
		}
		if ( strcmp (argv[i], "gray") == 0 )
			HOG = false;
	}

	// Create KCFTracker object
	//KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
	vector<KCFTracker> tracker(10);

	// Tracker results
	//Rect result;
	vector<Rect> result(10);

	// label the first image
	org = imread("0001.jpg", CV_LOAD_IMAGE_COLOR);
   	org.copyTo(img);
        org.copyTo(tmp);
        imshow("img",img);
        setMouseCallback("img",on_mouse,0);
        waitKey(0);

	for (int i = 0;i < num_obj;i++){
		tracker[i]= KCFTracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
		tracker[i].init( Track_rec[i], org );
		rectangle( org, Track_rec[i].tl(), Track_rec[i].br(), Scalar( 0, 0, 255 ), 4, 8 );
		cur_pt = (Track_rec[i].tl() + Track_rec[i].br())/2 ;
		sprintf(temp,"(%d,%d)",cur_pt.x,cur_pt.y);
		putText(org,temp,cur_pt,FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,0,255), 2, 8);
	}

	//video record
	double rate = 25.0; //ftp
	Size videoSize(org.cols,org.rows);
	VideoWriter writer("VideoTest.avi", CV_FOURCC('M', 'J', 'P', 'G'), rate, videoSize);
	writer<<org;

	// Path to list.txt
	// ifstream listFile;
	// string fileName = "images.txt";
  	// listFile.open(fileName);

  	// Read groundtruth for the 1st frame
  	// ifstream groundtruthFile;
	// string groundtruth = "region.txt";
  	// groundtruthFile.open(groundtruth);
  	// string firstLine;
  	// getline(groundtruthFile, firstLine);
	// groundtruthFile.close();

  	// istringstream ss(firstLine);

  	// Read groundtruth like a dumb
  	// float x1, y1, x2, y2, x3, y3, x4, y4;
  	// char ch;
	// ss >> x1;
	// ss >> ch;
	// ss >> y1;
	// ss >> ch;
	// ss >> x2;
	// ss >> ch;
	// ss >> y2;
	// ss >> ch;
	// ss >> x3;
	// ss >> ch;
	// ss >> y3;
	// ss >> ch;
	// ss >> x4;
	// ss >> ch;
	// ss >> y4;

	// Using min and max of X and Y for groundtruth rectangle
	// float xMin =  min(x1, min(x2, min(x3, x4)));
	// float yMin =  min(y1, min(y2, min(y3, y4)));
	// float width = max(x1, max(x2, max(x3, x4))) - xMin;
	// float height = max(y1, max(y2, max(y3, y4))) - yMin;


	// Read Images
	// ifstream listFramesFile;
	// string listFrames = "images.txt";
	// listFramesFile.open(listFrames);
	// string frameName;


	// Write Results
	// ofstream resultsFile;
	// string resultsPath = "output.txt";
	// resultsFile.open(resultsPath);

	// Frame readed
    //int N = 99;
	Mat frame;
	string frameName;
	char imgName[100];
	string path = "./";

	for (int n = 1; n < 158; n++){
		sprintf(imgName, "%d.jpg", n);
		frameName = path + imgName;
		frame = imread(frameName, CV_LOAD_IMAGE_COLOR);// Read each frame

		// Update tracker
		for (int i =0;i< num_obj;i++){
			result[i] = tracker[i].update(frame);
			rectangle( frame, result[i].tl(),result[i].br(), Scalar( 0, 0, 255 ), 4, 8 );
			cur_pt = (result[i].tl() + result[i].br())/2 ;
            //srand(time(NULL));
            double k = 210+rand()%11/10.0*5.0;
            sprintf(temp,"(%d,%d)depth:%.2f",cur_pt.x,cur_pt.y,k);
			putText(frame,temp,cur_pt,FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,0,255), 2, 8);
		}

		if (!SILENT){
			writer<<frame;
			imshow("image", frame);
			waitKey(1);
		}
	}



/*
	// Frame counter
	int nFrames = 0;


	while ( getline(listFramesFile, frameName) ){
		frameName = frameName;

		// Read each frame from the list
		frame = imread(frameName, CV_LOAD_IMAGE_COLOR);

		// First frame, give the groundtruth to the tracker
		if (nFrames == 0) {
			tracker.init( Rect(xMin, yMin, width, height), frame );
			rectangle( frame, Point( xMin, yMin ), Point( xMin+width, yMin+height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << xMin << "," << yMin << "," << width << "," << height << endl;
		}
		// Update
		else{
			result = tracker.update(frame);
			rectangle( frame, Point( result.x, result.y ), Point( result.x+result.width, result.y+result.height), Scalar( 0, 255, 255 ), 1, 8 );
			resultsFile << result.x << "," << result.y << "," << result.width << "," << result.height << endl;
		}

		nFrames++;

		if (!SILENT){
			imshow("Image", frame);
			waitKey(1);
		}
	}
	resultsFile.close();
	listFile.close();
*/
}
