#define HAVE_MEMCPY //для исключения переопределения memcpy в mpg123.h(из cmp3.h)
#include <atomic>
#include <windows.h>
#include "cmp3.h"
#include "AuxWnd.h"
#include <dsound.h>
#include <mmreg.h>

#define SND_ALLOW 2

extern UINT PIPED_SIZE;
char buff[128000];
static DWORD NOBR, NOBW;
HANDLE hEventPlay;
HANDLE hRPiped, hWPiped;
HWAVEOUT hPlay;
WAVEHDR Header[2];
LPSTR Addr=0;
SIZE_T S_T; 
static SYSTEM_INFO si;
WAVEFORMATEX wfx;
char NullPCM[8192];
extern int ECPRetCode;
	
static int rez;
int j;
int out1, out2;
Cmp3 mp3p;
///U
FILE *fraw;
UINT SizeRaw=0;
///U
UFileTime TSrvPlayStop;

extern HANDLE hRPipemp3;
extern UINT BlockSizeIn;
extern BOOL IsLocalMonoMode;
extern UINT WOD;
extern DWORD WaitTime;
extern int mp3bitrate;
static UINT BytesInBuffer;
extern std::atomic<UINT>BytesInBufferA;
extern int WaitBufferSize;
extern char WODName[64];
BOOL TriggerFillEmpty = 1;
int TriggerCount = 0;
extern PanelData PanDat;

int GetPcm(LPSTR pcmfifo, BOOL WaitFillBuff);
char PcmSndFileName[256] = "C:\\Temp\\EraSnd.wav";
int WritePcmSnd(char* Snd, int SizeSnd);

LPDIRECTSOUND DS=NULL;
LPGUID LPCard=NULL;
void EnableDirectXFilter();
void wOW(HWAVEOUT hPlay, LPWAVEHDR Header, UINT Sz);
bool IsControlStopEvent = false;
void ClearPipeBuffer(/*HANDLE hRPiped*/);
void PlayReset(/*HANDLE hRPiped, HWAVEOUT hPlay, WAVEHDR Header, HANDLE hEventPlay*/);

//UU
DWORD TicCur1, TicCur1Prev;
DWORD TicCur2, TicCur2Prev;
BOOL IsRecStart = FALSE;
HANDLE hRFT = NULL;
void RecFrameTime();
DWORD TicStart;
DWORD TicPrev;
//UU

unsigned long _stdcall ThPlay(void*) {
	//using namespace play;
	BytesInBuffer = 0;
	BytesInBufferA.store(0);

	hRPiped = hWPiped = INVALID_HANDLE_VALUE;

	hEventPlay = CreateEvent(0, FALSE, FALSE, 0);
	ZeroMemory(NullPCM, BlockSizeIn);

	GetSystemInfo(&si);
	Header[0].lpData = (LPSTR)VirtualAlloc(0,
		(BlockSizeIn * 2 + si.dwPageSize - 1) / si.dwPageSize*si.dwPageSize,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	Header[1].lpData = Header[0].lpData + BlockSizeIn;
	Header[0].dwBufferLength = Header[1].dwBufferLength = BlockSizeIn;

	ZeroMemory((void*)&wfx, sizeof(wfx));
	wfx.wFormatTag = 1;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 8000;
	wfx.nAvgBytesPerSec = 16000;
	wfx.nBlockAlign = 4;//2;
	wfx.wBitsPerSample = 16;
	wfx.cbSize = 0;
	if (waveOutOpen(&hPlay, WOD, &wfx, (DWORD)hEventPlay, 0, CALLBACK_EVENT)) { return 0; }
	rez = waveOutPrepareHeader(hPlay, Header, sizeof(WAVEHDR));
	rez = waveOutPrepareHeader(hPlay, Header + 1, sizeof(WAVEHDR));

	EnableDirectXFilter(); //исключает глюк драйверов дешёвых VIA-карточек, на которых не включается ФНЧ 
	//при работе через api из Winmm.dll, - элементарный вызов DirectSound - включает фильтр
	//***************************************************************************************************
PLAYSTART:
	GetPcm(Header->lpData, 1);
	GetPcm((Header + 1)->lpData, 0);
	wOW(hPlay, Header, sizeof(WAVEHDR));
	wOW(hPlay, Header + 1, sizeof(WAVEHDR));

	while (1) {
	WFSO:
			
		WaitForSingleObject(hEventPlay, WaitTime);
		if (IsControlStopEvent) {
			IsControlStopEvent = false;
			PlayReset();
		}
		
		if ((Header->dwFlags&WHDR_DONE) && ((Header + 1)->dwFlags&WHDR_DONE)) {
			goto PLAYSTART;
		}

		if (Header->dwFlags&WHDR_DONE) {
			GetPcm(Header->lpData, 0);
			if ((Header->dwFlags&WHDR_DONE) && ((Header + 1)->dwFlags&WHDR_DONE)) {
				GetPcm((Header + 1)->lpData, 1);
				wOW(hPlay, Header, sizeof(WAVEHDR));
				wOW(hPlay, Header + 1, sizeof(WAVEHDR));
				goto WFSO;
			}
			wOW(hPlay, Header, sizeof(WAVEHDR));
		}

		if ((Header + 1)->dwFlags&WHDR_DONE) {
			GetPcm((Header + 1)->lpData, 0);
			if ((Header->dwFlags&WHDR_DONE) && ((Header + 1)->dwFlags&WHDR_DONE)) {
				GetPcm(Header->lpData, 1);
				wOW(hPlay, Header + 1, sizeof(WAVEHDR));
				wOW(hPlay, Header, sizeof(WAVEHDR));
				goto WFSO;
			}
			wOW(hPlay, Header + 1, sizeof(WAVEHDR));
		}

	}

	return 0;
}

//=======================================================================================
void wOW(HWAVEOUT hPlay, LPWAVEHDR Header, UINT Sz) {
UFileTime TCur;

	if (TSrvPlayStop.int64 == 0 || PanDat.IsRecord) {
		waveOutWrite(hPlay, Header, Sz);
		TSrvPlayStop.int64 = 0;
		return;
	}
	
	GetSystemTimeAsFileTime(&TCur.FileTime);
	if (((TCur.int64 - TSrvPlayStop.int64) / 10000) >= 2000) {
		TSrvPlayStop.int64 = 0;
	}
	PlayReset();
}

//***************************************************************************************************
int GetPcm(LPSTR pcmfifo, BOOL WaitFillBuff) {

	char *curfifo;
	UINT Reading;
	char PNPBuff[4096];
	DWORD DWPNPR;
	DWORD DWPNPA;
	DWORD DWPNPM;
	BOOL IsContinuousRec = false;

	Reading = 0;
	curfifo = pcmfifo;

	while (hRPiped == INVALID_HANDLE_VALUE || !(ECPRetCode&SND_ALLOW)) {//+защита(SND_ALLOW)
		Sleep(333); //Ждём пока клиент не подцепится к серверу и не выдаст готовый дескриптор
	}
	//u
	//PrnParam(BytesInBuffer);
	//u
	if (!PanDat.IsContinuousRec) {
		while ((BytesInBufferA.load() < (UINT)WaitBufferSize && WaitFillBuff) && !PanDat.IsContinuousRec) {
			Sleep(72);
		}
	}
	if (PanDat.IsContinuousRec) {
		if (!TriggerFillEmpty) {
			goto DO_GP;
		}
		//if(TriggerFillEmpty)
		if (BytesInBufferA.load() < (UINT)WaitBufferSize) {
			memcpy(pcmfifo, NullPCM, BlockSizeIn);
			WritePcmSnd(pcmfifo, BlockSizeIn);
			return 1;
		}
		TriggerFillEmpty = 0;
	}

DO_GP:
	do {
		if (hRPiped == INVALID_HANDLE_VALUE) { 
			break;
		}
		IsContinuousRec = PanDat.IsContinuousRec; //запоминаем текущее состояние для сравнения после ReadFile
		if (PanDat.IsContinuousRec && (BytesInBufferA.load() < (UINT)BlockSizeIn)) {
			TriggerFillEmpty = 1;
			memcpy(pcmfifo, NullPCM, BlockSizeIn);
			WritePcmSnd(pcmfifo, BlockSizeIn);
			return 1;
		}
		PanDat.IsReadPCMLock = true;
		ReadFile(hRPiped, curfifo, BlockSizeIn - Reading, &NOBR, NULL);
		PanDat.IsReadPCMLock = false;
		if (!IsContinuousRec && PanDat.IsContinuousRec) {//за время ожидания ReadFile произошла смена режима записи на ContinuousRec
			goto DO_GP;
		}


		////////////U
		if (SizeRaw < 1600000) {
			//fwrite(curfifo, 1, NOBR, fraw);
			SizeRaw += NOBR;
		}
		else {
			//fclose(fraw);
		}

		////////////U
		BytesInBufferA -= NOBR;
		Reading += NOBR;
		curfifo += NOBR;
	} while (Reading < BlockSizeIn);

	if (PanDat.IsContinuousRec) {
		WritePcmSnd(pcmfifo, BlockSizeIn);
	}
	return 1;
}

//***************************************************************************************************
BOOL CALLBACK DSEC( LPGUID lpGuid,LPCSTR lpcstrDescription,LPCSTR lpcstrModule,LPVOID lpContext){
if(!strncmp(WODName,lpcstrDescription,strlen(WODName))) LPCard=lpGuid;
return 1;
}

void EnableDirectXFilter(){

if(strlen(WODName)) DirectSoundEnumerate(DSEC,NULL);
HRESULT hr=DirectSoundCreate(LPCard, &DS, NULL);

}


///////////////////////////int WritePcmSnd(char* Snd, int SizeSnd)//////////////////////////
int WritePcmSnd(char* Snd, int SizeSnd) {
	static DWORD NOBW;

	if (PanDat.IsRecord && !PanDat.IsPause && (PanDat.hfpcm != INVALID_HANDLE_VALUE)) {
		//UU
		//RecFrameTime();
		IsRecStart = TRUE;
		//UU
		PanDat.IsPcmFileLocked = TRUE;
		WriteFile(PanDat.hfpcm, Snd, (DWORD)SizeSnd, &NOBW, NULL);
		PanDat.NOBWPCM_ALL += NOBW;
		PanDat.IsPcmFileLocked = FALSE;
		return 1;
	}
	else {
		//UU
		if (hRFT != NULL) {
			CloseHandle(hRFT);
			hRFT = NULL;
			IsRecStart = FALSE;
		}
		//UU
		return 0;
	}
}

//////////////////////////////////void ClearPipeBuffer()//////////////////////////////////
void ClearPipeBuffer(/*HANDLE hRPiped*/) {
char *pEmpty;
DWORD DW_BR;
	
	pEmpty = (char*)malloc(PIPED_SIZE);
	ReadFile(hRPiped, pEmpty, PIPED_SIZE, &DW_BR, NULL);
	BytesInBufferA.store(0);
	free(pEmpty);
}

void PlayReset(/*HANDLE hRPiped, HWAVEOUT hPlay, WAVEHDR Header, HANDLE hEventPlay*/) {
	waveOutReset(hPlay);
	ClearPipeBuffer();
	Header->dwFlags |= WHDR_DONE;
	(Header + 1)->dwFlags |= WHDR_DONE;
	SetEvent(hEventPlay);
}


//UU_TEST
void RecFrameTime() {
char tcur[256];
DWORD tlen;
DWORD DWTW;
DWORD diff1, diff2;
UINT FP;

	if (hRFT == NULL) {
		hRFT = CreateFile("C:\\SCR\\RFT.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
	}
	diff1 = TicCur1 - TicCur1Prev;
	diff2 = TicCur2 - TicCur1;
	FP = BytesInBufferA.load();
	sprintf(tcur, "Tin(diff)=%dms WFSO_wait=%dms BuffFill=%dbyte\n", diff1, diff2, FP);
	tlen =(DWORD)strlen(tcur);
	WriteFile(hRFT, tcur, tlen, &DWTW, NULL);

}