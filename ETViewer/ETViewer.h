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

#ifndef __AFXWIN_H__
    #error incluye 'stdafx.h' antes de incluir este archivo para PCH
#endif

#include "resource.h"       // Símbolos principales
#include "SourceFileContainer.h"       // Símbolos principales
#include "TraceController.h"      
#include "MainFrm.h"      
#include "FileMonitor.h"      

#define RECENT_PDB_FILE_BASE_INDEX					3000
#define RECENT_PDB_FILE_MAX							15
#define RECENT_SOURCE_FILE_BASE_INDEX				4000
#define RECENT_SOURCE_FILE_MAX						15
#define RECENT_LOG_FILE_BASE_INDEX					5000
#define RECENT_LOG_FILE_MAX							15

#define FILE_CHANGE_TIMER_ID						1
#define FILE_CHANGE_EXPIRATION_TIME 5000
#define FILE_CHANGE_TIMER_PERIOD	500

enum 
{
    eETViewerColumn_BASE=0x0E000000,
    eETViewerColumn_Text=eETViewerColumn_BASE,
    eETViewerColumn_PIDTID,
    eETViewerColumn_Time,
    eETViewerColumn_TimeStamp,
    eETViewerColumn_Index,
    eETViewerColumn_Source,
    eETViewerColumn_Component,
    eETViewerColumn_Method,
    eETViewerColumn_Level,
    eETViewerColumn_Flag
};

enum 
{
    eETViewerSortDirection_Ascending,
    eETViewerSortDirection_Descending,
};

class CHightLightFilter
{
    std::tstring        m_Text;
    DWORD				m_dwTextLen;
    DWORD				m_TextColor;
    DWORD				m_BkColor;
    bool				m_bEnabled;
    HPEN				m_hPen;
    HBRUSH				m_hBrush;

public:


    HPEN		GetPen(){return m_hPen;}
    HBRUSH		GetBrush(){return m_hBrush;}

    COLORREF	GetTextColor(){return m_TextColor;}
    void		SetTextColor(COLORREF textColor){m_TextColor=textColor;UpdateObjects();}
    COLORREF	GetBkColor(){return m_BkColor;}
    void		SetBkColor(COLORREF bkColor){m_BkColor=bkColor;UpdateObjects();}
    void		SetEnabled(bool bEnable){m_bEnabled=bEnable;}
    bool		GetEnabled(){return m_bEnabled;}

    void		    SetText(std::tstring sText){m_Text=sText;m_dwTextLen=(DWORD)_tcslen(m_Text.c_str());}
    std::tstring	&GetText(){return m_Text;}
    DWORD		    GetTextLen(){return m_dwTextLen;}

    CHightLightFilter(const CHightLightFilter &otherFilter)
    {
        m_Text=otherFilter.m_Text;
        m_TextColor=otherFilter.m_TextColor;
        m_BkColor=otherFilter.m_BkColor;
        m_bEnabled=otherFilter.m_bEnabled;
        m_dwTextLen=otherFilter.m_dwTextLen;
        UpdateObjects();
    }
    CHightLightFilter &operator = (CHightLightFilter &otherFilter)
    {
        m_Text=otherFilter.m_Text;
        m_TextColor=otherFilter.m_TextColor;
        m_BkColor=otherFilter.m_BkColor;
        m_bEnabled=otherFilter.m_bEnabled;
        m_dwTextLen=otherFilter.m_dwTextLen;
        UpdateObjects();
        return *this;
    }

    CHightLightFilter()
    {
        m_Text.clear();
        m_bEnabled=true;
        m_dwTextLen=0;
        m_hPen=NULL;
        m_hBrush=NULL;
        SetTextColor(RGB(0,0,0));
        SetBkColor(RGB(255,255,255));
    }
    ~CHightLightFilter()
    {
        if(m_hPen){DeleteObject(m_hPen);m_hPen=NULL;}
        if(m_hBrush){DeleteObject(m_hBrush);m_hBrush=NULL;}
    }

    void UpdateObjects()
    {
        if(m_hPen){DeleteObject(m_hPen);m_hPen=NULL;}
        if(m_hBrush){DeleteObject(m_hBrush);m_hBrush=NULL;}

        POINT	 P={1,1};
        LOGPEN   LP={0};
        LP.lopnColor=m_BkColor;
        LP.lopnWidth=P;
        LP.lopnStyle=PS_SOLID;

        LOGBRUSH LB={0};
        LB.lbColor=m_BkColor;
        LB.lbStyle=BS_SOLID;

        m_hBrush=CreateBrushIndirect(&LB);
        m_hPen=CreatePenIndirect(&LP);
    }

    BEGIN_PERSIST_MAP(CHightLightFilter)
        PERSIST(m_Text,      _T("Text"))
        PERSIST(m_dwTextLen, _T("TextLength"))
        PERSIST(m_TextColor, _T("TextColor"))
        PERSIST(m_BkColor,   _T("BkColor"))
        PERSIST(m_bEnabled,  _T("Enabled"))
    END_PERSIST_MAP()

};

DECLARE_SERIALIZABLE(CHightLightFilter)

class CFilter
{
public:

    std::tstring		m_Text;
    DWORD       		m_dwTextLen;
    bool		        m_bInclusionFilter;

    CFilter()
    {
        m_Text[0]=0;
        m_bInclusionFilter=true;
        m_dwTextLen=0;
    }
    ~CFilter(){}
};

enum eFileMonitoringMode
{
    eFileMonitoringMode_None,
    eFileMonitoringMode_AutoReload,
    eFileMonitoringMode_Ignore,
    eFileMonitoringMode_Ask
};


struct SPendingFileChangeOperation
{
    std::tstring sFileName;
    DWORD  dwChangeTime; // Tick count
};

DECLARE_SERIALIZABLE_ENUMERATION(eFileMonitoringMode);

class CETViewerApp : public CWinApp, public IFileMonitorCallback
{
public:
    CETViewerApp();
    ~CETViewerApp();

    std::set<CTraceProvider *> m_sProviders;
    CTracePDBReader		  m_PDBReader;

    HANDLE m_hInstantTraceMutex;
    HANDLE m_hSingleInstanceMutex;

    std::list<SPendingFileChangeOperation> m_lPendingFileChanges;
    HANDLE							  m_hPendingFileChangesMutex;
    bool							  m_bCheckingFileChangeOperations;

    void OnClose();

// Reemplazos
public:

    void AddFileChangeOperation(std::tstring sFileName);
    void RemoveFileChangeOperation(std::tstring sFileName);
    void RemoveExpiredFileChangeOperations();
    void CheckFileChangeOperations();

    void SetFileAssociation(TCHAR *pExtension,TCHAR *pFileTypeName,TCHAR *pFileDescription,TCHAR *pCommandLineTag);

    virtual BOOL InitInstance();

    std::tstring		     m_sConfigFile;
    CConfigFile				 m_ConfigFile;

    std::deque<CHightLightFilter> m_HighLightFilters;
    std::deque<CHightLightFilter> m_SplittedHighLightFilters;
    std::deque<std::tstring>		 m_RecentSourceFiles;
    std::deque<std::tstring>		 m_RecentPDBFiles;
    std::deque<std::tstring>		 m_RecentLogFiles;
    std::deque<std::tstring>		 m_SourceDirectories;

    std::deque<CFilter>			m_dSplittedInstantFilters;

    std::tstring 					m_InstantIncludeFilter;
    std::tstring 					m_InstantExcludeFilter;
    std::deque<std::tstring>		m_InstantIncludeFilterList;
    std::deque<std::tstring>		m_InstantExcludeFilterList;
    bool					m_bAssociateETL;
    bool					m_bAssociatePDB;
    bool					m_bAssociateSources;

    eFileMonitoringMode		m_ePDBMonitoringMode;
    eFileMonitoringMode		m_eSourceMonitoringMode;

    CSourceFileContainer m_SourceFileContainer;
    CTraceController	 m_Controller;
    CMainFrame			*m_pFrame;
    CFileMonitor		*m_pFileMonitor;

    void RefreshRecentFilesMenus();

    void UpdateHighLightFilters();
    void UpdateInstantFilters();

    bool AddProvider(CTraceProvider *pProvider);
    void ReloadProvider(CTraceProvider *pProvider);
    bool ReloadPDBProviders(std::tstring sFileName);
    void ReloadAllProviders();
    void RemoveProvider(CTraceProvider *pProvider);
    void RemoveAllProviders();

    void AddRecentSourceFile(const TCHAR *pFile);
    void AddRecentPDBFile(const TCHAR *pFile);
    void AddRecentLogFile(const TCHAR *pFile);
    bool OpenCodeAddress(const TCHAR *pFile,DWORD dwLine,bool bShowErrorIfFailed);
    bool OpenPDB(const TCHAR *pFile,bool bShowErrorIfFailed);
    bool OpenETL(const TCHAR *pFile);
    bool CreateETL(const TCHAR *pFile);
    void CloseETL();
    void CloseSession();
    void UpdateFileMonitor();
    void UpdateFileAssociations();
    void ProcessSessionChange();
    BOOL ProcessCommandLine(int argc,TCHAR **argv);

    void LookupError(const TCHAR *pText);
    bool FilterTrace(const TCHAR *pText);

    // IFileMonitorCallback
    void OnFileChanged(std::tstring sFile);

    BEGIN_PERSIST_MAP(CETViewerApp)
        PERSIST(m_InstantIncludeFilter, _T("IncludeFilter"))
        PERSIST(m_InstantExcludeFilter,_T("ExcludeFilter"))
        PERSIST(m_InstantIncludeFilterList,_T("IncludeFilterList"))
        PERSIST(m_InstantExcludeFilterList,_T("ExcludeFilterList"))
        PERSIST(m_HighLightFilters,_T("HighLightFilters"))
        PERSIST(m_RecentSourceFiles,_T("RecentSourceFiles"))
        PERSIST(m_RecentPDBFiles,_T("RecentPDBFiles"))
        PERSIST_FLAGS(m_RecentLogFiles,_T("m_RecentLogFiles"),PF_NORMAL|PF_OPTIONAL)
        PERSIST_FLAGS(m_SourceDirectories,_T("SourceDirectories"),PF_NORMAL|PF_OPTIONAL)
        PERSIST_VALUE_FLAGS(m_bAssociateETL,_T("AssociateETL"),false,PF_NORMAL|PF_OPTIONAL)
        PERSIST_VALUE_FLAGS(m_bAssociatePDB,_T("AssociatePDB"),false,PF_NORMAL|PF_OPTIONAL)
        PERSIST_VALUE_FLAGS(m_bAssociateSources,_T("AssociateSources"),false,PF_NORMAL|PF_OPTIONAL)
        PERSIST_VALUE_FLAGS(m_ePDBMonitoringMode,_T("PDBMonitoringMode"),eFileMonitoringMode_AutoReload,PF_NORMAL|PF_OPTIONAL)
        PERSIST_VALUE_FLAGS(m_eSourceMonitoringMode,_T("SourceMonitoringMode"),eFileMonitoringMode_AutoReload,PF_NORMAL|PF_OPTIONAL)
    END_PERSIST_MAP();

    DECLARE_CONFIG_FILE_MEDIA();
// Implementación
    afx_msg void OnAppAbout();
    afx_msg void OnRecentPDBFile(UINT nID);
    afx_msg void OnRecentSourceFile(UINT nID);
    afx_msg void OnRecentLogFile(UINT nID);
    DECLARE_MESSAGE_MAP()
    virtual int ExitInstance();
};

extern CETViewerApp theApp;
