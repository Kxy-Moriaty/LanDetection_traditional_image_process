#define _CRT_SECURE_NO_WARNINGS
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// KxyImgRW.cpp : ��дͼ��
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <vector>

#include "KxyImgRW.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ����/����
//
////////////////////////////////////////////////////////////////////////////////////////////////////
KxyImgRW::KxyImgRW()
{
	//<�ڹ��캯���а����еĳ�Ա��������ֵ>
	//�Ƿ��ʼ���ɹ�
	m_isInitOK = false;
	//ͼ������
	m_width = 0;
	m_height = 0;
	m_size = 0;
	//�ڴ�
	m_pRGBImg = NULL;
	m_memSize = 0;
}
void KxyImgRW::Dump()
{
	//Ϊ��ε��ڴ������ͷ�дһ��ר�ŵ��ڴ��ͷź���
	if (m_pRGBImg) { delete m_pRGBImg; m_pRGBImg = NULL; }
	m_memSize = 0;
}
KxyImgRW::~KxyImgRW() 
{
	//���������ͷ������ڴ�
	Dump();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////
//// ��ʼ�����ڴ�����
////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//bool KxyImgRW::initialize(const char* filename)
//{
//	//BmpHeader��¼����ʵ�Ŀ��,
//	//����������ʵ�Ŀ��,ȥ����4�������չ.
//	FILE *fp;
//	BITMAPFILEHEADER FileHeader;
//	BITMAPINFOHEADER BmpHeader;
//	//	BYTE *m_pRGBImg;
//	unsigned int size;
//	int Suc = 1, w, h, k, extend, tmp;
//
//	// Open File
//	m_width = m_height = 0;
//	if ((fp = fopen(filename, "rb")) == NULL) return NULL;
//	// Read Struct Info
//	if (fread((void *)&FileHeader, 1, sizeof(FileHeader), fp) != sizeof(FileHeader)) Suc = -1;
//	if (fread((void *)&BmpHeader, 1, sizeof(BmpHeader), fp) != sizeof(BmpHeader)) Suc = -1;
//	if ((Suc == -1) ||
//		(FileHeader.bfOffBits<sizeof(FileHeader) + sizeof(BmpHeader))
//		)
//	{
//		fclose(fp);
//		return false;
//	}
//	// ��ȡͼ��߿���Ϣ�������ڴ�
//	m_width = w = BmpHeader.biWidth;
//	m_height = h = BmpHeader.biHeight;
//	extend = ((w * 3 + 3) / 4 * 4) - w * 3;
//	m_size = (m_width)*(m_height)* 3;
//	fseek(fp, FileHeader.bfOffBits, SEEK_SET);
//	//���ͼ���СС�ڵ����ѿ��ڴ棬�����������ڴ�
//	if (m_size > m_memSize) {
//		//���ͷ�
//		Dump();
//		//������
//		m_pRGBImg = new BYTE[m_size];
//		m_memSize = m_size;
//	}
//	//��ʼ���ɹ���־
//	if (m_pRGBImg) m_isInitOK = true;
//
//	fclose(fp);
//	return true;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ��д����
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//�������ж�̬�����ڴ棬����ǰ��ȡͼ���С������������ڴ棬�����������ڴ棬�����������ڴ�
BYTE* KxyImgRW::ReadRGBImg(const char* filename)
{
	//BmpHeader��¼����ʵ�Ŀ��,
	//����������ʵ�Ŀ��,ȥ����4�������չ.
	FILE *fp;
	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;
	//	BYTE *m_pRGBImg;
	unsigned int size;
	int Suc = 1, w, h, k, extend, tmp;

	// Open File
	m_width = m_height = 0;
	if ((fp = fopen(filename, "rb")) == NULL) return NULL;
	// Read Struct Info
	if (fread((void *)&FileHeader, 1, sizeof(FileHeader), fp) != sizeof(FileHeader)) Suc = -1;
	if (fread((void *)&BmpHeader, 1, sizeof(BmpHeader), fp) != sizeof(BmpHeader)) Suc = -1;
	if ((Suc == -1) ||
		(FileHeader.bfOffBits<sizeof(FileHeader) + sizeof(BmpHeader))
		)
	{
		fclose(fp);
		return false;
	}
	// ��ͼ
	m_width = w = BmpHeader.biWidth;
	m_height = h = BmpHeader.biHeight;
	extend = ((w * 3 + 3) / 4 * 4) - w * 3;
	m_size = (m_width)*(m_height)* 3;
	fseek(fp, FileHeader.bfOffBits, SEEK_SET);
	//���ͼ���СС�ڵ����ѿ��ڴ棬�����������ڴ�
	if (m_size > m_memSize) {
		//���ͷ�
		Dump();
		//������
		m_pRGBImg = new BYTE[m_size];
		m_memSize = m_size;
	}
	//
	for (int i = 0; i<h; i++)
	{
		if ((int)fread(m_pRGBImg + (h - 1 - i)*w * 3, sizeof(BYTE), w * 3, fp) != w * 3)
		{
			fclose(fp);
			//	delete m_pRGBImg;
			//	m_pRGBImg = NULL;
			return false;
		}
		for (k = 0; k<extend; k++) // ���������
		{
			if (fwrite((void *)&tmp, sizeof(BYTE), 1, fp) != 1) Suc = false;
		}
	}

	fclose(fp);
	return m_pRGBImg;
}
//дͼ��
bool KxyImgRW::WriteRGBImg(const char* filename)
{
	//BmpHeader��¼����ʵ�Ŀ��
	//����Ȳ���4�ı���ʱ�Զ���ӳ�4�ı���,������4�������չ.
	FILE *fp;
	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;
	bool Suc = true;
	int i, extend;
	BYTE *pCur;

	// Open File
	if ((fp = fopen(filename, "w+b")) == NULL) { return false; }
	// Fill the FileHeader
	FileHeader.bfType = ((WORD)('M' << 8) | 'B');
	FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	FileHeader.bfSize = FileHeader.bfOffBits + m_width*m_height * 3L;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;
	if (fwrite((void *)&FileHeader, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER)) Suc = false;
	// Fill the ImgHeader
	BmpHeader.biSize = 40;
	BmpHeader.biWidth = m_width;
	BmpHeader.biHeight = m_height;
	BmpHeader.biPlanes = 1;
	BmpHeader.biBitCount = 24;
	BmpHeader.biCompression = 0;
	BmpHeader.biSizeImage = 0;
	BmpHeader.biXPelsPerMeter = 0;
	BmpHeader.biYPelsPerMeter = 0;
	BmpHeader.biClrUsed = 0;
	BmpHeader.biClrImportant = 0;
	if (fwrite((void *)&BmpHeader, 1, sizeof(BITMAPINFOHEADER), fp) != sizeof(BITMAPINFOHEADER)) Suc = false;
	// write image data
	extend = ((m_width * 3 + 3) / 4 * 4) - m_width * 3;
	if (extend == 0)
	{
		for (pCur = m_pRGBImg + (m_height - 1) * 3 * m_width; pCur >= m_pRGBImg; pCur -= 3 * m_width)
		{
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // ��ʵ������
		}
	}
	else
	{
		for (pCur = m_pRGBImg + (m_height - 1) * 3 * m_width; pCur >= m_pRGBImg; pCur -= 3 * m_width)
		{
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // ��ʵ������
			for (i = 0; i<extend; i++) // ���������
			{
				if (fwrite((void *)(pCur + 3 * (m_width - 1) + 0), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);
	return Suc;
}

//дͼ��
bool KxyImgRW::WriteRGBImg(BYTE *pRGBImg, const char* filename)
{
	//BmpHeader��¼����ʵ�Ŀ��
	//����Ȳ���4�ı���ʱ�Զ���ӳ�4�ı���,������4�������չ.
	FILE *fp;
	BITMAPFILEHEADER FileHeader;
	BITMAPINFOHEADER BmpHeader;
	bool Suc = true;
	int i, extend;
	BYTE *pCur;

	// Open File
	if ((fp = fopen(filename, "w+b")) == NULL) { return false; }
	// Fill the FileHeader
	FileHeader.bfType = ((WORD)('M' << 8) | 'B');
	FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	FileHeader.bfSize = FileHeader.bfOffBits + m_width*m_height * 3L;
	FileHeader.bfReserved1 = 0;
	FileHeader.bfReserved2 = 0;
	if (fwrite((void *)&FileHeader, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER)) Suc = false;
	// Fill the ImgHeader
	BmpHeader.biSize = 40;
	BmpHeader.biWidth = m_width;
	BmpHeader.biHeight = m_height;
	BmpHeader.biPlanes = 1;
	BmpHeader.biBitCount = 24;
	BmpHeader.biCompression = 0;
	BmpHeader.biSizeImage = 0;
	BmpHeader.biXPelsPerMeter = 0;
	BmpHeader.biYPelsPerMeter = 0;
	BmpHeader.biClrUsed = 0;
	BmpHeader.biClrImportant = 0;
	if (fwrite((void *)&BmpHeader, 1, sizeof(BITMAPINFOHEADER), fp) != sizeof(BITMAPINFOHEADER)) Suc = false;
	// write image data
	extend = ((m_width * 3 + 3) / 4 * 4) - m_width * 3;
	if (extend == 0)
	{
		for (pCur = pRGBImg + (m_height - 1) * 3 * m_width; pCur >= pRGBImg; pCur -= 3 * m_width)
		{
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // ��ʵ������
		}
	}
	else
	{
		for (pCur = pRGBImg + (m_height - 1) * 3 * m_width; pCur >= pRGBImg; pCur -= 3 * m_width)
		{
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // ��ʵ������
			for (i = 0; i<extend; i++) // ���������
			{
				if (fwrite((void *)(pCur + 3 * (m_width - 1) + 0), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);
	return Suc;
}

//��ÿ��
int KxyImgRW::GetHeight()
{
	return m_height;
}
int KxyImgRW::GetWidth()
{
	return m_width;
}
