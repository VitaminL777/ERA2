#if !defined(AFX_REPEAT_H__2CDD3CE5_0035_4E52_A9A3_0B43187450AD__INCLUDED_)
#define AFX_REPEAT_H__2CDD3CE5_0035_4E52_A9A3_0B43187450AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Repeat.h : header file
//
#include "structs.h"


/////////////////////////////////////////////////////////////////////////////
// Repeat dialog

class Repeat : public CDialog
{
// Construction
public:
	Repeat(CWnd* pParent = NULL);   // standard constructor
	PPanelData ppd;

// Dialog Data
	//{{AFX_DATA(Repeat)
	enum { IDD = IDD_REPEAT };
	CProgressCtrl	m_Progress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Repeat)
	public:
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Repeat)
	virtual BOOL OnInitDialog();
//	afx_msg void OnTimer(UINT nIDEvent);
	//virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnBnClickedCancel();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg void OnBnClickedCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REPEAT_H__2CDD3CE5_0035_4E52_A9A3_0B43187450AD__INCLUDED_)
