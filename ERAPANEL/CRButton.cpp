// CRButton.cpp: файл реализации
//

#include "stdafx.h"
#include "ERAPANEL.h"
#include "CRButton.h"


// CRButton

IMPLEMENT_DYNAMIC(CRButton, CBitmapButton)

CRButton::CRButton()
{

}

CRButton::~CRButton()
{
}


BEGIN_MESSAGE_MAP(CRButton, CBitmapButton)
	//{{AFX_MSG_MAP(CRButton)
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



// Обработчики сообщений CRButton
void CRButton::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	NMHDR hdr;
	hdr.code = NM_RCLICK;
	hdr.hwndFrom = this->GetSafeHwnd();
	hdr.idFrom = GetDlgCtrlID();
	TRACE("OnRButDown");
	this->GetParent()->SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}



