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

#include "windows.h"
#include "TCHAR.H"
#include "Dbghelp.h"
#include "Wmistr.h"
#include "Evntrace.h"
#include "TraceProvider.h"

struct STraceEvenTracingNormalizedData
{
    bool	bFormatted;

    DWORD  dwSequenceIndex;
    GUID   sourceFileGUID;
    WORD   sourceTraceIndex;
    int    nParamBuffer;
    BYTE  *pParamBuffer;

    DWORD	dwProcessId;
    DWORD	dwThreadId;

    SYSTEMTIME systemTime;
    LARGE_INTEGER timeStamp;

    // Only available when formatted.

    std::tstring    sText;
    std::tstring    sSource;
    std::tstring    sSourceFile;
    std::tstring    sFunction;
    std::tstring    sComponent;
    std::tstring    sLevel;
    std::tstring    sFlag;
    DWORD	dwLine;

    STraceEvenTracingNormalizedData()
    {
        bFormatted=false;
        dwLine=0;
        dwSequenceIndex=0;
        dwProcessId=0;
        dwThreadId=0;
        memset(&systemTime,0,sizeof(systemTime));
        timeStamp.QuadPart=0;
        sourceFileGUID=GUID_NULL;
        sourceTraceIndex=0;
        nParamBuffer=0;
        pParamBuffer=NULL;
    }

    ~STraceEvenTracingNormalizedData()
    {
        delete [] pParamBuffer;
        pParamBuffer=NULL;
    }
};

class ITraceEvents
{
public:
    virtual void ProcessTrace(STraceEvenTracingNormalizedData *pTraceData)=0;
    virtual void ProcessUnknownTrace(STraceEvenTracingNormalizedData *pTraceData)=0;
};

enum eTraceControllerSessionType
{
    eTraceControllerSessionType_None,
    eTraceControllerSessionType_RealTime,
    eTraceControllerSessionType_ReadLog,
    eTraceControllerSessionType_CreateLog
};

class CTraceController
{
    struct STraceProviderData
    {
        CTraceProvider   *pProvider;
        DWORD			  dwLevel;
        DWORD			  dwFlags;
        bool			  bEnabled;

        STraceProviderData(){pProvider=NULL;dwFlags=0;dwLevel=0;bEnabled=false;}
    };

    HANDLE m_hMutex;

    std::map<STraceFormatEntryKey,STraceFormatEntry *> m_FormatEntries;
    std::map<GUID,STraceProviderData,CGUIDComparer> m_Providers;

    ITraceEvents *m_piEventCallback;
    
    bool m_bPaused;

    std::tstring    m_sLogFileName;
    std::tstring    m_sSessionName;
    TRACEHANDLE     m_hSession;
    TRACEHANDLE     m_hConsumerSession;
    EVENT_TRACE_LOGFILE m_ConsumerProperties;
    HANDLE  m_hConsumerThread;

    EVENT_TRACE_PROPERTIES *m_pSessionProperties;
    
    LARGE_INTEGER m_liPerformanceFrequency;
    LARGE_INTEGER m_liReferenceCounter;
    FILETIME	  m_liReferenceFileTime;
    bool		  m_bQPCTimeStamp;

    eTraceControllerSessionType m_eSessionType;

    static VOID WINAPI	EventCallback(PEVENT_TRACE pEvent);
    static DWORD WINAPI ConsumerThread(LPVOID lpThreadParameter);

    void InitializeRealTimeSession(const TCHAR *pSessionName);
    void InitializeCreateLog(const TCHAR *pSessionName,const TCHAR *pFileName);

    bool Format(STraceEvenTracingNormalizedData *pData);

    void RemoveProviderFormatEntries(CTraceProvider *pProvider);
    void AddProviderFormatEntries(CTraceProvider *pProvider);

public:

    eTraceControllerSessionType GetSessionType();

    ULONG OpenLog(const TCHAR *pLogFile,ITraceEvents *piEvents);
    ULONG CreateLog(const TCHAR *pLogFile,ITraceEvents *piEvents);
    ULONG StartRealTime(TCHAR *pSessionName,ITraceEvents *piEvents);
    void  Stop();

    void  Pause(bool bPause);
    bool  IsPaused();

    void FlushTraces();

    bool AddProvider(CTraceProvider *pProvider,DWORD dwFlags,DWORD dwLevel);
    void RemoveProvider(CTraceProvider *pProvider);
    void ReplaceProvider(CTraceProvider *pOldProvider, CTraceProvider *pNewProvider);

    DWORD	GetProviderFlags(CTraceProvider *pProvider);
    void	SetProviderFlags(CTraceProvider *pProvider,DWORD dwFlags);
    DWORD	GetProviderLevel(CTraceProvider *pProvider);
    void	SetProviderLevel(CTraceProvider *pProvider,DWORD dwLevel);

    bool	IsProviderEnabled(CTraceProvider *pProvider);
    void	EnableProvider(CTraceProvider *pProvider,bool bEnabled);

    bool	FormatTrace(STraceEvenTracingNormalizedData *pData);

    std::tstring GetFileName();

    CTraceController(void);
    ~CTraceController(void);
};
