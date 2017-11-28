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
#include "ETViewer.h"

#include "MainFrm.h"
#include "ProviderTree.h"
#include "LeftPane.h"
#include "ETViewerView.h"
#include "HighLightPane.h"
#include "SettingsDialog.h"
#include ".\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD g_dwRegisteredMessage=RegisterWindowMessage(_T("ETVIEWER-IPC"));
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_DROPFILES()
    ON_BN_CLICKED(IDC_BT_START_STOP_CAPTURE, OnStartStop)
    ON_BN_CLICKED(IDC_BT_OPEN_FILE, OnOpenFile)
    ON_BN_CLICKED(IDC_BT_SHOW_SOURCE_CONTAINER, OnShowSourceContainer)
    ON_BN_CLICKED(IDC_BT_CLEAR, OnClear)
    ON_BN_CLICKED(IDC_BT_CLEAR_SELECTED, OnClearSelected)
    ON_BN_CLICKED(IDC_BT_HIGHLIGHT_FILTERS, OnHighlightFilters)
    ON_BN_CLICKED(IDC_BT_ERRORLOOKUP, OnErrorLookup)
    ON_BN_CLICKED(IDC_BT_ENSURE_VISIBLE, OnEnsureVisible)
    ON_BN_CLICKED(IDC_BT_SAVE, OnSave)
    ON_BN_CLICKED(IDC_BT_FIND, OnFind)
    ON_BN_CLICKED(IDOK, OnOk)
    ON_BN_CLICKED(IDCANCEL, OnCancel)
    ON_COMMAND(ID_FILE_OPEN, OnOpenFile)
    ON_COMMAND(ID_FILE_CLOSELOG, OnCloseLog)
    ON_UPDATE_COMMAND_UI(ID_FILE_CLOSELOG, OnUpdateFileCloselog)
    ON_COMMAND(ID_FILE_LOGTOFILE, OnFileLogtofile)
    ON_UPDATE_COMMAND_UI(ID_FILE_LOGTOFILE, OnUpdateFileLogtofile)
    ON_COMMAND(ID_FILE_STOPLOGGINTOFILE, OnFileStoploggintofile)
    ON_UPDATE_COMMAND_UI(ID_FILE_STOPLOGGINTOFILE, OnUpdateFileStoploggintofile)
    ON_REGISTERED_MESSAGE(g_dwRegisteredMessage,OnIPCCommand)
    ON_COMMAND(ID_EDIT_SETTINGS, OnEditSettings)
    ON_WM_TIMER()
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // Indicador de línea de estado
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};


// Construcción o destrucción de CMainFrame

CMainFrame::CMainFrame()
{
    // TODO: agregar aquí el código de inicialización adicional de miembros
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_MainDialogBar.Create(this, IDD_MAIN_DIALOG_BAR, 
        CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
    {
        TRACE0("No se pudo crear el control dialogbar\n");
        return -1;		// No se pudo crear
    }

    if (!m_FilterDialogBar.Create(this, IDD_FILTER_DIALOG_BAR, 
        CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
    {
        TRACE0("No se pudo crear el control dialogbar\n");
        return -1;		// No se pudo crear
    }

    if (!m_wndReBar.Create(this,RBS_BANDBORDERS) ||
        !m_wndReBar.AddBar(&m_MainDialogBar) ||
        !m_wndReBar.AddBar(&m_FilterDialogBar))
    {
        TRACE0("No se pudo crear el control rebar\n");
        return -1;      // No se pudo crear
    }

    // TODO: quitarlo si no desea información sobre herramientas
    m_FilterDialogBar.SetBarStyle(m_FilterDialogBar.GetBarStyle() |	CBRS_TOOLTIPS | CBRS_FLYBY );
    m_MainDialogBar.SetBarStyle(m_MainDialogBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY );
    m_MainDialogBar.InitDialogBar();
    m_FilterDialogBar.InitDialogBar();
    m_wndReBar.SendMessage(RB_MAXIMIZEBAND,1);

    SetTimer(FILE_CHANGE_TIMER_ID,FILE_CHANGE_TIMER_PERIOD,NULL);

    ::DragAcceptFiles(m_hWnd,TRUE);
    return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
    CCreateContext* pContext)
{
    // Crear ventana divisora
    if (!m_wndSplitter.CreateStatic(this, 1, 2))
        return FALSE;

    if (!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CETViewerView), CSize(100, 100), pContext) ||
        !m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftPane), CSize(250, 100), pContext))
    {
        m_wndSplitter.DestroyWindow();
        return FALSE;
    }

    if(S_OK != theApp.m_Controller.StartRealTime(_T("ETVIEWER_SESSION"), GetTracePane()))
    {
        MessageBox(_T("Could not initialize RealTime Trace Session"), _T("ETVIEWER"));
    }

    return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;

    cs.style&=~FWS_ADDTOTITLE;
    cs.style&=~FWS_PREFIXTITLE ;
    return TRUE;
}


// Diagnósticos de CMainFrame

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// Controladores de mensaje de CMainFrame

CETViewerView* CMainFrame::GetTracePane()
{
    if(m_wndSplitter.m_hWnd==NULL){return NULL;}
    CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
    CETViewerView* pView = DYNAMIC_DOWNCAST(CETViewerView, pWnd);
    return pView;
}

CProviderTree* CMainFrame::GetProviderTree()
{
    if(m_wndSplitter.m_hWnd==NULL){return NULL;}
    CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
    CLeftPane* pLeftPane = DYNAMIC_DOWNCAST(CLeftPane, pWnd);
    CProviderTree* pProviderTree=DYNAMIC_DOWNCAST(CProviderTree,pLeftPane->m_wndSplitter.GetPane(0, 0));
    return pProviderTree;
}

CHighLightPane* CMainFrame::GetHighLightPane()
{
    if(m_wndSplitter.m_hWnd==NULL){return NULL;}
    CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
    CLeftPane* pLeftPane = DYNAMIC_DOWNCAST(CLeftPane, pWnd);
    CHighLightPane* pHighLightPane=DYNAMIC_DOWNCAST(CHighLightPane,pLeftPane->m_wndSplitter.GetPane(1, 0));
    return pHighLightPane;
}

void CMainFrame::OnDestroy()
{
    theApp.OnClose();
    CFrameWnd::OnDestroy();
}

bool CMainFrame::IsAncestorOf(HWND hWnd,HWND hAncestor)
{
    HWND hParent=hWnd;
    do
    {
        hParent=::GetParent(hParent);
        if(hParent==hAncestor){return true;}
    } 
    while(hParent);
    return false;
}

void CMainFrame::OnOk()
{
    HWND hWnd=::GetFocus();
    if(IsAncestorOf(hWnd,m_FilterDialogBar.m_hWnd))
    {
        m_FilterDialogBar.OnOk();
    }
}

void CMainFrame::OnCancel()
{
    HWND hWnd=::GetFocus();
    if(IsAncestorOf(hWnd,m_FilterDialogBar.m_hWnd))
    {
        m_FilterDialogBar.OnCancel();
    }
}

void CMainFrame::OnClear()
{
    GetTracePane()->Clear();
}

void CMainFrame::OnClearSelected()
{
    GetTracePane()->ClearSelected();
}

void CMainFrame::OnHighlightFilters()
{
    GetTracePane()->OnHighLightFilters();
}

void CMainFrame::LookupError(const TCHAR *pErrorString)
{
    TCHAR finalMessage[2000]={0},message[2000]={0},errorStr[2000]={0};
    TCHAR *pValidChars=_T("0x1234567890abcdefABCDEFX");
    unsigned  len=(unsigned )_tcslen(pErrorString),added=0;
    for(unsigned x=0;x<len;x++)
    {
        if(_tcschr(pValidChars,pErrorString[x])!=0)
        {
            errorStr[added]=pErrorString[x];
            added++;
        }
        else
        {
            // a invalid TCHAR after valid TCHARs marks the end.
            if(added){break;}
        }
    }
    if(_tcscmp(pErrorString,errorStr)!=0){m_MainDialogBar.m_EDErrorLookup.SetWindowText(errorStr);}


    DWORD errorCode=0;
    bool ok = (errorCode = _ttoi(errorStr)) != 0;
    if (!ok){ ok = (_stscanf_s(errorStr, _T("%x"), &errorCode) != 0); }
    if (!ok){ ok = (_stscanf_s(errorStr, _T("x%x"), &errorCode) != 0); }
    if (!ok){ ok = (_stscanf_s(errorStr, _T("0x%x"), &errorCode) != 0); }
    if(errorCode==0){return;}

    if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,errorCode,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &message,_countof(message), NULL)!=0)
    {
        _stprintf_s(finalMessage, _T("%s=%s"), errorStr, message);
        MessageBox(finalMessage,_T("ETViewer"),MB_OK|MB_ICONINFORMATION);
    }
    else
    {
        _stprintf_s(finalMessage,_T("Cannot find the error %s"),errorStr);
        MessageBox(finalMessage,_T("ETViewer"),MB_OK|MB_ICONSTOP);
    }
}

void CMainFrame::OnErrorLookup()
{
    TCHAR temp[1024]={0};
    int count=m_MainDialogBar.m_EDErrorLookup.GetWindowText(temp,_countof(temp));
    LookupError(temp);
    m_MainDialogBar.m_EDErrorLookup.SetFocus();
}

void CMainFrame::OnEnsureVisible()
{
    GetTracePane()->EnableShowLastTrace(!GetTracePane()->IsShowLastTraceEnabled());
    m_MainDialogBar.UpdateBitmaps();
}

void CMainFrame::OnSave()
{
    GetTracePane()->OnSave();
}

void CMainFrame::OnFind()
{
    GetTracePane()->OnFind();
}


bool CMainFrame::OpenFile(const TCHAR *pFile,bool *pbKnownFileType)
{
    TCHAR sExtension[MAX_PATH]={0};
    _tsplitpath_s(pFile, NULL, 0, NULL, 0, NULL, 0, sExtension, MAX_PATH);
    _tcsupr_s(sExtension);
    if(_tcscmp(sExtension,_T(".PDB"))==0)
    {
        if(pbKnownFileType){*pbKnownFileType=true;}
        return theApp.OpenPDB(pFile,true);
    }
    else if(_tcscmp(sExtension,_T(".C"))==0 || _tcscmp(sExtension,_T(".H"))==0 || 
        _tcscmp(sExtension,_T(".CPP"))==0 || _tcscmp(sExtension,_T(".INL"))==0)
    {
        if(pbKnownFileType){*pbKnownFileType=true;}
        return theApp.OpenCodeAddress(pFile,0,true);
    }
    else if(_tcscmp(sExtension,_T(".ETL"))==0)
    {
        if(pbKnownFileType){*pbKnownFileType=true;}
        return theApp.OpenETL(pFile);
    }
    else
    {
        if(pbKnownFileType){*pbKnownFileType=false;}
    }
    return false;
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
    // TODO: Add your message handler code here and/or call default
    bool bFilesOfUnknownFormat=false;
    UINT nFiles=DragQueryFile(hDropInfo,0xFFFFFFFF,NULL,0);
    for(unsigned x=0;x<nFiles;x++)
    {
        bool bKnownType=false;
        TCHAR sFullFilePath[MAX_PATH]={0};
        DragQueryFile(hDropInfo,x,sFullFilePath,MAX_PATH);
        OpenFile(sFullFilePath,&bKnownType);
        if(!bKnownType){bFilesOfUnknownFormat=true;}
    }
    if(bFilesOfUnknownFormat)
    {
        MessageBox(_T("Unknown files dropped"),_T("ETViewer"),MB_ICONSTOP|MB_OK);
    }
    CFrameWnd::OnDropFiles(hDropInfo);
}

void CMainFrame::OnStartStop()
{
    theApp.m_Controller.Pause(!theApp.m_Controller.IsPaused());
    m_MainDialogBar.UpdateBitmaps();
}

void CMainFrame::OnOpenFile()
{
    TCHAR sTempBuffer[1024*10]={0};
    CFileDialog dialog(TRUE,_T("pdb"),NULL,OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST,_T("All Supported Files|*.pdb;*.etl;*.c;*.cpp;*.h;*.inl|PDB Files(*.pdb)|*.pdb|Event Tracing Log Files(*.etl)|*.etl|Source Files(*.c *.cpp *.h *.inl)|*.c;*.cpp;*.h;*.inl|All Files(*.*)|*.*|"));
    dialog.GetOFN().lpstrFile=sTempBuffer;
    dialog.GetOFN().nMaxFile=_countof(sTempBuffer);

    if(dialog.DoModal()==IDOK)
    {
        std::tstring sPath;
        TCHAR *pString=dialog.GetOFN().lpstrFile;

        //lpstrFile es un array de cadenas, si solo hay una cadena solo se ha seleccionado un archivo,
        // en caso contrario la primera cadena es el directorio y las demas son los nombres de los archivos.

        if(pString[_tcslen(pString)+1]!=0)
        {
            sPath=pString;
            sPath+=_T("\\");
            pString=pString+_tcslen(pString)+1;
        }
        while(pString[0]!=0)
        {
            std::tstring sProvider = sPath;
            sProvider+=pString;
            OpenFile(sProvider.c_str(),NULL);
            pString=pString+_tcslen(pString)+1;
        }
    }
}

void CMainFrame::OnShowSourceContainer()
{
    theApp.m_SourceFileContainer.ShowWindow(SW_SHOW);
}

void CMainFrame::OnCloseLog()
{
    if(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_ReadLog)
    {
        theApp.CloseETL();
    }
}
void CMainFrame::OnUpdateFileCloselog(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_ReadLog);
}

void CMainFrame::OnFileLogtofile()
{
    CFileDialog dialog(FALSE,_T("etl"),NULL,OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,_T("Event Tracing Log Files(*.etl)|*.etl|All Files(*.*)|*.*"));
    if(dialog.DoModal()==IDOK)
    {
        theApp.CreateETL(dialog.GetOFN().lpstrFile);
    }
}

void CMainFrame::OnUpdateFileLogtofile(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(theApp.m_Controller.GetSessionType()!=eTraceControllerSessionType_CreateLog);
}

void CMainFrame::OnFileStoploggintofile()
{
    if(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_CreateLog)
    {
        theApp.CloseETL();
    }
}

void CMainFrame::OnUpdateFileStoploggintofile(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_CreateLog);
}

void CMainFrame::OnSessionTypeChanged()
{
    std::tstring sCaption;
    switch(theApp.m_Controller.GetSessionType())
    {
    case eTraceControllerSessionType_None:
        sCaption=_T("ETViewer - No session");
        break;
    case eTraceControllerSessionType_RealTime:
        sCaption=_T("ETViewer - Real Time");
        break;	
    case eTraceControllerSessionType_ReadLog:
        sCaption=_T("ETViewer - Open Log File '");
        sCaption+=theApp.m_Controller.GetFileName();
        sCaption+=_T("'");
        break;	

    case eTraceControllerSessionType_CreateLog:
        sCaption=_T("ETViewer - Logging to File '");
        sCaption+=theApp.m_Controller.GetFileName();
        sCaption+=_T("'");
        break;	
    }
    SetWindowText(sCaption.c_str());

    m_MainDialogBar.OnSessionTypeChanged();
    m_FilterDialogBar.OnSessionTypeChanged();
}

LRESULT CMainFrame::OnIPCCommand(WPARAM wParam,LPARAM lParam)
{
    TCHAR sCommandLine[1024*10]={0};
    SetForegroundWindow();
    BringToTop(SW_SHOW);
    GlobalGetAtomName(wParam,sCommandLine,_countof(sCommandLine));
    TCHAR *pArgs[]={sCommandLine};
    theApp.ProcessCommandLine(1,pArgs);
    return 0L;
};

void CMainFrame::OnEditSettings()
{
    CSettingsDialog dialog;
    dialog.DoModal();	
    theApp.UpdateFileMonitor();
}

void CMainFrame::OnTimer( UINT nTimerId)
{
    if(nTimerId==FILE_CHANGE_TIMER_ID)
    {
        theApp.CheckFileChangeOperations();
    }
}

