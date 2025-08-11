#include "stdafx.h"
#include "ProcCmdFunc.h"
#include "structs.h"
#include "Photo.h"

int i;
DWORD ThPhotoID;
HANDLE hThPhoto;
PPanelData PPD;
BOOL IsSCRWndClose;
static BOOL IsPhotoBusy = FALSE;
static STARTUPINFO si;
static PROCESS_INFORMATION pi;

BOOL CALLBACK CloseSCRWnd(HWND hWnd, LPARAM lParam);

int PhotoProc(PPanelData ppd) {
	if (!IsPhotoBusy) {
		PPD = ppd;
		//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThPhoto, NULL, 0, &ThPhotoID);
		::SetLayeredWindowAttributes(PPD->hWndSelf, 0, 0, LWA_ALPHA);
		ThPhoto(NULL);
		::SetLayeredWindowAttributes(PPD->hWndSelf, 0, 255, LWA_ALPHA);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//unsigned long _stdcall ThPhoto(void*) {
unsigned long _stdcall ThPhoto(void*) {
	IsPhotoBusy = TRUE;
	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;
	CreateProcess(NULL, PPD->SCRBin, NULL, NULL, 0, 0, NULL, NULL, &si, &pi);

	i = 0; IsSCRWndClose = FALSE;
	while (!IsSCRWndClose && i < 25) {
		Sleep(200);
		EnumWindows(CloseSCRWnd, pi.dwProcessId);
		i++;
	}
	
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	IsPhotoBusy = FALSE;
	return 0;
}
//**********************BOOL CALLBACK CloseSCRWnd(HWND hWnd, LPARAM lParam)*********
BOOL CALLBACK CloseSCRWnd(HWND hWnd, LPARAM lParam) {

	DWORD dwProcessID;
	char WndText[256];

	GetWindowThreadProcessId(hWnd, &dwProcessID);
	if (IsWindowVisible(hWnd) && dwProcessID == (DWORD)lParam) {
		/*GetWindowText(hWnd, WndText, sizeof(WndText));
		if (strstr(WndText, RAPartWndName)) {
			PanDat.HWRAMainWnd = hWnd;
		}
		*/
		Sleep(700);
		TerminateProcess(pi.hProcess, 0);
		//CloseWindow(hWnd);
		IsSCRWndClose = TRUE;
	}
	return TRUE;
}
