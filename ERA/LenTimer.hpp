DWORD ThLenTimerID;
HANDLE hThLenTimer;
HANDLE hEvLenTimer = NULL;
HANDLE hEvLTSync = NULL;
UFileTime ULTStart, ULTWakeup, ULT0, ULT1, ULT2, ULTSlept;
SYSTEMTIME STLTStart, SLT, STLT1, STLT2, *pST;
DWORD LTSleep, LTSleepS, WFSOLT;
WORD WULTReason;

unsigned long _stdcall ThLenTimer(void*);


//--------------------------------------------------------------------------------------
int LTChangeStateByRecordState(/*PanelData PanDat, ERAState EraState*/){
BYTE RS;
int ret = 0;

	if (!EraState.LenTimerSelect){
		if (!(PanDat.LTState&LTS_START)) { //������ �� ������ � �� �������
			return 0;
		}
		else { //PanDat.LTState&LTS_START //������ ������ ��� ���������� �������
			WULTReason = WULT_STOP;
			SetEvent(hEvLenTimer);
			return 1;
		}
	}
	//if(EraState.LenTimerSelect)
	RS = EraState.RecordState;
	if (RS&RS_LRECORD) {//�������� ������
		if (!(PanDat.LTState&LTS_START)) { //� LT �� �������
			hThLenTimer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThLenTimer, NULL, 0, &ThLenTimerID);
		}
		else if (RS&RS_LPAUSE) { //���. ������ � ������ �����
			WULTReason = WULT_PAUSEON;
			SetEvent(hEvLenTimer);
			ret = 1;
		}
		else if (!(RS&RS_LPAUSE)) { //���. ������ � ������ �����
			WULTReason = WULT_PAUSEOFF;
			SetEvent(hEvLenTimer);
			ret = 1;
		}
	}
	else {//!RS&RS_LRECORD //��������� ������
		if (PanDat.LTState&LTS_START) { //� LT �������
			WULTReason = WULT_STOP;
			SetEvent(hEvLenTimer);
			ret = 1;
		}
		else { //LT �� �������

		}
	}
	return ret;
}

//--------------------------------------------------------------------------------------
int LTSetTime(/*PanelData PanDat, ERAState EraState*/) {
	if (!(PanDat.LTState&LTS_START)) { return 0; }
	WULTReason = WULT_SETTIME;
	SetEvent(hEvLenTimer);
	return 1;
}

//--------------------------------------------------------------------------------------
int LTGetTime(/*PanelData PanDat, ERAState EraState*/) {
	if (!(PanDat.LTState&LTS_START)) { return 0; }
	if (PanDat.LTState&LTS_PAUSE) {
		EraState.LenTimer = SLT;
	}
	else {
		WULTReason = WULT_GETTIME;
		SetEvent(hEvLenTimer);
		WaitForSingleObject(hEvLTSync, 2000);
	}
	return 1;
}


unsigned long _stdcall ThLenTimer(void*) {
	
	pST = &EraState.LenTimer;
	hEvLenTimer = CreateEvent(NULL, FALSE, FALSE, NULL);
	hEvLTSync = CreateEvent(NULL, FALSE, FALSE, NULL);
LT_ST:
	GetSystemTime(&STLT1);
	STLT1.wHour = STLT1.wMinute = STLT1.wSecond = STLT1.wMilliseconds = 0;
	SystemTimeToFileTime(&STLT1, &ULT1.FileTime); ULT0 = ULT1;
	STLT1.wHour = pST->wHour; STLT1.wMinute = pST->wMinute; STLT1.wSecond = pST->wSecond; 
	SystemTimeToFileTime(&STLT1, &ULT2.FileTime);
	LTSleep = LTSleepS = (DWORD)((ULT2.int64 - ULT1.int64) / 10000);
	GetSystemTimeAsFileTime(&ULTStart.FileTime);
	PanDat.LTState |= LTS_START;
LT_WFSO:
	DWORD DWWait = PanDat.LTState&LTS_PAUSE ? INFINITE : LTSleep;
	WFSOLT = WaitForSingleObject(hEvLenTimer, DWWait);
	if (WFSOLT == WAIT_TIMEOUT) { //�������� ������
		goto LT_PM;
	}
	else if (WFSOLT != WAIT_OBJECT_0) { //����������� ������
		goto LT_CH;
	}
	else { // WFSOLT == WAIT_OBJECT_0 (����� ����� �� ������� ������ � ��������� ��������)
		
		switch (WULTReason) 
		{
			case WULT_STOP:
				goto LT_CH;

			case WULT_PAUSEON:
				PanDat.LTState |= LTS_PAUSE;
				goto LT_CGT;
			
			case WULT_PAUSEOFF:
				PanDat.LTState &= ~LTS_PAUSE;
				goto LT_ST;
								
			case WULT_SETTIME:
				goto LT_ST;

			case WULT_GETTIME:
LT_CGT:
				GetSystemTimeAsFileTime(&ULTWakeup.FileTime);
				ULTSlept.int64 = (ULTWakeup.int64 - ULTStart.int64) / 10000;
				LTSleep = LTSleepS - (DWORD)(ULTSlept.int64);
				ULT1.int64 = ULT0.int64 + (UINT64)((UINT64)LTSleep * 10000);
				FileTimeToSystemTime(&ULT1.FileTime, &SLT);
				EraState.LenTimer = SLT;
				SetEvent(hEvLTSync);
				goto LT_WFSO;

			default: //����������� �������(������ �� ������ ����)
				goto LT_CH;
		}
	}

LT_PM:
	if (EraState.RecordState&RS_LRECORD) {
		::PostMessage(PanDat.hWndSelf, PanDat.UWM_Msg, 0, 0); //���������� ��������� ������ ���������� ������ "����"
		EraState.LenTimer.wHour = 2; EraState.LenTimer.wMinute = EraState.LenTimer.wSecond = 0;
		EraState.LenTimerSelect = false;
		//������ ������� �������� ���������� �� ��������� �������
		PanDat.pEraSt->MapChParams |= (CH_LENTIMERVALUE | CH_TYPETIMERSEL);
		PanDat.ButEvent = BE_CHANGERPARAMS;
		::SetEvent(PanDat.hEvent[0]);
	}
LT_CH:
	CloseHandle(hEvLenTimer); hEvLenTimer = NULL;
	CloseHandle(hEvLTSync); hEvLTSync = NULL;
	PanDat.LTState = 0;
	return 1;
}