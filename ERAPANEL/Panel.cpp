// Panel.cpp : implementation file
//

#include "stdafx.h"
#include "ERAPANEL.h"
#include "Panel.h"
#include "Photo.h"
#include "MediaFile.h"
#include "Importf.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PPanelData ppd_;
HCURSOR hCurGrn;
HCURSOR hCurPrev;
UINT ThHelp(void*);
void DisplayTimer(CPanel *pth);
int VideoRecord(PPanelData ppd);
int MsgBox2(char *Txt, char *Caption);
unsigned long _stdcall ThMsgBox2(void*);
extern HANDLE hEvVideoReady;
extern HANDLE hEvUnionSaveReady;
char DenyRRTxt[] = "Режим удалённой записи в контексте данного подключения недоступен.";
char DenyRRCpt[] = "Режим удалённой записи недоступен";
char TerminateRRTxt[] = "Аварийное прерывание удалённой записи!";
char TerminateRRCpt[] = "Аварийное прерывание удалённой записи";
//DisplayTimer params:
BOOL PrevIsRecord = FALSE;
BOOL PrevIsPause = FALSE;
BOOL IsDTActive = FALSE;
UFileTime T0, TStart, TCur;
//U
int iTmp;
//U


#define UWM_MSG (WM_APP + 100)

typedef struct _MsgCont {
	char *Txt;
	char *Caption;
}MsgCont, *pMsgCont;

MsgCont MessageCont;

BEGIN_MESSAGE_MAP(CPanel, CDialog)
	//{{AFX_MSG_MAP(CPanel)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_NCACTIVATE()
	ON_WM_SETCURSOR()
	ON_WM_ACTIVATEAPP()
	ON_BN_CLICKED(IDRECORD, OnRECORD)
	ON_NOTIFY(NM_RCLICK, IDRECORD, OnRRECORD)
	ON_BN_CLICKED(IDPAUSE, OnPAUSE)
	ON_BN_CLICKED(IDSTOP, OnSTOP)
	ON_NOTIFY(NM_RCLICK, IDMODE, OnRMODE)
	ON_BN_CLICKED(IDMODE, OnMODE)
	ON_BN_CLICKED(IDPHOTO, OnPHOTO)
	ON_BN_CLICKED(IDMONO, OnMONO)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDOFF, OnOFF)
	ON_BN_CLICKED(IDEXIT, OnEXIT)
	ON_MESSAGE(UWM_MSG, OnRecSignal)
	//}}AFX_MSG_MAP
	ON_WM_MOVE()
END_MESSAGE_MAP()



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPanel dialog

CPanel::CPanel(CWnd* pParent /*=NULL*/)
	: CDialog(CPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPanel)
	//}}AFX_DATA_INIT
IsHiddenPanel = 1;
ToSetHide = 0;
SelectMode = 0;
GTCLastClickMode = 0;
XYOffsetDef = FALSE;
ExtCall = FALSE;
AfxBeginThread(ThHelp,this); //запуск вспомогательного потока
m_hIcon1 = AfxGetApp()->LoadIcon(IDR_ICON1);
m_hIcon2 = AfxGetApp()->LoadIcon(IDR_ICON2);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPanel::OnInitDialog()
{
	//ModifyStyleEx(0,WS_EX_TOOLWINDOW);
	LogMsg("OnInitPanel");
	CDialog::OnInitDialog();
	iWidth = GetSystemMetrics(SM_CXSCREEN);
	iHeight = GetSystemMetrics(SM_CYSCREEN);
	//int yBord = GetSystemMetrics(SM_CYEDGE);
	//int yCapt = GetSystemMetrics(SM_CYCAPTION);
	GetClientRect(&ClRect);
	GetWindowRect(&WRect);
	AugX = (WRect.right - WRect.left) - ClRect.right;
	AugY = (WRect.bottom - WRect.top) - ClRect.bottom;

	

	W_H = ppd->AddYSize + AugY;
	AXS = ppd->AddXSize;
	XPos=iWidth-ppd->X;
	YPos=0;
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon1, TRUE);			// Set big icon
	//SetIcon(m_hIcon1, FALSE);		// Set small icon
	m_btnRecord.Create("",WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW,CRect(0,0,30,24),this,IDRECORD);
	m_btnStop.Create("",WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW,CRect(31,0,61,24),this,IDSTOP);
	m_btnPause.Create("",WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW,CRect(62,0,92,24),this,IDPAUSE);
	m_btnMode.Create("",WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW,CRect(93,0,123,24),this,IDMODE);
	m_btnPhoto.Create("", WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW, CRect(124,0,154,24),this,IDPHOTO);
	m_btnMono.Create("",WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW,CRect(155,0,185,24),this,IDMONO);
	m_btnOff.Create("",WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW,CRect(186,0,216,24),this,IDOFF);
	m_btnExit.Create("",WS_VISIBLE|WS_CHILD|BS_BITMAP|BS_PUSHBUTTON|BS_OWNERDRAW,CRect(217,0,247,24),this,IDEXIT);
	if(ppd->IsRecord){
		m_btnRecord.LoadBitmaps(IDB_REC2,IDB_REC3,0,0);
		if(ppd->IsPause)
			m_btnPause.LoadBitmaps(IDB_PAUSE4,IDB_PAUSE5,0,0);
		else//!ppd->IsPause
			m_btnPause.LoadBitmaps(IDB_PAUSE1,IDB_PAUSE2,0,0);
		m_btnStop.LoadBitmaps(IDB_STOP1,IDB_STOP2,0,0);
	IsHiddenPanel=0;
	}
	else{//!ppd->IsRecord
		m_btnRecord.LoadBitmaps(IDB_REC0,IDB_REC1,0,0);
		m_btnStop.LoadBitmaps(IDB_STOP0,0,0,0);
		m_btnPause.LoadBitmaps(IDB_PAUSE0, 0, 0, 0);
		m_btnPhoto.LoadBitmaps(IDB_PHOTO1, IDB_PHOTO2, 0, 0);
	}

	if(ppd->IsVideoMode)
		m_btnMode.LoadBitmaps(IDB_MODE2,IDB_MODE3,0,0);
	else//!ppd->IsVideoMode
    m_btnMode.LoadBitmaps(IDB_MODE0,IDB_MODE1,0,0);
	
	if(ppd->IsMono)
		m_btnMono.LoadBitmaps(IDB_MONO3,IDB_MONO4,0,0);
	else//!ppd->IsMono
    m_btnMono.LoadBitmaps(IDB_MONO1,IDB_MONO2,0,0);
	
    m_btnOff.LoadBitmaps(IDB_OFF1,IDB_OFF2,0,0);
	m_btnExit.LoadBitmaps(IDB_EXIT1,IDB_EXIT2,0,0);
	
	// TODO: Add extra initialization here
	ppd->hWndSelf = this->m_hWnd; //hWnd своей формы для передачи в фото-режиме
	ppd->UWM_Msg = UWM_MSG;
	ppd->hProcFF = NULL;
	::SetWindowText(this->m_hWnd, ppd->Name); //заголовок окна в соответствии с профилем
	ChangeVisibleBySrvState();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnPaint() 
{
	SetCurrentVisible();
CDialog::OnPaint();
	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPanel)
	DDX_Control(pDX, IDC_EDIT, m_edit);
	//}}AFX_DATA_MAP
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPanel message handlers

void CPanel::OnClose() 
{
SetHidden(0);
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPanel::OnNcActivate(BOOL bActive) 
{
Sleep(3);
this->BringWindowToTop();

if(IsHiddenPanel){	return 0;}
//SetHidden(1);
//ToSetHide=1;

//return 1;
return CDialog::OnNcActivate(bActive) ;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnActivateApp(BOOL bActive, DWORD dwThreadID){
	if (!bActive) {
		ToSetHide = 1;
	}
	else {
		ToSetHide = 0;
	}

	return CDialog::OnActivateApp(bActive, dwThreadID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPanel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	
	if(message==WM_RBUTTONDOWN ){
	SetVisible();
	}
	
	if(IsHiddenPanel){
	SetCursor(hCurGrn);
	return 1;
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::SetHidden(int smoothly)
{
	int i;
if (ppd->IsRecord || ppd->IsSaveDialog || ppd->IsPhotoMode) {
	return;
}
if(smoothly){
	for(i=0;i<W_H;i++){
	::SetWindowPos(this->m_hWnd,HWND_TOPMOST,XPos,YPos-i,ppd->XSize+AXS,W_H,SWP_SHOWWINDOW);
	Sleep(8);
	}
}
else{//faster
::SetWindowPos(this->m_hWnd,HWND_TOPMOST,XPos,YPos,ppd->XSize+AXS,2,SWP_SHOWWINDOW);
}

SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED); 
::SetLayeredWindowAttributes(this->m_hWnd, 0, 1, LWA_ALPHA);
::SetWindowPos(this->m_hWnd,HWND_TOPMOST,XPos,YPos,ppd->XSize+AXS,2,SWP_SHOWWINDOW);
IsHiddenPanel=1;

RATop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::SetVisible(bool UseCoord, int X, int Y)
{
	if (UseCoord) {
		::SetWindowPos(this->m_hWnd, HWND_TOPMOST, X, Y, 251 + AXS, W_H, SWP_SHOWWINDOW);
	}
	else {
		::SetWindowPos(this->m_hWnd, HWND_TOPMOST, XPos, YPos, 251 + AXS, W_H, SWP_SHOWWINDOW | SWP_NOMOVE);
	}
SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED); 
::SetLayeredWindowAttributes(this->m_hWnd, 0, 255, LWA_ALPHA);
IsHiddenPanel=0;
ppd->IsSaveDialog=0;

//RATop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::SetRecordVisible()
{
	if (!ppd->IsVideoMode) {
		::SetWindowPos(this->m_hWnd, HWND_TOPMOST, XPos, YPos, 129+AXS, W_H, SWP_SHOWWINDOW | SWP_NOMOVE);
		SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(this->m_hWnd, 0, 255, LWA_ALPHA);
	}
	else {//IsVideoMode
		::SetWindowPos(this->m_hWnd, HWND_TOPMOST, XPos, YPos, 129+AXS, W_H, SWP_SHOWWINDOW | SWP_NOMOVE);
		SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(this->m_hWnd, 0, 80, LWA_ALPHA);
	}


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::SetPhotoVisible()
{
	if (ppd->IsPhotoMode) {
		::SetWindowPos(this->m_hWnd, HWND_TOPMOST, XPos, YPos, 67+AXS, W_H, SWP_SHOWWINDOW | SWP_NOMOVE);
		SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(this->m_hWnd, 0, 255, LWA_ALPHA);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::SetWaitVisible()
{
	if (ppd->IsWaitMode) {
		::SetWindowPos(this->m_hWnd, HWND_TOPMOST, XPos, YPos, 34+AXS, W_H, SWP_SHOWWINDOW | SWP_NOMOVE);
		SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		::SetLayeredWindowAttributes(this->m_hWnd, 0, 255, LWA_ALPHA);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::SetSaveVisible()
{
if(!ppd->FileSize && !ppd->IsVideoMode && !ppd->IsAudioMode2){//ничего не записано в audiomode - значит отмена диалога сохранки
OnCancel();
}
//::SetWindowPos(this->m_hWnd,HWND_TOPMOST,XPos,YPos,ppd->XSize+AXS,W_H+55,SWP_SHOWWINDOW|SWP_NOMOVE);
::SetWindowPos(this->m_hWnd, HWND_TOPMOST, XPos, YPos, ppd->XSize + AXS, ClRect.bottom + AugY, SWP_SHOWWINDOW | SWP_NOMOVE);
SetWindowLong(this->m_hWnd, GWL_EXSTYLE, GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED); 
::SetLayeredWindowAttributes(this->m_hWnd, 0, 255, LWA_ALPHA);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::SetCurrentVisible()
{
if (ppd->IsPhotoMode) {
	SetPhotoVisible();
	return;
}
if(ppd->IsRecord){
	SetRecordVisible();
	return;
}
if(ppd->IsSaveDialog){
	SetSaveVisible();
	return;
}

if (IsHiddenPanel) {
	if (ppd->pEraSt->RecordState&RS_RRECORD) {
		SetVisible(true, ppd->CoordX, ppd->CoordY);
		OnRRecordStart();
	}
	else {
		SetHidden();
	}
}
else
SetVisible();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnRECORD()
{

	if (ppd->IsPhotoMode) {
		//Under construction
		PhotoProc(ppd);
		//Sleep(2);
		//::SetWindowPos(ppd->HWRAMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE);
		::SetForegroundWindow(ppd->HWRAMainWnd);
		return;
	}

	if (ppd->pEraSt->SelectRecMode >= RRVIDEOMODE) {
		if (ppd->IsRecord) {
			ppd->ReqChangeRRecState = REQ_RRECSTOP;
		}
		else { //!ppd->IsRecord
			ppd->ReqChangeRRecState = REQ_RRECSTART;
		}
	}
	else { //ppd->pEraSt->SelectRecMode < RRVIDEOMODE
		if (ppd->IsRecord) {
			OnSTOP();
			return;
			/*
			NoRecord:
				m_btnRecord.LoadBitmaps(IDB_REC0,IDB_REC1,0,0);
				m_btnPause.LoadBitmaps(IDB_PAUSE0,0,0,0);
				m_btnStop.LoadBitmaps(IDB_STOP0,0,0,0);
				if(!ppd->IsRecord){return;}//Выход по невозможности создать mp3-файл
					if(ppd->IsVideoMode){
						ppd->VideoMode=VIDEOSTOP;
						VideoRecord(ppd);
					}
				ppd->IsSaveDialog=1;
				ppd->IsRecord=0;
				ppd->IsPause=0;
				SetSaveVisible();
				*/
		}
		else { //if !ppd->IsRecord
			if (!StartRecordProc()) {
				return;
			}
			m_btnRecord.LoadBitmaps(IDB_REC2, IDB_REC3, 0, 0);
			m_btnPause.LoadBitmaps(IDB_PAUSE1, IDB_PAUSE2, 0, 0);
			m_btnStop.LoadBitmaps(IDB_STOP1, IDB_STOP2, 0, 0);
			ppd->IsRecord = 1;
			ppd->IsRecordProcedure = 1;
			ppd->IsPause = 0;
			ppd->IsSaveDialog = 0;
		}
		m_btnRecord.Invalidate();
		m_btnPause.Invalidate();
		m_btnStop.Invalidate();
		m_btnRecord.UpdateWindow();
		m_btnPause.UpdateWindow();
		m_btnStop.UpdateWindow();

		SetCurrentVisible();
	}
	ppd->ButEvent = BE_CHANGERECSTATE;
	::SetEvent(ppd->hEvent[0]); //сигнал к отправке смены состояния записи на удалённую сторону
}

////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnRRecordStart() //Запуск по сообщению от подтверждения серверной стороной смены состояния удалённой записи
{
	LogMsg("CPanel::OnRRecordStart()");
	m_btnRecord.LoadBitmaps(IDB_REC2, IDB_REC3, 0, 0);
	m_btnPause.LoadBitmaps(IDB_PAUSE1, IDB_PAUSE2, 0, 0);
	m_btnStop.LoadBitmaps(IDB_STOP1, IDB_STOP2, 0, 0);
	ppd->IsRecord = 1;
	ppd->IsPause = 0;
	ppd->IsSaveDialog = 0;
	m_btnRecord.Invalidate();
	m_btnPause.Invalidate();
	m_btnStop.Invalidate();
	m_btnRecord.UpdateWindow();
	m_btnPause.UpdateWindow();
	m_btnStop.UpdateWindow();
	SetCurrentVisible();

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnPAUSE()
{
	if (!ppd->IsRecord) { return; }
	if (ppd->pEraSt->SelectRecMode == VIDEOMODE || \
		ppd->pEraSt->SelectRecMode == RRVIDEOMODE) {
		return;
	}
	if (ppd->pEraSt->SelectRecMode >= RRVIDEOMODE) {
		ppd->ReqChangeRRecState = REQ_RRECPAUSE;
	}
	else { //if (ppd->pEraSt->SelectRecMode < RRVIDEOMODE)
		if (ppd->IsPause) {
			m_btnPause.LoadBitmaps(IDB_PAUSE1, IDB_PAUSE2, 0, 0);
			ppd->IsPause = 0;
		}
		else {//!IsPause
			m_btnPause.LoadBitmaps(IDB_PAUSE4, IDB_PAUSE5, 0, 0);
			ppd->IsPause = 1;
		}
		PauseRecordProc();
		m_btnPause.Invalidate();
		m_btnPause.UpdateWindow();
		SetCurrentVisible();
	}
	ppd->ButEvent = BE_CHANGERECSTATE;
	::SetEvent(ppd->hEvent[0]); //сигнал к отправке смены состояния записи на удалённую сторону
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnSTOP()
{
	if (!ppd->IsRecord && !ppd->IsPhotoMode) {
		LogMsg("OnSTOP_return_in_begin",__LINE__);
		return;
	}
	if (ppd->IsPhotoMode) {
		ppd->IsPhotoMode = 0;
		m_btnRecord.LoadBitmaps(IDB_REC0, IDB_REC1, 0, 0);
		m_btnPause.LoadBitmaps(IDB_PAUSE0, 0, 0, 0);
		m_btnStop.LoadBitmaps(IDB_STOP0, 0, 0, 0);
		goto MBTN_OS;
	}
	//if(ppd->IsRecord):
	if (ppd->pEraSt->SelectRecMode >= RRVIDEOMODE) {
		ppd->ReqChangeRRecState = REQ_RRECSTOP;
		LogMsg("OnSTOP", __LINE__); 
	}
	else { //ppd->pEraSt->SelectRecMode < RRVIDEOMODE
		LogMsg("OnSTOP", __LINE__); 
		m_btnRecord.LoadBitmaps(IDB_REC0, IDB_REC1, 0, 0);
		m_btnPause.LoadBitmaps(IDB_PAUSE0, 0, 0, 0);
		m_btnStop.LoadBitmaps(IDB_STOP0, 0, 0, 0);
		if (ppd->IsPhotoMode) {
			ppd->IsPhotoMode = 0;
		}
		else {//!ppd->IsPhotoMode
			if (ppd->IsVideoMode) {
				if (!StopVideoRecord()) {
					return;
				}
			}
			ppd->IsSaveDialog = 1;
			ppd->IsRecord = 0;
			ppd->IsPause = 0;
			if (ppd->IsAudioMode2) {
				StopAudio2Record();
			}
		}
MBTN_OS:
		m_btnRecord.Invalidate();
		m_btnPause.Invalidate();
		m_btnStop.Invalidate();
		m_btnRecord.UpdateWindow();
		m_btnPause.UpdateWindow();
		m_btnStop.UpdateWindow();
		SetSaveVisible();

	}
	ppd->ButEvent = BE_CHANGERECSTATE;
	::SetEvent(ppd->hEvent[0]); //сигнал к отправке смены состояния записи на удалённую сторону
}

////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnRRecordStop(BOOL ShowSaveVisible) //Запуск по сообщению от подтверждения серверной стороной смены состояния удалённой записи
{
	m_btnRecord.LoadBitmaps(IDB_REC0, IDB_REC1, 0, 0);
	m_btnPause.LoadBitmaps(IDB_PAUSE0, 0, 0, 0);
	m_btnStop.LoadBitmaps(IDB_STOP0, 0, 0, 0);
	ppd->IsSaveDialog = 1;
	ppd->IsRecord = 0;
	ppd->IsPause = 0;
	
	m_btnRecord.Invalidate();
	m_btnPause.Invalidate();
	m_btnStop.Invalidate();
	m_btnRecord.UpdateWindow();
	m_btnPause.UpdateWindow();
	m_btnStop.UpdateWindow();
	if (ShowSaveVisible) {
		SetSaveVisible();
	}
	else {
		SetVisible();
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnMODE() 
{
	DWORD GTCCur;
	DWORD DiffGTC;
	BOOL IsContinuousRec;
	char buff[4] = { '0','0','0','0' };
	DWORD NOBW;
		
	if (ppd->IsRecord) { return; }
	IsContinuousRec = ppd->IsContinuousRec;
	if (ExtCall) {
		goto SW_ONMODE;
	}
	GTCCur = GetTickCount();
	DiffGTC = GTCCur- GTCLastClickMode;
	GTCLastClickMode = GTCCur;

	SelectMode++;
	if (SelectMode > 1 && DiffGTC > MAX_MS_AM2) {
		SelectMode = 0;
	}
	if (SelectMode >= 3) {
		SelectMode = 0;
	}
	ppd->pEraSt->SelectRecMode = SelectMode;

SW_ONMODE:
	switch (SelectMode) {
	
	case AUDIOMODE:
		ppd->IsAudioMode = 1;
		ppd->IsVideoMode = 0;
		ppd->IsAudioMode2 = 0;
		ppd->IsContinuousRec = 0;
		m_btnMode.LoadBitmaps(IDB_MODE0, IDB_MODE1, 0, 0);
		SetIcon(m_hIcon1, TRUE);
		SetIcon(m_hIcon1, FALSE);
		break;
	case VIDEOMODE:
		ppd->IsAudioMode = 0;
		ppd->IsVideoMode = 1;
		ppd->IsAudioMode2 = 1;
		ppd->IsContinuousRec = 1;
		m_btnMode.LoadBitmaps(IDB_MODE2, IDB_MODE3, 0, 0);
		SetIcon(m_hIcon2, TRUE);
		SetIcon(m_hIcon2, FALSE);
		break;
	case AUDIOMODE2:
		ppd->IsAudioMode = 0;
		ppd->IsVideoMode = 0;
		ppd->IsAudioMode2 = 1;
		ppd->IsContinuousRec = 1;
		m_btnMode.LoadBitmaps(IDB_MODE4, IDB_MODE5, 0, 0);
		SetIcon(m_hIcon1, TRUE);
		SetIcon(m_hIcon1, FALSE);
		break;
	}

m_btnMode.Invalidate();
m_btnMode.UpdateWindow();
if (!IsContinuousRec && ppd->IsContinuousRec) {//произошло включение режима ContinuousRec
	if (ppd->IsReadPCMLock) {//если заблокирован ReadFile в GetPcm(play.cpp)
		WriteFile(ppd->hWPiped, buff, 1, &NOBW, NULL); //Делаем разблокировку ReadFile в GetPcm(play.cpp)
	}
}
if (ppd->hEvent[0] != NULL) {
	ppd->ButEvent = BE_CHANGERSELMODE;
	ppd->pEraSt->SelectRecMode = SelectMode;
	if (!ExtCall) { //при автоматическом вызове функции, новое состояние не отправляем удалённой стороне, т.к. ею и продиктовано в данном случае
		::SetEvent(ppd->hEvent[0]); //сигнал к отправке смены режима записи на удалённую сторону
	}
}
ExtCall = FALSE;
}

///////////////////////////////////////////////////////////////////////
void CPanel::OnRMODE(NMHDR * pNotifyStruct, LRESULT * result) {

	if (!ExtCall && ppd->pEraSt->RecordState&RS_RRECORD) { //отключение в режиме удалённой записи
		LogMsg("Отключение в режиме удалённой записи по RClick RecorMode");
		ppd->ButEvent = BE_EXIT;
		if (ppd->hEvent[0] != NULL)
			::SetEvent(ppd->hEvent[0]);
		SetHidden(0);
		return; 
	}
	else if (ppd->IsRecord) { //если локальная запись - ничего не делаем
		return;
	}

	if(!ExtCall && (SelectMode == RRVIDEOMODE || SelectMode == RRAUDIOMODE)) { return; }
	if (ExtCall) {
		if (SelectMode != RRVIDEOMODE && SelectMode != RRAUDIOMODE) {
			return;
		}
		goto RR_ORM;
	}
	if (!ppd->IsAllowRemoteRecord && SelectMode == VIDEOMODE) {
		::MessageBox(NULL, DenyRRTxt, DenyRRCpt, MB_OK | MB_ICONWARNING);
		return;
	}
	if (SelectMode == RRVIDEOMODE || SelectMode == VIDEOMODE) {
		SelectMode = RRVIDEOMODE;
	}
	else{ //SelectMode == AUDIOMODE || SelectMode == AUDIOMODE2 || SelectMode == RRAUDIOMODE
		SelectMode = RRAUDIOMODE;
	}
	
RR_ORM:
	ppd->pEraSt->SelectRecMode = SelectMode;
	if (SelectMode == RRVIDEOMODE) {
		ppd->IsAudioMode = 0;
		ppd->IsVideoMode = 1;
		ppd->IsAudioMode2 = 1;
		ppd->IsContinuousRec = 1;
		m_btnMode.LoadBitmaps(IDB_MODERV, IDB_MODERV, 0, 0);
		SetIcon(m_hIcon2, TRUE);
		SetIcon(m_hIcon2, FALSE);
	}
	if (SelectMode == RRAUDIOMODE) {
		ppd->IsAudioMode = 1;
		ppd->IsVideoMode = 0;
		ppd->IsAudioMode2 = 1;
		ppd->IsContinuousRec = 1;
		m_btnMode.LoadBitmaps(IDB_MODERS, IDB_MODERS, 0, 0);
		SetIcon(m_hIcon1, TRUE);
		SetIcon(m_hIcon1, FALSE);
	}

	m_btnMode.Invalidate();
	m_btnMode.UpdateWindow();
	
	if (ppd->hEvent[0] != NULL) {
		ppd->ButEvent = BE_CHANGERSELMODE;
		ppd->pEraSt->SelectRecMode = SelectMode;
		if (!ExtCall) { //при автоматическом вызове функции, новое состояние не отправляем удалённой стороне, т.к. ею и продиктовано в данном случае
			::SetEvent(ppd->hEvent[0]);
		}
	}
	ExtCall = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnRRECORD(NMHDR * pNotifyStruct, LRESULT * result) {
	ppd->ButEvent = BE_PRECHRPARAMS;
	::SetEvent(ppd->hEvent[0]);
	Sleep(10);
	pET = new CERATimer(ppd);
	pET->Create(IDD_ERATIMER, this); //Создание немодального диалогового окна
	pET->ShowWindow(SW_SHOW);

	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::ChangeVisibleBySrvState() {
	if (ppd->pSrvEraSt == NULL) {
		return;
	}
	SelectMode = ppd->pSrvEraSt->SelectRecMode;
	ExtCall = TRUE; //Флаг для функций OnMODE и OnRMODE указывающий, что изменение произошло не по нажатию кнопки Mode, а извне
	if (SelectMode < RRVIDEOMODE) {
		OnMODE();
	}
	else { //SelectMode >= RRVIDEOMODE
		OnRMODE(NULL, NULL);
	}
	//...
	//...
	ppd->pSrvEraSt = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnPHOTO()
{
	m_btnRecord.LoadBitmaps(IDB_PHOTO3, IDB_PHOTO4, 0, 0);
	m_btnStop.LoadBitmaps(IDB_STOP1, IDB_STOP2, 0, 0);
	ppd->IsPhotoMode = 1;
	
	m_btnRecord.Invalidate();
	m_btnStop.Invalidate();
	m_btnRecord.UpdateWindow();
	m_btnStop.UpdateWindow();

	SetCurrentVisible();

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnMONO()
{
	if (ppd->IsMono == ppd->RealMonoMode) {//если реальный моно/стерео режим такой же как на кнопке, то это нажатие на кнопку
		ppd->ButEvent = BE_CHANGEMONOMODE;	//даём сигнал к инверсии режима(моно/стерео) на кодирующей(mp3) стороне
		if (ppd->hEvent[0] != NULL)
			::SetEvent(ppd->hEvent[0]);
	}
	else {//если реальный моно/стерео режим отличается от иконки на кнопке, - внутренний вызов для перерисовки кнопки
		ppd->IsMono = ppd->RealMonoMode;
		if (ppd->IsMono)
			m_btnMono.LoadBitmaps(IDB_MONO3, IDB_MONO4, 0, 0);
		else
			m_btnMono.LoadBitmaps(IDB_MONO1, IDB_MONO2, 0, 0);

		m_btnMono.Invalidate();
		m_btnMono.UpdateWindow();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnSave() 
{
::ZeroMemory(UserFileName,127);
m_edit.GetLine(0,UserFileName,128);

if(UserFileName[0] == (char)0 || UserFileName[0] == (char)0x20 || (char)UserFileName[0] == (char)0x80){
::ZeroMemory(UserFileName,127);
}
m_edit.SetSel(0,-1);
m_edit.Clear();
ppd->IsToSave=1;
SaveRecordProc();
ppd->IsSaveDialog=0;
ppd->IsRecordProcedure = 0;
SetVisible();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnOK(){//действие по нажатию Enter
	RATop();
} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnCancel() 
{
m_edit.SetSel(0,-1);
m_edit.Clear();
ppd->IsToSave = 0;
CancelRecordProc();
ppd->IsSaveDialog = 0;
ppd->IsRecordProcedure = 0;
SetVisible();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnEXIT()
{
	ppd->ButEvent = BE_EXIT;
if (ppd->hProcFF != NULL) {
	TerminateProcess(ppd->hProcFF, 0); //Грубо завершаем ffmpeg, если он имеется
}
if(ppd->hEvent[0]!=NULL)
::SetEvent(ppd->hEvent[0]);
//IsHiddenPanel=1;
SetHidden(0);
//this->EndDialog(1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPanel::OnOFF()
{
	ppd->ButEvent = BE_OFF;
if (ppd->hProcFF != NULL) {
	TerminateProcess(ppd->hProcFF, 0);
}
if(ppd->hEvent[0]!=NULL)
::SetEvent(ppd->hEvent[0]);
//IsHiddenPanel=1;
SetHidden(0);
//this->EndDialog(1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CPanel::OnRecSignal(WPARAM wpar, LPARAM lpar) {

	switch (wpar) {
	
	case PANMSG_STOP:
		OnSTOP();
		break;
	
	case PANMSG_RRSTART:
		OnRRecordStart();
		break;

	case PANMSG_RRSTOP:
		OnRRecordStop(FALSE);
		break;

	case PANMSG_RRPAUSE:
		iTmp = 19;
		break;

	case PANMSG_RRTERMINATE:
		::MessageBox(NULL, TerminateRRTxt, TerminateRRCpt, MB_OK | MB_ICONWARNING);
		if (ppd->IsRecord) {
			OnRRecordStop(FALSE);
		}
		break;

	default:
		break;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPanel::StartRecordProc()
{
if(ppd->IsVideoMode){
	if (!StartVideoRecord()) {
		MsgBox2("Не завершён предыдущий сеанс записи!","Ошибка записи");
		return 0;
	}
}

if(ppd->IsAudioMode){
FullMp3Name[0]=0;
CreateFullName(FullMp3Name);
ppd->FileSize=0;
ppd->hfmp3=NULL;
//ppd->hfmp3=fopen(FullMp3Name,"wb");
ppd->hfmp3=CreateFile(FullMp3Name, GENERIC_WRITE, 0, 0,CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
if(ppd->hfmp3==NULL || (int)ppd->hfmp3==-1) {return 0;}
}

if(ppd->IsAudioMode2){
	StartAudio2Record();
}
return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPanel::PauseRecordProc()
{
if(ppd->IsVideoMode){
	if(ppd->IsPause) ppd->VideoMode=VIDEOPAUSE;
	else ppd->VideoMode=VIDEORECORD;
VideoRecord(ppd);
return 1;
}
//---------------------------------------------------------------------------------------------------------------------------
else{//AudioMode
// флага ppd->IsPause достаточно пока для паузы
}

return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPanel::SaveRecordProc()
{
	if (ppd->IsVideoMode || ppd->IsAudioMode2) {
		ppd->IsWaitMode = TRUE;
		m_btnRecord.LoadBitmaps(IDB_WAIT, IDB_WAIT, 0, 0);
		m_btnRecord.Invalidate();
		m_btnRecord.UpdateWindow();
		SetWaitVisible();
	}
if(ppd->IsVideoMode){
	if (WaitForSingleObject(hEvVideoReady, MAX_WAIT_VIDEOFILE) == WAIT_TIMEOUT) {
		return 0; //подвисание ffmpeg - звук без видео в этом режиме не нужен-выходим...
	}
	if (UserFileName[0]==(char)0) {
		goto IAM; }
	strcpy(NewFileName, UserFileName);
	CreateFullName(NewFileName, MP4_NAME);
	MoveFile(ppd->FullVideoName, NewFileName);
	strcpy(ppd->FullVideoName, NewFileName);
	NewFileName[0] = 0;
}
IAM:
if(ppd->IsAudioMode){
if(ppd->hfmp3==NULL || ppd->hfmp3==INVALID_HANDLE_VALUE){return 0;}
//fclose(ppd->hfmp3);
CloseHandle(ppd->hfmp3);
ppd->hfmp3=INVALID_HANDLE_VALUE;
if(!strlen(UserFileName)){return 1;}
strcpy(NewFileName,ppd->FilePath);
strcat(NewFileName,UserFileName);
strcat(NewFileName,".mp3");
MoveFile(FullMp3Name,NewFileName);
FullMp3Name[0]=NewFileName[0]=0;
return 1;
}

if (ppd->IsAudioMode2) {
	if (!ppd->IsPcmFileReady) { return 0; }
	if (UserFileName[0] == (char)0) {
		goto U_SRP;
	}
	strcpy(NewFileName, UserFileName);
	CreateFullName(NewFileName, PCM_NAME);
	MoveFile(ppd->FullPcmName, NewFileName);
	strcpy(ppd->FullPcmName, NewFileName);
	NewFileName[0] = 0;
}
U_SRP:
if (ppd->IsVideoMode && ppd->IsAudioMode2) {
	UnionAVProc();
	WaitForSingleObject(hEvUnionSaveReady, MAX_WAIT_UNIONFILE);
}
if (ppd->IsWaitMode) {
				
	ppd->IsWaitMode = FALSE;
	m_btnRecord.LoadBitmaps(IDB_REC0, IDB_REC1, 0, 0);
	m_btnRecord.Invalidate();
	m_btnRecord.UpdateWindow();
	SetVisible();
}
return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPanel::CancelRecordProc()
{
if(ppd->IsVideoMode){
	if(!strlen(ppd->FullVideoName)){goto CRP_IAM;}
	for (int n = 0; n < 10; n++) {
		if (DeleteFile(ppd->FullVideoName)) {
			break;
		}
		Sleep(700);
	}
	ppd->FullVideoName[0] = 0;
}

CRP_IAM:
if(ppd->IsAudioMode){
if(ppd->hfmp3==NULL || ppd->hfmp3==INVALID_HANDLE_VALUE){return 0;}
CloseHandle(ppd->hfmp3);
ppd->hfmp3=INVALID_HANDLE_VALUE;
DeleteFile(FullMp3Name);
FullMp3Name[0]=0;
return 1;
}

if (ppd->IsAudioMode2) {
	if (ppd->hfpcm != INVALID_HANDLE_VALUE) {
		CloseHandle(ppd->hfpcm);
		ppd->hfpcm == INVALID_HANDLE_VALUE;
	}
	DeleteFile(ppd->FullPcmName);
	ppd->FullPcmName[0] = 0;
	return 1;
}
return 0;
}


BOOL CPanel::PreCreateWindow(CREATESTRUCT& cs) 
{
//if( !CDialog::PreCreateWindow(cs) )
 //       return FALSE;
	
cs.dwExStyle |= WS_EX_TOOLWINDOW;
return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UINT ThHelp(void* This) {//дополнительная функция окна в отдельном потоке


	HWND hA;
	CPanel *pth = (CPanel *)This;
	POINT cur;
	RECT Rect;
	int nSetHide = 0;

	while (1) {
		Sleep(500);
		DisplayTimer(pth);

		while (pth->ToSetHide) { //если поступил сигнал об установке невидимости после выхода курсора за границы окна панели
			GetWindowRect(pth->m_hWnd, &Rect);
			GetCursorPos(&cur);
			if (cur.x >= Rect.left &&				//проверка, что курсор
				cur.x <= Rect.right &&		//находится в области
				cur.y >= Rect.top &&				//окна панели
				cur.y <= Rect.bottom) {
				nSetHide = 0;
				break;
			}
			if (nSetHide >= 3) {
				pth->SetHidden(0);
				pth->ToSetHide = 0;
				nSetHide = 0;
			}
			else {
				if (!pth->ppd->IsEraTimePanelActive) { //не активно окно EraTimer
					nSetHide++; //1.5 секунды на деактивацию окна при потере фокуса и смещения курсора мыши
				}
			}
			break;
		}

		if (!pth->ppd->IsPhotoMode) {
			pth->BringWindowToTop();
		}
		if (pth->ppd->IsSaveDialog) {
			hA = GetForegroundWindow();/*
			::SendMessage(hA,WM_NCACTIVATE,0,0);
			::SendMessage(hA,WM_KILLFOCUS,0,0);
			::SendMessage(pth->m_hWnd,WM_NCACTIVATE,1,0);
			::SendMessage(pth->m_hWnd,WM_ACTIVATE,1,0);
			::SendMessage(pth->m_hWnd,WM_SETFOCUS,0,0);
			*/
		}

		if (pth->ppd->IsMono != pth->ppd->RealMonoMode) //если реальный моно/стерео-режим отличается от изображения на кнопке,
			pth->OnMONO();								//то перерисовываем кнопочку

	}


	return 1;
}

///////////////////////////////////void CPanel::RATop()//////////////////////////////////////////
void CPanel::RATop() {
	if (ppd->HWRAMainWnd != 0)
	::SetForegroundWindow(ppd->HWRAMainWnd);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MsgBox2(char *Txt, char *Caption) {
	DWORD ThMsgBox2ID;
	HANDLE hThMsgBox2;

	MessageCont.Txt=Txt;
	MessageCont.Caption=Caption;
	hThMsgBox2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThMsgBox2, NULL, 0, &ThMsgBox2ID);
	CloseHandle(hThMsgBox2);

	return 1;
}
//-----------------------------------------------------------------------------------------------------------------------
unsigned long _stdcall ThMsgBox2(void*) {
	::MessageBox(NULL, MessageCont.Txt, MessageCont.Caption, MB_ICONWARNING | MB_SYSTEMMODAL);
	return 0;
}


//=======================================================================================
void DisplayTimer(CPanel *pth) {
PPanelData ppd = pth->ppd;
SYSTEMTIME ST;
UFileTime T;
char DT[32];
/*
BOOL PrevIsRecord = FALSE;
BOOL PrevIsPause = FALSE;
UFileTime T0, TStart, TCur;
*/

if (!IsDTActive && ((!PrevIsRecord && !ppd->IsRecord) || \
	(PrevIsPause && ppd->IsPause))) {
	return;
}
if (!PrevIsRecord && ppd->IsRecord) { //старт записи
	IsDTActive = TRUE;
	GetSystemTime(&ST);
	ST.wHour = ST.wMinute = ST.wSecond = ST.wMilliseconds = 0;
	SystemTimeToFileTime(&ST, &T0.FileTime);
	GetSystemTimeAsFileTime(&TStart.FileTime); TCur = TStart;
}
else if (PrevIsRecord && ppd->IsRecord) { //запись продолжается
	if (PrevIsPause && ppd->IsPause) { //продолжается нажатие паузы
		return;
	}
	GetSystemTimeAsFileTime(&TCur.FileTime);
	if (!PrevIsPause && ppd->IsPause) {//только что нажата пауза
		T0.int64 += (TCur.int64 - TStart.int64);
		goto DT_EX;
	}
	if (PrevIsPause && !ppd->IsPause) {//только что отжата пауза
		GetSystemTimeAsFileTime(&TStart.FileTime); TCur = TStart;
	}
}

if (ppd->IsRecord) {
	T.int64 = T0.int64 + (TCur.int64 - TStart.int64);
	FileTimeToSystemTime(&T.FileTime, &ST);
	sprintf(DT, "%d:%02d.%02d", (int)ST.wHour, (int)ST.wMinute, (int)ST.wSecond);
	SetWindowText(pth->m_hWnd, DT);
}
if (!ppd->IsRecordProcedure) {
	SetWindowText(pth->m_hWnd, ppd->Name);
	IsDTActive = FALSE;
}

DT_EX:
PrevIsRecord = ppd->IsRecord;
PrevIsPause = ppd->IsPause;
	return;
}

void CPanel::OnMove(int x, int y)
{
RECT R;
	
CDialog::OnMove(x, y);
if (!XYOffsetDef) {
	this->GetWindowRect(&R);
	XOffset = x - R.left;
	YOffset = y - R.top;
	XYOffsetDef = TRUE;
}
ppd->pEraSt->PanCoordX = x - XOffset;
ppd->pEraSt->PanCoordY = y - YOffset;
}

#include "Importf.hpp"