#if !defined(AFX_PANEL_H__887304CB_D8C2_4E3E_AE57_F3B0B9E6E1F3__INCLUDED_)
#define AFX_PANEL_H__887304CB_D8C2_4E3E_AE57_F3B0B9E6E1F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Panel.h : header file
//
#include "structs.h"
#include "CRButton.h"
#include "CERATimer.h"

#define WS_EX_LAYERED           0x00080000 //для рисования прозрачных окон в VC6
/////////////////////////////////////////////////////////////////////////////
// CPanel dialog

class CPanel : public CDialog
{
// Construction
public:
	CPanel(CWnd* pParent = NULL);   // standard constructor
	int ToSetHide;
	int XPos;
	int YPos;
	int W_H;
	int AXS;
	int AugX;
	int AugY;
	int iWidth;
	int iHeight;
	RECT WRect;
	RECT ClRect;
	BOOL XYOffsetDef;
	int XOffset;
	int YOffset;
	int SelectMode;
	BOOL ExtCall;
	DWORD GTCLastClickMode;
	CERATimer *pCET;
	CERATimer *pET;
	void SetHidden(int smoothly=0);
// Dialog Data
	//{{AFX_DATA(CPanel)
	enum { IDD = IDD_PANEL };
	CEdit	m_edit;
	PPanelData ppd;
	CRButton		m_btnRecord;
	CBitmapButton	m_btnStop;
	CBitmapButton	m_btnPause;
	CRButton		m_btnMode;
	CBitmapButton	m_btnMono;
	CBitmapButton	m_btnOff;
	CBitmapButton	m_btnExit;
	CBitmapButton	m_btnPhoto;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPanel)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon1, m_hIcon2;
	int IsHiddenPanel;
	char Mp3Name[128];
	char UserFileName[128];
	char FullMp3Name[256];
	char NewFileName[256];
	void SetVisible(bool UseCoord = false, int X = 0, int Y = 0);
	void SetPhotoVisible();
	void SetWaitVisible();
	void SetRecordVisible();
	void SetCurrentVisible();
	void SetSaveVisible();
	void ChangeVisibleBySrvState();
	void RATop();
	int  StartRecordProc();
	int  PauseRecordProc();
	int  SaveRecordProc();
	int  CancelRecordProc();
	char tmp[256];
	// Generated message map functions
	//{{AFX_MSG(CPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg BOOL OnNcActivate( BOOL bActive );
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnRECORD();
	afx_msg void OnRRECORD(NMHDR * pNotifyStruct, LRESULT * result);
	afx_msg void OnPAUSE();
	afx_msg void OnSTOP();
	afx_msg void OnMODE();
	afx_msg void OnRMODE(NMHDR * pNotifyStruct, LRESULT * result);
	afx_msg void OnPHOTO();
	afx_msg LRESULT OnRecSignal(WPARAM wpar, LPARAM lpar);
public:
	afx_msg void OnMONO();
protected:
	afx_msg void OnSave();
	afx_msg void OnOFF();
	afx_msg void OnEXIT();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	void OnRRecordStart();
	void OnRRecordStop(BOOL ShowSaveVisible = TRUE);
public:
	afx_msg void OnMove(int x, int y);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PANEL_H__887304CB_D8C2_4E3E_AE57_F3B0B9E6E1F3__INCLUDED_)
