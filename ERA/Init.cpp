//***************************************************************************************************
//***************************E__R__A(EXPANSION of REMOTE ACCESS) ************************************
//***************************************************************************************************

#include <windows.h>
#include <userenv.h>
#include <wtsapi32.h>
#include <stdio.h>
#include "ini.h"
#include "structs.h"
#include "control.h"
#include "capture.h"
#include "encode.h"
#include "decode.h"
#include "play.h"
#include "profile.h"
#include "service.h"
#include "regy.h"
#include "AuxWnd.h"
#include "ECP.h"
#include "tinyxml2.h"
#include "nitaxml.h"
#include "NitaTimer.h"
#include "Proc.h"
#include "LogMsg.h"
#include "funcs.h"


static int rez;

BOOL IsCrypt = TRUE;
int NumDevs;
WAVEINCAPS WIDC;
WAVEOUTCAPS WODC;
ini Cini;
char PathIni[128];
char famatech[128];
char WIDName[64] = { 0,0 };
char WODName[64] = { 0,0 };
char StaticConnect[96];
char LocalHostName[96];
char tmp[256];
int RunAsService=0;
int ID = 0;
int USE_ECP = 1;
char Gate[128];
int EnableLog = 0;
int DebugMode = 0;
char Aerodrome[64];
BOOL IsSender=FALSE;
BOOL IsServer=FALSE;
BOOL IsPlayer=FALSE;
BOOL IsClient=FALSE;
BOOL IsProfile=FALSE;
BOOL LocalTestMode=FALSE;
BOOL StaticClientMode=FALSE;
BOOL IsAuxWnd=FALSE;
int TCPPort=3390;
int RAPort=0;
int TcpNoDelay=1;
BOOL RComm=FALSE;
UINT WID, WOD;
int nBlockSize=1;
UINT BlockSizeIn=0;
BOOL IsLocalMonoMode = TRUE;
int WaitBufferSize=0;//2304;
UINT BytesInBuffer=0;
int mp3bitrate=24;
int MixToMono=0;
BOOL IsRemoteRecord = FALSE;
DWORD RemoteRecTimer = 0;
char LogoffEventName[128];

char NitaTimerXML[] = "Config\\_System\\Granite\\play_script.xml";
char NitaTimerFullXML[256];
char NitaRoot[128] = "C:\\Nita";
int NitaTimerPort = 0;
char NitaTimerMcAddr[64] = "";
char NitaTimerSenderIF[64] = "";

int StaticGain = 1;
int EnableEraPanel=0;
int PostPressWinkey=0;
PanelData PanDat;
DWORD ThControlID;
DWORD ThCaptureID;
DWORD ThEncodeID;
DWORD ThDecodeID;
DWORD ThPlayID;
DWORD ThProfileID;
DWORD ThNitaTimerID;
DWORD ThHelperID;
HANDLE hThControl;
HANDLE hThCapture;
HANDLE hThEncode;
HANDLE hThDecode;
HANDLE hThPlay;
HANDLE hThProfile;
HANDLE hThNitaTimer;
HANDLE hThHelper;
WORD ToPlaceKey=0;
ERAState EraState;
ERAState SrvState;
HANDLE hEvLoadPanelDll = NULL;
NETRES NetRes;

int ReverseStereo=0;
int SendSilentBlocks=0;
int TresholdOfSilent = 2;
int GetParam();
int GetParamXML();
int ParseCmdLine(char *pcmd, char *Profile, char *ERA_ini, int* pRunAsServ);
int CheckFamatech();
void InitContainerKey();
int GetIDByConnect(char *Connect);
void FillDefaultEraState(PERAState pest);
void EFSet(/*PanelData PanDat*/); //Export Functions Set
BOOL IsWinSockInit = FALSE;
extern int ECPRetCode;
extern WORD SrvContainerKey;
char PwdCrypt[128];

/////////ProfileParameters///////////////
char Profile[128];
char ERAini[128] = "ERA.ini";
int type=0;
char Connect[128];
char Param[256];
char RAbin[128]="C:\\Program Files\\Radmin Viewer 3\\Radmin.exe";
char Wnd_Name[200]="—истема безопасности Radmin:";
char RAPartWndName[256]="ѕодключение к удаленному рабочему столу";
char user[128]="grn";
char passw[128]="grn";
char InitCmd[400];
char InitCmdW[400];
int initflag=0;
int NotUseEraSound=0;
char SGBin[] = "ffmpeg.exe";
char SGFull[2048];
char SGPreset[1700]="-y -f gdigrab -r 2 -i desktop -c:v h264  -preset veryslow -pix_fmt yuv420p";
extern char FFBin[256];
extern char FFPreset[1500];
extern char FileSavePath[256];
//extern char NetSavePath[256];
char EventNameEra[64] = "ControlEra2Event_";
char EraPanelLibName[] = "ERAPANEL2";
BOOL EraLogin = FALSE;
/////////ProfileParameters///////////////

//U
unsigned long _stdcall ThHelper(void*);
unsigned long _stdcall ThHelp2(void*);
//U

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
//////////инициализируем подсиситему Windows Sockets дл€ всех потоков, где это потребуетс€//////////
if (!IsWinSockInit) {
	WORD Sver = MAKEWORD(2, 2);
	WSADATA wsd;
	if (0 != WSAStartup(Sver, &wsd)) { return 0; }
	IsWinSockInit = TRUE;
}

if(RunAsService){goto START;}

BlockSizeIn=nBlockSize*2304; 
ParseCmdLine(lpCmdLine, Profile, ERAini, &RunAsService);
ZeroMemory(&PanDat,sizeof(PanDat));
 CheckFamatech();
 if(!GetParam()){return 0;}
 if (USE_ECP) {
	 ECPRetCode = DataExchange(Gate, ID, 1);//SrvContainerKey
 }
 else {
	 ECPRetCode = 3;
	 SrvContainerKey = 0xF930;
 }

 if(RunAsService){
	 ServiceStart();//снова вызовет WinMain, но уже с параметрами =-1(на вс€кий случай)
	 return 0;
 }
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////S_____T_____A_____R_____T/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
START:
///////////////////////ѕровер€ем, не запущена ли ещЄ одна копи€ приложени€//////////////////////////
if(OpenEvent(EVENT_MODIFY_STATE,FALSE, EventNameEra)){
	MessageBox(0,"Ќевозможен запуск более одной копии данного приложени€!",
		"ѕредупреждение",MB_ICONWARNING|MB_SYSTEMMODAL);
	return 0;
}

//--------------------------------------------------------------------------------------------------
FillDefaultEraState(&EraState);
hThControl=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThControl,NULL,0,&ThControlID);

if(IsSender && !NotUseEraSound){
hThCapture=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThCapture,NULL,0,&ThCaptureID);	
}
if(IsPlayer && !NotUseEraSound)
hThPlay=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThPlay,NULL,0,&ThPlayID);

if(IsProfile||LocalTestMode||StaticClientMode)
hThProfile=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThProfile,NULL,0,&ThProfileID);

if (PanDat.UseNitaTimer && IsServer) {
hThNitaTimer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThNitaTimer, NULL, 0, &ThNitaTimerID);
}
//UU
if (IsServer) {
	DWORD ThHelp2ID;
	HANDLE hThHelp2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThHelp2, NULL, 0, &ThHelp2ID);
}
//UU
//--------------------------------------------------------------------------------------------------------------------
AuxWnd(hInstance);
//----------------------------------if(EnableEraPanel)----------------------------------------------------------------
if(EnableEraPanel){
	PanDat.pEraSt = &EraState;
	CALLPANEL CallPanel;
	hEvLoadPanelDll = CreateEvent(NULL, FALSE, FALSE, NULL);
	WaitForSingleObject(hEvLoadPanelDll, INFINITE); //ожидание создани€ терминального сокета дл€ получени€ SrvState
	if (PanDat.pSrvEraSt != NULL) {
		PanDat.UseNitaTimer = PanDat.pSrvEraSt->UseNitaTimer;
	}
	LoadLibrary(EraPanelLibName);
	HMODULE hECP;
	if(!(hECP=GetModuleHandle(EraPanelLibName))){
		ExitThread(0);
	}
	//U
	EFSet();
	//U
	CallPanel=(CALLPANEL)GetProcAddress(hECP,"CallPanel");
	hThHelper = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThHelper, NULL, 0, &ThHelperID);
	CallPanel(&PanDat);//передаЄм адрес структуры PanelData внутрь класса CPanel
	return 1;
}
//-----------------------------------fi(EnableEraPanel)---------------------------------------------------------------
ExitThread(0);
}
//--------------------------------------------------------------------------------------------------------------------
//UU
unsigned long _stdcall ThHelp2(void*) {
	while (1) {
		Sleep(2000);
		TerminalSendCommand(CODETICK, "", 0);
	}
	return 0;
}
//UU
//U
unsigned long _stdcall ThHelper(void*) {
	Sleep(10000);
	//::PostMessage(PanDat.hWndSelf, PanDat.UWM_Msg, 3, 11);
	return 0;
}
//U

//******************************************GetParamXML()********************************************
int GetParamXML() {
	nitaxml PSCfg;
	int tp, rez;
	int ret;
	
	GetEnvironmentVariable("NitaRoot", NitaRoot, sizeof(NitaRoot));
	sprintf(NitaTimerFullXML, "%s\\%s", NitaRoot, NitaTimerXML);
	
	if (!PSCfg.XMLOpen(NitaTimerFullXML) || !PSCfg.SetCurrentKey(NULL)) {
		return 0;
	}
	PSCfg.GetParam((char*)"SockMcAddrSend", NitaTimerMcAddr, &tp);
	PSCfg.GetParam((char*)"SockPortSend", &NitaTimerPort, &tp);
	PSCfg.GetParam((char*)"InterfaceSend", NitaTimerSenderIF, &tp);
	
	PSCfg.XMLClose();
	ret = (!strlen(NitaTimerMcAddr) || !NitaTimerPort) ? 0 : 1;
	return ret;
}


//******************************************GetParam()********************************************
int GetParam(){
UINT SYM;

GetEnvironmentVariableA("ProgramData", PathIni, sizeof(PathIni));
sprintf(PathIni, "%s\\ERA\\%s", PathIni, ERAini);
if (Cini.ReadIni(PathIni, 0)) { goto GETPARAM; }
GetSystemDirectory(PathIni, sizeof(PathIni));
sprintf(PathIni, "%s\\%s", PathIni, ERAini);
if (Cini.ReadIni(PathIni, 0)) { goto GETPARAM; }
GetWindowsDirectory(PathIni, sizeof(PathIni));
sprintf(PathIni, "%s\\%s", PathIni, ERAini);
if (Cini.ReadIni(PathIni, 0)) { goto GETPARAM; }
return 0;

GETPARAM:
PanDat.UseNitaTimer = NOT_USE_T;
if (!RunAsService) {//ключ командной строки "-s" имеет приоритет над значением "RunAsService" в Era.ini
	Cini.OpenParam("RunAsService", tmp, &RunAsService);
}
Cini.OpenParam("TCPPort",tmp,&TCPPort);
Cini.OpenParam("TCP_NODELAY",tmp,&TcpNoDelay);
Cini.OpenParam("LocalTestMode",tmp,&LocalTestMode);
Cini.OpenParam("EnableLog", tmp, &EnableLog);
Cini.OpenParam("DebugMode", tmp, &DebugMode);
Cini.OpenParam("Aerodrome", Aerodrome);
Cini.OpenParam("ECP", tmp, &USE_ECP);
if (USE_ECP) {
	Cini.OpenParam("Gate", Gate);
}
Cini.OpenParam("ID", tmp, &ID);
IsSender=FALSE;
while(Cini.SetSection("SOUND_SERVER")){
Cini.OpenParam("InputCard",WIDName);
Cini.OpenParam("mp3bitrate",tmp,&mp3bitrate);
Cini.OpenParam("SendSilentBlocks",tmp,&SendSilentBlocks);
Cini.OpenParam("TresholdOfSilent",tmp,&TresholdOfSilent);
Cini.OpenParam("SrvSocketRule",tmp,(int*)&ToPlaceKey);
Cini.OpenParam("MixToMono",tmp,&MixToMono);
Cini.OpenParam("UseNitaTimer", tmp, &PanDat.UseNitaTimer);
if (PanDat.UseNitaTimer) {
	Cini.OpenParam("NitaTimerPort", tmp, &NitaTimerPort);
	Cini.OpenParam("NitaTimerMcAddr", NitaTimerMcAddr);
	Cini.OpenParam("NitaTimerSenderIF", NitaTimerSenderIF);
	PanDat.UseNitaTimer = (!NitaTimerPort || !strlen(NitaTimerMcAddr) || !strlen(NitaTimerSenderIF)) ? \
		USE_T_PARAMS_FROM_XML : USE_T_PARAMS_FROM_INI;
	if (PanDat.UseNitaTimer == USE_T_PARAMS_FROM_XML) {
		if (!GetParamXML()) {
			PanDat.UseNitaTimer = 0;
		}
	}
}
Cini.OpenParam("FFBin", FFBin);
Cini.OpenParam("FFPreset", FFPreset);
Cini.OpenParam("FileSavePath", FileSavePath);
int lfsp = strlen(FileSavePath) - 1;
if (FileSavePath[lfsp] == '\\') {
	FileSavePath[lfsp] = 0;
}
Cini.OpenParam("NetSavePath", NetRes.Share);
strcpy(NetRes.MountPoint, "Y:");
Cini.OpenParam("NetUser", NetRes.User);
Cini.OpenParam("NetPwdCrypt", PwdCrypt);

int ii, jj, kk;
ii = jj = kk = 0;
while (PwdCrypt[ii] && PwdCrypt[ii] != '\r' && PwdCrypt[ii] != '\n') {
	tmp[jj] = PwdCrypt[ii];
	if (jj == 1) {
		tmp[jj + 1] = 0;
		SYM = strtoul(tmp, 0, 16);
		NetRes.Password[kk] = SYM; kk++;
		jj = -1;
	}
	ii++; jj++;
}
NetRes.Password[kk] = 0;
DeCPT(NetRes.Password, kk);

Cini.OpenParam("LogoffEventName", LogoffEventName);
Cini.OpenParam("StaticGain", tmp, &StaticGain);
	if(ToPlaceKey>0){
		Cini.UpdateParam("0");
	}
IsSender=IsServer=TRUE;
strcat(EventNameEra, "Server");
break;
}

IsPlayer=FALSE;
while(Cini.SetSection("SOUND_CLIENT")){
Cini.OpenParam("OutputCard",WODName);
Cini.OpenParam("LocalHostName",LocalHostName);
Cini.OpenParam("StaticConnect",StaticConnect);
Cini.OpenParam("nBlocksWait",tmp,&WaitBufferSize);
Cini.OpenParam("ReverseStereo",tmp,&ReverseStereo);
Cini.OpenParam("EnableEraPanel",tmp,&EnableEraPanel);
Cini.OpenParam("PostPressWinkey",tmp,&PostPressWinkey);
strcpy(PanDat.FilePath,"C:\\Temp\\");//ѕуть сохранки по умолчанию
Cini.OpenParam("FileSavePath",PanDat.FilePath);
Cini.OpenParam("SGPreset", SGPreset);
Cini.OpenParam("SGBin", SGBin);
Cini.OpenParam("SCRBin", PanDat.SCRBin);
Cini.OpenParam("SCRPartWndName", PanDat.SCRPartWndName);

PanDat.X = 300;
PanDat.XSize = 260;
Cini.OpenParam("XOffsetPanel", tmp, &PanDat.X);
Cini.OpenParam("XSizePanel", tmp, &PanDat.XSize);
PanDat.AddXSize = 0;//если не определено в ERA.ini
PanDat.AddYSize = 31;//если не определено в ERA.ini
Cini.OpenParam("AddXSize", tmp, &PanDat.AddXSize);
Cini.OpenParam("AddYSize", tmp, &PanDat.AddYSize);

	if(WaitBufferSize>64){
		WaitBufferSize=16;
		rez=Cini.UpdateParam(itoa(WaitBufferSize,tmp,10));
	}
	if (WaitBufferSize < 1) {
		WaitBufferSize = 1;
	}

	WaitBufferSize*=2304;
IsPlayer=IsClient=TRUE;
strcat(EventNameEra, "Client");
break;
}

IsProfile=FALSE;
initflag=0;
/////////////////≈сли определЄн профиль при запуске, забираем параметры профил€///////////////
while(strlen(Profile)){
 if(Cini.SetSection("RA")){
	Cini.OpenParam("user",user);
	Cini.OpenParam("passw",passw);
	Cini.OpenParam("WndName",Wnd_Name);
	Cini.OpenParam("RAbin",RAbin);
	Cini.OpenParam("param",Param);
 }
 if(Cini.SetSection(Profile)){
	if (Cini.OpenParam("Name", PanDat.Name) <= 0) { //если им€ не определено в профиле - берЄм им€ профил€ из командной строки
		strcpy(PanDat.Name, Profile);
	}
	Cini.OpenParam("TCPPort",tmp,&TCPPort);
	Cini.OpenParam("RAPort",tmp,&RAPort);
	Cini.OpenParam("connect",Connect);
	if (!strlen(Connect)) {
		strcpy(Connect, Profile);
	}
	if (!ID) {
		ID = GetIDByConnect(Connect);
	}
	Cini.OpenParam("NotUseEraSound",tmp,&NotUseEraSound);
	Cini.OpenParam("type",tmp,&type);//type=0 - RDP type=1 - RA
	Cini.OpenParam("user",user);
	Cini.OpenParam("passw",passw);
	Cini.OpenParam("WndName",Wnd_Name);
	Cini.OpenParam("RAbin",RAbin);
	Cini.OpenParam("param",Param);
	Cini.OpenParam("RAPartWndName",RAPartWndName);
	if(Cini.OpenParam("init",InitCmd)>0)//определена инициализаци€ без ожидани€ завершени€
	initflag|=1;
	if(Cini.OpenParam("initw",InitCmdW)>0)//определена инициализаци€ с ожиданием завершени€
	initflag|=2;
	Cini.OpenParam("SGPreset", SGPreset);
	PanDat.pSGPreset = SGPreset;
	Cini.OpenParam("AllowRemoteRecord", tmp, &PanDat.IsAllowRemoteRecord);
	Cini.OpenParam("EraLogin", tmp, &EraLogin);
	IsProfile=TRUE;
 }
break;
}
///////////////////////////////////////////////////////////////////////////////////////////////
if(Cini.SetSection("AUX_WND")){
IsAuxWnd=TRUE;
}

Cini.SaveAndCloseIni();

if(IsSender){
NumDevs=waveInGetNumDevs();
if(!NumDevs){
	LogMsg("No input sound devices were found!");
	return 0;
}
if (WIDName[0] == 0) { //не задано им€ звукового у-ва ввода, берЄм по умолчанию
	WID = WAVE_MAPPER;
}
else {//задано им€ звукового у-ва ввода
	while (NumDevs) {
		WID = NumDevs - 1;
		waveInGetDevCaps(WID, &WIDC, sizeof(WAVEINCAPS));
		if (!strcmp(WIDC.szPname, WIDName)) { break; }
		NumDevs--;
		if (!NumDevs) {
			LogMsg("The specified input sound device was not found! = \"", WIDName,"\"");
			return 0;
		}
	}
}
}//fi(IsSender)

WOD=0;
if(strlen(WODName)){ //задано им€ звукового у-ва воспроизведени€
	if(IsPlayer){
		NumDevs = waveOutGetNumDevs();
		if (!NumDevs) {
			LogMsg("No output sound devices were found!");
			return 0;
		}
		while (NumDevs) {
			WOD = NumDevs - 1;
			waveOutGetDevCaps(WOD, &WODC, sizeof(WAVEINCAPS));
			if (!strcmp(WODC.szPname, WODName)) { break; }
			NumDevs--;
			if (!NumDevs) {
				LogMsg("The specified input sound device was not found! = \"", WODName, "\"");
				return 0;
			}
		}//while(NumDevs)
	}//fi(IsPlayer)
}//fi(strlen(WODName))
else {//не задано им€ звукового у-ва воспроизведени€, берЄм по умолчанию
	WOD = WAVE_MAPPER;
}
if(IsPlayer && !IsProfile){StaticClientMode=TRUE;}

return (IsSender|IsPlayer|IsProfile);
}

//####################################################CheckFamatech()########################################################
int CheckFamatech(){
DWORD NOR;
GetSystemDirectory(famatech,128);
strcat(famatech,"\\famatech.dat");
HANDLE hfam=CreateFile( (LPCTSTR)famatech,GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,0,
				OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,0);
if(hfam==INVALID_HANDLE_VALUE)
return 0;
int gfs=GetFileSize(hfam,NULL);
if(gfs<8 || gfs>128)
goto CH;
ReadFile(hfam,RAbin,gfs,&NOR,NULL);
RAbin[gfs]=0;
CH:
CloseHandle(hfam);
return 1;
}
//####################################################CheckFamatech()########################################################
//############################################################################################################
int GetIDByConnect(char *Connect) {
struct sockaddr_in clnt;
hostent *phst;
int RetID = 0;
int I1, I2, I3;
char ChID[64];

	if (Connect[0] < 0x40) {//m.е. задан обычный IP-адрес
		clnt.sin_addr.s_addr = inet_addr((const char*)Connect);
	}
	else {// >=0x40, задано им€ и необходимо определить IP-адрес по имени
		phst = gethostbyname((const char*)Connect);
		if (phst == NULL) {
			return 0;
		}
		DWORD* pdw = (DWORD*)phst->h_addr_list[0];
		clnt.sin_addr.s_addr = *pdw;
	}
	I1= clnt.sin_addr.S_un.S_un_b.s_b1;
	I2 = clnt.sin_addr.S_un.S_un_b.s_b2;
	I3 = clnt.sin_addr.S_un.S_un_b.s_b3;
	sprintf(ChID, "%d%d%d", I1, I2, I3);
	RetID = atoi(ChID);
	return RetID;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ParseCmdLine(char *pcmd, char *Profile, char *ERA_ini, int* pRunAsServ) {
	int i, j, k, n, len;
	char *p[2];
	char swap[128];
	
	Profile[0] = 0;
	len = strlen(pcmd);
	if (!len) {
		return 1;
	}
	if (strstr(pcmd, "-s")) {
		*pRunAsServ = 1;
		return 1;
	}
	p[0] = Profile; p[1] = ERA_ini;
	j = k = 0;
	for (i = 0; i < len; i++) {
		if (pcmd[i] == ' ') {
			*(p[j] + k) = 0;
			if (!j) { 
				j++; k = 0;
			}
			continue;
		}
		*(p[j] + k) = pcmd[i];
		n = j + 1;
		k++;
	}
	*(p[j] + k) = 0;
	if (n == 1 && strstr(p[0], ".ini")) {
		strcpy(p[1], p[0]);
		Profile[0] = 0;
	}
	else if (n ==2 && strstr(p[0], ".ini")) {
		strcpy(swap, p[0]);
		strcpy(p[0], p[1]);
		strcpy(p[1], swap);
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FillDefaultEraState(PERAState pest) {
	ZeroMemory(pest, sizeof(ERAState));
	pest->F903 = (WORD)0xF903;
	//pest->IsMonoMode = 0;
	//pest->SelectRecMode = AUDIOMODE;
	//pest->IsRemoteRecord = 0;
	//pest->RecFileName[0] = 0;
	//pest->MapChParams = 0;
	//pest->NitaTimerSelect = 0;
	//pest->LenTimerSelect = 0;
	//pest->RecFileName[0] = 0;
	pest->IsAutoCopy = 1;
	pest->LenTimer.wHour = 2;
}

void EFSet(/*PanelData PanDat*/) {
	PanDat.ef.TerminalSendCommand = TerminalSendCommand;
	PanDat.ef.TerminalSendCommandAndState = TerminalSendCommandAndState;
	PanDat.ef.LogMsg1 = LogMsg;
	PanDat.ef.LogMsg2 = LogMsg;
	PanDat.ef.LogMsg3 = LogMsg;
	PanDat.ef.LogMsg4 = LogMsg;
	PanDat.ef.LogMsg5 = LogMsg;
	PanDat.ef.LogMsg6 = LogMsg;
	PanDat.ef.LogMsg7 = LogMsg;
	PanDat.ef.LogMsg8 = LogMsg;
	PanDat.ef.LogMsg9 = LogMsg;
	PanDat.ef.LogMsg10 = LogMsg;
}