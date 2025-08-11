#include <windows.h>
#include "service.h"
                                                        
SERVICE_STATUS_HANDLE sshSH;
SERVICE_TABLE_ENTRY dispatchTable;
SERVICE_STATUS S_S;
int rezi;
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow);


VOID ServiceStart(){
	dispatchTable.lpServiceName="mysrv";
	dispatchTable.lpServiceProc=(LPSERVICE_MAIN_FUNCTION)service_main;
	rezi=StartServiceCtrlDispatcher(&dispatchTable);
}


VOID WINAPI service_main(DWORD dwArgc,LPTSTR *lpszArgv)
{

sshSH=RegisterServiceCtrlHandler(dispatchTable.lpServiceName, service_ctrl);
S_S.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
S_S.dwServiceSpecificExitCode=0;
S_S.dwCurrentState=SERVICE_RUNNING;
S_S.dwWin32ExitCode=NO_ERROR;
S_S.dwCheckPoint=0;
S_S.dwControlsAccepted=SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
S_S.dwWaitHint=0;
SetServiceStatus(sshSH,&S_S);

WinMain((HINSTANCE)-1,(HINSTANCE)-1,"-1",-1);
}

VOID WINAPI service_ctrl(DWORD dwCtrlCode)
{
	switch(dwCtrlCode)
	{
	case SERVICE_CONTROL_STOP:
	/*
	SetEvent(hWFMO[0]);
	while(!PermitStop){
		Sleep(300);
	}
	LogMsg(WorkDir,"stop");
	*/
	S_S.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	S_S.dwServiceSpecificExitCode	=0;
	S_S.dwCurrentState=SERVICE_STOPPED;
	S_S.dwWin32ExitCode=NO_ERROR;
	S_S.dwCheckPoint=0;
	S_S.dwControlsAccepted=0;
	S_S.dwWaitHint=0;
	SetServiceStatus(sshSH,&S_S);
	break;

	case SERVICE_CONTROL_SHUTDOWN:
	/*
	SetEvent(hWFMO[0]);
	while(!PermitStop){
		Sleep(300);
	}
	LogMsg(WorkDir,"stop");
	*/
	S_S.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	S_S.dwServiceSpecificExitCode	=0;
	S_S.dwCurrentState=SERVICE_STOPPED;
	S_S.dwWin32ExitCode=NO_ERROR;
	S_S.dwCheckPoint=0;
	S_S.dwControlsAccepted=0;
	S_S.dwWaitHint=0;
	SetServiceStatus(sshSH,&S_S);
	break;


	case SERVICE_CONTROL_INTERROGATE:
		break;

	default:
		break;
	}

}