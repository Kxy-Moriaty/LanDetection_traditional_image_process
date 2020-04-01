#pragma once
#ifndef Kxy_IMG_RW
#define Kxy_IMG_RW

#include <Windows.h>
using namespace std;

class KxyImgRW 
{
public:
	//����/����
	KxyImgRW();
	~KxyImgRW();
	//��/д
	BYTE* ReadRGBImg(const char* filename);
	bool WriteRGBImg(const char* filename);
	bool WriteRGBImg(BYTE *pRGBImg, const char* filename);
	//��ʼ��
//	bool initialize(const char* filename);
	//��ÿ��
	int GetWidth();
	int GetHeight();
private:
	//��ʼ���ɹ�
	bool m_isInitOK;
	//�ڴ�
	BYTE* m_pRGBImg;
	int m_memSize;//�������ڴ��С
	//ͼ������
	int m_width;
	int m_height;
	int m_size;

private:
	//�ڴ��ͷ�
	void Dump();
};

#endif // !Kxy_IMG_RW
