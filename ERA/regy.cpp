#include "windows.h"
#include "regy.h"

BOOL IsKeyOperationsNeed=TRUE;
static HKEY h_klm;
static char ValueName[64];
static unsigned char VData[64];
static DWORD DWVN;
static DWORD DWVD;
static DWORD DWTYPE;
static WORD  *pw;
extern WORD SrvContainerKey;
extern WORD ToPlaceKey;

static void *Tmp;
static char *TmpCh;

static char Chara[]={(char)0xB4,(char)0x9D,(char)0x85,(char)0x7E,(char)0x96,(char)0xB6,(char)0x96,(char)0x8F,(char)0x8F,
					 (char)0x81,(char)0xA7,(char)0x92,(char)0x93,(char)0x75,(char)0x85,(char)0x71,(char)0x87};

static char Stora[]={(char)0xA1,(char)0xAA,(char)0x9F,(char)0xA3,(char)0xB3,(char)0xBA,(char)0xA8,(char)0xAA,(char)0x7f,
				     (char)0x77,(char)0x76,(char)0x8C,(char)0x90,(char)0x79,(char)0xA3,(char)0x8E,(char)0x8C,(char)0x75,
					 (char)0x6E,(char)0x8A,(char)0x8A,(char)0x8C,(char)0x81,(char)0x6F,(char)0x96,(char)0x98,(char)0x83,
					 (char)0x81,(char)0x6A,(char)0x63,(char)0x7F,(char)0x7F,(char)0x8E,(char)0x90,(char)0x7B,(char)0x79,
					 (char)0x62,(char)0x72,(char)0x76,(char)0x5F,(char)0x93,(char)0x73,(char)0x6C,(char)0x6C,(char)0x5E};

static int rez, i,k,j,len;

void InitContainerKey(){

Tmp=malloc(128);
TmpCh=(char*)Tmp;
strcpy(TmpCh,Stora);

for(i=0;i<(int)strlen(Stora);i++){
TmpCh[i]+=i;
TmpCh[i]^=(char)0xF2;
}

RegOpenKey(HKEY_LOCAL_MACHINE,TmpCh,&h_klm);
i=0;
while(1){
	DWVN=DWVD=64;
	rez=RegEnumValue(h_klm,i,ValueName,&DWVN,NULL,&DWTYPE,VData,&DWVD);
	if(rez==ERROR_NO_MORE_ITEMS||i>1000){
		break;
	}
	i++;
	k=0;
	while((char)((ValueName[k]^(char)0xF2)-k)==(char)Chara[k]){
		k++;
		if(k>=16){
		pw=(WORD*)VData;
		SrvContainerKey=(WORD)0xF2F2^*pw;
		break;
		}
	}

}//end_while(1)

RegCloseKey(h_klm);
free(Tmp);
return;

}

//************************************int PlaceContainerKey(WORD key)*******************************************
int PlaceContainerKey(){
	if(!ToPlaceKey){return 0;}
Tmp=malloc(128);
TmpCh=(char*)Tmp;
strcpy(TmpCh,Stora);
h_klm=0;
DWORD DWKEY;

for(i=0;i<(int)strlen(Stora);i++){
TmpCh[i]+=i;
TmpCh[i]^=(char)0xF2;
}

if(ERROR_SUCCESS==RegOpenKey(HKEY_LOCAL_MACHINE,TmpCh,&h_klm)){
	strcpy(TmpCh,Chara);
	for(i=0;i<(int)strlen(Chara);i++){
	TmpCh[i]+=i;
	TmpCh[i]^=(char)0xF2;
	}

	DWKEY=ToPlaceKey;
	ToPlaceKey=0;
	rez=RegSetValueEx(h_klm,TmpCh,NULL,REG_DWORD, (unsigned char *)&DWKEY,(DWORD)4);
	RegCloseKey(h_klm);
}

free(Tmp);

return 1;
}


//*****************************************int KeyOperations()**************************************************
int KeyOperations(){
	
if(IsKeyOperationsNeed==FALSE)
	return 0;

if(ToPlaceKey)
PlaceContainerKey();

InitContainerKey();
IsKeyOperationsNeed=FALSE;

return 1;

}