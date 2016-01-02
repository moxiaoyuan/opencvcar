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
	/*Mat img=imread("003.jpg");
	cvtColor(img,img,CV_BGR2GRAY);
	Mat contours;
	Canny(img,contours,125,350);
	threshold(contours,contours,125,255,THRESH_BINARY);
	namedWindow("tu");
	imshow("tu",contours);
	/*if(img.empty())
	{
		cout<<"error";
		return -1;
	}
	imshow("tu",img);*/
	/*waitKey();*/
	Mat car_img=imread("003.jpg");
	Mat gray;
	cvtColor(car_img,gray,CV_BGR2GRAY);
	//高斯滤波器滤波去噪
	int ksize = 3;
	Mat g_gray;
	Mat G_kernel = getGaussianKernel(ksize,0.3*((ksize-1)*0.5-1)+0.8);
	filter2D(gray,g_gray,-1,G_kernel);
	//Sobel x,y,方向
	Mat sobel_x,sobel_y;
	Sobel(g_gray,sobel_x,CV_16S,1,0,3);
	Sobel(g_gray,sobel_y,CV_16S,0,1,3);
	Mat abs_x,abs_y;
	convertScaleAbs(sobel_x,abs_x);
	convertScaleAbs(sobel_y,abs_y);
	Mat grad;
	addWeighted(abs_x,0.5,abs_y,0.5,0,grad);
	Mat img_bin;
	threshold(gray,img_bin,0,255,CV_THRESH_BINARY | CV_THRESH_OTSU);
	//二值化
	Mat gray_bi;
	threshold(gray,gray_bi,0,255,CV_THRESH_OTSU);
	//灰度拉伸
//	Mat equ_img;
//	equalizeHist(gray_bi,equ_img);
	float num[256], p[256],p1[256];
	memset(num,0,sizeof(num));// 清空三个数组
	memset(p,0,sizeof(p));
	memset(p1,0,sizeof(p1));
	long wMulh = gray_bi.cols * gray_bi.rows;
	for (int i = 0; i < gray_bi.cols; i++)
	{
		for (int j = 0; j < gray_bi.rows; j++)
		{
			int v = gray_bi.at<uchar>(j,i);
			num[v]++;
		}
	}
	for (int i = 0; i < 256; i++)//存放图像各个灰度级的出现概率
	{
		p[i] = num[i] / wMulh;
	}
	for (int i = 0; i < 256; i++)//求存放各个灰度级之前的概率和
	{
		for (int k = 0; k <= i; k++)
		{
			p1[i]+=p[k];
		}
	}
	for (int x = 0; x < gray_bi.cols; x++)
	{
		for (int y = 0; y < gray_bi.rows; y++)
		{
			int v = gray_bi.at<uchar>(y,x);
			gray_bi.at<uchar>(y,x) = p1[v]*255 + 0.5;
		}
	}
	//边缘增强
	Mat gray_c;
	Canny(gray_bi,gray_c,50,150,3);

	//水平投影和垂直投影
	int imgR[400] = {0};
	bool tag = false;
	int imgTop =0;int imgBottom = 0;
	int img_h1,img_h2;
	for (int ht = 0; ht < gray_c.rows; ht++)
	{
		for (int wt = 0; wt < gray_c.cols; wt++)
		{
			if (gray_c.at<uchar>(ht,wt) != 0)
			{
				imgR[ht]++;
			}
		}
		if ( (!tag)&& imgR[ht] > 10)
		{
			img_h1 = ht;
			tag = true;
		}
		if (tag && imgR[ht] <10)
		{
			img_h2 =ht;
			tag = false;
		}
		if (img_h2-img_h1<50 && img_h2 - img_h1 >25)
		{
			imgTop = img_h1;
			imgBottom = img_h2;
			break;
		}
	}
	
	int imgR_w[300] = {0};
	int img_w1 = 0;
	int img_w2 = 0;
	int imgRight = 0;
	int imgLeft = 0;
	bool tag2 = false;
	for (int wt_new = 2; wt_new < gray_c.cols; wt_new++)
	{
		for (int ht_new = imgTop; ht_new < imgBottom; ht_new++)
		{
			if (255 == gray_c.at<uchar>(ht_new,wt_new))
			{
				imgR_w[wt_new]++;
			}
		}

		if ( (!tag2) && (imgR_w[wt_new-2] +imgR_w[wt_new-1] + imgR_w[wt_new])/3 > 5)
		{
			img_w1 = wt_new;
			tag2 = true;
		}
		if (tag2 && (imgR_w[wt_new-2] +imgR_w[wt_new-1] + imgR_w[wt_new])/3 <5)
		{
			img_w2 =wt_new;
			tag2 = false;
		}
		if (img_w2 - img_w1 <180 && img_w2-img_w1 > 70)
		{
			imgLeft = img_w1;
			imgRight = img_w2;
			break;
		}
	}

	Mat imgroi;
	//imgroi = car_img( Rect(imgTop,imgLeft,imgRight-imgLeft,imgTop-imgBottom));
	imgroi = car_img( Rect(imgLeft,imgTop+10,115,30));

//////////////////////////////////////////////////////////////////////////////////////
	//第三部分：字符分割
	//二值化
	Mat roi_g, roi_bi;
	cvtColor(imgroi,roi_g,CV_BGR2GRAY);
	threshold(roi_g,roi_bi,0,255,CV_THRESH_OTSU);

	bool lab = false;	//是否进入一个字符分割状态
	bool black = false;	//是否发现黑点
	bool change = false;
	int xnum = 0;
	int rect_left;
	int rect_right;
	//Rect selection;
	Mat fg1,fg2,fg3,fg4,fg5,fg6,fg7;
	for (int wt = 0; wt < roi_bi.cols; wt++)
	{
		int count = 0;
		for (int ht = 0; ht < roi_bi.rows; ht++)
		{
			if ((255==roi_bi.at<uchar>(ht,wt)) && (!change))
			{
				count++;
				change = true;
			}
			else if((0==roi_bi.at<uchar>(ht,wt))&&(change))
			{
				count++;
				change = false;
			}
		}
		if (!lab&&(count>5))
		{
			rect_left = wt - 3;
			lab = true;
		}
		if (lab&&(count<5)&&(wt>(rect_left+8))&& (xnum<7) )
		{
			rect_right = wt + 2;
			lab = false;
			CvPoint pt1,pt2;
			pt1.x = rect_left;
			pt1.y = 0;
			pt2.x = rect_right;
			pt2.y = roi_bi.cols-1;
			int s_x = pt1.x + 1;
			int s_y = pt1.y;
			int s_width = rect_right - rect_left +1;
			int s_height = roi_bi.rows -1;
			if (xnum == 0)
			{
				fg1 = roi_bi(Rect(s_x,s_y,s_width,s_height));
			} 
			if (xnum == 1)
			{
				fg2 = roi_bi(Rect(s_x,s_y,s_width,s_height));
			}			
			if (xnum == 2)
			{
				fg3 = roi_bi(Rect(s_x,s_y,s_width,s_height));
			}			
			if (xnum == 3)
			{
				fg4 = roi_bi(Rect(s_x,s_y,s_width,s_height));
			}
			if (xnum == 4)
			{
				fg5 = roi_bi(Rect(s_x,s_y,s_width,s_height));
			}
			if (xnum == 5)
			{
				fg6 = roi_bi(Rect(s_x,s_y,s_width,s_height));
			}
			if (xnum == 6)
			{
				fg7 = roi_bi(Rect(s_x,s_y,s_width,s_height));
			}
			xnum++;
		}
	}
	cvWaitKey(50000);
	return 0;
}