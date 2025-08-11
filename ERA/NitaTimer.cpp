#include "Winsock2.h"
#include "ws2tcpip.h"
#include <Mstcpip.h>
#include <windows.h>
#include "structs.h"
#include "control.h"
#include "NitaTimer.h"
#include "MediaSrv.h"
#include "LogMsg.h"

extern PanelData PanDat;
extern ERAState EraState;
extern int NitaTimerPort;
extern char NitaTimerMcAddr[64];
extern char NitaTimerSenderIF[64];
UINT64 nsDay = 864000000000;//100ns-интервалов в сутках

int sock;
int flag_on = 1;              /* socket option flag */
WSADATA wsaData;
struct sockaddr_in mc_addr;   /* socket address structure */
struct sockaddr_in saddr;
struct ip_mreq mc_req;        /* multicast request structure */
char* mc_addr_str;            /* multicast IP address */
unsigned short mc_port;       /* multicast port */
struct sockaddr_in from_addr; /* packet source */
char recv_str[1200];     /* buffer to receive string */
static fd_set rdfd;
int recv_len;                 /* length of string received */
int from_len;        /* source addr length */
int wsaerr;
static BOOL IsFirstIter = TRUE;
int PlayState = PS_STOP;
int PrevPlayState = PS_STOP;
std::atomic<int> NitaTimerStateA;
int NitaTimerSigType = NTST_NOSET;
UFileTime PlayTime, PlayTimePrev, TimerTime;
NITATIMER NiTimer;
SYSTEMTIME TT;
HANDLE *phEvT = NULL;
std::atomic<UFileTime> PlayTimeA;
std::atomic<int>PlayStateA;
extern BOOL IsSessionExist;


unsigned long _stdcall ThNitaTimer(void*) {
	
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		goto THNT_EX;
	}
NITATIMER_SOCK:
	if(!IsFirstIter){
		PanDat.UseNitaTimer = 0;
		EraState.UseNitaTimer = FALSE;
		if (sock != 0) {
			closesocket(sock);
		}
		Sleep(1000);
	}
	IsFirstIter = FALSE;
	NitaTimerStateA.store(NTS_NOSET);
	
	sock = 0; /* create socket to join multicast group on */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		goto THNT_EX;
	}
	/* set reuse port to on to allow multiple binds per host */
	if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&flag_on, sizeof(flag_on))) < 0) {
		goto THNT_EX;
	}
	/* construct a multicast address structure */
	memset(&mc_addr, 0, sizeof(mc_addr));
	mc_addr.sin_family = AF_INET;
	mc_addr.sin_addr.s_addr = inet_addr(NitaTimerSenderIF); //htonl(INADDR_ANY);
	mc_port = (unsigned short)(NitaTimerPort);
	mc_addr.sin_port = htons(mc_port);

	/* bind to multicast address to socket */
	if ((bind(sock, (struct sockaddr *) &mc_addr, sizeof(mc_addr))) < 0) {
		goto NITATIMER_SOCK;
	}
	//присоединяем сокет к multicast-группе
	mc_req.imr_multiaddr.s_addr = inet_addr(NitaTimerMcAddr);
	mc_req.imr_interface.s_addr = inet_addr(NitaTimerSenderIF);
	if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mc_req, sizeof(mc_req)) < 0) {
		goto NITATIMER_SOCK;
	}
	PanDat.UseNitaTimer = 1;
	EraState.UseNitaTimer = TRUE;
	memset(recv_str, 0, sizeof(recv_str));
	from_len = sizeof(from_addr);
	memset(&from_addr, 0, from_len);

	PlayState = PS_STOP;
	PlayTime.int64 = PlayTimePrev.int64 = 0;
	while (1) {
		if ((recv_len = recvfrom(sock, recv_str, sizeof(recv_str), 0, (struct sockaddr*)&from_addr, &from_len)) < 0) {
			goto NITATIMER_SOCK;
		}

		if (recv_len >= sizeof(NITATIMER)) {
			NiTimer = *(PNITATIMER)recv_str;
			PlayState = NiTimer.PlayState;
			if (PlayState > PS_MOVE) {
				continue;
			}
			if (PlayTime.int64 != 0) {
				PlayTimePrev = PlayTime;
			}
			PlayTime = NiTimer.PlayTime;
			PlayTimeA.store(PlayTime);
			PlayStateA.store(PlayState);
		}
		//U
		if (PrevPlayState == PS_PLAY && PlayState != PS_PLAY) {
			TerminalSendCommand(NTSPLAYSTOP, "", 0);
		}
		//U
		switch (NitaTimerStateA.load()) {
		
		case NTS_ACTIVE:
			if (PlayState == PS_PLAY && PlayTime.int64 >= TimerTime.int64) {
				LogMsg("TimerSignal!");
				if (EraState.RecordState && (!(EraState.RecordState&(RS_LPAUSE | RS_RPAUSE)))) { //любая запись и не на паузе
					if (NitaTimerSigType == NTST_EVENT && phEvT != NULL) {
						SetEvent(*phEvT);
					}
					else if (NitaTimerSigType == NTST_MSG) {
						LogMsg("TerminalSendCommand_NTST_MSG");
						TerminalSendCommand(NITATIMERSIGNAL, "", 0);
					}
					else if (NitaTimerSigType == NTST_MSSTOP) {
						RRecStopF();
						if (IsSessionExist) {
							LogMsg("TerminalSendCommand_IsSessionExist");
							TerminalSendCommand(NITATIMERSIGNAL, "", 0);
						}
						else {
							LogMsg("NitaTimer_RS_STOP");
							EraState.RecordState = RS_STOP;
						}
					}
					NitaTimerSigType = NTST_NOSET;
					NitaTimerStateA.store(NTS_NOSET);
				}
			}
			else if (PlayState == PS_PAUSE || PlayState == PS_STOP || PlayTimePrev.int64 > PlayTime.int64) {
				NitaTimerStateA.store(NTS_PRESET);
			}
			break;
		
		case NTS_PRESET:
			if (PlayState == PS_PLAY) {
				NitaReactivateTimer();
			}
			break;
		
		case NTS_NOSET:
		default:
			break;
		}
		PrevPlayState = PlayState;
		//Sleep(10);
	}
THNT_EX:
	PanDat.UseNitaTimer = 0;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int NitaTimerActivate(HANDLE *phEv, BOOL MSStop) {

	if (!PanDat.UseNitaTimer || NitaTimerStateA.load() == NTS_NOSET) {
		return 0;
	}
	if (phEv != NULL) {
		phEvT = phEv;
		NitaTimerSigType = NTST_EVENT; //тип сигнализации таймера через объект-событие
	}
	else if (MSStop) {
		phEvT = NULL;
		NitaTimerSigType = NTST_MSSTOP; //тип сигнализации таймера через внутреннюю стоп-функцию в MediaSrv.cpp
	}
	else { // phEv == NULL
		phEvT = NULL;
		NitaTimerSigType = NTST_MSG; //тип сигнализации таймера через отправку сообщения (TerminalSendCommand)
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NitaTimerDeactivate() {
	
	NitaTimerSigType = NTST_NOSET;
	NitaTimerStateA.store(NTS_NOSET);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int NitaTimerPreSet(SYSTEMTIME *pTT) {

	if (!PanDat.UseNitaTimer) { return 0; }
	phEvT = NULL;
	TT = *pTT;
	NitaTimerStateA.store(NTS_PRESET);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void NitaReactivateTimer(/*UFileTime PlayTime, UFileTime TimerTime, SYSTEMTIME TT*/) {
	UFileTime UFTPlay, UFTTTc;
	SYSTEMTIME TTc;
	
	if (NitaTimerStateA.load() != NTS_PRESET) {
		return;
	}
	FileTimeToSystemTime(&PlayTime.FileTime, &TTc);
	TTc.wHour = TT.wHour; TTc.wMinute = TT.wMinute; TTc.wSecond = TT.wSecond; TTc.wMilliseconds = 0;
	SystemTimeToFileTime(&TTc, &UFTTTc.FileTime);
	if (PlayTime.int64 > UFTTTc.int64){ //сутки точки воспроизведения и сработки таймера - не совпадают
		//добавляем сутки к времени таймера
		UFTTTc.int64 += nsDay;
	}
	TimerTime.int64 = UFTTTc.int64;
	NitaTimerStateA.store(NTS_ACTIVE);
}