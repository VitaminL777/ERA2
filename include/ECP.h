typedef struct _ECPACK {
	DWORD B7;//0xB7770315
	DWORD ID;
	DWORD SetState;
	DWORD RetCode;
}ECPACK, *PECPACK;

int DataExchange(char *Gate, int ID, int SetState);//EP
int SendCont(PECPACK PEP);
int RecvCont(PECPACK PEP, DWORD timeout);