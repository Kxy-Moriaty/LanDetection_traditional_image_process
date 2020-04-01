#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include<algorithm>
#include <string.h>
#include <iostream>
#include <math.h>
#include <time.h>

#ifndef BMP_FILE_H
#define BMP_FILE_H

BYTE *RmwRead8BitBmpFile2Img(const char * filename,int *width,int *height);
bool RmwWriteByteImg2BmpFile(BYTE *pImg,int width,int height,const char * filename);

BYTE *RmwRead24BitBmpFile2Img(const char * filename,int *width,int *height);
bool RmwWrite24BitImg2BmpFile(BYTE *pImg,int width,int height,const char * filename);

#endif
