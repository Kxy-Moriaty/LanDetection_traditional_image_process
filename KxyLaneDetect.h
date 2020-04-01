#define _CRT_SECURE_NO_WARNINGS
// KxyLaneDetect.h : header file
//
#ifndef Kxy_LANE_DETEC_H
#define Kxy_LANE_DETEC_H

#include <windows.h>
#include<vector>
using namespace std;

class KxyLaneDetect
{
	//���к���
public:
	//����/����
	KxyLaneDetect();
	~KxyLaneDetect();
    //��ʼ��
    bool Initialize(bool isRGB,int width,int height);
	//ִ��
	bool DoNext(BYTE *pRGBImg, int tY, int tlX, int trX, int bY, int blX, int brX, int maxDetectWidth,int minDetectWidth,int contrast);
    //���
    BYTE *GetResImg();
	int GetResWidth();
    int GetResHeight();
	////����縴��24λ��ɫͼ
	//void CopyRGBImage(BYTE *pRGBImg);

	//�����ͨ��
	void CalBlueChannel();
	//�����ͨ��
	void CalGreenChannel();
	//��ú�ͨ��
	void CalRedChannel();
	//����8λ�Ҷ�ͼ�����ͼ	//ͼ�����ظ������ô���2^24=16777216Լ1600������
	void CalGry8bIntegralImage();
	//�ֲ�ͶӰrowNUM�㣬��ȡ���������꣬�洢��featureX/Y��;	//maxDetectWidth���ײ��е��߿��
	//BottomYMidYTopY�ֱ�Ϊ���������㻮�ֵ�Y���꣨ԭͼ���꣩��������������ڼ�¼������������±꣬�Ա�Hough�任ʱ���ñ���
	void KxyLaneDetect::CalFPoints();
	//����任:threshold���������������С��Ŀ
	void KxyLaneDetect::Hough(int threshold);
	// �ж��Ƿ�Ϊ����е���	//�β�  theta��thro��ֱ�߼��������		angleRange:�е���theta�ķ�Χ	leftBorderRatio\rightBorderRatio��[0,1]:�е�����ʧ�߷�Χ������ֵ��
	bool KxyLaneDetect::IsLane(int theta, int thro, unsigned int angleRange, double leftBorderRatio, double rightBorderRatio);
	////�ֶ���ʹ�õ��е����б���
	bool KxyLaneDetect::IsLane_improve(int theta, int thro, unsigned int angleRange, double leftBorderRatio, double rightBorderRatio);
	//���ߵ�m_pResImg	//theta��thro��ֱ�߼��������
	void KxyLaneDetect::DrawLine(int theta, int thro, int TopY, int BottomY,int color);
	//�ж��Ƿ�Ϊ���
	bool KxyLaneDetect::IsCurve();
	//����߶�ƴ��
	void KxyLaneDetect::LineJoint();
	//������������
	void KxyLaneDetect::CurveFitting();
	//Ϊ����ͼ����ƣ���ͨ�õ�hough�仯��startY��endYָʾ�ֶ������������±�Ŀ�ʼ�����
	void KxyLaneDetect::Hough_improve(int threshold,int startY, int endY,int color,int layer);




	//˽�к���
private:
	// �ڴ��ͷ�
	void Dump();
	// ����
	void Debug();
	//�ڲ����������24λλͼĳͨ��
	void KxyLaneDetect::CalChannelBase(int firstPixel = 0);
	//�ڲ�����������[-90,90]�����Ǻ�����*4096
	void KxyLaneDetect::CalSinLUTM4096();
	void KxyLaneDetect::CalCosLUTM4096();

	//˽�г�Ա
private:
	//��ʼ���ɹ�
	bool m_isInitOK;
	//ͼ������
	bool m_isRGB;
	int m_width;
	int m_height;
//	const char * filename;
	//�ڴ�
	BYTE *m_pRGBImg;
	BYTE *m_pGryImg;
	BYTE *m_pResImg;//��ɫͼ
	BYTE *m_pTemImg;
	int *m_pSumImg;
	int m_memSize;
	int *m_prjHist;//�ֲ�ͶӰֱ��ͼ
	int *m_prjTmpH;//�ֲ�ͶӰ��ʱֱ��ͼ��m_prjHist�˲���
	//sin��cos����4096�����α�
	int m_sinLUT[181];	//�ڴ�ָ��
	int m_cosLUT[181];
	int *m_sinLUTM4096; //����ָ�룬ָ��m_sinLUT[90]
	int *m_cosLUTM4096;
	//����ͷָ��,����Ȥ����
	BYTE *m_pROI;
	//����������,��m_pROI�е�����������
	vector<int> m_FPntXROI;
	vector<int> m_FPntYROI;
	int m_FPntNum;//��������Ŀ
	//�����־
	bool m_isCurve = false;
	//��������ֲ�Hough�任ʱ���������±�
	int m_IndexBottom;
	int m_IndexMid;
	int m_IndexTop;
	//�����߶���Ϣ(ע�⣬�ӵ�1Ԫ�ؼ��𣬼���0Ԫ�ز��ƣ�
	vector<double> m_FirTheta;
	vector<double> m_FirThro;
	int m_FirLineNum;

	vector<double> m_SecTheta;
	vector<double> m_SecThro;
	int m_SecLineNum;
	
	vector<double> m_ThiTheta;
	vector<double> m_ThiThro;
	int m_ThiLineNum;
	//���ߣ������¼���ߣ���ʮ��λ�ֱ������и��������߶εı�ţ�
	int *m_curve;
	int m_curveNum;

	//�ڲ�������
	int m_nFrameID;

	//��·ģ�Ͳ����Լ������䴫�ݲ���
	int m_tlX, m_trX;//tl��topleft	tr��topright
	int m_tY;//tY:topY
	int m_blX, m_brX;//bl��bottomleft br��bottomright
	int m_bY;//bY:bottomY
	int m_maxDetectWidth;//ͼ��ײ����е��߿��
	int m_minDetectWidth;//m_tY�����е��߿��
	int m_layerNum;//�ֲ�ͶӰ����
	double m_contrast;//�е�����·��Աȶ�
	int m_BottomY;
	int m_MidY;
	int m_TopY;//������
	
};

//�����������������Сֵ
void CalIntArrayMaxMin(int a[], int n, int &max, int &min);

//������������ĺ���ƽ��ֵ
void CalIntArraySumAvg(int a[], int n, int &sum, double &avg);

//���������飩�˲����ý����������˱�Ե	//his:ԭ����	hisLen:��������	 filterLen:�˲�����С	
void HistogramAvgFilter(int *his, int hisLen, int filterLen, int *res);

//�������	a-b=res
void CalIntArrayDifference(int *a, int *b, int len, int *res);

//�������
void polyfit(int n, double *x, double *y, int poly_n, double p[]);
void gauss_solve(int n, double A[], double x[], double b[]);


#endif
