#include <atomic>
#include "Winsock2.h"
#include "ws2tcpip.h"
#include "windows.h"
#include "cmp3.h"
#include "Structs.h"
#include "AuxWnd.h"

//#define PIPED_SIZE 600000
UINT PIPED_SIZE = 100000;
Cmp3 mp3d;
BOOL IsMp3dClear = true;
static DWORD NOBR, NOBW;
static int rez;
static HANDLE hGCT, hGCP;
extern HANDLE hRPiped, hWPiped;
unsigned char mp3src[4096];
extern HANDLE hRPipemp3;
extern int ReverseStereo;
extern UINT BlockSizeIn;
static UINT BytesInBuffer;
std::atomic<UINT>BytesInBufferA;
extern SOCKET sockSNDR;
char pcm[163840];
char *curpcm;
unsigned char *curmp3;
int pcmsize;
extern PanelData PanDat;
//U
int fault=0;
int IsFirstFrame;
int BugMp3=-1;
int ii=0;
//U
UINT pcmready;
//FILE *fmp3d;

void ExitDecode();
int RecFrom(char* mp3src,int mp3sz);

unsigned long _stdcall ThDecode(void*){
	
hGCP=GetCurrentProcess();
hGCT=GetCurrentThread();
SetPriorityClass(hGCP,REALTIME_PRIORITY_CLASS);
SetThreadPriority(hGCT,THREAD_PRIORITY_TIME_CRITICAL);

CreatePipe(&hRPiped,&hWPiped,NULL,PIPED_SIZE);
BytesInBuffer = 0;
BytesInBufferA.store(0);

PanDat.hWPiped = hWPiped;
mp3d.ppd=&PanDat;
mp3d.InitDecode(ReverseStereo);

curpcm=pcm;
pcmready=0;
IsFirstFrame=1;
DWORD WFBW;

   while(1){
	   if(!(NOBR=RecFrom((char*)mp3src,512))) //признак невалидного сокета и следовательно
		   	ExitDecode(); //сигнал к завершению потока
	//ReadFile(hRPipemp3,mp3src,512,&NOBR,NULL);
	//U
	//fwrite(mp3src, 1, NOBR, fmp3d);
	//U
	curmp3=mp3src;
	if(IsFirstFrame){
		IsFirstFrame=!(mp3d.AlignFirstFrame(&curmp3,&NOBR));
	}
	if(NOBR){
		
		if(PanDat.IsRecord && !PanDat.IsPause && PanDat.IsAudioMode && !PanDat.IsContinuousRec){//непрерывная аудиозапись только в PCM
			
			WriteFile(PanDat.hfmp3,curmp3,NOBR,&WFBW,NULL);
			PanDat.FileSize+=WFBW;
		}
		
		mp3d.decode(curmp3,NOBR,curpcm,&pcmsize);
		IsMp3dClear = false;
	}
		if(!pcmsize){
			fault++;
			if(fault>20){
				ReverseStereo^=1;
				mp3d.CloseDecode();
				mp3d.InitDecode(ReverseStereo);
				Sleep(50);
				fault=0;
			}
		}
		else{
			fault=0;
		}
	curpcm+=pcmsize;
	pcmready+=pcmsize;
	  while(pcmready>=BlockSizeIn){
		  if(BytesInBufferA.load() >= PIPED_SIZE){
			  curpcm=pcm;
			  pcmready=0;
			  break;}
		  WriteFile(hWPiped,pcm,pcmready,&NOBW,NULL);
		  BytesInBufferA += NOBW;
		  curpcm=pcm;
		  pcmready=0;
		  break;
	  }

   }//while(1)


return 0;
}

//////////////////////////int RecFrom(char* mp3src,int mp3sz)//////////////////////////////////
int RecFrom(char* mp3src,int mp3sz){
int rezr=0;
int rezs;
int gso;
BOOL Nagle;
int ilen;
fd_set rdfd;
timeval SockTime;

if(sockSNDR==INVALID_SOCKET){return 0;}

STSEL:
SockTime.tv_sec = 2;
SockTime.tv_usec = 0;
FD_ZERO(&rdfd);
FD_SET(sockSNDR, &rdfd);
rezs = select(0, &rdfd, NULL, NULL, &SockTime);

if (rezs == 1) {
	if (FD_ISSET(sockSNDR, &rdfd)) {
		rezr = recv(sockSNDR, mp3src, mp3sz, 0);
	}
}
else {
	if (rezs== 0 && !IsMp3dClear) {
		//Закомментировано, т.к. иногда вызывает глюк в начале воспроизведения после простоя, т.к. mp3-енкодер(на серверной стороне) не переинициализируется
		//mp3d.CloseDecode(); //очистка mp3-декодера, если более 2 сек. не поступали данные
		//mp3d.InitDecode(ReverseStereo);
		IsMp3dClear = true;
	}
	goto STSEL;
}

//rezr=recv(sockSNDR,mp3src,mp3sz,0);
ilen=sizeof(BOOL);
gso=getsockopt(sockSNDR,IPPROTO_TCP,TCP_NODELAY,(char*)&Nagle,&ilen);
Nagle=TRUE;
PrnParam2(rezr);

if(!rezr || rezr==SOCKET_ERROR )
return 0;

return rezr;
}

///////////////////////////////////ExitDecode()//////////////////////////////////////////////////
void ExitDecode(){
		if(sockSNDR!=INVALID_SOCKET){//если сокет типа нормальный
		closesocket(sockSNDR); //то все равно закрываем его, т.к. он уже выдал ошибку при приёме данных (RecFrom
		sockSNDR=INVALID_SOCKET;
		}
		CloseHandle(hWPiped);
		CloseHandle(hRPiped);
		hWPiped=hRPiped=INVALID_HANDLE_VALUE;
		//mp3d.CloseDecode
		ExitThread(0);
	
   }