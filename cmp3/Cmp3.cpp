// Cmp3.cpp: implementation of the Cmp3 class.
//
//////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "Cmp3.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Cmp3::Cmp3():b(sizeof(int) * 8)
{
ZeroMemory(buffer,sizeof(buffer));
ZeroMemory(leftbuf,sizeof(leftbuf));
ZeroMemory(rightbuf,sizeof(rightbuf));
ReverseStereo=0;

}

Cmp3::~Cmp3()
{


}
//****************encode2304***************************************************
int Cmp3::encode2304(int* sample_buffer, unsigned char* mp3buf,int* mp3sz)
{

__int16 *p16, *p1, *p2;
p16=(__int16*)sample_buffer;
p1=(__int16*)buffer+1;
p2=(__int16*)buffer[1]+1;
	for(i=0;i<576;i++){
	*p1=*p16++;
	*p2=*p16++;
	p1+=2;
	p2+=2;
	}

*mp3sz = lame_encode_buffer_int(gfp, buffer[0], buffer[1], 576, mp3buf, 4096);

return 0;
}

//****************decode*********************************************************

int Cmp3::decode(unsigned char* mp3buf,int imp3,char* pcm,int* pcmsz)
{
int ld;
int size;
extern int BugMp3;


ppcm= (short*)pcm;
size=0;

//ld=lame_decode1_headers(mp3buf, imp3, leftbuf, rightbuf,&mp3dstruct);
ld = hip_decode1_headers(h_dec, mp3buf, imp3, leftbuf, rightbuf, &mp3dstruct);
if(ld){
	ToStereo(ld);
	size=ld*4;
	}
if(!ld){
	*pcmsz=0;
	return 0;
}
//ld=lame_decode1(mp3buf, 0, leftbuf, rightbuf);
ld = hip_decode1(h_dec, mp3buf, 0, leftbuf, rightbuf);
if(!ld){
	*pcmsz=size;
	return size;
}
ToStereo(ld);
size+=(ld*4);
while(ld){
	//ld=lame_decode1(mp3buf, 0, leftbuf, rightbuf);
	ld = hip_decode1(h_dec, mp3buf, 0, leftbuf, rightbuf);
	if(ld){
		ToStereo(ld);
		size+=(ld*4);
	}
}

*pcmsz=size;
return size;

}

//*******************InitEncode*****************************************************
int Cmp3::InitEncode(int btrt){

//extern int MixToMono;
//b=sizeof(int) * 8; //not works
bitrate=btrt;
bytes_per_sample = 2;
gfp = lame_init();

//1

lame_set_VBR(gfp, vbr_default);
lame_set_VBR_quality(gfp,0);
lame_set_VBR_max_bitrate_kbps(gfp, bitrate);
lame_set_brate(gfp,bitrate);
lame_set_VBR_min_bitrate_kbps(gfp,bitrate-8);
lame_set_findReplayGain(gfp, 1);
lame_set_num_channels(gfp,2);
lame_set_in_samplerate(gfp,8000);
lame_set_write_id3tag_automatic(gfp,0);
/*
if(MixToMono)
lame_set_mode(gfp,(MPEG_mode)MONO);
else
*/
lame_set_mode(gfp,(MPEG_mode)JOINT_STEREO);

lame_init_params(gfp);


rez=lame_get_mode(gfp);
rez=lame_get_VBR_min_bitrate_kbps(gfp);
rez=lame_get_VBR_max_bitrate_kbps(gfp);
rez=lame_get_in_samplerate(gfp);
rez=lame_get_VBR(gfp);
rez=lame_get_findReplayGain(gfp);
rez=lame_get_framesize(gfp);
rez=lame_get_VBR(gfp);


return 1;

}

//*******************CloseEncode*****************************************************
int Cmp3::CloseEncode(){
	lame_close(gfp);
return 1;
}

//*******************int ReinitEncode()*********************************************
int Cmp3::ReinitEncode(){
	lame_close(gfp);
	InitEncode(bitrate);
return 1;
}

//*******************InitDecode*****************************************************
int Cmp3::InitDecode(BOOL RevSter){

	//lame_decode_init();
	h_dec = hip_decode_init();
	ReverseStereo=RevSter;

	return 1;
}

//*******************CloseDecode*****************************************************
int Cmp3::CloseDecode(){
	//lame_decode_exit();
	hip_decode_exit(h_dec);
	return 1;
}


//*******************ToStereo*****************************************************
int Cmp3::ToStereo(int Samples){

int i;
short *psl, *psr;

if(mp3dstruct.mode==3){//пришёл моно-фрейм
psr=leftbuf;
psl=leftbuf;
//ppd->RealMonoMode=1; //для правильного отображения моно-кнопки на панели
goto TOST;
}

//пришёл стерео-фрейм
//ppd->RealMonoMode=0; //для правильного отображения моно-кнопки на панели
if(ReverseStereo){
psl=rightbuf;
psr=leftbuf;
}
else{
psl=leftbuf;
psr=rightbuf;
}

TOST:
for(i=0;i<Samples;i++){
*ppcm++=*psl++;
*ppcm++=*psr++;

}

	return 1;
}

//*******************AlignFirstFrame************************************************

int Cmp3::AlignFirstFrame(unsigned char** ppmp3,DWORD *SzMp3){

unsigned char* pmp3;
DWORD ReSz;
DWORD CmpDW;
DWORD *pCmpDW;
	
ReSz=*SzMp3;
pmp3=*ppmp3;

while(ReSz){
	if(*pmp3==0xFF){
		pCmpDW=(DWORD*)pmp3;
		CmpDW=*pCmpDW;
		CmpDW&=MP3MASK;
		if(CmpDW==MP3CMPDWORD){goto AFFEXIT;}
	}
	pmp3++;
	ReSz--;
}
*SzMp3=0;
return 0;
AFFEXIT:
*ppmp3=pmp3;
*SzMp3=ReSz;
return 1;
}