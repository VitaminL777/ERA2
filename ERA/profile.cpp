#define _WIN32_WINNT 0x0500
#include "Winsock2.h"
#include "ws2tcpip.h"
#include "windows.h"
#include "structs.h"
#include "control.h"
#include "decode.h"
#include "LogMsg.h"

SOCKET sockCLNT=INVALID_SOCKET;
static struct sockaddr_in clnt;
char IPDest[64]="127.0.0.1";
static WORD CodeOper=0;
static int rez;
int numret=50;
static STARTUPINFO si;
static PROCESS_INFORMATION pi;
static int i=0;
static PSendStruct PSS;
static char tmp[128];
hostent *phst;
char* WndName;
char Fnd_Name[200]=" ";
char Ch_Name[200];
size_t LengthWndName; 
UINT WndMsg=0;
char FullParam[384]="/connect:";
char FullCmd[400];
char* lpn[2];
extern int TCPPort;
extern BOOL LocalTestMode;
extern BOOL StaticClientMode;
extern BOOL IsProfile;
extern BOOL IsDisconAnotherUser;
extern BOOL IsRefuseDisconn;
extern char ResponseData[256];
extern char Connect[128];
extern char StaticConnect[128];
extern char ActiveConnect[128];
extern int type;
extern char Param[256];
extern char RAbin[128];
extern char Wnd_Name[200];
extern char RAPartWndName[256];
extern char user[128];
extern char passw[128];
extern char InitCmd[400];
extern char InitCmdW[400];
extern char AnotherUser[96];
extern int initflag;
extern int NotUseEraSound;
extern int TcpNoDelay;
extern int RAPort;
extern int PostPressWinkey;
extern SOCKET sockSNDR;
extern SOCKET sockTRM;
extern HANDLE hThDecode;
extern DWORD ThDecodeID;
extern PanelData PanDat;
extern ERAState EraState;
SendStruct SSt;
BOOL FindWnd=FALSE;
BOOL IsProfileStart=FALSE;
HANDLE hEventEraSelect;
HWND hRAMW;
DWORD WFMO;
extern HANDLE hEvExit;

BOOL CALLBACK EWCB(HWND hWnd,LPARAM lp1);
BOOL CALLBACK EChWCB(HWND hChWnd,LPARAM lp2);
int StartProfile();
void NoConnectMessage(char* NoConnect);
void OutDisconMessage(char* AnotherUser);
void ButtonHandler();
void ExitProc(int CodeExit);
void GenerateKey(int vk, BOOL bExtended);
HWND GetRAMainWnd(DWORD PID);
BOOL CALLBACK PIDEnumWindows(HWND hWnd, LPARAM lParam);

unsigned long _stdcall ThProfile(void*){
////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(NotUseEraSound){
StartProfile();
ExitProc(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
if(LocalTestMode){
strcpy(SSt.IPDest,"127.0.0.1");
SSt.DataCont.CO=CREATESND;
sockCLNT=INVALID_SOCKET;
	if(DataComm(&SSt,&sockCLNT)){
	sockSNDR=sockCLNT;
	hThDecode=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThDecode,NULL,0,&ThDecodeID);
	ExitThread(0);
	}
ExitProc(0);
}
/////////////////////////////////////////StaticClientMode//////////////////////////////////////////////////
if(StaticClientMode){
int rep=3;
strcpy(PanDat.ConnectTo,StaticConnect);
PanDat.IsPassedFirstConnect=0; //для высвечивания панели установки соединения из ERAPANEL.dll
while(1)
{
	while(!CreateFullConnect(StaticConnect) && (--rep))
	{
		if(IsRefuseDisconn){
			ExitProc(0);}//Выходим при отказе от дисконнекта другого юзера
		Sleep(300);
	}
	if(!rep){
		PanDat.IsPassedFirstConnect=-1;
		NoConnectMessage(StaticConnect);
		ExitProc(0);
	}
	PanDat.IsPassedFirstConnect=1;//окончание высвечивания панели установки соединения из ERAPANEL.dll
	hThDecode=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThDecode,NULL,0,&ThDecodeID);
	WaitForSingleObject(hThDecode,INFINITE);
	if(IsDisconAnotherUser){//Если отключен пользователем извне - выводим ссобщение и выходим
		OutDisconMessage(AnotherUser);
		ExitProc(1);
	}
	IntCloseAll(); //Иначе, значит какой-то внештатный разрыв - повторяем попытку
	Sleep(500); //и теперь уже ставим 
	rep=30000;  //огромное кол-во попыток
}//end_while(1)
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//****************************************ProfileMode******************************************************************
if(IsProfile){
	int rep=3;
strcpy(PanDat.ConnectTo,Connect);
PanDat.IsPassedFirstConnect=0; //для высвечивания панели установки соединения из ERAPANEL.dll
while(1)
{
	while(!CreateFullConnect(Connect) && (--rep))
	{
		if(IsRefuseDisconn){
			ExitProc(0);}//Выходим при отказе от дисконнекта другого юзера


		if (PanDat.hEvent[0] != INVALID_HANDLE_VALUE && PanDat.hEvent[0] != 0 && \
			PanDat.hEvent[1] != INVALID_HANDLE_VALUE && PanDat.hEvent[1] != 0)
		{//если соединение было установлено, а потом - непредвиденно разорвано, то внутри процедуры 
			//его восстановления, нужно также обрабатывать события возможного закрытия программы
			WFMO = WaitForMultipleObjects(2, PanDat.hEvent, FALSE, 300);
			switch (WFMO) {
			
			case WAIT_TIMEOUT: //300 ms
				continue;
			
			case 0: //по нажатию кнопок в панели
				ButtonHandler();
				continue;
				break;
			
			case 1: //по прекращению профильного процесса
				TerminalSendCommand(CLOSEALL, "");
				//Sleep(200);
				WaitForSingleObject(hEvExit, 5000);
				SetEvent(hEvExit);
				ExitProc(1);
				break;
			}
		}
		else {//первичное установление соединения
			Sleep(300);
		}
	}
	if(!rep){
		PanDat.IsPassedFirstConnect=-1;
 		NoConnectMessage(Connect);
		ExitProc(0);
	}
	PanDat.IsPassedFirstConnect=1;//окончание высвечивания панели установки соединения из ERAPANEL.dll
	PanDat.hEvent[2]=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThDecode,NULL,0,&ThDecodeID);
	
	hEventEraSelect=OpenEvent(EVENT_ALL_ACCESS,FALSE,"EraSelectEvent"); //если событие существует - значит запуск с ERASELECT
	if(hEventEraSelect){		//и следовательно - сигналим ей, что запустились мы
		SetEvent(hEventEraSelect);
		CloseHandle(hEventEraSelect);
	}

	if(!StartProfile()){ExitProc(-1);}
WLOOP:
	//основная точка ожидания профильного потока в рабочем состоянии=============================================================
	WFMO=WaitForMultipleObjects(3,PanDat.hEvent,FALSE,INFINITE);
	switch (WFMO) {

	case WAIT_FAILED:
		break;

	case 0: //по нажатию кнопок в панели
		ButtonHandler();
		goto WLOOP;
		break;

	case 1: //по прекращению профильного процесса
		//TerminalSendCommand(CLOSEALL, "");
		LogMsg("Закрытие по прекращению профильного процесса!!! RecordState=",(int)EraState.RecordState);
		if (EraState.RecordState&RS_RRECORD) {
			TerminalSendCommandAndState(CLOSEALLSAVEERST, &EraState, 0);
		}
		else {
			TerminalSendCommandAndState(CLOSEALL, &EraState, 0);
		}
		WaitForSingleObject(hEvExit, 5000);
		SetEvent(hEvExit);
		ExitProc(1);
		break;

	case 2: //прекращение потока ThDecode (по закрытию сокета)
	//ничего не делаем - сваливаем обратно в цикл
		int DecEnd = 1;
		break;

	}//switch(WaitForMultipleObjects(3,PanDat.hEvent,FALSE,INFINITE)){

	if(IsDisconAnotherUser){//Если отключен пользователем извне - выводим ссобщение и выходим
		OutDisconMessage(AnotherUser);
		ExitProc(1);
	}
	IntCloseAll(); //Иначе, значит какой-то внештатный разрыв - повторяем попытку
	Sleep(500); //и теперь уже ставим 
	rep=30000;  //огромное кол-во попыток
}//end_while(1)

}//fi(IsProfile)
//****************************************ProfileMode******************************************************************

return 0;
}



//****************************int StartProfile()******************************************
int StartProfile(){
	if(IsProfileStart){return 1;}
rez=0;
if(type){//т.е. Radmin-подключение
strcat(FullParam,Connect);
	if(RAPort){
		strcat(FullParam,":");
		strcat(FullParam,itoa(RAPort,tmp,10));
	}
strcat(FullParam," ");
}
else{//RDP-подключение
FullParam[0]=0;
}

strcat(FullParam,Param);
lpn[0]=user;
lpn[1]=passw;
memset(&si,0,sizeof(STARTUPINFO));
memset(&pi,0,sizeof(PROCESS_INFORMATION));
si.cb=sizeof(STARTUPINFO);
si.dwFlags=STARTF_FORCEOFFFEEDBACK|STARTF_USESHOWWINDOW;
si.wShowWindow=SW_HIDE;
////////////////////////если определена инициализация///////////////////
if(initflag&1){
CreateProcess(NULL,InitCmd,NULL,NULL,0,0,NULL,NULL,&si,&pi);
CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);
}
if(initflag&2){
memset(&pi,0,sizeof(PROCESS_INFORMATION));
CreateProcess(NULL,InitCmdW,NULL,NULL,0,0,NULL,NULL,&si,&pi);
WaitForSingleObject(pi.hProcess,INFINITE);
CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);
}
///////////////////////////////////////////////////////////////////////

memset(&si,0,sizeof(STARTUPINFO));
memset(&pi,0,sizeof(PROCESS_INFORMATION));
si.cb=sizeof(STARTUPINFO);
si.dwFlags=STARTF_FORCEOFFFEEDBACK|STARTF_USESHOWWINDOW;
si.wShowWindow=SW_SHOWNORMAL;
strcpy(FullCmd,RAbin);
strcat(FullCmd," ");
strcat(FullCmd,FullParam);

if(!CreateProcess(NULL,FullCmd,NULL,NULL,0,0,NULL,NULL,&si,&pi)){
	return 0;
}

if(!type){//если RDP-подключение, никаких окон не ищем - передаём параметры и выходим
	//WaitForSingleObject(pi.hProcess,INFINITE);
	CloseHandle(pi.hThread);
	hRAMW = GetRAMainWnd(pi.dwProcessId);
	PanDat.hEvent[1]=pi.hProcess;
	IsProfileStart=1;
	return 1;
}

LengthWndName=strlen(Wnd_Name);
while(numret){
	EnumWindows(EWCB,NULL);
	if(FindWnd){break;}
	Sleep(200);
	numret--;
}
	
if(!FindWnd){
TerminateProcess(pi.hProcess,0);
CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);
return 0;
}

//WaitForSingleObject(pi.hProcess,INFINITE);
CloseHandle(pi.hThread);
PanDat.hEvent[1]=pi.hProcess;
IsProfileStart=1;
return 1;
}



///////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK EWCB(HWND hWnd,LPARAM lp1){
	GetWindowText(hWnd,Fnd_Name,100);
	if(0==strncmp(Fnd_Name,Wnd_Name,LengthWndName)){
	//ищем окошко с заданным заголовком и переключаемся на него
	SwitchToThisWindow(hWnd,TRUE);
	Sleep(333);
	EnumChildWindows(hWnd,EChWCB,NULL);
	FindWnd=TRUE;
	SendMessage(hWnd,WndMsg,0,0);
	return 0;
	}
return 1;
}

///////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK EChWCB(HWND hChWnd,LPARAM lp2){

GetWindowText(hChWnd,Ch_Name,100);

if(!strcmp(Ch_Name,"") && i<2){//вводим имя и пароль соответственно в первое и второе поле
	SendMessage(hChWnd,WM_SETTEXT,0,(LPARAM)lpn[i]);
i++;}

if((!strcmp(Ch_Name,"ОК")||!strcmp(Ch_Name,"OK")) && i>1){
SendMessage(hChWnd,BM_CLICK, 0, 0);//нажимаем клавишу ОК в окошке
}


return 1;
}
//******************************************************************************************************************







//********************************void NoConnectMessage(char* NoConnect)*********************************************
void NoConnectMessage(char* NoConnect)
{
char MessNoConn[96]="Невозможно установить соединение с '";

strcat(MessNoConn,NoConnect);
strcat(MessNoConn,"'");
MessageBox(0,MessNoConn,"Нет соединения",MB_ICONWARNING|MB_SYSTEMMODAL);

return;
}

//********************************void OutDisconMessage(char* AnotherUser)*********************************************
void OutDisconMessage(char* AnotherUser)
{
char MessNoConn[96]="Сеанс отключен клиентом '";

strcat(MessNoConn,AnotherUser);
strcat(MessNoConn,"'");
MessageBox(0,MessNoConn,"Отключено",MB_ICONWARNING|MB_SYSTEMMODAL);

return;
}



//********************************void ButtonHandler()******************************************************************

void ButtonHandler(){

	switch (PanDat.ButEvent) {
	
	case BE_EXIT:
		if (PanDat.hEvent[1] != 0 && PanDat.hEvent[1] != INVALID_HANDLE_VALUE) {
			TerminateProcess(PanDat.hEvent[1], 1);
		}
		//TerminalSendCommand(CLOSEALL, "");
		TerminalSendCommandAndState(CLOSEALLSAVEERST, &EraState, 0);
		Sleep(200);
		ExitProc(1);
		PanDat.ButEvent = BE_NONE;
		return;

	case BE_OFF:
		TerminalSendCommand(LOGOFF, "O");
		PanDat.ButEvent = BE_NONE;
		return;
	
	case BE_CHANGEMONOMODE:
		TerminalSendCommand(CHANGEMODE, "", 1);
		EraState.IsMonoMode = PanDat.RealMonoMode = (bool)ResponseData[0]; //ставим значение реального моно/стерео режима в соответствии с кодом ответа
		PanDat.ButEvent=BE_NONE;
		return;
	
	case BE_CHANGERSELMODE:
		TerminalSendCommandAndState(CHANGESELRECMODE, PanDat.pEraSt, 0);
		PanDat.ButEvent = BE_NONE;
		return;

	case BE_PRECHRPARAMS:
		LTGetTime();
		PanDat.ButEvent = BE_NONE;
		return;

	case BE_CHANGERPARAMS:
		LTChangeStateByRecordState();
		if (PanDat.pEraSt->MapChParams&CH_LENTIMERVALUE) {
			LTSetTime();
		}
		TerminalSendCommandAndState(CHANGERECPARAMS, PanDat.pEraSt, 0);
		PanDat.ButEvent = BE_NONE;
		return;
	
	case BE_CHANGERECSTATE:
		LogMsg("PanDat.IsRecord(on BE_CHANGERECSTATE)=", (int)PanDat.IsRecord);
		if (PanDat.pEraSt->SelectRecMode >= RRVIDEOMODE) {
			//ReqChangeRRecState = REQ_RRECSTOP
			if (PanDat.IsRecord) {
				if (PanDat.ReqChangeRRecState == REQ_RRECSTOP) {
					PanDat.pEraSt->RecordState = RS_STOP;
					LogMsg("TerminalSendCommandAndState(RRECSTOP)_", __LINE__);
					TerminalSendCommandAndState(RRECSTOP, PanDat.pEraSt, 0);
					goto LTCS;
				}
				if (PanDat.ReqChangeRRecState == REQ_RRECPAUSE) {
					if (!PanDat.IsPause) {
						PanDat.pEraSt->RecordState |= RS_RPAUSE;
						TerminalSendCommandAndState(RRECPAUSE, PanDat.pEraSt, 0);
					}
					else {//PanDat.IsPause
						PanDat.pEraSt->RecordState &= ~RS_RPAUSE;
						TerminalSendCommandAndState(RRECSTART, PanDat.pEraSt, 0);
					}
				}
			}
			else { //!PanDat.IsRecord
				if (PanDat.ReqChangeRRecState == REQ_RRECSTART) {
					PanDat.pEraSt->RecordState = RS_RRECORD;
					LogMsg("TerminalSendCommandAndState(RRECSTART)_", __LINE__);
					TerminalSendCommandAndState(RRECSTART, PanDat.pEraSt, 0);
				}
			}
		}
		else { //PanDat.pEraSt->SelectRecMode < RRVIDEOMODE
			if (PanDat.IsRecord) {
				if (!PanDat.IsPause) {
					PanDat.pEraSt->RecordState = RS_LRECORD;
					TerminalSendCommandAndState(LRECSTART, PanDat.pEraSt, 0);
				}
				else {//PanDat.IsPause
					PanDat.pEraSt->RecordState |= RS_LPAUSE;
					TerminalSendCommandAndState(LRECPAUSE, PanDat.pEraSt, 0);
				}
			}
			else { //!PanDat.IsRecord
				PanDat.pEraSt->RecordState = 0;
				TerminalSendCommandAndState(LRECSTOP, PanDat.pEraSt, 0);
			}
		}
LTCS:
		LTChangeStateByRecordState();
		PanDat.ButEvent = BE_NONE;
		PanDat.ReqChangeRRecState = REQ_NONE;
		return;
	
	case BE_SHOWTIMERPANEL:
		TerminalSendCommand(SHOWTIMERPANEL, "", 0);
		return;

	case BE_CLOSETIMERPANEL:
		TerminalSendCommand(CLOSETIMERPANEL, "", 0);
		return;

	case BE_ABORTNETCOPY:
		TerminalSendCommand(ABORTNETCOPY, "", 0);
		return;

	default:
		return;
	}
}


//********************************void ExitProc()******************************************************************
void ExitProc(int CodeExit){
	if (PanDat.hProcFF != NULL) {//грубо закрываем хвосты ffmpeg, если есть таковые
		TerminateProcess(PanDat.hProcFF, 0);
	}
if(PostPressWinkey)
GenerateKey (VK_LWIN, TRUE);
ExitProcess(CodeExit);
}
//********************************void GenerateKey ( int vk , BOOL bExtended)**************************************
void GenerateKey ( int vk , BOOL bExtended)
{
KEYBDINPUT kb={0};
INPUT Input={0};
  // нажатие 
  if ( bExtended )
  kb.dwFlags=KEYEVENTF_EXTENDEDKEY;
  kb.wVk=vk;  
  Input.type=INPUT_KEYBOARD;
  Input.ki  = kb;
  SendInput(1,&Input,sizeof(Input));
 // отпускание
  Sleep(20);
  ZeroMemory(&kb,sizeof(KEYBDINPUT));
  ZeroMemory(&Input,sizeof(INPUT));
  kb.dwFlags  =  KEYEVENTF_KEYUP;
  if ( bExtended )
  kb.dwFlags  |= KEYEVENTF_EXTENDEDKEY;
  kb.wVk    =  vk;
  Input.type  =  INPUT_KEYBOARD;
  Input.ki  =  kb;
  SendInput(1,&Input,sizeof(Input));
}

//////////////////////////////HWND GetRAMainWnd(DWORD PID)///////////////////////////////
HWND GetRAMainWnd(DWORD PID){
	int i;
	PanDat.HWRAMainWnd = 0;
	i = 0;
	while ((PanDat.HWRAMainWnd == 0) && i < 25) {
		Sleep(200);
		EnumWindows(PIDEnumWindows, PID);
		i++;
	}
	return PanDat.HWRAMainWnd;
}
//**********************BOOL CALLBACK PIDEnumWindows(HWND hWnd, LPARAM lParam)*********
BOOL CALLBACK PIDEnumWindows(HWND hWnd, LPARAM lParam) {

DWORD dwProcessID;
char WndText[256];

GetWindowThreadProcessId(hWnd, &dwProcessID);
if (IsWindowVisible(hWnd) && dwProcessID == (DWORD)lParam) {
	GetWindowText(hWnd, WndText, sizeof(WndText));
	if (strstr(WndText, RAPartWndName)) {
		PanDat.HWRAMainWnd = hWnd;
	}
}
return TRUE;
}