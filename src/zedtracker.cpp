#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <time.h>

#include <sl/Camera.hpp>
#include "kcftracker.hpp"
#include "uart.hpp"
#include <dirent.h>

using namespace std;
using namespace cv;

////////////////////////////////////////////////////////////////////////////
class detector
{
public:
	Point contours(Mat sin, Mat dst);
	Mat feature_extract(Mat image);
};

//////////////////////////////////////////////////////////////////////////////
Mat detector::feature_extract(Mat image)
{
	Mat kern = (Mat_<char>(5,5)<<0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0);
	cvtColor(image,image,CV_BGR2GRAY);
	medianBlur(image, image, 29);
	Canny(image, image,70,180,3);
	filter2D(image, image, image.depth(), kern);
//	threshold(image,image,127,255,CV_THRESH_BINARY);
//	adaptiveThreshold(image, image, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 2);
//	erode(image,image,Mat(3,3,CV_8U),Point(-1,-1),5);
//	dilate(image,image,Mat(3,3,CV_8U),Point(-1,-1),2);
//	morphologyEx(image,image,MORPH_OPEN,Mat(3,3,CV_8U),Point(-1,-1),1);
//	morphologyEx(image,image,MORPH_CLOSE,Mat(3,3,CV_8U),Point(-1,-1),1);
	return image;
}

//////////////////////////////////////////////////////////////////////////////
Point detector::contours(Mat sin, Mat dst)
{
/*-----------draw contours--------------*/	
	
	Point centre = Point(-1,-1);
	vector<vector<Point>> contour;
	vector<vector<Point>>::iterator itr;
	findContours(sin, contour, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	itr = contour.begin();
	while(itr!= contour.end()){
		if(fabs(contourArea(*itr))>700) itr++;
		else itr = contour.erase(itr);
	}

//	drawContours(dst, contour, -1, (0,0,255), 2);

/*----------choose the right box-------------*/	
	vector<RotatedRect> box(contour.size());
	int Max=contour.size();
	float area=0;
	Point2f vtx[4];
//	vector<float> conner;
//	vector<float>::iterator itr_min,itr_maxx,itr_maxy;

	for(int i=0;i<contour.size();i++){ 
		box[i] = minAreaRect(Mat(contour[i])); //dst.size[0]:480 dst.size[1]:640
//		conner.clear();	
//		for(int j=0;j<4;j++) conner.push_back(vtx[j].x);
//		for(int j=0;j<4;j++) conner.push_back(vtx[j].y);
//		itr_min = min_element(conner.begin(),conner.end());
//		itr_maxx = max_element(conner.begin(),conner.end()-5);
//		itr_maxy = max_element(conner.begin()+4,conner.end());

//		if(*itr_min > 5 && *itr_maxx < dst.cols-5 && *itr_maxy < dst.rows-5 && 
		if(max(box[i].size.width,box[i].size.height)/min(box[i].size.width,box[i].size.height)<4) {
			if(fabs(contourArea(contour[i])) > area) {
				area = fabs(contourArea(contour[i]));
				Max = i;
			}
		}
	}
	
	if(Max!=contour.size()) {
		box[Max].points(vtx); 
		centre = Point(box[Max].center.x,box[Max].center.y);
		circle(dst, centre, 5, Scalar(0,255,0),-1,8); 
		for(int j=0;j<4;j++) line(dst,vtx[j],vtx[(j+1)%4],Scalar(255,0,0));	
	}
	else centre = Point(-1,-1);
	return centre;
}

//////////////////////////////////////////////////////////////////////////////
// global variable
Mat org,img,tmp; 
int num_obj = 0;
vector<Rect> Track_rec(10);//create container

/////////////////////////////////////////////////////////////////////////////
void on_mouse(int event,int x,int y,int flags,void *ustc)//event:code for mouse event，x,y:coordinate of mouse，flags:code for drag  
{  
    static Point pre_pt = Point(-1,-1);//original coordinate  
    static Point cur_pt = Point(-1,-1);//real-time coordinate
	//static Point tmp_min = pre_pt;
	//static Point tmp_max = cur_pt;	
    char temp[16];  
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
        
		//tmp_min = pre_pt,tmp_max = cur_pt;		
        Track_rec[num_obj] = Rect(pre_pt,cur_pt);
		num_obj++;
		cout<<"Objects number is: "<<num_obj<<endl;
        if(num_obj<2) {
            cout << "Objects number less than 2; We need 2 objects" << endl;
        }
        else if(num_obj>2) {
            cout << "Objects number greater than 2; The latter objects will not be used" << endl;
        }
        waitKey(500);  
    }  
}

//////////////////////////////////////////////////////////////////////////////
//This function displays ZED camera information
void printCameraInformation(sl::Camera &zed) {
    printf("ZED Serial Number         : %d\n", zed.getCameraInformation().serial_number);
    printf("ZED Firmware              : %d\n", zed.getCameraInformation().firmware_version);
    printf("ZED Camera Resolution     : %dx%d\n", (int)zed.getResolution().width, (int)zed.getResolution().height);
    printf("ZED Camera FPS            : %d\n", (int) zed.getCameraFPS());
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]){

	if (argc > 5) return -1;
	bool HOG = true;
	bool FIXEDWINDOW = true;
	bool MULTISCALE = true;
	bool SILENT = false;
	bool LAB = false;

	for(int i = 0; i < argc; i++){
		if ( strcmp (argv[i], "hog") == 0 )
			HOG = true;
		if ( strcmp (argv[i], "non_fixed_window") == 0 )
			FIXEDWINDOW = false;
		if ( strcmp (argv[i], "singlescale") == 0 )
			MULTISCALE = false;
		if ( strcmp (argv[i], "no_show") == 0 )
			SILENT = true;
		if ( strcmp (argv[i], "lab") == 0 ){
			LAB = true;
			HOG = true;
		}
		if ( strcmp (argv[i], "gray") == 0 )
			HOG = false;
	}
	
	// Create KCFTracker object
	vector<KCFTracker> tracker(10);
	vector<Rect> result(10);

	////// image process variable ////////
	clock_t t=clock();
	Mat sin, dst;//, gray, blur, cany, smooth;
//	Mat kern = (Mat_<char>(5,5)<<0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,0);
//	Mat sin, dst, gray, gauss, thresh, dilated, element;
	Point centre;
    int xx_dis=0, yy_dis=0;
    int width = 640;
    int height = 480;
	detector detect;

	/////// zed Frame variable /////////
	Mat frame, frame_dep, frame_dis;
	char key;
    Point  tar1_cen, tar2_cen;
    int x_dis=0, y_dis=0, z_dis=0;

    ///////// Create a ZED camera //////////////////////////
    sl::Camera zed;

    //////////// Initialize ZED parameter ///////////////
    sl::ERROR_CODE err; // error state for all ZED SDK functions
    sl::InitParameters init_params;
    init_params.camera_resolution = sl::RESOLUTION_HD720 ;
    init_params.camera_fps = 60;
    init_params.depth_mode = sl::DEPTH_MODE_PERFORMANCE; //need quite a powerful graphic card in quality
    init_params.coordinate_units = sl::UNIT_MILLIMETER; // set meter as the opengl world will be in meters
    init_params.coordinate_system = sl::COORDINATE_SYSTEM_LEFT_HANDED_Z_UP; // opengl's coordinate system is right_handed
//	init_params.depth_minimum_distance = 100.0;

    /////////////// Open ZED ///////////////////////////////
    sl::RuntimeParameters runtimeParameters;
    runtimeParameters.sensing_mode = sl::SENSING_MODE_FILL;
    runtimeParameters.enable_depth = true;
//	zed.setConfidenceThreshold(99);
    err = zed.open(init_params);
//	waitKey(5);
    if (err != sl::SUCCESS) {
        std::cout << errorCode2str(err) << std::endl;
        zed.close();
        return EXIT_FAILURE; // quit if an error occurred
    }
    printCameraInformation(zed);

	//////////////// ZED image defination //////////////////////
    err = zed.grab();    
	if(err != sl::SUCCESS) return -1;
    sl::Mat zed_image(width, height, sl::MAT_TYPE_8U_C4);
    Mat zed_image_ocv= Mat(zed_image.getHeight(), zed_image.getWidth(), CV_8UC4, zed_image.getPtr<sl::uchar1>(sl::MEM_CPU));
    sl::Mat zed_depth(width, height, sl::MAT_TYPE_8U_C4);
    Mat zed_depth_ocv= Mat(zed_image.getHeight(), zed_image.getWidth(), CV_8UC4, zed_depth.getPtr<sl::uchar1>(sl::MEM_CPU));
    sl::Mat zed_disparity(width, height, sl::MAT_TYPE_32F_C1);
    Mat zed_disparity_ocv= Mat(zed_image.getHeight(), zed_image.getWidth(), CV_32FC1, zed_disparity.getPtr<sl::uchar1>(sl::MEM_CPU));

    //////////// retrieve and label the first image //////////////////
    zed.retrieveImage(zed_image, sl::VIEW_LEFT, sl::MEM_CPU, width, height);
    cvtColor(zed_image_ocv, org, CV_RGBA2RGB, 3);
    org.copyTo(img);
    org.copyTo(tmp);
    namedWindow("img");
    setMouseCallback("img", on_mouse,0);
    imshow("img",img);
    waitKey(0);
	
	//////////// KCF initialize ////////////////// 
	for (int i = 0;i < num_obj;i++){
		tracker[i]= KCFTracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
		tracker[i].init( Track_rec[i], org);
		rectangle( org, Track_rec[i].tl(), Track_rec[i].br(), Scalar( 0, 255, 255 ), 1, 8 );
	}	
    cout<<"KCF initialized"<<endl;

    ///////// Output video initialize //////////
    char EXT[] = "MJPG";
    int ex1 = EXT[0] | (EXT[1] << 8) | (EXT[2] << 16) | (EXT[3] << 24);
    VideoWriter writer,video;
    if(argv[2]!="NONE") writer.open(argv[2], ex1, 25, cv::Size(width, height), true);
	if(argv[3]!="NONE") video.open(argv[3], ex1, 25, cv::Size(width, height), true);
    if(!writer.isOpened()) cout << "Can not record writer for ZED" << endl;
    if(!video.isOpened()) cout << "Can not record video for single" << endl;

	//////// Open single camera /////////
	if(argv[1]=="NONE") cout << "please select a camera to open!" << endl;
	int num = atoi(argv[1]);
	VideoCapture cam(num);
	namedWindow("single");
	if(!cam.isOpened()) cout << "single fail" << endl;

    //////////// UART initialize ///////////////////////////   
	unsigned char x_byte=0, y_byte=0, z_byte=0, x_step=0, y_step=0, z_step=0, x_dir=0, y_dir=0, z_dir=0, xx_byte=0, yy_byte=0, xx_step=0, yy_step=0, xx_dir=0, yy_dir=0;
//	char *xx_ptr = &xx_step;
//	char *yy_ptr = &yy_step;
	unsigned char start=191; // 1011 1111
    unsigned char end=128; // 1000 0000
    unsigned char x_axis=0; // 0000 0000
    unsigned char y_axis=16; //0001 0000
    unsigned char z_axis=48; //0011 0000
	unsigned char zed_flag=0; //0000 0000
	unsigned char cam_flag=64; //0100 0000	
	bool uart_open_flag, uart_init_flag;
	int send_flag=0;
    std::vector<char> send_list;
    Uart uart("ttyUSB0", 38400);
    uart_open_flag = uart.openFile();
    if (uart_open_flag == false)
        cout<< "UART fails to open "<<endl;
    uart_init_flag = uart.initPort();
    if (uart_init_flag == false)
        cout<< "UART fails to be inited "<<endl;


    while (1){

        err = zed.grab(runtimeParameters);
        if(err != sl::SUCCESS){
            zed.retrieveImage(zed_image, sl::VIEW_LEFT, sl::MEM_CPU, width, height);
            cvtColor(zed_image_ocv, frame, CV_RGBA2RGB, 3);
			
			//////// Update tracker //////////	
            for (int i =0;i< num_obj;i++){
                result[i] = tracker[i].update(frame);
                if(i==0) rectangle( frame, result[i].tl(),result[i].br(), Scalar( 255, 0, 0 ), 1, 8 );
                if(i==1) rectangle( frame, result[i].tl(),result[i].br(), Scalar( 0, 0, 255 ), 1, 8 );
            }

			///////// single camera detect ////////
			if(cam.isOpened()){
				centre = Point(-1,-1);
				cam >> sin;		
//				sin = imread("1.jpg", CV_LOAD_IMAGE_COLOR);
				resize(sin,sin,Size(width,height));
				dst = sin.clone();

//				cvtColor(sin.clone(), gray, COLOR_BGR2GRAY);
//				medianBlur(gray, blur, 29);
//				Canny(blur, cany,70,180,3);
//				filter2D(cany, smooth, gray.depth(), kern);

//				GaussianBlur(gray, gauss, Size(5,5), 0, 0);
//				element = getStructuringElement(MORPH_ELLIPSE, Size(4,4));
//				adaptiveThreshold(gauss, thresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 2);
//				dilate(thresh, dilated, element);
				sin = detect.feature_extract(sin);
				centre = detect.contours(sin, dst);

	            /////////// single uart data ///////////
				if(centre.x==-1 && centre.y==-1){ //no object detected
					xx_step = 255;					
					yy_step = 255;
				}
				else{
					xx_dis = 14.0 * (centre.x - 320) / width;      
					yy_dis = 14.0 * (centre.y - 240) / height;
					xx_step = (unsigned char)abs(xx_dis);
					yy_step = (unsigned char)abs(yy_dis);
//					gcvt(abs(x_dis),1, xx_ptr);
//					gcvt(abs(y_dis),1, yy_ptr);
					if(xx_dis > 0) xx_dir = 0;
					else xx_dir = 8;
					if(yy_dis > 0) yy_dir = 8;
					else yy_dir = 0;
				}

				/////////// ZED uart data //////////////
				// 1 is soft arm and 2 is object
				tar1_cen = Point((result[0].tl().x+result[0].br().x)/2, (result[0].tl().y+result[0].br().y)/2);
				tar2_cen = Point((result[1].tl().x+result[1].br().x)/2, (result[1].tl().y+result[1].br().y)/2);
				x_dis = 7.0 * (tar1_cen.x - tar2_cen.x) / 160;//width
				z_dis = 7.0 * (tar1_cen.y - tar2_cen.y) / 160; //height
				x_step = (unsigned char)abs(x_dis);
				z_step = (unsigned char)abs(z_dis);
				if(x_dis > 0) x_dir = 8; //0000 1000
				else x_dir = 0; //0000 0000
				if(z_dis > 0) z_dir = 0;
				else z_dir = 8;

	            zed.retrieveImage(zed_depth, sl::VIEW_DEPTH, sl::MEM_CPU, width, height);
			    cvtColor(zed_depth_ocv, frame_dep, CV_RGBA2GRAY, 1);
			    zed.retrieveMeasure(zed_disparity, sl::MEASURE_DISPARITY, sl::MEM_CPU, width, height);

	            if (zed_disparity_ocv.at<float>(tar1_cen) == sl::TOO_CLOSE)
	               cout<< "Target 1 is too close"<<endl;
	            else if (zed_disparity_ocv.at<float>(tar2_cen) == sl::TOO_CLOSE)
	               cout<< "Target 2 is too close"<<endl;
			    else {
				   y_dis = 7.0 * (10 * zed_disparity_ocv.at<float>(tar1_cen) - 10 * zed_disparity_ocv.at<float>(tar2_cen)) / width;
                   y_step = (unsigned char)abs(y_dis);
                   if(y_dis > 0) y_dir = 0;
                   else y_dir = 8;
				}

//				cout << tar1_cen.x << ", " << tar2_cen.x << ", " << zed_disparity_ocv.at<float>(tar1_cen) << ", " << zed_disparity_ocv.at<float>(tar2_cen) << endl;			
				x_byte = x_axis | x_dir | x_step | zed_flag;
				y_byte = y_axis | y_dir | y_step | zed_flag;
				z_byte = z_axis | z_dir | z_step | zed_flag;
				xx_byte = x_axis | xx_dir | xx_step | cam_flag;
				yy_byte = y_axis | yy_dir | yy_step | cam_flag;
//				cout<< (int)start << ", " <<(int)xx_byte << ", "<< (int)yy_byte << ", " << (int)x_byte << ", " << (int)y_byte << ", "<< (int)z_byte << ", " << (int)end <<endl;

				///////// send uart data /////////////////
				if (uart_open_flag && uart_init_flag) {
					send_list.clear();
					send_list.push_back(start);
					send_list.push_back(xx_byte);
					send_list.push_back(yy_byte);
					send_list.push_back(x_byte);
					send_list.push_back(y_byte);
					send_list.push_back(z_byte);
					send_list.push_back(end);

					if(clock()-t>=2000000) {
						send_flag = uart.send(send_list);
						t = clock();
						cout << tar1_cen.x << ", " << tar2_cen.x << ", " << zed_disparity_ocv.at<float>(tar1_cen) << ", " << zed_disparity_ocv.at<float>(tar2_cen) << endl;							
						cout<< (int)start << ", " <<(int)xx_byte << ", "<< (int)yy_byte << ", " << (int)x_byte << ", " << (int)y_byte << ", "<< (int)z_byte << ", " << (int)end <<endl;
					}
//					cout <<sizeof(send_list)<< endl;
				}

				if (!SILENT){
					imshow("img", frame);
					imshow("depth", frame_dep);
					imshow("single", dst);
				}

				if(writer.isOpened()) writer << frame;
				if(video.isOpened()) video << dst;
				if(waitKey(1) == 27) break;			
			}

			else {
				cout << "single camera open error" << endl;
				break;
			}

        }
	}

    zed.close();
    uart.closeFile();
	cam.release();
    if(writer.isOpened()) writer.release();
    return 0;
}
