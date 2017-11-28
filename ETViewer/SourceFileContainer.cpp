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
#include "SourceFileContainer.h"
#include ".\sourcefilecontainer.h"

/////////////////////////////////////////////////////////////////////////////
// CSourceFileContainer dialog

CSourceFileContainer::CSourceFileContainer(CWnd* pParent /*=NULL*/)
    : CDialog(CSourceFileContainer::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSourceFileContainer)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CSourceFileContainer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSourceFileContainer)
    DDX_Control(pDX, IDC_BT_RECENT_FILE, m_BTRecentFiles);
    DDX_Control(pDX, IDC_BT_FIND, m_BTFind);
    DDX_Control(pDX, IDC_BT_COPY, m_BTCopy);
    DDX_Control(pDX, IDC_BT_OPEN_FILE, m_BTOpenFile);
    DDX_Control(pDX, IDC_TC_SOURCE_FILES, m_TCSourceFiles);
    DDX_Control(pDX, IDC_BT_CLOSE_FILE, m_BTCloseFile);
    //}}AFX_DATA_MAP
}

//<winuser.h>
BEGIN_MESSAGE_MAP(CSourceFileContainer, CDialog)
    //{{AFX_MSG_MAP(CSourceFileContainer)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BT_CLOSE_FILE, OnCloseFile)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TC_SOURCE_FILES, OnSourceFileChanged)
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BT_OPEN_FILE, OnOpenFile)
    ON_BN_CLICKED(IDC_BT_COPY, OnCopy)
    ON_BN_CLICKED(IDC_BT_FIND, OnFind)
    ON_BN_CLICKED(IDC_BT_RECENT_FILE, OnRecentFile)
    //}}AFX_MSG_MAP
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSourceFileContainer message handlers

BOOL CSourceFileContainer::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    m_BTCloseFile.SetIcon((HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_CLOSE_SMALL),IMAGE_ICON,16,16,0));
    m_BTOpenFile.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_OPEN_FILE)));
    m_BTFind.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_FIND)));
    m_BTCopy.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_COPY)));

    HICON hIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_SOURCE_FILE),IMAGE_ICON,16,16,0);
    SetIcon(hIcon,TRUE);
    SetIcon(hIcon,FALSE);
    SetMetrics();
    return TRUE;  
}

CSourceFileViewer *CSourceFileContainer::GetViewerAt(int index)
{
    if(index==-1){return NULL;}

    TCITEM item={0};
    item.mask=TCIF_PARAM;
    m_TCSourceFiles.GetItem(index,&item);
    return (CSourceFileViewer *)item.lParam;
}

void CSourceFileContainer::OnDestroy() 
{
    CDialog::OnDestroy();
    m_TCSourceFiles.DeleteAllItems();
}

void CSourceFileContainer::OnCloseFile() 
{
    int index=m_TCSourceFiles.GetCurSel();
    CSourceFileViewer *pViewer=GetViewerAt(index);
    if(pViewer){delete pViewer;m_TCSourceFiles.DeleteItem(index);}

    m_TCSourceFiles.SetCurSel(index<m_TCSourceFiles.GetItemCount()?index:m_TCSourceFiles.GetItemCount()-1);
    
    ShowSelectedFile();
    SetMetrics();
    //if(m_TCSourceFiles.GetItemCount()==0){ShowWindow(SW_HIDE);}
}

void CSourceFileContainer::ShowSelectedFile() 
{
    int index=m_TCSourceFiles.GetCurSel();
    for(int x=0;x<m_TCSourceFiles.GetItemCount();x++)
    {
        CSourceFileViewer *pViewer=GetViewerAt(x);
        pViewer->ShowWindow(x==index?SW_SHOW:SW_HIDE);
        pViewer->m_EDFile.SetFocus();
    }
}

void CSourceFileContainer::OnSourceFileChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
    // TODO: Add your control notification handler code here
    
    ShowSelectedFile();
    *pResult = 0;
}
void CSourceFileContainer::GetFiles(std::set<std::tstring> *psFiles)
{
    psFiles->clear();
    for(int x=0;x<m_TCSourceFiles.GetItemCount();x++)
    {
        CSourceFileViewer *pViewer=GetViewerAt(x);
        psFiles->insert(pViewer->GetFile());
    }
}

bool CSourceFileContainer::ShowFile(const TCHAR *pFile,int line,bool bShowErrorIfFailed)
{
    TCHAR A[MAX_PATH]={0},B[MAX_PATH]={0};
    for(int x=0;x<m_TCSourceFiles.GetItemCount();x++)
    {
        CSourceFileViewer *pViewer=GetViewerAt(x);

        _tcscpy_s(A,pFile);
        _tcsupr_s(A);
        _tcscpy_s(B,pViewer->GetFile().c_str());
        _tcsupr_s(B);
        if(_tcscmp(A,B)==0)
        {
            m_TCSourceFiles.SetCurSel(x);
            pViewer->ShowLine(line);
            ShowSelectedFile();
            ShowWindow(SW_SHOW);
            return true;
        }
    }
    CSourceFileViewer *pViewer=new CSourceFileViewer(this);
    pViewer->Create(IDD_SOURCE_FILE_VIEWER,this);
    if(pViewer->OpenFile(pFile,line,bShowErrorIfFailed)!=0)
    {
        delete pViewer;
        return false;
    }

    ShowWindow(SW_SHOW);
    pViewer->SetWindowPos(NULL,0,20,0,0,SWP_NOZORDER|SWP_NOSIZE);

    TCHAR file[MAX_PATH]={0},ext[MAX_PATH]={0};

    _tsplitpath_s(pFile,NULL,0,NULL,0,file,MAX_PATH,ext,MAX_PATH);
    _tcscat_s(file,ext);

    int index=m_TCSourceFiles.InsertItem(TCIF_PARAM|TCIF_TEXT,m_TCSourceFiles.GetItemCount(),file,0,(DWORD)pViewer,0,0);
    m_TCSourceFiles.SetCurSel(index);
    ShowSelectedFile();
    SetMetrics();
    return true;
}

void CSourceFileContainer::ReloadFile( const TCHAR *sFile )
{
    for(int x=0;x<m_TCSourceFiles.GetItemCount();x++)
    {
        CSourceFileViewer *pViewer=GetViewerAt(x);
        if(_tcsicmp(pViewer->GetFile().c_str(),sFile)==0)
        {
            pViewer->Reload();
            break;
        }
    }
}

void CSourceFileContainer::SetMetrics()
{
    RECT clientRect,tabRect,openRect,closeRect;
    GetClientRect(&clientRect);
    m_TCSourceFiles.GetWindowRect(&tabRect);
    m_BTOpenFile.GetWindowRect(&openRect);
    m_BTCloseFile.GetWindowRect(&closeRect);
    ScreenToClient(&tabRect);
    ScreenToClient(&openRect);
    ScreenToClient(&closeRect);

    SIZE clientSize=GetRectSize(clientRect);
    SIZE tabSize=GetRectSize(tabRect);
    SIZE closeSize=GetRectSize(closeRect);
    SIZE viewerSize;
    POINT closePos=GetRectPos(closeRect);
    POINT tabPos,viewerPos;

    tabPos.x=0;
    tabPos.y=openRect.bottom+2;
    tabSize.cx=clientSize.cx;

    viewerPos.x=0;
    viewerPos.y=tabPos.y+tabSize.cy;
    viewerSize.cx=clientSize.cx;
    viewerSize.cy=clientSize.cy-(viewerPos.y);

    closePos.x=clientSize.cx-(16+4);
    closePos.y=4;
    closeSize.cx=16;
    closeSize.cy=16;

    m_TCSourceFiles.SetWindowPos(NULL,tabPos.x,tabPos.y,tabSize.cx,tabSize.cy,SWP_NOZORDER);
    m_BTCloseFile.SetWindowPos(NULL,closePos.x,closePos.y,closeSize.cx,closeSize.cy,SWP_NOZORDER);
    for(int x=0;x<m_TCSourceFiles.GetItemCount();x++)
    {
        CSourceFileViewer *pViewer=GetViewerAt(x);
        pViewer->SetWindowPos(NULL,viewerPos.x,viewerPos.y,viewerSize.cx,viewerSize.cy,SWP_NOZORDER);
    }
}

void CSourceFileContainer::SelectNext()
{
    int index=m_TCSourceFiles.GetCurSel()+1;
    if(index>=m_TCSourceFiles.GetItemCount()){index=0;}
    m_TCSourceFiles.SetCurSel(index);
    ShowSelectedFile();
}

void CSourceFileContainer::SelectPrevious()
{
    int index=m_TCSourceFiles.GetCurSel()-1;
    if(index<0){index=m_TCSourceFiles.GetItemCount()-1;}
    m_TCSourceFiles.SetCurSel(index);
    ShowSelectedFile();
}

void CSourceFileContainer::OnSize(UINT nType, int cx, int cy) 
{
    CDialog::OnSize(nType, cx, cy);
    
    if(m_TCSourceFiles.m_hWnd){SetMetrics();}
}

void CSourceFileContainer::BrowseForAndShowFile()
{
    TCHAR sTempBuffer[1024*10]={0};
    CFileDialog dialog(TRUE,_T("c"),NULL,OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR,_T("Source Files(*.c *.cpp *.h *.inl)|*.c;*.cpp;*.h;*.inl|All Files(*.*)|*.*"),IsWindowVisible()?this:theApp.m_pMainWnd);
    dialog.GetOFN().lpstrFile=sTempBuffer;
    dialog.GetOFN().nMaxFile=_countof(sTempBuffer);
    if(dialog.DoModal()==IDOK)
    {
        std::tstring sPath;
        TCHAR *pString=dialog.GetOFN().lpstrFile;

        if(pString[_tcslen(pString)+1]!=0)
        {
            sPath=pString;
            sPath+=_T("\\");
            pString=pString+_tcslen(pString)+1;
        }
        while(pString[0]!=0)
        {
            std::tstring sFileName = sPath;
            sFileName+=pString;

            ShowFile(sFileName.c_str(),true);
            pString=pString+_tcslen(pString)+1;
        }
    }
}

void CSourceFileContainer::OnOpenFile() 
{
    BrowseForAndShowFile();
}

void CSourceFileContainer::OnCopy() 
{
    CSourceFileViewer *pViewer=GetViewerAt(m_TCSourceFiles.GetCurSel());
    if(pViewer){pViewer->Copy();}
}

void CSourceFileContainer::OnFind() 
{
    CSourceFileViewer *pViewer=GetViewerAt(m_TCSourceFiles.GetCurSel());
    if(pViewer){pViewer->ShowFindDialog();}
}

void CSourceFileContainer::OnRecentFile() 
{
    if(theApp.m_RecentSourceFiles.size()==0){return;}

    CMenu *pMenu=new CMenu;
    pMenu->CreatePopupMenu();
    
    for(unsigned x=0;x<theApp.m_RecentSourceFiles.size();x++)
    {
        TCHAR *pText=(TCHAR*)theApp.m_RecentSourceFiles[x].c_str();
        pMenu->InsertMenu(x,MF_BYPOSITION,RECENT_SOURCE_FILE_BASE_INDEX+x,pText);
    }
    POINT p={0};
    ::GetCursorPos(&p);
    int res=::TrackPopupMenu(pMenu->m_hMenu,TPM_LEFTALIGN|TPM_RETURNCMD,p.x,p.y,0,m_hWnd,NULL);
    if(res)
    {
        TCHAR file[MAX_PATH]={0};
        _tcscpy_s(file,(TCHAR*)theApp.m_RecentSourceFiles[res-RECENT_SOURCE_FILE_BASE_INDEX].c_str());
        ShowFile(file,0,true);
    }
    delete pMenu;
    
}

BOOL CSourceFileContainer::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    int index=m_TCSourceFiles.GetCurSel();
    CSourceFileViewer *pViewer=GetViewerAt(index);
    if(pViewer)
    {
        RECT rect={0};
        pViewer->GetWindowRect(&rect);
        ScreenToClient(&rect);
        pDC->ExcludeClipRect(&rect);
    }

    return CDialog::OnEraseBkgnd(pDC);
}
