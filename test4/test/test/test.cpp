#include<opencv2\opencv.hpp>
#include <opencv2/core/core.hpp> 
#include<iostream>
#include<opencv2/nonfree/features2d.hpp>
#include<opencv2/nonfree/nonfree.hpp>
#include<opencv2/opencv.hpp>
#include"cv.h"
#include"cxcore.h"
using namespace std;
using namespace cv; 
int main()
{
	//图像预处理（根据颜色特征）
	char*path = "009.jpg";
	IplImage*SrcImage = cvLoadImage(path);
	if(!SrcImage)return 0;
    IplImage *imgH=NULL,*imgS=NULL,*imgV=NULL,*imgHSV=NULL,*imgGray=NULL;
    imgHSV=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,3);
    imgH=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    imgS=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    imgV=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    cvCvtColor(SrcImage,imgHSV,CV_BGR2HSV);
    cvSplit(imgHSV,imgH,imgS,imgV,NULL);
    cvInRangeS(imgH,cvScalar(94,0,0,0),cvScalar(115,0,0,0),imgH);   
    cvInRangeS(imgS,cvScalar(90,0,0,0),cvScalar(255,0,0,0),imgS);   
    cvInRangeS(imgV,cvScalar(36,0,0,0),cvScalar(255,0,0,0),imgV);
    IplImage *imgTemp=NULL,*imgHsvBinnary=NULL;
    imgTemp=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    imgHsvBinnary=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    cvAnd(imgH,imgS,imgTemp);
    cvAnd(imgTemp,imgV,imgHsvBinnary);
    //形态学去噪
    //定义结构元素
    IplConvKernel *element=0;   //自定义核
    int values[2]={255,255};
    int rows=2,cols=1,anchor_x=0,anchor_y=1;
    element = cvCreateStructuringElementEx(cols,rows,anchor_x,anchor_y,CV_SHAPE_CUSTOM,values); 
    cvDilate(imgHsvBinnary,imgHsvBinnary,element,1);    
    cvErode(imgHsvBinnary,imgHsvBinnary,element,2);  
    cvNamedWindow("imgh1");
    cvShowImage("imgh1",imgHsvBinnary);
	//车牌定位
    //行定位（根据车牌的区域的图像特征进行定位）
    int hop_num=8; 
    int num=0;     
    int begin=0;   
    int mark_Row[2]={0},k=0;
    int mark_Row1[2]={0};
    //第一次定位
    for(int i=SrcImage->height-1;i>=0;i--)
    {
        num=0;
        for(int j=0;j<SrcImage->width-1;j++)
        {
            if(cvGet2D(imgHsvBinnary,i,j).val[0]!=cvGet2D(imgHsvBinnary,i,j+1).val[0])
               {
                    num++;
               }
        }
        if(num>hop_num)
            {
                mark_Row[k]=i;
                k=1;
             }
    }
   cvLine(SrcImage,cvPoint(0,mark_Row[0]),cvPoint(SrcImage->width,mark_Row[0]),CV_RGB(255,255,0)); 
    cvLine(SrcImage,cvPoint(0,mark_Row[1]),cvPoint(SrcImage->width,mark_Row[1]),CV_RGB(255,255,0));
    //列定位
    int mark_col[2]={0},mark_col1[2]={0},num_col=0,k_col=0;
    int a[100]={0},Thresold_col=7;
    for(int j=0;j<SrcImage->width;j++)
    {
        num_col=0;
        for(int i=mark_Row[1];i<mark_Row[0];i++)
            if(cvGet2D(imgHsvBinnary,i,j).val[0]>0)
                num_col++;
                if(num_col>Thresold_col)
                {
                    mark_col[k_col]=j;
                    k_col=1;
                }
    }
    int i=0;
    cvLine(SrcImage,cvPoint(mark_col[0],0),cvPoint(mark_col[0],SrcImage->height),CV_RGB(255,0,0));
    cvLine(SrcImage,cvPoint(mark_col[1],0),cvPoint(mark_col[1],SrcImage->height),CV_RGB(255,0,0));
    IplImage *imgLicense;
    int license_Width1=(mark_col[1]-mark_col[0]);
    int license_Height1 =mark_Row[0]-mark_Row[1];
    if(license_Width1/license_Height1<3)
    {
        int real_height1 = license_Width1/3; 
        mark_Row[1] = mark_Row[0]-real_height1;
        license_Height1 = real_height1;
    }
    //第二次定位（在第一次定位的基础之上）
    k=0;
    for(int i=mark_Row[0];i>mark_Row[1];i--)
    {
        num=0;
        for(int j=mark_col[0];j<mark_col[1];j++)
        {
            if(cvGet2D(imgHsvBinnary,i,j).val[0]!=cvGet2D(imgHsvBinnary,i,j+1).val[0])  //左右两列的值不等则视为一次跳变
               {
                    num++;
               }
        }
        if(num>8)
            {
                mark_Row1[k]=i;
                k=1;
             }
    }
    k_col=0;
    for(int j=mark_col[0];j<mark_col[1];j++)
    {
        num_col=0;
        for(int i=mark_Row1[1];i<mark_Row1[0];i++) 
            if(cvGet2D(imgHsvBinnary,i,j).val[0]>0)
                num_col++;
                if(num_col>6)
                {
                    mark_col1[k_col]=j;
                    k_col=1;
                }
    }
    int license_Width=(mark_col1[1]-mark_col1[0]);
    int license_Height =mark_Row1[0]-mark_Row1[1];
    if(license_Width/license_Height<3)
    {
        int real_height = license_Width/3;  //车牌的宽度和高度比大概为3:1
        mark_Row1[1] = mark_Row1[0]-real_height;
        license_Height = real_height;
    }
    cvSetImageROI(SrcImage,cvRect(mark_col1[0],mark_Row1[1],license_Width,license_Height));
    cvSetImageROI(imgHsvBinnary,cvRect(mark_col1[0],mark_Row1[1],license_Width,license_Height));
    imgLicense=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,SrcImage->nChannels);
    cvCopy(SrcImage,imgLicense,0);
    cvResetImageROI(SrcImage);
    cvResetImageROI(imgHsvBinnary);
	cvNamedWindow("chepai");
    cvShowImage("chepai",imgLicense);
    cvLine(SrcImage,cvPoint(mark_col1[0],mark_Row1[0]),cvPoint(mark_col1[1],mark_Row1[0]),CV_RGB(255,0,0));
    cvLine(SrcImage,cvPoint(mark_col1[1],mark_Row1[0]),cvPoint(mark_col1[1],mark_Row1[1]),CV_RGB(255,0,0));
    cvLine(SrcImage,cvPoint(mark_col1[1],mark_Row1[1]),cvPoint(mark_col1[0],mark_Row1[1]),CV_RGB(255,0,0));
    cvLine(SrcImage,cvPoint(mark_col1[0],mark_Row1[1]),cvPoint(mark_col1[0],mark_Row1[0]),CV_RGB(255,0,0));
    cvNamedWindow("src");
    cvShowImage("src",SrcImage);
	waitKey();
}