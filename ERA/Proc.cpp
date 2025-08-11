#include "nowarnings.h"
#include <windows.h>
#include <wtsapi32.h>
#include <stdio.h>
#include <userenv.h>
#include "Proc.h"
#include "LogMsg.h"

Proc::Proc()
{

}

Proc::~Proc()
{

}


LPWSTR Proc::AnsiToUnicode(LPCSTR lpAnsiStr)
{
	LPWSTR lpUnicodeStr = NULL;
	int iLen;
	// вычисляем длину будущей Unicode строки
	if ((iLen = MultiByteToWideChar(CP_ACP, 0, lpAnsiStr, -1, NULL, 0)) != 0)
		// выделяем память под будущую Unicode строку
		if ((lpUnicodeStr = (LPWSTR)malloc(iLen * sizeof(WCHAR))) != NULL)
			// кодируем существующую ANSI строку в Unicode
			if (MultiByteToWideChar(CP_ACP, 0, lpAnsiStr, -1, lpUnicodeStr, iLen) == 0)
			{
				// ОШИБКА
				free(lpUnicodeStr);
				lpUnicodeStr = NULL;
			}

	return lpUnicodeStr;
}

LPWSTR Proc::GetProcDirectory(LPSTR Full) {

	int i, len;

	char temp[384];

	strcpy(temp, Full);
	len = strlen(temp);
	i = len - 1;
	while (i > 0) {
		if (temp[i] == 0x5C) {
			break;
		}
		i--;
	}
	temp[i] = 0;
	if (strlen(temp) == 0) {
		return 0;
	}

	return AnsiToUnicode(temp);
}


int Proc::CreateProc(char *cmd, WORD Show, DWORD Wait, PROCESS_INFORMATION *ppi) {
	BOOL brz;

	if (!strlen(cmd)) { return 0; }
	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = Show;

	brz = ::CreateProcessA(NULL, cmd, NULL, NULL, 0, 0, NULL, NULL, &si, &pi);
	if (pi.hProcess == NULL) {
		return 0;
	}
	if (Wait == 0) { //не ожидаем завершения процесса
		if (ppi == NULL) {
			CloseHandle(pi.hThread); pi.hThread = NULL;
			CloseHandle(pi.hProcess); pi.hProcess = NULL;
		}
		else {
			ppi = &pi;
		}
		return 1;
	}
	WaitForSingleObject(pi.hProcess, Wait);
	CloseHandle(pi.hThread); pi.hThread = NULL;
	CloseHandle(pi.hProcess); pi.hProcess = NULL;

	return 1;
}

//////////////////////////////////////////////////////////////////////
int Proc::CreateProcActiveUser(char *cmd, WORD Show, DWORD Wait, PROCESS_INFORMATION *ppi) {
	DWORD SecID = 0;
	DWORD dwrz;
	HANDLE hTk;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPVOID lpEnvironment;
	BOOL ok;

	//SecID = WTSGetActiveConsoleSessionId();
	SecID = GetActiveSessionId();
	if (!SecID) {
		return 0;
	}
	dwrz = WTSQueryUserToken(SecID, &hTk);
	if (!dwrz) {
		return 0;
	}
	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = Show;
	si.lpDesktop = "winsta0\\default";
	ok = CreateEnvironmentBlock(&lpEnvironment, hTk, false);

	ok = CreateProcessAsUser(
		hTk,
		NULL,
		cmd,
		NULL,
		NULL,
		FALSE,
		CREATE_UNICODE_ENVIRONMENT | NORMAL_PRIORITY_CLASS,
		lpEnvironment,
		NULL,
		&si,
		&pi);

	if (pi.hProcess == NULL) {
		return 0;
	}
	if (Wait == 0) { //не ожидаем завершения процесса
		if (ppi == NULL) {
			CloseHandle(pi.hThread); pi.hThread = NULL;
			CloseHandle(pi.hProcess); pi.hProcess = NULL;
		}
		else {
			ppi = &pi;
		}
		return 1;
	}
	WaitForSingleObject(pi.hProcess, Wait);
	CloseHandle(pi.hThread); pi.hThread = NULL;
	CloseHandle(pi.hProcess); pi.hProcess = NULL;

	return 1;
}

//-------------------------------------------------------------------------------------
int Proc::CreateProcActiveUser(char *cmd, STARTUPINFO *psi, PROCESS_INFORMATION *ppi, BOOL InheritH) {
	DWORD SecID = 0;
	DWORD dwrz;
	HANDLE hTk;
	LPVOID lpEnvironment;
	BOOL ok;

	if (psi == NULL || ppi == NULL || !(SecID = GetActiveSessionId())) {
		return 0;
	}
	dwrz = WTSQueryUserToken(SecID, &hTk);
	if (!dwrz) {
		return 0;
	}
	psi->cb = sizeof(STARTUPINFO);
	psi->lpDesktop = "winsta0\\default";
	ok = CreateEnvironmentBlock(&lpEnvironment, hTk, false);

	ok = CreateProcessAsUser(
		hTk,
		NULL,
		cmd,
		NULL,
		NULL,
		InheritH,
		CREATE_UNICODE_ENVIRONMENT | NORMAL_PRIORITY_CLASS,
		lpEnvironment,
		NULL,
		psi,
		ppi);

	if (!ok || ppi->hProcess == NULL) {
		return 0;
	}
	CloseHandle(ppi->hThread); ppi->hThread = NULL;
	return 1;
}


DWORD Proc::GetActiveSessionId()
{
	WTS_SESSION_INFO *pSessionInfo;
	DWORD n_sessions = 0;
	DWORD SessionId = 0;
	BOOL ok = FALSE;
	
	ok = WTSEnumerateSessions(WTS_CURRENT_SERVER, 0, 1, &pSessionInfo, &n_sessions);
	if (!ok) {
		return 0;
	}

	for (DWORD i = 0; i < n_sessions; ++i)
	{
		if (pSessionInfo[i].State == WTSActive)
		{
			SessionId = pSessionInfo[i].SessionId;
			break;
		}
	}
	WTSFreeMemory(pSessionInfo);
	return SessionId;
}

//////////////////////////////////////////////////////////////////////
void Proc::ProcCloseH() {
	if (pi.hThread != NULL) {
		CloseHandle(pi.hThread); pi.hThread = NULL;
	}
	if (pi.hProcess != NULL) {
		CloseHandle(pi.hProcess); pi.hProcess = NULL;
	}
}
