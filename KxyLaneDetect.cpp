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
// ����/��������
//
////////////////////////////////////////////////////////////////////////////////////////////////////

KxyLaneDetect::KxyLaneDetect()
{  	//<�ڹ��캯���а����еĳ�Ա��������ֵ>
	//�Ƿ��ʼ���ɹ�
	m_isInitOK = false;
	//ͼ������
	m_isRGB = false;
	m_width = 0;
	m_height = 0;
	//�ڴ�ռ�
	m_pRGBImg = NULL;
	m_pGryImg = NULL;
	m_pResImg = NULL;
	m_pSumImg = NULL;
	m_pTemImg = NULL;
	m_prjHist = NULL;
	m_prjTmpH = NULL;
	m_memSize = 0; //��������ڴ��ֽ���
				   //����Ȥ����ָ��
	m_pROI = NULL;
	m_FPntNum = 0;
	//�ڲ�������
	m_nFrameID = 0;
	//����sin��cos����4096�����α�
	CalSinLUTM4096();
	CalCosLUTM4096();
	//�����־
	m_isCurve = false;
	//�������߶���Ϣ��¼
	
	m_FirLineNum = 0;

	
	m_SecLineNum = 0;

	
	m_ThiLineNum = 0;

	//���߸���
	m_curve = NULL;
	m_curveNum = 0;

	//��������
	m_tY = 0;
	m_tlX = 0;//tl��topleft	tr��topright
	m_trX = 0;
	m_bY = 0;
	m_blX = 0;//bl��bottomleft br��bottomright
	m_brX = 0;
	m_maxDetectWidth = 0;//ͼ��ײ����е��߿��
	m_layerNum = 0;//�ֲ�ͶӰ����
}

KxyLaneDetect::~KxyLaneDetect()
{	//<�������������ͷ����е��ڴ�>
	Dump();
}

void KxyLaneDetect::Dump()
{   //<дһ��ר�ŵ������ͷ��ڴ�ĺ���,��Ϊ�ڴ���ж�ε��������ͷ�>
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
	////ɾ��sin��cos����4096�����α�
	//delete m_sinLUT;
	//delete m_cosLUT;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ��ʼ��
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool KxyLaneDetect::Initialize(bool isRGB, int width, int height)
{   //<�ڳ�ʼ�������ж����еĳ�Ա��������ֵ>
	//<���ڳ�ʼ�������������ڴ�,��Ϊ��ʼ���������ᱻƵ������>
	//<��ʼ���������Ա���ε���>
	//<�ڴ�����ʱ,���Ѿ�������ڴ���ڱ�����Ҫ���ڴ�,���ٴ�����>

	//step.1------ͼ������--------------------------//
	m_isRGB = isRGB;
	m_width = width;
	m_height = height;
	//step.2------�ڴ�����--------------------------//
	if (m_width*m_height > m_memSize)
	{
		//���ͷ�
		Dump();
		//������
		m_pRGBImg = new BYTE[m_width*m_height * 3]; //��ɫͼ��
		m_pGryImg = new BYTE[m_width*m_height];  //�Ҷ�ͼ��
		m_pTemImg = new BYTE[m_width*m_height];
		m_pResImg = new BYTE[m_width*m_height * 3]; //���ͼ��
		m_pSumImg = new int[m_width*m_height];//����ͼ
		m_FPntXROI.resize(200);
		m_FPntYROI.resize(200);//����������
		m_prjHist = new int[m_width];//�ֲ�ͶӰֱ��ͼ
									 //��¼����Ĵ�С
		m_prjTmpH = new int[m_width];
		m_memSize = m_width*m_height;
		//����Ȥ�����°벿�֣�  m_pROIΪ����Ȥ�����ͷָ��
		//m_pROI = m_pGryImg + (m_height >> 1)*m_width;
		//m_pROI = m_pGryImg + (m_height *56/100)*m_width;//��268�У�������0�У�

		//�������߶���Ϣ
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
	//step.3------��ʼ���ɹ���־--------------------//
	m_isInitOK = (m_pRGBImg && m_pGryImg  && m_pResImg && m_pSumImg);
	return m_isInitOK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ִ��
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//(������ͼ��ָ�룬��·���򶥲�y���꣬��·��������x����·��������x����·����ײ�y����·��������x����·��������x��ͼ��ײ��е��߿�ȣ�
bool KxyLaneDetect::DoNext(BYTE *pRGBImg, int tY, int tlX, int trX, int bY, int blX, int brX, int maxDetectWidth, int minDetectWidth, int contrast)
{
	// step.0----------------------------------�жϳ�ʼ���Ƿ�ɹ����������������----------------------------------------------//
	if (!m_isInitOK) return false;
	m_tY = tY;
	m_tlX = tlX, m_trX = trX;//tl��topleft	tr��topright
	m_bY = bY;
	m_blX = blX, m_brX = brX;//bl��bottomleft br��bottomright
	m_maxDetectWidth = maxDetectWidth;//ͼ��ײ����е��߿��
	m_minDetectWidth = minDetectWidth;
	//m_pROI = m_pGryImg + (m_height * 56 / 100)*m_width;//��268�У�������0�У�
	m_pROI = m_pGryImg + tY*m_width;//����Ȥ��������ȷ��
	m_layerNum = (m_bY - m_tY) / 3;
	m_contrast = contrast;
	m_TopY = tY + 3;
	m_MidY = tY + (bY - tY)*0.25;///0.25//0.3
	m_BottomY = tY + (bY - tY)*0.52;///0.52//0.63

	// step.1----------------------------------���β�ͼ���Ƶ�m_pRGBImg��-----------------------------------------------------//
	memcpy(m_pRGBImg, pRGBImg, 3 * m_memSize);
	memcpy(m_pResImg, m_pRGBImg, 3 * m_memSize);

	// step.2----------------------------------�ò�ɫͼ�����ͨ��ֵ��Ϊ�Ҷ�ֵ�õ�m_pGryImg-------------------------------------//
	CalGreenChannel();
#ifdef DEBUG_MOOD
	/*char filename[100];
	sprintf(filename,"G:\\TestImage\\a%d.bmp", m_nFrameID);
	RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);*/
#endif

	// step.3----------------------------------����m_pGryImg�Ļ���ͼ-----------------------------------------------------------//
	CalGry8bIntegralImage();

	// step.4----------------------------------ͨ���ֲ�ͶӰ����õ�������------------------------------------------------------//
	CalFPoints();

	// step.5----------------------------------�����Σ�����������ʧ���λ�ù�ϵ��֤�Ƿ�Ϊ���----------------------------------//
	m_isCurve = false;
	//IsCurve();
	//LineJoint();

	// step.6----------------------------------�ж��Ƿ�Ϊ���------------------------------------------------------------------//
	if (m_isCurve)
	{
	// step.7----------------------------------�����������ϣ�����------------------------------------------------------------//
		CurveFitting();
	}
	else
	{
	// step.8----------------------------------����ֱ��������------------------------------------------------------------------//
		Hough(m_layerNum>>3);
	}



	//ĳ���ض�֡�ĵ���,����10
	/*if (m_nFrameID==10)
	{
	Debug();
	}*/

	//����
#ifdef DEBUG_MOOD
	Debug();
#endif


	//�ڲ��������ۼ�
	m_nFrameID++;
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ���
//
////////////////////////////////////////////////////////////////////////////////////////////////////

BYTE *KxyLaneDetect::GetResImg()
{   //���ؽ��ͼ��
	return m_pResImg;
}
int KxyLaneDetect::GetResWidth()
{   //���ؽ��ͼ��Ŀ��
	return m_width;
}
int KxyLaneDetect::GetResHeight()
{   //���ؽ��ͼ��ĸ߶�
	return m_height;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ����
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void KxyLaneDetect::Debug()
{   //<������ڲ�����: ��Ļ������ļ������>

	//--debug;

	//�ֲ㼰���������
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


	//�ֲ㻯�߲���

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


	//�����߶�չʾ
	//for(i=1;)


	sprintf(filename, "G:\\TestImage_draft\\gry%06d .bmp", m_nFrameID);
	RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);

	sprintf(filename, "G:\\TestImage_draft\\point%06d.bmp", m_nFrameID);
//	RmwWrite24BitImg2BmpFile(m_pResImg, m_width, m_height, filename);



	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//˽�г�Ա����
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//˽�к���������[-90,90]�����Ǻ�����*4096
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

//˽�к��������24λλͼĳͨ��
void KxyLaneDetect::CalChannelBase(int firstPixel)
{
	/*
	���ĳͨ��
	firstPixel==0:��
	firstPixel==1:��
	firstPixel==2:��
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
//���г�Ա����
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//�����ͨ��
void KxyLaneDetect::CalBlueChannel()
{
	CalChannelBase(0);
}
//�����ͨ��
void KxyLaneDetect::CalGreenChannel()
{
	CalChannelBase(1);
}
//��ú�ͨ��
void KxyLaneDetect::CalRedChannel()
{
	CalChannelBase(2);
}

//����8λ�Ҷ�ͼ�����ͼ	ͼ�����ظ������ô���2^24=16777216Լ1600������
void KxyLaneDetect::CalGry8bIntegralImage()
{
	BYTE *pGry;
	int *pSum;
	int x, y;

	// step.1-----------------�����һ��--------------------//
	pGry = m_pGryImg;
	pSum = m_pSumImg;
	*(pSum++) = *(pGry++);		//��һ��Ԫ��
	for (x = 1; x < m_width; x++) *(pSum++) = *(pSum - 1) + (*(pGry++));
	// step.2-----------------����������--------------------//
	for (y = 1; y < m_height; y++)
	{
		*(pSum++) = *(pSum - m_width) + *(pGry++); //����ÿ�е�һ��
		for (x = 1; x < m_width; x++) //����������
		{
			*(pSum++) = *(pGry++)
				+ (*(pSum - 1))
				+ (*(pSum - m_width))
				- (*(pSum - 1 - m_width));
		}
	}
	// step.3-----------------�����뷵��--------------------//
	return;
}

//�ֲ�ͶӰrowNUM�㣬��ȡ���������꣬x/y����ֱ�洢��featureX/featureY��;	//maxDetectWidth���ײ��е��߿��
//BottomYMidYTopY�ֱ�Ϊ���������㻮�ֵ�Y���꣨ԭͼ���꣩��������������ڼ�¼������������±꣬�Ա�Hough�任ʱ���ñ���
void KxyLaneDetect::CalFPoints()
{
	int x, y, yUp;
	int i, j, k;
	int  *rowfirst, *rowend, *pCur;

	//widthΪͼ�񼴸���Ȥ�����ȣ�yROIΪ����Ȥ������ԭͼ�е�y���꣬heightΪ����Ȥ�����ͼ��߶�
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	//row_per_layer��rowPerLayerΪ�ֲ�ͶӰʱÿ���������deertaRowΪ���ڲ���������
	//double row, row_per_layer = 2.0 *height / m_layerNum, deertaRow = 2.0 *height / (m_layerNum*(m_layerNum - 1));
#ifdef FIR_KIND_OF_WAY_LAYER
	double row, row_per_layer = 2.0 *height / m_layerNum, deertaRow = 1.5 *height / (m_layerNum*(m_layerNum - 1));//�����м�һ���x�У�����һ��������һ��ֱ�Ϊ0.5x��2x�У�ͨ����������˳�ֵ��
																												  //�����У�row_per_layer=6,deertaRow=0.0655
	double detect_Width, deertaDetectWidth = 1.0*m_maxDetectWidth / m_layerNum;//detect_Width��detectWidthΪ����е��߻���Ŀ�ȣ�deertaDetectWidthΪ���ڲ����е��߻���Ŀ�Ȳ�

#elif defined SEC_KIND_OF_WAY_LAYER
	double row, row_per_layer = 8.0 *height / (m_layerNum * 5), deertaRow = 1.5 *row_per_layer / (2 * (m_layerNum - 1));//�����м�һ���x�У�����һ��������һ��ֱ�Ϊ0.5x��2x�У�ͨ����������˳�ֵ��
																														//�����У�row_per_layer=4.845,deertaRow=0.05267
	double detect_Width, deertaDetectWidth = 1.0*m_maxDetectWidth / m_layerNum;//detect_Width��detectWidthΪ����е��߻���Ŀ�ȣ�deertaDetectWidthΪ���ڲ����е��߻���Ŀ�Ȳ�

#elif defined Thi_KIND_OF_WAY_LAYER


	double row, row_per_layer = 8.0 *height / (m_layerNum * 5), deertaRow = 1.5 *row_per_layer / (2 * (m_layerNum - 1));//�����м�һ���x�У�����һ��������һ��ֱ�Ϊ0.5x��2x�У�ͨ����������˳�ֵ��
	double detect_Width, deertaDetectWidth = 1.0*(m_maxDetectWidth - m_minDetectWidth) / (m_layerNum - 1);//detect_Width��detectWidthΪ����е��߻���Ŀ�ȣ�deertaDetectWidthΪ���ڲ����е��߻���Ŀ�Ȳ�

#else																								  

	double row, row_per_layer = 5, deertaRow = 4.0 / (m_layerNum - 1);//�����м�һ���x�У�����һ��������һ��ֱ�Ϊ0.5x��2x�У�ͨ����������˳�ֵ��
	double detect_Width = m_maxDetectWidth, deertaDetectWidth = 1.0*(m_maxDetectWidth - m_minDetectWidth) / (m_layerNum - 1);//detect_Width��detectWidthΪ����е��߻���Ŀ�ȣ�deertaDetectWidthΪ���ڲ����е��߻���Ŀ�Ȳ�

#endif																								 

	int rowPerLayer = row_per_layer;
	//	double detect_Width, deertaDetectWidth = 1.0*m_maxDetectWidth / m_layerNum;//detect_Width��detectWidthΪ����е��߻���Ŀ�ȣ�deertaDetectWidthΪ���ڲ����е��߻���Ŀ�Ȳ�
	//	double slope = (m_trX - m_tlX)*1.0 / (m_brX - m_blX);
	int detectWidth;

	//fPntIndexΪ��������±꣬roadWidthΪ��⵽�Ŀ��ܵ��е��߿�ȣ�roadHeightΪ��⵽�Ŀ��ܵ��е�����ͶӰ��ֱֵ��ͼ�еĲ���߶�ֵ
	//roadRightΪ���ܵ��е����Ҳ�X����
	int fPntIndex = 0;
	int roadWidth, roadHeight;
	int roadRight;
	//flagΪ�Ƿ�Ϊ�������е��ߵı�־
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

	//-------------------------��ʼ�е��߼��-------------------------//
	fPntIndex = 0;
	m_FPntNum = 0;
	//fPntIndex = m_FPntNum;
	detect_Width = m_maxDetectWidth, detectWidth = detect_Width;
	for (row = y = m_height - 1; y > yROI; )		//���µ��Ϸֲ�ͶӰ
	{
		memset(m_prjHist, 0, width * sizeof(int));
		memset(m_prjTmpH, 0, width * sizeof(int));

		//step1.-------------------------����һάͶӰͼM1-------------------------//
		pCur = m_pSumImg + y*width;
		//yUp = y - row_per_layer;
		yUp = row - row_per_layer;
		rowfirst = m_pSumImg + yUp*width;
		//m_prjHist[0] = *pCur - *rowfirst;
		for (j = 1; j < width - 1; j++)
			m_prjHist[j] = *(pCur + j) - *(pCur + j - 1) - (*(rowfirst + j) - *(rowfirst + j - 1));
		//��Ե��
		m_prjHist[0] = m_prjHist[1], m_prjHist[width - 1] = m_prjHist[width - 2];

		//step2.-------------------------һάͶӰͼM1��ֵ�˲�ƽ���õ�M2-------------------------//
		HistogramAvgFilter(m_prjHist, width, int(2 * detect_Width), m_prjTmpH);
		//step3.-------------------------����M1,M2��ֵM-------------------------//
		CalIntArrayDifference(m_prjHist, m_prjTmpH, width, m_prjHist);

		//step4.-------------------------�����ȡM�е�������-------------------------//
		halfDetectWidth = (detectWidth >> 1);
		for (i = 0; i < width; /*i++*/)
		{
			if (m_prjHist[i] <= 0)	//�ҵ�M��һ���Ǹ���
			{
				i++;
				continue;
			}

			for (j = i + 1, roadHeight = -99999999; j < width; j++)	//�ҵ�һ������Ͳ���ĸ߶�
			{
				if (m_prjHist[j] < 0)
					break;
				if (m_prjHist[j] > roadHeight)
					roadHeight = m_prjHist[j];
			}

			roadWidth = j - i;

			//if (!(halfDetectWidth < roadWidth&&roadWidth < halfDetectWidth * 3))	//�����Ȳ�����̫����ȥ
			//	//if (!(detectWidth*3/4 < roadWidth&&roadWidth < detectWidth * 5 / 4))	//�����Ȳ�����̫����ȥ
			//{
			//	i = j + 1;	//��С���Ӷ�
			//	continue;
			//}
			if (roadHeight < roadWidth * m_contrast)			//����б�ʲ�������ȥ
			{
				i = j + 1;
				continue;
			}

			//��Ⲩ�����ҵĲ���
			for (flag = true, k = max(0, i - halfDetectWidth); k < i; k++)//��Ⲩ����ߵĲ���
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
			for (flag = true, k = j; k < min(width, j + halfDetectWidth); k++)//��Ⲩ���ұߵĲ���
				if (m_prjHist[k] > 0)
				{
					flag = false;
					break;
				}
			if (flag == false)
			{
				i = k;	//��С���Ӷ�
				continue;
			}
			roadRight = k;
			//��Ⲩ�����ҵĲ������

			//��¼�����������꣬����m_FPntXROI��m_FPntYROI�У�������m_FPntNum��fPntIndex
			if (fPntIndex >= m_FPntXROI.size())	//��չ������С//���ռ������һ�ο�һ��
			{
				m_FPntXROI.resize(2 * m_FPntXROI.size());
				m_FPntYROI.resize(2 * m_FPntYROI.size());
			}
			//���е����е�Ϊ������
			m_FPntXROI[fPntIndex] = ((i + j) >> 1);
			m_FPntYROI[fPntIndex] = ((y + yUp) >> 1) - yROI;//������Y����Ϊ����Ȥ�����е�Y����
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
			//	if (fPntIndex >= m_FPntXROI.size())	//��չ������С
			//	{
			//		m_FPntXROI.resize(2 * m_FPntXROI.size());
			//		m_FPntYROI.resize(2 * m_FPntYROI.size());
			//	}
			//	m_FPntXROI[fPntIndex] = k;
			//	m_FPntYROI[fPntIndex] = ((y + yUp) >> 1)- yROI;
			//	m_FPntNum++;
			//	fPntIndex++;
			//}

			//ƫ��i��Ѱ���¸�����
			i = roadRight;
			//i = k;
			//i += (detectWidth - 1);
		}

		//�Ƶ���һ��ͶӰ����
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

//���ߵ�m_pResImg	//theta��thro��ֱ�߼��������
//ע�⣬�˴�TopY��BottomYΪROI�е����꣬�м�
void KxyLaneDetect::DrawLine(int theta, int thro, int TopY, int BottomY, int color)
{
	int i, j;
	double rad = 1.0 / 180 * 3.14159;//����

									 //widthΪͼ�񼴸���Ȥ�����ȣ�yROIΪ����Ȥ������ԭͼ�е�y���꣬heightΪ����Ȥ�����ͼ��߶�
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;

	//x=thro/cos��theta��+ y*sin��theta��		ʹtemp=thro/cos��theta��������x=temp + y*tan��theta��
	double Tan = tan(theta*rad);
	int temp = thro / cos(theta*rad);
	//pRGBROIָ���ɫͼ��ĸ���Ȥ����
	BYTE *pRGBROI = m_pResImg + yROI*width * 3;
	BYTE *pCur;

	//����ÿ��y���꣬������ֱ���ϵ�x���꣬���õ��ɫ
	for (i = TopY; i < BottomY; i++)
	{
		j = temp - int(i*Tan);
		//�����곬��ͼ��Χ����continue
		if (j >= width || j < 0)
			continue;
		//��ֱ���ϵĵ��ɫ
		pCur = pRGBROI + (i*width + j) * 3;
		*pCur = 0;
		*(pCur + 1) = 0;
		*(pCur + 2) = 0;
		*(pCur + color) = 255;

		//--------------���ֵ�------------//
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

// �ж��Ƿ�Ϊ����е��ߣ���ɸѡ����ȷ�е���	//�β�  theta��thro��ֱ�߼��������		angleRange:�е���theta�ķ�Χ	leftBorderRatio\rightBorderRatio��[0,1]:�е�����ʧ�߷�Χ������ֵ��
bool KxyLaneDetect::IsLane(int theta, int thro, unsigned int  angleRange, double leftBorderRatio, double rightBorderRatio)
{
	int tmp, i;
	int width = m_width,
		yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)),
		height = m_height - yROI;
	int topY, bottomY, topX, bottomX;

	//step1.-------------------------�жϸ�ֱ��б���Ƿ����е��ߵĿɽ��ܷ�Χ��-------------------------//
	if (theta<-int(angleRange) || theta>int(angleRange))
		return false;

	//step2.-------------------------�жϸ�ֱ���Ƿ񾭹��е�����ʧ����--------------------------------//
	tmp = width*cos(theta*3.14159 / 180);
	if (!(leftBorderRatio*tmp < thro&&thro < rightBorderRatio*tmp))
		return false;

	//step3.-------------------------�жϸ�ֱ�߳����Ƿ񹻳�--------------------------------//
	topY = 99999999, bottomY = -1;
	//�ӵײ������ҵ�һ������ֱ�ߵĵ㣬��¼ΪbottomX\Y
	for (i = 0; i < m_FPntNum; i++)
	{
		if (abs(((m_FPntXROI[i] * m_cosLUTM4096[theta] + m_FPntYROI[i] * m_sinLUTM4096[theta]) >> 12) - thro) <= 1)///����߶δ���������
		{
			bottomY = m_FPntYROI[i];
			bottomX = m_FPntXROI[i];
			break;
		}
	}
	//�Ӷ��������ҵ�һ������ֱ�ߵĵ㣬��¼ΪtopX\Y
	for (i = m_FPntNum - 1; i >= 0; i--)
	{
		if (abs(((m_FPntXROI[i] * m_cosLUTM4096[theta] + m_FPntYROI[i] * m_sinLUTM4096[theta]) >> 12) - thro) <= 1)///����߶δ���������
		{
			topY = m_FPntYROI[i];
			topX = m_FPntXROI[i];
			break;
		}
	}
	//�жϳ����Ƿ񹻳���������������������Ϊ����Ȥ����߶ȵ�һ�룩
	if ((topX - bottomX)*(topX - bottomX) + (topY - bottomY)*(topY - bottomY) < ((m_height - m_tY)*(m_height - m_tY) >> 2))
		return false;

	//step4.-------------------------����ɸѡ������true--------------------------------//
	return true;

}

// �ж��Ƿ�Ϊ����е��ߣ���ɸѡ����ȷ�е���	//�β�  theta��thro��ֱ�߼��������		angleRange:�е���theta�ķ�Χ	leftBorderRatio\rightBorderRatio��[0,1]:�е�����ʧ�߷�Χ������ֵ��
bool KxyLaneDetect::IsLane_improve(int theta, int thro, unsigned int  angleRange, double leftBorderRatio, double rightBorderRatio)
{
	int tmp, i;
	int width = m_width,
		yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)),
		height = m_height - yROI;
	int topY, bottomY, topX, bottomX;

	//step1.-------------------------�жϸ�ֱ��б���Ƿ����е��ߵĿɽ��ܷ�Χ��-------------------------//
	if (theta<-int(angleRange) || theta>int(angleRange))
		return false;

	//step2.-------------------------�жϸ�ֱ���Ƿ񾭹��е�����ʧ����--------------------------------//
	tmp = width*cos(theta*3.14159 / 180);
	if (!(leftBorderRatio*tmp < thro&&thro < rightBorderRatio*tmp))
		return false;

	
	//step3.-------------------------����ɸѡ������true--------------------------------//
	return true;

}


//����任:threshold���������������С��Ŀ ver3.0
void KxyLaneDetect::Hough(int threshold)
{
	int deertaTheta = 15, deertaThro = max(m_width, m_height) >> 2;	//deertaThetaΪ����е��߾�ȷλ�õĽǶȦѿ�ȣ�deertaThroΪ�ȿ��
	int tmp;
	int i, j;
	int thro, theta, throNew, thetaNew, throReal, thetaReal;		//�������(theta)��[-90,90]����(thro)��[-height,diagonalLen]
	int pntCount;		//�ܼ�⵽����������Ŀ

						//widthΪͼ�񼴸���Ȥ�����ȣ�yROIΪ����Ȥ������ԭͼ�е�y���꣬heightΪ����Ȥ�����ͼ��߶�
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	//diagonalLenΪ����Ȥ�����ͼ��Խ��߿��
	int diagonalLen = sqrt(width*width + height*height);
	int halfHeight = (height >> 1), halfDiagonalLen = (diagonalLen >> 1);

	//����thro�Ŀ��Ϊԭ����һ�룬������������ͬ��thro��Ϊһ�࣬��������ٶ�
	int thetaSpan = 180 + 1, throSpan = halfDiagonalLen + halfHeight + 1;
	int centerIndex = 90 * throSpan + halfHeight;			//���ĵ�����

	int AccumArrLen = thetaSpan*throSpan;					//�������鳤��
	int *pAccumArr = new int[AccumArrLen];					//��������
	memset(pAccumArr, 0, sizeof(int)*AccumArrLen);
	int *pArr = pAccumArr + centerIndex;					//pArrָ�����������м�

															//step1.-------------------------�����ü�������-------------------------//
	for (theta = -90; theta <= 90; theta++)
	{
		for (j = 0; j < m_FPntNum; j++)
		{
			thro = ((m_FPntXROI[j] * m_cosLUTM4096[theta] + (m_FPntYROI[j]) * m_sinLUTM4096[theta]) >> 13);	//
			pArr[theta*throSpan + thro]++;
		}
	}
	//step2.-------------------------���ݼ��������ҵ�ֱ�ߣ�������-------------------------//
	for (theta = -89; theta <= 90; theta++)
	{
		for (thro = -halfHeight; thro <= halfDiagonalLen; thro++)
		{
			pntCount = pArr[theta*throSpan + thro];
			if (pntCount >= threshold)										//�ҵ��ܴ���һ����Ŀ��threshold��������������е���
			{
				thetaReal = thetaNew = theta, throReal = throNew = thro;
				for (thetaNew = theta; thetaNew < min(90, theta + deertaTheta); thetaNew++)	//�ҵ������е��ߵľ�׼λ��
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

				if (IsLane(theta, 2 * thro, 70, 0.38, 0.68))		//�ж��Ƿ�Ϊ����е���
				{
					DrawLine(theta, 2 * thro, 0, m_height - yROI, 2);						//��Ϊ����е�������
																							//cout << theta << " " << 2 * thro << endl;
				}
				theta += deertaTheta;			//�Ƶ���һ���������
				break;
			}
		}
	}
	//step3.-------------------------�ͷ��ڴ沢����-------------------------//
	delete pAccumArr;
	return;
}

//�ж��Ƿ�Ϊ��������ж����ݸı䣬����ж���LineJoint()�н��У�������ʵ�ʽ��зֶ�ֱ�߼��
bool KxyLaneDetect::IsCurve()
{
	//��ʼ��ÿ���߶�����
	m_FirLineNum = 0;
	m_SecLineNum = 0;
	m_ThiLineNum = 0;

	m_IndexTop = m_FPntNum - 1;
	m_IndexMid = 0;
	m_IndexBottom = 0;


	//��¼ÿһ�����������ʼ�±꣬����hough�仯
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

	//ÿһ��hough�仯
	//��ֵ���ݷֲ�ͶӰ������ȷ��
	Hough_improve(m_layerNum / 12,m_IndexMid, m_IndexTop, 0, 1);
	Hough_improve(m_layerNum / 12, m_IndexBottom, m_IndexMid, 1, 2);
	Hough_improve(m_layerNum / 9, 0, m_IndexBottom, 2, 3);


	return 1;
}

//�ֶ���ʹ�õĻ���
void KxyLaneDetect::Hough_improve(int threshold, int startIndex, int endIndex, int color, int layer)
{
	int deertaTheta = 25, deertaThro = max(m_width, m_height) >> 2;	//deertaThetaΪ����е��߾�ȷλ�õĽǶȦȿ�ȣ�deertaThroΪ�ѿ��
	int tmp;
	int i, j;
	int thro, theta, throNew, thetaNew, throReal, thetaReal;		//�������(theta)��[-90,90]����(thro)��[-height,diagonalLen]
	int pntCount;		//�ܼ�⵽����������Ŀ

	//widthΪͼ�񼴸���Ȥ�����ȣ�yROIΪ����Ȥ������ԭͼ�е�y���꣬heightΪ����Ȥ�����ͼ��߶�
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	//diagonalLenΪ����Ȥ�����ͼ��Խ��߿��
	int diagonalLen = sqrt(width*width + height*height);
	int halfHeight = (height >> 1), halfDiagonalLen = (diagonalLen >> 1);

	//����thro�Ŀ��Ϊԭ����һ�룬������������ͬ��thro��Ϊһ�࣬��������ٶ�
	int thetaSpan = 180 + 1, throSpan = halfDiagonalLen + halfHeight + 1;
	int centerIndex = 90 * throSpan + halfHeight;			//���ĵ�����

	int AccumArrLen = thetaSpan*throSpan;					//�������鳤��
	int *pAccumArr = new int[AccumArrLen];					//��������
	memset(pAccumArr, 0, sizeof(int)*AccumArrLen);
	int *pArr = pAccumArr + centerIndex;					//pArrָ�����������м�

	//step1.-------------------------�����ü�������-------------------------//
	for (theta = -90; theta <= 90; theta++)
	{
		for (j = startIndex; j < endIndex; j++)
		{
			thro = ((m_FPntXROI[j] * m_cosLUTM4096[theta] + (m_FPntYROI[j]) * m_sinLUTM4096[theta]) >> 13);	//
			pArr[theta*throSpan + thro]++;
		}
	}
	//step2.-------------------------���ݼ��������ҵ�ֱ�ߣ�������-------------------------//
	for (theta = -89; theta <= 90; theta++)
	{
		for (thro = -halfHeight; thro <= halfDiagonalLen; thro++)
		{
			pntCount = pArr[theta*throSpan + thro];
			if (pntCount >= threshold)										//�ҵ��ܴ���һ����Ŀ��threshold��������������е���
			{
				//printf("found!\n");
				thetaReal = thetaNew = theta, throReal = throNew = thro;
				for (thetaNew = theta; thetaNew < min(90, theta + deertaTheta); thetaNew++)	//�ҵ������е��ߵľ�׼λ��
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



				//�ж��Ƿ�Ϊ����е���
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


					//	DrawLine(theta, 2 * thro, m_FPntYROI[endIndex], m_FPntYROI[startIndex],color);						//��Ϊ����е�������
					//cout << theta << " " << 2 * thro << endl;
				}



				theta += deertaTheta;			//�Ƶ���һ���������
				break;
			}
		}
	}
	//step3.-------------------------�ͷ��ڴ沢����-------------------------//
	delete pAccumArr;
	return;

}

void KxyLaneDetect::LineJoint()
{
	//------------------------------------------------��ʼ��
	m_curveNum = 0;
	memset(m_curve, 0, sizeof(int) * 15);
	int i, j, k;
	int x1, x2b, x2t, x3;
	//widthΪͼ�񼴸���Ȥ�����ȣ�yROIΪ����Ȥ������ԭͼ�е�y���꣬heightΪ����Ȥ�����ͼ��߶�
	int width = m_width, yROI = (m_pROI - m_pGryImg) / (m_width * sizeof(BYTE)), height = m_height - yROI;
	int MidY, BottomY;
	MidY = m_MidY - yROI, BottomY = m_BottomY - yROI;
	double rad = 1.0 / 180 * 3.14159;//����
	double Tan, theta, thro;
	int temp;
	bool already;
	int top, mid, bot;
	int thetaDifSum;//����ǶȲ��ۼӺ�

	//------------------------------------------���м�������������߶�ƴ��
	for (i = 1; i <= m_SecLineNum; i++)
	{
		theta = m_SecTheta[i];
		thro = m_SecThro[i];
		Tan = tan(theta * rad);
		temp = thro / cos(theta*rad);
		x1 = temp - int(BottomY*Tan);
		already = false;
		//�м��͵ײ�ƴ��
		for (j = 1; j <= m_ThiLineNum; j++)
		{
			x2b = m_ThiThro[j] / cos(m_ThiTheta[j] * rad) - int(BottomY*tan(m_ThiTheta[j] * rad));
			//����м���߶κ͵ײ��߶�ƴ�ӳɹ�
			if (std::abs(x2b - x1) <= m_maxDetectWidth * 1.5)//���m_maxDetectWidth * 1.5����Ŀ���Ϊһ������
			{
				m_curveNum++;
				already = true;
				m_curve[m_curveNum] = j * 100 + i * 10;

				//����������м��ƴ��
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

		//����м���߶�û�к͵ײ�ƴ��δ�ɹ�
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




	///�ӵײ�����ƴ���߶�
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
	//		if (std::abs(x2b - x1) <= m_maxDetectWidth * 1.5)//���m_maxDetectWidth * 1.5����Ŀ���Ϊһ������
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





	//----------------------�ж�ֱ���������-------------------//
	for (i = 1; i <= m_curveNum; i++)
	{
		thetaDifSum = 0;
		bot = m_curve[i] / 100;
		mid = (m_curve[i] % 100) / 10;
		top = m_curve[i] % 10;

		//�������ײ�û���߶Σ����ֵΪ��
		thetaDifSum += (m_ThiTheta[bot] == 0 ? 0 : abs(m_SecTheta[mid] - m_ThiTheta[bot]));
		thetaDifSum += (m_FirTheta[top] == 0 ? 0 : abs(m_SecTheta[mid] - m_FirTheta[top]));

		//�о�,�Ƕ��ۼӺʹ���15��
		if (thetaDifSum>15||top==0)
		{
			m_isCurve = true;
			break;
		}
	}



	//���ԣ���ɾ
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
	double rad = 1.0 / 180 * 3.14159;//����
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

	//������
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

		//���һ��������֮ǰ�����������㼰���ϵ���ͬ��������
		for (t = 1; t <= i-1; t++)
		{
			tem = 0;
			tem += (((m_curve[t] / 100) - bot) == 0 ? 1 : 0);//�ײ���ͬ��1��0
			tem += (((m_curve[t] % 100) - mid) == 0 ? 1 : 0);
			tem += (((m_curve[t] % 10) - top) == 0 ? 1 : 0);

			//��������㼰������ͬ
			if (tem >= 2)
			{
				already = 1;
				break;
			}
		}
		//��������㼰������ͬ
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
		/*=======���y=a0+a1*x+a2*x^2+����+apoly_n*x^poly_n========*/
		/*=====n�����ݸ��� xy������ֵ poly_n�Ƕ���ʽ������======*/
		/*===����a0,a1,a2,����a[poly_n]��ϵ����������һ�������=====*/
		memset(poly, 0, sizeof(double) * 4);
		//����ײ�Ϊ�㣬���ƶ�������ͷָ��ָ���м��
		
		FX = FPntCurveX;
		FY = FPntCurveY;

		if (bot == 0)
		{
			FX = FPntCurveX + 5;
			FY = FPntCurveY + 5;
		}
		//ע�⣡����X��Y���껥��
		polyfit(FPntNum, FY, FX, 3, poly);

		//����
		BYTE *pRGBROI = m_pResImg + yROI*m_width * 3;
		BYTE *pCur;
		
		//�������û�У���ֻ��������
		if (top == 0) {
			drawstartY = m_MidY-yROI;
		}
		else {
			drawstartY = 0;
		}

		//����ײ�û�У���ֻ��������
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
	sprintf(filename, "G:\\�������2��\\new%06d %d��.bmp", m_nFrameID, m_curveNum);
	//RmwWriteByteImg2BmpFile(m_pGryImg, m_width, m_height, filename);

	sprintf(filename, "G:\\TestImage_draft\\pointnew%06d %d��.bmp", m_nFrameID, m_curveNum);
	//RmwWrite24BitImg2BmpFile(m_pResImg, m_width, m_height, filename);



}



////����縴��24λ��ɫͼ
//void KxyLaneDetect::CopyRGBImage(BYTE *pRGBImg)
//{
//	memcpy(m_pRGBImg, pRGBImg, m_memSize * 3);
//}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//��纯��
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//�����������������Сֵ
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

//������������ĺ���ƽ��ֵ
void CalIntArraySumAvg(int a[], int n, int &sum, double &avg)
{
	int i;
	sum = 0;
	for (i = 0; i < n; i++)
		sum += a[i];
	avg = 1.0*sum / n;
}

//���������飩�˲����ý����������˱�Ե	//his:ԭ����	hisLen:��������	 filterLen:�˲�����С	
void HistogramAvgFilter(int *his, int hisLen, int filterLen, int *res)
{
	//ת��filterLenΪ����
	if (filterLen % 2 == 0)
		filterLen++;

	int i, j;
	int sum, avg;
	int *pre, *next;
	int halfFilterLen = filterLen / 2;

	int edgeFilterLen, edgeHalfFilterLen, edgeSum, edgeAvg, edgeRight, edgeLeft;
	// step.1-----------------�������Ե--------------------//
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

	// step.2-----------------�����һ�������˲���Ԫ��--------------------//
	sum = edgeSum + his[edgeRight] + his[edgeRight + 1 + 1];
	res[halfFilterLen] = avg = sum / filterLen;
	pre = &his[0];
	next = &his[filterLen];

	// step.3-----------------�м䴦��--------------------//
	for (i = halfFilterLen + 1; i < hisLen - halfFilterLen; i++)
	{


		sum = sum - *pre + *next;
		avg = sum / filterLen;
		res[i] = avg;

		pre++, next++;
	}

	// step.4-----------------�����ұ�Ե--------------------//
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

	// step.5-----------------����--------------------//
	return;

}

//�������	a-b=res
void CalIntArrayDifference(int *a, int *b, int len, int *res)
{
	int i;
	for (i = 0; i < len; i++)
	{
		res[i] = a[i] - b[i];
	}
}

/*==================polyfit(n,x,y,poly_n,a)===================*/
/*=======���y=a0+a1*x+a2*x^2+����+apoly_n*x^poly_n========*/
/*=====n�����ݸ��� xy������ֵ poly_n�Ƕ���ʽ������======*/
/*===����a0,a1,a2,����a[poly_n]��ϵ����������һ�������=====*/
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
////    ��˹��Ԫ�� 
////    n: ϵ���ĸ���  
////    ata: ���Ծ���  
////    sumxy: ���Է������Yֵ  
////    p: ������ϵĽ��  
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

