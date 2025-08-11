// ERAPANEL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "ERAPANEL.h"
#include "Panel.h"
#include "Repeat.h"
#include "MediaFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern HCURSOR hCurGrn;
extern PPanelData ppd_;

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CERAPANELApp

BEGIN_MESSAGE_MAP(CERAPANELApp, CWinApp)
	//{{AFX_MSG_MAP(CERAPANELApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CERAPANELApp construction

CERAPANELApp::CERAPANELApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CERAPANELApp object

CERAPANELApp theApp;



/////////////////////////////////////////////////////////////////////////////
// CERAPANELApp initialization

BOOL CERAPANELApp::InitInstance()
{
	// Register all OLE server (factories) as running.  This enables the
	//  OLE libraries to create objects from other applications.
	COleObjectFactory::RegisterAll();
	hCurGrn=LoadCursor(IDC_CURSOR1);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Special entry points required for inproc servers


STDAPI CallPanel(PPanelData ppd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());//дл€ корректной работы mfc-классов внутри библиотечных функций
Sleep(1000);
if(!ppd->IsPassedFirstConnect){
Repeat repeat;
repeat.ppd=ppd;
repeat.DoModal();
}
if(ppd->IsPassedFirstConnect==-1){Sleep(-1);}
CHideWin Chw;//создаЄм экземл€р класса простейшего невидимого окна
CPanel panel(&Chw);//передаЄм CWnd невидимого окна нашему диалогу в качестве родител€ дл€ того, чтобы программа не отображалась 
//на панели задач, работает только совместно со сн€тым EXSTYLE WS_EX_APPWINDOW в секции диалога файла ресурсов (ERAPANEL.rc)
panel.ppd=ppd;//передаЄм адрес структуры PanelData внутрь класса CPanel
ppd_ = ppd;
ppd->IsAudioMode = 1;
InitPPanelData(ppd);//передаЄм адрес структуры PanelData внутрь MediaFile.cpp
panel.pCET = new CERATimer(ppd); //создаЄм экземпл€р ERATimer с передачей ему адреса структуры PanelData
panel.DoModal();//запускаем модальный диалог

	return AfxDllCanUnloadNow();
}

STDAPI CallPanelVoid()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
PanelData pd;
::ZeroMemory(&pd,sizeof(pd));
pd.X=220;
pd.IsMono=1;
pd.IsRecord=0;
pd.IsPause=1;
pd.IsAudioMode2 = 0;
pd.IsPassedFirstConnect=1;

CallPanel(&pd);

	return AfxDllCanUnloadNow();
}

CHideWin::CHideWin()
{
 	// —оздание окна с заголовком. »спользуетс€
 	// встроенный в MFC
  	// класс окна, поэтому первый параметр 0.
 	this->Create( 0, "ERA_Hide");
}
