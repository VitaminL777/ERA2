#pragma once
class Proc
{
public:
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	int CreateProc(char *cmd, WORD Show = SW_SHOWNORMAL, DWORD Wait = 0, PROCESS_INFORMATION *ppi = NULL);
	int CreateProcActiveUser(char *cmd, WORD Show = SW_SHOWNORMAL, DWORD Wait = 0, PROCESS_INFORMATION *ppi = NULL);
	int CreateProcActiveUser(char *cmd, STARTUPINFO *psi, PROCESS_INFORMATION *ppi, BOOL InheritH = FALSE);
	void ProcCloseH();
	LPWSTR AnsiToUnicode(LPCSTR lpAnsiStr);
	LPWSTR GetProcDirectory(LPSTR Full);
	Proc();
	virtual ~Proc();


private:
	DWORD GetActiveSessionId();


};