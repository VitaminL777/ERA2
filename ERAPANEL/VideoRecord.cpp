
#include "stdafx.h"
#include "ProcCmdFunc.h"
#include "structs.h"

#define NOWINABLE

DWORD GPID, DPID;
HWND hfw, hfwe;
BOOL StartNow=0;
BOOL IsRecording=0, IsPaused=0, IsStoped=0;
BOOL StopFlag=0;
int NumRep=20;
int rez;

int VideoRecord(PPanelData ppd)
{
 
NumRep=20;
StopFlag=StartNow=IsRecording=IsPaused=IsStoped=0;

if(ppd->VideoMode==VIDEOSTOP) StopFlag=1;


GPID=GetProcessID("ScreenVCR.exe");

if(!GPID && !StopFlag){
ShellExecute(NULL,"open","ScreenVCR.exe",NULL,NULL,SW_HIDE);
Sleep(500);
GPID=GetProcessID("ScreenVCR.exe");
	if(!GPID){return 0;}
StartNow=1;
IsStoped=1;
}


hfw=FindWindow("TfmMain","Total Screen Recorder Gold");

/*
INPUT inp;


MENUBARINFO mbi;
rez=GetMenuBarInfo(hfw,OBJID_CLIENT,0,&mbi);


HMENU hmenu=GetMenu((HWND)0x904a4);

*/


if(hfw){
	IsStoped=1;}

if(!hfw && StartNow){
	while(NumRep){
		Sleep(250);
	hfw=FindWindow("TfmMain","Total Screen Recorder Gold");
	if(hfw){goto GWTPID;}
	NumRep--;
	}
return 0;
}

if(!hfw && !StartNow){
hfw=FindWindow("TfmMain","Recording");
	if(hfw){
	IsRecording=1; IsStoped=0;
	goto GWTPID;
	}

hfw=FindWindow("TfmMain","Paused");

	if(hfw){
	IsPaused=1; IsStoped=0;
	goto GWTPID;
	}

return 0;
}


GWTPID:
GetWindowThreadProcessId(hfw,&DPID);
if(GPID!=DPID){
	return 0;
}
///////////основное окно Total Screen Recorder идентифицировано, ищем кнопочный ToolBar

hfwe=FindWindowEx(hfw,NULL,"TToolBar","");

if(!hfwe) {return 0;}
	if(StopFlag){goto StopTSR;}

//—тарт записи(либо пауза)
if(IsPaused || IsStoped){
	Beep(2000,700);
}

if(IsRecording){
	Beep(2000,300);
	Sleep(150);
	Beep(2000,300);
}

SendMessage(hfwe,WM_LBUTTONDOWN,MK_LBUTTON,MAKELONG(10, 10));
SendMessage(hfwe,WM_LBUTTONUP,MK_LBUTTON,MAKELONG(10, 10));
return 0;

StopTSR:

	Beep(2000,250);
	Sleep(100);
	Beep(2000,250);
	Sleep(100);
	Beep(2000,250);

SendMessage(hfwe,WM_LBUTTONDOWN,MK_LBUTTON,MAKELONG(50, 10));
SendMessage(hfwe,WM_LBUTTONUP,MK_LBUTTON,MAKELONG(50, 10));
Sleep(2000);
KillProcess("ScreenVCR.exe");
return 0;


}



