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

#include "StdAfx.h"
#include ".\filemonitor.h"
#include <sys/stat.h>

CFileMonitor::CFileMonitor(IFileMonitorCallback *piCallback)
{
    m_piCallback=piCallback;
    m_hStop=CreateEvent(NULL,TRUE,FALSE,NULL);
    m_hMutex=CreateMutex(NULL,FALSE,NULL);
    m_hThread=NULL;
}

CFileMonitor::~CFileMonitor(void)
{
    if(m_hStop){CloseHandle(m_hStop);m_hStop=NULL;}
    if(m_hMutex){CloseHandle(m_hMutex);m_hMutex=NULL;}
}

void CFileMonitor::Start()
{
    if(m_hThread){return;}
    DWORD threadId=0;
    m_hThread=CreateThread(NULL,0,FileMonitorThread_Stub,this,0,&threadId);
}

void CFileMonitor::Stop()
{
    if(!m_hThread){return;}
    SetEvent(m_hStop);
    WaitForSingleObject(m_hThread,INFINITE);
    ResetEvent(m_hStop);
    CloseHandle(m_hThread);
    m_hThread=NULL;
}

void CFileMonitor::AddFile(std::tstring sFile)
{
    WaitForSingleObject(m_hMutex,INFINITE);
    std::tstring sTempFile = sFile;
    _tcsupr_s(const_cast<TCHAR *>(sTempFile.c_str()),sTempFile.length()+1);
    m_mMonitorizedFiles[sTempFile]=GetFileTimeStamp(sFile.c_str());
    ReleaseMutex(m_hMutex);
}

void CFileMonitor::GetFiles(std::set<std::tstring> *pdFilesToMonitor)
{
    WaitForSingleObject(m_hMutex,INFINITE);
    std::map<std::tstring, time_t>::iterator i;
    for(i=m_mMonitorizedFiles.begin();i!=m_mMonitorizedFiles.end();i++)
    {
        pdFilesToMonitor->insert(i->first);
    }
    ReleaseMutex(m_hMutex);
}

void CFileMonitor::SetFiles(std::set<std::tstring> *pdFilesToMonitor)
{
    WaitForSingleObject(m_hMutex,INFINITE);
    m_mMonitorizedFiles.clear();
    std::set<std::tstring>::iterator i;
    for(i=pdFilesToMonitor->begin();i!=pdFilesToMonitor->end();i++)
    {
        std::tstring sTempFile = *i;
        _tcsupr_s(const_cast<TCHAR *>(sTempFile.c_str()), sTempFile.length()+1);
        m_mMonitorizedFiles[sTempFile]=GetFileTimeStamp(sTempFile.c_str());
    }
    ReleaseMutex(m_hMutex);
}

void CFileMonitor::RemoveFile(std::tstring sFile)
{
    WaitForSingleObject(m_hMutex,INFINITE);
    std::tstring sTempFile = sFile;
    _tcsupr_s(const_cast<TCHAR *>(sTempFile.c_str()),sTempFile.length()+1);
    m_mMonitorizedFiles.erase(sTempFile);
    ReleaseMutex(m_hMutex);
}

DWORD WINAPI CFileMonitor::FileMonitorThread_Stub(LPVOID lpThreadParameter)
{
    CFileMonitor *pThis=(CFileMonitor*)lpThreadParameter;
    pThis->FileMonitorThread();
    return 0;
}

void CFileMonitor::FileMonitorThread()
{
    while(1)
    {
        DWORD dwWaitResult=WaitForSingleObject(m_hStop,1000);
        if(dwWaitResult==WAIT_OBJECT_0){break;}

        WaitForSingleObject(m_hMutex,INFINITE);
        std::map<std::tstring, time_t>::iterator i;
        for(i=m_mMonitorizedFiles.begin();i!=m_mMonitorizedFiles.end();i++)
        {
            time_t nNewTime=GetFileTimeStamp(i->first.c_str());
            if(nNewTime>i->second)
            {
                i->second=nNewTime;
                if(m_piCallback){m_piCallback->OnFileChanged(i->first.c_str());}
            }
        }
        ReleaseMutex(m_hMutex);
    }
}

time_t CFileMonitor::GetFileTimeStamp(const TCHAR *pFileName)
{
    struct _stat64 data;
    if (_tstat64(pFileName, &data) != 0){ return 0; }
    return data.st_mtime;
}
