
#include "stdafx.h"
#include "structs.h"
#include "MediaFile.h"
#include <mmreg.h>

PPanelData ppd;
char tmp[256];
static int rez;

//VideoRecord for:
HANDLE hEvVideoReady;
HANDLE hEvUnionSaveReady;
HANDLE hROut, hWOut, hRIn, hWIn;
STARTUPINFO si;
PROCESS_INFORMATION pi;
char VideoCmd[328] = "ffmpeg.exe -y -f gdigrab -r 3 -i desktop -c:v h264  -preset slower -pix_fmt yuv420p ";
char Qsym[] = "q";
DWORD BTW;
DWORD NOBW;
//VideoRecord for.

void InitPPanelData(PPanelData PPD) {
	ppd = PPD;
	hEvVideoReady = CreateEvent(0, FALSE, FALSE, 0);
	hEvUnionSaveReady = CreateEvent(0, FALSE, FALSE, 0);
	//Инициализация строки запуска ffmpeg:
	if (strlen(ppd->pSGPreset)) {
		strcpy(VideoCmd, "ffmpeg.exe ");
		strcat(VideoCmd, ppd->pSGPreset);
		strcat(VideoCmd, " ");
	}
	//SetEvent(hEvVideoReady); //инициализация для первичного запуска видеозаписи
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));  //для очистки, т.к. pi.hProcess используется в StartVideoRecord для оценки окончания предыдущего сеанса записи
}

int PreparePcmFile(){

return 0;
}

int CompletePcmFile(){

return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateNameFromTime(char *PName)
{
	SYSTEMTIME ST;
	int i;
	i = 0;

	GetLocalTime(&ST);
	itoa(ST.wYear, PName, 10);
	strcat(PName, "_");
	if (ST.wMonth<10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wMonth, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "_");
	i = 0;
	if (ST.wDay<10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wDay, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "_");
	i = 0;
	if (ST.wHour<10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wHour, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "-");
	i = 0;
	if (ST.wMinute<10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wMinute, &tmp[i], 10);
	strcat(PName, tmp);
	strcat(PName, "-");
	i = 0;
	if (ST.wSecond<10) {
		i = 1;
		tmp[0] = 0x30;
	}
	itoa(ST.wSecond, &tmp[i], 10);
	strcat(PName, tmp);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CreateFullName(char *PName, int Type)
{
	char Name[128];
	if (!strlen(PName)) CreateNameFromTime(Name);
	else strcpy(Name, PName);
	strcpy(PName, ppd->FilePath);
	strcat(PName, Name);
	strcpy(ppd->ShortFileName, Name);
	switch (Type) {
	case MP3_NAME:
		strcat(PName, ".mp3");
		break;
	case MP4_NAME:
		strcat(PName, ".mp4");
		break;
	case PCM_NAME:
		strcat(PName, ".wav");
		break;
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int StartVideoRecord() {
	char VideoFullCmd[256];

	if (pi.hProcess != NULL) {//предыдущий сеанс записи не завершён
		return 0; 
	}
	ppd->FullVideoName[0] = 0;
	CreateFullName(ppd->FullVideoName, MP4_NAME);
	strcpy(VideoFullCmd, VideoCmd);
	strcat(VideoFullCmd, ppd->FullVideoName);

	CreatePipe(&hRIn, &hWIn, NULL, 65536);
	rez = SetHandleInformation(hRIn, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	rez = SetHandleInformation(hWIn, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = hRIn;

	rez = CreateProcess(NULL, VideoFullCmd, NULL, NULL, 1, 0, NULL, NULL, &si, &pi);
	if (pi.hProcess == NULL) {
		return 0;
	}
	ppd->hProcFF = pi.hProcess;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int StopVideoRecord() {
	DWORD ThStopVideoID;
	HANDLE hThStopVideo;

	if (pi.hProcess == NULL) {
		return 0;
	}
	hThStopVideo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThStopVideo, NULL, 0, &ThStopVideoID);
	CloseHandle(hThStopVideo);
	Sleep(200);
	return 1;
}
//--------------------------------------------------------------------------------------------------------//
unsigned long _stdcall ThStopVideo(void*) {

	BTW = sizeof(Qsym);
	WriteFile(hWIn, Qsym, BTW, &NOBW, NULL);

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(hRIn);
	CloseHandle(hWIn);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	pi.hProcess = NULL;
	ppd->hProcFF = NULL;
	pi.hThread = NULL;
	SetEvent(hEvVideoReady);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int StartAudio2Record() {
	HANDLE hPCM;

	ppd->FullPcmName[0] = 0;
	ppd->IsPcmFileReady = 0;
	ppd->NOBWPCM_ALL = 0;
	ppd->IsPcmFileReady = FALSE;
	ppd->hfpcm = hPCM = INVALID_HANDLE_VALUE;
	CreateFullName(ppd->FullPcmName, PCM_NAME);
	hPCM = CreateFile(ppd->FullPcmName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
	if ((hPCM == INVALID_HANDLE_VALUE) ||
		SetFilePointer(hPCM, (LONG)sizeof(PCMHEAD), NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		return 0;
	}
	ppd->hfpcm = hPCM;
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int StopAudio2Record() {
	HANDLE hPCM;
	PCMHEAD PcmHead;
	if (ppd->hfpcm == INVALID_HANDLE_VALUE || ppd->hfpcm == 0) {
		return 0;
	}
	hPCM = ppd->hfpcm;
	while (ppd->IsPcmFileLocked) {//ожидание завершения функции записи в файл из play.cpp, если таковая была в данный момент
		Sleep(5);
	}
	ppd->hfpcm = INVALID_HANDLE_VALUE; //установка запрета записи в файл

	//HEADER_PROC
	strcpy(PcmHead.RIFF, "RIFF");
	PcmHead.RiffSz = ppd->NOBWPCM_ALL + sizeof(PcmHead) - 8;
	strcpy(PcmHead.WAVEfmt, "WAVEfmt ");
	PcmHead.FmtSz = sizeof(PCMWAVEFORMAT);
	PcmHead.wFormatTag = WAVE_FORMAT_PCM;
	PcmHead.nChannels = 2;
	PcmHead.nSamplesPerSec = 8000;
	PcmHead.wBitsPerSample = 16;
	PcmHead.nBlockAlign = (PcmHead.wBitsPerSample / 8)*PcmHead.nChannels;
	PcmHead.nAvgBytesPerSec = PcmHead.nSamplesPerSec*PcmHead.nBlockAlign;
	strcpy(PcmHead.DATA, "data");
	PcmHead.DataSz = ppd->NOBWPCM_ALL;
	SetFilePointer(hPCM, (LONG)0, NULL, FILE_BEGIN);
	WriteFile(hPCM, (LPCVOID)&PcmHead, (DWORD)sizeof(PcmHead), &NOBW, NULL);
	CloseHandle(hPCM);
	hPCM = INVALID_HANDLE_VALUE;
	ppd->IsPcmFileReady = TRUE;

	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int UnionAVProc() {
	DWORD ThUnionAVPID;
	HANDLE hThUnionAVP;

	hThUnionAVP = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThUnionAVP, NULL, 0, &ThUnionAVPID);
	CloseHandle(hThUnionAVP);

	return 1;
}
//-----------------------------------------------------------------------------------------------------------------------
unsigned long _stdcall ThUnionAVP(void*) {
	char UnionPath[256];
	char FullVideoName[256];
	char FullPcmName[256];
	char UnionFullCmd[512]="ffmpeg.exe -i ";
	char opt1[] = " -i ";
	char opt2[] = " -c:v copy -c:a aac ";
	int len;

	len = strlen(ppd->FilePath);
	if (!len) { return 0; }
	strcpy(UnionPath, ppd->FilePath);
	if (UnionPath[len-1] != '\\') {
		strcat(UnionPath, "\\");
	}
	strcat(UnionPath, "Union");
	CreateDirectory(UnionPath, NULL);
	strcat(UnionPath, "\\");
	strcat(UnionPath, ppd->ShortFileName);
	strcat(UnionPath, ".mp4");
	FNSpaceHandler(UnionPath);
	
	strcpy(FullVideoName, ppd->FullVideoName);
	FNSpaceHandler(FullVideoName);
	strcat(UnionFullCmd, FullVideoName);
	
	strcat(UnionFullCmd, opt1);

	strcpy(FullPcmName, ppd->FullPcmName);
	FNSpaceHandler(FullPcmName);
	strcat(UnionFullCmd, FullPcmName);

	strcat(UnionFullCmd, opt2);
	strcat(UnionFullCmd, UnionPath);

	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	rez = CreateProcess(NULL, UnionFullCmd, NULL, NULL, 0, 0, NULL, NULL, &si, &pi);
	if (pi.hProcess != NULL) {
		ppd->hProcFF = pi.hProcess;
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		pi.hProcess = ppd->hProcFF = NULL;
	}
	if (pi.hThread != NULL) {
		CloseHandle(pi.hThread);
		pi.hThread = NULL;
	}
	SetEvent(hEvUnionSaveReady);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int FNSpaceHandler(char *FileName) {

	if (strstr(FileName, " ")) {
		char *tmp = (char*)malloc(1024);
		strcpy(tmp, "\"");
		strcat(tmp, FileName);
		strcat(tmp, "\"");
		strcpy(FileName, tmp);
		free(tmp);
		return 1;
	}
	return 0;
}