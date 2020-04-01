#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <string>
#include <sstream>

#include"bmpFile.h"
#include "KxyLaneDetect.h"
#include"KxyImgRW.h"

//选择连续帧或单张
#define CONTINUOUS
#define SINGLE



//选择文件夹
#define BINKANGWEST
#define HUXIANG_XIAOYU
//#define XIAOXUE_1
using namespace std;

#ifdef CONTINUOUS
int main()
{
	clock_t start, end, last = 0, RunStart, RunEnd, RunLast;

	int i,num1=0, num2 = 100;
	char filename[100];
	KxyImgRW ImgStream;//彩色图像读写对象
	KxyLaneDetect LaneDetect;//行道线检测对象
	BYTE *pRGBImg, *pResImg;

	//彩色图像读写样本图像，该样本图像的大小代表接下来要处理的图像集中所有图像的大小
#ifdef BINKANGWEST
	sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\binkangwest\\image000000.bmp");
#elif defined HUXIANG_XIAOYU
	sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\湘湖_小雨\\image000000.bmp");
#elif defined XIAOXUE_1
	sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\小雪_1\\image000000.bmp");
#else
	sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\ROMA\\IMG000000.bmp");
#endif

	//-----------------------------------------------读模板图像--------------------------------------------//
	pRGBImg = ImgStream.ReadRGBImg(filename);
	if (pRGBImg == NULL)
	{
		printf("未成功读取模板图片");
		system("pause");

		return 0;
	}
	int height = ImgStream.GetHeight();
	int width = ImgStream.GetWidth();

	//----------------------------行道线检测对象初始化，通过样本图像的大小申请内存------------------------//
	LaneDetect.Initialize(true, width, height);

	last = 0;

	//开始计时
	start = clock();
	RunStart = clock();
	//---------------------------------------逐帧执行---------------------------//
	for (i = num1; i <= num2; i++)
	{
		//读图像
#ifdef BINKANGWEST
		sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\binkangwest\\image%06d.bmp", i);
#elif defined HUXIANG_XIAOYU
		sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\湘湖_小雨\\image%06d.bmp", i);
#elif defined XIAOXUE_1
		sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\小雪_1\\image%06d.bmp", i);
	//	sprintf(filename, "E:\\实验结果图\\Hough\\image%06d.bmp", i);
#else
		//------------实验测试部分------------------------------//
		//	sprintf(filename, "H:\\新建文件夹\\image%06d.bmp", i);
		sprintf(filename, "E:\\实验结果图\\灰度化\\image%06d.bmp", i);
		sprintf(filename, "E:\\实验结果图\\特征点\\image%06d.bmp", i);
		sprintf(filename, "E:\\实验结果图\\Hough\\IMG%06d.bmp", i);
		sprintf(filename, "G:\\ADAS_2_Student_SourceImage\\ROMA\\IMG%06d.bmp", i);
		//------------------------------------------------//
#endif




		//----------------------------------判断是否读成功---------------------------------//
		pRGBImg = ImgStream.ReadRGBImg(filename);
		if (pRGBImg == NULL)
		{
			printf("未成功读取第%d 张图片", i);
			break;
		}

		//开始计时
		start = clock();

		//--------------------------------------执行行道线检测-------------------------------//
#ifdef BINKANGWEST
		LaneDetect.DoNext(pRGBImg, 268, 374, 408, 479, 84, 708, 40, 0, 3);
#elif defined HUXIANG_XIAOYU
		LaneDetect.DoNext(pRGBImg, 314, 150, 467, 479, 5, 708, 25, 5, 1.5);
#elif defined XIAOXUE_1
		LaneDetect.DoNext(pRGBImg, 268, 374, 408, 479, 84, 708, 40, 0, 3);
#else
		LaneDetect.DoNext(pRGBImg, 430, 0, 1280, 1023, 0, 1280, 40, 0, 3);
#endif

		//-------------------------------------------获取结果--------------------------------------//
		pResImg = LaneDetect.GetResImg();

		//累加用时
		end = clock();
		last += end - start;

		
		//--------------------------------------------------写图像---------------------------//
#ifdef BINKANGWEST
		sprintf(filename, "G:\\ADAS_2_Student_Res_测\\binkangwest\\resImage%06d.bmp", i);
		sprintf(filename, "E:\\TestResImage\\ResIMG%06d.bmp", i);
#elif defined HUXIANG_XIAOYU
		sprintf(filename, "G:\\ADAS_2_Student_Res_测\\湘湖_小雨\\resImage%06d.bmp", i);
		sprintf(filename, "E:\\实验结果图\\测试\\ResIMG%06d.bmp", i);

#elif defined XIAOXUE_1
		sprintf(filename, "G:\\ADAS_2_Student_Res_测\\小雪_1\\resImage%06d.bmp", i);
	//	sprintf(filename, "E:\\实验结果图\\Hough\\Resimage%06d.bmp", i);
#else
		sprintf(filename, "G:\\ADAS_2_Student_Res_测\\ROMA\\IMG%06d.bmp", i);
#endif
		//ImgStream.WriteRGBImg(pResImg, filename);

		//		sprintf(filename, "G:\\ADAS_2_Student_Res_改3\\拟合点寻找\\resImage%06d.bmp", i);
		//		sprintf(filename, "G:\\ADAS_2_Student_Res_改3\\粗拟合\\d.bmp");
		//		sprintf(filename, "G:\\拟合点显示2\\d.bmp");
		// 		sprintf(filename, "G:\\TestImage_draft\\test%d.bmp",i);
		//		sprintf(filename, "H:\\新建文件夹 (2)\\image%06d.bmp", i);

		ImgStream.WriteRGBImg(pResImg, filename);

	}


	RunEnd = clock();
	RunLast = RunEnd - RunStart;

	cout << "处理runtime= " << last << " ms,  " << last*1.0 / (1 + num2 - num1) << "ms/frame,  " << (1 + num2 - num1)*1000.0 / last << " frame per sec." << endl;
	cout << "包括读写磁盘runtime= " << RunLast << " ms, " << (1 + num2 - num1)*1000.0 / RunLast << " per sec.";

	system("pause");

	return 0;
}

#elif defined SINGLE
int main()
{
	//单张

	int i, num = 100;
	char filename[100];
	KxyImgRW ImgStream;//彩色图像读写对象
	KxyLaneDetect LaneDetect;//行道线检测对象
	BYTE *pRGBImg, *pResImg;

	//sprintf(filename, "E:\\实验结果图\\阴影遮挡\\image.bmp");
	//sprintf(filename, "E:\\实验结果图\\分层投影\\image.bmp");
	sprintf(filename, "E:\\实验结果图\\特征点\\image.bmp");
	sprintf(filename, "E:\\实验结果图\\Hough\\image.bmp");
	sprintf(filename, "E:\\实验结果图\\曲线\\image.bmp");

	pRGBImg = ImgStream.ReadRGBImg(filename);
	if (pRGBImg == NULL)
	{
		printf("未成功读取模板图片");
		system("pause");

		return 0;
	}
	int height = ImgStream.GetHeight();
	int width = ImgStream.GetWidth();

	//行道线检测对象初始化，通过样本图像的大小申请内存
	LaneDetect.Initialize(true, width, height);

	//	sprintf(filename, "E:\\实验结果图\\阴影遮挡\\image.bmp");
	sprintf(filename, "E:\\实验结果图\\分层投影\\image.bmp");
	//	sprintf(filename, "E:\\实验结果图\\特征点\\image.bmp");
	//	sprintf(filename, "E:\\实验结果图\\Hough\\image.bmp");
	//	sprintf(filename, "E:\\实验结果图\\曲线\\image.bmp");


	pRGBImg = ImgStream.ReadRGBImg(filename);
	if (pRGBImg == NULL)
	{
		printf("未成功读取实验图片");
		system("pause");

		return 0;
	}

	//LaneDetect.DoNext(pRGBImg, 0, 0, 1012, 270, 0, 1012, 30, 5, 0);
	//LaneDetect.DoNext(pRGBImg, 268, 374, 408, 479, 84, 708, 40, 0, 3);
	LaneDetect.DoNext(pRGBImg, 314, 150, 467, 479, 5, 708, 25, 5, 1.5);

	pResImg = LaneDetect.GetResImg();
	//	sprintf(filename, "E:\\实验结果图\\阴影遮挡\\ResImage.bmp");
	sprintf(filename, "E:\\实验结果图\\分层投影\\Resimage.bmp");
	//	sprintf(filename, "E:\\实验结果图\\Hough\\Rescheimage.bmp");
	//	sprintf(filename, "E:\\实验结果图\\曲线\\Resimage.bmp");

	ImgStream.WriteRGBImg(pResImg, filename);

}

#endif