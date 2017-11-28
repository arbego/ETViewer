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

class IFileMonitorCallback
{
public:
    virtual void OnFileChanged(std::tstring sFile)=0;
};
class CFileMonitor
{
    IFileMonitorCallback *m_piCallback;
    HANDLE				 m_hStop;
    HANDLE				 m_hThread;
    HANDLE				 m_hMutex;

    std::map<std::tstring,time_t>   m_mMonitorizedFiles;

    static DWORD WINAPI FileMonitorThread_Stub(LPVOID lpThreadParameter);
    void FileMonitorThread();

    time_t GetFileTimeStamp(const TCHAR *pFileName);

public:

    void Start();
    void Stop();
    void AddFile(std::tstring sFile);
    void RemoveFile(std::tstring sFile);
    void GetFiles(std::set<std::tstring> *pdFilesToMonitor);
    void SetFiles(std::set<std::tstring> *pdFilesToMonitor);

    CFileMonitor(IFileMonitorCallback *piCallback);
    ~CFileMonitor(void);
};
