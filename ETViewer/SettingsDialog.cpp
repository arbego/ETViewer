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
#include "ETViewerDoc.h"
#include "ETViewerView.h"
#include "SettingsDialog.h"
#include ".\settingsdialog.h"


// CSettingsDialog dialog

IMPLEMENT_DYNAMIC(CSettingsDialog, CDialog)
CSettingsDialog::CSettingsDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSettingsDialog::IDD, pParent)
{
    m_dwTraceFontSize=0;
    m_OldListViewProc=NULL;
}

CSettingsDialog::~CSettingsDialog()
{
}

void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LW_SOURCE_PATHS, m_LWSourcePaths);
    DDX_Control(pDX, IDC_BT_ADD, m_BTAdd);
    DDX_Control(pDX, IDC_BT_REMOVE, m_BTRemove);
    DDX_Control(pDX, IDC_BT_UP, m_BTUp);
    DDX_Control(pDX, IDC_BT_DOWN, m_BTDown);
    DDX_Control(pDX, IDC_CB_ASSOCIATE_PDB, m_CBAssociatePDB);
    DDX_Control(pDX, IDC_CB_ASSOCIATE_ETL, m_CBAssociateETL);
    DDX_Control(pDX, IDC_CB_ASSOCIATE_SOURCES, m_CBAssociateSources);
    DDX_Control(pDX, IDC_ST_FONT_FAMILY, m_STFontSample);
    DDX_Control(pDX, IDC_BT_SELECT_FONT, m_BTSelectFont);
    DDX_Control(pDX, IDC_RB_PDB_RELOAD_AUTO, m_RBPDBReloadAuto);
    DDX_Control(pDX, IDC_RB_PDB_RELOAD_ASK, m_RBPDBReloadAsk);
    DDX_Control(pDX, IDC_RB_PDB_RELOAD_DISABLED, m_RBPDBReloadDisabled);
    DDX_Control(pDX, IDC_RB_SOURCE_RELOAD_AUTO, m_RBSourceReloadAuto);
    DDX_Control(pDX, IDC_RB_SOURCE_RELOAD_ASK, m_RBSourceReloadAsk);
    DDX_Control(pDX, IDC_RB_SOURCE_RELOAD_DISABLED, m_RBSourceReloadDisabled);
    DDX_Control(pDX, IDOK, m_BTOk);
    DDX_Control(pDX, IDCANCEL, m_BTCancel);
}


BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
    ON_BN_CLICKED(IDC_BT_ADD, OnAddSourcePath)
    ON_BN_CLICKED(IDC_BT_REMOVE, OnRemoveSourcePath)
    ON_BN_CLICKED(IDC_BT_UP, OnMoveSourceUp)
    ON_BN_CLICKED(IDC_BT_DOWN, OnMoveSourceDown)
    ON_BN_CLICKED(IDC_BT_SELECT_FONT, OnSelectFont)
    ON_BN_CLICKED(IDC_RB_PDB_RELOAD_AUTO, OnPDBReloadChanged)
    ON_BN_CLICKED(IDC_RB_PDB_RELOAD_ASK, OnPDBReloadChanged)
    ON_BN_CLICKED(IDC_RB_PDB_RELOAD_DISABLED, OnPDBReloadChanged)
    ON_BN_CLICKED(IDC_RB_SOURCE_RELOAD_AUTO, OnSourceReloadChanged)
    ON_BN_CLICKED(IDC_RB_SOURCE_RELOAD_ASK, OnSourceReloadChanged)
    ON_BN_CLICKED(IDC_RB_SOURCE_RELOAD_DISABLED, OnSourceReloadChanged)
    ON_BN_CLICKED(IDOK, OnOk)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


void CSettingsDialog::OnSelectFont()
{
    HDC hdc=::GetDC(m_hWnd);
    LOGFONT logFont={0};
    logFont.lfHeight = m_dwTraceFontSize;
    _tcscpy_s(logFont.lfFaceName, m_sTraceFont.c_str());
    
    CFontDialog dialog(&logFont);
    if(dialog.DoModal()==IDOK)
    {
        m_dwTraceFontSize= -logFont.lfHeight;
        m_sTraceFont=logFont.lfFaceName;
        UpdateFont();
    }

    ::ReleaseDC(m_hWnd,hdc);
}

void CSettingsDialog::OnPDBReloadChanged()
{
    if(m_RBPDBReloadAuto.GetCheck()==BST_CHECKED){theApp.m_ePDBMonitoringMode=eFileMonitoringMode_AutoReload;}
    else if(m_RBPDBReloadAsk.GetCheck()==BST_CHECKED){theApp.m_ePDBMonitoringMode=eFileMonitoringMode_Ask;}
    else {theApp.m_ePDBMonitoringMode=eFileMonitoringMode_Ignore;}
}

void CSettingsDialog::OnSourceReloadChanged()
{
    if(m_RBSourceReloadAuto.GetCheck()==BST_CHECKED){theApp.m_eSourceMonitoringMode=eFileMonitoringMode_AutoReload;}
    else if(m_RBSourceReloadAsk.GetCheck()==BST_CHECKED){theApp.m_eSourceMonitoringMode=eFileMonitoringMode_Ask;}
    else {theApp.m_eSourceMonitoringMode=eFileMonitoringMode_Ignore;}
}

BOOL CSettingsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_BTUp	.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_UP)));
    m_BTDown.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_DOWN)));

    m_LWSourcePaths.InsertColumn(1,_T("Path"),LVCFMT_LEFT,400,0);
    m_LWSourcePaths.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_INFOTIP);
    
    m_OldListViewProc=(WNDPROC)GetWindowLong(m_LWSourcePaths.m_hWnd,GWL_WNDPROC);
    SetWindowLong(m_LWSourcePaths.m_hWnd,GWL_USERDATA,(DWORD)this);
    SetWindowLong(m_LWSourcePaths.m_hWnd,GWL_WNDPROC,(DWORD)ListViewProc);

    m_CBAssociateETL.SetCheck(theApp.m_bAssociateETL?BST_CHECKED:BST_UNCHECKED);
    m_CBAssociatePDB.SetCheck(theApp.m_bAssociatePDB?BST_CHECKED:BST_UNCHECKED);
    m_CBAssociateSources.SetCheck(theApp.m_bAssociateSources?BST_CHECKED:BST_UNCHECKED);

    m_RBPDBReloadAuto.SetCheck(theApp.m_ePDBMonitoringMode==eFileMonitoringMode_AutoReload?BST_CHECKED:BST_UNCHECKED);
    m_RBPDBReloadAsk.SetCheck(theApp.m_ePDBMonitoringMode==eFileMonitoringMode_Ask?BST_CHECKED:BST_UNCHECKED);
    m_RBPDBReloadDisabled.SetCheck(theApp.m_ePDBMonitoringMode==eFileMonitoringMode_Ignore || theApp.m_ePDBMonitoringMode==eFileMonitoringMode_None?BST_CHECKED:BST_UNCHECKED);

    m_RBSourceReloadAuto.SetCheck(theApp.m_eSourceMonitoringMode==eFileMonitoringMode_AutoReload?BST_CHECKED:BST_UNCHECKED);
    m_RBSourceReloadAsk.SetCheck(theApp.m_eSourceMonitoringMode==eFileMonitoringMode_Ask?BST_CHECKED:BST_UNCHECKED);
    m_RBSourceReloadDisabled.SetCheck(theApp.m_eSourceMonitoringMode==eFileMonitoringMode_Ignore || theApp.m_eSourceMonitoringMode==eFileMonitoringMode_None?BST_CHECKED:BST_UNCHECKED);

    theApp.m_pFrame->GetTracePane()->GetTraceFont(&m_sTraceFont,&m_dwTraceFontSize);
    UpdateFont();

    DWORD x;
    for(x=0;x<theApp.m_SourceDirectories.size();x++)
    {
        m_LWSourcePaths.InsertItem(x,theApp.m_SourceDirectories[x].c_str());
    }
    return TRUE;
}

LRESULT CALLBACK CSettingsDialog::ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CSettingsDialog *pThis=(CSettingsDialog *)GetWindowLong(hwnd,GWL_USERDATA);
    if(uMsg==WM_KEYDOWN)
    {
        bool pushedLControl=(GetKeyState(VK_LCONTROL)>>15)?true:false;
        bool pushedRControl=(GetKeyState(VK_RCONTROL)>>15)?true:false;
        if(wParam==VK_UP && (pushedLControl||pushedRControl))  {pThis->OnMoveSourceUp();return 0;}
        if(wParam==VK_DOWN && (pushedLControl||pushedRControl)){pThis->OnMoveSourceDown();return 0;}
        if(wParam==VK_INSERT){pThis->OnInsertSourcePath();return 0;}
        if(wParam==VK_DELETE){pThis->OnRemoveSourcePath();return 0;}
    }

    return CallWindowProc(pThis->m_OldListViewProc,hwnd,uMsg,wParam,lParam);
}

void CSettingsDialog::OnOk()
{
    theApp.m_SourceDirectories.clear();
    int x;
    for(x=0;x<m_LWSourcePaths.GetItemCount();x++)
    {
        TCHAR sTempText[1024]={0};
        LVITEM item={0};
        item.iItem=x;
        item.mask=LVIF_TEXT;
        item.pszText=sTempText;
        item.cchTextMax=_countof(sTempText);
        m_LWSourcePaths.GetItem(&item);
        theApp.m_SourceDirectories.push_back(sTempText);
    }

    theApp.m_bAssociateETL=(m_CBAssociateETL.GetCheck()==BST_CHECKED);
    theApp.m_bAssociatePDB=(m_CBAssociatePDB.GetCheck()==BST_CHECKED);
    bool bSources=(m_CBAssociateSources.GetCheck()==BST_CHECKED);

    if(!theApp.m_bAssociateSources && bSources)
    {
        if(MessageBox(_T("Do you really want to associate Source Files with ETViewer?"),_T("ETViewer"),MB_ICONEXCLAMATION|MB_YESNO)==IDYES)
        {
            theApp.m_bAssociateSources=bSources;
        }
    }
    else
    {
        theApp.m_bAssociateSources=bSources;
    }
    theApp.UpdateFileAssociations();

    theApp.m_pFrame->GetTracePane()->SetTraceFont(m_sTraceFont,m_dwTraceFontSize);

    OnOK();
}

void CSettingsDialog::UpdateFont()
{
    HDC hdc=::GetDC(m_hWnd);

    m_TraceFont.DeleteObject();
    m_TraceFont.CreateFont(m_dwTraceFontSize,0,0,0,0,0,0,0,0,0,0,0,0,m_sTraceFont.c_str());

    TCHAR sFontText[1024]={0};
    DWORD dwTraceFontSize=MulDiv(m_dwTraceFontSize,72,::GetDeviceCaps(hdc,LOGPIXELSY));
    _stprintf_s(sFontText,_T("%s, %d"),m_sTraceFont.c_str(),dwTraceFontSize);
    m_STFontSample.SetFont(&m_TraceFont);
    m_STFontSample.SetWindowText(sFontText);
    ::ReleaseDC(m_hWnd,hdc);

}

void CSettingsDialog::AddSourcePath(int index)
{
    TCHAR sTempPath[MAX_PATH]={0};
    BROWSEINFO browseInfo={0};
    browseInfo.hwndOwner=m_hWnd;
    browseInfo.lpszTitle=_T("Select the source path to add");
    browseInfo.pszDisplayName=sTempPath;
    browseInfo.ulFlags|=BIF_RETURNONLYFSDIRS;
    LPITEMIDLIST pResult=SHBrowseForFolder(&browseInfo);
    if(pResult)
    {
        SHGetPathFromIDList(pResult,sTempPath);
        int pathLen=_tcslen(sTempPath);
        if(pathLen)
        {
            if(sTempPath[pathLen-1]==_T('\\')){sTempPath[pathLen-1]=0;}
            m_LWSourcePaths.InsertItem(index,sTempPath);
            CoTaskMemFree(pResult);
        }
    }
}
void CSettingsDialog::OnAddSourcePath()
{
    AddSourcePath(m_LWSourcePaths.GetItemCount());
}

void CSettingsDialog::OnInsertSourcePath()
{
    int sel=m_LWSourcePaths.GetNextItem(-1,LVNI_SELECTED);
    int index=sel==-1?m_LWSourcePaths.GetItemCount():sel;
    AddSourcePath(index);
}

void CSettingsDialog::OnRemoveSourcePath()
{
    int sel=m_LWSourcePaths.GetNextItem(-1,LVNI_SELECTED);
    if(sel!=-1)
    {
        m_LWSourcePaths.DeleteItem(sel);
    }
}

void CSettingsDialog::OnMoveSourceUp()
{
    int sel=m_LWSourcePaths.GetNextItem(-1,LVNI_SELECTED);
    if(sel<1){return;}

    SwapItems(sel,sel-1);
}

void CSettingsDialog::OnMoveSourceDown()
{
    int sel=m_LWSourcePaths.GetNextItem(-1,LVNI_SELECTED);
    if(sel==-1 || sel>=m_LWSourcePaths.GetItemCount()-1){return;}

    SwapItems(sel,sel+1);
}

void CSettingsDialog::SwapItems(int index1,int index2) 
{
    int finallySelected=-1;
    int finallyUnselected=-1;
    int sel=m_LWSourcePaths.GetNextItem(-1,LVNI_SELECTED);
    if(sel==index1){finallySelected=index2;finallyUnselected=index1;}
    if(sel==index2){finallySelected=index1;finallyUnselected=index2;}

    TCHAR item1Text[1024]={0},item2Text[1024]={0};
    LVITEM item1={0},item2={0};
    item1.iItem=index1;
    item1.cchTextMax=1024;
    item1.pszText=item1Text;
    item1.mask=LVIF_TEXT;
    item2.iItem=index2;
    item2.pszText=item2Text;
    item2.cchTextMax=1024;
    item2.mask=LVIF_TEXT;

    m_LWSourcePaths.GetItem(&item1);
    m_LWSourcePaths.GetItem(&item2);

    item1.iItem=index2;
    item2.iItem=index1;

    m_LWSourcePaths.SetItem(&item1);
    m_LWSourcePaths.SetItem(&item2);
    if(finallySelected!=-1)		{m_LWSourcePaths.SetItemState(finallySelected,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);}
    if(finallyUnselected!=-1)	{m_LWSourcePaths.SetItemState(finallyUnselected,LVIS_SELECTED|LVIS_FOCUSED,0);}
}

void CSettingsDialog::OnDestroy()
{	
    CDialog::OnDestroy();
}
