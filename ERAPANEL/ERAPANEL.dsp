# Microsoft Developer Studio Project File - Name="ERAPANEL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ERAPANEL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ERAPANEL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ERAPANEL.mak" CFG="ERAPANEL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ERAPANEL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ERAPANEL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ERAPANEL - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 psapi.lib userenv.lib /nologo /subsystem:windows /dll /machine:I386 /libpath:"../LIB"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cd            .\Release           	copy      .\erapanel.dll      c:\windows\system32\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ERAPANEL - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x419 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 psapi.lib userenv.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"../LIB"

!ENDIF 

# Begin Target

# Name "ERAPANEL - Win32 Release"
# Name "ERAPANEL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cpau.cpp
# End Source File
# Begin Source File

SOURCE=.\ERAPANEL.cpp
# End Source File
# Begin Source File

SOURCE=.\ERAPANEL.def
# End Source File
# Begin Source File

SOURCE=.\ERAPANEL.odl
# End Source File
# Begin Source File

SOURCE=.\ERAPANEL.rc
# End Source File
# Begin Source File

SOURCE=.\Panel.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcCmdFunc.cpp
# End Source File
# Begin Source File

SOURCE=.\Repeat.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\VideoRecord.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cpau.h
# End Source File
# Begin Source File

SOURCE=.\ERAPANEL.h
# End Source File
# Begin Source File

SOURCE=.\Panel.h
# End Source File
# Begin Source File

SOURCE=.\ProcCmdFunc.h
# End Source File
# Begin Source File

SOURCE=.\psapi.h
# End Source File
# Begin Source File

SOURCE=.\Repeat.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Camera.ico
# End Source File
# Begin Source File

SOURCE=.\bmp\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\ERAPANEL.rc2
# End Source File
# Begin Source File

SOURCE=.\res\ERARECP.ico
# End Source File
# Begin Source File

SOURCE=.\bmp\exit_1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\exit_2.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Granite.ico
# End Source File
# Begin Source File

SOURCE=.\bmp\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\bmp\Mode_0.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Mode_1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Mode_2.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Mode_3.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Mono_1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Mono_2.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Mono_3.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Mono_4.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Off_1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Off_2.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Pause_0.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Pause_1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Pause_2.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Pause_3.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Pause_4.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Pause_5.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Rec_0.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Rec_1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Rec_2.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Rec_3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Sound.ico
# End Source File
# Begin Source File

SOURCE=.\bmp\Stop_0.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Stop_1.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp\Stop_2.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
