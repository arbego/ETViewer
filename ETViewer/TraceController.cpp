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

#include ".\tracecontroller.h"
#include <versionhelpers.h>

using namespace std;

struct S_NEW_FORMAT_MOF_DATA
{
    DWORD sequenceId;
    GUID  sourceFileGUID;
    FILETIME timeStamp;
    DWORD threadId;
    DWORD processId;
    BYTE  *params;
};

struct S_OLD_FORMAT_MOF_DATA
{
    WORD  traceIndex;
    BYTE  *params;
};

DWORD g_TraceControllerTLS=TlsAlloc();

CTraceController::CTraceController(void)
{
    m_eSessionType=eTraceControllerSessionType_None;
    m_hMutex=CreateMutex(NULL,FALSE,NULL);
    m_piEventCallback=NULL;
    m_hSession=NULL;
    m_pSessionProperties=NULL;
    m_bPaused=false;
    m_liReferenceFileTime.dwHighDateTime=0;
    m_liReferenceFileTime.dwLowDateTime=0;
    m_liPerformanceFrequency.QuadPart=0;
    m_liReferenceCounter.QuadPart=0;

    m_bQPCTimeStamp=false;
    m_hConsumerThread=NULL;
    m_hConsumerSession=(TRACEHANDLE)INVALID_HANDLE_VALUE;
    memset(&m_ConsumerProperties,0,sizeof(m_ConsumerProperties));

}

CTraceController::~CTraceController(void)
{
    if(m_hMutex){CloseHandle(m_hMutex);m_hMutex=NULL;}
}

bool CTraceController::AddProvider(CTraceProvider *pProvider,DWORD dwFlags,DWORD dwLevel)
{
    bool bNewProvider=false;

    WaitForSingleObject(m_hMutex,INFINITE);
    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());

    AddProviderFormatEntries(pProvider);

    if(i==m_Providers.end())
    {
        bNewProvider = true;
        STraceProviderData data;
        data.pProvider=pProvider;
        data.dwLevel=dwLevel;
        data.dwFlags=dwFlags;
        data.bEnabled=true;
        m_Providers[pProvider->GetGUID()]=data;

        if(m_hSession)
        {
            GUID providerGUID=pProvider->GetGUID();
            EnableTrace(TRUE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }
    }
    else
    {
        for(const auto curFile : pProvider->GetFileList())
        {
            m_Providers[pProvider->GetGUID()].pProvider->AddFileName(curFile);
        }
    }

    ReleaseMutex(m_hMutex);
    return bNewProvider;
}

void CTraceController::RemoveProvider(CTraceProvider *pProvider)
{
    WaitForSingleObject(m_hMutex,INFINITE);

    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());

    if(i!=m_Providers.end())
    {
        STraceProviderData data=i->second;

        RemoveProviderFormatEntries(pProvider);

        m_Providers.erase(i);

        if(m_hSession)
        {
            GUID providerGUID=pProvider->GetGUID();
            EnableTrace(FALSE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }
    }
    ReleaseMutex(m_hMutex);
}

void CTraceController::AddProviderFormatEntries(CTraceProvider *pProvider)
{
    vector<STraceFormatEntry *> lEntries;
    pProvider->GetFormatEntries(&lEntries);

    for(unsigned x=0;x<lEntries.size();x++)
    {
        STraceFormatEntry *pEntry=lEntries[x];
        m_FormatEntries[STraceFormatEntryKey(pEntry)]=pEntry;
    }
}

void CTraceController::RemoveProviderFormatEntries(CTraceProvider *pProvider)
{
    map<STraceFormatEntryKey,STraceFormatEntry *>::iterator i;

    for(i=m_FormatEntries.begin();i!=m_FormatEntries.end();)
    {
        STraceFormatEntry *pEntry=i->second;
        if(pEntry->m_pProvider->GetGUID()==pProvider->GetGUID())
        {
            i=m_FormatEntries.erase(i);
        }
        else
        {
            i++;
        }
    }
}

void CTraceController::ReplaceProvider(CTraceProvider *pOldProvider, CTraceProvider *pNewProvider)
{	
    if(pOldProvider->GetGUID()!=pNewProvider->GetGUID()){return;}

    WaitForSingleObject(m_hMutex,INFINITE);
    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pOldProvider->GetGUID());

    if(i!=m_Providers.end())
    {
        RemoveProviderFormatEntries(pOldProvider);
        i->second.pProvider=pNewProvider;
        AddProviderFormatEntries(pNewProvider);
    }

    ReleaseMutex(m_hMutex);
}

bool CTraceController::FormatTrace(STraceEvenTracingNormalizedData *pData)
{
    return Format(pData);
}

bool CTraceController::Format(STraceEvenTracingNormalizedData *pData)
{
    TCHAR sTraceText[1024];

    WaitForSingleObject(m_hMutex,INFINITE);

    STraceFormatEntryKey key(pData->sourceFileGUID,pData->sourceTraceIndex);

    map<STraceFormatEntryKey,STraceFormatEntry *>::iterator i=m_FormatEntries.find(key);
    if(i==m_FormatEntries.end())
    {
        ReleaseMutex(m_hMutex);
        return false;
    }

    STraceFormatEntry *pFormatEntry=i->second;
    CTraceSourceFile *pSourceFile=pFormatEntry->m_pSourceFile;

    UCHAR *pCurrentParam=pData->pParamBuffer;
    int currentLen=0;
    TCHAR *pTempBuffer=new TCHAR [_countof(sTraceText)];
    memset(pTempBuffer,0,_countof(sTraceText)*sizeof(TCHAR));

    for(DWORD x=0;x<pFormatEntry->m_vElements.size();x++)
    {
        DWORD dwValue=0;
        SHORT wValue=0;
        UCHAR cValue=0;
        float fValue=0;
        double dValue=0;
        LARGE_INTEGER qValue={0};

        STraceFormatElement *pElement=&pFormatEntry->m_vElements[x];
        switch(pElement->eType)
        {
        case eTraceFormatElementType_Byte:
            cValue=*(UCHAR*)pCurrentParam;
            pCurrentParam+=sizeof(UCHAR);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText)-currentLen, pElement->pFormatString, cValue);
            break;

        case eTraceFormatElementType_Word:
            wValue=*(SHORT*)pCurrentParam;
            pCurrentParam+=sizeof(SHORT);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, wValue);
            break;

        case eTraceFormatElementType_DWord:
            dwValue=*(DWORD*)pCurrentParam;
            pCurrentParam+=sizeof(DWORD);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, dwValue);
            break;

        case eTraceFormatElementType_Float:
            fValue=*(float*)pCurrentParam;
            pCurrentParam+=sizeof(float);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, fValue);
            break;

        case eTraceFormatElementType_Double:
            dValue=*(double*)pCurrentParam;
            pCurrentParam+=sizeof(double);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, dValue);
            break;

        case eTraceFormatElementType_Quad:
            qValue=*(LARGE_INTEGER*)pCurrentParam;
            pCurrentParam+=sizeof(LARGE_INTEGER);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, qValue);
            break;

        case eTraceFormatElementType_Pointer:
            dwValue=*(DWORD*)pCurrentParam;
            pCurrentParam+=sizeof(DWORD);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, dwValue);
            break;

        case eTraceFormatElementType_QuadPointer:
            qValue=*(LARGE_INTEGER*)pCurrentParam;
            pCurrentParam+=sizeof(LARGE_INTEGER);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, qValue);
            break;
        case eTraceFormatElementType_Unknown:
            qValue=*(LARGE_INTEGER*)pCurrentParam;
            pCurrentParam+=sizeof(LARGE_INTEGER);
            currentLen += _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, qValue);
            break;
        case eTraceFormatElementType_AnsiString:
            {
                DWORD dwBytes = _stprintf_s(sTraceText + currentLen, _countof(sTraceText) - currentLen, pElement->pFormatString, pCurrentParam);
                currentLen+=dwBytes;
                pCurrentParam+=_tcslen((TCHAR*)pCurrentParam)+1;
            }
            break;
        case eTraceFormatElementType_UnicodeString:
            {
                _stprintf_s(pTempBuffer, _countof(sTraceText), _T("%ws"), (WCHAR*)pCurrentParam);
                DWORD dwBytes=_stprintf_s(sTraceText+currentLen,_countof(sTraceText)-currentLen,pElement->pFormatString,pTempBuffer);
                currentLen+=dwBytes;
                pCurrentParam+=(wcslen((WCHAR*)pCurrentParam)+1)*2;
            }
            break;
        case eTraceFormatElementType_ConstantString:
            _tcscpy_s(sTraceText+currentLen,_countof(sTraceText)-currentLen,pElement->pFormatString);
            currentLen+=(int)_tcslen(sTraceText+currentLen);
            break;
        }
    }
    
    sTraceText[currentLen]=0;

    pData->bFormatted=true;
    pData->dwLine=pFormatEntry->m_dwLine;
    pData->sText=sTraceText;
    pData->sSource=pSourceFile->GetFileNameWithPath();
    pData->sSourceFile=pSourceFile->GetFileName();
    pData->sComponent=pSourceFile->GetProvider()->GetComponentName();
    pData->sFunction=pFormatEntry->m_sFunction;
    pData->sLevel=pFormatEntry->m_sLevel;
    pData->sFlag=pFormatEntry->m_sFlag;

    ReleaseMutex(m_hMutex);
    delete [] pTempBuffer;
    return true;
}

// {68FDD900-4A3E-11D1-84F4-0000F80464E3} ETL file header event GUID (GUID of the first event received when reading an ETL file. It containes header information)
static const GUID GUID_EventTraceEvent = { 0x68fdd900, 0x4a3e, 0x11d1, { 0x84, 0xf4, 0, 0, 0xf8, 0x04, 0x64, 0xe3 } };

VOID WINAPI CTraceController::EventCallback(PEVENT_TRACE pEvent)
{
    LARGE_INTEGER traceTimeStamp={0};

    CTraceController *pController=(CTraceController *)TlsGetValue(g_TraceControllerTLS);
    STraceEvenTracingNormalizedData eventData;
    if(pEvent->MofData==NULL){return;}
    if(pEvent->Header.Guid==GUID_EventTraceEvent){return;}

    if(pEvent->Header.HeaderType!=0 || pEvent->Header.Guid==GUID_NULL) // new WDK format.
    {
        S_NEW_FORMAT_MOF_DATA *pData=(S_NEW_FORMAT_MOF_DATA *)pEvent->MofData;
        eventData.dwProcessId=pData->processId;
        eventData.dwThreadId=pData->threadId;
        eventData.dwSequenceIndex=pData->sequenceId;
        eventData.sourceFileGUID=pData->sourceFileGUID;
        eventData.sourceTraceIndex=LOWORD(pEvent->Header.Version);
        memcpy(&traceTimeStamp,&pData->timeStamp,sizeof(LARGE_INTEGER));
        eventData.nParamBuffer = pEvent->MofLength;// -sizeof(S_NEW_FORMAT_MOF_DATA);
        if(eventData.nParamBuffer)
        {
            eventData.pParamBuffer=(BYTE*)&pData->params;
        }
    }
    else// old WDK format.
    {
        S_OLD_FORMAT_MOF_DATA *pData=(S_OLD_FORMAT_MOF_DATA *)pEvent->MofData;
        eventData.dwProcessId=pEvent->Header.ProcessId;
        eventData.dwThreadId=pEvent->Header.ThreadId;
        eventData.dwSequenceIndex=0;
        eventData.sourceFileGUID=pEvent->Header.Guid;
        memcpy(&traceTimeStamp,&pEvent->Header.TimeStamp,sizeof(LARGE_INTEGER));
        eventData.sourceTraceIndex=pData->traceIndex;
        eventData.nParamBuffer = pEvent->MofLength;//-sizeof(S_OLD_FORMAT_MOF_DATA);
        if(eventData.nParamBuffer)
        {
            eventData.pParamBuffer=(BYTE*)&pData->params;
        }
    }

    FILETIME	tempFileTime,localFileTime;

    if(!pController->m_bQPCTimeStamp)
    {
        // Processing the time as a FileTime

        tempFileTime.dwHighDateTime=traceTimeStamp.HighPart;
        tempFileTime.dwLowDateTime=traceTimeStamp.LowPart;

        LARGE_INTEGER liTemp;
        liTemp.HighPart=pController->m_liReferenceFileTime.dwHighDateTime;
        liTemp.LowPart=pController->m_liReferenceFileTime.dwLowDateTime;
        traceTimeStamp.QuadPart-=liTemp.QuadPart;
    }
    else
    {
        // Processing the time as Performance Counter
        traceTimeStamp.QuadPart=((traceTimeStamp.QuadPart*10000000)/pController->m_liPerformanceFrequency.QuadPart);

        if(pController->m_eSessionType==eTraceControllerSessionType_ReadLog)
        {
            // Initialization of the reference relative time (timestamp) for ETL files (first received event time)
            if(pController->m_liReferenceCounter.QuadPart==0)
            {
                pController->m_liReferenceCounter.QuadPart=traceTimeStamp.QuadPart;
            }
        }

        traceTimeStamp.QuadPart-=pController->m_liReferenceCounter.QuadPart;

        LARGE_INTEGER liTemp;
        liTemp.HighPart=pController->m_liReferenceFileTime.dwHighDateTime;
        liTemp.LowPart=pController->m_liReferenceFileTime.dwLowDateTime;
        liTemp.QuadPart+=traceTimeStamp.QuadPart;

        tempFileTime.dwHighDateTime=liTemp.HighPart;
        tempFileTime.dwLowDateTime=liTemp.LowPart;
    }

    FileTimeToLocalFileTime(&tempFileTime,&localFileTime);
    FileTimeToSystemTime(&localFileTime,&eventData.systemTime);

    eventData.timeStamp=traceTimeStamp;

    CTraceSourceFile *pSourceFile=NULL;
    STraceFormatEntry *pFormatEntry=NULL;

    if(!pController->Format(&eventData))
    {
        WCHAR CLSID[100]={0};
        StringFromGUID2(eventData.sourceFileGUID,CLSID,sizeof(CLSID)/2);
        printf("Failed to format trace, header type %x, source id %ws\n",pEvent->Header.HeaderType,CLSID);

        if(pController->m_piEventCallback)
        {
            pController->m_piEventCallback->ProcessUnknownTrace(&eventData);	
        }
    }
    else
    {
        if(pController->m_piEventCallback)
        {
            pController->m_piEventCallback->ProcessTrace(&eventData);	
        }
    }
    // This is done to skip buffer deletion on STraceEvenTracingNormalizedData destructor

    eventData.nParamBuffer=0;
    eventData.pParamBuffer=NULL;
}

DWORD WINAPI CTraceController::ConsumerThread(LPVOID lpThreadParameter)
{
    CTraceController *pController=(CTraceController *)lpThreadParameter;

    TlsSetValue(g_TraceControllerTLS,pController);

    if(pController->m_hConsumerSession!=(TRACEHANDLE)INVALID_HANDLE_VALUE)
    {
        ProcessTrace(&pController->m_hConsumerSession,1,NULL,NULL);
    }

    return 0L;
}

void CTraceController::InitializeRealTimeSession(const TCHAR *pSessionName)
{
    ULONG dwBufferLen=sizeof(EVENT_TRACE_PROPERTIES)+(ULONG)((_tcslen(pSessionName)+1)*sizeof(TCHAR));
    m_pSessionProperties=(EVENT_TRACE_PROPERTIES *)new char [dwBufferLen];
    memset(m_pSessionProperties,0,dwBufferLen);
    m_pSessionProperties->Wnode.BufferSize = dwBufferLen;
    CoCreateGuid(&m_pSessionProperties->Wnode.Guid);
    if(IsWindowsVistaOrGreater())
    {
        m_bQPCTimeStamp = true;
        m_pSessionProperties->Wnode.ClientContext = 1; // If OS >= Vista use Performance Counter		
    }
    else
    {
        m_bQPCTimeStamp = false;
        m_pSessionProperties->Wnode.ClientContext = 2; // If OS < Vista use FileTime
    }
    m_pSessionProperties->Wnode.Flags=WNODE_FLAG_TRACED_GUID;
    m_pSessionProperties->FlushTimer=1;
    m_pSessionProperties->LogFileMode=EVENT_TRACE_USE_LOCAL_SEQUENCE|EVENT_TRACE_REAL_TIME_MODE;
    m_pSessionProperties->LoggerNameOffset=sizeof(EVENT_TRACE_PROPERTIES);

    m_ConsumerProperties.LoggerName=const_cast<TCHAR *>(m_sSessionName.c_str());
    m_ConsumerProperties.LogFileMode=EVENT_TRACE_REAL_TIME_MODE;
    m_ConsumerProperties.EventCallback = EventCallback;

    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    SystemTimeToFileTime(&systemTime,&m_liReferenceFileTime);
    QueryPerformanceFrequency(&m_liPerformanceFrequency);
    QueryPerformanceCounter(&m_liReferenceCounter);
    m_liReferenceCounter.QuadPart=(m_liReferenceCounter.QuadPart*10000000)/m_liPerformanceFrequency.QuadPart;
}


ULONG CTraceController::OpenLog(const TCHAR *pLogFileName,ITraceEvents *piEvents)
{
    ULONG errorCode=ERROR_SUCCESS;

    if(m_eSessionType!=eTraceControllerSessionType_None){return ERROR_ALREADY_EXISTS;}

    m_eSessionType=eTraceControllerSessionType_ReadLog;

    m_bQPCTimeStamp=true;
    m_sLogFileName=pLogFileName;
    m_piEventCallback=piEvents;

    m_ConsumerProperties.LogFileName=const_cast<TCHAR *>(m_sLogFileName.c_str());
    m_ConsumerProperties.LogFileMode=0;
    m_ConsumerProperties.EventCallback = EventCallback;


    m_hConsumerSession=OpenTrace(&m_ConsumerProperties);
    if(m_hConsumerSession!=(TRACEHANDLE)INVALID_HANDLE_VALUE)
    {
        DWORD threadId=0;
        
        m_liReferenceFileTime.dwHighDateTime=m_ConsumerProperties.LogfileHeader.StartTime.HighPart;
        m_liReferenceFileTime.dwLowDateTime=m_ConsumerProperties.LogfileHeader.StartTime.LowPart;
        m_liReferenceCounter.QuadPart=0;
        m_liPerformanceFrequency=m_ConsumerProperties.LogfileHeader.PerfFreq;

        m_hConsumerThread=CreateThread(NULL,0,ConsumerThread,this,0,&threadId);
        if((HANDLE)m_hConsumerThread==NULL)
        {	
            errorCode=GetLastError(); 
        }
    }
    else
    {
        errorCode=GetLastError(); 
    }
    if(errorCode!=ERROR_SUCCESS)
    {
        Stop();
    }
    return errorCode;
}

ULONG CTraceController::StartRealTime(TCHAR *pSessionName,ITraceEvents *piEvents)
{
    if(m_eSessionType!=eTraceControllerSessionType_None){return ERROR_ALREADY_EXISTS;}

    m_eSessionType=eTraceControllerSessionType_RealTime;

    m_sSessionName=pSessionName;
    m_piEventCallback=piEvents;

    InitializeRealTimeSession(m_sSessionName.c_str());
    ULONG errorCode=StartTrace(&m_hSession,m_sSessionName.c_str(),m_pSessionProperties);
    if(errorCode==ERROR_ALREADY_EXISTS)
    {
        StopTrace(NULL,m_sSessionName.c_str(),m_pSessionProperties);
        delete m_pSessionProperties;

        InitializeRealTimeSession(m_sSessionName.c_str());
        errorCode=StartTrace(&m_hSession,m_sSessionName.c_str(),m_pSessionProperties);

    }
    if(errorCode==ERROR_SUCCESS)
    {
        map<GUID,STraceProviderData,CGUIDComparer>::iterator i;

        for(i=m_Providers.begin();i!=m_Providers.end();i++)
        {
            STraceProviderData data=i->second;
            GUID providerGUID=data.pProvider->GetGUID();
            EnableTrace(TRUE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }
        m_hConsumerSession=OpenTrace(&m_ConsumerProperties);
        if(m_hConsumerSession!=(TRACEHANDLE)INVALID_HANDLE_VALUE)
        {
            DWORD threadId=0;
            m_hConsumerThread=CreateThread(NULL,0,ConsumerThread,this,0,&threadId);
        }
    }
    else
    {
        Stop();
    }
    return errorCode;
}

void CTraceController::InitializeCreateLog(const TCHAR *pSessionName,const TCHAR *pFileName)
{
    ULONG dwBufferLen=sizeof(EVENT_TRACE_PROPERTIES)+((ULONG)(_tcslen(pSessionName)+1)*sizeof(TCHAR))+1024;
    m_pSessionProperties=(EVENT_TRACE_PROPERTIES *)new char [dwBufferLen];
    memset(m_pSessionProperties,0,sizeof(EVENT_TRACE_PROPERTIES));
    m_pSessionProperties->Wnode.BufferSize = dwBufferLen;
    CoCreateGuid(&m_pSessionProperties->Wnode.Guid);
    if(IsWindowsVistaOrGreater())
    {
        m_bQPCTimeStamp = true;
        m_pSessionProperties->Wnode.ClientContext = 1; // If OS >= Vista use Performance Counter
    }
    else
    {
        m_bQPCTimeStamp = false;
        m_pSessionProperties->Wnode.ClientContext = 2; // If OS < Vista use FileTime
    }
    m_pSessionProperties->Wnode.Flags=WNODE_FLAG_TRACED_GUID;
    m_pSessionProperties->FlushTimer=1;
    m_pSessionProperties->LogFileMode=EVENT_TRACE_FILE_MODE_CIRCULAR|EVENT_TRACE_USE_LOCAL_SEQUENCE|EVENT_TRACE_REAL_TIME_MODE;
    m_pSessionProperties->LoggerNameOffset=sizeof(EVENT_TRACE_PROPERTIES);
    m_pSessionProperties->LogFileNameOffset =sizeof(EVENT_TRACE_PROPERTIES)+(ULONG)_tcslen(pSessionName)+1;
    m_pSessionProperties->MaximumFileSize = 100;

    _tcscpy_s(((TCHAR*)m_pSessionProperties)+m_pSessionProperties->LogFileNameOffset,dwBufferLen-m_pSessionProperties->LogFileNameOffset,pFileName);

    m_ConsumerProperties.LoggerName=const_cast<TCHAR *>(m_sSessionName.c_str());
    m_ConsumerProperties.LogFileMode=EVENT_TRACE_REAL_TIME_MODE;
    m_ConsumerProperties.EventCallback = EventCallback;

    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    SystemTimeToFileTime(&systemTime,&m_liReferenceFileTime);
    QueryPerformanceFrequency(&m_liPerformanceFrequency);
    QueryPerformanceCounter(&m_liReferenceCounter);
    m_liReferenceCounter.QuadPart=(m_liReferenceCounter.QuadPart*10000000)/m_liPerformanceFrequency.QuadPart;
}

ULONG CTraceController::CreateLog(const TCHAR *pFileName,ITraceEvents *piEvents)
{
    if(m_eSessionType!=eTraceControllerSessionType_None){return ERROR_ALREADY_EXISTS;}

    m_sLogFileName=pFileName;
    m_eSessionType=eTraceControllerSessionType_CreateLog;
    m_sSessionName=_T("ETVIEWER LOG TO FILE SESSION");
    m_piEventCallback=piEvents;

    InitializeCreateLog(m_sSessionName.c_str(),pFileName);
    ULONG errorCode=StartTrace(&m_hSession,m_sSessionName.c_str(),m_pSessionProperties);
    if(errorCode==ERROR_ALREADY_EXISTS)
    {
        StopTrace(NULL,m_sSessionName.c_str(),m_pSessionProperties);
        delete m_pSessionProperties;

        InitializeRealTimeSession(m_sSessionName.c_str());
        errorCode=StartTrace(&m_hSession,m_sSessionName.c_str(),m_pSessionProperties);

    }
    if(errorCode==ERROR_SUCCESS)
    {
        map<GUID,STraceProviderData,CGUIDComparer>::iterator i;

        for(i=m_Providers.begin();i!=m_Providers.end();i++)
        {
            STraceProviderData data=i->second;
            GUID providerGUID=data.pProvider->GetGUID();
            EnableTrace(TRUE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }
        m_hConsumerSession=OpenTrace(&m_ConsumerProperties);
        if(m_hConsumerSession!=(TRACEHANDLE)INVALID_HANDLE_VALUE)
        {
            DWORD threadId=0;
            m_hConsumerThread=CreateThread(NULL,0,ConsumerThread,this,0,&threadId);
        }
    }
    else
    {
        Stop();
    }
    return errorCode;
}

void CTraceController::Stop()
{
    if(m_eSessionType==eTraceControllerSessionType_None){return;}
    
    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;

    if(m_hSession)
    {
        for(i=m_Providers.begin();i!=m_Providers.end();i++)
        {
            STraceProviderData data=i->second;
            GUID providerGUID=data.pProvider->GetGUID();
            EnableTrace(FALSE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }

        StopTrace(m_hSession,m_sSessionName.c_str(),m_pSessionProperties);
        
        
        m_hSession=NULL;
        m_piEventCallback=NULL;
    }

    if(m_hConsumerThread)
    {
        WaitForSingleObject(m_hConsumerThread,INFINITE);
        if(m_hConsumerThread){CloseHandle(m_hConsumerThread);m_hConsumerThread=NULL;}
        memset(&m_ConsumerProperties,0,sizeof(m_ConsumerProperties));
    }		
    if(m_hConsumerSession!=(TRACEHANDLE)INVALID_HANDLE_VALUE){CloseTrace(m_hConsumerSession);m_hConsumerSession=(TRACEHANDLE)INVALID_HANDLE_VALUE;}

    delete m_pSessionProperties;
    m_pSessionProperties=NULL;

    m_eSessionType=eTraceControllerSessionType_None;
}

DWORD CTraceController::GetProviderFlags(CTraceProvider *pProvider)
{
    DWORD dwFlags=0;
    WaitForSingleObject(m_hMutex,INFINITE);

    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());
    if(i!=m_Providers.end())
    {
        STraceProviderData data=i->second;
        dwFlags=data.dwFlags;
    }

    ReleaseMutex(m_hMutex);
    return dwFlags;
}

void CTraceController::SetProviderFlags(CTraceProvider *pProvider,DWORD dwFlags)
{
    WaitForSingleObject(m_hMutex,INFINITE);

    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());
    if(i!=m_Providers.end())
    {
        i->second.dwFlags=dwFlags;
        STraceProviderData data=i->second;
        if(data.bEnabled && !m_bPaused && m_eSessionType!=eTraceControllerSessionType_ReadLog)
        {
            GUID providerGUID=data.pProvider->GetGUID();
            // If OS < Vista is mandatory to disable the provider in order to change its trace settings (level or flags).
            if(!IsWindowsVistaOrGreater())
            {
                EnableTrace(FALSE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
            }
            EnableTrace(TRUE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }
    }

    ReleaseMutex(m_hMutex);
}

DWORD CTraceController::GetProviderLevel(CTraceProvider *pProvider)
{
    DWORD dwLevel=0;
    WaitForSingleObject(m_hMutex,INFINITE);

    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());
    if(i!=m_Providers.end())
    {
        STraceProviderData data=i->second;
        dwLevel=data.dwLevel;
    }

    ReleaseMutex(m_hMutex);
    return dwLevel;
}

void CTraceController::SetProviderLevel(CTraceProvider *pProvider,DWORD dwLevel)
{
    WaitForSingleObject(m_hMutex,INFINITE);

    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());
    if(i!=m_Providers.end())
    {
        i->second.dwLevel=dwLevel;
        STraceProviderData data=i->second;
        if(data.bEnabled && !m_bPaused && m_eSessionType!=eTraceControllerSessionType_ReadLog)
        {
            GUID providerGUID=data.pProvider->GetGUID();
            // If OS < Vista is mandatory to disable the provider in order to change its trace settings (level or flags).
            if(!IsWindowsVistaOrGreater())
            {
                EnableTrace(FALSE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
            }
            EnableTrace(TRUE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }
    }

    ReleaseMutex(m_hMutex);
}

bool CTraceController::IsProviderEnabled(CTraceProvider *pProvider)
{
    bool bEnabled=false;

    WaitForSingleObject(m_hMutex,INFINITE);

    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());
    if(i!=m_Providers.end())
    {
        STraceProviderData data=i->second;
        bEnabled=data.bEnabled;
    }

    ReleaseMutex(m_hMutex);
    return bEnabled;
}

void CTraceController::EnableProvider(CTraceProvider *pProvider,bool bEnabled)
{

    WaitForSingleObject(m_hMutex,INFINITE);

    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;
    i=m_Providers.find(pProvider->GetGUID());
    if(i!=m_Providers.end())
    {
        STraceProviderData data=i->second;
        GUID providerGUID=data.pProvider->GetGUID();
        if(data.bEnabled!=bEnabled)
        {
            if(!m_bPaused && m_eSessionType!=eTraceControllerSessionType_ReadLog)
            {
                EnableTrace(bEnabled?TRUE:FALSE,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
            }
            i->second.bEnabled=bEnabled;
        }
    }

    ReleaseMutex(m_hMutex);
}

void CTraceController::FlushTraces()
{
    if(m_hSession)
    {
        ControlTrace(m_hSession,m_sSessionName.c_str(),m_pSessionProperties,EVENT_TRACE_CONTROL_FLUSH);
    }
};

void CTraceController::Pause(bool bPause)
{
    WaitForSingleObject(m_hMutex,INFINITE);
    map<GUID,STraceProviderData,CGUIDComparer>::iterator i;

    m_bPaused=bPause;

    if(m_hSession && m_eSessionType!=eTraceControllerSessionType_ReadLog)
    {
        for(i=m_Providers.begin();i!=m_Providers.end();i++)
        {
            STraceProviderData data=i->second;
            GUID providerGUID=data.pProvider->GetGUID();
            EnableTrace(!m_bPaused && data.bEnabled,data.dwFlags,data.dwLevel,&providerGUID,m_hSession);
        }
    }
    ReleaseMutex(m_hMutex);
}

bool CTraceController::IsPaused()
{
    return m_bPaused;
}

eTraceControllerSessionType CTraceController::GetSessionType()
{
    return m_eSessionType;
}

tstring CTraceController::GetFileName()
{
    return m_sLogFileName;
}
