#ifndef __MYSTRUCTS__

#include <stdio.h>

typedef struct _NETRES {
	char Share[256];
	char MountPoint[64];
	char User[64];
	char Password[96];
}NETRES, *PNETRES;


#pragma pack(push, 1)
typedef struct _ERAState {
	WORD F903;
	WORD SelectRecMode;
	bool IsMonoMode;
	BYTE RecordState;
	bool UseNitaTimer;
	bool NitaTimerSelect;
	bool LenTimerSelect;
	bool IsAutoCopy;
	int PanCoordX;
	int PanCoordY;
	WORD MapChParams; //����� ���������� ���������� ��� ���� �������� (CO) = CHANGERECPARAMS
	SYSTEMTIME NitaTimer;
	SYSTEMTIME LenTimer;
	char RecFileName[90];
} ERAState, *PERAState;
#pragma pack(pop)


//typedef export functions(for panel dll)
typedef int (*TerminalSendCommand_)(WORD, char*, BOOL);
typedef int (*TerminalSendCommandAndState_)(WORD, PERAState, BOOL);
typedef int (*LogMsg_1)(char *);
typedef int (*LogMsg_2)(char *, int);
typedef int (*LogMsg_3)(char *, char *);
typedef int (*LogMsg_4)(char *, char *, char *);
typedef int (*LogMsg_5)(char *, char *, int);
typedef int (*LogMsg_6)(char *, int, char *);
typedef int (*LogMsg_7)(char *, int, char *, int);
typedef int (*LogMsg_8)(char *, char *, char *, int);
typedef int (*LogMsg_9)(char *, char *, int i, char *, int);
typedef int (*LogMsg_10)(char *, char *, char *, char *);
//typedef export functions(for panel dll)
typedef struct _EF { //Export Functions
	TerminalSendCommand_ TerminalSendCommand;
	TerminalSendCommandAndState_ TerminalSendCommandAndState;
	LogMsg_1 LogMsg1;
	LogMsg_2 LogMsg2;
	LogMsg_3 LogMsg3;
	LogMsg_4 LogMsg4;
	LogMsg_5 LogMsg5;
	LogMsg_6 LogMsg6;
	LogMsg_7 LogMsg7;
	LogMsg_8 LogMsg8;
	LogMsg_9 LogMsg9;
	LogMsg_10 LogMsg10;
} EF, *PEF;


typedef struct PCMHEAD {
	char	RIFF[4];
	DWORD	RiffSz;
	char	WAVEfmt[8];
	DWORD	FmtSz;
	WORD	wFormatTag;         /* format type */
	WORD	nChannels;          /* number of channels (i.e. mono, stereo...) */
	DWORD	nSamplesPerSec;     /* sample rate */
	DWORD	nAvgBytesPerSec;    /* for buffer estimation */
	WORD	nBlockAlign;        /* block size of data */
	WORD	wBitsPerSample;
	char	DATA[4];
	DWORD	DataSz;
}PPCMHEAD;

union W2B{
	WORD WRD;
	BYTE chr[2];
};

union CHFL{
	float FL;
	BYTE chr[4];;
};

union UFileTime{
	FILETIME FileTime;
	__int64 int64;
	char charFT[8];
};

typedef union _SIZE64 {
	struct {
		DWORD Low;
		DWORD High;
	} DW;
	UINT64 UI64;
} SIZE64;

typedef struct _OC{
char OC[64][16];
}OC, *pOC;


typedef struct _DataContainer{

	WORD F930;
	WORD CO;
	FILETIME FT;
	char Data[256];

} DataContainer, *PDataContainer;

typedef struct _U64D {
	union {
		struct {
			UINT64 U1;
			UINT64 U2;
			UINT64 U3;
			UINT64 U4;
		};
		struct {
			DWORD DW1;
			DWORD DW2;
			DWORD DW3;
			DWORD DW4;
			DWORD DW5;
			DWORD DW6;
			DWORD DW7;
			DWORD DW8;
		};
	};
	char Data[224];
} U64D, *PU64D;

typedef struct _SendStruct{
	char IPDest[64];
	DataContainer DataCont;

} SendStruct, *PSendStruct;


//���� ��������� ������ (Record State):
#define RS_STOP				(BYTE)0x00 //������ �����������
#define RS_LRECORD			(BYTE)0x01 //��������� ������ ��������
#define RS_RRECORD			(BYTE)0x02 //�������� ������ ��������
#define RS_LPAUSE			(BYTE)0x04 //������ ����� ��������� ������
#define RS_RPAUSE			(BYTE)0x08 //������ ����� �������� ������

//���� ����� ���������� ����������(MapChParams) ��� ���� �������� (CO) = CHANGERECPARAMS:
#define CH_TYPETIMERSEL		(WORD)0x0001
#define CH_NITATIMERVALUE	(WORD)0x0002
#define CH_LENTIMERVALUE	(WORD)0x0004
#define CH_FILENAME			(WORD)0x0008
#define CH_AUTOCOPY			(WORD)0x0010

#pragma pack(push, 1)
typedef struct _PanelData{
	int X;
	int Y;
	int XSize;
	int AddYSize;
	int AddXSize;
	int CoordX;
	int CoordY;
	int  IsPassedFirstConnect;
	int  VideoMode;
	WORD ButEvent;
	HWND HWRAMainWnd;
	HWND hWndSelf;
	HWND hWndEraTimer;//����� ���� EraTimer
	HWND hWndPrCop; //����� ���� ��������-���������� ����������� ����� �� ����
	WORD UWM_Msg; //���������, ���������� �� ����������, �� ��������� CPanel ������, ���������� ��������� CPanel
	BOOL IsAudioMode;
	BOOL IsAudioMode2;
	BOOL IsContinuousRec;
	BOOL IsVideoMode;
	BOOL IsPhotoMode;
	BOOL IsWaitMode;
	BOOL IsRecord; //������� ����, ��� ��� ��������������� ������
	BOOL IsPause; //������� ����, ��� ������ �����
	BOOL IsRecordProcedure; //������� ����, ��� ��� ������ ��� ���������� ����� ����� ������
	BOOL IsMono;
	BOOL RealMonoMode; //���������� �������� ��������� ����(1) ��� ������(0), ������ ��� ������������� mp3-�������
	BOOL IsSaveDialog;
	BOOL IsToSave;
	BOOL IsPcmFileReady;
	BOOL IsPcmFileLocked;
	BOOL IsReadPCMLock;
	BOOL IsAllowRemoteRecord;
	char ReqChangeRRecState; //������ �� ��������� ��������� �������� ������
	HANDLE hfmp3;
	HANDLE hfpcm;
	DWORD NOBWPCM_ALL;
	PERAState pSrvEraSt;
	PERAState pEraSt;
	char Name[96]; //����� ��������� ������
	char FilePath[128];
	char ConnectTo[96];
	char SCRBin[192];
	char SCRPartWndName[192];
	char FullPcmName[192];
	char FullVideoName[192];
	char ShortFileName[64];
	char NetCopyFileName[192];
	__int64 FileSize;
	HANDLE hEvent[4];//0-������� ������, 1-����� ������ 'Decode', 2-����� ����������� ��������, 3-������
	HANDLE hWPiped;
	HANDLE hProcFF;
	char *pSGPreset;
	int UseNitaTimer;
	char LTState; //��������� LenTimer
	bool IsEraTimePanelActive;
	EF ef;
} PanelData, *PPanelData;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _NITATIMER {
	char PlayState;
	UFileTime PlayTime;
} NITATIMER, *PNITATIMER;
#pragma pack(pop)

//Button Events(from panel to ERA-main module):
#define BE_NONE				(WORD)0
#define BE_EXIT				(WORD)1
#define BE_OFF				(WORD)2
#define BE_CHANGEMONOMODE	(WORD)3
#define BE_CHANGERSELMODE	(WORD)4
#define BE_RRECSTART		(WORD)5
#define BE_RRECSTOP			(WORD)6
#define BE_RRECPAUSE		(WORD)7
#define BE_LRECSTART		(WORD)8
#define BE_LRECSTOP			(WORD)9
#define BE_LRECPAUSE		(WORD)10
#define BE_CHANGERPARAMS	(WORD)11
#define BE_PRECHRPARAMS		(WORD)12
#define BE_CHANGERECSTATE	(WORD)13
#define BE_SHOWTIMERPANEL	(WORD)14
#define BE_CLOSETIMERPANEL	(WORD)15
#define BE_ABORTNETCOPY		(WORD)16

#define VIDEOSTOP	0
#define VIDEOPAUSE	1
#define VIDEORECORD	2

//Select Record Modes:
#define AUDIOMODE	0
#define VIDEOMODE	1
#define AUDIOMODE2	2
#define RRVIDEOMODE	3
#define RRAUDIOMODE	4

//���� ������� �� ��������� ��������� �������� ������(ReqChangeRRecState)
#define REQ_NONE		0	
#define REQ_RRECSTART	1
#define REQ_RRECSTOP	2
#define REQ_RRECPAUSE	4

//��������� ��� ������ �� ��������� �������(����� PostMessage):
#define PANMSG_STOP			0	//���� �� ������� � ������ ��������� ������
#define PANMSG_RRSTOP		1	//���� �������� ������
#define PANMSG_RRSTART		2	//����� �������� ������
#define PANMSG_RRPAUSE		3	//����� �������� ������
#define PANMSG_RRTERMINATE	4	//��������� ��������� ������

#define MP3_NAME	0
#define MP4_NAME	1
#define PCM_NAME	2

//SaveType:
#define AUDIO_TYPE  1
#define AUDIO2_TYPE 2
#define VIDEO_TYPE	4
//SaveType.

#define MAX_WAIT_VIDEOFILE 90000
#define MAX_WAIT_UNIONFILE 100000

#define MAX_MS_AM2 3000

typedef BOOL (WINAPI *CALLPANEL)(PPanelData);

//Code Operations(CO):
#define CREATESESSION (WORD)0
#define RUN (WORD)1
#define RUNVISIBLE (WORD)2
#define CLOSEAW (WORD)3
#define MESSBOX (WORD)4
#define CREATESND (WORD)5
#define CLOSEALL (WORD)6
#define CLOSEALLSAVEERST (WORD)7 //�������� ������ � ������� ��������� EraState
#define DISCONANOTHERUSER (WORD)8
#define FLAGKEY (WORD)9
#define LOGOFF	(WORD)10
#define CHANGEMODE (WORD)11 //Mono(Stereo)-Mode
#define CHANGESELRECMODE (WORD)12
#define CHANGERECPARAMS (WORD)13
#define GETSTATE (WORD)14
#define RRECSTART (WORD)15 //Start Remote Record(������� �� �������(������ ������) � �������)
#define RRECSTOP (WORD)16 //Stop Remote Record(������� �� �������(������ ������) � �������)
#define RRECPAUSE (WORD)17 //Pause Remote Record(������� �� �������(������ ������) � �������)
#define LRECSTART (WORD)18 //Start Local Record
#define LRECSTOP (WORD)19 //Stop Local Record
#define LRECPAUSE (WORD)20 //Pause Local Record
#define RRECTIMERSET (WORD)21 //Set Timer for Remote Record
#define NITATIMERSET (WORD)22 //Set NitaTimer for Local Record
#define RRNITATIMERSET (WORD)23 //Set NitaTimer for Remote Record
#define NITATIMERSIGNAL (WORD)24
#define NTSPLAYSTOP	(WORD)25
#define RRECSTATEACCEPT (WORD)26 //(�� ������� � �������)������������� ��������� �������� �������� ������� �� ��������� ��������� �������� ������
#define RRECSTOPSRV (WORD)27 //(�� ������� � �������)�������� ������� �� ��������� �������� ������
#define RRE�TERMINATESRV (WORD)28 //(�� ������� � �������)�������� ������� �� ��������� ��������� �������� ������
#define CLOSEACCEPT (WORD)29 //(�� ������� � �������)�������� ������������� �������� ������� �� ��������
#define SHOWTIMERPANEL (WORD)30 //(�� ������� � �������)����� ������ ������� ������������
#define CLOSETIMERPANEL (WORD)31 //(�� ������� � �������)����� ������ ������� �����������
#define CODETICK (WORD)32 //�������� ���(��� �������)
#define SHOWPERCNETCOPY (WORD)33 //(�� ������� � �������)���������� ������� ����������� ����� �� ����(��� �������� ���� � ��������)
#define ABORTNETCOPY (WORD)34 //(�� ������� � �������)�������� ����������� ������ ����� �� ����

#define ERALOGIN (WORD)0x070
#define CREATESESSIONERALOGIN (WORD)0x071

#define COREMOTE (WORD)0x100
#define CODEBUSY (WORD)0x1000
#define CODEOK (WORD)0x1001

//���� ��������� LenTimer (LTState):
#define LTS_START		(BYTE)1		//������ �������
#define LTS_PAUSE		(BYTE)2		//������ ������� � �� �����

//WakeUp LenTimer Reasons (WULTReason):
#define WULT_NONE		(WORD)0
#define WULT_STOP		(WORD)1
#define WULT_PAUSEON	(WORD)2
#define WULT_PAUSEOFF	(WORD)3
#define WULT_SETTIME	(WORD)4
#define WULT_GETTIME	(WORD)5

//Change bool-parameters:
#define CH_NONE			0
#define CH_TO_ENABLE	1
#define CH_TO_SEL		1
#define CH_TO_DISABLE	2
#define CH_TO_DESEL		2

//NetCopy States:
#define NC_COMPLETED	(DWORD)1
#define NC_CANCELED		(DWORD)2

//Macros "How Change parameter":
#define HOW_CH(P,Pprev) P && !Pprev ? CH_TO_ENABLE : \
(Pprev && !P ? CH_TO_DISABLE : CH_NONE)


#define __MYSTRUCTS__
#endif