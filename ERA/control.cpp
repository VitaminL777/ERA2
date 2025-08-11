#include "Winsock2.h"
#include "ws2tcpip.h"
#include <Mstcpip.h>
#include <windows.h>
#include "structs.h"
#include "control.h"
#include "regy.h"
#define HAVE_MEMCPY  //для запрета переопределения функции memcpy в cmp3.h
#include "cmp3.h"
#include "ECP.h"
#include "NitaTimer.h"
#include "MediaSrv.h"
#include "LogMsg.h"

SOCKET nSMTPServerSocket=0;
SOCKET sockSRV=INVALID_SOCKET;
SOCKET sockTRM=INVALID_SOCKET;
SOCKET sockSNDS=INVALID_SOCKET;
SOCKET sockSNDR=INVALID_SOCKET;
SOCKET clnt;
fd_set rdfd;
timeval SockTime;
struct sockaddr_in srva;
struct sockaddr_in dest;
struct sockaddr_in local;
struct sockaddr_in out;
struct ip_mreq mcast;
BOOL IsSessionExist=FALSE;
BOOL IsSndExist=FALSE;
BOOL IsTerminal=FALSE;
BOOL IsDisconAnotherUser = FALSE;
BOOL IsRefuseDisconn = FALSE;
BOOL IsShowTimerPanel = FALSE;
extern Cmp3 mp3;
extern BOOL IsServer;
extern BOOL IsClient;
extern int TCPPort;
extern int TcpNoDelay;
struct tcp_keepalive alive;
extern int SendSilentBlocks;
extern int MixToMono;
extern BOOL IsRemoteRecord;
extern DWORD RemoteRecTimer;
extern char LocalHostName[96];
extern PanelData PanDat;
extern ERAState EraState;
extern ERAState SrvState;
extern HANDLE hEvLoadPanelDll;
extern int USE_ECP;
extern UFileTime TSrvPlayStop;
int nConnect;
int iLength;
int iMsg = 0;
int iEnd = 0;
static WORD CodeOper;
int wsrez;
int ll;
int sb;
int outlen;
char ResponseData[256];
char keystr[256];
char sBuf[4096];
char TermName[96];
char AnotherUser[96];
char ActiveConnect[128];
char WarningConnect[256];
char WarningConnect1[]="В данный момент АРМ используется клиентом '";
char WarningConnect2[]="', произвести его принудительное отключение?";
char WarnRRDeny[] = "На подключаемом АРМ активирован режим удалённой записи, но в контексте данного подключения он недоступен, подключение будет разорвано.";
char tmpc[256];
PDataContainer PDC;
static STARTUPINFO si;
static PROCESS_INFORMATION pi;
WORD SrvContainerKey=0;
extern WORD ToPlaceKey;
//U
char tempo[512];
//U
int RunCmd(PDataContainer PDC);
int CloseActiveWindow();
int TerminalAlive();
int CreateSession();
int CloseSession(int Mode = 0);
int CreateSnd();
int CloseSnd();
int CloseAll();
int SendCode(WORD SCode, char* Data=0);
int SendCodeAndState(WORD SCode, PERAState pEraSt);
int FlagHandler(PDataContainer PDC);
int LogoffHandler();
int EraLoginHandler();
int GetSrvState(PDataContainer PDC /*, PERAState SrvState*/);
int opt;
DWORD dwSize;
int ECPRetCode = 0;
int ChanSelNitaTimer = CH_NONE;
int ChanSelLenTimer = CH_NONE;
extern int ID;
extern char Gate[128];
extern ECPACK EP;
extern char EventNameEra[64];
extern char LogoffEventName[128];
char MTM;
HANDLE hEvExit;
HANDLE hMutTSC;
PERAState pestr = NULL;
extern HANDLE hEventPlay;
extern bool IsControlStopEvent;
extern BOOL EraLogin;
SendStruct SSTerm;
SOCKET socktemp;
//extern HANDLE hThMediaSrv;
extern DWORD ThMediaSrvID;
extern BOOL RRecStart;
extern BOOL RRecStop;
//extern BOOL IsRRecVideo;
//extern BOOL IsRRecAudio;
//extern BOOL IsRRecMp3Stop;
//extern BOOL IsRRecMp4Stop;
extern U64D LastNetCopy;
extern BOOL CancelNetCopy;

int RRecStartF();
int RRecStopF();
int RRecPauseF();

#include "LenTimer.hpp"

unsigned long _stdcall ThControl(void*){

hEvExit = CreateEvent(NULL, FALSE, FALSE, NULL);
PanDat.hEvent[0]=CreateEvent(NULL,FALSE,FALSE, EventNameEra);
hMutTSC = CreateMutex(NULL, FALSE, NULL);

sockSRV=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
srva.sin_family = AF_INET;   
srva.sin_addr.s_addr = htonl(INADDR_ANY);
srva.sin_port = htons(TCPPort);


wsrez=bind(sockSRV,(struct sockaddr*)&srva,sizeof(srva));
wsrez=listen(sockSRV,8);
//U_reconstruct
alive.onoff = 1;
alive.keepalivetime = 5000;
alive.keepaliveinterval = 1000;
wsrez = WSAIoctl(sockSRV, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &dwSize, NULL, NULL);
dwSize = sizeof(int);
//U_reconstruct

outlen=sizeof(out);
PDC=(_DataContainer*)sBuf;


SockTime.tv_sec=1000;
SockTime.tv_usec=0;

while (1) {
	FD_ZERO(&rdfd);
	FD_SET(sockSRV, &rdfd);
	if (sockTRM != -1) {
		FD_SET(sockTRM, &rdfd);
	}
	wsrez = select(0, &rdfd, NULL, NULL, &SockTime);
	if (!wsrez) {
		TerminalAlive();
		continue;
	}
	if (FD_ISSET(sockTRM, &rdfd)) {
		clnt = sockTRM;
		IsTerminal = TRUE;
		goto RECV;
	}
	if (!FD_ISSET(sockSRV, &rdfd)) {
		continue;
	}

	IsTerminal = FALSE;
	clnt = accept(sockSRV, (struct sockaddr*)&out, &outlen);
RECV:
	wsrez = sizeof(DataContainer);
	wsrez = recv(clnt, sBuf, 4096, 0);
	if (wsrez == -1 || !wsrez) {
		if (clnt != -1) { closesocket(clnt); }
		if (IsTerminal) { 
			CloseSession(); 
		}
		continue;
	}
	///////////////////////////////Защита//////////////////////////////////////////////////////
	//KeyOperations();//SrvContainerKey
	//ECPRetCode = DataExchange(Gate, ID, 1);//SrvContainerKey
	///////////////////////////////Защита//////////////////////////////////////////////////////
	/*PDC=(_DataContainer*)sBuf;*/
	if (PDC->F930 != SrvContainerKey) {
		if (!IsTerminal) //Ключик не подошёл, да ещё и не терминал - левый коннект
			closesocket(clnt);//поэтому закрываем его
		continue;
	}
	if (PDC->CO >= COREMOTE) {//проверка, что пришла команда на удалённый терминал
		if (PDC->CO < CODEBUSY)// и не код ответа
			TerminalSendCommand(PDC);
		goto ENDSELECT;
	}
	CodeOper = PDC->CO;
	if (IsServer) { //приём команд серверной стороной==============================================================================
		switch (CodeOper)
		{
		case RUN:
			SendCode(CODEOK);
			RunCmd(PDC);
			break;

		case RUNVISIBLE:
			SendCode(CODEOK);
			RunCmd(PDC);
			break;

		case CLOSEAW:
			SendCode(CODEOK);
			CloseActiveWindow();
			break;

		case CREATESESSION:
			CreateSession();
			break;

		case CREATESESSIONERALOGIN:
			CreateSession();
			EraLoginHandler();
			break;

		case GETSTATE:
			EraState.F903 = (WORD)0xF903;
			SendCodeAndState(CODEOK, &EraState);
			break;

		case CLOSEALL:
			TerminalSendCommand(CLOSEACCEPT, "", 0);
			Sleep(50);
			CloseAll();
			break;

		case CLOSEALLSAVEERST:
			EraState = *(PERAState)PDC->Data; //сохраняем EraState при отключении клиента от сервера
			TerminalSendCommand(CLOSEACCEPT, "", 0);
			Sleep(50);
			CloseAll();
			break;

		case MESSBOX:
			SendCode(CODEOK);
			strcpy(tmpc, PDC->Data);
			MessageBox(0, tmpc, "Warning", MB_ICONWARNING | MB_SYSTEMMODAL);
			break;

		case CREATESND:
			CreateSnd();
			break;

		case DISCONANOTHERUSER:
			if (!IsServer) {
				strcpy(AnotherUser, PDC->Data);
				IsDisconAnotherUser = 1;
			}
			break;

		case FLAGKEY:
			SendCode(CODEOK);
			FlagHandler(PDC);
			break;

		case LOGOFF:
			LogoffHandler();
			break;

		case ERALOGIN:
			EraLoginHandler();
			break;

		case CHANGEMODE: //здесь имеется ввиду моно/стерео режим
			MixToMono = (MixToMono + 1) & 1; //инверсия режима(моно/стерео)
			EraState.IsMonoMode = (char)MixToMono;
			MTM = (char)MixToMono;
			SendCode(CODEOK, &MTM); //возвращаем вновь установленный режим (старый формат возврата состояния)
			break;

		case CHANGESELRECMODE:
			EraState.SelectRecMode = ((PERAState)PDC->Data)->SelectRecMode;
			SendCodeAndState(CODEOK, &EraState);
			break;
		
		case CHANGERECPARAMS:
			pestr = (PERAState)PDC->Data;
			
			if (pestr->MapChParams) {
				if (pestr->MapChParams&CH_TYPETIMERSEL) {
					ChanSelNitaTimer = HOW_CH(pestr->NitaTimerSelect, EraState.NitaTimerSelect);
					EraState.NitaTimerSelect = pestr->NitaTimerSelect;
					ChanSelLenTimer = HOW_CH(pestr->LenTimerSelect, EraState.LenTimerSelect);
					EraState.LenTimerSelect = pestr->LenTimerSelect;
				}
				if (pestr->MapChParams&CH_NITATIMERVALUE) {
					EraState.NitaTimer = pestr->NitaTimer;
					if (EraState.NitaTimerSelect) {
						ChanSelNitaTimer = CH_TO_SEL;
					}
				}
				if (pestr->MapChParams&CH_LENTIMERVALUE) {
					EraState.LenTimer = pestr->LenTimer;
					if (EraState.LenTimerSelect) {
						ChanSelLenTimer = CH_TO_SEL;
					}
				}
				if (pestr->MapChParams&CH_AUTOCOPY) {
					EraState.IsAutoCopy = pestr->IsAutoCopy;
				}
				if (pestr->MapChParams&CH_FILENAME) {
					strcpy(EraState.RecFileName, pestr->RecFileName);
				}
			}
			EraState.MapChParams = pestr->MapChParams;
			SendCodeAndState(CHANGERECPARAMS, &EraState); //в данном случае на клиентской стороне будет расцениваться как подтверждение приёма параметров
			EraState.MapChParams = 0;

			if (ChanSelNitaTimer == CH_TO_SEL) { //если выбран/обновлён NitaTimer сразу предустанавливаем его
				NitaTimerPreSet(&EraState.NitaTimer);
				if (EraState.RecordState&RS_LRECORD) { //если ещё и запись идёт в этот момент - активируем таймер
					NitaTimerActivate();
				}
				else if (EraState.RecordState&RS_RRECORD) {
					NitaTimerActivate(NULL, TRUE);
				}
			}
			else if (ChanSelNitaTimer == CH_TO_DESEL) { //если снят NitaTimer
				NitaTimerDeactivate();
			}
			break;

		case LRECSTART:
		case LRECSTOP:
			pestr = (PERAState)PDC->Data;
			EraState.RecordState = pestr->RecordState;
			if (EraState.RecordState&RS_LRECORD) {
				if (EraState.NitaTimerSelect) {
					if (NitaTimerStateA.load() == NTS_NOSET) {
						NitaTimerPreSet(&EraState.NitaTimer);
					}
					NitaTimerActivate();
				}
			}
			break;

		case RRECSTART:
			if (ThMediaSrvID) { //Уже(ещё) существует поток MediaSrv
				TerminalSendCommand(RREСTERMINATESRV, "", 0);
				EraState.RecordState = RS_STOP;
				break;
			}
			RRecStart = TRUE;
			if (EraState.NitaTimerSelect) {
				if (NitaTimerStateA.load() == NTS_NOSET) {
					NitaTimerPreSet(&EraState.NitaTimer);
				}
				NitaTimerActivate(NULL,TRUE);
			}
			goto RRR;
		case RRECSTOP:
			if (!ThMediaSrvID) { //Не существует поток MediaSrv
				pestr = (PERAState)PDC->Data;
				EraState.RecordState = pestr->RecordState;
				SendCodeAndState(RRECSTATEACCEPT, &EraState); //подтверждение изменения состояния удалённой записи для клиента
				break;
			}
			RRecStop = TRUE;
			NitaTimerDeactivate();
			goto RRR;
		case RRECPAUSE:
			if (!ThMediaSrvID) { //Не существует поток MediaSrv
				break;
			}
RRR:
			pestr = (PERAState)PDC->Data;
			EraState.RecordState = pestr->RecordState;
			SendCodeAndState(RRECSTATEACCEPT, &EraState); //подтверждение изменения состояния удалённой записи для клиента
			if (RRecStart && !RRecStop) {
				RRecStartF();
			}
			else if (RRecStart && RRecStop) {
				RRecStopF();
			}
			else if (RRecStart && !RRecStop) {
				RRecPauseF();
			}
			break;

		case SHOWTIMERPANEL:
			IsShowTimerPanel = TRUE;
			LogMsg("Receive SHOWTIMERPANEL");
			if (LastNetCopy.DW2 == 1) {//есть завершённые копирования файлов медиа по сети
				TerminalSendCommand(SHOWPERCNETCOPY, &LastNetCopy, 0);
			}
			break;

		case CLOSETIMERPANEL:
			IsShowTimerPanel = FALSE;
			LogMsg("Receive CLOSETIMERPANEL");
			break;

		case ABORTNETCOPY:
			CancelNetCopy = TRUE;
			LogMsg("Receive ABORTNETCOPY");
			break;

		default:
			break;
		}
	}//fi (IsServer)
	if (IsClient) {//приём команд клиентской стороной==============================================================================
		switch (CodeOper) {
		case CREATESESSION:
			CreateSession();
			break;

		case CLOSEACCEPT:
			CloseSession();
			break;
		
		case CHANGERECPARAMS: //в данном случае будет расцениваться как подтверждение приёма параметров серверной стороной
			pestr = (PERAState)PDC->Data;
			if (pestr->MapChParams) {
				if (pestr->MapChParams&CH_TYPETIMERSEL) {
					EraState.NitaTimerSelect = pestr->NitaTimerSelect;
					EraState.LenTimerSelect = pestr->LenTimerSelect;
				}
				if (pestr->MapChParams&CH_NITATIMERVALUE) {
					EraState.NitaTimer = pestr->NitaTimer;
				}
				if (pestr->MapChParams&CH_LENTIMERVALUE) {
					EraState.LenTimer = pestr->LenTimer;
				}
				if (pestr->MapChParams&CH_AUTOCOPY) {
					EraState.IsAutoCopy = pestr->IsAutoCopy;
				}
				if (pestr->MapChParams&CH_FILENAME) {
					strcpy(EraState.RecFileName, pestr->RecFileName);
				}
			}
			EraState.MapChParams = 0;
			break;

		case NITATIMERSIGNAL: //приём сигнала остановки записи от NitaTimer серверной стороны
			if (PanDat.IsRecord) {
				::PostMessage(PanDat.hWndSelf, PanDat.UWM_Msg, PANMSG_STOP, 0); //отправляем сообщение панели программно нажать "Стоп"
			}
			break;
		
		case NTSPLAYSTOP:
			if (!PanDat.IsRecord) { //в процессе записи отключаем ускорение остановки
				GetSystemTimeAsFileTime(&TSrvPlayStop.FileTime);
				SetEvent(hEventPlay); IsControlStopEvent = true;//сигналим потоку воспроизведения, что СТОП
			}
			break;

		case RRECSTATEACCEPT: //приём подтверждения изменения состояния удалённой записи от серверной стороны
			pestr = (PERAState)PDC->Data;
			switch (pestr->RecordState) {
			case RS_RRECORD:
				::PostMessage(PanDat.hWndSelf, PanDat.UWM_Msg, PANMSG_RRSTART, 0);
				break;
			
			case RS_STOP:
				::PostMessage(PanDat.hWndSelf, PanDat.UWM_Msg, PANMSG_RRSTOP, 0);
				break;
			
			case RS_RPAUSE:
				::PostMessage(PanDat.hWndSelf, PanDat.UWM_Msg, PANMSG_RRPAUSE, 0);
				break;

			default:
				break;

			}
			break;

		case RREСTERMINATESRV: //Abnormal terminate record
			EraState.RecordState = RS_STOP;
			::PostMessage(PanDat.hWndSelf, PanDat.UWM_Msg, PANMSG_RRTERMINATE, 0); //отправляем сообщение панели об аварийной остановке записи
			break;

		case SHOWPERCNETCOPY:
			U64D udprog = *(U64D*)PDC->Data;
			if (PanDat.hWndEraTimer != NULL) {
				if (udprog.DW2 == NC_COMPLETED) {
					strcpy(PanDat.NetCopyFileName, udprog.Data);
					::PostMessage(PanDat.hWndEraTimer, PanDat.UWM_Msg, PANMSG_RRTERMINATE, 1); //сигнал прописать "ФАЙЛ СКОПИРОВАН"
				}
				else if (udprog.DW2 == NC_CANCELED) {
					::SendMessage(PanDat.hWndPrCop, 0x402, 0, 0);
					::PostMessage(PanDat.hWndEraTimer, PanDat.UWM_Msg, PANMSG_RRTERMINATE, 2);
				}
				else if (PanDat.hWndPrCop != NULL) { //udprog.DW2 != NC_COMPLETED && udprog.DW2 != NC_CANCELED
					strcpy(PanDat.NetCopyFileName, udprog.Data);
					//отправляем сигнал отобразить имя копируемого файла
					::PostMessage(PanDat.hWndEraTimer, PanDat.UWM_Msg, PANMSG_RRTERMINATE, 0);
					//PBM_SETPOS = 0x402;
					//отправка сигнала перерисовать процент копирования прогресс-индикатора
					::SendMessage(PanDat.hWndPrCop, 0x402, (WPARAM)udprog.DW1, 0);
				}
			}
			break;

		default:
			break;
		}
	}//fi (IsClient)

ENDSELECT:
	if (clnt != -1 && !IsTerminal) //сокеты, которые нужны, забираются в соответ. case с пометкой clnt=-1,
		closesocket(clnt);//чтоб их не закрыть, остальные, кроме терминального - закрываем, т.к. они разовые
}//end_while(1)


return 0;
}
////////////////////////int SendCode()////////////////////////////////////////////////
int SendCode(WORD SCode, char* Data){
DataContainer DCSC;
DCSC=*PDC;
DCSC.CO=SCode;
if (Data) {
	strcpy(DCSC.Data, Data);
}
wsrez=send(clnt,(char*)&DCSC,sizeof(DataContainer),0);
return 1;
}

//////////////////////////////////////////////////////////////////////////////////////
int SendCodeAndState(WORD SCode, PERAState pEraSt) {
DataContainer DCSC;
DCSC = *PDC;
DCSC.CO = SCode;

*(PERAState)DCSC.Data = *pEraSt;
wsrez = send(clnt, (char*)&DCSC, sizeof(DataContainer), 0);
return 1;
}

//************************int RunCmd(PDataContainer PDC)**************************
int RunCmd(PDataContainer PDC){

memset(&si,0,sizeof(STARTUPINFO));
memset(&pi,0,sizeof(PROCESS_INFORMATION));
si.cb=sizeof(STARTUPINFO);
si.dwFlags=STARTF_FORCEOFFFEEDBACK|STARTF_USESHOWWINDOW;

if(PDC->CO==RUNVISIBLE)
si.wShowWindow=SW_SHOWNORMAL;
else
si.wShowWindow=SW_HIDE;


if(!CreateProcess(NULL,PDC->Data,NULL,NULL,0,0,NULL,NULL,&si,&pi)){
	return 0;
}

CloseHandle(pi.hThread);
CloseHandle(pi.hProcess);

return 1;
}

//************************int CloseActiveWindow()******************************
int CloseActiveWindow(){
HWND haw=GetForegroundWindow();
PostMessage(haw,WM_CLOSE,0,0);
CloseHandle(haw);

return 1;
}


//*************************int TerminalAlive()*********************************
int TerminalAlive(){

	return 0;
}

//*************************int CreateSession()**********************************

int CreateSession(){
	if(IsSessionExist){
		SendCode(CODEBUSY,TermName);
		return 0;
	}
	else{
		//SendCode(CODEOK);
		//FillEraState(&EraState);
		if (IsServer) {
			SendCodeAndState(CODEOK, &EraState); //отправка состояния от сервера - клиенту
		}
		if (IsClient) {
			SendCode(CODEOK); //отправка подтверждения от клиента серверу
		}
		if(sockTRM==-1){
		strcpy(TermName,PDC->Data);
		sockTRM=clnt;
		clnt=-1;
		}
		else{//sockTRM==any_socket
		}
		IsSessionExist=TRUE;
		return 1;
	}
}


//***************************************int CloseSession()********************************************
int CloseSession(int Mode){
	if (Mode == 0) {
		if(USE_ECP){
			ECPRetCode = DataExchange(Gate, ID, 0);
		}
		else {
			ECPRetCode = 3;
			SrvContainerKey = 0xF930;
		}
		SetEvent(hEvExit);
	}
	if(sockTRM!=-1){closesocket(sockTRM);}
	IsSessionExist=FALSE;
	sockTRM=-1;

	return 1;
}

//*************************int CloseAll()***************************************************************
//---функция_закрытия_по_сигналу_из_вне__есть_также_функция_внутреннего_закрытия_IntCloseAll()
int CloseAll(){
	if(!IsTerminal){//самому себе не нужно отправлять сигнал об отключении другим юзером,т.к. сам себя отключаешь
	PDC->CO=DISCONANOTHERUSER;
	TerminalSendCommand(PDC);
	}
	if (IsSndExist) {
		CloseSnd();
	}
	if (IsSessionExist) {
		CloseSession();
	}
	SendCode(CODEOK);
	return 1;
}

//*************************int CreateSnd()****************************************
int CreateSnd(){
	if(IsSndExist){
		CloseSnd();
	}
		SendCode(CODEOK);
		setsockopt(clnt,IPPROTO_TCP,TCP_NODELAY,(char*)&TcpNoDelay,4);
		sockSNDS=clnt;
		clnt=-1;
		IsSndExist=TRUE;
		return 1;
	
}


//*****************************int CloseSnd()**************************************
int CloseSnd(){
	if(sockSNDS!=-1){closesocket(sockSNDS);}
	sockSNDS=INVALID_SOCKET;
	IsSndExist=FALSE;
	
	return 1;
}

//*************************int DataComm(PSendStruct PSS, SOCKET *PS, BOOL Response=0)**************
int DataComm(PSendStruct PSS, SOCKET *PS, BOOL Response){

struct sockaddr_in clnt;
hostent *phst;
int wsrez;
int NewSocket=0;

if(*PS==INVALID_SOCKET)
{
		NewSocket=1;
		*PS=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		clnt.sin_family = AF_INET;   
		clnt.sin_port = htons(TCPPort);

	if(PSS->IPDest[0] < 0x40){//m.е. задан обычный IP-адрес
		clnt.sin_addr.s_addr = inet_addr(PSS->IPDest);
	}
	else{// >=0x40, задано имя и необходимо определить IP-адрес по имени
		if(!(phst=gethostbyname(PSS->IPDest))){goto CLOSESOCK;} //Имя не разрешено - выходим
		DWORD* pdw=(DWORD*)phst->h_addr_list[0];
		clnt.sin_addr.s_addr=*pdw;
	}
	wsrez=connect(*PS,(sockaddr*) &clnt,sizeof(clnt)); 
	if(wsrez){
CLOSESOCK:
		closesocket(*PS);
		*PS=INVALID_SOCKET;
		return 0;
	}
}//fi(*PS==INVALID_SOCKET)
 PSS->DataCont.F930=0xF930;

wsrez=send(*PS,(char*)&PSS->DataCont,sizeof(DataContainer),0);
if(wsrez<=0){return 0;}
if(NewSocket||Response){//на терминальном сокете подтверждение просматривается только при установленном флаге Response
wsrez=recv(*PS,(char*)&PSS->DataCont,sizeof(DataContainer),0);
if(wsrez<=0){return 0;}
}
return 1;

}


//*************************int CreateTerminal(char*)***********************************
int CreateTerminal(char* Addr) {

	int YESNO;

	CloseSession(1);
	if (!strlen(LocalHostName))
		gethostname(LocalHostName, sizeof(LocalHostName));
ACT:
	strcpy(SSTerm.IPDest, Addr);
	if (EraLogin) {
		SSTerm.DataCont.CO = CREATESESSIONERALOGIN;
	}
	else {
		SSTerm.DataCont.CO = CREATESESSION;
	}
	strcpy(SSTerm.DataCont.Data, LocalHostName);
	socktemp = -1;
	if (!DataComm(&SSTerm, &socktemp)) {
		goto CLOSECT; 
	}

	if (SSTerm.DataCont.CO == CODEBUSY)
	{
		if (!strcmp(SSTerm.DataCont.Data, LocalHostName)) { goto CLOSEUSER; }//Не выводим сообщение, если терминал занят нами же(некорректно закрытая сессия) 
		strcpy(WarningConnect, WarningConnect1);
		strcat(WarningConnect, SSTerm.DataCont.Data);
		strcat(WarningConnect, WarningConnect2);
		YESNO = MessageBox(NULL, WarningConnect, "Предупреждение", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_SYSTEMMODAL);
		if (YESNO != IDYES) { IsRefuseDisconn = 1; return 0; } //Ставим флаг отказа от дисконнекта другого юзера и выходим
	CLOSEUSER:
		if (socktemp != -1) { closesocket(socktemp); socktemp = -1; }
		strcpy(SSTerm.IPDest, Addr);
		SSTerm.DataCont.CO = CLOSEALL;	//Процедура отключения другого юзера
		strcpy(SSTerm.DataCont.Data, LocalHostName);
		if (!DataComm(&SSTerm, &socktemp)) { 
			goto CLOSECT; 
		}
		Sleep(50);
		goto ACT;
	}//fi(SSTerm.DataCont.CO==CODEBUSY)



	if (SSTerm.DataCont.CO == CODEOK)
	{
		if (GetSrvState(&SSTerm.DataCont/*, PERAState SrvState*/)) {
			if (!PanDat.IsAllowRemoteRecord && ((SrvState.RecordState&RS_RRECORD) || SrvState.SelectRecMode == RRVIDEOMODE)) {
				MessageBox(NULL, WarnRRDeny, "Предупреждение!", MB_ICONWARNING | MB_OK);
				IsRefuseDisconn = 1;
				goto CLOSECT;
			}
			PanDat.IsMono = PanDat.RealMonoMode = SrvState.IsMonoMode;
			PanDat.pSrvEraSt = &SrvState;
			//?
			EraState = SrvState;
			EraState.RecordState &= ~RS_LRECORD; //выключаем возможный признак локальной записи при подключении(если "затесался" на серверной стороне)
			LogMsg("On CreateTerminal EraState.RecordState=", (int)EraState.RecordState);
			PanDat.CoordX = EraState.PanCoordX;
			PanDat.CoordY = EraState.PanCoordY;
			//?
			SetEvent(hEvLoadPanelDll); //даём команду загрузить ERAPANEL2.dll
		}

		sockTRM = socktemp;
		strcpy(SSTerm.IPDest, "127.0.0.1"); //также отправляем сигнал своему обработчику взять данный
		SSTerm.DataCont.CO = CREATESESSION;  //сокет как терминальный в процедуру select
		socktemp = INVALID_SOCKET;
		if (!DataComm(&SSTerm, &socktemp)) { 
			goto CLOSECT; 
		}
		if (socktemp != -1) { closesocket(socktemp); socktemp = -1; } //локальный сокет сразу закрываем,т.к. он нужен был только для сигнализации
		if (SSTerm.DataCont.CO == CODEOK) { 
			return 1; 
		}
	}
CLOSECT:
	if (socktemp != -1)
		closesocket(socktemp);
	if (sockTRM != -1)
		closesocket(sockTRM);
	return 0;
}

//*************************int SendCommand(char* Addr, WORD CO, char* Data)************************************
int SendCommand(char* Addr, WORD CO, char* Data)
{
SendStruct SSSC;
SOCKET socktemp;

if(!Addr)
strcpy(SSSC.IPDest,ActiveConnect);
else{//if(Addr)
	if(!strlen(Addr))
	strcpy(SSSC.IPDest,ActiveConnect);
	else
	strcpy(SSSC.IPDest,Addr);
}
SSSC.DataCont.CO=CO;
strcpy(SSSC.DataCont.Data,Data);
socktemp=-1;
if(!DataComm(&SSSC,&socktemp)){return 0;}
if(CO==CREATESND)
	sockSNDR=socktemp;

return 1;
}

//*************************int TerminalSendCommand(WORD CO, char* Data)****************************************
int TerminalSendCommand(WORD CO, char* Data, BOOL Response)
{
SendStruct SSTS;
	if(sockTRM==-1 || !IsSessionExist){return 0;}

WaitForSingleObject(hMutTSC, INFINITE);
SSTS.DataCont.CO=CO;
strcpy(SSTS.DataCont.Data,Data);
if(!DataComm(&SSTS,&sockTRM,Response)){
	ReleaseMutex(hMutTSC);
	return 0;
}
if (Response) {
	memcpy(ResponseData, SSTS.DataCont.Data, 256); //данные команды ответа, если нужны
}
ReleaseMutex(hMutTSC);
return 1;
}

//-----отправка массива чисел(4 UINT64 или 8 DWORD), размещённых в DataCont.Data-----
int TerminalSendCommand(WORD CO, U64D* pUD, BOOL Response)
{
	SendStruct SSTS;
	if (sockTRM == -1 || !IsSessionExist) { return 0; }
	U64D u64d;

	WaitForSingleObject(hMutTSC, INFINITE);
	SSTS.DataCont.CO = CO;
	*(U64D*)(SSTS.DataCont.Data) = *pUD;
	if (!DataComm(&SSTS, &sockTRM, Response)) {
		ReleaseMutex(hMutTSC);
		return 0;
	}
	if (Response) {
		memcpy(ResponseData, SSTS.DataCont.Data, 256); //данные команды ответа, если нужны
	}
	ReleaseMutex(hMutTSC);
	return 1;
}

//*************************int TerminalSendCommand(PDataContainer PDC)***********************************
int TerminalSendCommand(PDataContainer PDC, BOOL Response)
{
SendStruct SSTS;
	if(sockTRM==-1 || !IsSessionExist){return 0;}
WaitForSingleObject(hMutTSC, INFINITE);
SSTS.DataCont=*PDC;
SSTS.DataCont.CO &= ~COREMOTE; 
if(!DataComm(&SSTS,&sockTRM,Response)){
	ReleaseMutex(hMutTSC);
	return 0;
}
if (Response) {
	memcpy(ResponseData, SSTS.DataCont.Data, 256); //данные команды ответа, если нужны
}
ReleaseMutex(hMutTSC);
return 1;
}

//*******************************************************************************************************
int TerminalSendCommandAndState(WORD CO, PERAState pEraSt, BOOL Response) {
	SendStruct SSTS;
	if (sockTRM == -1 || !IsSessionExist) { return 0; }

	WaitForSingleObject(hMutTSC, INFINITE);
	SSTS.DataCont.CO = CO;
	*(PERAState)(SSTS.DataCont.Data) = *pEraSt;
	if (!DataComm(&SSTS, &sockTRM, Response)) {
		ReleaseMutex(hMutTSC);
		return 0; 
	}
	if (Response) {
		memcpy(ResponseData, SSTS.DataCont.Data, 256); //данные команды ответа, если нужны
	}
	ReleaseMutex(hMutTSC);
	return 1;
}

//******************************int CreateFullConnect(char* ConnectTo)***********************************
int CreateFullConnect(char* ConnectTo)
{
SendStruct SSF;
SOCKET socktemp;
int rep=3; //три попытки соединения через 500 мс

if(!CreateTerminal(ConnectTo))
return 0;

strcpy(SSF.IPDest,ConnectTo);
SSF.DataCont.CO=CREATESND;
socktemp=-1;
 while(!DataComm(&SSF,&socktemp)){
	Sleep(500);
	rep--;
	if(!rep){
		TerminalSendCommand(CLOSEALL,"");
		if(sockTRM!=-1){closesocket(sockTRM);}
		return 0;
	}
 }
wsrez=setsockopt(socktemp,IPPROTO_TCP,TCP_NODELAY,(char*)&TcpNoDelay,4);
opt = 1;
wsrez = setsockopt(socktemp, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt, sizeof(opt));
alive.onoff = 1;
alive.keepalivetime = 5000;
alive.keepaliveinterval = 1000;
wsrez=WSAIoctl(socktemp, SIO_KEEPALIVE_VALS, &alive, sizeof(alive), NULL, 0, &dwSize, NULL, NULL);

sockSNDR=socktemp;
socktemp=-1;
strcpy(ActiveConnect,ConnectTo);
return 1;
}


//*************************int IntCloseAll()***************************************************************
int IntCloseAll()
{
if(!TerminalSendCommand(CLOSEALL,""))
SendCommand("",CLOSEALL,"");
CloseSession();
if(sockSNDS!=-1){closesocket(sockSNDS);}
sockSNDS=-1;
if(sockSNDR!=-1){closesocket(sockSNDR);}
sockSNDR=-1;
ActiveConnect[0]=0;
return 1;
}

//*****************************int FlagHandler(PDataContainer PDC)******************************************
int FlagHandler(PDataContainer PDC){
BYTE Key;
int rez;

HWINSTA hwst=OpenWindowStation("Winsta0",FALSE,
WINSTA_ACCESSCLIPBOARD	| 
WINSTA_ACCESSGLOBALATOMS| 
WINSTA_CREATEDESKTOP    | 
WINSTA_ENUMDESKTOPS     | 
WINSTA_ENUMERATE        | 
WINSTA_EXITWINDOWS      | 
WINSTA_READATTRIBUTES   | 
WINSTA_READSCREEN       | 
WINSTA_WRITEATTRIBUTES);


rez=SetProcessWindowStation(hwst);

HDESK hOD=OpenDesktop("Default",0,FALSE,
DESKTOP_CREATEMENU		|
DESKTOP_CREATEWINDOW	|
DESKTOP_ENUMERATE		|
DESKTOP_HOOKCONTROL		|
DESKTOP_JOURNALPLAYBACK	|
DESKTOP_JOURNALRECORD	|
DESKTOP_READOBJECTS		|
DESKTOP_SWITCHDESKTOP	|
DESKTOP_WRITEOBJECTS);

rez=SetThreadDesktop(hOD);

Key=PDC->Data[0];

keybd_event( VK_LWIN,0,0,0);
//keybd_event( VK_LMENU,0, 0,0);
keybd_event( Key,0, 0,0);
keybd_event( Key,0,KEYEVENTF_KEYUP,0);
//eybd_event( VK_LMENU,0,KEYEVENTF_KEYUP,0);
keybd_event( VK_LWIN,0,KEYEVENTF_KEYUP,0);

CloseDesktop(hOD);
CloseWindowStation(hwst);

return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int GetSrvState(PDataContainer PDC /*, ERAState SrvState*/) {

	if (*(WORD*)PDC->Data != 0xF903) {
		SSTerm.DataCont.CO = GETSTATE;
		if (!DataComm(&SSTerm, &socktemp, 1) || *(WORD*)PDC->Data != 0xF903) {
			return 0;
		}
	}
	SrvState = *(PERAState)PDC->Data;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LogoffHandler() {
HANDLE hEv = NULL;

	if (!strlen(LogoffEventName)) {
		return 0;
	}
	hEv = OpenEvent(EVENT_MODIFY_STATE, FALSE, LogoffEventName);
	if (hEv != NULL && hEv != INVALID_HANDLE_VALUE) {
		SetEvent(hEv);
		CloseHandle(hEv);
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int EraLoginHandler() {
HANDLE hEv;
HANDLE hEv2;
int ret = 0;

	hEv = OpenEvent(EVENT_MODIFY_STATE, FALSE, "Global\\Era_Cred_Prov");
	if (hEv != NULL && hEv != INVALID_HANDLE_VALUE) {
		SetEvent(hEv);	ret = 1;
		CloseHandle(hEv);
	}
	hEv2 = OpenEvent(EVENT_MODIFY_STATE, FALSE, "Global\\Era_Cred_Prov2");
	if (hEv2 != NULL && hEv2 != INVALID_HANDLE_VALUE) {
		SetEvent(hEv2);	ret = 1;
		CloseHandle(hEv2);
	}

	return ret;
}
