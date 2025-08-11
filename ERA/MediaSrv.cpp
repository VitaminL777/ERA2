#include <atomic>
#include <windows.h>
#include <vector>  // подключили библиотеку
#include <algorithm>  // sort
#include "structs.h"
#define HAVE_MEMCPY  //для запрета переопределения функции memcpy в cmp3.h
#include "cmp3.h"
#include "control.h"
#include "MediaSrv.h"
#include "Proc.h"
#include "funcs.h"
#include "LogMsg.h"


using namespace std;

HANDLE hThMediaSrv;
HANDLE hThMSrvVideo;
HANDLE hThMediaCopy;
DWORD ThMediaSrvID = 0;
DWORD ThMSrvVideoID = 0;
DWORD ThMediaCopyID = 0;
DWORD ThMSrvSendProgrID = 0;
BOOL RRecStart = FALSE;
BOOL RRecStop = FALSE;
BOOL IsRRecVideo = FALSE;
BOOL IsRRecAudio = FALSE;
BOOL IsMp3Valid = FALSE;
BOOL IsMp4Valid = FALSE;
BOOL IsUnionValid = FALSE;
BOOL IsRRmp3 = FALSE; //флаг, указывающий на необходимость записи второго потока pcm в capture.cpp (для серверного mp3)
BOOL IsRRecMp3Stop;
BOOL IsRRecMp4Stop;
BOOL CancelNetCopy = FALSE;
HANDLE hRRPipe;
HANDLE hRWPipe;
HANDLE hROut, hWOut, hRIn, hWIn;
HANDLE hRmp3;
HANDLE hRmp4Ev = NULL;
HANDLE hEvMSSP = NULL;
U64D UDSSP;
atomic<U64D> UDSSPA;
U64D LastNetCopy;
BOOL IsQSent = FALSE;
STARTUPINFO si;
PROCESS_INFORMATION pi;
char FFBin[256] = "ffmpeg.exe";
char FFPreset[1500] = "-y -f gdigrab -r 2 -i desktop -c:v h264  -preset veryslow -pix_fmt yuv420p";
char FileSavePath[200] = "C:\\SCR";
char FFPath[1024];
char FFConcat[] = "%s -y -i %s -i %s -c:v copy -strict -1 -c:a copy %s";
//char NetSavePath[256];
char Mp3Path[256];
char Mp3FPath[256];
char Mp4Path[256];
char Mp4UPath[256]; //Union mp4 full path
extern Cmp3 Rmp3;
extern ERAState EraState;
extern char Aerodrome[64];
extern int DebugMode;
extern NETRES NetRes;
extern BOOL IsShowTimerPanel;
Proc proc;

DWORD CopyProgress(
	LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,
	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,
	HANDLE hDestinationFile,
	LPVOID lpData
);

typedef struct _COPYFILE {
	char FName[256];
}COPYFILE;

COPYFILE cf;
vector <COPYFILE> CF ;

unsigned long _stdcall ThMSrvVideo(void*);
unsigned long _stdcall ThMediaCopy(void*);
unsigned long _stdcall ThMSrvSendProgr(void*);
void CreateFullNames();
void CreateNameFromTime(char *PName);
int RRecMp3();
int RRecMp4();
int CreateUnionMp4();
int CheckUnionValid();

unsigned long _stdcall ThMediaSrv(void*) {
	
	IsRRecMp3Stop = IsRRecMp4Stop = IsQSent = IsMp3Valid = IsMp4Valid = IsUnionValid = FALSE;
	hRIn = hWIn = hRmp4Ev = NULL;
	CreateFullNames();

	if (IsRRecVideo) {
		hThMSrvVideo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThMSrvVideo, NULL, 0, &ThMSrvVideoID);
		hRmp4Ev = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	if (IsRRecAudio) {
		IsMp3Valid = RRecMp3();
		if (!IsMp3Valid) {
			DeleteFile(Mp3Path);
		}
	}
	if (IsRRecVideo && hRmp4Ev != NULL) {
		WaitForSingleObject(hRmp4Ev, INFINITE);
		if (!IsMp4Valid) {
			DeleteFile(Mp4Path);
			DeleteFile(Mp3Path);
		}
		else if (IsRRecAudio && IsMp3Valid) {
			CreateUnionMp4(); //{..CF.push_back(cf);}
		}
	}
	if (IsMp3Valid && !IsRRecVideo) {
		MoveFile(Mp3Path, Mp3FPath);
		if (EraState.IsAutoCopy) {
			strcpy(cf.FName, Mp3FPath);
			CF.push_back(cf);
		}
	}
	if (EraState.IsAutoCopy && CF.size() && !ThMediaCopyID) {
		hThMediaCopy = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThMediaCopy, NULL, 0, &ThMediaCopyID);
	}
	CloseHandle(hThMediaSrv); ThMediaSrvID = 0;
	RRecStart = RRecStop = IsRRecVideo = IsRRecAudio = IsMp3Valid = IsMp4Valid = FALSE;
	CloseHandle(hRmp4Ev); hRmp4Ev = NULL;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long _stdcall ThMSrvVideo(void*) {
	if (RRecMp4()) { //нормальное завершение ffmpeg через "Q"
		IsMp4Valid = TRUE;
	}
	else {//аварийное завершение ffmpeg
		TerminalSendCommand(RREСTERMINATESRV, "", 0);
		RRecStopF();
		IsMp4Valid = FALSE;
	}
	SetEvent(hRmp4Ev);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateNameFromTime(char *PName)
{
	SYSTEMTIME ST;
	int i;
	i = 0;
	char tmp[92];

	GetLocalTime(&ST);
	itoa(ST.wYear, PName, 10);
	strcat(PName, "_");
	if (ST.wMonth < 10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wMonth, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "_");
	i = 0;
	if (ST.wDay < 10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wDay, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "_");
	i = 0;
	if (ST.wHour < 10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wHour, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "-");
	i = 0;
	if (ST.wMinute < 10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wMinute, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "-");
	i = 0;
	if (ST.wSecond < 10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wSecond, &tmp[i], 10);
	strcat(PName, tmp);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateFullNames()
{
	char TName[92];
	
	CreateNameFromTime(TName);
	sprintf(Mp3Path, "%s\\%s.mp3", FileSavePath, TName);
	sprintf(Mp4Path, "%s\\%s.mp4", FileSavePath, TName);
	sprintf(Mp3FPath, "%s\\%s_%s.mp3", FileSavePath, Aerodrome, TName);
	sprintf(Mp4UPath, "%s\\%s_%s.mp4", FileSavePath, Aerodrome, TName);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RRecMp3() {
	char pcmtomp3[16384];
	unsigned char mp3buf[4096];
	DWORD NOBR, NOBW, dwret;
	int mp3sz;
	int ret = 1;

	if (!Rmp3.bitrate) { //не проинициализирован экземпляр класса
		return 0;
	}
	if (!CreatePipe(&hRRPipe, &hRWPipe, NULL, 65536)) { return 0; }
	hRmp3 = CreateFile(Mp3Path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
	if (hRmp3 == INVALID_HANDLE_VALUE) {
		CloseHandle(hRRPipe); CloseHandle(hRWPipe);
		hRRPipe = hRWPipe = NULL;
		return 0;
	}
	IsRRmp3 = TRUE; //сигналим потоку в capture.cpp, чтобы записывал в наш pipe данные
	while (!IsRRecMp3Stop) {
		if (!ReadFile(hRRPipe, pcmtomp3, 2304, &NOBR, NULL)) { break; }
		Rmp3.encode2304((int*)pcmtomp3, mp3buf, &mp3sz);
		if (mp3sz) {
			if (!WriteFile(hRmp3, mp3buf, (DWORD)mp3sz, &NOBW, NULL)) { break; }
		}
	}
	if (!IsRRecMp3Stop) {
		ret = 0;
	}
	IsRRmp3 = IsRRecMp3Stop = FALSE;
	CloseHandle(hRRPipe); CloseHandle(hRWPipe); CloseHandle(hRmp3);
	hRRPipe = hRWPipe = hRmp3 = NULL;
	
	Rmp3.ReinitEncode();

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RRecMp4() {
	BOOL brz;
	int ret = 0;

	sprintf(FFPath, "%s %s %s", FFBin, FFPreset, Mp4Path);
	CreatePipe(&hRIn, &hWIn, NULL, 65536);
	brz = SetHandleInformation(hRIn, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	brz = SetHandleInformation(hWIn, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = hRIn;

	if (DebugMode) {
		brz = CreateProcess(NULL, FFPath, NULL, NULL, 1, 0, NULL, NULL, &si, &pi);
	}
	else {
		brz = proc.CreateProcActiveUser(FFPath, &si, &pi, TRUE);
	}
	if (pi.hProcess == NULL) {
		goto CH_RRMP4;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess); pi.hProcess = NULL;
	ret = IsQSent ? 1 : 0; //определяем причину завершения ffmpeg (отправка "Q" или аварийно)
CH_RRMP4:	
	CloseHandle(hRIn); CloseHandle(hWIn);
	hRIn = hWIn = NULL;
	
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CreateUnionMp4() {
	Proc proc;
	
	sprintf(FFPath, FFConcat, FFBin, Mp4Path, Mp3Path, Mp4UPath);
	proc.CreateProc(FFPath, SW_HIDE, 120000);
	if (CheckUnionValid()) {
		if (EraState.IsAutoCopy) {//добавление в базу на копирование по сети
			strcpy(cf.FName, Mp4UPath); 
			CF.push_back(cf);
		}
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CheckUnionValid() {
	WIN32_FILE_ATTRIBUTE_DATA wfad;
	SIZE64 Mp4Sz, USz;

	if (!GetFileAttributesExA(Mp4Path, GetFileExInfoStandard, &wfad)) {
		return 0;
	}
	Mp4Sz.DW.Low = wfad.nFileSizeLow;
	Mp4Sz.DW.High = wfad.nFileSizeHigh;
	if (!GetFileAttributesExA(Mp4UPath, GetFileExInfoStandard, &wfad)) {
		return 0;
	}
	USz.DW.Low = wfad.nFileSizeLow;
	USz.DW.High = wfad.nFileSizeHigh;
	if (USz.UI64 > Mp4Sz.UI64) {
		DeleteFile(Mp4Path);
		DeleteFile(Mp3Path);
		return 1;
	}
	DeleteFile(Mp4UPath);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int RRecStartF() {
	if (EraState.SelectRecMode == RRAUDIOMODE) {
		IsRRecAudio = TRUE;
	}
	else if (EraState.SelectRecMode == RRVIDEOMODE) {
		IsRRecAudio = IsRRecVideo = TRUE;
	}
	else {
		return 0;
	}
	if (!ThMediaSrvID) {
		hThMediaSrv = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThMediaSrv, NULL, 0, &ThMediaSrvID);
	}
	if (hThMediaSrv == NULL || hThMediaSrv == INVALID_HANDLE_VALUE) {
		RRecStart = FALSE;
		return 0;
	}
	return 1;
}

//---------------------------------------------------------------------------------------------------------------
int RRecStopF() {
	DWORD NOBW;

	if (EraState.SelectRecMode == RRAUDIOMODE) {
		IsRRecMp3Stop = TRUE;
	}
	if (EraState.SelectRecMode == RRVIDEOMODE) {
		IsRRecMp3Stop = IsRRecMp4Stop = TRUE;
		if (hWIn != NULL) {
			IsQSent = TRUE;
			WriteFile(hWIn, "q", 1, &NOBW, NULL);
		}
	}
	return 1;
}


//----------------------------------------------------------------------------------------------------------------
int RRecPauseF() {
	return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////
unsigned long _stdcall ThMediaCopy(void*) { //NETRES NetRes, vector <COPYFILE> CF
	NETRESOURCE NETRes;
	DWORD DWREZ;
	BOOL IsNetConnect = FALSE;
	int AttConn = 2;
	int ret = 0;
	char NetName[256]; //сетевое имя
	char ShortName[128];
	
	ZeroMemory(&NETRes, (SIZE_T)sizeof NETRESOURCE);
	NETRes.dwType = RESOURCETYPE_DISK;
	if (NetRes.MountPoint[0]) {
		NETRes.lpLocalName = NetRes.MountPoint;
	}
	else {
		goto EX_THMC;
	}
	NETRes.lpRemoteName = NetRes.Share;
WAC:
	DWREZ = WNetAddConnection2(&NETRes, NetRes.Password, NetRes.User, CONNECT_TEMPORARY);
	if (DWREZ != NO_ERROR) {
		if (AttConn) {
			WNetCancelConnection2(NETRes.lpLocalName, NULL, TRUE);
			Sleep(1000); AttConn--;
			goto WAC;
		}
		else {
			goto EX_THMC;
		}
	}
	else { //DWREZ == NO_ERROR
		IsNetConnect = TRUE;
	}
	
	//U
	/*
	while (1) {
		COPYFILE cft;
		strcpy(cft.FName, "C:\\Temp\\Temp.iso");
		auto iter1 = CF.cbegin(); // константный итератор указывает на первый элемент
		CF.insert(iter1, cft);
		DeleteFile("Y:\\Temp.iso");
		break;
	}*/
	//U

	if (CF.size() && !ThMSrvSendProgrID) {
		ZeroMemory(&UDSSP, sizeof(U64D));
		ZeroMemory(&LastNetCopy, sizeof(U64D));
		UDSSPA.store(UDSSP);
		//запуск потока отправки прогресса сетевого копирования клиенту
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThMSrvSendProgr, NULL, 0, &ThMSrvSendProgrID);
	}
	while (CF.size()) {
		if (GetShortName(CF[0].FName, ShortName)) {
			strcpy(UDSSP.Data, ShortName);
			UDSSPA.store(UDSSP);
			SetEvent(hEvMSSP);
			sprintf(NetName, "%s\\%s", NETRes.lpLocalName, ShortName);
			CopyFileExA(CF[0].FName, NetName, (LPPROGRESS_ROUTINE)CopyProgress, NULL, &CancelNetCopy, NULL);
			UDSSP.DW1 = 0;
			UDSSPA.store(UDSSP);
			auto iter = CF.cbegin(); // указатель на первый элемент
			CF.erase(iter);   // удаляем первый элемент
		}
		if (CancelNetCopy) {
			CF.clear();
		}
	}
	if (hEvMSSP != NULL) {
		ThMSrvSendProgrID = 0; //сигнал к остановке потока ThMSrvSendProgrID
		SetEvent(hEvMSSP); 
	}

	if (CancelNetCopy) {
		CancelNetCopy = FALSE;
		LastNetCopy.Data[0] = 0;
		LastNetCopy.DW1 = 0;
		LastNetCopy.DW2 = NC_CANCELED; //признак отменённого копирования
		if (IsShowTimerPanel) {
			TerminalSendCommand(SHOWPERCNETCOPY, &LastNetCopy, 0);
		}
	}
	else {//!CancelNetCopy
		LastNetCopy = UDSSPA.load();
		LastNetCopy.DW2 = NC_COMPLETED; //признак завершённого копирования
		if (IsShowTimerPanel) {
			TerminalSendCommand(SHOWPERCNETCOPY, &LastNetCopy, 0);
		}
	}

EX_THMC:
	if (IsNetConnect) {
		WNetCancelConnection2(NETRes.lpLocalName, NULL, TRUE);
	}
	ThMediaCopyID = 0;
	return ret;
}

/////////////////////////////////DWORD CopyProgress//////////////////////////////////////////
DWORD CopyProgress(
	LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,
	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,
	HANDLE hDestinationFile,
	LPVOID lpData
)
{
	DWORD Val, MaxVal;
	MaxVal = (DWORD)(TotalFileSize.QuadPart / 1024);
	Val = (DWORD)(TotalBytesTransferred.QuadPart / 1024);
	UDSSP.DW1 = (Val * 100) / MaxVal;
	UDSSPA.store(UDSSP);

	return PROGRESS_CONTINUE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long _stdcall ThMSrvSendProgr(void*) {
	U64D UDSSP;
	hEvMSSP = CreateEvent(NULL, FALSE, FALSE, NULL);

	while (1) {
		if (IsShowTimerPanel) {
			UDSSP = UDSSPA.load();
			TerminalSendCommand(SHOWPERCNETCOPY, &UDSSP, 0);
		}
		WaitForSingleObject(hEvMSSP, 1000);
			if (!ThMSrvSendProgrID) { //сигнал к остановке потока
				break;
			}
	}
	CloseHandle(hEvMSSP);
	hEvMSSP = NULL; ThMSrvSendProgrID = 0;
	UDSSP.DW1 = 0; UDSSPA.store(UDSSP);
	return 0;
}