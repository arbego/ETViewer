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
#include "ProviderTree.h"
#include ".\providertree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProviderTree

IMPLEMENT_DYNCREATE(CProviderTree, CTreeView)

BEGIN_MESSAGE_MAP(CProviderTree, CTreeView)
    ON_WM_CREATE()
    ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
    ON_WM_KEYDOWN()
    ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// Construcción o destrucción de CProviderTree

CProviderTree::CProviderTree()
{
    // TODO: agregar aquí el código de construcción
    m_hImageList=ImageList_Create(16,16,ILC_COLOR4|ILC_MASK,0,10);;
    m_hPlayIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_PLAY),IMAGE_ICON,16,16,0);
    m_hPlayBlockedIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_PLAY_BLOCKED),IMAGE_ICON,16,16,0);
    m_hRecordIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_RECORD),IMAGE_ICON,16,16,0);
    m_hPauseIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_PAUSE),IMAGE_ICON,16,16,0);
    m_hModuleIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_MODULE),IMAGE_ICON,16,16,0);
    m_hLevelIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_LEVEL),IMAGE_ICON,16,16,0);
    m_hFlagsIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_FLAGS),IMAGE_ICON,16,16,0);
    m_hCheckedIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_CHECKED),IMAGE_ICON,16,16,0);
    m_hUncheckedIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_UNCHECKED),IMAGE_ICON,16,16,0);
    m_hSelectedIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_SELECTED),IMAGE_ICON,16,16,0);
    m_iPlayIcon=ImageList_AddIcon(m_hImageList,m_hPlayIcon);
    m_iPlayBlockedIcon=ImageList_AddIcon(m_hImageList,m_hPlayBlockedIcon);
    m_iRecordIcon=ImageList_AddIcon(m_hImageList,m_hRecordIcon);
    m_iPauseIcon=ImageList_AddIcon(m_hImageList,m_hPauseIcon);
    m_iModuleIcon = ImageList_AddIcon(m_hImageList,m_hModuleIcon);
    m_iLevelIcon=ImageList_AddIcon(m_hImageList,m_hLevelIcon);
    m_iFlagsIcon=ImageList_AddIcon(m_hImageList,m_hFlagsIcon);
    m_iCheckedIcon=ImageList_AddIcon(m_hImageList,m_hCheckedIcon);
    m_iUncheckedIcon=ImageList_AddIcon(m_hImageList,m_hUncheckedIcon);
    m_iSelectedIcon=ImageList_AddIcon(m_hImageList,m_hSelectedIcon);
}

CProviderTree::~CProviderTree()
{
}

BOOL CProviderTree::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: modificar aquí la clase Window o los estilos cambiando CREATESTRUCT cs

    cs.style|=TVS_FULLROWSELECT;
    cs.style|=TVS_HASBUTTONS;
    cs.style|=TVS_HASLINES;
    cs.style|=TVS_LINESATROOT;

    return CTreeView::PreCreateWindow(cs);
}

void CProviderTree::OnInitialUpdate()
{
    CTreeView::OnInitialUpdate();

    // TODO: puede rellenar TreeView con elementos obteniendo acceso directamente
    //  a través de una llamada a GetTreeCtrl().

}


// Diagnósticos de CProviderTree

#ifdef _DEBUG
void CProviderTree::AssertValid() const
{
    CTreeView::AssertValid();
}

void CProviderTree::Dump(CDumpContext& dc) const
{
    CTreeView::Dump(dc);
}

CETViewerDoc* CProviderTree::GetDocument() // La versión de no depuración es en línea
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CETViewerDoc)));
    return (CETViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// Controladores de mensaje de CProviderTree

void CProviderTree::UpdateProviderSubTree(HTREEITEM hProviderItem)
{
    HTREEITEM hTempItem = NULL, hChild = NULL;
    CTreeCtrl &treeCtrl = GetTreeCtrl();
    CTraceProvider *pProvider = (CTraceProvider *)treeCtrl.GetItemData(hProviderItem);

    do
    {
        hChild=treeCtrl.GetChildItem(hProviderItem);
        if(hChild){treeCtrl.DeleteItem(hChild);}
    }
    while(hChild);

    
    if(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_ReadLog)
    {
        return;
    }

    HTREEITEM hModuleItems = treeCtrl.InsertItem(_T("Modules"), m_iModuleIcon, m_iModuleIcon, hProviderItem, TVI_LAST);
    for(const auto file : pProvider->GetFileList())
    {
        if(_tcslen(file.c_str()))
        {
            TCHAR filename[MAX_PATH] = { 0 };
            TCHAR extension[MAX_PATH] = { 0 };
            _tsplitpath_s(file.c_str(), NULL, 0, NULL, 0, filename, MAX_PATH, extension, MAX_PATH);
            std::tstring sfilename = filename;
            sfilename += _T(".");
            sfilename += extension;
            hTempItem = treeCtrl.InsertItem(sfilename.c_str(), 0, 0, hModuleItems, TVI_SORT);
        }
    }

    std::map<std::tstring, DWORD> traceLevels;
    std::map<std::tstring, DWORD>::iterator i;

    pProvider->GetSupportedFlags(&traceLevels);

    HTREEITEM hTraceFlagItems=treeCtrl.InsertItem(_T("Flags"),m_iFlagsIcon,m_iFlagsIcon,hProviderItem,TVI_LAST);

    for(i=traceLevels.begin();i!=traceLevels.end();i++)
    {
        if(_tcslen(i->first.c_str()))
        {
            hTempItem = treeCtrl.InsertItem(i->first.c_str(), 0, 0, hTraceFlagItems, TVI_SORT);
        }
        treeCtrl.SetItemData(hTempItem,i->second);
    }

    HTREEITEM hTraceLevelItems=treeCtrl.InsertItem(_T("Levels"),m_iLevelIcon,m_iLevelIcon,hProviderItem,TVI_LAST);

    hTempItem=treeCtrl.InsertItem(_T("TRACE_LEVEL_FATAL"),0,0,hTraceLevelItems,TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_FATAL);

    hTempItem = treeCtrl.InsertItem(_T("TRACE_LEVEL_ERROR"), 0, 0, hTraceLevelItems, TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_ERROR);

    hTempItem=treeCtrl.InsertItem(_T("TRACE_LEVEL_WARNING"),0,0,hTraceLevelItems,TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_WARNING);

    hTempItem=treeCtrl.InsertItem(_T("TRACE_LEVEL_INFORMATION"),0,0,hTraceLevelItems,TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_INFORMATION);

    hTempItem=treeCtrl.InsertItem(_T("TRACE_LEVEL_VERBOSE"),0,0,hTraceLevelItems,TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_VERBOSE);

    hTempItem = treeCtrl.InsertItem(_T("TRACE_LEVEL_RESERVED6"), 0, 0, hTraceLevelItems, TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_RESERVED6);

    hTempItem = treeCtrl.InsertItem(_T("TRACE_LEVEL_RESERVED7"), 0, 0, hTraceLevelItems, TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_RESERVED7);

    hTempItem = treeCtrl.InsertItem(_T("TRACE_LEVEL_RESERVED8"), 0, 0, hTraceLevelItems, TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_RESERVED8);

    hTempItem = treeCtrl.InsertItem(_T("TRACE_LEVEL_RESERVED9"), 0, 0, hTraceLevelItems, TVI_LAST);
    treeCtrl.SetItemData(hTempItem,TRACE_LEVEL_RESERVED9);

    treeCtrl.Expand(hProviderItem,TVE_EXPAND);
    treeCtrl.Expand(hTraceFlagItems,TVE_EXPAND);
}

void CProviderTree::UpdateProviderIcons(HTREEITEM hItem)
{
    CTreeCtrl &treeCtrl=GetTreeCtrl();
    CTraceProvider *pProvider=(CTraceProvider *)treeCtrl.GetItemData(hItem);

    int iIcon=-1;
    
    if(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_ReadLog)
    {
        iIcon=m_iPlayBlockedIcon;
    }
    else if(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_CreateLog)
    {
        iIcon=theApp.m_Controller.IsProviderEnabled(pProvider)?m_iRecordIcon:m_iPauseIcon;
    }
    else 
    {
        iIcon=theApp.m_Controller.IsProviderEnabled(pProvider)?m_iPlayIcon:m_iPauseIcon;
    }
    treeCtrl.SetItemImage(hItem,iIcon,iIcon);

    HTREEITEM hChild=treeCtrl.GetChildItem(hItem);
    while(hChild)
    {
        CString sText=treeCtrl.GetItemText(hChild);
        if(sText == _T("Levels"))
        {
            HTREEITEM hLevelItem=treeCtrl.GetChildItem(hChild);
            while(hLevelItem)
            {
                bool bChecked=treeCtrl.GetItemData(hLevelItem)==theApp.m_Controller.GetProviderLevel(pProvider);
                treeCtrl.SetItemImage(hLevelItem,bChecked?m_iCheckedIcon:m_iUncheckedIcon,bChecked?m_iCheckedIcon:m_iUncheckedIcon);

                hLevelItem=treeCtrl.GetNextSiblingItem(hLevelItem);
            }
        }
        else if(sText == _T("Flags"))
        {
            HTREEITEM hFlagItem=treeCtrl.GetChildItem(hChild);
            while(hFlagItem)
            {
                bool bChecked=(treeCtrl.GetItemData(hFlagItem)&theApp.m_Controller.GetProviderFlags(pProvider))?true:false;
                treeCtrl.SetItemImage(hFlagItem,bChecked?m_iCheckedIcon:m_iUncheckedIcon,bChecked?m_iCheckedIcon:m_iUncheckedIcon);

                hFlagItem=treeCtrl.GetNextSiblingItem(hFlagItem);
            }
        }

        hChild=treeCtrl.GetNextSiblingItem(hChild);
    }
}

void CProviderTree::OnAddProvider(CTraceProvider *pProvider)
{
    CTreeCtrl &treeCtrl=GetTreeCtrl();

    HTREEITEM hProviderItem=treeCtrl.InsertItem(pProvider->GetComponentName().c_str(),m_iPlayIcon,m_iPlayIcon,TVI_ROOT,TVI_SORT);
    treeCtrl.SetItemData(hProviderItem,(DWORD_PTR)pProvider);
    
    UpdateProviderSubTree(hProviderItem);
    UpdateProviderIcons(hProviderItem);
}

void CProviderTree::OnRemoveProvider(CTraceProvider *pProvider)
{
    CTreeCtrl &treeCtrl=GetTreeCtrl();

    HTREEITEM hChild=treeCtrl.GetChildItem(TVI_ROOT);
    while(hChild)
    {
        CTraceProvider *pTempProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hChild);
        if(pTempProvider==pProvider)
        {
            treeCtrl.DeleteItem(hChild);
            break;
        }
        hChild=treeCtrl.GetNextSiblingItem(hChild);
    }
}

void CProviderTree::OnReplaceProvider(CTraceProvider *pOldProvider,CTraceProvider *pNewProvider)
{
    CTreeCtrl &treeCtrl=GetTreeCtrl();

    HTREEITEM hChild=treeCtrl.GetChildItem(TVI_ROOT);
    while(hChild)
    {
        CTraceProvider *pTempProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hChild);
        if(pTempProvider==pOldProvider)
        {
            treeCtrl.SetItemData(hChild,(DWORD)pNewProvider);
            UpdateProviderSubTree(hChild);
            UpdateProviderIcons(hChild);
            break;
        }
        hChild=treeCtrl.GetNextSiblingItem(hChild);
    }
}

void CProviderTree::OnProvidersModified()
{
    OnSessionTypeChanged();
}

void CProviderTree::OnSessionTypeChanged()
{
    CTreeCtrl &treeCtrl=GetTreeCtrl();

    HTREEITEM hChild=treeCtrl.GetChildItem(TVI_ROOT);
    while(hChild)
    {
        UpdateProviderSubTree(hChild);
        UpdateProviderIcons(hChild);
        hChild=treeCtrl.GetNextSiblingItem(hChild);
    }
}

void CProviderTree::OnRemoveSelectedProvider()
{
    CTreeCtrl &treeCtrl=GetTreeCtrl();
    HTREEITEM hItem=treeCtrl.GetSelectedItem();
    if(hItem && treeCtrl.GetParentItem(hItem)==NULL)
    {
        CTraceProvider *pProvider=(CTraceProvider *)treeCtrl.GetItemData(hItem);
        theApp.RemoveProvider(pProvider);
    }
}

int CProviderTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CTreeView::OnCreate(lpCreateStruct) == -1)
        return -1;

    TreeView_SetImageList(GetTreeCtrl().m_hWnd,m_hImageList,TVSIL_NORMAL);
    
    // TODO:  Add your specialized creation code here
    return 0;
}

void CProviderTree::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    *pResult = 0;

    POINT pos={0};
    TVHITTESTINFO hitTestInfo={0};
    GetCursorPos(&hitTestInfo.pt);
    ScreenToClient(&hitTestInfo.pt);
    HTREEITEM hItem=GetTreeCtrl().HitTest(&hitTestInfo);
    if(hitTestInfo.flags&TVHT_ONITEMICON && theApp.m_Controller.GetSessionType()!=eTraceControllerSessionType_ReadLog)
    {
        HTREEITEM hParentItem=GetTreeCtrl().GetParentItem(hItem);
        CString sText=GetTreeCtrl().GetItemText(hParentItem);
        if(sText=="Levels")
        {
            HTREEITEM hProviderItem=GetTreeCtrl().GetParentItem(hParentItem);
            CTraceProvider *pProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hProviderItem);
            theApp.m_Controller.SetProviderLevel(pProvider,GetTreeCtrl().GetItemData(hItem));
            UpdateProviderIcons(hProviderItem);
        }
        else if(sText=="Flags")
        {
            HTREEITEM hProviderItem=GetTreeCtrl().GetParentItem(hParentItem);
            CTraceProvider *pProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hProviderItem);
            DWORD dwFlags=theApp.m_Controller.GetProviderFlags(pProvider);
            DWORD dwSelectedFlag=GetTreeCtrl().GetItemData(hItem);
            if(dwSelectedFlag&dwFlags)
            {
                dwFlags&=~dwSelectedFlag;
            }
            else
            {
                dwFlags|=dwSelectedFlag;
            }
            theApp.m_Controller.SetProviderFlags(pProvider,dwFlags);
            UpdateProviderIcons(hProviderItem);
        }
        else if(hParentItem==NULL)
        {
            CTraceProvider *pProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hItem);
            bool bEnabled=theApp.m_Controller.IsProviderEnabled(pProvider);
            theApp.m_Controller.EnableProvider(pProvider,!bEnabled);
            UpdateProviderIcons(hItem);
        }
    }
}

void CProviderTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if(nChar==VK_INSERT)
    {	
        theApp.m_pFrame->OnOpenFile();
    }

    if(nChar==VK_DELETE)
    {
        OnRemoveSelectedProvider();
    }

    CTreeView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CProviderTree::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMITEMACTIVATE *pActivate=(NMITEMACTIVATE *)pNMHDR;

    bool bOpenLogSession=(theApp.m_Controller.GetSessionType()!=eTraceControllerSessionType_ReadLog);

    POINT pos;
    GetCursorPos(&pos);
    ScreenToClient(&pos);

    UINT flags=0;
    CTraceProvider *pProvider=NULL;
    CTreeCtrl &treeCtrl=GetTreeCtrl();
    HTREEITEM hProviderItem=0;
    HTREEITEM hItem=treeCtrl.HitTest(CPoint(pos),&flags);
    HTREEITEM hChild=NULL;
    treeCtrl.Select(hItem,TVGN_CARET);

    while(hItem)
    {
        if(treeCtrl.GetParentItem(hItem)==NULL)
        {
            hProviderItem=hItem;
            pProvider=(CTraceProvider *)treeCtrl.GetItemData(hItem);
            break;
        }
        else
        {
            hItem=treeCtrl.GetParentItem(hItem);
        }
    }
    if(hProviderItem)
    {
        pProvider=(CTraceProvider *)treeCtrl.GetItemData(hProviderItem);
    }

    HMENU hMenu=GetSubMenu(LoadMenu(AfxGetResourceHandle(),MAKEINTRESOURCE(IDM_PROVIDER_TREE)),0);
    CMenu *pMenu=new CMenu;
    pMenu->Attach(hMenu);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_START,pProvider && bOpenLogSession && !theApp.m_Controller.IsProviderEnabled(pProvider)?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_STOP,pProvider && bOpenLogSession && theApp.m_Controller.IsProviderEnabled(pProvider)?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_RELOAD_PROVIDER,pProvider?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_SET_ALL_FLAGS,pProvider && bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_CLEAR_ALL_FLAGS,pProvider && bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_DELETE_PROVIDER,pProvider?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_FATAL,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_ERROR,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_WARNING,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_INFORMATION, bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_VERBOSE,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_RESERVED6,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_RESERVED7,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_RESERVED8,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_ALL_RESERVED9,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_START_ALL,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_STOP_ALL,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_SET_ALL_FLAGS_ALL,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);
    pMenu->EnableMenuItem(ID_PROVIDER_TREE_CLEAR_ALL_FLAGS_ALL,bOpenLogSession?MF_ENABLED:MF_DISABLED|MF_GRAYED);

    POINT p={0};
    ::GetCursorPos(&p);
    int res=::TrackPopupMenu(pMenu->m_hMenu,TPM_LEFTALIGN|TPM_RETURNCMD,p.x,p.y,0,m_hWnd,NULL);
    switch(res)
    {
    case ID_PROVIDER_TREE_ADD_PROVIDER:
        theApp.m_pFrame->OnOpenFile();
        break;
    case ID_PROVIDER_TREE_RELOAD_PROVIDER:
        theApp.ReloadProvider(pProvider);
        break;	
    case ID_PROVIDER_TREE_RELOAD_ALL_PROVIDERS:
        theApp.ReloadAllProviders();
        break;		
    case ID_PROVIDER_TREE_DELETE_PROVIDER:
        OnRemoveSelectedProvider();
        break;	
    case ID_PROVIDER_TREE_DELETE_ALL_PROVIDERS:
        theApp.RemoveAllProviders();
        break;	
    case ID_PROVIDER_TREE_START:
        theApp.m_Controller.EnableProvider(pProvider,TRUE);
        UpdateProviderIcons(hProviderItem);
        break;		
    case ID_PROVIDER_TREE_STOP:
        theApp.m_Controller.EnableProvider(pProvider,FALSE);
        UpdateProviderIcons(hProviderItem);
        break;	
    case ID_PROVIDER_TREE_SET_ALL_FLAGS:
        theApp.m_Controller.SetProviderFlags(pProvider,pProvider->GetAllSupportedFlagsMask());
        UpdateProviderIcons(hProviderItem);
        break;		
    case ID_PROVIDER_TREE_CLEAR_ALL_FLAGS:
        theApp.m_Controller.SetProviderFlags(pProvider,0);
        UpdateProviderIcons(hProviderItem);
        break;		
    case ID_PROVIDER_TREE_START_ALL:
        hChild=treeCtrl.GetChildItem(TVI_ROOT);
        while(hChild)
        {
            CTraceProvider *pTempProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hChild);
            theApp.m_Controller.EnableProvider(pTempProvider,TRUE);
            UpdateProviderIcons(hChild);
            hChild=treeCtrl.GetNextSiblingItem(hChild);
        }
        break;		
    case ID_PROVIDER_TREE_STOP_ALL:
        hChild=treeCtrl.GetChildItem(TVI_ROOT);
        while(hChild)
        {
            CTraceProvider *pTempProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hChild);
            theApp.m_Controller.EnableProvider(pTempProvider,FALSE);
            UpdateProviderIcons(hChild);
            hChild=treeCtrl.GetNextSiblingItem(hChild);
        }
        break;	
    case ID_PROVIDER_TREE_SET_ALL_FLAGS_ALL:
        hChild=treeCtrl.GetChildItem(TVI_ROOT);
        while(hChild)
        {
            CTraceProvider *pTempProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hChild);
            theApp.m_Controller.SetProviderFlags(pTempProvider,pTempProvider->GetAllSupportedFlagsMask());
            UpdateProviderIcons(hChild);
            hChild=treeCtrl.GetNextSiblingItem(hChild);
        }
        break;		
    case ID_PROVIDER_TREE_CLEAR_ALL_FLAGS_ALL:
        hChild=treeCtrl.GetChildItem(TVI_ROOT);
        while(hChild)
        {
            CTraceProvider *pTempProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hChild);
            theApp.m_Controller.SetProviderFlags(pTempProvider,0);
            UpdateProviderIcons(hChild);
            hChild=treeCtrl.GetNextSiblingItem(hChild);
        }
        break;

    case ID_PROVIDER_TREE_ALL_FATAL:
    case ID_PROVIDER_TREE_ALL_ERROR:
    case ID_PROVIDER_TREE_ALL_WARNING:
    case ID_PROVIDER_TREE_ALL_INFORMATION:
    case ID_PROVIDER_TREE_ALL_VERBOSE:
    case ID_PROVIDER_TREE_ALL_RESERVED6:
    case ID_PROVIDER_TREE_ALL_RESERVED7:
    case ID_PROVIDER_TREE_ALL_RESERVED8:
    case ID_PROVIDER_TREE_ALL_RESERVED9:

        SetAllProviderLevel(res-ID_PROVIDER_TREE_ALL_FATAL+1);
        break;		
    }

    delete pMenu;
}

BOOL CProviderTree::OnEraseBkgnd(CDC* pDC)
{
    return CTreeView::OnEraseBkgnd(pDC);
}

void CProviderTree::SetAllProviderLevel(DWORD dwLevel)
{
    CTreeCtrl &treeCtrl=GetTreeCtrl();

    HTREEITEM hChild=treeCtrl.GetChildItem(TVI_ROOT);
    while(hChild)
    {
        CTraceProvider *pTempProvider=(CTraceProvider *)GetTreeCtrl().GetItemData(hChild);
        theApp.m_Controller.SetProviderLevel(pTempProvider,dwLevel);
        UpdateProviderIcons(hChild);
        hChild=treeCtrl.GetNextSiblingItem(hChild);
    }
}