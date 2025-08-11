// Cmp3.h: interface for the Cmp3 class.
//
//////////////////////////////////////////////////////////////////////
#include "mpg123.h"
#include "interface.h"
#include "lame.h"
#include "Structs.h"

#if !defined(AFX_CMP3_H__F8AF40CE_021F_48A9_99FF_6E643651D375__INCLUDED_)
#define AFX_CMP3_H__F8AF40CE_021F_48A9_99FF_6E643651D375__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class Cmp3  
{
public:

#define MP3MASK		0x0F0CFFFF
#define MP3CMPDWORD	0x0408E3FF

lame_global_flags *gfp;
MPSTR mp;
Cmp3();
virtual ~Cmp3();
int samples_read;
int bitrate;
int buffer[2][576];
int *p;
int iters;
int imp3;
int bytes_per_sample;
unsigned char *ip;
const int b;
int *op;
int i, rez, ret;
int InitDecode(BOOL RevSter);
int InitEncode(int btrt);
int ReinitEncode();
int CloseDecode();
int CloseEncode();
int encode2304(int* sample_buffer, unsigned char* mp3buf,int* mp3sz);
int decode(unsigned char* mp3buf,int imp3,char* pcm,int* pcmsz);
int AlignFirstFrame(unsigned char** ppmp3,DWORD *SzMp3);
BOOL ReverseStereo;
short *ppcm;
PPanelData ppd;
mp3data_struct mp3dstruct;
hip_t h_dec;


private:
short leftbuf[65535], rightbuf[65535];
int ToStereo(int Samples);

};

#endif // !defined(AFX_CMP3_H__F8AF40CE_021F_48A9_99FF_6E643651D375__INCLUDED_)
