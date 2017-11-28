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

#include "stdafx.h"
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>
#include "ETViewer.h"
#include "MainFrm.h"

#include "ETViewerDoc.h"
#include "ETViewerView.h"
#include "ProviderTree.h"
#include ".\etviewer.h"
#include "FileMonitor.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ET_VIEWER_NAME "ETViewer"

extern DWORD g_dwRegisteredMessage;

// CETViewerApp

BEGIN_MESSAGE_MAP(CETViewerApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    // Comandos de documento estándar basados en archivo
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
    ON_COMMAND_RANGE(RECENT_PDB_FILE_BASE_INDEX,RECENT_PDB_FILE_BASE_INDEX+RECENT_PDB_FILE_MAX,OnRecentPDBFile)
    ON_COMMAND_RANGE(RECENT_SOURCE_FILE_BASE_INDEX,RECENT_SOURCE_FILE_BASE_INDEX+RECENT_SOURCE_FILE_MAX,OnRecentSourceFile)
    ON_COMMAND_RANGE(RECENT_LOG_FILE_BASE_INDEX,RECENT_LOG_FILE_BASE_INDEX+RECENT_LOG_FILE_MAX,OnRecentLogFile)
END_MESSAGE_MAP()


// Construcción de CETViewerApp

CETViewerApp::CETViewerApp()
{	
    TCHAR tempDrive[MAX_PATH]={0},tempPath[MAX_PATH]={0};
    TCHAR currentModule[MAX_PATH]={0};
    GetModuleFileName(NULL,currentModule,_countof(currentModule));
    _tsplitpath_s(currentModule, tempDrive, MAX_PATH, tempPath, MAX_PATH, NULL, 0, NULL, 0);

    m_sConfigFile=tempDrive;
    m_sConfigFile+=tempPath;
    m_sConfigFile+=_T("\\ETViewer.cfg");

    m_bCheckingFileChangeOperations=false;
    m_hPendingFileChangesMutex=CreateMutex(NULL,FALSE,NULL);
    m_hInstantTraceMutex=CreateMutex(NULL,FALSE,NULL);
    m_InstantIncludeFilter=_T("*");
    m_eSourceMonitoringMode=eFileMonitoringMode_AutoReload;
    m_ePDBMonitoringMode=eFileMonitoringMode_AutoReload;

    m_ConfigFile.Open(m_sConfigFile.c_str());

    m_pFileMonitor=new CFileMonitor(this);

    LoadFrom(&m_ConfigFile,_T("Application"));
    UpdateHighLightFilters();
    UpdateInstantFilters();

}

CETViewerApp::~CETViewerApp()
{
    std::set<CTraceProvider *>::iterator i;
    for(i=m_sProviders.begin();i!=m_sProviders.end();i++)
    {
        CTraceProvider *pProvider=*i;
        delete pProvider;
    }
    m_sProviders.clear();
    if(m_hInstantTraceMutex){CloseHandle(m_hInstantTraceMutex);m_hInstantTraceMutex=NULL;}
    if(m_hPendingFileChangesMutex){CloseHandle(m_hPendingFileChangesMutex);m_hPendingFileChangesMutex=NULL;}

    m_pFileMonitor->Stop();
    delete m_pFileMonitor;
    m_pFileMonitor=NULL;
}

// El único objeto CETViewerApp

CETViewerApp theApp;

// Inicialización de CETViewerApp

BOOL CETViewerApp::InitInstance()
{
    CoInitialize(NULL);

    bool bFirstInstance=FALSE;
    DWORD dwLastError=ERROR_SUCCESS;
    m_hSingleInstanceMutex=CreateMutex(NULL,TRUE,_T("ETViewerSingleInstanceMutex"));
    if(m_hSingleInstanceMutex)
    {
        dwLastError=GetLastError();
        if(dwLastError!=ERROR_ALREADY_EXISTS)
        {
            bFirstInstance=TRUE;
        }
    }

    //If UAC is enabled we need to open up some messages so drag and drop will work if we are admin and explorer isn't

    typedef BOOL(WINAPI *PFN_CHANGEWINDOWMESSAGEFILTER) (UINT, DWORD);
    HMODULE hUser32 = GetModuleHandle(_T("user32.dll"));
    if (0 != hUser32)
    {
        PFN_CHANGEWINDOWMESSAGEFILTER pfnChangeWindowMessageFilter = (PFN_CHANGEWINDOWMESSAGEFILTER) ::GetProcAddress(hUser32, "ChangeWindowMessageFilter");
        if (pfnChangeWindowMessageFilter)
        {
            (*pfnChangeWindowMessageFilter)(WM_DROPFILES, MSGFLT_ADD);
            (*pfnChangeWindowMessageFilter)(WM_COPYDATA, MSGFLT_ADD);
            (*pfnChangeWindowMessageFilter)(0x0049, MSGFLT_ADD);
        }
    }

    // If this is not the first instance, send the command line files to
    // to the first instance, this way, a user can double click an element
    // on a folder to add it to ETViewer.
    if(!bFirstInstance)
    {
        for(int x=0;x<__argc;x++)
        {
            if (__wargv[x][0] == _T('/') || __wargv[x][0] == _T('-'))
            {
                ATOM hAtom = GlobalAddAtom(__wargv[x]);
                if(hAtom)
                {
                    // PostMessage is not an option because the Atom can be deleted before the message is received
                    // SendMessage does not return in both Vista and Windows7 with UAC on
                    ::SendMessageTimeout(HWND_BROADCAST,g_dwRegisteredMessage,hAtom,0,SMTO_BLOCK,1000,NULL);
                    GlobalDeleteAtom(hAtom);
                }
            }
        }
        return FALSE;
    }

    // Windows XP requiere InitCommonControls() si un manifiesto de 
    // aplicación especifica el uso de ComCtl32.dll versión 6 o posterior para habilitar
    // estilos visuales. De lo contrario, se generará un error al crear ventanas.
    InitCommonControls();

    CWinApp::InitInstance();

    CSingleDocTemplate* pDocTemplate;
    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CETViewerDoc),
        RUNTIME_CLASS(CMainFrame),       // Ventana de marco MDI principal
        RUNTIME_CLASS(CProviderTree));
    if (!pDocTemplate)
        return FALSE;

    AddDocTemplate(pDocTemplate);

    this->m_nCmdShow = SW_HIDE;

    // Analizar línea de comandos para comandos Shell estándar, DDE, Archivo Abrir
    CCommandLineInfo cmdInfo;
    //ParseCommandLine(cmdInfo);
    // Enviar comandos especificados en la línea de comandos. Devolverá FALSE si
    // la aplicación se inició con los modificadores /RegServer, /Register, /Unregserver o /Unregister.
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    m_SourceFileContainer.Create(IDD_SOURCE_FILE_CONTAINER,m_pFrame);

    // Se ha inicializado la única ventana; mostrarla y actualizarla
    m_pFrame=dynamic_cast<CMainFrame *>(m_pMainWnd);
    // Llamar a DragAcceptFiles sólo si existe un sufijo
    //  En una aplicación SDI, esto debe ocurrir después de ProcessShellCommand

    m_SourceFileContainer.SetWindowPos(&CWnd::wndNoTopMost,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);

    RefreshRecentFilesMenus();
    ProcessSessionChange();
    BOOL bContinue = ProcessCommandLine(__argc,__wargv);
    if(bContinue)
    {
        m_pMainWnd->ShowWindow(SW_SHOW);
        m_pMainWnd->UpdateWindow();
        m_pFileMonitor->Start();
    }
    return bContinue;
}

void CETViewerApp::SetFileAssociation(TCHAR *pExtension,TCHAR *pFileTypeName,TCHAR *pFileDescription,TCHAR *pCommandLineTag)
{
    bool bAlreadyPresent=false;
    HKEY hKey=NULL;
    DWORD dwError=RegOpenKey(HKEY_CLASSES_ROOT,pExtension,&hKey);
    if(dwError==ERROR_SUCCESS)
    {
        long dwSize=MAX_PATH;
        TCHAR  sValue[MAX_PATH]={0};
        dwError=RegQueryValue(hKey,NULL,sValue,&dwSize);
        if(dwError==ERROR_SUCCESS && _tcscmp(sValue,pFileTypeName)==0)
        {
            bAlreadyPresent=true;
        }
        RegCloseKey(hKey);
        hKey=NULL;
    }
    if(!bAlreadyPresent)
    {
        dwError=RegCreateKey(HKEY_CLASSES_ROOT,pExtension,&hKey);
        if(dwError==ERROR_SUCCESS)
        {
            dwError=RegSetValue(hKey,NULL,REG_SZ,pFileTypeName,_tcslen(pFileTypeName)+1);
        }
        RegCloseKey(hKey);
        hKey=NULL;
    }
    dwError=RegOpenKey(HKEY_CLASSES_ROOT,pFileTypeName,&hKey);
    if(hKey){RegCloseKey(hKey);hKey=NULL;}

    dwError=RegCreateKey(HKEY_CLASSES_ROOT,pFileTypeName,&hKey);
    if(dwError==ERROR_SUCCESS)
    {
        dwError=RegSetValue(hKey,NULL,REG_SZ,pFileDescription,_tcslen(pFileDescription)+1);
    }
    if(hKey){RegCloseKey(hKey);hKey=NULL;}

    if(dwError==ERROR_SUCCESS)
    {
        std::tstring sKeyName = pFileTypeName;
        sKeyName+=_T("\\shell\\open\\command");
        dwError=RegCreateKey(HKEY_CLASSES_ROOT,sKeyName.c_str(),&hKey);
        if(dwError==ERROR_SUCCESS)
        {
            TCHAR pModule[MAX_PATH]={0};
            TCHAR pCommand[MAX_PATH]={0};
            GetModuleFileName(NULL,pModule,MAX_PATH);
            _stprintf_s(pCommand, _T("\"%s\" %s"), pModule, pCommandLineTag);
            dwError=RegSetValue(hKey,NULL,REG_SZ,pCommand,_tcslen(pCommand)+1);
        }
        if(hKey){RegCloseKey(hKey);hKey=NULL;}
    }
}

BOOL CETViewerApp::ProcessCommandLine(int argc,TCHAR **argw)
{
    unsigned x=0;
    bool bPDBSpecified=false,bSilent=false,bSourceSpecified=false;
    bool bLevelSpecified=false,bValidLevel=true,bFailedToOpenETL=false;
    DWORD dwLevel=TRACE_LEVEL_VERBOSE;
    std::tstring sETLFile;

    std::vector<std::tstring> vPDBsToLoad;
    std::vector<std::tstring> vSourcesToLoad;

    for(x=0;x<(ULONG)argc;x++)
    {
        DWORD dwTmpLen = _tcslen(argw[x])+1;
        TCHAR *sTemp=new TCHAR [dwTmpLen];
        _tcscpy_s(sTemp,dwTmpLen,argw[x]);
        _tcsupr_s(sTemp,dwTmpLen);
        if(	_tcscmp(sTemp,_T("-V"))==0 || 
            _tcscmp(sTemp,_T("/V"))==0)
        {
            MessageBox(NULL,_T("ETViewer v0.9\n"),_T("ETViewer"),MB_OK);
            return FALSE;
        }
        else if (_tcscmp(sTemp, _T("-H")) == 0 ||
                 _tcscmp(sTemp, _T("/H")) == 0)
        {
            MessageBox(NULL,_T("ETViewer v1.1\n\n") 
                            _T("Command line options:\n\n")
                            _T("  -v   Show version\n")
                            _T("  -h   Show this help\n")
                            _T("  -s   Silent: Do not warn about statup errors\n")
                            _T("  -pdb:<file filter>  PDB files to load, for example -pdb:C:\\Sample\\*.pdb\n")
                            _T("  -src:<file filter>   Source files to load, for example -src:C:\\Sample\\*.cpp\n")
                            _T("  -etl:<file> Load the specified .etl file on startup\n ")
                            _T("  -l:<level> Initial tracing level:\n")
                            _T("     FATAL\n")
                            _T("     ERROR\n")
                            _T("     WARNING\n")
                            _T("     INFORMATION\n")
                            _T("     VERBOSE\n")
                            _T("     RESERVED6\n")
                            _T("     RESERVED7\n")
                            _T("     RESERVED8\n")
                            _T("     RESERVED9"), 
                            _T("ETViewer"), MB_OK);
            return FALSE;
        }
        else if(_tcsncmp(sTemp,_T("-PDB:"),_tcslen(_T("-PDB:")))==0 || 
            _tcsncmp(sTemp, _T("/PDB:"), _tcslen(_T("/PDB:"))) == 0)
        {
            TCHAR drive[MAX_PATH]={0};
            TCHAR path[MAX_PATH]={0};
            TCHAR *pPDB=argw[x]+_tcslen(_T("-PDB:"));

            bPDBSpecified=true;

            _tsplitpath_s(pPDB, drive, MAX_PATH, path, MAX_PATH, NULL, 0, NULL, 0);

            WIN32_FIND_DATA findData={0};
            HANDLE hFind=FindFirstFile(pPDB,&findData);
            if(hFind!=INVALID_HANDLE_VALUE)
            {
                do
                {
                    TCHAR tempFile[MAX_PATH]={0};
                    _tcscpy_s(tempFile,drive);
                    _tcscpy_s(tempFile, path);
                    _tcscpy_s(tempFile, findData.cFileName);

                    vPDBsToLoad.push_back(tempFile);
                }
                while(FindNextFile(hFind,&findData));
                FindClose(hFind);
            }
        }
        else if(_tcsncmp(sTemp,_T("-SRC:"),_tcslen(_T("-SRC:")))==0 || 
                _tcsncmp(sTemp, _T("/SRC:"), _tcslen(_T("/SRC:"))) == 0)
        {
            TCHAR drive[MAX_PATH]={0};
            TCHAR path[MAX_PATH]={0};
            TCHAR *pSource=argw[x]+_tcslen(_T("-SRC:"));

            bSourceSpecified=true;

            _tsplitpath_s(pSource, drive, MAX_PATH, path, MAX_PATH, NULL, 0, NULL, 0);

            WIN32_FIND_DATA findData={0};
            HANDLE hFind=FindFirstFile(pSource,&findData);
            if(hFind!=INVALID_HANDLE_VALUE)
            {
                do
                {
                    TCHAR tempFile[MAX_PATH]={0};
                    _tcscpy_s(tempFile, drive);
                    _tcscpy_s(tempFile, path);
                    _tcscpy_s(tempFile, findData.cFileName);

                    vSourcesToLoad.push_back(tempFile);
                }
                while(FindNextFile(hFind,&findData));
                FindClose(hFind);
            }
        }
        else if (_tcsncmp(sTemp, _T("-ETL:"), _tcslen(_T("-ETL:"))) == 0 ||
                 _tcsncmp(sTemp, _T("/ETL:"), _tcslen(_T("/ETL:"))) == 0)
        {
            sETLFile=(sTemp+5);

            if(!OpenETL(sETLFile.c_str()))
            {
                bFailedToOpenETL=true;
            }
        }		
        else if (_tcsncmp(sTemp, _T("-L:"), _tcslen(_T("/L:"))) == 0)
        {
            if (_tcscmp(sTemp, _T("-L:NONE")) == 0){ dwLevel = TRACE_LEVEL_NONE; }
            else if (_tcscmp(sTemp, _T("-L:FATAL")) == 0){ dwLevel = TRACE_LEVEL_FATAL; }
            else if (_tcscmp(sTemp, _T("-L:ERROR")) == 0){ dwLevel = TRACE_LEVEL_ERROR; }
            else if (_tcscmp(sTemp, _T("-L:WARNING")) == 0){ dwLevel = TRACE_LEVEL_WARNING; }
            else if (_tcscmp(sTemp, _T("-L:INFORMATION")) == 0){ dwLevel = TRACE_LEVEL_INFORMATION; }
            else if (_tcscmp(sTemp, _T("-L:VERBOSE")) == 0){ dwLevel = TRACE_LEVEL_VERBOSE; }
            else if (_tcscmp(sTemp, _T("-L:RESERVED6")) == 0){ dwLevel = TRACE_LEVEL_RESERVED6; }
            else if (_tcscmp(sTemp, _T("-L:RESERVED7")) == 0){ dwLevel = TRACE_LEVEL_RESERVED7; }
            else if (_tcscmp(sTemp, _T("-L:RESERVED8")) == 0){ dwLevel = TRACE_LEVEL_RESERVED8; }
            else if (_tcscmp(sTemp, _T("-L:RESERVED9")) == 0){ dwLevel = TRACE_LEVEL_RESERVED9; }
            else 
            {
                bValidLevel=false;
            }
        }
        else if (_tcsncmp(sTemp, _T("-S"), _tcslen(_T("/S"))) == 0)
        {
            bSilent=true;
        }

        delete [] sTemp;
    }
    for(x=0;x<vPDBsToLoad.size();x++)
    {
        OpenPDB(vPDBsToLoad[x].c_str(),!bSilent);
    }
    for(x=0;x<vSourcesToLoad.size();x++)
    {
        OpenCodeAddress(vSourcesToLoad[x].c_str(),0,!bSilent);
    }	
    if(!bSilent)
    {
        if(bPDBSpecified && vPDBsToLoad.size()==0)
        {
            MessageBox(GetActiveWindow(),_T("No PDB file was found in the specified paths"),_T("ETViewer"),MB_ICONSTOP|MB_OK);
        }
        if(bSourceSpecified && vSourcesToLoad.size()==0)
        {
            MessageBox(GetActiveWindow(),_T("No source file was found in the specified paths"),_T("ETViewer"),MB_ICONSTOP|MB_OK);
        }
        if(bFailedToOpenETL)
        {
            std::tstring sText = _T("Failed to open .etl file '");
            sText+=sETLFile;
            sText+=_T("'");
            MessageBox(GetActiveWindow(),sText.c_str(),_T("ETViewer"),MB_ICONSTOP|MB_OK);
        }		
        if(!bValidLevel)
        {
            MessageBox(GetActiveWindow(),_T("The specified level is not valid, please use the following values\r\n\r\n\t-L:NONE\r\n\t-L:FATAL\r\n\t-L:ERROR\r\n\t-L:WARNING\r\n\t-L:INFORMATION\r\n\t-L:VERBOSE\r\n\t-L:RESERVED6\r\n\t-L:RESERVED7\r\n\t-L:RESERVED8\r\n\t-L:RESERVED9"),_T("ETViewer"),MB_ICONSTOP|MB_OK);
        }
    }
    if(bValidLevel)
    {
        m_pFrame->GetProviderTree()->SetAllProviderLevel(dwLevel);
    }
    return TRUE;
}


// Cuadro de diálogo CAboutDlg utilizado para el comando Acerca de

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Datos del cuadro de diálogo
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX/DDV

// Implementación
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// Comando de la aplicación para ejecutar el cuadro de diálogo
void CETViewerApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}


// Controladores de mensaje de CETViewerApp


void CETViewerApp::UpdateHighLightFilters()
{
    m_SplittedHighLightFilters.clear();
    for(DWORD x=0;x<m_HighLightFilters.size();x++)
    {
        m_HighLightFilters[x].UpdateObjects();

        if(m_HighLightFilters[x].GetEnabled())
        {
            TCHAR temp[1024];
            _tcscpy_s(temp,m_HighLightFilters[x].GetText().c_str());
            _tcsupr_s(temp);

            TCHAR* nextToken = NULL;
            TCHAR* token=_tcstok_s(temp,_T(";"),&nextToken);
            while(token!=NULL)
            {
                CHightLightFilter filter=m_HighLightFilters[x];
                filter.SetText(token);
                m_SplittedHighLightFilters.push_back(filter);
                token=_tcstok_s(NULL,_T(";"),&nextToken);
            }
        }
    }
    if(m_pFrame && m_pFrame->GetTracePane())
    {
        m_pFrame->GetTracePane()->InvalidateRect(NULL);
    }
        
}	
void CETViewerApp::UpdateInstantFilters()
{
    TCHAR temp[2048]={0};
    TCHAR* nextToken = NULL;
    TCHAR* token=NULL;

    WaitForSingleObject(m_hInstantTraceMutex,INFINITE);

    m_dSplittedInstantFilters.clear();

    _tcscpy_s(temp,m_InstantExcludeFilter.c_str());
    _tcsupr_s(temp);
    
    token=_tcstok_s(temp,_T(";"),&nextToken);
    while(token!=NULL)
    {
        CFilter filter;
        filter.m_Text=token;
        filter.m_bInclusionFilter=false;
        filter.m_dwTextLen=(DWORD)_tcslen(token);
        m_dSplittedInstantFilters.push_back(filter);
        token=_tcstok_s(NULL,_T(";"),&nextToken);
    }

    _tcscpy_s(temp,m_InstantIncludeFilter.c_str());
    _tcsupr_s(temp);
    nextToken = NULL;
    token=_tcstok_s(temp,_T(";"),&nextToken);
    while(token!=NULL)
    {
        CFilter filter;
        filter.m_Text=token;
        filter.m_bInclusionFilter=true;
        filter.m_dwTextLen=(DWORD)_tcslen(token);
        m_dSplittedInstantFilters.push_back(filter);
        token=_tcstok_s(NULL,_T(";"),&nextToken);
    }

    ReleaseMutex(m_hInstantTraceMutex);
}	

void CETViewerApp::LookupError(const TCHAR *pText)
{
    m_pFrame->LookupError(pText);
}


void CETViewerApp::AddRecentSourceFile(const TCHAR *pFile)
{
    std::tstring file;
    file=pFile;
    std::deque<std::tstring>::iterator i;
    for(i=m_RecentSourceFiles.begin();i!=m_RecentSourceFiles.end();i++)
    {
        if(_tcscmp(i->c_str(),pFile)==0)
        {
            m_RecentSourceFiles.erase(i);
            break;
        }
    }
    m_RecentSourceFiles.push_front(file);
    RefreshRecentFilesMenus();
    if(m_RecentSourceFiles.size()>=RECENT_SOURCE_FILE_MAX)
    {m_RecentSourceFiles.pop_back();}
}

void CETViewerApp::AddRecentPDBFile(const TCHAR *pFile)
{
    std::tstring file;
    file=pFile;
    std::deque<std::tstring>::iterator i;
    for(i=m_RecentPDBFiles.begin();i!=m_RecentPDBFiles.end();i++)
    {
        if(_tcscmp(i->c_str(),pFile)==0)
        {
            m_RecentPDBFiles.erase(i);
            break;
        }
    }
    m_RecentPDBFiles.push_front(file);
    RefreshRecentFilesMenus();
    if(m_RecentPDBFiles.size()>=RECENT_PDB_FILE_MAX)
    {m_RecentPDBFiles.pop_back();}
}

void CETViewerApp::AddRecentLogFile(const TCHAR *pFile)
{
    std::tstring file;
    file=pFile;
    std::deque<std::tstring>::iterator i;
    for(i=m_RecentLogFiles.begin();i!=m_RecentLogFiles.end();i++)
    {
        if(_tcscmp(i->c_str(),pFile)==0)
        {
            m_RecentLogFiles.erase(i);
            break;
        }
    }
    m_RecentLogFiles.push_front(file);
    RefreshRecentFilesMenus();
    if(m_RecentLogFiles.size()>=RECENT_LOG_FILE_MAX)
    {m_RecentLogFiles.pop_back();}
}

bool CETViewerApp::OpenPDB(const TCHAR *pFile,bool bShowErrorIfFailed)
{
    std::vector<CTraceProvider *> vProviders;
    eTraceReaderError eErrorCode=m_PDBReader.LoadFromPDB(pFile,&vProviders);
    if(eErrorCode==eTraceReaderError_Success)
    {
        AddRecentPDBFile(pFile);

        for(unsigned x=0;x<vProviders.size();x++)
        {
            CTraceProvider  *pProvider=vProviders[x];
            if(pProvider)
            {
                if(!AddProvider(pProvider))
                {
                    if(bShowErrorIfFailed)
                    {
                        std::tstring sTemp = _T("The provider '");
                        sTemp+=pProvider->GetComponentName().c_str();
                        sTemp+=_T(" has already been loaded");
                        MessageBox(GetActiveWindow(),sTemp.c_str(),_T("ETViewer"),MB_ICONSTOP|MB_OK);
                    }
                    delete pProvider;
                }
            }
        }
    }
    else
    {
        if(bShowErrorIfFailed)
        {
            std::tstring sTemp = _T("Failed to load pdb file '");
            sTemp+=pFile;
            sTemp+=_T("'.\r\n\r\n");
            switch(eErrorCode)
            {
            case eTraceReaderError_NoMemory:
                sTemp+=_T("Not enought memory.");
                break;
            case eTraceReaderError_SymEngineInitFailed:
                sTemp+=_T("Failed to initialize the symbol engine.");
                break;
            case eTraceReaderError_FailedToEnumerateSymbols:
                sTemp += _T("Failed to enumerate PDB symbols.");
                break;			
            case eTraceReaderError_SymEngineLoadModule:
                sTemp += _T("Failed to load module by the symbol engine.");
                break;			
            case eTraceReaderError_NoProviderFoundInPDB:
                sTemp += _T("No provider was found in the PDB file.");
                break;			
            case eTraceReaderError_DBGHelpNotFound:
                sTemp += _T("DBGHelp.dll was not found.");
                break;
            case eTraceReaderError_DBGHelpWrongVersion:
                sTemp += _T("Wrong version of DBGHelp.dll found (does not have the expected exports).");
                break;
            case eTraceReaderError_PDBNotFound:
                sTemp += _T("Cannot open the PDB file.");
                break;
            case eTraceReaderError_Generic:
                sTemp += _T("Generic error.");
                break;			
            default:
                sTemp += _T("Unknown error");
                break;
            }
            MessageBox(GetActiveWindow(),sTemp.c_str(),_T("ETViewer"),MB_ICONSTOP|MB_OK);
        }
    }
    UpdateFileMonitor();
    return (eErrorCode==eTraceReaderError_Success);
}

bool CETViewerApp::OpenCodeAddress(const TCHAR *pFile,DWORD dwLine,bool bShowErrorIfFailed)
{	
    bool result=false;

    if(_tcscmp(pFile,_T(""))==0){return false;}

    std::tstring sFile = pFile;

    bool bAccesible=false;
    if(_taccess(sFile.c_str(), 0))
    {
        for(unsigned x=0;x<theApp.m_SourceDirectories.size();x++)
        {
            const TCHAR *pRemainder=pFile;

            do
            {
                pRemainder=_tcschr(pRemainder,_T('\\'));
                if(pRemainder)
                {
                    std::tstring sTempFile = theApp.m_SourceDirectories[x];
                    sTempFile+=pRemainder;

                    if(_taccess(sTempFile.c_str(),0)==0)
                    {
                        sFile=sTempFile;
                        bAccesible=true;
                        break;
                    }
                    // Skip \\ for the next iteration
                    pRemainder=(pRemainder+1); 
                }
            }while(pRemainder);

            if(bAccesible){break;}
        }
    }
    else
    {
        bAccesible=true;
    }
    if(!bAccesible){return false;}
    result=m_SourceFileContainer.ShowFile(sFile.c_str(),dwLine,bShowErrorIfFailed);
    m_SourceFileContainer.BringWindowToTop();
    UpdateFileMonitor();
    return result;
}

bool CETViewerApp::OpenETL(const TCHAR *pFile)
{
    if(_tcscmp(pFile,_T(""))==0){return false;}

    m_pFrame->GetTracePane()->Clear();
    m_Controller.Stop();
    ProcessSessionChange();

    if(m_Controller.OpenLog(pFile,m_pFrame->GetTracePane())!=ERROR_SUCCESS)
    {
        m_Controller.StartRealTime(_T("ETVIEWER_SESSION"),m_pFrame->GetTracePane());
        ProcessSessionChange();
        return false;
    }
    else
    {
        theApp.AddRecentLogFile(pFile);
        ProcessSessionChange();
        return true;
    }
}

bool CETViewerApp::CreateETL(const TCHAR *pFile)
{
    if(_tcscmp(pFile,_T(""))==0){return false;}

    m_pFrame->GetTracePane()->Clear();
    m_Controller.Stop();
    ProcessSessionChange();

    if(m_Controller.CreateLog(pFile,m_pFrame->GetTracePane())!=ERROR_SUCCESS)
    {
        m_Controller.StartRealTime(_T("ETVIEWER_SESSION"),m_pFrame->GetTracePane());
        ProcessSessionChange();
        return false;
    }
    else
    {
        theApp.AddRecentLogFile(pFile);
        ProcessSessionChange();
        return true;
    }
}

void CETViewerApp::CloseETL()
{
    m_Controller.Stop();
    ProcessSessionChange();

    m_Controller.StartRealTime(_T("ETVIEWER_SESSION"),m_pFrame->GetTracePane());
    ProcessSessionChange();
}

void CETViewerApp::CloseSession()
{
    m_Controller.Stop();
    ProcessSessionChange();
}

void CETViewerApp::ProcessSessionChange()
{
    if(m_pFrame)
    {
        CProviderTree *pTree=m_pFrame->GetProviderTree();
        CETViewerView *pTraceViewer=m_pFrame->GetTracePane();
        m_pFrame->OnSessionTypeChanged();
        pTree->OnSessionTypeChanged();
        pTraceViewer->OnSessionTypeChanged();
    }
}

bool CETViewerApp::AddProvider(CTraceProvider *pProvider)
{
    CETViewerView *pTraceViewer = m_pFrame->GetTracePane();
    CProviderTree *pTree = m_pFrame->GetProviderTree();
    if(theApp.m_Controller.AddProvider(pProvider,pProvider->GetAllSupportedFlagsMask(),TRACE_LEVEL_VERBOSE))
    {
        //provider already exists, just added to that one.
        pTree->OnAddProvider(pProvider);
        pTraceViewer->OnAddProvider(pProvider);
        m_sProviders.insert(pProvider);
    }
    
    pTree->OnProvidersModified();
    pTraceViewer->OnProvidersModified();
    return true;
}

void CETViewerApp::RemoveProvider(CTraceProvider *pProvider)
{
    CProviderTree *pTree=m_pFrame->GetProviderTree();
    CETViewerView *pTraceViewer=m_pFrame->GetTracePane();
    theApp.m_Controller.RemoveProvider(pProvider);
    pTree->OnRemoveProvider(pProvider);
    pTraceViewer->OnRemoveProvider(pProvider);
    delete pProvider;
    m_sProviders.erase(pProvider);
    UpdateFileMonitor();
}

void CETViewerApp::RemoveAllProviders()
{
    CETViewerView *pTraceViewer=m_pFrame->GetTracePane();
    CProviderTree *pTree=m_pFrame->GetProviderTree();

    while(m_sProviders.size())
    {
        CTraceProvider *pProvider=*m_sProviders.begin();	
        theApp.m_Controller.RemoveProvider(pProvider);
        pTree->OnRemoveProvider(pProvider);
        pTraceViewer->OnRemoveProvider(pProvider);
        m_sProviders.erase(pProvider);
    }
    UpdateFileMonitor();
}

void CETViewerApp::ReloadProvider(CTraceProvider *pProvider)
{
    CETViewerView *pTraceViewer=m_pFrame->GetTracePane();
    CProviderTree *pTree=m_pFrame->GetProviderTree();

    std::vector<CTraceProvider *> loadedProviders;
    CTracePDBReader reader;

    for(const auto sCurFileName : pProvider->GetFileList())
    {
        reader.LoadFromPDB(sCurFileName.c_str(), &loadedProviders);
    }
    
    unsigned x;
    bool bReplaced=false;
    for(x=0;x<loadedProviders.size();x++)
    {
        CTraceProvider *pLoadedProvider=loadedProviders[x];
        if(!pLoadedProvider)
        {
            continue;
        }
        if(!bReplaced && pLoadedProvider->GetGUID()==pProvider->GetGUID())
        {
            bReplaced=true;
            m_sProviders.erase(pProvider);
            m_Controller.ReplaceProvider(pProvider,pLoadedProvider);
            pTree->OnReplaceProvider(pProvider,pLoadedProvider);
            pTraceViewer->OnReplaceProvider(pProvider,pLoadedProvider);
            m_sProviders.insert(pLoadedProvider);
            delete pProvider;
        }
        else
        {
            delete pLoadedProvider;
        }
    }
    if(bReplaced)
    {
        pTree->OnProvidersModified();
        pTraceViewer->OnProvidersModified();
    }
}

void CETViewerApp::ReloadAllProviders()
{
    CETViewerView *pTraceViewer=m_pFrame->GetTracePane();
    CProviderTree *pTree=m_pFrame->GetProviderTree();

    std::set<std::tstring> sPDBsToLoad;
    std::set<std::tstring>::iterator iPDB;
    std::set<CTraceProvider *>::iterator iProvider;
    for(iProvider=m_sProviders.begin();iProvider!=m_sProviders.end();iProvider++)
    {
        CTraceProvider *pProvider=*iProvider;
        for(const auto sCurFileName : pProvider->GetFileList())
        {
            sPDBsToLoad.insert(sCurFileName);
        }
    }
    bool bAnyReplaced=false;

    for(iPDB=sPDBsToLoad.begin();iPDB!=sPDBsToLoad.end();iPDB++)
    {
        std::tstring sPDB = *iPDB;
        std::vector<CTraceProvider *> loadedProviders;
        CTracePDBReader reader;
        reader.LoadFromPDB(sPDB.c_str(),&loadedProviders);
        unsigned x;
        for(x=0;x<loadedProviders.size();x++)
        {
            CTraceProvider *pLoadedProvider=loadedProviders[x];
            if(!pLoadedProvider)
            {
                continue;
            }

            CTraceProvider *pExistingProvider=NULL;
            for(iProvider=m_sProviders.begin();iProvider!=m_sProviders.end();iProvider++)
            {
                CTraceProvider *pProvider=*iProvider;				
                if(pProvider->GetGUID()==pLoadedProvider->GetGUID())
                {
                    pExistingProvider=pProvider;
                    break;
                }
            }
            if(pExistingProvider)
            {
                bAnyReplaced=true;
                m_sProviders.erase(pExistingProvider);
                m_Controller.ReplaceProvider(pExistingProvider,pLoadedProvider);
                pTree->OnReplaceProvider(pExistingProvider,pLoadedProvider);
                pTraceViewer->OnReplaceProvider(pExistingProvider,pLoadedProvider);
                m_sProviders.insert(pLoadedProvider);
                delete pExistingProvider;
            }
            else
            {
                delete pLoadedProvider;
            }
        }
    }
    if(bAnyReplaced)
    {
        pTree->OnProvidersModified();
        pTraceViewer->OnProvidersModified();
    }
}

bool CETViewerApp::ReloadPDBProviders(std::tstring sFileName)
{
    CETViewerView *pTraceViewer=m_pFrame->GetTracePane();
    CProviderTree *pTree=m_pFrame->GetProviderTree();

    bool bModified=false;
    std::vector<CTraceProvider *> loadedProviders;
    std::set<CTraceProvider *>::iterator iProvider;
    CTracePDBReader reader;
    if(reader.LoadFromPDB(sFileName.c_str(),&loadedProviders)!=eTraceReaderError_Success)
    {
        return false;
    }
    unsigned x;
    for(x=0;x<loadedProviders.size();x++)
    {
        CTraceProvider *pLoadedProvider=loadedProviders[x];
        if(!pLoadedProvider)
        {
            continue;
        }
        CTraceProvider *pExistingProvider=NULL;
        for(iProvider=m_sProviders.begin();iProvider!=m_sProviders.end();iProvider++)
        {
            CTraceProvider *pProvider=*iProvider;				
            if(pProvider && pProvider->GetGUID()==pLoadedProvider->GetGUID())
            {
                pExistingProvider=pProvider;
                break;
            }
        }
        if(pExistingProvider)
        {
            bModified=true;
            m_sProviders.erase(pExistingProvider);
            m_Controller.ReplaceProvider(pExistingProvider,pLoadedProvider);
            pTree->OnReplaceProvider(pExistingProvider,pLoadedProvider);
            pTraceViewer->OnReplaceProvider(pExistingProvider,pLoadedProvider);
            m_sProviders.insert(pLoadedProvider);
            delete pExistingProvider;
        }
        else
        {
            bModified=true;
            m_Controller.AddProvider(pLoadedProvider,pLoadedProvider->GetAllSupportedFlagsMask(),TRACE_LEVEL_VERBOSE);
            pTree->OnAddProvider(pLoadedProvider);
            pTraceViewer->OnAddProvider(pLoadedProvider);
            m_sProviders.insert(pLoadedProvider);
        }
    }

    if(bModified)
    {
        pTree->OnProvidersModified();
        pTraceViewer->OnProvidersModified();
    }
    return true;
}

bool CETViewerApp::FilterTrace(const TCHAR *pText)
{
    WaitForSingleObject(m_hInstantTraceMutex,INFINITE);

    bool res=false;

    TCHAR tempText[2048];
    tempText[0]=0;
    unsigned textLen=0;
    while(pText[textLen]!=0)
    {
        if(pText[textLen]>=_T('a') && pText[textLen]<=_T('z'))
        {
            tempText[textLen]=pText[textLen]-_T('a')+_T('A');
        }
        else
        {
            tempText[textLen]=pText[textLen];
        }
        textLen++;
    }
    tempText[textLen]=0;
        
    // Text Filters must always be ordered by relevance, the first filter that matches the criteria
    // is the effective filter 

    bool bPassed=true;
    unsigned x;
    for(x=0;x<m_dSplittedInstantFilters.size();x++)
    {
        int index=0,maxTextSearchSize=textLen-m_dSplittedInstantFilters[x].m_dwTextLen;
        if(maxTextSearchSize>0)
        {
            CFilter *pFilter=&m_dSplittedInstantFilters[x];
            const TCHAR *pFilterText=m_dSplittedInstantFilters[x].m_Text.c_str();
            if(pFilterText[0]==_T('*'))
            {
                bPassed=(pFilter->m_bInclusionFilter)?true:false;
                ReleaseMutex(m_hInstantTraceMutex);
                return bPassed;
            }

            while(index<=maxTextSearchSize)
            {
                if(memcmp(tempText+index,pFilterText,pFilter->m_dwTextLen)==0)
                {
                    bPassed=(pFilter->m_bInclusionFilter)?true:false;
                    ReleaseMutex(m_hInstantTraceMutex);
                    return bPassed;
                }
                index++;
            }
        }
    }

    ReleaseMutex(m_hInstantTraceMutex);

    return false;
}

void CETViewerApp::RefreshRecentFilesMenus()
{
    CMenu *pLogFilesMenu=m_pFrame->GetMenu()->GetSubMenu(0)->GetSubMenu(9);
    CMenu *pSourceFilesMenu=m_pFrame->GetMenu()->GetSubMenu(0)->GetSubMenu(8);
    CMenu *pPDBFilesMenu=m_pFrame->GetMenu()->GetSubMenu(0)->GetSubMenu(7);

    if(pSourceFilesMenu==NULL || pPDBFilesMenu==NULL || pLogFilesMenu==NULL){return;}
    unsigned x;
    while(pPDBFilesMenu->GetMenuItemCount()){pPDBFilesMenu->RemoveMenu(0,MF_BYPOSITION);}
    while(pSourceFilesMenu->GetMenuItemCount()){pSourceFilesMenu->RemoveMenu(0,MF_BYPOSITION);}
    while(pLogFilesMenu->GetMenuItemCount()){pLogFilesMenu->RemoveMenu(0,MF_BYPOSITION);}

    for(x=0;x<m_RecentPDBFiles.size();x++)
    {
        TCHAR *pText=(TCHAR*)m_RecentPDBFiles[x].c_str();
        pPDBFilesMenu->InsertMenu(x,MF_BYPOSITION,RECENT_PDB_FILE_BASE_INDEX+x,pText);
    }
    for(x=0;x<m_RecentSourceFiles.size();x++)
    {
        TCHAR *pText=(TCHAR*)m_RecentSourceFiles[x].c_str();
        pSourceFilesMenu->InsertMenu(x,MF_BYPOSITION,RECENT_SOURCE_FILE_BASE_INDEX+x,pText);
    }
    for(x=0;x<m_RecentLogFiles.size();x++)
    {
        TCHAR *pText=(TCHAR*)m_RecentLogFiles[x].c_str();
        pLogFilesMenu->InsertMenu(x,MF_BYPOSITION,RECENT_LOG_FILE_BASE_INDEX+x,pText);
    }

    if(m_RecentPDBFiles.size()==0){pPDBFilesMenu->InsertMenu(x,MF_BYPOSITION,RECENT_PDB_FILE_BASE_INDEX,_T("<No Recent File>"));}
    if(m_RecentSourceFiles.size()==0){pSourceFilesMenu->InsertMenu(x,MF_BYPOSITION,RECENT_SOURCE_FILE_BASE_INDEX,_T("<No Recent File>"));}
    if(m_RecentLogFiles.size()==0){pLogFilesMenu->InsertMenu(x,MF_BYPOSITION,RECENT_LOG_FILE_BASE_INDEX,_T("<No Recent File>"));}
}

void CETViewerApp::OnRecentPDBFile(UINT nID)
{
    if(m_RecentPDBFiles.size()==0){return;}
    std::tstring file = m_RecentPDBFiles[nID - RECENT_PDB_FILE_BASE_INDEX];
    m_pFrame->OpenFile(file.c_str(),NULL);
}

void CETViewerApp::OnRecentSourceFile(UINT nID)
{
    if(m_RecentSourceFiles.size()==0){return;}
    std::tstring file = m_RecentSourceFiles[nID - RECENT_SOURCE_FILE_BASE_INDEX];
    OpenCodeAddress(file.c_str(),0,true);
}

void CETViewerApp::OnRecentLogFile(UINT nID)
{
    if(m_RecentLogFiles.size()==0){return;}
    std::tstring file = m_RecentLogFiles[nID - RECENT_LOG_FILE_BASE_INDEX];
    m_pFrame->OpenFile(file.c_str(),NULL);
}

void CETViewerApp::OnClose()
{
    SaveTo(&m_ConfigFile,_T("Application"));
    m_pFrame->GetTracePane()->Save(&theApp.m_ConfigFile);
    m_ConfigFile.Save(m_sConfigFile.c_str());
    if(m_hSingleInstanceMutex){CloseHandle(m_hSingleInstanceMutex);m_hSingleInstanceMutex=NULL;}

    CloseSession();
}

int CETViewerApp::ExitInstance()
{
    int res=CWinApp::ExitInstance();
    CoUninitialize();
    return res;
}

void CETViewerApp::UpdateFileAssociations()
{
    if(m_bAssociatePDB){SetFileAssociation(_T(".pdb"),_T("pdbfile"),_T("Program Database"),_T("-pdb:\"%1\""));}
    if(m_bAssociateETL){SetFileAssociation(_T(".etl"),_T("etlfile"),_T("Event Tracing for Windows (ETW) Log"),_T("-etl:\"%1\""));}
    if(m_bAssociateSources)
    {
        SetFileAssociation(_T(".cpp"), _T("cppfile"), _T("C++ Source File"), _T("-src:\"%1\""));
        SetFileAssociation(_T(".c"), _T("cfile"), _T("C Source File"), _T("-src:\"%1\""));
        SetFileAssociation(_T(".h"), _T("hfile"), _T("Header File"), _T("-src:\"%1\""));
        SetFileAssociation(_T(".inl"), _T("inlfile"), _T("Inline File"), _T("-src:\"%1\""));
    }
}

void CETViewerApp::OnFileChanged(std::tstring sFile)
{
    AddFileChangeOperation(sFile);
}

void CETViewerApp::AddFileChangeOperation(std::tstring sFileName)
{
    WaitForSingleObject(m_hPendingFileChangesMutex,INFINITE);
    bool bFound=false;
    std::list<SPendingFileChangeOperation>::iterator i;
    for(i=m_lPendingFileChanges.begin();i!=m_lPendingFileChanges.end();i++)
    {
        SPendingFileChangeOperation op=*i;
        if(op.sFileName==sFileName)
        {
            bFound=true;
            break;
        }
    }
    if(!bFound)
    {
        SPendingFileChangeOperation op;
        op.sFileName=sFileName;
        op.dwChangeTime=GetTickCount();
        m_lPendingFileChanges.push_back(op);
    }
    ReleaseMutex(m_hPendingFileChangesMutex);
}

void CETViewerApp::RemoveFileChangeOperation(std::tstring sFileName)
{
    WaitForSingleObject(m_hPendingFileChangesMutex,INFINITE);

    std::list<SPendingFileChangeOperation>::iterator i;
    for(i=m_lPendingFileChanges.begin();i!=m_lPendingFileChanges.end();)
    {
        SPendingFileChangeOperation op=*i;
        if(op.sFileName==sFileName)
        {
            i=m_lPendingFileChanges.erase(i);
        }
        else
        {
            i++;
        }
    }

    ReleaseMutex(m_hPendingFileChangesMutex);
}

void CETViewerApp::RemoveExpiredFileChangeOperations()
{
    WaitForSingleObject(m_hPendingFileChangesMutex,INFINITE);

    std::list<SPendingFileChangeOperation>::iterator i;
    for(i=m_lPendingFileChanges.begin();i!=m_lPendingFileChanges.end();)
    {
        SPendingFileChangeOperation op=*i;
        if((op.dwChangeTime+FILE_CHANGE_EXPIRATION_TIME)<GetTickCount())
        {
            i=m_lPendingFileChanges.erase(i);
        }
        else
        {
            i++;
        }
    }

    ReleaseMutex(m_hPendingFileChangesMutex);
}

void CETViewerApp::CheckFileChangeOperations()
{
    if(m_bCheckingFileChangeOperations){return;}
    m_bCheckingFileChangeOperations=true;

    if( m_eSourceMonitoringMode==eFileMonitoringMode_AutoReload ||
        m_ePDBMonitoringMode==eFileMonitoringMode_AutoReload)
    {
        RemoveExpiredFileChangeOperations();
    }

    WaitForSingleObject(m_hPendingFileChangesMutex,INFINITE);

    bool bRetryOperation=false;

    std::list<SPendingFileChangeOperation>::iterator i;
    for(i=m_lPendingFileChanges.begin();i!=m_lPendingFileChanges.end();)
    {
        SPendingFileChangeOperation op=*i;
        TCHAR sExt[MAX_PATH]={0};
        _tsplitpath_s(op.sFileName.c_str(),NULL,0,NULL,0,NULL,0,sExt,MAX_PATH);
        bool bRemoveOperation=false;
        bool bIsPDB=(_tcsicmp(sExt,_T(".PDB"))==0);
        bool bMustAsk=(bIsPDB && m_ePDBMonitoringMode==eFileMonitoringMode_Ask) ||	
                      (!bIsPDB && m_eSourceMonitoringMode==eFileMonitoringMode_Ask);

        if(!bRetryOperation)
        {
            if(bMustAsk)
            {
                std::tstring sQuestion;
                sQuestion=_T("File '");
                sQuestion+=op.sFileName;
                sQuestion+=_T("' has changed.\r\n\r\nDo you want to reload it?");

                if(MessageBox(m_pFrame->m_hWnd,sQuestion.c_str(),_T("ETViewer"),MB_ICONQUESTION|MB_YESNO)==IDNO)
                {
                    bRemoveOperation=true;
                }
            }
        }

        bRetryOperation=false;

        if(!bRemoveOperation)
        {
            bool bReloadOk=true;

            if(bIsPDB)
            {
                bReloadOk=ReloadPDBProviders(op.sFileName.c_str());
            }
            else 
            {
                m_SourceFileContainer.ReloadFile(op.sFileName.c_str());
            }

            if(bReloadOk)
            {
                bRemoveOperation=true;
            }
            else
            {
                if(bMustAsk)
                {
                    std::tstring sQuestion;
                    sQuestion=_T("File '");
                    sQuestion+=op.sFileName;
                    sQuestion+=_T("' cannot be reloaded.\r\n\r\nDo you want to retry?");

                    if(MessageBox(m_pFrame->m_hWnd,sQuestion.c_str(),_T("ETViewer"),MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
                    {
                        bRetryOperation=true;
                    }
                    else
                    {
                        bRemoveOperation=true;
                    }
                }
            }
        }

        if(bRemoveOperation)
        {
            i=m_lPendingFileChanges.erase(i);
        }
        else if(!bRetryOperation)
        {
            i++;
        }
    }

    ReleaseMutex(m_hPendingFileChangesMutex);

    m_bCheckingFileChangeOperations=false;
};

void CETViewerApp::UpdateFileMonitor()
{
    std::set<std::tstring> sFiles;
    
    if(m_eSourceMonitoringMode!=eFileMonitoringMode_None &&
        m_eSourceMonitoringMode!=eFileMonitoringMode_Ignore )
    {
        m_SourceFileContainer.GetFiles(&sFiles);
    }

    if(m_ePDBMonitoringMode!=eFileMonitoringMode_None &&
        m_ePDBMonitoringMode!=eFileMonitoringMode_Ignore )
    {
        std::set<CTraceProvider *>::iterator i;
        for(auto curProvider : m_sProviders)
        {
            for(const auto curFileName : curProvider->GetFileList())
            {
                sFiles.insert(curFileName);
            }
        }
    }

    m_pFileMonitor->SetFiles(&sFiles);
}
