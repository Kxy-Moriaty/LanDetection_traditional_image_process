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
	//公有函数
public:
	//构造/析构
	KxyLaneDetect();
	~KxyLaneDetect();
    //初始化
    bool Initialize(bool isRGB,int width,int height);
	//执行
	bool DoNext(BYTE *pRGBImg, int tY, int tlX, int trX, int bY, int blX, int brX, int maxDetectWidth,int minDetectWidth,int contrast);
    //结果
    BYTE *GetResImg();
	int GetResWidth();
    int GetResHeight();
	////从外界复制24位彩色图
	//void CopyRGBImage(BYTE *pRGBImg);

	//获得蓝通道
	void CalBlueChannel();
	//获得绿通道
	void CalGreenChannel();
	//获得红通道
	void CalRedChannel();
	//计算8位灰度图像积分图	//图像像素个数不得大于2^24=16777216约1600万像素
	void CalGry8bIntegralImage();
	//分层投影rowNUM层，获取特征点坐标，存储在featureX/Y中;	//maxDetectWidth：底部行道线宽度
	//BottomYMidYTopY分别为弯道检查三层划分的Y坐标（原图坐标），引入的意义在于记录特征点坐标的下标，以便Hough变换时不用遍历
	void KxyLaneDetect::CalFPoints();
	//霍夫变换:threshold代表检测特征点的最小数目
	void KxyLaneDetect::Hough(int threshold);
	// 判断是否为真的行道线	//形参  theta、thro：直线极坐标参数		angleRange:行道线theta的范围	leftBorderRatio\rightBorderRatio∈[0,1]:行道线消失线范围（比例值）
	bool KxyLaneDetect::IsLane(int theta, int thro, unsigned int angleRange, double leftBorderRatio, double rightBorderRatio);
	////分段中使用的行道线判别函数
	bool KxyLaneDetect::IsLane_improve(int theta, int thro, unsigned int angleRange, double leftBorderRatio, double rightBorderRatio);
	//划线到m_pResImg	//theta、thro：直线极坐标参数
	void KxyLaneDetect::DrawLine(int theta, int thro, int TopY, int BottomY,int color);
	//判断是否为弯道
	bool KxyLaneDetect::IsCurve();
	//弯道线段拼接
	void KxyLaneDetect::LineJoint();
	//弯道则曲线拟合
	void KxyLaneDetect::CurveFitting();
	//为三段图像设计，更通用的hough变化，startY与endY指示分段特征点数组下标的开始与结束
	void KxyLaneDetect::Hough_improve(int threshold,int startY, int endY,int color,int layer);




	//私有函数
private:
	// 内存释放
	void Dump();
	// 调试
	void Debug();
	//内部函数：获得24位位图某通道
	void KxyLaneDetect::CalChannelBase(int firstPixel = 0);
	//内部函数：计算[-90,90]的三角函数表*4096
	void KxyLaneDetect::CalSinLUTM4096();
	void KxyLaneDetect::CalCosLUTM4096();

	//私有成员
private:
	//初始化成功
	bool m_isInitOK;
	//图像属性
	bool m_isRGB;
	int m_width;
	int m_height;
//	const char * filename;
	//内存
	BYTE *m_pRGBImg;
	BYTE *m_pGryImg;
	BYTE *m_pResImg;//彩色图
	BYTE *m_pTemImg;
	int *m_pSumImg;
	int m_memSize;
	int *m_prjHist;//分层投影直方图
	int *m_prjTmpH;//分层投影临时直方图（m_prjHist滤波后）
	//sin和cos乘以4096的整形表
	int m_sinLUT[181];	//内存指针
	int m_cosLUT[181];
	int *m_sinLUTM4096; //操作指针，指向m_sinLUT[90]
	int *m_cosLUTM4096;
	//操作头指针,感兴趣区域；
	BYTE *m_pROI;
	//特征点坐标,是m_pROI中的特征点坐标
	vector<int> m_FPntXROI;
	vector<int> m_FPntYROI;
	int m_FPntNum;//特征点数目
	//弯道标志
	bool m_isCurve = false;
	//计算弯道分层Hough变换时，特征点下标
	int m_IndexBottom;
	int m_IndexMid;
	int m_IndexTop;
	//三层线段信息(注意，从第1元素记起，即第0元素不计）
	vector<double> m_FirTheta;
	vector<double> m_FirThro;
	int m_FirLineNum;

	vector<double> m_SecTheta;
	vector<double> m_SecThro;
	int m_SecLineNum;
	
	vector<double> m_ThiTheta;
	vector<double> m_ThiThro;
	int m_ThiLineNum;
	//曲线，编码记录折线，百十个位分别代表底中高三层中线段的编号；
	int *m_curve;
	int m_curveNum;

	//内部计数器
	int m_nFrameID;

	//道路模型参数以及函数间传递参数
	int m_tlX, m_trX;//tl：topleft	tr：topright
	int m_tY;//tY:topY
	int m_blX, m_brX;//bl：bottomleft br：bottomright
	int m_bY;//bY:bottomY
	int m_maxDetectWidth;//图像底部的行道线宽度
	int m_minDetectWidth;//m_tY处的行道线宽度
	int m_layerNum;//分层投影层数
	double m_contrast;//行道线与路面对比度
	int m_BottomY;
	int m_MidY;
	int m_TopY;//分三段
	
};

//计算整形数组最大最小值
void CalIntArrayMaxMin(int a[], int n, int &max, int &min);

//计算整形数组的和与平均值
void CalIntArraySumAvg(int a[], int n, int &sum, double &avg);

//向量（数组）滤波，用渐进法处理了边缘	//his:原向量	hisLen:向量长度	 filterLen:滤波器大小	
void HistogramAvgFilter(int *his, int hisLen, int filterLen, int *res);

//向量求差	a-b=res
void CalIntArrayDifference(int *a, int *b, int len, int *res);

//曲线拟合
void polyfit(int n, double *x, double *y, int poly_n, double p[]);
void gauss_solve(int n, double A[], double x[], double b[]);


#endif
