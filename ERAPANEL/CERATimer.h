#pragma once
#include "structs.h"

// Диалоговое окно CERATimer

class CERATimer : public CDialogEx
{
	DECLARE_DYNAMIC(CERATimer)

public:
	CERATimer(PPanelData pPD, CWnd* pParent = nullptr);   // стандартный конструктор
	virtual ~CERATimer();
	PPanelData ppd;
	PERAState pest;
	ERAState TPanelState;


// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ERATIMER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV
	

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CButton m_chsrnt;
	CDateTimeCtrl m_dtsrnt;
	afx_msg void OnBnClickedChsrnt();
	CButton m_chsrlen;
	afx_msg void OnBnClickedChsrlen();
	CDateTimeCtrl m_dtsrlen;
	CButton m_chautocpy;
	CEdit m_edfname;
	CButton m_butset;
	afx_msg void OnBnClickedSet();
	CButton m_butabort;
	CProgressCtrl m_progcopy;
	afx_msg LRESULT OnRecSignal(WPARAM wpar, LPARAM lpar);
	afx_msg void OnClose();
	HWND hPrCop;
	CStatic m_copyfname;
	char LastCopyFileName[192];
	CStatic m_fready;
	afx_msg void OnBnClickedAbort();
	bool IsShowAbortBut;
};
