//  ETViewer, an easy to use ETW / WPP trace viewer
//  Copyright (C) 2011  Javier Martin Garcia (javiermartingarcia@gmail.com)
//  
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////////////
//
// Project HomePage: http://etviewer.codeplex.com/
// For any comment or question, mail to: etviewer@gmail.com
//
////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <windows.h>
#include <set>
#include <map>
#include "tstring.h"
#include <vector>
#include <deque>
#include <dbghelp.h>

#define TRACE_LEVEL_NONE        0   // Tracing is not on
#define TRACE_LEVEL_FATAL       1   // Abnormal exit or termination
#define TRACE_LEVEL_ERROR       2   // Severe errors that need logging
#define TRACE_LEVEL_WARNING     3   // Warnings such as allocation failure
#define TRACE_LEVEL_INFORMATION 4   // Includes non-error cases(for example, Entry-Exit)
#define TRACE_LEVEL_VERBOSE     5   // Detailed traces from intermediate steps
#define TRACE_LEVEL_RESERVED6   6
#define TRACE_LEVEL_RESERVED7   7
#define TRACE_LEVEL_RESERVED8   8
#define TRACE_LEVEL_RESERVED9   9

class CGUIDComparer
{
public :

    bool operator ()(const GUID &ref1, const GUID &ref2) const 
    {
        return memcmp(&ref1,&ref2,sizeof(GUID))<0;
    }

};

enum eTraceFormatElementType
{
    eTraceFormatElementType_Unknown,
    eTraceFormatElementType_ConstantString,
    eTraceFormatElementType_Byte,
    eTraceFormatElementType_Word,
    eTraceFormatElementType_DWord,
    eTraceFormatElementType_Float,
    eTraceFormatElementType_Double,
    eTraceFormatElementType_Quad,
    eTraceFormatElementType_Pointer,
    eTraceFormatElementType_QuadPointer,
    eTraceFormatElementType_AnsiString,
    eTraceFormatElementType_UnicodeString
};



struct STraceFormatElement
{
    TCHAR					*pFormatString;
    eTraceFormatElementType	 eType;

    STraceFormatElement():eType(eTraceFormatElementType_Unknown),pFormatString(NULL){}
};

struct STraceFormatParam
{
    eTraceFormatElementType	 eType;

    STraceFormatParam():eType(eTraceFormatElementType_Unknown){}
};
class CTraceSourceFile;
class CTraceProvider;

struct STraceFormatEntry
{
    GUID						        m_SourceFileGUID;
    DWORD						        m_dwIndex;
    DWORD						        m_dwLine;
    std::vector<STraceFormatElement>    m_vElements;
    std::vector<STraceFormatParam>	    m_vParams;
    std::tstring						m_sFunction;
    std::tstring						m_sLevel;
    std::tstring						m_sFlag;
    CTraceSourceFile			        *m_pSourceFile;
    CTraceProvider				        *m_pProvider;

    void InitializeFromBuffer(TCHAR *pBuffer);

    STraceFormatEntry();
    ~STraceFormatEntry();
};

struct STraceFormatEntryKey
{
    GUID	m_SourceFileGUID;
    DWORD	m_dwIndex;

    bool operator <(const STraceFormatEntryKey& other) const
    {
        int nComp=memcmp(&m_SourceFileGUID,&other.m_SourceFileGUID,sizeof(GUID));
        if(nComp<0){return true;}
        if(nComp>0){return false;}
        return m_dwIndex<other.m_dwIndex;
    }

    STraceFormatEntryKey(STraceFormatEntry *pValue){m_SourceFileGUID=pValue->m_SourceFileGUID;m_dwIndex=pValue->m_dwIndex;}
    STraceFormatEntryKey(GUID &sourceFileGUID,DWORD dwIndex){m_SourceFileGUID=sourceFileGUID;m_dwIndex=dwIndex;}
};

class CTraceProvider;

class CTraceSourceFile
{
    GUID	m_SourceFileGUID;
    LPCTSTR	m_SourceFileName;
    TCHAR	m_SourceFileNameWithPath[MAX_PATH];

    CTraceProvider *m_pProvider;

public:

    GUID			GetGUID();
    std::tstring	GetFileName();
    std::tstring    GetFileNameWithPath();

    CTraceProvider *GetProvider();
    void			SetProvider(CTraceProvider *pProvider);

    CTraceSourceFile(GUID sourceFileGUID,const TCHAR *pSourceFile);
    ~CTraceSourceFile(void);
};


class CTraceProvider
{
    GUID						    m_ProviderGUID;
    std::map<std::tstring,DWORD>	m_TraceFlags;
    std::tstring					m_sComponentName;
    std::set<std::tstring>          m_sFileList;
    DWORD						    m_dwAllSupportedFlagsMask;

    std::vector<STraceFormatEntry *>				m_FormatEntries;
    std::map<GUID,CTraceSourceFile *,CGUIDComparer>	m_sSourceFiles;

    void FreeAll();

public:

    GUID	GetGUID();
    void	SetGUID(GUID guid);

    DWORD	GetAllSupportedFlagsMask();
    void	GetSupportedFlags(std::map<std::tstring,DWORD> *pmFlags);
    void	AddSupportedFlag(std::tstring sName,DWORD dwValue);
    DWORD	GetSupportedFlagValue(std::tstring sName);

    std::tstring	GetComponentName();
    std::set<std::tstring>	GetFileList();

    void                AddFileName(std::tstring sFileName);
    void				AddSourceFile(CTraceSourceFile *pSourceFile);
    void 				GetSourceFiles(std::vector<CTraceSourceFile*> *pvSources);
    CTraceSourceFile*	GetSourceFile(GUID sourceGUID);

    void				AddFormatEntry(STraceFormatEntry *pFormatEntry);
    void 				GetFormatEntries(std::vector<STraceFormatEntry*> *pvFormatEntries);

    CTraceProvider(std::tstring sComponentName,std::tstring sFileName);
    ~CTraceProvider(void);
};

enum eTraceReaderError
{
    eTraceReaderError_Success,
    eTraceReaderError_Generic,
    eTraceReaderError_PDBNotFound,
    eTraceReaderError_NoProviderFoundInPDB,
    eTraceReaderError_FailedToEnumerateSymbols,
    eTraceReaderError_DBGHelpNotFound,
    eTraceReaderError_DBGHelpWrongVersion,
    eTraceReaderError_SymEngineLoadModule,
    eTraceReaderError_SymEngineInitFailed,
    eTraceReaderError_NoMemory,
};

class CTraceReader
{
protected:
    std::deque<STraceFormatEntry *>                     m_sTempFormatEntries;
    std::map<GUID, CTraceSourceFile*, CGUIDComparer>    m_sTempSourceFiles;
    std::map<std::tstring, CTraceProvider*>             m_sTempProviders;
    std::tstring                                        m_sFileName;

    void				 AddSourceFile(GUID guid, std::tstring sFileName);
    CTraceSourceFile	*FindSourceFile(GUID sourceFile);
    CTraceProvider		*FindOrCreateProvider(std::tstring sProviderName);
    CTraceProvider		*FindProviderByFlag(std::tstring sFlagName);

};

class CTracePDBReader : protected CTraceReader
{
public:
     eTraceReaderError LoadFromPDB(LPCTSTR pPDB,std::vector<CTraceProvider *> *pvProviders);
     static BOOL CALLBACK TypeEnumerationCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);
     static BOOL CALLBACK SymbolEnumerationCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);
};