// Repeat.cpp : implementation file
//

#include "stdafx.h"
#include "ERAPANEL.h"
#include "Repeat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Repeat dialog


Repeat::Repeat(CWnd* pParent /*=NULL*/)
	: CDialog(Repeat::IDD, pParent)
{
	//{{AFX_DATA_INIT(Repeat)
	//}}AFX_DATA_INIT
}


void Repeat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Repeat)
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress);
	//}}AFX_DATA_MAP
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL Repeat::OnInitDialog() 
{
	CDialog::OnInitDialog();

SetWindowText(ppd->ConnectTo); //¬ качестве заголовка - им€ ресурса соединени€
SetTimer(ID_TIMER_REPEAT,200,0);
m_Progress.SetRange(0,200);

	return TRUE;  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(Repeat, CDialog)
	//{{AFX_MSG_MAP(Repeat)
//	ON_WM_TIMER()
	//}}AFX_MSG_MAP
//	ON_BN_CLICKED(IDCANCEL, &Repeat::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &Repeat::OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Repeat message handlers


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void Repeat::OnTimer(UINT nIDEvent) 
//{
//static int i=0;
//
//m_Progress.SetPos(i);
//i++;
//if(i>200){ExitProcess(0);}
//
//if(ppd->IsPassedFirstConnect == 1){
//i=200;
//m_Progress.SetPos(i);
//Sleep(500);
//EndDialog(1);
//}
//
//if(ppd->IsPassedFirstConnect == -1){
//i=0;
//m_Progress.SetPos(i);
//Sleep(10000);
//ExitProcess(0);
//}
//	
//	CDialog::OnTimer(nIDEvent);
//}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Repeat::OnTimer(UINT_PTR nIDEvent)
{
	static int i=0;

m_Progress.SetPos(i);
i++;
if(i>200){ExitProcess(0);}

if(ppd->IsPassedFirstConnect == 1){
i=200;
m_Progress.SetPos(i);
Sleep(500);
EndDialog(1);
}

if(ppd->IsPassedFirstConnect == -1){
i=0;
m_Progress.SetPos(i);
Sleep(10000);
ExitProcess(0);
}
	CDialog::OnTimer(nIDEvent);
}

void Repeat::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	ExitProcess(0);
	OnCancel();
}
