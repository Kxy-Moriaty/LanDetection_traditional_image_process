#define _CRT_SECURE_NO_WARNINGS

//#define FIR_KIND_OF_WAY_LAYER
//#define SEC_KIND_OF_WAY_LAYER
//#define Thi_KIND_OF_WAY_LAYER
#define NEW_KIND_OF_WAY_LAYER
//#define DEBUG_MOOD



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// KxyLaneDetect.cpp : implementation file
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include<vector>

#include "KxyLaneDetect.h"
#include"RGBImageBaseProcessing.h"
#include"bmpFile.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 构造/析构函数
//
////////////////////////////////////////////////////////////////////////////////////////////////////

KxyLaneDetect::KxyLaneDetect()
{  	//<在构造函数中把所有的成员变量赋初值>
	//是否初始化成功
	m_isInitOK = false;
	//图像属性
	m_isRGB = false;
	m_width = 0;
	m_height = 0;
	//内存空间
	m_pRGBImg = NULL;
	m_pGryImg = NULL;
	m_pResImg = NULL;
	m_pSumImg = NULL;
	m_pTemImg = NULL;
	m_prjHist = NULL;
	m_prjTmpH = NULL;
	m_memSize = 0; //已申请的内存字节数
				   //感兴趣区域指针
	m_pROI = NULL;
	m_FPntNum = 0;
	//内部计数器
	m_nFrameID = 0;
	//计算sin和cos乘以4096的整形表
	CalSinLUTM4096();
	CalCosLUTM4096();
	//弯道标志
	m_isCurve = false;
	//三层中线段信息记录
	
	m_FirLineNum = 0;

	
	m_SecLineNum = 0;

	
	m_ThiLineNum = 0;

	//曲线个数
	m_curve = NULL;
	m_curveNum = 0;

	//参数传递
	m_tY = 0;
	m_tlX = 0;//tl：topleft	tr：topright
	m_trX = 0;
	m_bY = 0;
	m_blX = 0;//bl：bottomleft br：bottomright
	m_brX = 0;
	m_maxDetectWidth = 0;//图像底部的行道线宽度
	m_layerNum = 0;//分层投影层数
}

KxyLaneDetect::~KxyLaneDetect()
{	//<在析构函数中释放所有的内存>
	Dump();
}

void KxyLaneDetect::Dump()
{   //<写一个专门的用作释放内存的函数,因为内存会有多次的申请与释放>
	if (m_pRGBImg) { delete m_pRGBImg; m_pRGBImg = NULL; }
	if (m_pGryImg) { delete m_pGryImg; m_pGryImg = NULL; }
	if (m_pResImg) { delete m_pResImg; m_pResImg = NULL; }
	if (m_pTemImg) { delete m_pTemImg; m_pTemImg = NULL; }
	if (m_pSumImg) { delete m_pSumImg; m_pSumImg = NULL; }
	if (m_prjHist) { delete m_prjHist; m_prjHist = NULL; }
	if (m_prjTmpH) { delete m_prjTmpH; m_prjTmpH = NULL; }

	m_FirTheta.clear();
	m_FirThro.clear();
	m_FirLineNum = 0;
	m_SecTheta.clear();
	m_SecThro.clear();
	m_SecLineNum = 0;
	m_ThiTheta.clear();
	m_ThiThro.clear();
	m_ThiLineNum = 0;

	if (m_curve) { delete m_curve; m_curve = NULL; }
	m_curveNum = 0;

	m_memSize = 0;
	m_FPntXROI.clear();
	m_FPntYROI.clear();
	////删除sin和cos乘以4096的整形表
	//delete m_sinLUT;
	//delete m_cosLUT;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 初始化
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool KxyLaneDetect::Initialize(bool isRGB, int width, int height)
{   //<在初始化函数中对所有的成员变量赋初值>
	//<仅在初始化函数中申请内存,因为初始化函数不会被频繁调用>
	//<初始化函数可以被多次调用>
	//<内存申请时,若已经申请的内存大于本次需要的内存,则不再次申请>

	//step.1------图像属性--------------------------//
	m_isRGB = isRGB;
	m_width = width;
	m_height = height;
	//step.2------内存申请--------------------------//
	if (m_width*m_height > m_memSize)
	{
		//先释放
		Dump();
		//后申请
		m_pRGBImg = new BYTE[m_width*m_height * 3]; //彩色图像
		m_pGryImg = new BYTE[m_width*m_height];  //灰度图像
		m_pTemImg = new BYTE[m_width*m_height];
		m_pResImg = new BYTE[m_width*m_height * 3]; //结果图像
		m_pSumImg = new int[m_width*m_height];//积分图
		m_FPntXROI.resize(200);
		m_FPntYROI.resize(200);//特征点坐标
		m_prjHist = new int[m_width];//分层投影直方图
									 //记录申请的大小
		m_prjTmpH = new int[m_width];
		m_memSize = m_width*m_height;
		//感兴趣区域（下半部分）  m_pROI为感兴趣区域的头指针
		//m_pROI = m_pGryImg + (m_height >> 1)*m_width;
		//m_pROI = m_pGryImg + (m_height *56/100)*m_width;//第268行（包括第0行）

		//三层中线段信息
		m_FirTheta.resize(15);
		m_FirThro.resize(15);
		m_FirLineNum = 0;
		m_SecTheta.resize(15);
		m_SecThro.resize(15);
		m_SecLineNum = 0;
		m_ThiTheta.resize(15);
		m_ThiThro.resize(15);
		m_ThiLineNum = 0;

		m_curve = new int[15];
		m_curveNum = 0;
	}
	//step.3------初始化成功标志--------------------//
	m_isInitOK = (m_pRGBImg && m_pGryImg  && m_pResImg && m_pSumImg);
	return m_isInitOK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 执行
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//(待处理图像指针，道路区域顶部y坐标，道路区域左上x，道路区域右上x，道路区域底部y，道路区域左下x，道路区域右下x，图像底部行道线宽度）
bool KxyLaneDetect::DoNext(BYTE *pRGBImg, int tY, int tlX, int trX, int bY, int blX, int brX, int maxDetectWidth, int minDetectWidth, int contrast)
{
	// step.0----------------------------------判断初始化是否成功，并计算所需参数----------------------------------------------//
	if (!m_isInitOK) return false;
	m_tY = tY;
	m_tlX = tlX, m_trX = trX;//tl：topleft	tr：topright
	m_bY = bY;
	m_blX = blX, m_brX = brX;//bl：bottomleft br：bottomright
	m_maxDetectWidth = maxDetectWidth;//图像底部的行道线宽度
	m_minDetectWidth = minDetectWidth;
	//m_pROI = m_pGryImg + (m_height * 56 / 100)*m_width;//第268行（包括第0行）
	m_pROI = m_pGryImg + tY*m_width;//感兴趣区域坐标确定
	m_layerNum = (m_bY - m_tY) / 3;
	m_contrast = contrast;
	m_TopY = tY + 3;
	m_MidY = tY + (bY - tY)*0.25;///0.25//0.3
	m_BottomY = tY + (bY - tY)*0.52;///0.52//0.63

	// step.1----------------------------------将形参图像复制到m_pRGBImg中-----------------------------------------------------//
	memcpy(m_pRGBImg, pRGBImg, 3 * m_memSize);
	memcpy(m_pResImg, m_pRGBImg, 3 * m_memSize);

	// step.2----------------------------------用彩色图像的绿通道值作为灰度值得到m_pGryImg-------------------------------------//
	CalGreenChannel();
#ifdef DEBUG_MOOD
	/*char filename[100];
	sprintf(filename,"G:\\TestImage\\a%d.bmp", m_nFrameID);
	RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);*/
#endif

	// step.3----------------------------------计算m_pGryImg的积分图-----------------------------------------------------------//
	CalGry8bIntegralImage();

	// step.4----------------------------------通过分层投影计算得到特征点------------------------------------------------------//
	CalFPoints();

	// step.5----------------------------------分三段，利用三段消失点的位置关系验证是否为弯道----------------------------------//
	m_isCurve = false;
	//IsCurve();
	//LineJoint();

	// step.6----------------------------------判断是否为弯道------------------------------------------------------------------//
	if (m_isCurve)
	{
	// step.7----------------------------------若是弯道，拟合，画线------------------------------------------------------------//
		CurveFitting();
	}
	else
	{
	// step.8----------------------------------若是直道，画线------------------------------------------------------------------//
		Hough(m_layerNum>>3);
	}



	//某个特定帧的调试,比如10
	/*if (m_nFrameID==10)
	{
	Debug();
	}*/

	//调试
#ifdef DEBUG_MOOD
	Debug();
#endif


	//内部计数器累加
	m_nFrameID++;
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 结果
//
////////////////////////////////////////////////////////////////////////////////////////////////////

BYTE *KxyLaneDetect::GetResImg()
{   //返回结果图像
	return m_pResImg;
}
int KxyLaneDetect::GetResWidth()
{   //返回结果图像的宽度
	return m_width;
}
int KxyLaneDetect::GetResHeight()
{   //返回结果图像的高度
	return m_height;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 调试
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void KxyLaneDetect::Debug()
{   //<本类的内部调试: 屏幕输出、文件输出等>

	//--debug;

	//分层及特征点测试
	int i;
	int yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE));
	memset(m_pResImg, 0, m_memSize * sizeof(BYTE));
	BYTE* pCur;
	for (i = 0; i < m_FPntNum; i++)
	{
		m_pResImg[m_FPntYROI[i] * m_width + m_FPntXROI[i]] = 255;
		m_pGryImg[(m_FPntYROI[i] + yROI) * m_width + m_FPntXROI[i]] = 0;

	}

	for (pCur = m_pGryImg + m_width * m_TopY; pCur < m_pGryImg + m_width *(m_TopY + 1); pCur++)
		*pCur = 0;
	for (pCur = m_pGryImg + m_width * m_MidY; pCur < m_pGryImg + m_width *(m_MidY + 1); pCur++)
		*pCur = 0;
	for (pCur = m_pGryImg + m_width * m_BottomY; pCur < m_pGryImg + m_width *(m_BottomY + 1); pCur++)
		*pCur = 0;

	char filename[100];
	//
	//#ifdef FIR_KIND_OF_WAY_LAYER
	//	sprintf(filename, "G:\\TestImage_1\\res%06d.bmp", m_nFrameID);
	//	RmwWriteByteImg2BmpFile(m_pResImg, m_width, m_height, filename);
	//	sprintf(filename, "G:\\TestImage_1\\gry%06d.bmp", m_nFrameID);
	//	RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);
	//#elif defined SEC_KIND_OF_WAY_LAYER
	//	sprintf(filename, "G:\\TestImage_2\\res%06d.bmp", m_nFrameID);
	//	RmwWriteByteImg2BmpFile(m_pResImg, m_width, m_height, filename);
	//	sprintf(filename, "G:\\TestImage_2\\gry%06d.bmp", m_nFrameID);
	//	RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);
	//#else
	//	sprintf(filename, "G:\\TestImage_3\\res%06d.bmp", m_nFrameID);
	//	RmwWriteByteImg2BmpFile(m_pResImg, m_width, m_height, filename);
	//	sprintf(filename, "G:\\TestImage_3\\gry%06d.bmp", m_nFrameID);
	//	RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);
	//#endif
	//	//int bbb = 0;

	/*AddChannel2RGB24b(m_pResImg, "Green", m_pRGBImg, m_width, m_height);
	RmwWrite24BitImg2BmpFile(m_pRGBImg, width, height, "..\\ResImage\\resRGB.bmp");*/


	//分层化线测试

	/*int bot, mid, top;
	for (i = 1; i <= m_curveNum; i++)
	{
	bot = m_curve[i] / 100;
	mid = (m_curve[i] % 100) / 10;
	top = m_curve[i] % 10;
	DrawLine(m_ThiTheta[bot], m_ThiThro[bot], 390 - 268, 479 - 268, i % 3);
	DrawLine(m_SecTheta[mid], m_SecThro[mid], 340 - 268, 390 - 268, i % 3);
	DrawLine(m_FirTheta[top], m_FirThro[top], 300 - 268, 340 - 268, i % 3);
	}*/


	//所有线段展示
	//for(i=1;)


	sprintf(filename, "G:\\TestImage_draft\\gry%06d .bmp", m_nFrameID);
	RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);

	sprintf(filename, "G:\\TestImage_draft\\point%06d.bmp", m_nFrameID);
//	RmwWrite24BitImg2BmpFile(m_pResImg, m_width, m_height, filename);



	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//私有成员函数
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//私有函数：计算[-90,90]的三角函数表*4096
void KxyLaneDetect::CalSinLUTM4096()
{
	int i;
	double rad = 3.14159 / 180;
	m_sinLUTM4096 = &m_sinLUT[90];

	for (i = -90; i <= 90; i++)
		m_sinLUTM4096[i] = sin(i*rad) * 4096;
}
void KxyLaneDetect::CalCosLUTM4096()
{
	int i;
	double rad = 3.14159 / 180;
	m_cosLUTM4096 = &m_cosLUT[90];

	for (i = -90; i <= 90; i++)
		m_cosLUTM4096[i] = cos(i*rad) * 4096;
}

//私有函数：获得24位位图某通道
void KxyLaneDetect::CalChannelBase(int firstPixel)
{
	/*
	获得某通道
	firstPixel==0:蓝
	firstPixel==1:绿
	firstPixel==2:红
	*/

	int i, j;
	int grySize = m_width * m_height;
	BYTE *pRGB = m_pRGBImg + firstPixel;
	BYTE *pGry = m_pGryImg;

	for (i = 0; i < grySize; i++)
	{
		*(pGry) = *(pRGB);
		pGry++;
		pRGB = pRGB + 3;
	}

	/*for (i = firstPixel, j = 0; i < rgbSize; i += 3, j++)
	{
	m_pGryImg[j] = m_pRGBImg[i];
	}*/
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//公有成员函数
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//获得蓝通道
void KxyLaneDetect::CalBlueChannel()
{
	CalChannelBase(0);
}
//获得绿通道
void KxyLaneDetect::CalGreenChannel()
{
	CalChannelBase(1);
}
//获得红通道
void KxyLaneDetect::CalRedChannel()
{
	CalChannelBase(2);
}

//计算8位灰度图像积分图	图像像素个数不得大于2^24=16777216约1600万像素
void KxyLaneDetect::CalGry8bIntegralImage()
{
	BYTE *pGry;
	int *pSum;
	int x, y;

	// step.1-----------------先求第一行--------------------//
	pGry = m_pGryImg;
	pSum = m_pSumImg;
	*(pSum++) = *(pGry++);		//第一个元素
	for (x = 1; x < m_width; x++) *(pSum++) = *(pSum - 1) + (*(pGry++));
	// step.2-----------------再求其他行--------------------//
	for (y = 1; y < m_height; y++)
	{
		*(pSum++) = *(pSum - m_width) + *(pGry++); //先求每行第一列
		for (x = 1; x < m_width; x++) //再求其他列
		{
			*(pSum++) = *(pGry++)
				+ (*(pSum - 1))
				+ (*(pSum - m_width))
				- (*(pSum - 1 - m_width));
		}
	}
	// step.3-----------------调试与返回--------------------//
	return;
}

//分层投影rowNUM层，获取特征点坐标，x/y坐标分别存储在featureX/featureY中;	//maxDetectWidth：底部行道线宽度
//BottomYMidYTopY分别为弯道检查三层划分的Y坐标（原图坐标），引入的意义在于记录特征点坐标的下标，以便Hough变换时不用遍历
void KxyLaneDetect::CalFPoints()
{
	int x, y, yUp;
	int i, j, k;
	int  *rowfirst, *rowend, *pCur;

	//width为图像即感兴趣区域宽度，yROI为感兴趣区域在原图中的y坐标，height为感兴趣区域的图像高度
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	//row_per_layer、rowPerLayer为分层投影时每层的行数，deertaRow为相邻层间的行数差
	//double row, row_per_layer = 2.0 *height / m_layerNum, deertaRow = 2.0 *height / (m_layerNum*(m_layerNum - 1));
#ifdef FIR_KIND_OF_WAY_LAYER
	double row, row_per_layer = 2.0 *height / m_layerNum, deertaRow = 1.5 *height / (m_layerNum*(m_layerNum - 1));//假设中间一层高x行，最上一层与最下一层分别为0.5x和2x行，通过计算获得如此初值。
																												  //该例中，row_per_layer=6,deertaRow=0.0655
	double detect_Width, deertaDetectWidth = 1.0*m_maxDetectWidth / m_layerNum;//detect_Width、detectWidth为检测行道线滑块的宽度，deertaDetectWidth为相邻层间的行道线滑块的宽度差

#elif defined SEC_KIND_OF_WAY_LAYER
	double row, row_per_layer = 8.0 *height / (m_layerNum * 5), deertaRow = 1.5 *row_per_layer / (2 * (m_layerNum - 1));//假设中间一层高x行，最上一层与最下一层分别为0.5x和2x行，通过计算获得如此初值。
																														//该例中，row_per_layer=4.845,deertaRow=0.05267
	double detect_Width, deertaDetectWidth = 1.0*m_maxDetectWidth / m_layerNum;//detect_Width、detectWidth为检测行道线滑块的宽度，deertaDetectWidth为相邻层间的行道线滑块的宽度差

#elif defined Thi_KIND_OF_WAY_LAYER


	double row, row_per_layer = 8.0 *height / (m_layerNum * 5), deertaRow = 1.5 *row_per_layer / (2 * (m_layerNum - 1));//假设中间一层高x行，最上一层与最下一层分别为0.5x和2x行，通过计算获得如此初值。
	double detect_Width, deertaDetectWidth = 1.0*(m_maxDetectWidth - m_minDetectWidth) / (m_layerNum - 1);//detect_Width、detectWidth为检测行道线滑块的宽度，deertaDetectWidth为相邻层间的行道线滑块的宽度差

#else																								  

	double row, row_per_layer = 5, deertaRow = 4.0 / (m_layerNum - 1);//假设中间一层高x行，最上一层与最下一层分别为0.5x和2x行，通过计算获得如此初值。
	double detect_Width = m_maxDetectWidth, deertaDetectWidth = 1.0*(m_maxDetectWidth - m_minDetectWidth) / (m_layerNum - 1);//detect_Width、detectWidth为检测行道线滑块的宽度，deertaDetectWidth为相邻层间的行道线滑块的宽度差

#endif																								 

	int rowPerLayer = row_per_layer;
	//	double detect_Width, deertaDetectWidth = 1.0*m_maxDetectWidth / m_layerNum;//detect_Width、detectWidth为检测行道线滑块的宽度，deertaDetectWidth为相邻层间的行道线滑块的宽度差
	//	double slope = (m_trX - m_tlX)*1.0 / (m_brX - m_blX);
	int detectWidth;

	//fPntIndex为特征点的下标，roadWidth为检测到的可能的行道线宽度，roadHeight为检测到的可能的行道线在投影差值直方图中的波峰高度值
	//roadRight为可能的行道线右侧X坐标
	int fPntIndex = 0;
	int roadWidth, roadHeight;
	int roadRight;
	//flag为是否为真正的行道线的标志
	bool flag;
	int maxSum, preMaxSum, sum;

	int halfDetectWidth;

	/*for (y = 0; y < m_height - row_per_layer; y += row_per_layer)
	{
	memset(m_prjHist, 0, width * sizeof(int));
	memset(m_prjTmpH, 0, width * sizeof(int));

	rowfirst = m_pSumImg + y*width;
	i = y + row_per_layer;
	pCur = m_pSumImg + i*width;
	m_prjHist[0] = *pCur - *rowfirst;
	for (j = 1; j < width; j++)
	m_prjHist[j] = *(pCur + j) - *(pCur + j - 1) - (*(rowfirst + j) - *(rowfirst + j - 1));

	HistogramAvgFilter(m_prjHist, width, 101, m_prjTmpH);
	CalIntArrayDifference(m_prjHist, m_prjTmpH, width, m_prjHist);
	}*/

	//-------------------------开始行道线检测-------------------------//
	fPntIndex = 0;
	m_FPntNum = 0;
	//fPntIndex = m_FPntNum;
	detect_Width = m_maxDetectWidth, detectWidth = detect_Width;
	for (row = y = m_height - 1; y > yROI; )		//从下到上分层投影
	{
		memset(m_prjHist, 0, width * sizeof(int));
		memset(m_prjTmpH, 0, width * sizeof(int));

		//step1.-------------------------计算一维投影图M1-------------------------//
		pCur = m_pSumImg + y*width;
		//yUp = y - row_per_layer;
		yUp = row - row_per_layer;
		rowfirst = m_pSumImg + yUp*width;
		//m_prjHist[0] = *pCur - *rowfirst;
		for (j = 1; j < width - 1; j++)
			m_prjHist[j] = *(pCur + j) - *(pCur + j - 1) - (*(rowfirst + j) - *(rowfirst + j - 1));
		//边缘点
		m_prjHist[0] = m_prjHist[1], m_prjHist[width - 1] = m_prjHist[width - 2];

		//step2.-------------------------一维投影图M1均值滤波平滑得到M2-------------------------//
		HistogramAvgFilter(m_prjHist, width, int(2 * detect_Width), m_prjTmpH);
		//step3.-------------------------计算M1,M2差值M-------------------------//
		CalIntArrayDifference(m_prjHist, m_prjTmpH, width, m_prjHist);

		//step4.-------------------------计算获取M中的特征点-------------------------//
		halfDetectWidth = (detectWidth >> 1);
		for (i = 0; i < width; /*i++*/)
		{
			if (m_prjHist[i] <= 0)	//找到M第一个非负点
			{
				i++;
				continue;
			}

			for (j = i + 1, roadHeight = -99999999; j < width; j++)	//找到一个波峰和波峰的高度
			{
				if (m_prjHist[j] < 0)
					break;
				if (m_prjHist[j] > roadHeight)
					roadHeight = m_prjHist[j];
			}

			roadWidth = j - i;

			//if (!(halfDetectWidth < roadWidth&&roadWidth < halfDetectWidth * 3))	//波峰宽度不够或太宽，舍去
			//	//if (!(detectWidth*3/4 < roadWidth&&roadWidth < detectWidth * 5 / 4))	//波峰宽度不够或太宽，舍去
			//{
			//	i = j + 1;	//减小复杂度
			//	continue;
			//}
			if (roadHeight < roadWidth * m_contrast)			//波峰斜率不够，舍去
			{
				i = j + 1;
				continue;
			}

			//检测波峰左右的波谷
			for (flag = true, k = max(0, i - halfDetectWidth); k < i; k++)//检测波峰左边的波谷
			{
				if (m_prjHist[k] > 0)
				{
					flag = false;
					break;
				}
			}
			if (flag == false)
			{
				i = j;
				continue;
			}
			for (flag = true, k = j; k < min(width, j + halfDetectWidth); k++)//检测波峰右边的波谷
				if (m_prjHist[k] > 0)
				{
					flag = false;
					break;
				}
			if (flag == false)
			{
				i = k;	//减小复杂度
				continue;
			}
			roadRight = k;
			//检测波峰左右的波谷完成

			//记录下特征点坐标，存入m_FPntXROI、m_FPntYROI中，并更新m_FPntNum和fPntIndex
			if (fPntIndex >= m_FPntXROI.size())	//扩展容器大小//开空间很慢，一次开一倍
			{
				m_FPntXROI.resize(2 * m_FPntXROI.size());
				m_FPntYROI.resize(2 * m_FPntYROI.size());
			}
			//用行道线中点为特征点
			m_FPntXROI[fPntIndex] = ((i + j) >> 1);
			m_FPntYROI[fPntIndex] = ((y + yUp) >> 1) - yROI;//特征点Y坐标为感兴趣区域中的Y坐标
			m_FPntNum++;
			fPntIndex++;

			/*if (m_FPntYROI[fPntIndex] + yROI<BottomY && m_FPntYROI[fPntIndex - 1] + yROI>BottomY)
			{
			m_IndexBottom = fPntIndex;

			}
			if (m_FPntYROI[fPntIndex] + yROI<MidY && m_FPntYROI[fPntIndex - 1] + yROI>MidY)
			{
			m_IndexMid = fPntIndex;

			}
			if (m_FPntYROI[fPntIndex] + yROI<TopY && m_FPntYROI[fPntIndex - 1] + yROI>TopY)
			{
			m_IndexTop = fPntIndex;

			}*/


			//for (k = i; k < j; k++)
			//{
			//	if (fPntIndex >= m_FPntXROI.size())	//扩展容器大小
			//	{
			//		m_FPntXROI.resize(2 * m_FPntXROI.size());
			//		m_FPntYROI.resize(2 * m_FPntYROI.size());
			//	}
			//	m_FPntXROI[fPntIndex] = k;
			//	m_FPntYROI[fPntIndex] = ((y + yUp) >> 1)- yROI;
			//	m_FPntNum++;
			//	fPntIndex++;
			//}

			//偏移i，寻找下个波峰
			i = roadRight;
			//i = k;
			//i += (detectWidth - 1);
		}

		//移到下一个投影区域
		row -= row_per_layer, y = row, row_per_layer -= deertaRow;
#ifdef NEW_KIND_OF_WAY_LAYER
		detect_Width -= deertaDetectWidth, detectWidth = detect_Width;
		//	detect_Width = detect_Width*slope, detectWidth = detect_Width;
#else
		detect_Width -= deertaDetectWidth, detectWidth = detect_Width;
#endif

		if (row_per_layer <= 1 || detectWidth <= m_minDetectWidth)
			break;
	}


	/*AddChannel2RGB24b(m_pResImg, "Green", m_pRGBImg, m_width, m_height);
	RmwWrite24BitImg2BmpFile(m_pRGBImg, width, height, "..\\ResImage\\resRGB.bmp");*/
}

//划线到m_pResImg	//theta、thro：直线极坐标参数
//注意，此处TopY，BottomY为ROI中的坐标，切记
void KxyLaneDetect::DrawLine(int theta, int thro, int TopY, int BottomY, int color)
{
	int i, j;
	double rad = 1.0 / 180 * 3.14159;//弧度

									 //width为图像即感兴趣区域宽度，yROI为感兴趣区域在原图中的y坐标，height为感兴趣区域的图像高度
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;

	//x=thro/cos（theta）+ y*sin（theta）		使temp=thro/cos（theta），则有x=temp + y*tan（theta）
	double Tan = tan(theta*rad);
	int temp = thro / cos(theta*rad);
	//pRGBROI指向彩色图像的感兴趣区域
	BYTE *pRGBROI = m_pResImg + yROI*width * 3;
	BYTE *pCur;

	//对于每个y坐标，计算其直线上的x坐标，将该点变色
	for (i = TopY; i < BottomY; i++)
	{
		j = temp - int(i*Tan);
		//横坐标超出图像范围，则continue
		if (j >= width || j < 0)
			continue;
		//将直线上的点变色
		pCur = pRGBROI + (i*width + j) * 3;
		*pCur = 0;
		*(pCur + 1) = 0;
		*(pCur + 2) = 0;
		*(pCur + color) = 255;

		//--------------画粗点------------//
		/*pCur = pRGBROI + (i*width + j+1) * 3;
		*pCur = 0;
		*(pCur + 1) = 0;
		*(pCur + 2) = 0;
		*(pCur + color) = 255;
		pCur = pRGBROI + (i*width + j -1 ) * 3;
		*pCur = 0;
		*(pCur + 1) = 0;
		*(pCur + 2) = 0;
		*(pCur + color) = 255;
		pCur = pRGBROI + ((i+1)*width + j + 1) * 3;
		*pCur = 0;
		*(pCur + 1) = 0;
		*(pCur + 2) = 0;
		*(pCur + color) = 255;
		pCur = pRGBROI + ((i - 1)*width + j + 1) * 3;
		*pCur = 0;
		*(pCur + 1) = 0;
		*(pCur + 2) = 0;
		*(pCur + color) = 255;*/
		//----------------------------//

	}

}

// 判断是否为真的行道线，即筛选出正确行道线	//形参  theta、thro：直线极坐标参数		angleRange:行道线theta的范围	leftBorderRatio\rightBorderRatio∈[0,1]:行道线消失线范围（比例值）
bool KxyLaneDetect::IsLane(int theta, int thro, unsigned int  angleRange, double leftBorderRatio, double rightBorderRatio)
{
	int tmp, i;
	int width = m_width,
		yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)),
		height = m_height - yROI;
	int topY, bottomY, topX, bottomX;

	//step1.-------------------------判断该直线斜率是否在行道线的可接受范围内-------------------------//
	if (theta<-int(angleRange) || theta>int(angleRange))
		return false;

	//step2.-------------------------判断该直线是否经过行道线消失区域--------------------------------//
	tmp = width*cos(theta*3.14159 / 180);
	if (!(leftBorderRatio*tmp < thro&&thro < rightBorderRatio*tmp))
		return false;

	//step3.-------------------------判断该直线长度是否够长--------------------------------//
	topY = 99999999, bottomY = -1;
	//从底部往上找第一个穿过直线的点，记录为bottomX\Y
	for (i = 0; i < m_FPntNum; i++)
	{
		if (abs(((m_FPntXROI[i] * m_cosLUTM4096[theta] + m_FPntYROI[i] * m_sinLUTM4096[theta]) >> 12) - thro) <= 1)///如果线段穿过特征点
		{
			bottomY = m_FPntYROI[i];
			bottomX = m_FPntXROI[i];
			break;
		}
	}
	//从顶部往下找第一个穿过直线的点，记录为topX\Y
	for (i = m_FPntNum - 1; i >= 0; i--)
	{
		if (abs(((m_FPntXROI[i] * m_cosLUTM4096[theta] + m_FPntYROI[i] * m_sinLUTM4096[theta]) >> 12) - thro) <= 1)///如果线段穿过特征点
		{
			topY = m_FPntYROI[i];
			topX = m_FPntXROI[i];
			break;
		}
	}
	//判断长度是否够长，若不够则舍弃（长度为感兴趣区域高度的一半）
	if ((topX - bottomX)*(topX - bottomX) + (topY - bottomY)*(topY - bottomY) < ((m_height - m_tY)*(m_height - m_tY) >> 2))
		return false;

	//step4.-------------------------经过筛选，返回true--------------------------------//
	return true;

}

// 判断是否为真的行道线，即筛选出正确行道线	//形参  theta、thro：直线极坐标参数		angleRange:行道线theta的范围	leftBorderRatio\rightBorderRatio∈[0,1]:行道线消失线范围（比例值）
bool KxyLaneDetect::IsLane_improve(int theta, int thro, unsigned int  angleRange, double leftBorderRatio, double rightBorderRatio)
{
	int tmp, i;
	int width = m_width,
		yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)),
		height = m_height - yROI;
	int topY, bottomY, topX, bottomX;

	//step1.-------------------------判断该直线斜率是否在行道线的可接受范围内-------------------------//
	if (theta<-int(angleRange) || theta>int(angleRange))
		return false;

	//step2.-------------------------判断该直线是否经过行道线消失区域--------------------------------//
	tmp = width*cos(theta*3.14159 / 180);
	if (!(leftBorderRatio*tmp < thro&&thro < rightBorderRatio*tmp))
		return false;

	
	//step3.-------------------------经过筛选，返回true--------------------------------//
	return true;

}


//霍夫变换:threshold代表检测特征点的最小数目 ver3.0
void KxyLaneDetect::Hough(int threshold)
{
	int deertaTheta = 15, deertaThro = max(m_width, m_height) >> 2;	//deertaTheta为检测行道线精确位置的角度ρ跨度，deertaThro为θ跨度
	int tmp;
	int i, j;
	int thro, theta, throNew, thetaNew, throReal, thetaReal;		//极坐标θ(theta)∈[-90,90]，ρ(thro)∈[-height,diagonalLen]
	int pntCount;		//能检测到的特征点数目

						//width为图像即感兴趣区域宽度，yROI为感兴趣区域在原图中的y坐标，height为感兴趣区域的图像高度
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	//diagonalLen为感兴趣区域的图像对角线宽度
	int diagonalLen = sqrt(width*width + height*height);
	int halfHeight = (height >> 1), halfDiagonalLen = (diagonalLen >> 1);

	//缩短thro的跨度为原来的一半，将相邻两个不同的thro归为一类，提高运算速度
	int thetaSpan = 180 + 1, throSpan = halfDiagonalLen + halfHeight + 1;
	int centerIndex = 90 * throSpan + halfHeight;			//中心点坐标

	int AccumArrLen = thetaSpan*throSpan;					//计数数组长度
	int *pAccumArr = new int[AccumArrLen];					//计数数组
	memset(pAccumArr, 0, sizeof(int)*AccumArrLen);
	int *pArr = pAccumArr + centerIndex;					//pArr指向计数数组的中间

															//step1.-------------------------计算获得计数数组-------------------------//
	for (theta = -90; theta <= 90; theta++)
	{
		for (j = 0; j < m_FPntNum; j++)
		{
			thro = ((m_FPntXROI[j] * m_cosLUTM4096[theta] + (m_FPntYROI[j]) * m_sinLUTM4096[theta]) >> 13);	//
			pArr[theta*throSpan + thro]++;
		}
	}
	//step2.-------------------------根据计数数组找到直线，并划线-------------------------//
	for (theta = -89; theta <= 90; theta++)
	{
		for (thro = -halfHeight; thro <= halfDiagonalLen; thro++)
		{
			pntCount = pArr[theta*throSpan + thro];
			if (pntCount >= threshold)										//找到能穿过一定数目（threshold）特征点的疑似行道线
			{
				thetaReal = thetaNew = theta, throReal = throNew = thro;
				for (thetaNew = theta; thetaNew < min(90, theta + deertaTheta); thetaNew++)	//找到疑似行道线的精准位置
				{
					for (throNew = max(-halfHeight, thro - deertaThro); throNew < min(thro + deertaThro, halfDiagonalLen); throNew++)
					{
						if (pArr[thetaNew*throSpan + throNew] > pntCount)
						{
							pntCount = pArr[thetaNew*throSpan + throNew];
							thetaReal = thetaNew;
							throReal = throNew;
						}
					}
				}
				theta = thetaReal, thro = throReal;

				if (IsLane(theta, 2 * thro, 70, 0.38, 0.68))		//判断是否为真的行道线
				{
					DrawLine(theta, 2 * thro, 0, m_height - yROI, 2);						//若为真的行道线则划线
																							//cout << theta << " " << 2 * thro << endl;
				}
				theta += deertaTheta;			//移到下一个检测区域
				break;
			}
		}
	}
	//step3.-------------------------释放内存并返回-------------------------//
	delete pAccumArr;
	return;
}

//判断是否为弯道，因判定依据改变，弯道判定在LineJoint()中进行，本函数实际进行分段直线检测
bool KxyLaneDetect::IsCurve()
{
	//初始化每层线段数量
	m_FirLineNum = 0;
	m_SecLineNum = 0;
	m_ThiLineNum = 0;

	m_IndexTop = m_FPntNum - 1;
	m_IndexMid = 0;
	m_IndexBottom = 0;


	//记录每一层的特征点起始下标，便于hough变化
	int i, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE));
	for (i = 0; i < m_FPntNum - 1; i++)
	{
		if ((m_FPntYROI[i] + yROI >= m_BottomY) && (m_FPntYROI[i + 1] + yROI<m_BottomY))
		{
			m_IndexBottom = i;
		}
		if (m_FPntYROI[i] + yROI >= m_MidY && m_FPntYROI[i + 1] + yROI<m_MidY)
		{
			m_IndexMid = i;
		}
		if (m_FPntYROI[i] + yROI >= m_TopY && m_FPntYROI[i + 1] + yROI<m_TopY)
		{
			m_IndexTop = i;
		}
	}

	/*memset(m_FirTheta, 0, sizeof(double) * 15);
	memset(m_FirThro, 0, sizeof(double) * 15);
	memset(m_SecTheta, 0, sizeof(double) * 15);
	memset(m_SecThro, 0, sizeof(double) * 15);
	memset(m_ThiTheta, 0, sizeof(double) * 15);
	memset(m_ThiThro, 0, sizeof(double) * 15);
*/

	//每一层hough变化
	//阈值根据分层投影层数来确定
	Hough_improve(m_layerNum / 12,m_IndexMid, m_IndexTop, 0, 1);
	Hough_improve(m_layerNum / 12, m_IndexBottom, m_IndexMid, 1, 2);
	Hough_improve(m_layerNum / 9, 0, m_IndexBottom, 2, 3);


	return 1;
}

//分段中使用的霍夫
void KxyLaneDetect::Hough_improve(int threshold, int startIndex, int endIndex, int color, int layer)
{
	int deertaTheta = 25, deertaThro = max(m_width, m_height) >> 2;	//deertaTheta为检测行道线精确位置的角度θ跨度，deertaThro为ρ跨度
	int tmp;
	int i, j;
	int thro, theta, throNew, thetaNew, throReal, thetaReal;		//极坐标θ(theta)∈[-90,90]，ρ(thro)∈[-height,diagonalLen]
	int pntCount;		//能检测到的特征点数目

	//width为图像即感兴趣区域宽度，yROI为感兴趣区域在原图中的y坐标，height为感兴趣区域的图像高度
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	//diagonalLen为感兴趣区域的图像对角线宽度
	int diagonalLen = sqrt(width*width + height*height);
	int halfHeight = (height >> 1), halfDiagonalLen = (diagonalLen >> 1);

	//缩短thro的跨度为原来的一半，将相邻两个不同的thro归为一类，提高运算速度
	int thetaSpan = 180 + 1, throSpan = halfDiagonalLen + halfHeight + 1;
	int centerIndex = 90 * throSpan + halfHeight;			//中心点坐标

	int AccumArrLen = thetaSpan*throSpan;					//计数数组长度
	int *pAccumArr = new int[AccumArrLen];					//计数数组
	memset(pAccumArr, 0, sizeof(int)*AccumArrLen);
	int *pArr = pAccumArr + centerIndex;					//pArr指向计数数组的中间

	//step1.-------------------------计算获得计数数组-------------------------//
	for (theta = -90; theta <= 90; theta++)
	{
		for (j = startIndex; j < endIndex; j++)
		{
			thro = ((m_FPntXROI[j] * m_cosLUTM4096[theta] + (m_FPntYROI[j]) * m_sinLUTM4096[theta]) >> 13);	//
			pArr[theta*throSpan + thro]++;
		}
	}
	//step2.-------------------------根据计数数组找到直线，并划线-------------------------//
	for (theta = -89; theta <= 90; theta++)
	{
		for (thro = -halfHeight; thro <= halfDiagonalLen; thro++)
		{
			pntCount = pArr[theta*throSpan + thro];
			if (pntCount >= threshold)										//找到能穿过一定数目（threshold）特征点的疑似行道线
			{
				//printf("found!\n");
				thetaReal = thetaNew = theta, throReal = throNew = thro;
				for (thetaNew = theta; thetaNew < min(90, theta + deertaTheta); thetaNew++)	//找到疑似行道线的精准位置
				{
					for (throNew = max(-halfHeight, thro - deertaThro); throNew < min(thro + deertaThro, halfDiagonalLen); throNew++)
					{
						if (pArr[thetaNew*throSpan + throNew] > pntCount)
						{
							pntCount = pArr[thetaNew*throSpan + throNew];
							thetaReal = thetaNew;
							throReal = throNew;
						}
					}
				}
				theta = thetaReal, thro = throReal;



				//判断是否为真的行道线
				if (IsLane_improve(theta, 2 * thro, 70, 0.38, 0.68))
				{
					switch (layer)
					{
					case 1:
						m_FirLineNum++;
						m_FirTheta[m_FirLineNum] = theta;
						m_FirThro[m_FirLineNum] = 2 * thro;
						break;
					case 2:
						m_SecLineNum++;
						m_SecTheta[m_SecLineNum] = theta;
						m_SecThro[m_SecLineNum] = 2 * thro;
						break;
					case 3:
						m_ThiLineNum++;
						m_ThiTheta[m_ThiLineNum] = theta;
						m_ThiThro[m_ThiLineNum] = 2 * thro;
						break;
					}


					//	DrawLine(theta, 2 * thro, m_FPntYROI[endIndex], m_FPntYROI[startIndex],color);						//若为真的行道线则划线
					//cout << theta << " " << 2 * thro << endl;
				}



				theta += deertaTheta;			//移到下一个检测区域
				break;
			}
		}
	}
	//step3.-------------------------释放内存并返回-------------------------//
	delete pAccumArr;
	return;

}

void KxyLaneDetect::LineJoint()
{
	//------------------------------------------------初始化
	m_curveNum = 0;
	memset(m_curve, 0, sizeof(int) * 15);
	int i, j, k;
	int x1, x2b, x2t, x3;
	//width为图像即感兴趣区域宽度，yROI为感兴趣区域在原图中的y坐标，height为感兴趣区域的图像高度
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	int MidY, BottomY;
	MidY = m_MidY - yROI, BottomY = m_BottomY - yROI;
	double rad = 1.0 / 180 * 3.14159;//弧度
	double Tan, theta, thro;
	int temp;
	bool already;
	int top, mid, bot;
	int thetaDifSum;//三层角度差累加和

	//------------------------------------------从中间层往下再往上线段拼接
	for (i = 1; i <= m_SecLineNum; i++)
	{
		theta = m_SecTheta[i];
		thro = m_SecThro[i];
		Tan = tan(theta * rad);
		temp = thro / cos(theta*rad);
		x1 = temp - int(BottomY*Tan);
		already = false;
		//中间层和底层拼接
		for (j = 1; j <= m_ThiLineNum; j++)
		{
			x2b = m_ThiThro[j] / cos(m_ThiTheta[j] * rad) - int(BottomY*tan(m_ThiTheta[j] * rad));
			//如果中间层线段和底层线段拼接成功
			if (std::abs(x2b - x1) <= m_maxDetectWidth * 1.5)//相差m_maxDetectWidth * 1.5距离的课融为一条折线
			{
				m_curveNum++;
				already = true;
				m_curve[m_curveNum] = j * 100 + i * 10;

				//继续顶层和中间层拼接
				x2t = m_SecThro[i] / cos(m_SecTheta[i] * rad) - int(MidY*tan(m_SecTheta[i] * rad));
				for (k = 1; k <= m_FirLineNum; k++)
				{
					x3 = m_FirThro[k] / cos(m_FirTheta[k] * rad) - int(MidY*tan(m_FirTheta[k] * rad));
					if (std::abs(x3 - x2t) <= m_maxDetectWidth * 1.5)
					{
						m_curve[m_curveNum] = m_curve[m_curveNum] - m_curve[m_curveNum] % 10 + k;
					}
				}
			}	
		}

		//如果中间层线段没有和底层拼接未成功
		if (already == false)
		{
			x2t = m_SecThro[i] / cos(m_SecTheta[i] * rad) - int(MidY*tan(m_SecTheta[i] * rad));
			for (k = 1; k <= m_FirLineNum; k++)
			{
				x3 = m_FirThro[k] / cos(m_FirTheta[k] * rad) - int(MidY*tan(m_FirTheta[k] * rad));
				if (std::abs(x3 - x2t) <= m_maxDetectWidth * 1)
				{
					m_curveNum++;
					m_curve[m_curveNum] = i * 10 + k;
				}
			}
		}
		
	}




	///从底层往上拼接线段
	//for (i = 1; i <= m_ThiLineNum; i++)
	//{
	//	theta = m_ThiTheta[i];
	//	thro = m_ThiThro[i];
	//	Tan = tan(theta * rad);
	//	temp = thro / cos(theta*rad);
	//	x1 = temp - int(BottomY*Tan);
	//	for (j = 1; j <= m_SecLineNum; j++)
	//	{
	//		x2b = m_SecThro[j] / cos(m_SecTheta[j] * rad) - int(BottomY*tan(m_SecTheta[j] * rad));
	//		if (std::abs(x2b - x1) <= m_maxDetectWidth * 1.5)//相差m_maxDetectWidth * 1.5距离的课融为一条折线
	//		{
	//			m_curveNum++;
	//			m_curve[m_curveNum] = i * 100 + j * 10;
	//			x2t = m_SecThro[j] / cos(m_SecTheta[j] * rad) - int(MidY*tan(m_SecTheta[j] * rad));
	//			for (k = 1; k <= m_FirLineNum; k++)
	//			{
	//				x3 = m_FirThro[k] / cos(m_FirTheta[k] * rad) - int(MidY*tan(m_FirTheta[k] * rad));
	//				if (std::abs(x3 - x2t) <= m_maxDetectWidth * 1.5)
	//				{
	//					m_curve[m_curveNum] = m_curve[m_curveNum] - m_curve[m_curveNum] % 10 + k;
	//				}
	//			}
	//		}
	//	}
	//}





	//----------------------判断直道还是弯道-------------------//
	for (i = 1; i <= m_curveNum; i++)
	{
		thetaDifSum = 0;
		bot = m_curve[i] / 100;
		mid = (m_curve[i] % 100) / 10;
		top = m_curve[i] % 10;

		//如果顶层底层没有线段，则差值为零
		thetaDifSum += (m_ThiTheta[bot] == 0 ? 0 : abs(m_SecTheta[mid] - m_ThiTheta[bot]));
		thetaDifSum += (m_FirTheta[top] == 0 ? 0 : abs(m_SecTheta[mid] - m_FirTheta[top]));

		//判据,角度累加和大于15度
		if (thetaDifSum>15||top==0)
		{
			m_isCurve = true;
			break;
		}
	}



	//测试，待删
	/*int bot, mid, top;
	for (i = 1; i <= m_curveNum; i++)
	{
	bot = m_curve[i] / 100;
	mid = (m_curve[i] % 100) / 10;
	top = m_curve[i] % 10;
	DrawLine(m_ThiTheta[bot], m_ThiThro[bot], m_BottomY-yROI, m_height-yROI, i % 3);
	DrawLine(m_SecTheta[mid], m_SecThro[mid], m_MidY-yROI, m_BottomY-yROI, i % 3);
	DrawLine(m_FirTheta[top], m_FirThro[top], yROI-yROI, m_MidY-yROI, i % 3);
	}*/

}


void KxyLaneDetect::CurveFitting()
{
	int i, j, k, X, Y;
	int bot, mid, top;
	int t,tem;
	bool already;
	int drawstartY,drawendY;
	int H = m_height - m_tY;
	int yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE));
	double rad = 1.0 / 180 * 3.14159;//弧度
	int FPntNum = 0;
	double FPntCurveX[15] = { 0 };

	//double FPntCurveY[15] = { H*0.93, H*0.87, H*0.81, H*0.75, H*0.69,
	//						  H*0.575,H*0.52, H*0.465,H*0.41,H*0.355,
	//						  H*0.25, H*0.20, H*0.15, H*0.10, H*0.05, };

	double FPntCurveY[15] = { H*0.95, H*0.90, H*0.80, H*0.70, H*0.60,
							H*0.50, H*0.45, H*0.40, H*0.35, H*0.30,
							H*0.25, H*0.20, H*0.15, H*0.10, H*0.05, };

	double *FX;
	double *FY;
	int n = 0;
	double poly[5];
	int *x, *y;

	//划折线
	/*for (i = 1; i <= m_curveNum; i++)
	{
	bot = m_curve[i] / 100;
	mid = (m_curve[i] % 100) / 10;
	top = m_curve[i] % 10;
	DrawLine(m_ThiTheta[bot], m_ThiThro[bot], 417-314, 479-314, i % 3);
	DrawLine(m_SecTheta[mid], m_SecThro[mid], 362-314, 417-314, i % 3);
	DrawLine(m_FirTheta[top], m_FirThro[top], 314-314, 362-314, i % 3);
	}*/

	for (i = 1; i <= m_curveNum; i++)
	{
		already = 0;
		FPntNum = 0;
		memset(FPntCurveX, 0, sizeof(int) * 15);
		bot = m_curve[i] / 100;
		mid = (m_curve[i] % 100) / 10;
		top = m_curve[i] % 10;

	//	memset(FPntCurveX, 0, sizeof(double) * 15);

		//如果一条折线与之前的折线有两层及以上的相同，则舍弃
		for (t = 1; t <= i-1; t++)
		{
			tem = 0;
			tem += (((m_curve[t] / 100) - bot) == 0 ? 1 : 0);//底层相同？1：0
			tem += (((m_curve[t] % 100) - mid) == 0 ? 1 : 0);
			tem += (((m_curve[t] % 10) - top) == 0 ? 1 : 0);

			//如果有两层及以上相同
			if (tem >= 2)
			{
				already = 1;
				break;
			}
		}
		//如果有两层及以上相同
		if (already)
		{
			continue;
		}

		
		for (j = 0; j < 5; j++)
		{
			Y = FPntCurveY[j];
			if (bot != 0) {
				FPntNum += 1;
				FPntCurveX[j] = m_ThiThro[bot] / cos(m_ThiTheta[bot] * rad) - (Y*tan(m_ThiTheta[bot] * rad));
			}
		}
		for (j = 5; j < 10; j++)
		{
			Y = FPntCurveY[j];
			if (mid != 0) {
				FPntNum += 1;
				FPntCurveX[j] = m_SecThro[mid] / cos(m_SecTheta[mid] * rad) - (Y*tan(m_SecTheta[mid] * rad));
			}
		}
		for (j = 10; j < 15; j++)
		{
			Y = FPntCurveY[j];
			if (top != 0) {
				FPntNum += 1;
				FPntCurveX[j] = m_FirThro[top] / cos(m_FirTheta[top] * rad) - (Y*tan(m_FirTheta[top] * rad));
			}
		}

		//


		/*==================polyfit(n,x,y,poly_n,a)===================*/
		/*=======拟合y=a0+a1*x+a2*x^2+……+apoly_n*x^poly_n========*/
		/*=====n是数据个数 xy是数据值 poly_n是多项式的项数======*/
		/*===返回a0,a1,a2,……a[poly_n]，系数比项数多一（常数项）=====*/
		memset(poly, 0, sizeof(double) * 4);
		//如果底层为零，则移动特征点头指针指向中间层
		
		FX = FPntCurveX;
		FY = FPntCurveY;

		if (bot == 0)
		{
			FX = FPntCurveX + 5;
			FY = FPntCurveY + 5;
		}
		//注意！！！X，Y坐标互换
		polyfit(FPntNum, FY, FX, 3, poly);

		//画线
		BYTE *pRGBROI = m_pResImg + yROI*m_width * 3;
		BYTE *pCur;
		
		//如果顶层没有，则只画下两层
		if (top == 0) {
			drawstartY = m_MidY-yROI;
		}
		else {
			drawstartY = 0;
		}

		//如果底层没有，则只画上两层
		if (bot == 0) {
			drawendY = m_BottomY - yROI;
		}
		else {
			drawendY = m_height - yROI;
		}

		for (k = drawstartY; k < drawendY; k++)
		{
			j = poly[0] + poly[1] * k + poly[2] * k*k + poly[3] * k*k*k;

			if (j >= m_width || j < 0)
				continue;
			pCur = pRGBROI + (k * m_width + j) * 3;
			*pCur = 255;
			*(pCur + 1) = 0;
			*(pCur + 2) = 255;
			//*(pCur + 2 % 3) = 255;//

		}

	}

	char filename[100];
	sprintf(filename, "G:\\曲线拟合2次\\new%06d %d条.bmp", m_nFrameID, m_curveNum);
	//RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);

	sprintf(filename, "G:\\TestImage_draft\\pointnew%06d %d条.bmp", m_nFrameID, m_curveNum);
	//RmwWrite24BitImg2BmpFile(m_pResImg, m_width, m_height, filename);



}



////从外界复制24位彩色图
//void KxyLaneDetect::CopyRGBImage(BYTE *pRGBImg)
//{
//	memcpy(m_pRGBImg, pRGBImg, m_memSize * 3);
//}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//外界函数
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//计算整形数组最大最小值
void CalIntArrayMaxMin(int a[], int n, int &max, int &min)
{
	min = max = a[0];
	int i;
	for (i = 0; i < n; i++)
	{
		if (a[i] > max)
			max = a[i];
		if (a[i] < min)
			min = a[i];
	}
}

//计算整形数组的和与平均值
void CalIntArraySumAvg(int a[], int n, int &sum, double &avg)
{
	int i;
	sum = 0;
	for (i = 0; i < n; i++)
		sum += a[i];
	avg = 1.0*sum / n;
}

//向量（数组）滤波，用渐进法处理了边缘	//his:原向量	hisLen:向量长度	 filterLen:滤波器大小	
void HistogramAvgFilter(int *his, int hisLen, int filterLen, int *res)
{
	//转化filterLen为奇数
	if (filterLen % 2 == 0)
		filterLen++;

	int i, j;
	int sum, avg;
	int *pre, *next;
	int halfFilterLen = filterLen / 2;

	int edgeFilterLen, edgeHalfFilterLen, edgeSum, edgeAvg, edgeRight, edgeLeft;
	// step.1-----------------计算左边缘--------------------//
	edgeRight = 1;
	edgeFilterLen = 1;
	edgeHalfFilterLen = 0;
	edgeSum = res[0] = his[0];
	for (i = 1; i < halfFilterLen; i++)
	{
		edgeSum = edgeSum + his[edgeRight] + his[edgeRight + 1];

		edgeRight += 2;
		edgeFilterLen += 2;
		/*edgeHalfFilterLen += 1;*/

		res[i] /*= edgeAvg*/ = edgeSum / edgeFilterLen;
	}

	// step.2-----------------计算第一个可以滤波的元素--------------------//
	sum = edgeSum + his[edgeRight] + his[edgeRight + 1 + 1];
	res[halfFilterLen] = avg = sum / filterLen;
	pre = &his[0];
	next = &his[filterLen];

	// step.3-----------------中间处理--------------------//
	for (i = halfFilterLen + 1; i < hisLen - halfFilterLen; i++)
	{


		sum = sum - *pre + *next;
		avg = sum / filterLen;
		res[i] = avg;

		pre++, next++;
	}

	// step.4-----------------计算右边缘--------------------//
	edgeLeft = hisLen - halfFilterLen - 1 - halfFilterLen;
	edgeFilterLen = filterLen;
	edgeHalfFilterLen = halfFilterLen;
	edgeSum = sum;

	for (i = hisLen - halfFilterLen; i < hisLen; i++)
	{
		edgeSum = edgeSum - his[edgeLeft] - his[edgeLeft + 1];

		edgeLeft += 2;
		edgeFilterLen -= 2;
		/*edgeHalfFilterLen -= 1;*/

		res[i] /*= edgeAvg*/ = edgeSum / edgeFilterLen;
	}

	// step.5-----------------返回--------------------//
	return;

}

//向量求差	a-b=res
void CalIntArrayDifference(int *a, int *b, int len, int *res)
{
	int i;
	for (i = 0; i < len; i++)
	{
		res[i] = a[i] - b[i];
	}
}

/*==================polyfit(n,x,y,poly_n,a)===================*/
/*=======拟合y=a0+a1*x+a2*x^2+……+apoly_n*x^poly_n========*/
/*=====n是数据个数 xy是数据值 poly_n是多项式的项数======*/
/*===返回a0,a1,a2,……a[poly_n]，系数比项数多一（常数项）=====*/
void polyfit(int n, double x[], double y[], int poly_n, double p[])
{
	int i, j;
	double *tempx, *tempy, *sumxx, *sumxy, *ata;

	tempx = (double *)calloc(n, sizeof(double));
	sumxx = (double *)calloc((poly_n * 2 + 1), sizeof(double));
	tempy = (double *)calloc(n, sizeof(double));
	sumxy = (double *)calloc((poly_n + 1), sizeof(double));
	ata = (double *)calloc((poly_n + 1)*(poly_n + 1), sizeof(double));
	for (i = 0; i<n; i++)
	{
		tempx[i] = 1;
		tempy[i] = y[i];
	}
	for (i = 0; i<2 * poly_n + 1; i++)
	{
		for (sumxx[i] = 0, j = 0; j<n; j++)
		{
			sumxx[i] += tempx[j];
			tempx[j] *= x[j];
		}
	}

	for (i = 0; i<poly_n + 1; i++)
	{
		for (sumxy[i] = 0, j = 0; j<n; j++)
		{
			sumxy[i] += tempy[j];
			tempy[j] *= x[j];
		}
	}

	for (i = 0; i<poly_n + 1; i++)
	{
		for (j = 0; j<poly_n + 1; j++)
		{
			ata[i*(poly_n + 1) + j] = sumxx[i + j];
		}
	}
	gauss_solve(poly_n + 1, ata, p, sumxy);

	free(tempx);
	free(sumxx);
	free(tempy);
	free(sumxy);
	free(ata);
}

/*============================================================*/
////    高斯消元法 
////    n: 系数的个数  
////    ata: 线性矩阵  
////    sumxy: 线性方程组的Y值  
////    p: 返回拟合的结果  
/*============================================================*/
void gauss_solve(int n, double A[], double x[], double b[])
{
	int i, j, k, r;
	double max;
	for (k = 0; k<n - 1; k++)
	{
		max = fabs(A[k*n + k]);                 // find maxmum   
		r = k;
		for (i = k + 1; i<n - 1; i++)
		{
			if (max<fabs(A[i*n + i]))
			{
				max = fabs(A[i*n + i]);
				r = i;
			}
		}
		if (r != k)
		{
			for (i = 0; i<n; i++)        //change array:A[k]&A[r]  
			{
				max = A[k*n + i];
				A[k*n + i] = A[r*n + i];
				A[r*n + i] = max;
			}

			max = b[k];                    //change array:b[k]&b[r]  
			b[k] = b[r];
			b[r] = max;
		}

		for (i = k + 1; i<n; i++)
		{
			for (j = k + 1; j<n; j++)
				A[i*n + j] -= A[i*n + k] * A[k*n + j] / A[k*n + k];
			b[i] -= A[i*n + k] * b[k] / A[k*n + k];
		}
	}

	for (i = n - 1; i >= 0; x[i] /= A[i*n + i], i--)
	{
		for (j = i + 1, x[i] = b[i]; j<n; j++)
			x[i] -= A[i*n + j] * x[j];
	}

}

