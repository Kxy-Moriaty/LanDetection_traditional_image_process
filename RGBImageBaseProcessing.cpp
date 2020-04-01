#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<algorithm>
#include <string.h>
#include <iostream>
#include <math.h>
#include <time.h>

//内部函数：获得24位位图某通道


//转为行4对齐位图，创建行连续的位图
//适用与8，24，32位等位图
BYTE* Img2AlignedImg(BYTE *pSourceImg, int width, int height,int bit_per_pixel)
{
	int i, j;

	int deertaX = (bit_per_pixel>>3) * width;
	int modd = deertaX % 4;
	int deerta = (4 - modd) % 4;
	int pitch = deertaX + deerta;
	int size = pitch*height;
	BYTE *pResImg = new BYTE[size];

	if (deerta == 0)
	{
		memcpy(pResImg, pSourceImg, size*sizeof(BYTE));
		return pResImg;
	}
	for (i = 0; i < height; i++)
	{
		memcpy(pResImg + i*pitch, pSourceImg + i * deertaX, deertaX*sizeof(BYTE));
		for (j = 0; j < deerta; j++)
			*(pResImg + i*deertaX + j) = 0;
	}
	return pResImg;
}

//转为行不4对齐位图，创建存储于连续内存的位图
//适用与8，24，32位等位图
BYTE* Img2MisalignedImg(BYTE *pSourceImg, int width, int height, int bit_per_pixel)
{
	int i;

	int deertaX = (bit_per_pixel >> 3) * width;
	int modd = deertaX % 4;
	int deerta = (4 - modd) % 4;
	int pitch = deertaX + deerta;
	int size = pitch*height;
	BYTE *pResImg = new BYTE[size];

	if (deerta == 0)
	{
		memcpy(pResImg, pSourceImg, size*sizeof(BYTE));
		return pResImg;
	}

	for (i = 0; i < height; i++)
		memcpy(pResImg + i*deertaX, pSourceImg + i*pitch, deertaX*sizeof(BYTE));
	return pResImg;
}

void GetRGB24bLUT16(int *&Red, int *&Green, int *&Blue)
{
	int i;
	static bool firstRun = true;
	static int R[256], G[256], B[256];

	if (firstRun == true)
	{
		for (i = 0; i < 256; i++)
		{
			R[i] = i * 19595;
			G[i] = i * 38469;
			B[i] = i * 7471;
		}
		firstRun = false;
	}
	Red = R, Green = G, Blue = B;
}

//24位彩色图转8位灰度图
void RGB24b2GryImg(BYTE *pRGBImg, int width, int height, BYTE *pGryImg)
{
	int grysize = width*height, rgbsize = 3 * grysize;
	BYTE *pRGB, *pEnd;
	BYTE *pGry;
	int *RedLUT, *GreenLUT, *BlueLUT;
	GetRGB24bLUT16(RedLUT, GreenLUT, BlueLUT);
	int sum;
	pEnd = pRGBImg + rgbsize;
	for (pRGB = pRGBImg, pGry = pGryImg; pRGB < pEnd;)
	{
		sum = BlueLUT[*pRGB++];  //B*0.114
		sum += GreenLUT[*pRGB++]; //G*0.587
		sum += RedLUT[*pRGB++]; //R*0.299
		*(pGry++) = sum >> 16;
	}
}

//24位彩色图转8位灰度图,自动分配灰度图内存空间
BYTE* RGB24b2GryImg(BYTE *pRGBImg, int width, int height)
{
	BYTE *pGryImg = new BYTE[width*height];

	RGB24b2GryImg(pRGBImg, width, height, pGryImg);

	return pGryImg;
}



//?存在问题
void RGB32b2GryImg(BYTE *pRGBAImg, BYTE *pGryImg, int width, int height)
{
	BYTE *pRGBA, *pEnd;
	BYTE *pGry;
	int sum;
	pEnd = pRGBAImg + 4 * width*height;
	for (pRGBA = pRGBAImg, pGry = pGryImg; pRGBA < pEnd;)
	{
		pRGBA++;
		sum = *(pRGBA++) * 7471;  //B*0.114
		sum += *(pRGBA++) * 38469; //G*0.587
		sum += *(pRGBA++) * 19595; //R*0.299
		*(pGry++) = sum >> 16;
	}
	return;
}

bool AddChannel2RGB24b(BYTE *pGryImg, char channel[], BYTE *pRGBImg, int width, int height)
{
	int i, j;
	int rgbSize = width*height * 3;
	if (strcmp("blue", channel) == 0)
		for (i = 0, j = 0; i < rgbSize; i += 3, j++)
		{
			pRGBImg[i] = pGryImg[j];
		}
	else
		if (strcmp("green", channel) == 0)
			for (i = 1, j = 0; i < rgbSize; i += 3, j++)
			{
				pRGBImg[i] = pGryImg[j];
			}
		else
			if (strcmp("red", channel) == 0)
				for (i = 2, j = 0; i < rgbSize; i += 3, j++)
				{
					pRGBImg[i] = pGryImg[j];
				}
			else
				return false;
	return true;
}

bool DelChannelFromRGB24b(BYTE *pRGBImg, char channel[], int width, int height)
{
	int i, j;
	int rgbSize = width*height * 3;
	if (strcmp("blue", channel) == 0)
		for (i = 0, j = 0; i < rgbSize; i += 3, j++)
		{
			pRGBImg[i] = 0;
		}
	else
		if (strcmp("green", channel) == 0)
			for (i = 1, j = 0; i < rgbSize; i += 3, j++)
			{
				pRGBImg[i] = 0;
			}
		else
			if (strcmp("red", channel) == 0)
				for (i = 2, j = 0; i < rgbSize; i += 3, j++)
				{
					pRGBImg[i] = 0;
				}
			else
				return false;
	return true;
}

void GetBrightness(BYTE *pRGBImg, int width, int height)
{
	//;
}