#pragma once

#include <atomic>

//ѕереопределение констант из winsock.h т.к. дл€ Win7(8) они определены неправильно
#define IP_MULTICAST_TTL	10
#define IP_MULTICAST_LOOP	11
#define IP_ADD_MEMBERSHIP	12
#define IP_DROP_MEMBERSHIP	13


//Nita Timer Use:
#define NOT_USE_T				0
#define USE_T_PARAMS_FROM_INI	1
#define USE_T_PARAMS_FROM_XML	2

//Play States:
#define PS_PLAY				0
#define PS_PAUSE			1
#define PS_STOP				2
#define PS_MOVE				3

//NitaTimerStates:
#define NTS_NOSET			0
#define NTS_PRESET			1
#define NTS_ACTIVE			2

//NitaTimer Signal Types(NitaTimeSigType):
#define NTST_NOSET			0
#define NTST_EVENT			1
#define NTST_MSG			2
#define NTST_MSSTOP			3 //Media SRV Stop


//основные три атомарных переменных “аймера дл€ использовани€ извне
extern std::atomic<UFileTime> PlayTimeA;
extern std::atomic<int>PlayStateA;
extern std::atomic<int> NitaTimerStateA;

unsigned long _stdcall ThNitaTimer(void*);
int NitaTimerActivate(HANDLE *phEv = NULL, BOOL MSStop = FALSE);
void NitaTimerDeactivate();
int NitaTimerPreSet(SYSTEMTIME *pTT);
void NitaReactivateTimer();
