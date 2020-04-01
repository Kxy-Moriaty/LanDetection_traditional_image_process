
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
