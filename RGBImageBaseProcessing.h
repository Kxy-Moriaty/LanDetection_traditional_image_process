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

//תΪ��4����λͼ��������������λͼ
//������8��24��32λ��λͼ
BYTE* Img2AlignedImg(BYTE *pSourceImg, int width, int height, int bit_per_pixel);
//תΪ�в�4����λͼ�������洢�������ڴ��λͼ
//������8��24��32λ��λͼ
BYTE* Img2MisalignedImg(BYTE *pSourceImg, int width, int height, int bit_per_pixel);

//24λ��ɫͼת8λ�Ҷ�ͼ
void RGB24b2GryImg(BYTE *pRGBImg, int width, int height, BYTE *pGryImg);

//24λ��ɫͼת8λ�Ҷ�ͼ,�Զ�����Ҷ�ͼ�ڴ�ռ�
BYTE* RGB24b2GryImg(BYTE *pRGBImg, int width, int height);


//void RGB32b2GryImg(BYTE *pRGBAImg, BYTE *pGryImg, int width, int height);



bool AddChannel2RGB24b(BYTE *pGryImg, char channel[], BYTE *pRGBImg, int width, int height);

bool DelChannelFromRGB24b(BYTE *pRGBImg, char channel[], int width, int height);


#endif