#ifndef nitax_INCLUDED
#define nitax_INCLUDED

//typedef int64_t  INT64, *PINT64;
//typedef unsigned long long	UINT64;

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <fstream> 
#include "tinyxml2.h"

namespace XML = tinyxml2;
#define MAXPARLEN 256
#define MAXKEYSLEN 10000
//SearchFrom:
#define FROM_ROOT		0
#define FROM_ROOT_END	0
#define FROM_CUR		1
#define FROM_CUR_END	1
#define FROM_ROOT_BEGIN	2 //for CreateParam func.
#define FROM_CUR_BEGIN	3 //for CreateParam func.
//SearchFrom

//where to insert param
#define INS_BEFORE	0
#define INS_AFTER	1
//where to insert param

//TypeElement:
#define NITA_KEY   1
#define NITA_PARAM 2
#define NITA_UNKNOWN -1
//TypeElement
//GetNextSubKey SETS&STATES:
#define GNS_NEXT 0
#define GNS_LOAD 1
#define GNS_STOP 2
//GetNextSubKey SETS&STATES
//StateNextKey & StateNextPar:
#define STATE_START	0
#define STATE_NEXT	1
#define STATE_STOP	2
//StateNextKey & StateNextPar

#pragma once

typedef struct _XMLFind {
	XML::XMLElement *XMLENext;
	int StateNext;
} XMLFind, *PXMLFind;

class nitaxml
{
public:
	BOOL IsXMLOpen;
	char XMLFullName[256];
	XML::XMLDocument *xmldoc;
	XML::XMLError xmlerr;
	XML::XMLElement *XMLE[1000];
	XML::XMLElement *XMLERoot;
	XML::XMLElement *XMLECur;
	XML::XMLElement *XMLECurSav;
	XML::XMLElement *XMLEPar;
	nitaxml();
	~nitaxml();
	int XMLOpen(char *XMLName, bool CreateIfNotExists = false);
	int XMLOpen2(char *XMLName, char *FindElem); //Not NITA XML-format
	int XMLNew(char *XMLName);
	int XMLClose();
	int XMLSave(char *XMLName);
	int XMLSave();
	int FullGetParam(char *ParamName, char *ParamValue, int *TypeElement);
	int FullGetParam(char *ParamName, int *IntValue, int *TypeElement);
	int FullGetParam(char *ParamName, INT64 *IntValue, int *TypeElement);
	int FullGetParam(char *ParamName, double *dValue, int *TypeElement);
	int GetParam(char *ParamName, char *ParamValue, int *TypeElement, int SymLimit = 0);
	int GetParam(char *ParamName, int *IntValue, int *TypeElement);
	int GetParam(char *ParamName, INT64 *IntValue, int *TypeElement);
	int GetParam(char *ParamName, UINT64 *IntValue, int *TypeElement);
	int GetParam(char *ParamName, double *dValue, int *TypeElement);
	int GetAttr(char *ParamName, char *AttrName, char *AttrValue, int From = FROM_CUR);
	int GetAttr(char *ParamName, char *AttrName, INT64 *AttrValue, int From = FROM_CUR);
	int GetAttr(char *ParamName, char *AttrName, int *AttrValue, int From = FROM_CUR);
	int FullSetParam(char *ParamName, char *ParamValue);
	int FullSetParam(char *ParamName, INT64 IntValue);
	int SetParam(char *ParamName, char *ParamValue, int From = FROM_CUR);
	int SetParam(char *ParamName, INT64 IntValue, int From = FROM_CUR);
	int SetParam(char *ParamName, UINT64 IntValue, int From = FROM_CUR);
	int SetCurrentKey(char *KeyName);
	int CreateParam(char *ParamName, char *ParamValue, int From = FROM_CUR);
	int CreateParam(char *ParamName, INT64 IntValue, int From = FROM_CUR);
	int CreateParam(char *ParamName, UINT64 IntValue, int From = FROM_CUR);
	int InsertParam(char *ParamName, char *ParamValue, char *ReferParam, int Where = INS_BEFORE, int From = FROM_CUR);
	int SetAttr(char *ParamName, char *AttrName, char *AttrValue, int From = FROM_CUR);
	int SetAttr(char *ParamName, char *AttrName, INT64 AttrValue, int From = FROM_CUR);
	int CreateKey(char *KeyName, BOOL SetCurKey = FALSE);
	int DeleteParam(char *ParamName, int From = FROM_CUR);
	int DeleteKey(char *KeyName, int From = FROM_CUR);
	int DeleteAttr(char *ParamName, char *AttrName, int From = FROM_CUR);
	PXMLFind GetNextInit(char *ParKeyName);
	int GetNextKey(PXMLFind *ppXF, char *KeyName);
	int GetNextParam(PXMLFind *ppXF, char *ParamName, char *Value);
	int GetNextParam(PXMLFind *ppXF, char *ParamName, int *IVal);
	int GetNextParam(PXMLFind *ppXF, char *ParamName, UINT64 *IVal);
	int GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, char *Value, char *Param2Name = 0, char *Value2 = 0); //Not NITA XML-format
	int GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, char *Value, char *Param2Name = 0, int *Value2 = 0); //Not NITA XML-format
	int GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, char *Value, char *Param2Name = 0, INT64 *Value2 = 0); //Not NITA XML-format
	int GetNextStr(PXMLFind *ppXF, char *ElemName, char *ParamName, INT64 *Value, char *Param2Name = 0, INT64 *Value2 = 0); //Not NITA XML-format
	int GetStrAttr(PXMLFind *ppXF, char *AttrName, char *Value, char *Attr2Name = 0, char *Value2 = 0); //Not NITA XML-format
	
private:
	char FullName[MAXPARLEN];
	char *KeysSet[MAXKEYSLEN];
	//int StateNextKey, StateNextParam;
	//XML::XMLElement *XMLENextKey;
	//XML::XMLElement *XMLENextParam;
	XML::XMLElement* FindXMLElement(char *ParamName, int SearchFrom, int *TypeElement, int Back = 0);
	XML::XMLElement* GetXMLElement(XML::XMLElement *ParentEl, char **PKeysSet, int Back = 0);
	/** Разбор полной строки параметра в приватный набор ключей KeysSet[MAXKEYSLEN]	*/
	int KeysParse(char *ParamName);
	int GetNextSubKey(char *KeyName, char *ShortSubKey, char *FullSubKey, int Set = GNS_NEXT);
	
	
	};

#endif // nitax_INCLUDED
