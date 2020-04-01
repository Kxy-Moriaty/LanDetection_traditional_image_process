#pragma once
#ifndef Kxy_IMG_RW
#define Kxy_IMG_RW

#include <Windows.h>
using namespace std;

class KxyImgRW 
{
public:
	//构造/析构
	KxyImgRW();
	~KxyImgRW();
	//读/写
	BYTE* ReadRGBImg(const char* filename);
	bool WriteRGBImg(const char* filename);
	bool WriteRGBImg(BYTE *pRGBImg, const char* filename);
	//初始化
//	bool initialize(const char* filename);
	//获得宽高
	int GetWidth();
	int GetHeight();
private:
	//初始化成功
	bool m_isInitOK;
	//内存
	BYTE* m_pRGBImg;
	int m_memSize;//已申请内存大小
	//图像属性
	int m_width;
	int m_height;
	int m_size;

private:
	//内存释放
	void Dump();
};

#endif // !Kxy_IMG_RW
