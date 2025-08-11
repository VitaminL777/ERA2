#include "windows.h"
#include <stdio.h>
#include <mmsystem.h>
#include "encode.h"
#include "AuxWnd.h"
#include "LogMsg.h"

//#define DEBUG_MODE

static int k;
static int h, c, nnn;
static HANDLE hGCT, hGCP;
HANDLE hEventIn;
HWAVEIN hWID;
WAVEFORMATEX wfxin;
SYSTEM_INFO si;
WAVEHDR HeaderIn[2];
WAVEHDR *curhdrin;
int lastin;
DWORD WaitTime=1000;
DWORD NOBW;
HANDLE hRPipe, hWPipe;
char *pcharpipe;
UINT CountSilentBlocks=0;
int IsSilenceBlock_ = 0;
extern UINT WID, WOD;
extern int nBlockSize;
extern int TresholdOfSilent;
extern UINT BlockSizeIn;
extern int SendSilentBlocks;
extern int MixToMono;
extern HANDLE hThEncode;
extern DWORD ThEncodeID;
extern BOOL IsRRmp3;
extern HANDLE hRWPipe;

int IsSilentBlock(__int16* pcmin);
void ToMono(short *Pcm, int Size);

unsigned long _stdcall ThCapture(void*){

hGCP=GetCurrentProcess();
hGCT=GetCurrentThread();
SetPriorityClass(hGCP,REALTIME_PRIORITY_CLASS);
SetThreadPriority(hGCT,THREAD_PRIORITY_TIME_CRITICAL);
CloseHandle(hGCT);
CloseHandle(hGCP);

CreatePipe(&hRPipe,&hWPipe,NULL,65536);

hThEncode=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThEncode,NULL,0,&ThEncodeID);
CloseHandle(hThEncode);

hEventIn = CreateEvent(0, FALSE, FALSE, 0);
wfxin.wFormatTag=WAVE_FORMAT_PCM;
wfxin.nChannels=2;
wfxin.nSamplesPerSec=8000;
wfxin.nAvgBytesPerSec=16000;
wfxin.nBlockAlign=4;//2;
wfxin.wBitsPerSample=16;
wfxin.cbSize=0;

OPENWID:
if(waveInOpen(&hWID,WID,&wfxin,(DWORD)hEventIn,0,CALLBACK_EVENT)){
Sleep(2500);
goto OPENWID;
}

GetSystemInfo(&si);
HeaderIn[0].lpData=(LPSTR) VirtualAlloc(0, 
			(BlockSizeIn*2+si.dwPageSize-1)/si.dwPageSize*si.dwPageSize,
			MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
HeaderIn[1].lpData = HeaderIn[0].lpData+BlockSizeIn;
HeaderIn[0].dwBufferLength = HeaderIn[1].dwBufferLength = BlockSizeIn;
waveInPrepareHeader(hWID, HeaderIn, sizeof(WAVEHDR));
waveInPrepareHeader(hWID, HeaderIn+1, sizeof(WAVEHDR));

waveInAddBuffer(hWID, HeaderIn, sizeof(WAVEHDR));
waveInAddBuffer(hWID, HeaderIn+1, sizeof(WAVEHDR));
lastin=1;
ResetEvent(hEventIn);
waveInStart(hWID);

while(1){
c=0;
WaitForSingleObject(hEventIn,WaitTime);	
  while(c<2){
	  c++;
	  h=lastin+1;
		curhdrin=HeaderIn+((h++)&1);
		pcharpipe=curhdrin->lpData;
	 if(curhdrin->dwFlags&WHDR_DONE){
		for(k=0;k<nBlockSize;k++){
			IsSilenceBlock_ = IsSilentBlock((__int16*)pcharpipe);
			if (MixToMono && ((!IsSilenceBlock_ || CountSilentBlocks < 9) || IsRRmp3)) {
				ToMono((short*)pcharpipe, 2304);
			}
			if (!IsSilenceBlock_ || CountSilentBlocks < 9) {
				WriteFile(hWPipe, pcharpipe, 2304, &NOBW, NULL);
			}
			if (IsRRmp3) {
				WriteFile(hRWPipe, pcharpipe, 2304, &NOBW, NULL); //для записи в mp3 на серверной стороне
			}
		 pcharpipe+=2304;
		}
	
		waveInAddBuffer(hWID, curhdrin, sizeof(WAVEHDR));
		if(curhdrin==HeaderIn){lastin=0;}
		else{lastin=1;}
	 }
  }//end_while(c<2)

}//end_while(1)


	return 0;
}



//***********************************int IsSilentBlock(int* pcmin)********************************
int IsSilentBlock(__int16* pcmin){
int FullBlockSize;
__int16 diff;

if(SendSilentBlocks){return 0;}
if(CountSilentBlocks > 100){CountSilentBlocks--;}
FullBlockSize=576;
while(FullBlockSize--){
	diff = *pcmin - *(pcmin + 1);
	pcmin++;
	if (diff < 0) {
		diff = ~diff; }
	if(diff > TresholdOfSilent){

	CountSilentBlocks=0;
	return 0;
	}	
}
CountSilentBlocks++;
return 1;
}

//=====================================================================================================
void ToMono(short *Pcm, int Size) {
	short shsum;
	long lsum;

	for (int i = 0; i < Size / 2; i += 2) {
		lsum = Pcm[i] + Pcm[i + 1];
		if (lsum > 32767) {
			shsum = 32767;
		}
		else if (lsum < -32767) {
			shsum = -32767;
		}
		else {
			shsum = (short)lsum;
		}
		Pcm[i] = Pcm[i + 1] = shsum;
	}

	return;
}