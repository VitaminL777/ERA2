int GetCorrectCmdLine(LPSTR Cmd, LPSTR CmdLine);
int GetShortModuleName(LPSTR Full, LPSTR Short);
int GetFullModuleNameAndParams(LPSTR Full, LPSTR FullModuleName, LPSTR Params, LPSTR SelfParams);
int ClearIFEO(LPSTR Cmd);
int RestoreIFEO();
int GetFlagAndCorrect(LPSTR Cmd, int* POperFlag);
int WaitProcess(char* ProcStr, DWORD TimeOut);
int IsProcessExist(char* ProcStr);
int KillProcess(char* ProcStr);
int CloseProcess(char* ProcStr, DWORD TimeWait);//TimeWait in seconds
int StartAndWaitProcess(HANDLE hToken,char* Desk,char* ProcStr, DWORD TimeOut, char* PRN, int FlagIFEO);
int SetProcDirectory(LPSTR Full);
DWORD GetProcessID(char* ProcStr);
BOOL CALLBACK CloseAppEnumWindows(HWND hWnd, LPARAM lParam);