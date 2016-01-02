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


//求取二值化的阈值
int myOtsu(IplImage *frame)
//int CLICENSE_PLATE_RECONGNITIONDlg::myOtsu(IplImage *frame) 
{
    #define GrayScale 256//frame灰度级
    int width = frame->width; 
    int height = frame->height; 
    int pixelCount[GrayScale]={0}; 
    float pixelPro[GrayScale]={0}; 
    int i, j, pixelSum = width * height, threshold = 0; 
    uchar* data = (uchar*)frame->imageData;

    //统计每个灰度级中像素的个数  
    for(i = 0; i < height; i++) 
    { 
        for(j = 0;j < width;j++) 
        { 
            pixelCount[(int)data[i * width + j]]++; 
        } 
    } 

    //计算每个灰度级的像素数目占整幅图像的比例  
    for(i = 0; i < GrayScale; i++) 
    { 
        pixelPro[i] = (float)pixelCount[i] / pixelSum; 
    } 
    //遍历灰度级[0,255],寻找合适的threshold 

    float w0, w1, u0tmp, u1tmp, u0, u1, deltaTmp, deltaMax = 0; 
    for(i = 0; i < GrayScale; i++) 
    { 
        w0 = w1 = u0tmp = u1tmp = u0 = u1 = deltaTmp = 0; 
        for(j = 0; j < GrayScale; j++) 
        { 
            if(j <= i)   //背景部分  
            { 
                w0 += pixelPro[j]; 
                u0tmp += j * pixelPro[j]; 
            } 
            else   //前景部分  
            { 
                w1 += pixelPro[j]; 
                u1tmp += j * pixelPro[j]; 
            } 
        } 
        u0 = u0tmp / w0; 
        u1 = u1tmp / w1; 
        deltaTmp = (float)(w0 *w1* pow((u0 - u1), 2)) ; 
        if(deltaTmp > deltaMax) 
        { 
            deltaMax = deltaTmp; 
            threshold = i; 
        } 
    } 
    return threshold;
}

int main()
{
	/*
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
	waitKey();*/
	//图像预处理（根据颜色特征）
	char*path = "f:\\111.jpg";
	IplImage*SrcImage = cvLoadImage(path);
	if(!SrcImage)return 0;
    IplImage *imgH=NULL,*imgS=NULL,*imgV=NULL,*imgHSV=NULL,*imgGray=NULL;
    imgHSV=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,3);
    imgH=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    imgS=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    imgV=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    cvCvtColor(SrcImage,imgHSV,CV_BGR2HSV); // 转为HSV图像
    cvSplit(imgHSV,imgH,imgS,imgV,NULL);    //将多个通道分别复制到各个单通道图像中
    cvInRangeS(imgH,cvScalar(94,0,0,0),cvScalar(115,0,0,0),imgH);   //选取各个通道的像素值范围，在范围内的则
    cvInRangeS(imgS,cvScalar(90,0,0,0),cvScalar(255,0,0,0),imgS);   //置1，否则置0（按颜色进行二值化）
    cvInRangeS(imgV,cvScalar(36,0,0,0),cvScalar(255,0,0,0),imgV);
    IplImage *imgTemp=NULL,*imgHsvBinnary=NULL;
    imgTemp=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    imgHsvBinnary=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    cvAnd(imgH,imgS,imgTemp);
    cvAnd(imgTemp,imgV,imgHsvBinnary);  //H,S,V三个通道分别按位求与，将所得的单通道图像保存到imgHsvBinnary中
    //形态学去噪
    //定义结构元素
    IplConvKernel *element=0;   //自定义核
    int values[2]={255,255};
    int rows=2,cols=1,anchor_x=0,anchor_y=1;
    element = cvCreateStructuringElementEx(cols,rows,anchor_x,anchor_y,CV_SHAPE_CUSTOM,values); 
    cvDilate(imgHsvBinnary,imgHsvBinnary,element,1);    //膨胀腐蚀
    cvErode(imgHsvBinnary,imgHsvBinnary,element,2);     //多次腐蚀（2次），消除噪声
    //cvNamedWindow("imgh1");
    //cvShowImage("imgh1",imgHsvBinnary);
    //阈值分割
    imgGray=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    cvCvtColor(SrcImage,imgGray,CV_RGB2GRAY);   //将原图转为单通道的灰色图像
    IplImage *imgRgbBinnary;
    imgRgbBinnary=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,1);
    int Thresold=myOtsu(imgGray);   //利用大津法求灰色图像阈值
    cvThreshold(imgGray,imgRgbBinnary,Thresold,255,CV_THRESH_OTSU); //利用大津阈值进行二值化
	//cvThreshold(imgGray,imgRgbBinnary,0,255,CV_THRESH_OTSU); //利用大津阈值进行二值化
    //cvNamedWindow("imgh2");
    //cvShowImage("imgh2",imgRgbBinnary);     //显示二值化图像
    //车牌定位
    //行定位（根据车牌的区域的图像特征进行定位）
    int hop_num=10; //字符连续跳变次数的阈值
    int num=0;      //计算跳变的次数
    int begin=0;    //跳变是否开始
    int mark_Row[2]={0},k=0;//第一次标记车牌的开始行与结束行
    int mark_Row1[2]={0};   //第二次标记
    //第一次定位
    for(int i=SrcImage->height-1;i>=0;i--)
    {
        num=0;
        for(int j=0;j<SrcImage->width-1;j++)
        {
            if(cvGet2D(imgHsvBinnary,i,j).val[0]!=cvGet2D(imgHsvBinnary,i,j+1).val[0])  //左右两列的值不等则视为一次跳变
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
    //cvLine(SrcImage,cvPoint(0,mark_Row[0]),cvPoint(SrcImage->width,mark_Row[0]),CV_RGB(255,255,0));   //在原图中画出所标记的两行
    //cvLine(SrcImage,cvPoint(0,mark_Row[1]),cvPoint(SrcImage->width,mark_Row[1]),CV_RGB(255,255,0));
    //列定位
    int mark_col[2]={0},mark_col1[2]={0},num_col=0,k_col=0;
    int a[100]={0},Thresold_col=7;
    for(int j=0;j<SrcImage->width;j++)
    {
        num_col=0;
        for(int i=mark_Row[1];i<mark_Row[0];i++)    //只扫描已经标记的两行之间的图像
            if(cvGet2D(imgHsvBinnary,i,j).val[0]>0)
                num_col++;
                if(num_col>Thresold_col)
                {
                    mark_col[k_col]=j;
                    k_col=1;
                }
    }
    int i=0;
    //cvLine(SrcImage,cvPoint(mark_col[0],0),cvPoint(mark_col[0],SrcImage->height),CV_RGB(255,0,0));
    //cvLine(SrcImage,cvPoint(mark_col[1],0),cvPoint(mark_col[1],SrcImage->height),CV_RGB(255,0,0));
    IplImage *imgLicense;
    int license_Width1=(mark_col[1]-mark_col[0]);
    int license_Height1 =mark_Row[0]-mark_Row[1];
    if(license_Width1/license_Height1<3)    //根据车牌的宽度和高度比对车牌区域进行修正
    {
        int real_height1 = license_Width1/3;    //车牌的宽度和高度比大概为3:1
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
        for(int i=mark_Row1[1];i<mark_Row1[0];i++)  //只扫描已经标记的两行之间的图像
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
    if(license_Width/license_Height<3)  //根据宽度和高度比再次修正
    {
        int real_height = license_Width/3;  //车牌的宽度和高度比大概为3:1
        mark_Row1[1] = mark_Row1[0]-real_height;
        license_Height = real_height;
    }
	IplImage*SrcLicenseimg1;
	IplImage*SrcLicenseimg2;
    cvSetImageROI(SrcImage,cvRect(mark_col1[0],mark_Row1[1],license_Width,license_Height)); //将车牌区域设置为ROI区域
    cvSetImageROI(imgRgbBinnary,cvRect(mark_col1[0],mark_Row1[1],license_Width,license_Height));
    cvSetImageROI(imgHsvBinnary,cvRect(mark_col1[0],mark_Row1[1],license_Width,license_Height));
    imgLicense=cvCreateImage(cvGetSize(SrcImage),SrcImage->depth,SrcImage->nChannels);  //用于显示的车牌图片
    SrcLicenseimg1=cvCreateImage(cvGetSize(imgRgbBinnary),imgRgbBinnary->depth,imgRgbBinnary->nChannels);
    SrcLicenseimg2=cvCreateImage(cvGetSize(imgHsvBinnary),imgHsvBinnary->depth,imgHsvBinnary->nChannels);
    cvCopy(SrcImage,imgLicense,0);
    cvCopy(imgRgbBinnary,SrcLicenseimg1,0); //将车牌区域拷贝到相应的图像中
    cvCopy(imgHsvBinnary,SrcLicenseimg2,0);
    //cvNamedWindow("SrcLicenseimg1");  //显示车牌的二值化化图片
    //cvShowImage("SrcLicenseimg1",SrcLicenseimg1);
    //cvNamedWindow("SrcLicenseimg2");  
    //cvShowImage("SrcLicenseimg2",SrcLicenseimg2);
    cvResetImageROI(SrcImage);  //取消ROI设置
    cvResetImageROI(imgRgbBinnary);
    cvResetImageROI(imgHsvBinnary);
	cvNamedWindow("license");
    cvShowImage("license",imgLicense);
    //cvResize(imgLicense,TheImage_plate);
    //ShowImage(TheImage_plate,IDC_LICENSE_AREA); //显示车牌
    cvLine(SrcImage,cvPoint(mark_col1[0],mark_Row1[0]),cvPoint(mark_col1[1],mark_Row1[0]),CV_RGB(255,0,0)); //在原图像中画出车牌区域
    cvLine(SrcImage,cvPoint(mark_col1[1],mark_Row1[0]),cvPoint(mark_col1[1],mark_Row1[1]),CV_RGB(255,0,0));
    cvLine(SrcImage,cvPoint(mark_col1[1],mark_Row1[1]),cvPoint(mark_col1[0],mark_Row1[1]),CV_RGB(255,0,0));
    cvLine(SrcImage,cvPoint(mark_col1[0],mark_Row1[1]),cvPoint(mark_col1[0],mark_Row1[0]),CV_RGB(255,0,0));
    cvNamedWindow("src");
    cvShowImage("src",SrcImage);
    //cvResize( SrcImage, TheImage );    //显示原图（已经画出了车牌区域）
    //ShowImage( TheImage, IDC_VIDEO_SHOW );
	waitKey();
}