#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<algorithm>
#include <string.h>
#include <iostream>
#include <math.h>
#include <time.h>

//获得8位灰度图像积分图
//图像像素个数不得大于2^24=16777216约1600万像素
void GetGry8bIntegralImage(BYTE *pGryImg, int width, int height, int *pSumImg)
{
	BYTE *pGry;
	int *pSum;
	int x, y;
	// step.1-----------------先求第一行--------------------//
	pGry = pGryImg;
	pSum = pSumImg;
	*(pSum++) = *(pGry++);		//第一个元素
	for (x = 1; x<width; x++) *(pSum++) = *(pSum - 1) + (*(pGry++));
	// step.2-----------------再求其他行--------------------//
	for (y = 1; y<height; y++)
	{
		*(pSum++) = *(pSum - width) + (*(pGry++)); //先求第一列
		for (x = 1; x<width; x++) //再求其他列
		{
			*(pSum++) = *(pGry++) + (*(pSum - 1)) + (*(pSum - width)) - (*(pSum - 1 - width));
		}
	}
}

void GetIntArrayMaxMin(int a[], int n, int &max, int &min)
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

void GetIntArraySumAvg(int a[], int n, int &sum, double &avg)
{
	int i;
	sum = 0;
	for (i = 0; i < n; i++)
		sum += a[i];
	avg = 1.0*sum / n;
}

//线性归一化到[0,1],再乘T转整形数组
bool StandNormalizationMT(int source[], int length, int T, int res[])
{
	if (!res)
		return false;

	int i;
	int max, min, deerta;
	int sum;
	double avg;
	GetIntArrayMaxMin(source, length, max, min);
	GetIntArraySumAvg(source, length, sum, avg);

	int average = avg;

	deerta = max - min;
	for (i = 0; i < length; i++)
		res[i] = T*(source[i] - min) / deerta;
	return true;
}

void HistogramAvgFilter(int *his, int len, int filterLen, int *res)
{
	if (filterLen % 2 == 0)
		filterLen++;

	int i, j;
	int sum, avg;
	int *pre, *next;
	int halfFilterLen = filterLen / 2;

	//bianyuanchuli
	for (i = 0; i < halfFilterLen; i++)
		res[i] = his[i];
	for (i = len - 1; i > len - 1 - halfFilterLen; i--)
		res[i] = his[i];

	for (sum = 0, i = 0; i < filterLen; i++)
		sum += his[i];
	avg = sum / filterLen;
	pre = &his[0];
	next = &his[filterLen];

	for (i = halfFilterLen; i < len - halfFilterLen; i++)
	{
		res[i] = avg;

		sum = sum - *pre + *next;
		avg = sum / filterLen;
		pre++, next++;
	}

}

//a-b=res
void GetIntArrayDifference(int *a, int *b, int len, int *res)
{
	int i;
	for (i = 0; i < len; i++)
	{
		res[i] = a[i] - b[i];
	}
}


//rowNUM表示分层投影时有rowNUM层
int GetColShadowAdvanced(BYTE *pGryImg, int width, int height, int rowNUM, BYTE *pResImg, int **pixelSum = NULL)
{
	if (pixelSum == NULL)
		return -1;

	int row_per_layer = height / rowNUM;
	int x, y, i, j;
	int *his = new int[width];
	int *hisN = new int[width];
	int *hisR = new int[width];

	memset(pResImg, 0, width*height*sizeof(BYTE));

	for (y = 0; y < height - row_per_layer; y += row_per_layer)
	{
		memset(his, 0, width * sizeof(int));
		memset(hisN, 0, width * sizeof(int));

		i = y + row_per_layer;
		for (j = 1; j < width; j++)
		{
			his[j] = pixelSum[i][j] - pixelSum[i][j - 1] - (pixelSum[y][j] - pixelSum[y][j - 1]);
		}

		HistogramAvgFilter(his, width, 101, hisN);
		GetIntArrayDifference(his, hisN, width, hisR);

		//StandNormalizationMT(his, width, row_per_layer, hisN);

		for (j = 0; j < width; j++)
		{
			for (i = y + row_per_layer - 1; i >= y; i--)
			{
				if (hisN[j])
					*(pResImg + i*width + j) = 255;
				hisN[j]--;
			}
		}
	}

}

void test1()
{
	int width, height;
	int size;
	BYTE *pRGBImg = RmwRead24BitBmpFile2Img("..\\BMPS\\qing.bmp", &width, &height);
	//BYTE *pRGBImg = RmwRead24BitBmpFile2Img("..\\BMPS\\yu.bmp", &width, &height);
	//BYTE *pRGBImg = RmwRead24BitBmpFile2Img("..\\BMPS\\xue.bmp", &width, &height);
	size = width*height;

	BYTE *pGryImg = new BYTE[size];
	BYTE *pGryImgN = new BYTE[size];
	BYTE *pResImg = new BYTE[size];

	RGB24b2GryImg(pRGBImg, width, height, pGryImg);
	GetGreenChannel(pRGBImg, pGryImgN, width, height);//用绿通道代替灰度值

	RmwWriteByteImg2BmpFile(pGryImg, width, height, "..\\BMPR\\gry.bmp");

	BYTE *pBuffImg = Gry8bCut(pGryImgN, width, height, 0, height * 2 / 3, width - 1, height - 1);
	height = height - 1 - height * 2 / 3 + 1;
	delete pGryImgN;
	pGryImgN = pBuffImg;
	RmwWriteByteImg2BmpFile(pGryImgN, width, height, "..\\BMPR\\gryN.bmp");

	int **pixelSum = GetGry8bIntegralImage(pGryImgN, width, height);

	GetColShadowAdvanced(pGryImgN, width, height, 20, pResImg, pixelSum);
	RmwWriteByteImg2BmpFile(pResImg, width, height, "..\\BMPR\\gryNShadow.bmp");

}

