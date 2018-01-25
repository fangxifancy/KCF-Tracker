#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"
//#include <dirent.h>

using namespace std;
using namespace cv;


Mat org,img;
int width[5], height[5];
int num_obj=0;
char temp[50];
static Point pre_pt = Point(-1,-1);//original coordinate
static Point cur_pt = Point(-1,-1);//real-time coordinate
vector<Rect> Truth_rec(5);//create container

void on_mouse(int event,int x,int y,int flags,void *ustc)//event:code for mouse event，x,y:coordinate of mouse，flags:code for drag
{

	if (event == CV_EVENT_LBUTTONDOWN)//press on left,read original coordinate,circle on point
    {
        org.copyTo(img);
        sprintf(temp,"(%d,%d)",x,y);
        pre_pt = Point(x,y);

		if(num_obj==0) cur_pt = Point(x+width[0],y+height[0]);
		else if(num_obj==1) cur_pt = Point(x+width[1],y+height[1]);	

		putText(img,temp,pre_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255),1,8);//show coordinate on window
		circle(img,pre_pt,1,Scalar(255,0,0,0),CV_FILLED,CV_AA,0);
		rectangle(img,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);        
		imshow("img",img);
    }
    else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//no press on left
    {
        org.copyTo(img);
        sprintf(temp,"(%d,%d)",x,y);
        pre_pt = Point(x,y);

		if(num_obj==0) cur_pt = Point(x+width[0],y+height[0]);
		else if(num_obj==1) cur_pt = Point(x+width[1],y+height[1]);	

        circle(img,pre_pt,1,Scalar(255,0,0,0),CV_FILLED,CV_AA,0);
		putText(img,temp,pre_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(255,0,0,0));//show mouse coordinate real-time(no press)
		rectangle(img,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);//show first fixed rectangle on img
        imshow("img",img);
    }

/*	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//press on left and drag, draw rectangle
    {
        org.copyTo(img);
        sprintf(temp,"(%d,%d)",x,y);
        cur_pt = Point(x,y);
        putText(img,temp,cur_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));
        rectangle(img,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);//show rectangle on img real-time
        imshow("img",img);
    }
*/
    else if (event == CV_EVENT_LBUTTONUP)//release left,record rectangle
    {
        org.copyTo(img);
        sprintf(temp,"(%d,%d)",x,y);
        pre_pt = Point(x,y);

		if(num_obj==0) cur_pt = Point(x+width[0],y+height[0]);
		else if(num_obj==1) cur_pt = Point(x+width[1],y+height[1]);	

		putText(img,temp,pre_pt,FONT_HERSHEY_SIMPLEX,0.5,Scalar(0,0,0,255));
        circle(img,pre_pt,1,Scalar(255,0,0,0),CV_FILLED,CV_AA,0);
        rectangle(img,pre_pt,cur_pt,Scalar(0,255,0,0),1,8,0);//record rectangle on img
        imshow("img",img);

		Truth_rec[num_obj] = Rect(pre_pt,cur_pt);
		num_obj++;
		cout<<"Objects number is: "<<num_obj<<endl;
		if(num_obj == 2) num_obj = 0;
        waitKey(500);
    }
}


int main(int argc, char* argv[]){

//	if (argc > 5) return -1;

/*	bool HOG = true;
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
*/
	width[0] = atoi(argv[1]);
	height[0] = atoi(argv[2]);
	width[1] = atoi(argv[3]);
	height[1] = atoi(argv[4]);
//	cout << strlen((char*)argv) << endl;

// Create KCFTracker object
	//KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
///	vector<KCFTracker> tracker(10);

	// Tracker results
	//Rect result;
//	vector<Rect> result(10);
//	int data[10];
	int key,num;
	ofstream truth;
	truth.open("groundtruth_rect.txt", ios::out | ios::app);
	if (truth.is_open()) cout << "open txt success!" << endl;

	width[0] = atoi(argv[1]);
	height[0] = atoi(argv[2]);
	width[1] = atoi(argv[3]);
	height[1] = atoi(argv[4]);
	num = atoi(argv[5]);

	// label each image
	string frameName;
	char imgName[100];
	string path = "./pic/";
	namedWindow("img");

	for (int n = num; n < 886; n++){
		sprintf(imgName, "%d.jpg", n);
		frameName = path + imgName;
		org = imread(frameName, CV_LOAD_IMAGE_COLOR);// Read each frame
		org.copyTo(img);
		imshow("img",img);
		setMouseCallback("img",on_mouse,0);

		key = waitKey(0);
		if(key==27){
			truth.close();
			break;
		}
	
/*		for(int j=0;j<num_obj;j++){		
			data[4*j] = Truth_rec[j].tl().x;		
			data[4*j+1] = Truth_rec[j].tl().y;			
			data[4*j+2] = width[j];
			data[4*j+3] = height[j];
		}
*/
		cout << Truth_rec[0].tl().x << " " << Truth_rec[0].tl().y <<" " << width[0] << " " << height[0] << " " << Truth_rec[1].tl().x << " " << Truth_rec[1].tl().y << " " << width[1] << " " << height[1] << endl;
		truth << Truth_rec[0].tl().x << "," << Truth_rec[0].tl().y << "," << width[0] << "," << height[0] << " " << Truth_rec[1].tl().x << "," << Truth_rec[1].tl().y << "," << width[1] << ","<< height[1] << endl;
		cout << n << " images are finished" << endl;
		cout << " " << endl;

		key = waitKey(0);
		if(key==27){
			truth.close();
			break;
		}

	}
//	truth.close();

/*	for (int i = 0;i < num_obj;i++){
		tracker[i]= KCFTracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
		tracker[i].init( Track_rec[i], org );
		rectangle( org, Track_rec[i].tl(), Track_rec[i].br(), Scalar( 0, 0, 255 ), 1, 8 );
		cur_pt = (Track_rec[i].tl() + Track_rec[i].br())/2 ;
		sprintf(temp,"(%d,%d)",cur_pt.x,cur_pt.y);
		putText(org,temp,cur_pt,FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,0,255), 1, 8);
	}
*/
	//video record
//	double rate = 25.0; //ftp
//	Size videoSize(org.cols,org.rows);
//	VideoWriter writer("VideoTest.avi", CV_FOURCC('M', 'J', 'P', 'G'), rate, videoSize);
//	writer<<org;

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
/**	Mat frame;
	string frameName;
	char imgName[100];
	string path = "./";

	for (int n = 1; n < 886; n++){
		sprintf(imgName, "%d.jpg", n);
		frameName = path + imgName;
		frame = imread(frameName, CV_LOAD_IMAGE_COLOR);// Read each frame

		// Update tracker
		for (int i =0;i< num_obj;i++){
			result[i] = tracker[i].update(frame);
			rectangle( frame, result[i].tl(),result[i].br(), Scalar( 0, 0, 255 ), 4, 8 );
			cur_pt = (result[i].tl() + result[i].br())/2 ;
            //srand(time(NULL));
//            double k = 210+rand()%11/10.0*5.0;
//            sprintf(temp,"(%d,%d)depth:%.2f",cur_pt.x,cur_pt.y,k);
			putText(frame,temp,cur_pt,FONT_HERSHEY_SIMPLEX,1,Scalar(0,0,0,255), 2, 8);
		}

		if (!SILENT){
//			writer<<frame;
			imshow("img", frame);
			waitKey(1);
		}
	}
**/


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
