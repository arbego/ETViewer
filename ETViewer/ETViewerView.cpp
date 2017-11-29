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
#include ".\etviewerview.h"
#include "HighLightPane.h"
#include "HighLightFiltersEditor.h"
#include "SaveAllTracesQuestionDialog.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CETViewerView

IMPLEMENT_DYNCREATE(CETViewerView, CListView)

BEGIN_MESSAGE_MAP(CETViewerView, CListView)
    ON_WM_STYLECHANGED()
    ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnCustomDraw)
    ON_COMMAND(ID_EDIT_FIND, OnEditFind)
    ON_WM_CREATE()
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
    ON_COMMAND(ID_CLEAR, OnClear)
    ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginEdit)
    ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndEdit)
    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetItemInfo)
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnLvnColumnclick)
END_MESSAGE_MAP()

// Construcción o destrucción de CETViewerView

CETViewerView::CETViewerView()
{
    // TODO: agregar aquí el código de construcción

    m_nUnformattedTraces=0;
    m_nLastFocusedSequenceIndex=0;
    m_pEdit=NULL;
    m_bFindDirectionUp=false;
    m_iHollowImage=0;
    m_iMarkerImage=0;
    m_iEditSubItem=0;
    m_hImageList=ImageList_Create(16,12,ILC_COLOR4|ILC_MASK,0,10);
    m_hHollowIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_HOLLOW),IMAGE_ICON,16,16,0);
    m_hMarkerIcon=(HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_MARKER),IMAGE_ICON,16,16,0);

    m_hTracesMutex=CreateMutex(NULL,FALSE,NULL);
    m_bShowLastTrace=true;

    m_pTraceFont=NULL;
    m_dwTraceFontSize=14;
    m_sTraceFont=_T("Lucida Console");

    m_cNormalTextColor=RGB(0,0,0);
    m_cNormalBkColor=RGB(255,255,255);
    m_cSelectedTextColor=GetSysColor(COLOR_HIGHLIGHTTEXT);
    m_cSelectedBkColor=GetSysColor(COLOR_HIGHLIGHT);

    POINT	 P={1,1};
    LOGPEN   LP={0};
    LOGBRUSH LB={0};

    LP.lopnWidth=P;
    LP.lopnStyle=PS_SOLID;
    LB.lbStyle=BS_SOLID;

    LP.lopnColor=m_cNormalBkColor;
    LB.lbColor=m_cNormalBkColor;

    m_hNormalPen=CreatePenIndirect(&LP);
    m_hNormalBrush=CreateBrushIndirect(&LB);

    LP.lopnColor=m_cSelectedBkColor;
    LB.lbColor=m_cSelectedBkColor;

    m_hSelectedPen=CreatePenIndirect(&LP);
    m_hSelectedBrush=CreateBrushIndirect(&LB);

    m_hHollowBrush=(HBRUSH)GetStockObject(HOLLOW_BRUSH);

    m_SortColumn=eETViewerColumn_Index;
    m_SortDirection=eETViewerSortDirection_Ascending;

}

CETViewerView::~CETViewerView()
{
    if(m_hTracesMutex){CloseHandle(m_hTracesMutex);m_hTracesMutex=NULL;}
    delete m_pTraceFont;

    if(m_hNormalBrush){DeleteObject(m_hNormalBrush);m_hNormalBrush=NULL;}
    if(m_hSelectedBrush){DeleteObject(m_hSelectedBrush);m_hSelectedBrush=NULL;}
    if(m_hNormalPen){DeleteObject(m_hNormalPen);m_hNormalPen=NULL;}
    if(m_hSelectedPen){DeleteObject(m_hSelectedPen);m_hSelectedPen=NULL;}
    if(m_hNormalBrush){DeleteObject(m_hNormalBrush);m_hNormalBrush=NULL;}
}

BOOL CETViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
    DWORD dwStyle=cs.style;
    cs.style|=LVS_REPORT|LVS_EDITLABELS;
    cs.style|=LVS_OWNERDATA;
    return CListView::PreCreateWindow(cs);
}

int CETViewerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (__super::OnCreate(lpCreateStruct) == -1)
        return -1;

    Load(&theApp.m_ConfigFile);

    if(m_ColumnInfo.size()==0)
    {
        m_ColumnInfo.clear();
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Text,_T("Text"),LVCFMT_LEFT,8000,true,0));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_PIDTID,_T("PID-TID"),LVCFMT_LEFT,100,false,-1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Time,_T("Time"),LVCFMT_LEFT,110,false,-1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_TimeStamp,_T("TimeStamp"),LVCFMT_LEFT,100,false,-1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Index, _T("Index"), LVCFMT_RIGHT, 60, false, -1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Source, _T("Source"), LVCFMT_LEFT, 300, false, -1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Component, _T("Component"), LVCFMT_LEFT, 100, false, -1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Method, _T("Method"), LVCFMT_LEFT, 100, false, -1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Level, _T("Level"), LVCFMT_LEFT, 100, false, -1));
        m_ColumnInfo.push_back(CColumnInfo(eETViewerColumn_Flag, _T("Flag"), LVCFMT_LEFT, 100, false, -1));
    }

    GetListCtrl().SetExtendedStyle(	LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_INFOTIP);

    m_OldListViewProc=(WNDPROC)GetWindowLong(GetListCtrl().m_hWnd,GWL_WNDPROC);
    SetWindowLong(GetListCtrl().m_hWnd,GWL_USERDATA,(DWORD)this);
    SetWindowLong(GetListCtrl().m_hWnd,GWL_WNDPROC,(DWORD)ListViewProc);

    ListView_SetImageList(GetListCtrl().m_hWnd,m_hImageList,LVSIL_SMALL);
    m_iHollowImage=ImageList_AddIcon(m_hImageList,m_hHollowIcon);
    m_iMarkerImage=ImageList_AddIcon(m_hImageList,m_hMarkerIcon);

    SetTimer(CAPTURE_TIMER,250,NULL);
    SetTimer(SHOW_LAST_TRACE_TIMER,1000,NULL);

    int iCurrentOrder=(m_ColumnInfo.size()-1);
    while(iCurrentOrder>=0) 
    {
        unsigned x;
        for(x=0;x<m_ColumnInfo.size();x++)
        {
            CColumnInfo *pColumn=&m_ColumnInfo[x];
            if(pColumn->visible && pColumn->iOrder==iCurrentOrder)
            {
                AddColumn(pColumn->id);
            }
        }
        iCurrentOrder--;
    } 
    UpdateColumns();
    UpdateFont();
    return 0;
}

void CETViewerView::UpdateColumns()
{
    m_mVisibleColumns.clear();

    int  pColumnOrder[100]={0};
    TCHAR sColumnText[1024]={0};
    CHeaderCtrl *pHeader=GetListCtrl().GetHeaderCtrl();
    unsigned nVisibleColumnCount=pHeader->GetItemCount();

    for(unsigned y=0;y<m_ColumnInfo.size();y++)
    {
        CColumnInfo *pColumn=&m_ColumnInfo[y];
        pColumn->iSubItem=-1;
        pColumn->iOrder=-1;
    }

    for(unsigned x=0;x<nVisibleColumnCount;x++)
    {
        LV_COLUMN column={0};
        column.mask=LVCF_SUBITEM|LVCF_TEXT;
        column.cchTextMax=_countof(sColumnText);
        column.pszText=sColumnText;
        GetListCtrl().GetColumn(x,&column);

        for(unsigned y=0;y<m_ColumnInfo.size();y++)
        {
            CColumnInfo *pColumn=&m_ColumnInfo[y];
            if(_tcscmp(pColumn->name.c_str(),sColumnText)==0)
            {
                pColumn->iSubItem=x;
                m_mVisibleColumns[x]=pColumn;				
                break;
            }
        }
    }

    GetListCtrl().GetColumnOrderArray(pColumnOrder,nVisibleColumnCount);
    for(unsigned x=0;x<nVisibleColumnCount;x++)
    {
        m_mVisibleColumns[pColumnOrder[x]]->iOrder=x;
    }

    GetListCtrl().RedrawItems(0,GetListCtrl().GetItemCount()-1);
}

void CETViewerView::UpdateFont()
{
    if(m_pTraceFont){delete m_pTraceFont;m_pTraceFont=NULL;}
    m_pTraceFont=new CFont;
    m_pTraceFont->CreateFont(m_dwTraceFontSize,0,0,0,0,FALSE,FALSE,FALSE,0,0,0,0,0,m_sTraceFont.c_str());
    GetListCtrl().SetFont(m_pTraceFont);
}


void CETViewerView::OnInitialUpdate()
{
    CListView::OnInitialUpdate();
}

// Diagnósticos de CETViewerView

#ifdef _DEBUG
void CETViewerView::AssertValid() const
{
    CListView::AssertValid();
}

void CETViewerView::Dump(CDumpContext& dc) const
{
    CListView::Dump(dc);
}

CETViewerDoc* CETViewerView::GetDocument() const // La versión de no depuración es en línea
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CETViewerDoc)));
    return (CETViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// Controladores de mensaje de CETViewerView
void CETViewerView::OnStyleChanged(int /*nStyleType*/, LPSTYLESTRUCT /*lpStyleStruct*/)
{
    //TODO: agregar código para que el usuario cambie el estilo de vista de la ventana
    
    Default();
}

void CETViewerView::OnDestroy()
{
    Clear();
    CListView::OnDestroy();
}

void CETViewerView::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMITEMACTIVATE *pActivate=(NMITEMACTIVATE *)pNMHDR;

    HMENU hMenu=GetSubMenu(LoadMenu(AfxGetResourceHandle(),MAKEINTRESOURCE(IDM_TRACE_LIST)),0);
    CMenu *pMenu=new CMenu;

    pMenu->Attach(hMenu);
    CMenu *pColumnsMenu=new CMenu;
    pColumnsMenu->CreatePopupMenu();
    for(unsigned x=0;x<m_ColumnInfo.size();x++)
    {
        DWORD flags=m_ColumnInfo[x].visible?MF_CHECKED:MF_UNCHECKED;
        pColumnsMenu->InsertMenu(0xFFFFFFFF,MF_BYPOSITION|flags,x+1,(TCHAR*)m_ColumnInfo[x].name.c_str());
    }
    pMenu->AppendMenu(MF_POPUP,(UINT_PTR)pColumnsMenu->m_hMenu,_T("Columns"));

    POINT p={0};
    ::GetCursorPos(&p);
    unsigned res=::TrackPopupMenu(pMenu->m_hMenu,TPM_LEFTALIGN|TPM_RETURNCMD,p.x,p.y,0,m_hWnd,NULL);
    if(res)
    {
        if(res<=m_ColumnInfo.size())
        {
            int colIndex=0,colCount=GetListCtrl().GetHeaderCtrl()->GetItemCount();
            CColumnInfo *pColumn=&m_ColumnInfo[res-1];
            pColumn->visible=!pColumn->visible;

            if(pColumn->visible)
            {
                AddColumn(pColumn->id);
            }
            else
            {
                RemoveColumn(pColumn->id);
            }
            UpdateColumns();
        }
        else if(res==ID_TRACE_LIST_OPEN_SOURCE_FILE)
        {
            if(pActivate->iItem!=-1)
            {
                std::tstring sFile;
                DWORD line=0;
                WaitForSingleObject(m_hTracesMutex,INFINITE);
                SETViewerTrace *pTrace=m_lTraces[pActivate->iItem];
                sFile=pTrace->trace.sSource.c_str();
                line=pTrace->trace.dwLine;
                ReleaseMutex(m_hTracesMutex);

                theApp.OpenCodeAddress(sFile.c_str(),line,true);
            }
        }
        else if(res==ID_TRACE_LIST_DELETE_SELECTION)
        {
            ClearSelected();
        }
        else if(res==ID_TRACE_LIST_DELETE_ALL)
        {
            Clear();
        }
        else if(res==ID_TRACE_LIST_COPY_SELECTION)
        {
            Copy(false);
        }
        else if(res==ID_TRACE_LIST_COPY_ALL)
        {
            Copy(true);
        }
        else if(res==ID_FONT_SMALLER)
        {
            OnFontSmaller();
        }
        else if(res==ID_FONT_BIGGER)
        {
            OnFontBigger();
        }
    }

    delete pColumnsMenu;
    delete pMenu;
}

void CETViewerView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMITEMACTIVATE *pActivate=(NMITEMACTIVATE*)pNMHDR;
    if(pActivate->iItem!=-1)
    {
        RECT R={0,0,1,1};
        HDC dc=::GetDC(m_hWnd);
        ::DrawText(dc,_T("A"),1,&R,DT_SINGLELINE|DT_CALCRECT);
        ::ReleaseDC(m_hWnd,dc);

        CRect rect;
        GetListCtrl().GetSubItemRect(pActivate->iItem,0,LVIR_LABEL,rect);
        unsigned index=(pActivate->ptAction.x-rect.left)/(R.right-R.left);

        TCHAR text[1024];
        GetListCtrl().GetItemText(pActivate->iItem,0,text,1024);
        unsigned length=(unsigned)_tcslen(text);
        if(index>=length){index=(length-1);}

        m_iEditSubItem=pActivate->iSubItem;
        GetListCtrl().EditLabel(pActivate->iItem);
        CEdit *pEdit=GetListCtrl().GetEditControl();
        if(pEdit)
        {
            RECT r={pActivate->ptAction.x,pActivate->ptAction.y,pActivate->ptAction.x,pActivate->ptAction.y};
            GetListCtrl().ClientToScreen(&r);
            GetListCtrl().GetEditControl()->ScreenToClient(&r);

            pEdit->PostMessage(EM_SETSEL,index,index);
            pEdit->PostMessage(WM_LBUTTONDOWN,0,MAKELONG(r.left,r.top));
            pEdit->PostMessage(WM_LBUTTONDBLCLK,0,MAKELONG(r.left,r.top));
        }
        m_iEditSubItem=0;
    }
    *pResult = 0;

}

void CETViewerView::GetTraceColors(SETViewerTrace *pTrace,COLORREF *pTextColor,COLORREF *pBkColor, HPEN *phPen,HBRUSH *phBrush)
{
    bool res=false;
    if(theApp.m_SplittedHighLightFilters.size()==0){return;}

    unsigned textLen=(unsigned)pTrace->trace.sText.size();
    const TCHAR *text=pTrace->trace.sText.c_str();

    TCHAR tempText[1024];
    textLen=textLen<(1024-1)?textLen:1024-1;
    memcpy(tempText,text,textLen);
    tempText[textLen]=0;

    unsigned x;
    for(x=0;x<textLen;x++)
    {
        if(tempText[x]>='a' && tempText[x]<='z'){tempText[x]+='A'-'a';}
    }


    // Text Filters must always be ordered by relevance, the first filter that matches the criteria
    // is the effective filter 


    for(x=0;x<theApp.m_SplittedHighLightFilters.size();x++)
    {
        CHightLightFilter *pFilter=&theApp.m_SplittedHighLightFilters[x];
        const TCHAR *pFilterText=pFilter->GetText().c_str();

        int index=0,maxTextSearchSize=textLen-pFilter->GetTextLen();
        if(maxTextSearchSize>0)
        {
            if(pFilterText[0]=='*')
            {
                *pTextColor=pFilter->GetTextColor();
                *pBkColor=pFilter->GetBkColor();
                *phPen=pFilter->GetPen();
                *phBrush=pFilter->GetBrush();
                return;
            }

            while(index<=maxTextSearchSize)
            {
                if(tempText[index]==pFilterText[0] && memcmp(tempText+index,pFilterText,pFilter->GetTextLen())==0)
                {
                    *pTextColor=pFilter->GetTextColor();
                    *pBkColor=pFilter->GetBkColor();
                    *phPen=pFilter->GetPen();
                    *phBrush=pFilter->GetBrush();
                    return;
                }
                index++;
            }
        }
    }
}

void CETViewerView::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
    LPNMLVCUSTOMDRAW  pDraw = (LPNMLVCUSTOMDRAW)pNMHDR;

    *pResult = CDRF_SKIPDEFAULT;

    switch(pDraw->nmcd.dwDrawStage) 
    {
    case CDDS_PREPAINT :
        *pResult=CDRF_NOTIFYITEMDRAW;
        return;	
    case CDDS_ITEMPREPAINT:
        *pResult=CDRF_NOTIFYSUBITEMDRAW;
        return;
    }
    if(pDraw->nmcd.dwDrawStage==(CDDS_ITEMPREPAINT | CDDS_SUBITEM))
    {
        SETViewerTrace *pTrace=NULL;
        CColumnInfo *pColumn=m_mVisibleColumns[pDraw->iSubItem];
        CRect labelRect,iconRect,fullItemRect;

        if(m_pEdit)
        {
            ExcludeClipRect(pDraw->nmcd.hdc,m_ListEditPos.x,m_ListEditPos.y,m_ListEditPos.x+m_ListEditSize.cx,m_ListEditPos.y+m_ListEditSize.cy);
        }

        WaitForSingleObject(m_hTracesMutex,INFINITE);
        pTrace=m_lTraces[pDraw->nmcd.dwItemSpec];

        GetListCtrl().GetSubItemRect((int)pDraw->nmcd.dwItemSpec,pDraw->iSubItem,LVIR_LABEL,labelRect);
        GetListCtrl().GetItemRect((int)pDraw->nmcd.dwItemSpec,iconRect,LVIR_ICON);
        GetListCtrl().GetItemRect((int)pDraw->nmcd.dwItemSpec,fullItemRect,LVIR_BOUNDS);

        TCHAR auxTextBuffer[1024];
        TCHAR *pText=GetTraceText(pTrace,m_mVisibleColumns[pDraw->iSubItem],auxTextBuffer,_countof(auxTextBuffer));
        if(pText==NULL){pText=auxTextBuffer;}
        
        pDraw->clrText=m_cNormalTextColor;
        pDraw->clrTextBk=m_cNormalBkColor;

        HPEN hPen=m_hNormalPen;
        HBRUSH hBrush=m_hNormalBrush;

        GetTraceColors(pTrace,&pDraw->clrText,&pDraw->clrTextBk,&hPen,&hBrush);
        DWORD state=GetListCtrl().GetItemState(pDraw->nmcd.dwItemSpec,LVIS_SELECTED|LVIS_FOCUSED);
        if(state&LVIS_SELECTED)
        {
            pDraw->clrText=m_cSelectedTextColor;
            pDraw->clrTextBk=m_cSelectedBkColor;
            hPen=m_hSelectedPen;
            hBrush=m_hSelectedBrush;
        }
        
        HGDIOBJ hOldBrush=SelectObject(pDraw->nmcd.hdc,hBrush);
        HGDIOBJ hOldPen=SelectObject(pDraw->nmcd.hdc,hPen);

        SetTextColor(pDraw->nmcd.hdc,pDraw->clrText);
        SetBkColor(pDraw->nmcd.hdc,pDraw->clrTextBk);

        if(pColumn->iOrder==0)
        {
            Rectangle(pDraw->nmcd.hdc,0,iconRect.top,iconRect.right,iconRect.bottom);
            DrawIconEx(pDraw->nmcd.hdc,iconRect.left,iconRect.top,pTrace->iImage==m_iMarkerImage?m_hMarkerIcon:m_hHollowIcon,16,16,0,NULL,DI_NORMAL);
        }
        Rectangle(pDraw->nmcd.hdc,labelRect.left,labelRect.top,labelRect.right,labelRect.bottom);

        if(state&LVIS_FOCUSED && GetFocus()==this)
        {
            SelectObject(pDraw->nmcd.hdc,m_hHollowBrush);
            Rectangle(pDraw->nmcd.hdc,fullItemRect.left,fullItemRect.top,fullItemRect.right,fullItemRect.bottom);
            DrawFocusRect(pDraw->nmcd.hdc,&fullItemRect);
        }
        SelectObject(pDraw->nmcd.hdc,hOldBrush);
        SelectObject(pDraw->nmcd.hdc,hOldPen);

        DWORD dwAlign=DT_LEFT;
        if(pColumn->format==LVCFMT_LEFT){dwAlign=DT_LEFT;}
        else if(pColumn->format==LVCFMT_CENTER){dwAlign=DT_CENTER;}
        else if(pColumn->format==LVCFMT_RIGHT){dwAlign=DT_RIGHT;}
        DrawText(pDraw->nmcd.hdc,pText,-1,&labelRect,DT_END_ELLIPSIS|DT_SINGLELINE|DT_NOPREFIX|DT_VCENTER|dwAlign);
        ReleaseMutex(m_hTracesMutex);

        *pResult = CDRF_SKIPDEFAULT;
    }
}

TCHAR *CETViewerView::GetTraceText(SETViewerTrace *pTrace,CColumnInfo *pColumn,TCHAR *pAuxBuffer,unsigned nAuxLen)
{
    pAuxBuffer[0]=0;

    switch(pColumn->id)
    {
    case eETViewerColumn_Text:
        _tcscpy_s(pAuxBuffer,nAuxLen,pTrace->trace.sText.c_str());
        break;
    case eETViewerColumn_PIDTID:
        _stprintf_s (pAuxBuffer,nAuxLen,_T("[%4d]-[0x%04x]"),pTrace->trace.dwProcessId,pTrace->trace.dwThreadId);
        break;
    case eETViewerColumn_Time:
        _stprintf_s(pAuxBuffer, nAuxLen, _T("%02d:%02d:%02d.%03d"), (DWORD)pTrace->trace.systemTime.wHour, (DWORD)pTrace->trace.systemTime.wMinute, (DWORD)pTrace->trace.systemTime.wSecond, (DWORD)pTrace->trace.systemTime.wMilliseconds);
        break;
    case eETViewerColumn_TimeStamp:
        _stprintf_s(pAuxBuffer, nAuxLen, _T("%f"), ((double)pTrace->trace.timeStamp.QuadPart) / 10000000.0);
        break;	
    case eETViewerColumn_Source:
        _stprintf_s(pAuxBuffer, nAuxLen, _T("%s(%d)"), pTrace->trace.sSourceFile.c_str(), pTrace->trace.dwLine);
        break;
    case eETViewerColumn_Index:
        _stprintf_s(pAuxBuffer, nAuxLen, _T("%d "), pTrace->trace.dwSequenceIndex);
        break;
    case eETViewerColumn_Component:
        _tcscpy_s(pAuxBuffer,nAuxLen,pTrace->trace.sComponent.c_str());
        break;
    case eETViewerColumn_Method:
        _tcscpy_s(pAuxBuffer,nAuxLen,pTrace->trace.sFunction.c_str());
        break;
    case eETViewerColumn_Flag:
        _tcscpy_s(pAuxBuffer,nAuxLen,pTrace->trace.sFlag.c_str());
        break;
    case eETViewerColumn_Level:
        _tcscpy_s(pAuxBuffer,nAuxLen,pTrace->trace.sLevel.c_str());
        break;
    }
    return NULL;
}

LRESULT CALLBACK CETViewerView::ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CETViewerView *pThis=(CETViewerView *)GetWindowLong(hwnd,GWL_USERDATA);
    if(uMsg==WM_ERASEBKGND)
    {
        int itemCount=pThis->GetListCtrl().GetItemCount();
        if(itemCount)
        {
            RECT firstRect,lastRect;
            pThis->GetListCtrl().GetItemRect(0,&firstRect,LVIR_BOUNDS);
            pThis->GetListCtrl().GetItemRect(itemCount-1,&lastRect,LVIR_BOUNDS);

            DWORD dwSize=pThis->GetListCtrl().SendMessage(LVM_APPROXIMATEVIEWRECT,-1,MAKELONG(0,pThis->GetListCtrl().GetItemCount()));
            SIZE size;
            size.cx=LOWORD(dwSize);
            size.cy=HIWORD(dwSize);

            HRGN hClip=CreateRectRgn(0,0,0xFFFF,0xFFFF);
            ExcludeClipRect((HDC)wParam,firstRect.left,firstRect.top,firstRect.right,lastRect.bottom);
            LRESULT lResult=CallWindowProc(pThis->m_OldListViewProc,hwnd,uMsg,wParam,lParam);
            SelectClipRgn((HDC)wParam,hClip);
            DeleteObject(hClip);
            return lResult;		
        }

    }
    
    if(uMsg==WM_KEYDOWN){pThis->ProcessSpecialKeyStroke(wParam);}
    if(uMsg==WM_MOUSEWHEEL)
    {
        bool pushedLControl=(GetKeyState(VK_LCONTROL)>>15)?true:false;
        bool pushedRControl=(GetKeyState(VK_RCONTROL)>>15)?true:false;
        bool pushedControl=(pushedLControl||pushedRControl);
        if(pushedControl)
        {
            int nDelta=(short)HIWORD(wParam); 
            if(nDelta>0)
            {
                pThis->OnFontBigger();
            }
            else
            {
                pThis->OnFontSmaller();
            }
        }
    }
    if(uMsg==WM_NOTIFY)
    {
        LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lParam);
        if(pNMHDR->code==HDN_ENDDRAG)
        {
            LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(lParam);
            CColumnInfo *pDraggedColumn=pThis->m_mVisibleColumns[phdr->iItem];

            int iIndex=phdr->iItem;
            int iNewPosition=phdr->pitem->iOrder;
            int iOldPosition=pDraggedColumn->iOrder;

            if(iNewPosition==0 || iOldPosition==0)
            {
                // Si cambia la columna en la posicion 0, procesarla de forma diferente para
                // que la columna con indice 0 siempre quede en primera posicion para que se vea el icono 
                // al principio.
                ::PostMessage(pThis->m_hWnd,WM_USER+0x101,0,0);
            }
            else
            {
                ::PostMessage(pThis->m_hWnd,WM_USER+0x100,0,0);
            }
        }
    }
    if(uMsg==WM_USER+0x100)
    {
        pThis->UpdateColumns();
    }
    if(uMsg==WM_USER+0x101)
    {
        // Para forzar a que la columna con indice 0 sea la primera se busca la que esta en la posicion 0
        // y elimina y se reinserta.

        // Actualizamos las posiciones despues de drop.
        pThis->UpdateColumns();

        // Coger la columna en la posicion 0.
        for(unsigned x=0;x<pThis->m_ColumnInfo.size();x++)
        {
            CColumnInfo *pColumn=&pThis->m_ColumnInfo[x];
            if(pColumn->visible && pColumn->iOrder==0)
            {
                // Asegurarnos de que es la columna 0
                pThis->RemoveColumn(pColumn->id);
                pThis->AddColumn(pColumn->id);
            }
        }
        // Actualizar indices y posiciones de nuevo.
        pThis->UpdateColumns();
    }	
    return CallWindowProc(pThis->m_OldListViewProc,hwnd,uMsg,wParam,lParam);
}

void CETViewerView::AddColumn(int id)
{
    CColumnInfo *pColumn=&m_ColumnInfo[id-eETViewerColumn_BASE];
    GetListCtrl().InsertColumn(0,pColumn->name.c_str(),pColumn->format,pColumn->width,pColumn->id);
}

void CETViewerView::RemoveColumn(int id)
{
    CColumnInfo *pColumn=&m_ColumnInfo[id-eETViewerColumn_BASE];
    int newSize=GetListCtrl().GetColumnWidth(pColumn->iSubItem);
    if(newSize){pColumn->width=newSize;}
    GetListCtrl().DeleteColumn(pColumn->iSubItem);
}

LRESULT CALLBACK CETViewerView::ListEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CETViewerView *pThis=(CETViewerView *)GetWindowLong(hwnd,GWL_USERDATA);
    if(uMsg==WM_MOVE)
    {
        ::SetWindowPos(hwnd,NULL,pThis->m_ListEditPos.x,pThis->m_ListEditPos.y,pThis->m_ListEditSize.cx,pThis->m_ListEditSize.cy,SWP_NOZORDER);
    }
    if(uMsg==WM_KEYDOWN)
    {
        bool pushedLControl=(GetKeyState(VK_LCONTROL)>>15)?true:false;
        bool pushedRControl=(GetKeyState(VK_RCONTROL)>>15)?true:false;
        bool pushedControl=(pushedLControl||pushedRControl);
        if(wParam=='F' && (pushedControl))
        {
            int begin=0,end=0;
            TCHAR text[1024]={0};
            TCHAR textToFind[1024]={0};
            pThis->GetListCtrl().GetEditControl()->GetSel(begin,end);
            if(begin!=end)
            {
                pThis->GetListCtrl().GetEditControl()->GetWindowText(text,1024);
                if(_tcslen(text)!=0)
                {
                    int len=min(end-begin,sizeof(pThis->m_LastTextToFind)-1);
                    _tcsncpy_s(textToFind,text+begin,len);
                    pThis->m_LastTextToFind=textToFind;
                }
            }
            pThis->BeginFind(pThis,pThis->GetListCtrl().m_hWnd,(TCHAR*)pThis->m_LastTextToFind.c_str());
        }
    }
    if(uMsg==WM_RBUTTONDOWN)
    {
        if(pThis->GetListCtrl().GetEditControl()==NULL ||pThis->GetListCtrl().GetEditControl()->m_hWnd==NULL){return 0;}
        int begin=0,end=0;
        TCHAR text[1024],codeAddress[1024]={0};
        POINT p;
        ::GetCursorPos(&p);
        pThis->GetListCtrl().GetEditControl()->GetSel(begin,end);
        pThis->GetListCtrl().GetEditControl()->GetWindowText(text,1024);
        HMENU hMenu=GetSubMenu(LoadMenu(AfxGetResourceHandle(),MAKEINTRESOURCE(IDM_TRACE_LIST_EDIT)),0);

        DWORD command=::TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_RETURNCMD,p.x,p.y,0,pThis->GetListCtrl().m_hWnd,NULL);
        switch(command)
        {
         case ID_GET_ERROR_DESCRIPTION:
            if(begin!=-1 && end!=-1)
            {
                memcpy(codeAddress,text+begin,end-begin);
                theApp.LookupError(codeAddress);
            }
            break;
        case ID_COPY:
            pThis->GetListCtrl().GetEditControl()->Copy();
            break;
        case ID_SELECT_ALL:
            pThis->GetListCtrl().GetEditControl()->SetSel(0,-1);
            break;
        }

        return 0L;
    }
    return CallWindowProc(pThis->m_OldListEditProc,hwnd,uMsg,wParam,lParam);
}

bool CETViewerView::FindNext()
{
    if(m_LastTextToFind==_T("")){return false;}
    return FindNext(m_LastTextToFind.c_str());
}
    
bool CETViewerView::FindNext(const TCHAR *pText)
{	
    if(_tcscmp(pText,_T(""))==0){return false;}

    m_LastTextToFind=pText;

    int sel=GetListCtrl().GetNextItem(-1,LVIS_FOCUSED);
    int index=FindText(pText,sel==-1?0:sel+(m_bFindDirectionUp?-1:1),m_bFindDirectionUp);
    if(index==-1)
    {
        CWnd *pParent=GetActiveWindow();
        if(!pParent){pParent=this;}

        std::tstring str = pText;
        str+=_T(" was not found");
        pParent->MessageBox(str.c_str(),_T("ETViewer"),MB_OK);
        return false;
    }
    else
    {
        sel=GetListCtrl().GetNextItem(-1,LVIS_FOCUSED);
        GetListCtrl().SetItemState(sel,0,LVIS_SELECTED|LVIS_FOCUSED);
        GetListCtrl().SetItemState(index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
        GetListCtrl().EnsureVisible(index,FALSE);
        return true;
    }
}

int CETViewerView::FindText(const TCHAR* pTextToFind,int baseIndex,bool up,bool stopAtEnd)
{
    TCHAR textToFind[1024];
    _tcscpy_s(textToFind,pTextToFind);
    if(!m_bMatchCaseInFind){_tcsupr_s(textToFind);}

    CHeaderCtrl* pHeader = GetListCtrl().GetHeaderCtrl();

    int x=0,count=GetListCtrl().GetItemCount(),increment=up?-1:1;
    baseIndex+=count; // to decrement with no danger.

    for(x=0;x<count;x+=increment)
    {
        int index=(baseIndex+x)%count;

        TCHAR text[1024];
        GetListCtrl().GetItemText(index,eETViewerColumn_Text,text,1000);
        if(!m_bMatchCaseInFind){_tcsupr_s(text);}
        if(_tcsstr(text,textToFind)!=NULL){return index;}

        if(stopAtEnd && up  && index==0)
        {break;}
        if(stopAtEnd && !up && index==(GetListCtrl().GetItemCount()-1))
        {break;}
    }
    return -1;
}

bool CETViewerView::FindAndMarkAll(const TCHAR *pTextToFind)
{
    bool res=false;
    int currentIndex=0;
    while(currentIndex<GetListCtrl().GetItemCount() && (currentIndex=FindText(pTextToFind,currentIndex,false,true))!=-1)	
    {
        WaitForSingleObject(m_hTracesMutex,INFINITE);
        SETViewerTrace *pTrace=m_lTraces[currentIndex];
        pTrace->iImage=(pTrace->iImage==m_iMarkerImage)?m_iHollowImage:m_iMarkerImage;
        ReleaseMutex(m_hTracesMutex);

        GetListCtrl().RedrawItems(currentIndex,currentIndex);
        res=true;

        currentIndex++;
    }
    return res;
}

bool CETViewerView::FindAndDeleteAll(const TCHAR *pTextToFind)
{
    WaitForSingleObject(m_hTracesMutex,INFINITE);

    std::deque<SETViewerTrace *> newList;

    bool res=false;
    int currentIndex=0,oldIndex=-1;
    while(currentIndex<GetListCtrl().GetItemCount() && (currentIndex=FindText(pTextToFind,currentIndex,false,true))!=-1)
    {
        for(int x=oldIndex+1;x<currentIndex;x++)
        {
            newList.push_back(m_lTraces[x]);
        }
        if(!m_lTraces[currentIndex]->trace.bFormatted)
        {
            m_nUnformattedTraces--;
        }
        delete m_lTraces[currentIndex];
        oldIndex=currentIndex;
        currentIndex++;
        res=true;
    }

    for(unsigned x=oldIndex+1;x<m_lTraces.size();x++)
    {
        newList.push_back(m_lTraces[x]);
    }

    m_lTraces=newList;

    GetListCtrl().SetItemCount((int)m_lTraces.size());

    ReleaseMutex(m_hTracesMutex);
    return res;
}

void CETViewerView::ProcessSpecialKeyStroke(WORD wParam)
{
    bool pushedLControl=(GetKeyState(VK_LCONTROL)>>15)?true:false;
    bool pushedRControl=(GetKeyState(VK_RCONTROL)>>15)?true:false;
    bool pushedLShift=(GetKeyState(VK_LSHIFT)>>15)?true:false;
    bool pushedRShift=(GetKeyState(VK_RSHIFT)>>15)?true:false;
    bool pushedControl=(pushedLControl||pushedRControl);
    bool pushedShift=(pushedLShift||pushedRShift);
    if(wParam=='X' && (pushedControl))  {Clear();}
    if(wParam=='C' && (pushedControl))  {Copy(false);}
    if(wParam=='S' && (pushedControl))  {OnSave();SetFocusOnOwnerWindow();}
    if(wParam=='F' && (pushedControl))  {OnFind();}
    if(wParam=='H' && (pushedControl))  {OnHighLightFilters();SetFocusOnOwnerWindow();}

    if(wParam==VK_F3 && !(pushedControl))
    {
        bool dir=m_bFindDirectionUp;
        m_bFindDirectionUp=pushedShift;
        FindNext();
        m_bFindDirectionUp=dir;
    }

    if(wParam==VK_F2 && !(pushedControl) && !(pushedShift))
    {
        LVITEM current={0};
        current.iItem=0;
        current.mask=LVIF_IMAGE;

        int index,beginIndex=0;
        int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
        if(sel!=-1)
        {
            current.iItem=sel;
            GetListCtrl().GetItem(&current);
            if(current.iImage==m_iMarkerImage){beginIndex=sel+1;}
        }
        // From begin to end;
        DWORD x,count=GetListCtrl().GetItemCount();
        for(x=0;x<count;x++)
        {
            index=(beginIndex+x)%count;
            current.iItem=index;
            GetListCtrl().GetItem(&current);
            if(current.iImage==m_iMarkerImage)
            {
                GetListCtrl().SetItemState(sel,0,LVIS_SELECTED|LVIS_FOCUSED);
                GetListCtrl().SetItemState(index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
                GetListCtrl().EnsureVisible(index,FALSE);
                break;
            }
        }
    }

    if(wParam==VK_F2 && (pushedControl) && !(pushedShift))
    {
        int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
        while(sel!=-1)
        {
            WaitForSingleObject(m_hTracesMutex,INFINITE);
            SETViewerTrace *pTrace=m_lTraces[sel];
            pTrace->iImage=(pTrace->iImage==m_iMarkerImage)?m_iHollowImage:m_iMarkerImage;
            ReleaseMutex(m_hTracesMutex);

            GetListCtrl().RedrawItems(sel,sel);

            sel=GetListCtrl().GetNextItem(sel,LVNI_SELECTED);
        }
    }

    if(wParam==VK_F2 && (pushedControl) && (pushedShift))
    {
        WaitForSingleObject(m_hTracesMutex,INFINITE);
        for(unsigned x=0;x<m_lTraces.size();x++)
        {
            SETViewerTrace *pTrace=m_lTraces[x];
            pTrace->iImage=m_iHollowImage;
        }
        ReleaseMutex(m_hTracesMutex);
        GetListCtrl().RedrawItems(0,GetListCtrl().GetItemCount()-1);
    }

    if(wParam==VK_SPACE)
    {
        if(m_ColumnInfo[eETViewerColumn_Text-eETViewerColumn_BASE].visible)
        {
            m_iEditSubItem=m_ColumnInfo[eETViewerColumn_Text-eETViewerColumn_BASE].iSubItem;
            int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
            GetListCtrl().EditLabel(sel);
        }
    }
    if(wParam==VK_DELETE)
    {
        ClearSelected();
    }
}

void CETViewerView::SetFocusOnOwnerWindow()
{
    SetFocus();
}

void CETViewerView::OnFind() 
{
    BeginFind(this,GetListCtrl().m_hWnd,m_LastTextToFind.c_str());
}

void CETViewerView::OnEditFind()
{
    OnFind();
}

void CETViewerView::ClearSelected() 
{
    int x,c;

    WaitForSingleObject(m_hTracesMutex,INFINITE);

    std::deque<SETViewerTrace *> newList;
    c=GetListCtrl().GetItemCount();
    for(x=c-1;x>=0;x--)
    {
        if(GetListCtrl().GetItemState(x,LVIS_SELECTED)&LVIS_SELECTED)
        {
            GetListCtrl().SetItemState(x,0,LVIS_SELECTED);

            if(!m_lTraces[x]->trace.bFormatted)
            {
                m_nUnformattedTraces--;
            }
            delete m_lTraces[x];
        }
        else
        {
            newList.push_front(m_lTraces[x]);
        }
    }	
    m_lTraces=newList;

    GetListCtrl().SetItemCount((int)m_lTraces.size());
    ReleaseMutex(m_hTracesMutex);
}

void CETViewerView::Copy(bool bAllTraces) 
{
    if(!OpenClipboard()){MessageBox(_T("Failed to open clipboard"),_T("Error"),MB_OK);}

    EmptyClipboard();
    TCHAR			*buffer=NULL;
    void			*globalBuffer=NULL;
    int				nSize=0;

    GetTransferBuffer(&nSize,&buffer,bAllTraces);
    if(nSize==0){CloseClipboard();return;}

    HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE,nSize); 
    if (hglbCopy == NULL){CloseClipboard();return ;}

    globalBuffer = GlobalLock(hglbCopy); 
    if(globalBuffer)
    {
        memcpy(globalBuffer ,buffer,nSize);
    }
    GlobalUnlock(hglbCopy); 

    if (SetClipboardData(CF_UNICODETEXT, hglbCopy) == NULL)
    {MessageBox(_T("Failed to copy data to clipboard"),_T("Error"),MB_OK);}

    CloseClipboard();
    delete [] buffer;
}

void CETViewerView::OnSave() 
{
    TCHAR			*buffer=NULL;
    int				nSize=0;
    bool			bAllTraces=true;

    if(GetListCtrl().GetNextItem(-1,LVNI_SELECTED)!=-1)
    {
        CSaveAllTracesQuestionDialog dialog;
        int res=dialog.DoModal();
        bAllTraces=(res==IDOK);
        if(res==IDCANCEL){return;}
    }

    GetTransferBuffer(&nSize,&buffer,bAllTraces);
    if(nSize==0){return;}

    CFileDialog dialog(FALSE,_T("txt"),NULL,OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT,_T("Text Files(*.txt)|*.txt|Log Files(*.log)|*.log|All Files(*.*)|*.*"));
    if(dialog.DoModal()==IDOK)
    {
        DWORD written=0;
        HANDLE hFile=CreateFile(dialog.GetOFN().lpstrFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
        if(hFile==INVALID_HANDLE_VALUE)
        {
            TCHAR sTemp[1024];
            _stprintf_s (sTemp,_T("Cannot create '%s' due to error %d"),dialog.GetOFN().lpstrFile,GetLastError());
            MessageBox(sTemp,_T("ETViewer"),MB_ICONSTOP|MB_OK);
            return;
        }
        //theApp.AddRecentTraceFile(VAW2A(info.fileName));
        WriteFile(hFile,buffer,nSize,&written,NULL);
        CloseHandle(hFile);
        hFile=NULL;
    }
}

void CETViewerView::Clear() 
{
    WaitForSingleObject(m_hTracesMutex,INFINITE);
    for(unsigned x=0;x<m_lTraces.size();x++)
    {
        delete m_lTraces[x];
    }
    m_lTraces.clear();
    m_nUnformattedTraces=0;

    GetListCtrl().DeleteAllItems();
    ReleaseMutex(m_hTracesMutex);
    //UpdateBitmaps();
}

void CETViewerView::GetTransferBuffer(int *pSize,TCHAR **buffer,bool bAllTraces)
{
    (*buffer)=NULL;
    (*pSize)=0;
    if(bAllTraces)
    {
        TCHAR temp[2048],columnText[2048];
        int index=0,count=GetListCtrl().GetItemCount();
        int columns=GetListCtrl().GetHeaderCtrl()->GetItemCount();
        DWORD size=0;
        *pSize=(count*1024);
        (*buffer)=new TCHAR[*pSize];
        if((*buffer)==NULL)
        {
            MessageBox(_T("Cannot allocate enough memory to perform the operation"),_T("ETViewer"),MB_ICONSTOP|MB_OK);
            return;
        }
        (*buffer)[0]=0;
        for(index=0;index<count;index++)
        {
            int templen=0;
            temp[0]=0;
            for(int x=0;x<columns;x++)
            {
                GetListCtrl().GetItemText(index,x,columnText,1000);
                // -3 : \r\n\0
                templen+=_sntprintf_s(temp+templen,_countof(temp)-templen, sizeof(temp)-3-templen,x==0?_T("%s"):_T("\t%s"),columnText);
                (*buffer)[size]=0;
            }
            temp[templen]=_T('\r');
            temp[templen+1]=_T('\n');
            temp[templen+2]=0;
            templen+=2;
            memcpy((*buffer) + size, temp, templen * sizeof(TCHAR) + sizeof(TCHAR));
            size+=templen;
        }
        *pSize = size * sizeof(TCHAR) + sizeof(TCHAR);
    }
    else
    {
        int x;
        TCHAR temp[2048],columnText[2048];
        std::list<DWORD>            selectedIndexes;
        std::list<DWORD>::iterator  i;
        for(x=0;x<GetListCtrl().GetItemCount();x++)
        {
            if(GetListCtrl().GetItemState(x,LVIS_SELECTED)&LVIS_SELECTED)
            {selectedIndexes.push_back(x);}
        }

        // insert all.
        if(selectedIndexes.size()==0)
        {
            for(int x=0;x<GetListCtrl().GetItemCount();x++)
            {
                selectedIndexes.push_back(x);
            }

        }

        int columns=GetListCtrl().GetHeaderCtrl()->GetItemCount();
        DWORD size=0;
        *pSize=((int)selectedIndexes.size()*1024);
        (*buffer)=new TCHAR [*pSize];
        if((*buffer)==NULL)
        {
            MessageBox(_T("Cannot allocate enough memory to perform the operation"),_T("ETViewer"),MB_ICONSTOP|MB_OK);
            return;
        }
        (*buffer)[0]=0;
        for(i=selectedIndexes.begin();i!=selectedIndexes.end();i++)
        {
            int templen=0;
            temp[0]=0;
            for(x=0;x<columns;x++)
            {
                int index=(*i);
                GetListCtrl().GetItemText(index,x,columnText,1000);
                // -3 : \r\n\0
                templen+=_sntprintf_s(temp+templen,_countof(temp)-templen,_countof(temp)-3-templen,x==0?_T("%s"):_T("\t%s"),columnText);
                (*buffer)[size]=0;
            }
            temp[templen]=_T('\r');
            temp[templen+1]=_T('\n');
            temp[templen+2]=0;
            templen+=2;
            memcpy((*buffer) + size, temp, templen * sizeof(TCHAR) + sizeof(TCHAR));
            size+=templen;
        }
        *pSize = size * sizeof(TCHAR) + sizeof(TCHAR);
    }
}

void CETViewerView::OnHighLightFilters() 
{
    CHighLightFiltersEditor dialog;
    dialog.DoModal();
    GetListCtrl().RedrawItems(0,GetListCtrl().GetItemCount());
    theApp.m_pFrame->GetHighLightPane()->LoadFilters();
}


void CETViewerView::ProcessTrace(STraceEvenTracingNormalizedData *pTraceData)
{
    if(pTraceData->bFormatted)
    {
        if(!theApp.FilterTrace(pTraceData->sText.c_str())){return;}
    }

    SETViewerTrace *pTrace=new SETViewerTrace;
    pTrace->trace=*pTraceData;
    pTrace->trace.nParamBuffer=0;
    pTrace->trace.pParamBuffer=NULL;

    WaitForSingleObject(m_hTracesMutex,INFINITE);
    m_lTraces.push_back(pTrace);
    ReleaseMutex(m_hTracesMutex);
}

void CETViewerView::ProcessUnknownTrace(STraceEvenTracingNormalizedData *pTraceData)
{
    TCHAR sText[1024]={0};
    WCHAR CLSID[100]={0};
    StringFromGUID2(pTraceData->sourceFileGUID,CLSID,sizeof(CLSID)/2);

    _stprintf_s(sText,_T("Unknown Trace, Source GUID %ws, Source Index %d"),CLSID,pTraceData->sourceTraceIndex);

    SETViewerTrace *pTrace=new SETViewerTrace;
    pTrace->trace=*pTraceData;
    pTrace->trace.sText=sText;
    if(pTraceData->nParamBuffer)
    {
        pTrace->trace.nParamBuffer=pTraceData->nParamBuffer;
        pTrace->trace.pParamBuffer=new unsigned char [pTraceData->nParamBuffer];
        memcpy(pTrace->trace.pParamBuffer,pTraceData->pParamBuffer,pTraceData->nParamBuffer);
    }

    WaitForSingleObject(m_hTracesMutex,INFINITE);
    m_lTraces.push_back(pTrace);
    m_nUnformattedTraces++;
    ReleaseMutex(m_hTracesMutex);
}

void CETViewerView::OnTimer(UINT nIDEvent) 
{
    if(nIDEvent==CAPTURE_TIMER)
    {
        theApp.m_Controller.FlushTraces();

        KillTimer(CAPTURE_TIMER);

        WaitForSingleObject(m_hTracesMutex,INFINITE);
        if(GetListCtrl().GetItemCount()!=m_lTraces.size())
        {
            SortItems(&m_ColumnInfo[m_SortColumn-eETViewerColumn_BASE]);
            GetListCtrl().SetItemCountEx((int)m_lTraces.size(),LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);
            GetListCtrl().RedrawItems(0,GetListCtrl().GetItemCount()-1);
        }
        ReleaseMutex(m_hTracesMutex);
    
        SetTimer(CAPTURE_TIMER,250,NULL);
    }

    if(nIDEvent==SHOW_LAST_TRACE_TIMER)
    {
        if(m_bShowLastTrace)
        {
            int nCount=0;
            bool bShowLastItem=false;
            WaitForSingleObject(m_hTracesMutex,INFINITE);
            nCount=GetListCtrl().GetItemCount();
            if(nCount)
            {
                SETViewerTrace *pLastTrace=m_lTraces[nCount-1];
                if(pLastTrace->trace.dwSequenceIndex!=m_nLastFocusedSequenceIndex)
                {
                    m_nLastFocusedSequenceIndex=pLastTrace->trace.dwSequenceIndex;
                    bShowLastItem=true;
                }
            }
            ReleaseMutex(m_hTracesMutex);

            if(bShowLastItem)
            {
                GetListCtrl().EnsureVisible(GetListCtrl().GetItemCount()-1,FALSE);
            }
        }
    }
}

void CETViewerView::OnClear()
{
    Clear();
}

void CETViewerView::OnCut()
{
    Clear();
}

void CETViewerView::OnCopy()
{
    Copy(false);
}

void CETViewerView::OnBeginEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    m_pEdit=GetListCtrl().GetEditControl();

    int iItem=pDispInfo->item.iItem;
    int iSubItem=m_mVisibleColumns[m_iEditSubItem]->iSubItem;

    TCHAR text[1024];
    CRect labelRect,clientRect;
    GetListCtrl().GetSubItemRect(iItem,iSubItem,LVIR_LABEL,labelRect);
    GetListCtrl().GetClientRect(&clientRect);
    GetListCtrl().GetItemText(iItem,iSubItem,text,1024);
    int length=(int)_tcslen(text);

    RECT singleCharRect={0,0,1,1};
    RECT textRect={0,0,1,1};
    HDC dc=::GetDC(m_hWnd);
    ::SelectObject(dc,m_pTraceFont->m_hObject);
    ::DrawText(dc,_T("A"),1,&singleCharRect,DT_SINGLELINE|DT_CALCRECT);
    ::DrawText(dc,text,length,&textRect,DT_SINGLELINE|DT_CALCRECT);
    ::ReleaseDC(m_hWnd,dc);

    int textSize=(textRect.right-textRect.left)+6;
    m_ListEditPos.x=labelRect.left>0?labelRect.left:0;
    m_ListEditPos.y=labelRect.top;
    m_ListEditSize.cx=textSize;
    m_ListEditSize.cy=labelRect.bottom-labelRect.top;

    int nMaximunWidth=clientRect.Size().cx;
    if(m_ListEditPos.x+m_ListEditSize.cx>nMaximunWidth)
    {
        m_ListEditPos.x=nMaximunWidth-m_ListEditSize.cx;
        if(m_ListEditPos.x<0)
        {
            m_ListEditPos.x=0;
            m_ListEditSize.cx=nMaximunWidth;
        }
    }

    m_pEdit->SetWindowText(text);

    // Set Selection

    int offset=labelRect.left<0?-labelRect.left:0;
    int index=(offset)/(singleCharRect.right-singleCharRect.left);
    if(index>=length){index=(length-1);}

    m_pEdit->PostMessage(EM_SETSEL,0,0);
    m_pEdit->PostMessage(EM_SETSEL,index,index);
    m_pEdit->PostMessage(EM_SETREADONLY,TRUE,0);

    // Hook window

    m_OldListEditProc=(WNDPROC)GetWindowLong(m_pEdit->m_hWnd,GWL_WNDPROC);
    SetWindowLong(m_pEdit->m_hWnd,GWL_USERDATA,(DWORD)this);
    SetWindowLong(m_pEdit->m_hWnd,GWL_WNDPROC,(DWORD)ListEditProc);

    *pResult = 0;
}

bool CETViewerView::IsShowLastTraceEnabled()
{
    return m_bShowLastTrace;
}

void CETViewerView::ResetShowLastTrace()
{
    WaitForSingleObject(m_hTracesMutex,INFINITE);
    m_nLastFocusedSequenceIndex=0;
    unsigned nCount=m_lTraces.size();
    if(nCount)
    {
        SETViewerTrace *pLastTrace=m_lTraces[nCount-1];
        m_nLastFocusedSequenceIndex=pLastTrace->trace.dwSequenceIndex;
    }
    ReleaseMutex(m_hTracesMutex);
}

void CETViewerView::EnableShowLastTrace(bool bEnable)
{
    ResetShowLastTrace();
    m_bShowLastTrace=bEnable;
}
void CETViewerView::OnGetItemInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    // TODO: Add your control notification handler code here

    WaitForSingleObject(m_hTracesMutex,INFINITE);

    if(pDispInfo->item.mask&LVIF_TEXT)
    {
        SETViewerTrace *pTrace=m_lTraces[pDispInfo->item.iItem];

//		_stprintf_s (pDispInfo->item.pszText,_T("%d"),pDispInfo->item.iSubItem);
        
        CColumnInfo *pColumnInfo=NULL;
        if(pDispInfo->item.iSubItem<eETViewerColumn_BASE)
        {
            pColumnInfo=m_mVisibleColumns[pDispInfo->item.iSubItem];
        }
        else
        {
            pColumnInfo=&m_ColumnInfo[pDispInfo->item.iSubItem-eETViewerColumn_BASE];
        }
        TCHAR *pText=GetTraceText(pTrace,pColumnInfo,pDispInfo->item.pszText,pDispInfo->item.cchTextMax);
        if(pText){_tcscpy_s(pDispInfo->item.pszText,pDispInfo->item.cchTextMax,pText);}
    }
    if(pDispInfo->item.mask&LVIF_PARAM)
    {
        pDispInfo->item.lParam=(LPARAM)m_lTraces[pDispInfo->item.iItem];
    }
    if(pDispInfo->item.mask&LVIF_IMAGE)
    {
        SETViewerTrace *pTrace=m_lTraces[pDispInfo->item.iItem];
        pDispInfo->item.iImage=pTrace->iImage;
    }
    ReleaseMutex(m_hTracesMutex);
    *pResult = 0;
}


void CETViewerView::OnEndEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
    m_pEdit=NULL;
    *pResult = 0;
}

void CETViewerView::OnFontBigger()
{
    if(m_dwTraceFontSize<100)
    {
        m_dwTraceFontSize+=2;
        UpdateFont();
    }
}

void CETViewerView::OnFontSmaller()
{
    if(m_dwTraceFontSize>4)
    {
        m_dwTraceFontSize-=2;
        UpdateFont();
    }
}
class CSortPredicate{public :virtual bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return false;}};

class CSortPredicateAscendingTimeStamp:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.timeStamp.QuadPart<t2->trace.timeStamp.QuadPart;}};
class CSortPredicateDescendingTimeStamp:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.timeStamp.QuadPart>t2->trace.timeStamp.QuadPart;}};
class CSortPredicateAscendingTime:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.timeStamp.QuadPart<t2->trace.timeStamp.QuadPart;}};
class CSortPredicateDescendingTime:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.timeStamp.QuadPart>t2->trace.timeStamp.QuadPart;}};
class CSortPredicateAscendingIndex:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.dwSequenceIndex<t2->trace.dwSequenceIndex;}};
class CSortPredicateDescendingIndex:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.dwSequenceIndex>t2->trace.dwSequenceIndex;}};
class CSortPredicateAscendingText:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sText<t2->trace.sText;}};
class CSortPredicateDescendingText:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sText>t2->trace.sText;}};
class CSortPredicateAscendingMethod:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sFunction<t2->trace.sFunction;}};
class CSortPredicateDescendingMethod:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sFunction>t2->trace.sFunction;}};
class CSortPredicateAscendingComponent:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sComponent<t2->trace.sComponent;}};
class CSortPredicateDescendingComponent:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sComponent>t2->trace.sComponent;}};
class CSortPredicateAscendingLevel:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sLevel<t2->trace.sLevel;}};
class CSortPredicateDescendingLevel:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sLevel>t2->trace.sLevel;}};
class CSortPredicateAscendingFlag:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sFlag<t2->trace.sFlag;}};
class CSortPredicateDescendingFlag:public CSortPredicate{public:bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const{return t1->trace.sFlag>t2->trace.sFlag;}};
class CSortPredicateAscendingSource:public CSortPredicate
{
    public:
        bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const
        {
            int bFileComp=_tcscmp(t1->trace.sSource.c_str(),t2->trace.sSource.c_str());
            if(bFileComp<0){return true;}
            if(bFileComp>0){return false;}
            return t1->trace.dwLine<t2->trace.dwLine;
        }
};
class CSortPredicateDescendingSource:public CSortPredicate
{
public:
    bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const
    {
        int bFileComp=_tcscmp(t1->trace.sSource.c_str(),t2->trace.sSource.c_str());
        if(bFileComp>0){return true;}
        if(bFileComp<0){return false;}
        return t1->trace.dwLine>t2->trace.dwLine;
    }
};
class CSortPredicateAscendingPIDTID:public CSortPredicate
{
public:
    bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const
    {
        if(t1->trace.dwProcessId<t2->trace.dwProcessId){return true;}
        if(t1->trace.dwProcessId>t2->trace.dwProcessId){return false;}
        return t1->trace.dwThreadId<t2->trace.dwThreadId;
    }
};
class CSortPredicateDescendingPIDTID:public CSortPredicate
{
public:
    bool operator()(const SETViewerTrace *t1,const SETViewerTrace *t2) const
    {
        if(t1->trace.dwProcessId>t2->trace.dwProcessId){return true;}
        if(t1->trace.dwProcessId<t2->trace.dwProcessId){return false;}
        return t1->trace.dwThreadId>t2->trace.dwThreadId;
    }
};

void CETViewerView::SortItems(CColumnInfo *pColumn)
{
    WaitForSingleObject(m_hTracesMutex,INFINITE);

    CSortPredicate *pPredicate=NULL;

    if(pColumn->id==eETViewerColumn_Index)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingIndex());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingIndex());
        }	
    }
    else if(pColumn->id==eETViewerColumn_TimeStamp)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingTimeStamp());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingTimeStamp());
        }
    }
    else if(pColumn->id==eETViewerColumn_PIDTID)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingPIDTID());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingPIDTID());
        }
    }
    else if(pColumn->id==eETViewerColumn_Source)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingSource());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingSource());
        }
    }
    else if(pColumn->id==eETViewerColumn_Text)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingText());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingText());
        }	
    }
    else if(pColumn->id==eETViewerColumn_Method)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingMethod());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingMethod());
        }	
    }
    else if(pColumn->id==eETViewerColumn_Component)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingComponent());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingComponent());
        }	
    }
    else if(pColumn->id==eETViewerColumn_Level)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingLevel());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingLevel());
        }	
    }
    else if(pColumn->id==eETViewerColumn_Flag)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingFlag());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingFlag());
        }	
    }
    else if(pColumn->id==eETViewerColumn_Time)
    {
        if(m_SortDirection==eETViewerSortDirection_Ascending)
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateAscendingTime());
        }
        else
        {
            stable_sort(m_lTraces.begin(),m_lTraces.end(),CSortPredicateDescendingTime());
        }	
    }
    ReleaseMutex(m_hTracesMutex);
}

void CETViewerView::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    CColumnInfo *pColumn=m_mVisibleColumns[pNMLV->iSubItem];
    if(m_SortColumn==pColumn->id)
    {
        m_SortDirection=m_SortDirection?eETViewerSortDirection_Ascending:eETViewerSortDirection_Descending;
    }
    else
    {
        m_SortDirection=eETViewerSortDirection_Ascending;
    }
    m_SortColumn=pColumn->id;
    SortItems(pColumn);
    ResetShowLastTrace();
    GetListCtrl().RedrawItems(0,GetListCtrl().GetItemCount()-1);
    *pResult = 0;
}

bool CETViewerView::Load(CConfigFile *pFile)
{
    bool bOk=LoadFrom(pFile,_T("TracePanel"));
    return bOk;
}
bool CETViewerView::Save(CConfigFile *pFile)
{
    unsigned x=0;
    for(x=0;x<m_ColumnInfo.size();x++)
    {
        CColumnInfo *pColumn=&m_ColumnInfo[x];
        if(pColumn->visible)
        {
            pColumn->width=GetListCtrl().GetColumnWidth(pColumn->iSubItem);
        }
    }
    return SaveTo(pFile,_T("TracePanel"));
}

void CETViewerView::OnAddProvider(CTraceProvider *pProvider){}
void CETViewerView::OnReplaceProvider(CTraceProvider *pOldProvider,CTraceProvider *pNewProvider){}
void CETViewerView::OnRemoveProvider(CTraceProvider *pProvider){}
void CETViewerView::OnProvidersModified()
{
    WaitForSingleObject(m_hTracesMutex,INFINITE);
    if(m_nUnformattedTraces)
    {		
        for(unsigned x=0;x<m_lTraces.size();x++)
        {
            SETViewerTrace *pTrace=m_lTraces[x];
            if(!pTrace->trace.bFormatted)
            {
                if(theApp.m_Controller.FormatTrace(&pTrace->trace))
                {
                    m_nUnformattedTraces--;
                }
            }
        }
        GetListCtrl().DeleteAllItems();
    }
    ReleaseMutex(m_hTracesMutex);
}

void CETViewerView::OnSessionTypeChanged()
{
    if(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_None)
    {
        Clear();
    }
}

void CETViewerView::SetTraceFont(std::tstring sTraceFont, DWORD dwFontSize)
{
    m_sTraceFont=sTraceFont;
    m_dwTraceFontSize=dwFontSize;
    UpdateFont();
}

void CETViewerView::GetTraceFont(std::tstring *psTraceFont, DWORD *pdwFontSize)
{
    *psTraceFont=m_sTraceFont;
    *pdwFontSize=m_dwTraceFontSize;
}
