// CERATimer.cpp: файл реализации
//

#include "stdafx.h"
#include "ERAPANEL.h"
#include "CERATimer.h"
#include "afxdialogex.h"
#include "Importf.h"

extern PPanelData ppd_;

#define UWM_MSG (WM_APP + 100)

// Диалоговое окно CERATimer

IMPLEMENT_DYNAMIC(CERATimer, CDialogEx)

CERATimer::CERATimer(PPanelData pPD, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ERATIMER, pParent)
{
	ppd = pPD;
	pest = pPD->pEraSt;
	pest->MapChParams = (WORD)0;
	LastCopyFileName[0] = 0;
}

CERATimer::~CERATimer()
{
}

void CERATimer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHSRNT, m_chsrnt);
	DDX_Control(pDX, IDC_DTSRNT, m_dtsrnt);
	DDX_Control(pDX, IDC_CHSRLEN, m_chsrlen);
	DDX_Control(pDX, IDC_DTSRLEN, m_dtsrlen);
	DDX_Control(pDX, IDC_CHAUTOCOPY, m_chautocpy);
	DDX_Control(pDX, IDSET, m_butset);
	DDX_Control(pDX, IDABORT, m_butabort);
	DDX_Control(pDX, IDC_PROGRCOPY, m_progcopy);
	DDX_Control(pDX, IDC_STFNAME, m_copyfname);
	DDX_Control(pDX, IDC_FREADY, m_fready);
}


BEGIN_MESSAGE_MAP(CERATimer, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHSRNT, &CERATimer::OnBnClickedChsrnt)
	ON_BN_CLICKED(IDC_CHSRLEN, &CERATimer::OnBnClickedChsrlen)
	ON_BN_CLICKED(IDSET, &CERATimer::OnBnClickedSet)
	ON_WM_NCDESTROY()
	ON_MESSAGE(UWM_MSG, OnRecSignal)
	ON_BN_CLICKED(IDABORT, &CERATimer::OnBnClickedAbort)
END_MESSAGE_MAP()


// Обработчики сообщений CERATimer


BOOL CERATimer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	LogMsg("OnInitTimerPanel");

	ppd->IsEraTimePanelActive = true;

	if (!ppd->UseNitaTimer) {
		m_chsrnt.EnableWindow(FALSE);
		m_dtsrnt.EnableWindow(FALSE);
	}
	if (ppd->pEraSt->SelectRecMode >= RRVIDEOMODE) {
		m_chsrlen.EnableWindow(FALSE);
		m_dtsrlen.EnableWindow(FALSE);
	}
	m_butabort.EnableWindow(FALSE);
	IsShowAbortBut = false;

	m_chsrnt.SetCheck((int)pest->NitaTimerSelect);
	m_chsrlen.SetCheck((int)pest->LenTimerSelect);
	SYSTEMTIME ST;
	GetSystemTime(&ST);
	ST.wMilliseconds = 0;
	ST.wHour = pest->NitaTimer.wHour; ST.wMinute = pest->NitaTimer.wMinute; ST.wSecond = pest->NitaTimer.wSecond;
	m_dtsrnt.SetTime(&ST);
	ST.wHour = pest->LenTimer.wHour; ST.wMinute = pest->LenTimer.wMinute; ST.wSecond = pest->LenTimer.wSecond;
	m_dtsrlen.SetTime(&ST);
	m_chautocpy.SetCheck((int)pest->IsAutoCopy);

	//m_copyfname.SetWindowTextA("FName_init_Test");

	hPrCop = ::GetDlgItem(this->m_hWnd, IDC_PROGRCOPY);
	ppd->hWndPrCop = hPrCop;
	ppd->hWndEraTimer = this->m_hWnd;
	LRESULT lrez;
	lrez = ::SendMessageA(hPrCop, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 100));
	lrez = ::SendMessage(hPrCop, PBM_SETSTEP, (WPARAM)1, 0);

	ppd->ButEvent = BE_SHOWTIMERPANEL;
	::SetEvent(ppd->hEvent[0]);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // Исключение: страница свойств OCX должна возвращать значение FALSE
}


void CERATimer::OnBnClickedChsrnt()
{
	m_chsrlen.SetCheck(0);
}


void CERATimer::OnBnClickedChsrlen()
{
	m_chsrnt.SetCheck(0);
}


void CERATimer::OnBnClickedSet()
{
	int rez;
	ZeroMemory(&TPanelState, sizeof(TPanelState));
	TPanelState.NitaTimerSelect = (bool)m_chsrnt.GetCheck();
	TPanelState.LenTimerSelect = (bool)m_chsrlen.GetCheck();
	TPanelState.IsAutoCopy = (bool)m_chautocpy.GetCheck();
	m_dtsrnt.GetTime(&TPanelState.NitaTimer);
	m_dtsrlen.GetTime(&TPanelState.LenTimer);
	
	/*if (!m_edfname.GetLine(0, TPanelState.RecFileName, sizeof(TPanelState.RecFileName))) {
		TPanelState.RecFileName[0] = 0;
	}*/

	pest->MapChParams = (WORD)0;
	if (pest->NitaTimerSelect != TPanelState.NitaTimerSelect || pest->LenTimerSelect != TPanelState.LenTimerSelect) {
		pest->MapChParams |= CH_TYPETIMERSEL;
		pest->NitaTimerSelect = TPanelState.NitaTimerSelect; pest->LenTimerSelect = TPanelState.LenTimerSelect;
	}
	if (pest->IsAutoCopy != TPanelState.IsAutoCopy) {
		pest->MapChParams |= CH_AUTOCOPY;
		pest->IsAutoCopy = TPanelState.IsAutoCopy;
	}
	if (pest->NitaTimer.wHour != TPanelState.NitaTimer.wHour || pest->NitaTimer.wMinute != TPanelState.NitaTimer.wMinute || \
		pest->NitaTimer.wSecond != TPanelState.NitaTimer.wSecond) {
		pest->MapChParams |= CH_NITATIMERVALUE;
		pest->NitaTimer = TPanelState.NitaTimer;
	}
	if (pest->LenTimer.wHour != TPanelState.LenTimer.wHour || pest->LenTimer.wMinute != TPanelState.LenTimer.wMinute || \
		pest->LenTimer.wSecond != TPanelState.LenTimer.wSecond) {
		pest->MapChParams |= CH_LENTIMERVALUE;
		pest->LenTimer = TPanelState.LenTimer;
	}
	if (strcmp(pest->RecFileName, TPanelState.RecFileName) != 0) {
		pest->MapChParams |= CH_FILENAME;
		strcpy(pest->RecFileName, TPanelState.RecFileName);
	}

	//ppd->ButEvent = BE_CLOSETIMERPANEL;
	//::SetEvent(ppd->hEvent[0]);
	
	//
	CDialogEx::OnOK();
	OnClose();
}


void CERATimer::OnClose()
{
	//CDialogEx::PostNcDestroy();
	ppd->IsEraTimePanelActive = false;
	if (ppd->pEraSt->MapChParams) { //были внесены изменения в панели таймера записи
		if (ppd->hEvent[0] != NULL) {
			ppd->ButEvent = BE_CHANGERPARAMS;
			::SetEvent(ppd->hEvent[0]);
			Sleep(10);
		}
	}
	ppd->hWndPrCop = NULL;
	ppd->hWndEraTimer = NULL;
	ppd->ButEvent = BE_CLOSETIMERPANEL;
	::SetEvent(ppd->hEvent[0]);
//	ppd->tsc(CLOSETIMERPANEL, "", 0);
	CDialogEx::OnClose();
}


LRESULT CERATimer::OnRecSignal(WPARAM wpar, LPARAM lpar)
{
	if (lpar == 1 && ppd->NetCopyFileName[0] != (char)0) {
		m_copyfname.SetWindowTextA(ppd->NetCopyFileName);
		strcpy(LastCopyFileName, ppd->NetCopyFileName);
		m_fready.SetWindowTextA("ФАЙЛ СКОПИРОВАН");
		m_fready.ShowWindow(SW_SHOW);
		m_progcopy.ShowWindow(SW_HIDE);
		if (IsShowAbortBut) {
			m_butabort.EnableWindow(FALSE);
			IsShowAbortBut = false;
		}

	}
	else if (lpar == 2) {
		m_butabort.EnableWindow(FALSE);
	}
	else {
		if (ppd->NetCopyFileName[0] == (char)0 || !strcmp(LastCopyFileName, ppd->NetCopyFileName)) {
			return 0;
		}
		m_copyfname.SetWindowTextA(ppd->NetCopyFileName);
		strcpy(LastCopyFileName, ppd->NetCopyFileName);
		if (!IsShowAbortBut) {
			m_fready.ShowWindow(SW_HIDE);
			m_progcopy.ShowWindow(SW_SHOW);
			m_butabort.EnableWindow(TRUE);
			IsShowAbortBut = true;
		}
	}
	return 0;
}

void CERATimer::OnBnClickedAbort()
{
	if (ppd->hEvent[0] != NULL) {
		ppd->ButEvent = BE_ABORTNETCOPY;
		::SetEvent(ppd->hEvent[0]);
	}
}
