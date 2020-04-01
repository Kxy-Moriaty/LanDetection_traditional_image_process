#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<algorithm>
#include <string.h>
#include <iostream>
#include <math.h>
#include <time.h>

#ifndef RGB_H
#define RGB_H

void GetBlueChannel(BYTE *pRGBImg, BYTE *pGryImg, int width, int height);
void GetGreenChannel(BYTE *pRGBImg, BYTE *pGryImg, int width, int height);
void GetRedChannel(BYTE *pRGBImg, BYTE *pGryImg, int width, int height);

//转为行4对齐位图，创建行连续的位图
//适用与8，24，32位等位图
BYTE* Img2AlignedImg(BYTE *pSourceImg, int width, int height, int bit_per_pixel);
//转为行不4对齐位图，创建存储于连续内存的位图
//适用与8，24，32位等位图
BYTE* Img2MisalignedImg(BYTE *pSourceImg, int width, int height, int bit_per_pixel);

//24位彩色图转8位灰度图
void RGB24b2GryImg(BYTE *pRGBImg, int width, int height, BYTE *pGryImg);

//24位彩色图转8位灰度图,自动分配灰度图内存空间
BYTE* RGB24b2GryImg(BYTE *pRGBImg, int width, int height);


//void RGB32b2GryImg(BYTE *pRGBAImg, BYTE *pGryImg, int width, int height);



bool AddChannel2RGB24b(BYTE *pGryImg, char channel[], BYTE *pRGBImg, int width, int height);

bool DelChannelFromRGB24b(BYTE *pRGBImg, char channel[], int width, int height);


#endif