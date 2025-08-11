#include "nitaxml.h"

const char NitaXmlHead[] = "<?xml version=\"1.0\" encoding=\"windows-1251\" ?>\n";

nitaxml::nitaxml()
{
	IsXMLOpen = 0;
	xmldoc = NULL;
	XMLFullName[0] = 0;
	XMLE[0] = 0;
	XMLERoot = 0;
	XMLECur = 0;
	FullName[0] = 0;
	KeysSet[0] = 0;
	
}

nitaxml::~nitaxml()
{
	if (xmldoc != NULL) {
		delete xmldoc;
		xmldoc = NULL;
	}
}

int nitaxml::XMLOpen(char *XMLName, bool CreateIfNotExists) {

	if (IsXMLOpen) {
		return 0;
	}
	xmldoc = new XML::XMLDocument;
	xmlerr=xmldoc->LoadFile(XMLName);
	if (xmlerr != XML::XML_SUCCESS) {
		if (!CreateIfNotExists || !XMLNew(XMLName)) {
			return 0;
		}
		xmlerr = xmldoc->LoadFile(XMLName);
		if (xmlerr != XML::XML_SUCCESS) {
			return 0;
		}
		XMLERoot = xmldoc->NewElement("n");
		xmldoc->InsertEndChild(XMLERoot);
		XMLERoot->SetAttribute("ver", "1.1");
	}
	XMLERoot = xmldoc->FirstChildElement("n");
	if (XMLERoot == NULL) {
		delete xmldoc;
		xmldoc = 0;
		return 0;
	}
	IsXMLOpen = 1;
	strcpy(XMLFullName, XMLName);
	return 1;
}

///////////////////////////////////////////////////////////////////////
int nitaxml::XMLOpen2(char *XMLName, char *FindElem) {

	if (IsXMLOpen) {
		return 0;
	}
	xmldoc = new XML::XMLDocument;
	xmlerr = xmldoc->LoadFile(XMLName);
	if (xmlerr != XML::XML_SUCCESS) {
		return 0;
	}
	XMLERoot = xmldoc->FirstChildElement(FindElem);
	if (XMLERoot == NULL) {
		delete xmldoc;
		xmldoc = 0;
		return 0;
	}
	IsXMLOpen = 1;
	strcpy(XMLFullName, XMLName);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::XMLNew(char *XMLName) {
int i;
HANDLE fxml;
DWORD DBW;

fxml = CreateFile(XMLName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
	CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
if (fxml == INVALID_HANDLE_VALUE) {
	return 0;
}
SetFilePointer(fxml, 0, 0, FILE_BEGIN);
SetEndOfFile(fxml);
WriteFile(fxml, NitaXmlHead, strlen(NitaXmlHead), &DBW, NULL);
CloseHandle(fxml);
	
return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::XMLClose() {

	if (!IsXMLOpen || xmldoc == NULL) {
		return 0;
	}
	delete xmldoc;
	xmldoc = NULL;
	IsXMLOpen = 0;
	return 1;
}

int nitaxml::XMLSave() {
	return XMLSave(XMLFullName);
}

int nitaxml::XMLSave(char *XMLName) {
	if (XMLName != NULL && strlen(XMLName)) {
		xmldoc->SaveFile(XMLName, 0);
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::KeysParse(char *ParamName) {
	int i=0, j=0;
	int ParNameLen = 0;

	if (ParamName == NULL) {
		return 0;
	}
	ParNameLen = strlen(ParamName);
	if (!IsXMLOpen || ParNameLen>MAXPARLEN || !ParNameLen ) {
		return 0;
	}
	strcpy(FullName, ParamName);
	while (i < MAXKEYSLEN) {
		KeysSet[i] = &FullName[j];
		while (j <= ParNameLen) {
			if (FullName[j] == 0x5C || FullName[j] == 0x2F || FullName[j] == 0) {
				FullName[j] = 0;
				i++; j++;
				break;
			}
			j++;
		}
		if (j >= ParNameLen) {
			break;
		}
	}
	KeysSet[i] = 0;  i = j = 0;
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XML::XMLElement* nitaxml::GetXMLElement(XML::XMLElement *ParentEl, char **PKeysSet, int Back) {

	char **PKS;
	XML::XMLElement *XMLFind;
	XML::XMLElement *ParE;
	const XML::XMLAttribute *xa;

	if (PKeysSet==NULL || !strcmp(*PKeysSet, "")) {//если пустой указатель на сеты ключей
		PKS = KeysSet;							//берём внутренний набор	
	}
	else {
		PKS = PKeysSet;
	}
	if (ParentEl == NULL) {
		ParE = XMLERoot;}
	else {
		ParE = ParentEl;}

	XMLFind = ParE->FirstChildElement("n");
	if (XMLFind == NULL) {
		return 0;
	}
	do {
		xa = XMLFind->FindAttribute("n");
			if (xa == NULL) {
				continue;
			}
			if (!strcmp(*PKS, xa->Value())) {
				PKS++;
				if (*PKS == NULL) {//последний элемент(ключ) в наборе
					XMLEPar = ParE;
					if (Back) {
						XMLFind = XMLFind->PreviousSiblingElement("n");
					}
					return XMLFind;
				}
				return GetXMLElement(XMLFind, PKS, Back);
			}
		
	} while (NULL != (XMLFind = XMLFind->NextSiblingElement("n")));
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::FullGetParam(char *ParamName, char *ParamValue, int *TypeElement ) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	XMLEFind=FindXMLElement(ParamName, FROM_ROOT, &Type);
	if (TypeElement != 0 && XMLEFind != 0) {
		*TypeElement = Type;
	}
	XMLECur = XMLECurSav;
	if (XMLEFind != NULL && Type == NITA_PARAM) {
		xa = XMLEFind->FindAttribute("v");
		if (xa == NULL) {
			return 0;
		}
		if (ParamValue != NULL) {
			strcpy(ParamValue, xa->Value());
		}
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::FullGetParam(char *ParamName, int *IntValue, int *TypeElement) {
	char ParamValue[256];

	if (!FullGetParam(ParamName, ParamValue, TypeElement)) {
		return 0;
	}
	*IntValue = strtol(ParamValue, NULL, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::FullGetParam(char *ParamName, INT64 *IntValue, int *TypeElement) {
	char ParamValue[256];
	
	if (!FullGetParam(ParamName, ParamValue, TypeElement)) {
		return 0;
	}
	*IntValue = strtoll(ParamValue, NULL, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::FullGetParam(char *ParamName, double *dValue, int *TypeElement) {
	char ParamValue[256];

	if (!FullGetParam(ParamName, ParamValue, TypeElement)) {
		return 0;
	}
	*dValue = strtod(ParamValue, NULL);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetParam(char *ParamName, char *ParamValue, int *TypeElement, int SymLimit) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	if (XMLECur == 0) {
		XMLECur = XMLERoot; //если текущий ключ не задан - задаём корневой
		if (XMLECur == 0) {
			return 0;
		}
	}
	XMLEFind = FindXMLElement(ParamName, FROM_CUR, &Type);
	XMLECur = XMLECurSav;
	if (TypeElement != 0 && XMLEFind != 0) {
		*TypeElement = Type;
	}
	if (XMLEFind != NULL && Type == NITA_PARAM) {
		xa = XMLEFind->FindAttribute("v");
		if (xa == NULL) {
			return 0;
		}
		if (ParamValue != NULL) {
			if (!SymLimit) {
				strcpy(ParamValue, xa->Value());
			}
			else {
				strncpy(ParamValue, xa->Value(), SymLimit);
				ParamValue[SymLimit] = 0;
			}
		}
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetParam(char *ParamName, int *IntValue, int *TypeElement) {
	char ParamValue[256];

	if (!GetParam(ParamName, ParamValue, TypeElement)) {
		return 0;
	}
	*IntValue = strtol(ParamValue, NULL, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetParam(char *ParamName, INT64 *IntValue, int *TypeElement) {
	char ParamValue[256];

	if (!GetParam(ParamName, ParamValue, TypeElement)) {
		return 0;
	}
	*IntValue = strtoll(ParamValue, NULL, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetParam(char *ParamName, UINT64 *IntValue, int *TypeElement) {
	char ParamValue[256];

	if (!GetParam(ParamName, ParamValue, TypeElement)) {
		return 0;
	}
	*IntValue = strtoull(ParamValue, NULL, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetParam(char *ParamName, double *dValue, int *TypeElement) {
	char ParamValue[256];

	if (!GetParam(ParamName, ParamValue, TypeElement)) {
		return 0;
	}
		*dValue = strtod(ParamValue, NULL);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XML::XMLElement* nitaxml::FindXMLElement(char *ParamName, int SearchFrom, int *TypeElement, int Back) {
	XML::XMLElement *XMLEPar;
	XML::XMLElement *XMLEFind=NULL;

	if (!KeysParse(ParamName)) {
		return 0; }

	if (SearchFrom == FROM_ROOT && XMLERoot != NULL) {
		//XMLEPar = XMLERoot; вызов GetXMLElement с нулевым первым аргументом включает XMLEPar = XMLERoot
		XMLEFind=GetXMLElement(0, 0, Back);
	}
	if (SearchFrom == FROM_CUR) {
		if (XMLECur == NULL) {
			XMLECur = XMLERoot;}
		XMLEFind = GetXMLElement(XMLECur, 0, Back);
	}
	if (XMLEFind != NULL) {
		if (TypeElement != NULL) {
			*TypeElement = NITA_UNKNOWN;
			if (XMLEFind->ClosingType()==1) {
				*TypeElement = NITA_PARAM;
			}
			if (XMLEFind->ClosingType()==0) {
				*TypeElement = NITA_KEY;
			}
		}
		return XMLEFind;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::FullSetParam(char *ParamName, char *ParamValue) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	XMLEFind = FindXMLElement(ParamName, FROM_ROOT, &Type);
	XMLECur = XMLECurSav;
	if (XMLEFind != NULL && Type == NITA_PARAM) {
		if (ParamValue != NULL) {
			XMLEFind->SetAttribute("v", ParamValue);
			return 1;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::FullSetParam(char *ParamName, INT64 IntValue) {
char ParamValue[32];

_i64toa(IntValue, ParamValue, 10);
return FullSetParam(ParamName, ParamValue);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::SetCurrentKey(char *KeyName) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;
	
	if (KeyName == NULL || !(strcmp(KeyName, ""))) {
		XMLECur = XMLERoot;
		return 1;
	}
	XMLEFind = FindXMLElement(KeyName, FROM_ROOT, &Type);
	if (XMLEFind != 0) {// && Type==NITA_KEY) {
		XMLECur = XMLEFind;
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::SetParam(char *ParamName, char *ParamValue, int From) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	XMLEFind = FindXMLElement(ParamName, From, &Type);
	XMLECur = XMLECurSav;

	if (XMLEFind != NULL && (Type == NITA_PARAM || Type == NITA_KEY)) {
		if (ParamValue != NULL) {
			XMLEFind->SetAttribute("v", ParamValue);
			return 1;
		}
	}
	if (XMLEFind == NULL) {
		return CreateParam(ParamName, ParamValue);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int  nitaxml::SetParam(char *ParamName, INT64 IntValue, int From) {
char ParamValue[32];

_i64toa(IntValue, ParamValue, 10);
return SetParam(ParamName, ParamValue, From);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int  nitaxml::SetParam(char *ParamName, UINT64 IntValue, int From) {
char ParamValue[32];

_ui64toa(IntValue, ParamValue, 10);
return SetParam(ParamName, ParamValue, From);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int  nitaxml::CreateParam(char *ParamName, char *ParamValue, int From) {
	XML::XMLElement *ne = NULL;
	XML::XMLNode *retn = NULL;
	int WhereInsert = 0;

	WhereInsert = From > 1 ? 1 : 0;
	From &= 1;
	
	if (xmldoc == NULL) {
		return 0;}

	XMLECurSav = XMLECur;
	ne = xmldoc->NewElement("n");
	ne->SetAttribute("n", ParamName);
	ne->SetAttribute("v", ParamValue);
	if (XMLECur == 0 || From == FROM_ROOT) {
		XMLECur = XMLERoot; //если текущий ключ не задан - задаём корневой
		if (XMLECur == 0) {
			XMLECur = XMLECurSav;
			return 0;}
	}
	if (!WhereInsert) {
		retn = XMLECur->InsertEndChild(ne);
	}
	else {
		retn = XMLECur->InsertFirstChild(ne);
	}
	XMLECur = XMLECurSav;
	if (retn == NULL) {
		return 0;
	}
	return 1;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::CreateParam(char *ParamName, UINT64 IntValue, int From) {
	char ParamValue[32];

	_ui64toa(IntValue, ParamValue, 10);
	return CreateParam(ParamName, ParamValue, From);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::CreateParam(char *ParamName, INT64 IntValue, int From) {
char ParamValue[32];

_i64toa(IntValue, ParamValue, 10);
return CreateParam(ParamName, ParamValue, From);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::InsertParam(char *ParamName, char *ParamValue, char *ReferParam, int Where, int From) {
	XML::XMLElement *ne = NULL;
	XML::XMLElement *einner = NULL;
	XML::XMLNode *InsE = NULL;
	XML::XMLNode *retn = NULL;
	int WhereInsert = 0;
	int TE;
	int ret = 0;
	int InsFirst = 0;
	
	if (xmldoc == NULL) {
		return 0;
	}
	XMLECurSav = XMLECur;
	InsFirst = 0;
	if (XMLECur == 0 || From == FROM_ROOT) {
		XMLECur = XMLERoot; //если текущий ключ не задан - задаём корневой
		if (XMLECur == 0) {
			XMLECur = XMLECurSav;
			return 0;
		}
	}
	if (Where == INS_AFTER) {
		if (!(einner = FindXMLElement(ReferParam, From, &TE))) {
			goto RET_NXIP;
		}
	}
	else if (Where == INS_BEFORE) {
		if (!(einner = FindXMLElement(ReferParam, From, &TE, 1))) {
			if (!(einner = FindXMLElement(ReferParam, From, &TE))) {
				goto RET_NXIP;
			}
			InsFirst = 1;
		}
	}
	ne = xmldoc->NewElement("n");
	ne->SetAttribute("n", ParamName);
	ne->SetAttribute("v", ParamValue);
	if(InsFirst) {
		InsE = XMLECur->InsertFirstChild(ne);
	}
	else {
		InsE = XMLECur->InsertAfterChild(einner, ne);
	}
	ret = 1;

RET_NXIP:
	XMLECur = XMLECurSav;
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::SetAttr(char *ParamName, char *AttrName, char *AttrValue, int From) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	XMLEFind = FindXMLElement(ParamName, From, &Type);
	XMLECur = XMLECurSav;

	if (XMLEFind != NULL && (Type == NITA_PARAM || Type == NITA_KEY)) {
		if (AttrValue != NULL) {
			XMLEFind->SetAttribute(AttrName, AttrValue);
			return 1;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::SetAttr(char *ParamName, char *AttrName, INT64 AttrValue, int From) {
char ParamValue[32];

_i64toa(AttrValue, ParamValue, 10);
return SetAttr(ParamName, AttrName, ParamValue, From);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetAttr(char *ParamName, char *AttrName, char *AttrValue, int From) {
int Type;
XML::XMLElement *XMLEFind = NULL;
const XML::XMLAttribute *xa;

if (!ParamName || !AttrName || !strlen(ParamName) || !strlen(AttrName)) {
	return 0;
}

XMLECurSav = XMLECur;
if (XMLECur == 0) {
	XMLECur = XMLERoot; //если текущий ключ не задан - задаём корневой
	if (XMLECur == 0) {
		return 0;
	}
}
XMLEFind = FindXMLElement(ParamName, From, &Type);
XMLECur = XMLECurSav;
	
if (XMLEFind != NULL) {
	xa = XMLEFind->FindAttribute(AttrName);
	if (xa == NULL) {
		return 0;
	}
	if (AttrValue != NULL) {
		strcpy(AttrValue, xa->Value());
	}
	return 1;
}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetAttr(char *ParamName, char *AttrName, INT64 *AttrValue, int From) {
char ParamValue[256];

if (!GetAttr(ParamName, AttrName, ParamValue, From)) {
	return 0;
}
*AttrValue = strtoll(ParamValue, NULL, 0);
return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetAttr(char *ParamName, char *AttrName, int *AttrValue, int From) {
char ParamValue[256];

if (!GetAttr(ParamName, AttrName, ParamValue, From)) {
	return 0;
}
*AttrValue = strtol(ParamValue, NULL, 0);
return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int  nitaxml::DeleteParam(char *ParamName, int From) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	XMLEPar = 0;
	XMLEFind = FindXMLElement(ParamName, From, &Type);
	XMLECur = XMLECurSav;
	if (From == FROM_ROOT) {
		XMLECur = XMLERoot;
	}
	
	if (XMLEFind != NULL && XMLEPar != NULL){// && Type == NITA_PARAM) {
		XMLEPar->DeleteChild(XMLEFind);
		XMLECur = XMLECurSav;
		return 1;
	}
	XMLECur = XMLECurSav;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int  nitaxml::DeleteKey(char *KeyName, int From) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	XMLEPar = 0;
	XMLEFind = FindXMLElement(KeyName, From, &Type);
	if (From == FROM_ROOT) {
		XMLECur = XMLERoot;
	}
	
	if (XMLEFind != NULL && XMLEPar != NULL) {
		XMLEFind->DeleteChildren();
		XMLEPar->DeleteChild(XMLEFind);
		XMLECur = XMLECurSav;
		return 1;
	}
	XMLECur = XMLECurSav;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::DeleteAttr(char *ParamName, char *AttrName, int From) {
	int Type;
	XML::XMLElement *XMLEFind = NULL;
	const XML::XMLAttribute *xa;

	XMLECurSav = XMLECur;
	XMLEFind = FindXMLElement(ParamName, From, &Type);
	if (From == FROM_ROOT) {
		XMLECur = XMLERoot;
	}

	if (XMLEFind != NULL) {
		XMLEFind->DeleteAttribute(AttrName);
		XMLECur = XMLECurSav;
		return 1;
	}
	XMLECur = XMLECurSav;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::CreateKey(char *KeyName, BOOL SetCurKey) {
	int rez;
	XML::XMLElement *ne = NULL;
	XML::XMLElement *ne2 = NULL;
	XML::XMLElement *ne3 = NULL;
	XML::XMLNode *nd = NULL;
	char SSK[256];
	char FSK[256];

	XMLECurSav = XMLECur;
	if (SetCurrentKey(KeyName)) {
		if (!SetCurKey) {
			XMLECur = XMLECurSav;
		}
		return 1;
	}
	XMLECur = XMLERoot;
	GetNextSubKey(KeyName, 0, 0, GNS_LOAD);
	while (GetNextSubKey(0, SSK, FSK)) {
		if (SetCurrentKey(FSK)) {
			continue;
		}
		ne = xmldoc->NewElement("n");
		ne->SetAttribute("n", SSK);
		XMLECur->InsertEndChild(ne);
		if (!SetCurrentKey(FSK)) {
			if (!SetCurKey) {
				XMLECur = XMLECurSav;
			}
			return 0;
		}
	}
	if (!SetCurKey) {
		XMLECur = XMLECurSav;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextSubKey(char *KeyName, char *ShortSubKey, char *FullSubKey, int Set){
static int State;
static char KEYNAME[256];
static char *SK, *SK2, *KE;
static int i, lenkey, lenfsk;
if (Set == GNS_LOAD) {
	if (!KeyName || !(lenkey=strlen(KeyName))) {
		return 0;
	}
	strcpy(KEYNAME, KeyName);
	SK = SK2 = KEYNAME;
	KE = SK + lenkey;
	State = GNS_NEXT;
	return 1;
}
if (Set == GNS_NEXT) {
	if (State == GNS_STOP || !ShortSubKey || !FullSubKey) {
		return 0;
	}
	i = 0;
	while (SK < KE && *SK != '/') {
		*(ShortSubKey+i) = *SK;
		SK++; i++;
	}
	*(ShortSubKey + i) = 0;
	lenfsk = SK - KEYNAME;
	strncpy(FullSubKey, KEYNAME, (size_t)lenfsk);
	*(FullSubKey + lenfsk) = 0;
	if (SK >= KE) {
		State = GNS_STOP;
	}
	else {//*SK != '/'
		SK++;
	}
	return 1;
}
return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PXMLFind nitaxml::GetNextInit(char *ParKeyName) {
	int Type;
	PXMLFind pXF;
	
	pXF = new XMLFind;
	pXF->StateNext = STATE_START;
	if (ParKeyName == NULL || !(strcmp(ParKeyName, ""))) {
		pXF->XMLENext = XMLERoot;
		return pXF;
	}
	pXF->XMLENext = FindXMLElement(ParKeyName, FROM_ROOT, &Type);
	if (pXF->XMLENext != 0) {// && Type==NITA_KEY) {
		return pXF;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextKey(PXMLFind *ppXF, char *KeyName) {
	const XML::XMLAttribute *xan, *xav;
	PXMLFind pXF;
	char keyname[256];

	pXF = *ppXF;

	if (pXF == NULL || pXF->StateNext == STATE_STOP || !KeyName) {
		return 0;
	}
	if (pXF->StateNext == STATE_START) {
		pXF->XMLENext = pXF->XMLENext->FirstChildElement("n");
		if (pXF->XMLENext == NULL) {
			pXF->StateNext = STATE_STOP;
			delete pXF; *ppXF = 0;
			return 0;
		}
		pXF->StateNext = STATE_NEXT;
		xan = pXF->XMLENext->FindAttribute("n");
		xav = pXF->XMLENext->FindAttribute("v");
		if (xan != NULL && xav == NULL) {
			strcpy(KeyName, xan->Value());
			return 1;
		}
	}
	if (pXF->StateNext == STATE_NEXT) {
		while (NULL != (pXF->XMLENext = pXF->XMLENext->NextSiblingElement("n"))) {
			xan = pXF->XMLENext->FindAttribute("n");
			xav = pXF->XMLENext->FindAttribute("v");
			if (xan != NULL && xav == NULL) {
				strcpy(KeyName, xan->Value());
				return 1;
			}
		}
	}
	pXF->StateNext = STATE_STOP;
	delete pXF; *ppXF = 0;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextParam(PXMLFind *ppXF, char *ParamName, char *Value) {
	const XML::XMLAttribute *xan, *xav;
	PXMLFind pXF;
	char keyname[256];

	pXF = *ppXF;
	if (pXF == NULL || pXF->StateNext == STATE_STOP || !ParamName || !Value) {
		return 0;
	}
	
	if (pXF->StateNext == STATE_START) {
		pXF->XMLENext = pXF->XMLENext->FirstChildElement("n");
		if (pXF->XMLENext == NULL) {
			pXF->StateNext = STATE_STOP;
			delete pXF; *ppXF = 0;
			return 0;
		}
		pXF->StateNext = STATE_NEXT;
		xan = pXF->XMLENext->FindAttribute("n");
		xav = pXF->XMLENext->FindAttribute("v");
		if (xan != NULL && xav != NULL) {
			strcpy(ParamName, xan->Value());
			strcpy(Value, xav->Value());
			return 1;
		}
	}
	if (pXF->StateNext == STATE_NEXT) {
		while (NULL != (pXF->XMLENext = pXF->XMLENext->NextSiblingElement("n"))) {
			xan = pXF->XMLENext->FindAttribute("n");
			xav = pXF->XMLENext->FindAttribute("v");
			if (xan != NULL && xav != NULL) {
				strcpy(ParamName, xan->Value());
				strcpy(Value, xav->Value());
				return 1;
			}
		}
	}
	pXF->StateNext = STATE_STOP;
	delete pXF; *ppXF = 0;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, char *Value, char *Param2Name, char *Value2) {
	const XML::XMLAttribute *xan, *xav;
	PXMLFind pXF;
	char keyname[256];

	pXF = *ppXF;
	if (pXF == NULL || pXF->StateNext == STATE_STOP || !ParamName || !Value) {
		return 0;
	}

	if (pXF->StateNext == STATE_START) {
		pXF->XMLENext = pXF->XMLENext->FirstChildElement(ElemName);
		if (pXF->XMLENext == NULL) {
			pXF->StateNext = STATE_STOP;
			delete pXF; *ppXF = 0;
			return 0;
		}
		pXF->StateNext = STATE_NEXT;
		xan = pXF->XMLENext->FindAttribute(ParamName);
		if (Param2Name != 0) {
			xav = pXF->XMLENext->FindAttribute(Param2Name);
		}
		if (xan != NULL) {
			strcpy(Value, xan->Value());
			if (Param2Name != 0 && xav != NULL) {
				strcpy(Value2, xav->Value());
			}
			return 1;
		}
	}
	if (pXF->StateNext == STATE_NEXT) {
		while (NULL != (pXF->XMLENext = pXF->XMLENext->NextSiblingElement(ElemName))) {
			xan = pXF->XMLENext->FindAttribute(ParamName);
			if (Param2Name != 0) {
				xav = pXF->XMLENext->FindAttribute(Param2Name);
			}
			if (xan != NULL ) {
				strcpy(Value, xan->Value());
				if (Param2Name != 0 && xav != NULL) {
					strcpy(Value2, xav->Value());
				}
				return 1;
			}
		}
	}
	pXF->StateNext = STATE_STOP;
	delete pXF; *ppXF = 0;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, char *Value, char *Param2Name, int *Value2) {
char ParamValue2[256];
INT64 vi64;

	if (!GetNextStr(ppXF, ElemName, ParamName, Value, Param2Name, ParamValue2)) {
		return 0;
	}
	vi64 = strtoll(ParamValue2, NULL, 0);
	*Value2 = (int)vi64;
	return 1;
}

																													 ///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, char *Value, char *Param2Name, INT64 *Value2) {
char ParamValue2[256];

	if (!GetNextStr(ppXF, ElemName, ParamName, Value, Param2Name, ParamValue2)) {
		return 0;
	}
	*Value2 = strtoll(ParamValue2, NULL, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, INT64 *Value, char *Param2Name, INT64 *Value2) {
char ParamValue[256];
char ParamValue2[256];

	if (!GetNextStr(ppXF, ElemName, ParamName, ParamValue, Param2Name, ParamValue2)) {
		return 0;
	}
	*Value = strtoll(ParamValue, NULL, 0);
	*Value2 = strtoll(ParamValue2, NULL, 0);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextParam(PXMLFind *ppXF, char *ParamName, int *IVal) {
char Value[256];
	
	int ret = GetNextParam(ppXF, ParamName, Value);
	*IVal = atoi(Value);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetNextParam(PXMLFind *ppXF, char *ParamName, UINT64 *IVal) {
char Value[256];

	int ret = GetNextParam(ppXF, ParamName, Value);
	*IVal = strtoull(Value, NULL, 10);
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int nitaxml::GetStrAttr(PXMLFind *ppXF, char *AttrName, char *Value, char *Attr2Name, char *Value2) {
	const XML::XMLAttribute *xav, *xav2;
	PXMLFind pXF;
	char keyname[256];

	pXF = *ppXF;
	if (pXF == NULL || pXF->StateNext != STATE_NEXT || !AttrName || !Value) {
		return 0;
	}
		xav = pXF->XMLENext->FindAttribute(AttrName);
		if (Attr2Name != 0) {
			xav2 = pXF->XMLENext->FindAttribute(Attr2Name);
		}
		if (xav != NULL) {
			strcpy(Value, xav->Value());
			if (Attr2Name != 0 && xav2 != NULL) {
				strcpy(Value2, xav2->Value());
			}
			return 1;
		}
	return 0;
}