
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
//���8λ�Ҷ�ͼ�����ͼ
//ͼ�����ظ������ô���2^24=16777216Լ1600������
void GetGry8bIntegralImage(BYTE *pGryImg, int width, int height, int *pSumImg)
{
	BYTE *pGry;
	int *pSum;
	int x, y;
	// step.1-----------------�����һ��--------------------//
	pGry = pGryImg;
	pSum = pSumImg;
	*(pSum++) = *(pGry++);		//��һ��Ԫ��
	for (x = 1; x<width; x++) *(pSum++) = *(pSum - 1) + (*(pGry++));
	// step.2-----------------����������--------------------//
	for (y = 1; y<height; y++)
	{
		*(pSum++) = *(pSum - width) + (*(pGry++)); //�����һ��
		for (x = 1; x<width; x++) //����������
		{
			*(pSum++) = *(pGry++) + (*(pSum - 1)) + (*(pSum - width)) - (*(pSum - 1 - width));
		}
	}
}
