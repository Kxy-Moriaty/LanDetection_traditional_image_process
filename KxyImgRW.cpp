#define _CRT_SECURE_NO_WARNINGS
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// KxyImgRW.cpp : 读写图像
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
// 构造/析构
//
////////////////////////////////////////////////////////////////////////////////////////////////////
KxyImgRW::KxyImgRW()
{
	//<在构造函数中把所有的成员变量赋初值>
	//是否初始化成功
	m_isInitOK = false;
	//图像属性
	m_width = 0;
	m_height = 0;
	m_size = 0;
	//内存
	m_pRGBImg = NULL;
	m_memSize = 0;
}
void KxyImgRW::Dump()
{
	//为多次的内存申请释放写一个专门的内存释放函数
	if (m_pRGBImg) { delete m_pRGBImg; m_pRGBImg = NULL; }
	m_memSize = 0;
}
KxyImgRW::~KxyImgRW() 
{
	//析构函数释放所有内存
	Dump();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////
//// 初始化，内存申请
////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//bool KxyImgRW::initialize(const char* filename)
//{
//	//BmpHeader记录的真实的宽度,
//	//读出的是真实的宽度,去掉了4对齐的扩展.
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
//	// 获取图像高宽信息，申请内存
//	m_width = w = BmpHeader.biWidth;
//	m_height = h = BmpHeader.biHeight;
//	extend = ((w * 3 + 3) / 4 * 4) - w * 3;
//	m_size = (m_width)*(m_height)* 3;
//	fseek(fp, FileHeader.bfOffBits, SEEK_SET);
//	//如果图像大小小于等于已开内存，则不用再申请内存
//	if (m_size > m_memSize) {
//		//先释放
//		Dump();
//		//在申请
//		m_pRGBImg = new BYTE[m_size];
//		m_memSize = m_size;
//	}
//	//初始化成功标志
//	if (m_pRGBImg) m_isInitOK = true;
//
//	fclose(fp);
//	return true;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// 读写操作
//
////////////////////////////////////////////////////////////////////////////////////////////////////

//读操作中动态申请内存，若当前读取图像大小大于已申请的内存，则重新申请内存，否则不申请新内存
BYTE* KxyImgRW::ReadRGBImg(const char* filename)
{
	//BmpHeader记录的真实的宽度,
	//读出的是真实的宽度,去掉了4对齐的扩展.
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
	// 读图
	m_width = w = BmpHeader.biWidth;
	m_height = h = BmpHeader.biHeight;
	extend = ((w * 3 + 3) / 4 * 4) - w * 3;
	m_size = (m_width)*(m_height)* 3;
	fseek(fp, FileHeader.bfOffBits, SEEK_SET);
	//如果图像大小小于等于已开内存，则不用再申请内存
	if (m_size > m_memSize) {
		//先释放
		Dump();
		//在申请
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
		for (k = 0; k<extend; k++) // 扩充的数据
		{
			if (fwrite((void *)&tmp, sizeof(BYTE), 1, fp) != 1) Suc = false;
		}
	}

	fclose(fp);
	return m_pRGBImg;
}
//写图像
bool KxyImgRW::WriteRGBImg(const char* filename)
{
	//BmpHeader记录的真实的宽度
	//当宽度不是4的倍数时自动添加成4的倍数,进行了4对齐的扩展.
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
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // 真实的数据
		}
	}
	else
	{
		for (pCur = m_pRGBImg + (m_height - 1) * 3 * m_width; pCur >= m_pRGBImg; pCur -= 3 * m_width)
		{
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // 真实的数据
			for (i = 0; i<extend; i++) // 扩充的数据
			{
				if (fwrite((void *)(pCur + 3 * (m_width - 1) + 0), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);
	return Suc;
}

//写图像
bool KxyImgRW::WriteRGBImg(BYTE *pRGBImg, const char* filename)
{
	//BmpHeader记录的真实的宽度
	//当宽度不是4的倍数时自动添加成4的倍数,进行了4对齐的扩展.
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
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // 真实的数据
		}
	}
	else
	{
		for (pCur = pRGBImg + (m_height - 1) * 3 * m_width; pCur >= pRGBImg; pCur -= 3 * m_width)
		{
			if (fwrite((void *)pCur, 1, m_width * 3, fp) != (unsigned int)(3 * m_width)) Suc = false; // 真实的数据
			for (i = 0; i<extend; i++) // 扩充的数据
			{
				if (fwrite((void *)(pCur + 3 * (m_width - 1) + 0), 1, 1, fp) != 1) Suc = false;
			}
		}
	}
	// return;
	fclose(fp);
	return Suc;
}

//获得宽高
int KxyImgRW::GetHeight()
{
	return m_height;
}
int KxyImgRW::GetWidth()
{
	return m_width;
}
