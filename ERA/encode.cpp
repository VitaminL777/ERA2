#include "Winsock2.h"
#include "ws2tcpip.h"
#include "windows.h"
#include "cmp3.h"
#include "Structs.h"


Cmp3 mp3;
Cmp3 Rmp3;
static DWORD NOBR, NOBW;
static int rez, l;
static HANDLE hGCT, hGCP;
HANDLE hRPipemp3, hWPipemp3;
extern HANDLE hRPipe;
extern HANDLE hWaitEncode;
extern int mp3bitrate;
extern int StaticGain;
extern SOCKET sockSNDS;
extern int MixToMono;
char pcmtomp3[16384];
unsigned char mp3buf[4096];
extern PanelData PanDat;
int *ptomp3;
int mp3sz;

//////U
UINT fmp3size=0;
FILE *fmp3;
////////U

int ToSend(char* mp3buf,int mp3sz);
int Gain(short *pcm, int size);
void ExitEncode();


unsigned long _stdcall ThEncode(void*){
	
hGCP=GetCurrentProcess();
hGCT=GetCurrentThread();
SetPriorityClass(hGCP,REALTIME_PRIORITY_CLASS);
SetThreadPriority(hGCT,THREAD_PRIORITY_TIME_CRITICAL);
CloseHandle(hGCT);
CloseHandle(hGCP);

///U
//fmp3 = fopen("C:\\5\\sinbmp3.mp3","wb");
////U

mp3.InitEncode(mp3bitrate);
Rmp3.InitEncode(mp3bitrate); //для записи mp3 на сервере
if(!CreatePipe(&hRPipemp3,&hWPipemp3,NULL,65536)){return 0;}

while(1){
ReadFile(hRPipe,pcmtomp3,2304,&NOBR,NULL);
 if(NOBR==2304){
	 if (StaticGain != 1) {
		 Gain((short*)pcmtomp3, 1152);
	 }
	 /*
	 while(PanDat.IsChangeMode){//Поступил запрос на смену режима кодирования(моно/стерео)
		Sleep(50);				//- ждём в цикле, пока он будет выполнен
	}
	*/
	 mp3.encode2304((int*)pcmtomp3,mp3buf,&mp3sz);
	 NOBW=ToSend((char*)mp3buf,mp3sz);
	 //ExitEncode();
	 //WriteFile(hWPipemp3,mp3buf,mp3sz,&NOBW,NULL);
	 if(fmp3size<120000){
	 //fwrite(mp3buf, 1, mp3sz, fmp3);
	 //fmp3size+=mp3sz;
	 }
	 else{
		 //fclose(fmp3);
	 }
 }
}


Sleep(-1);


return 0;
}

////////////////////////////////int ToSend(char mp3buf,int mp3sz)////////////////////////////////////////
int ToSend(char* mp3buf,int mp3sz){	
int rezs=0;

if(sockSNDS==INVALID_SOCKET){return -1;}

	rezs=send(sockSNDS,mp3buf,mp3sz,0);
	if(rezs==SOCKET_ERROR )
	return -1;

	return rezs;
}

//////////////////////////////////////////void ExitEncode()///////////////////////////////////////////////
void ExitEncode(){
	if(sockSNDS!=INVALID_SOCKET){
		closesocket(sockSNDS);
		sockSNDS=INVALID_SOCKET;
		}
		CloseHandle(hWPipemp3);
		CloseHandle(hRPipemp3);
		hWPipemp3=hRPipemp3=INVALID_HANDLE_VALUE;
		mp3.CloseEncode();
		ExitThread(0);


}

////////////////////////////////////////int Gain(short *pcm, int size)///////////////////////////////////
int Gain(short *pcm, int size) {
int IntPcm;
int i;

for (i = 0; i < size; i++) {
	IntPcm = (int)*pcm;
	IntPcm *= StaticGain;
	if (IntPcm > 32767) {
		IntPcm = 32767;
	}
	if (IntPcm < -32767) {
		IntPcm = -32767;
	}
	*pcm = (short)IntPcm;
	pcm++;
}
	return 1;
}
