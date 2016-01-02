#include<opencv2\opencv.hpp>
#include <opencv2/core/core.hpp> 
#include<iostream>
#include<opencv2/nonfree/features2d.hpp>
#include<opencv2/nonfree/nonfree.hpp>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv; 
int main()
{
	
	char*path = "f:\\1111.jpg";
	IplImage*frame = cvLoadImage(path);
	if(!frame)return 0;
	cvNamedWindow("frame",1);
	cvShowImage("frame",frame);
    //均值滤波
	cvSmooth(frame,frame,CV_MEDIAN);
	//灰度图
	IplImage*gray=cvCreateImage(cvGetSize(frame),frame->depth,1);
	cvCvtColor(frame,gray,CV_BGR2GRAY);
	cvNamedWindow("gray",1);
	cvShowImage("gray",gray);
	//边缘检测
	IplImage*temp=cvCreateImage(cvGetSize(gray),IPL_DEPTH_16S,1);
	//x方向梯度，垂直边缘
	cvSobel(gray,temp,2,0,3);
	IplImage*sobel=cvCreateImage(cvGetSize(temp),IPL_DEPTH_8U,1);
	cvConvertScale(temp,sobel,1,0);
	cvShowImage("sobel",sobel);
	//二值化
	IplImage*threshold=cvCreateImage(cvGetSize(sobel),gray->depth,1);
	cvThreshold(sobel,threshold,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
	cvNamedWindow("threshold",1);
	cvShowImage("threshold",threshold);
	//形态学变化
	IplConvKernel*kernal;
	IplImage*morph=cvCreateImage(cvGetSize(threshold),threshold->depth,1);
	//自定义1*3的核进行x方向的膨胀腐蚀
	kernal=cvCreateStructuringElementEx(3,1,1,0,CV_SHAPE_RECT);
	cvDilate(threshold,morph,kernal,2);//x膨胀联通数字
	cvErode(morph,morph,kernal,4);//x腐蚀去除碎片
	cvDilate(morph,morph,kernal,4);//x膨胀回复形态
	//自定义3*1的核进行y方向的膨胀腐蚀
	kernal=cvCreateStructuringElementEx(1,3,0,1,CV_SHAPE_RECT);
	cvErode(morph,morph,kernal,1);//y腐蚀去除碎片
	cvDilate(morph,morph,kernal,3);//y膨胀回复形态
	cvNamedWindow("erode",1);
	cvShowImage("erode",morph);
	//轮廓检测
	IplImage * frame_draw = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
    cvCopy(frame, frame_draw);
    CvMemStorage * storage = cvCreateMemStorage(0);  
    CvSeq * contour = 0;   
    int count = cvFindContours(morph,storage,&contour,sizeof(CvContour),CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);   
    CvSeq * _contour = contour;   
    for( ;contour != 0; contour = contour->h_next)
    {  		
	    double tmparea = fabs(cvContourArea(contour));  		 
	    CvRect aRect = cvBoundingRect( contour, 0 ); 
	    if(tmparea > ((frame->height*frame->width)/10))   
	    {  
		    cvSeqRemove(contour,0); //删除面积小于设定值的轮廓,1/10   
		    continue;  
	    } 
	    if (aRect.width < (aRect.height*2))  
	    {  
		    cvSeqRemove(contour,0); //删除宽高比例小于设定值的轮廓   
		    continue;  
	    }
	    if ((aRect.width/aRect.height) > 4 )
	    {  
		    cvSeqRemove(contour,0); //删除宽高比例小于设定值的轮廓   
		    continue;  
	    }
	    if((aRect.height * aRect.width) < ((frame->height * frame->width)/100))
	    {  
		    cvSeqRemove(contour,0); //删除宽高比例小于设定值的轮廓   
		    continue;  
	    }
	    CvScalar color = CV_RGB( 255, 0, 0); 
	    cvDrawContours(frame_draw, contour, color, color, 0, 1, 8 );//绘制外部和内部的轮廓
    }
    cvNamedWindow("轮廓", 1);
    cvShowImage("轮廓", frame_draw);
	waitKey();
}