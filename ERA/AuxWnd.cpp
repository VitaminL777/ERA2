#include <windows.h>
#include <Wingdi.h>
#include <stdio.h>
#include <string.h>


static DWORD DWERR;
char szWinName[]="AuxMsg";
char szWinName2[]="AuxMsg2";
char str[255];
char str2[255];
INT PrnInt=0;
INT PrnInt2=0;
static int rez;
static HWND hwnd;
static HWND hwnd2;
static unsigned int j=0;
static HDC hdc2;
extern BOOL IsAuxWnd;
COLORREF ColBrush;
COLORREF ColPen;
//typedef COLORREF (WINAPI *GDIADD)(HDC,COLORREF);
//GDIADD SetDCPenColor;
//GDIADD SetDCBrushColor;
HANDLE hTestEra=INVALID_HANDLE_VALUE;
char TestFileName[] = "C:\\Temp\\EraTest.log";

LRESULT CALLBACK WindowFunc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WindowFunc2(HWND, UINT, WPARAM, LPARAM);

int AuxWnd(HINSTANCE hNst){
	if(!IsAuxWnd){return 0;}

MSG msg;
WNDCLASSEX wcl;
WNDCLASSEX wcl2;

//-------SetDCPenColor и SetDCBrushColor не определены для VC6 - достаём их через LoadLibrary
//LoadLibrary("gdi32");
//HMODULE hGDI32=GetModuleHandle("gdi32");
//SetDCPenColor=(GDIADD)GetProcAddress(hGDI32,"SetDCPenColor");
//SetDCBrushColor=(GDIADD)GetProcAddress(hGDI32,"SetDCBrushColor");
//-------------------------------------------------------------------------------------------

wcl.hInstance=hNst;
wcl.lpszClassName=szWinName;
wcl.lpfnWndProc= WindowFunc;
wcl.style=0;
wcl.cbSize=sizeof(WNDCLASSEX);
wcl.hIcon=LoadIcon(NULL,IDI_APPLICATION);
wcl.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
wcl.hCursor=LoadCursor(NULL,IDC_ARROW);
wcl.lpszMenuName=NULL;
wcl.cbClsExtra=0;
wcl.cbWndExtra=0;
wcl.hbrBackground=(HBRUSH) GetStockObject(GRAY_BRUSH);
rez=RegisterClassEx(&wcl);

wcl2.hInstance=hNst;
wcl2.lpszClassName=szWinName2;
wcl2.lpfnWndProc= WindowFunc2;
wcl2.style=0;
wcl2.cbSize=sizeof(WNDCLASSEX);
wcl2.hIcon=LoadIcon(NULL,IDI_APPLICATION);
wcl2.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
wcl2.hCursor=LoadCursor(NULL,IDC_ARROW);
wcl2.lpszMenuName=NULL;
wcl2.cbClsExtra=0;
wcl2.cbWndExtra=0;
wcl2.hbrBackground=(HBRUSH) GetStockObject(BLACK_BRUSH);
rez=RegisterClassEx(&wcl2);

hwnd=CreateWindow(szWinName,"ERA Aux Message",WS_OVERLAPPED | WS_SYSMENU | WS_BORDER,0,0,1280,500,HWND_DESKTOP,NULL,NULL,NULL);
hwnd2=CreateWindow(szWinName2,"ERA Aux Message2",WS_OVERLAPPEDWINDOW,0,0,1280,500,HWND_DESKTOP,NULL,NULL,NULL);

DWERR=GetLastError();

rez=ShowWindow(hwnd,SW_SHOWNORMAL);
rez=ShowWindow(hwnd2,SW_SHOWNORMAL);
UpdateWindow(hwnd);
UpdateWindow(hwnd2);

while(GetMessage(&msg,NULL,0,0)){
	TranslateMessage(&msg);
	DispatchMessage(&msg);
}

return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowFunc(HWND hwnd, UINT mess, WPARAM wP , LPARAM lP){

static int first=1;
static HDC hdc;
static unsigned int k, p1, p2, p1p, p2p;
static HPEN hpen1, hpen2;
static HBRUSH hbr;

switch(mess){
	case WM_CHAR:
		if(first){
		hdc=GetDC(hwnd);
		SetTextColor(hdc,RGB(0,255,0));
		SetBkMode(hdc, OPAQUE); 
		SetBkColor(hdc,RGB(0,0,0));
		hbr=CreateSolidBrush(RGB(0,0,0));
		hpen1=CreatePen(PS_SOLID,NULL,RGB(0,255,0));
		hpen2=CreatePen(PS_SOLID,NULL,RGB(255,255,0));
		SelectObject(hdc,hbr);
		SelectObject(hdc,hpen1);

		ColPen=CLR_INVALID;
		ColPen=SetDCPenColor(hdc,RGB(0,255,0));
		ColPen=SetDCBrushColor(hdc,RGB(0,255,0));
		ColPen=SetDCBrushColor(hdc,RGB(0,255,0));
		first=0;
		}
		if(!j){
		MoveToEx(hdc,0,500,NULL);
		MoveToEx(hdc2,0,500,NULL);
		Rectangle(hdc,0,0,1280,500);
		Rectangle(hdc2,0,0,1280,500);
		}
	if(wP==1){
		SelectObject(hdc,hpen1);
		itoa(PrnInt,str,10);
		strcat(str,"               ");
		TextOut(hdc,10,0,str,strlen(str));
		
		k=j*5;
		p1=500-(PrnInt/128);
		MoveToEx(hdc,(j-1)*5,p1p,NULL);
		LineTo(hdc,k,p1);
		p1p=p1;
		j++;
	}
	if(wP==2){
		SelectObject(hdc,hpen2);
		itoa(PrnInt2,str,10);
		strcat(str,"               ");
		TextOut(hdc,10,20,str,strlen(str));
		
		k=j*5;
		p2=500-(PrnInt2/2);
		MoveToEx(hdc,(j-1)*5,p2p,NULL);
		LineTo(hdc,k,p2);
		p2p=p2;
		
	}
		if(k>1280){
		j=0;
		}
		UpdateWindow(hwnd);
		//ReleaseDC(hwnd,hdc);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd,mess,wP,lP);
}


return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowFunc2(HWND hwnd2, UINT mess, WPARAM wP , LPARAM lP){

static int first=1;

static unsigned int k;

switch(mess){
	case WM_CHAR:
		if(first){
		hdc2=GetDC(hwnd2);
		SetDCPenColor(hdc2,RGB(0,255,0));
		SetDCBrushColor(hdc2,RGB(0,255,0));
		SaveDC(hdc2);
		first=0;
		}
		if(!j)
		MoveToEx(hdc2,0,500,NULL);
		
		RestoreDC(hdc2,-1);
		itoa(PrnInt2,str2,10);
		strcat(str2,"               ");
		TextOut(hdc2,10,0,str2,strlen(str2));
		
		k=j*5;
		LineTo(hdc2,k,(500-(PrnInt2/2)));
		MoveToEx(hdc2,k,(500-(PrnInt2/2)),NULL);
		
		//ReleaseDC(hwnd,hdc);
		SaveDC(hdc2);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd2,mess,wP,lP);
}


return 0;
}

//*************************************PrnParam(int param)*******************************************************************
void PrnParam(int param){
	char LogParam[128];
	DWORD NOBW;

		if(!IsAuxWnd){return;}

		if (hTestEra == INVALID_HANDLE_VALUE) {
			hTestEra = CreateFile((LPCTSTR)TestFileName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
				OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
			if (hTestEra == INVALID_HANDLE_VALUE) {
				return;
			}
		}

		itoa(param, LogParam, 10);
		strcat(LogParam, "\n");
		WriteFile(hTestEra, LogParam, (DWORD)strlen(LogParam), &NOBW, NULL);
//PrnInt=param;
//SendMessage(hwnd,WM_CHAR,1,0);
return;
}
//*************************************PrnParam2(int param)*******************************************************************
void PrnParam2(int param){
	if(!IsAuxWnd){return;}
//PrnInt2=param;
//SendMessage(hwnd,WM_CHAR,2,0);
return;
}