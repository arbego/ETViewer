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
#include "HighLightPane.h"
#include ".\highlightpane.h"


// CHighLightPane

IMPLEMENT_DYNCREATE(CHighLightPane, CListView)

CHighLightPane::CHighLightPane()
{
    m_bDisableColumnResize=false;
    m_OldListViewProc=0;
    m_hImageList=ImageList_Create(32,16,ILC_COLOR4|ILC_MASK,0,10);
}

CHighLightPane::~CHighLightPane()
{
}

BEGIN_MESSAGE_MAP(CHighLightPane, CListView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_NOTIFY_REFLECT(NM_CLICK, OnFilterClicked)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnCustomDraw)
    ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndEdit)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
END_MESSAGE_MAP()


// CHighLightPane diagnostics

#ifdef _DEBUG
void CHighLightPane::AssertValid() const
{
    CListView::AssertValid();
}

void CHighLightPane::Dump(CDumpContext& dc) const
{
    CListView::Dump(dc);
}
#endif //_DEBUG


// CHighLightPane message handlers

int CHighLightPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CListView::OnCreate(lpCreateStruct) == -1)
        return -1;

    GetListCtrl().InsertColumn(1,_T("HighLight Filters"),LVCFMT_LEFT,400,0);
    GetListCtrl().SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_CHECKBOXES);

    m_OldListViewProc=(WNDPROC)GetWindowLong(GetListCtrl().m_hWnd,GWL_WNDPROC);
    SetWindowLong(GetListCtrl().m_hWnd,GWL_STYLE,GetListCtrl().GetStyle()|LVS_EDITLABELS);
    SetWindowLong(GetListCtrl().m_hWnd,GWL_USERDATA,(DWORD)this);
    SetWindowLong(GetListCtrl().m_hWnd,GWL_WNDPROC,(DWORD)ListViewProc);
    ListView_SetImageList(GetListCtrl().m_hWnd,m_hImageList,LVSIL_SMALL);

    CFont font;
    font.CreateStockObject(DEFAULT_GUI_FONT);
    SetFont(&font);
    LoadFilters();

    return 0;
}

void CHighLightPane::OnDown() 
{
    int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
    if(sel==-1 || sel>=GetListCtrl().GetItemCount()-1){return;}

    SwapItems(sel,sel+1);
}

void CHighLightPane::OnUp() 
{
    int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
    if(sel<1){return;}

    SwapItems(sel,sel-1);
}


void CHighLightPane::SwapItems(int index1,int index2) 
{
    int finallySelected=-1;
    int finallyUnselected=-1;
    int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
    if(sel==index1){finallySelected=index2;finallyUnselected=index1;}
    if(sel==index2){finallySelected=index1;finallyUnselected=index2;}

    TCHAR item1Text[1024]={0},item2Text[1024]={0};
    LVITEM item1={0},item2={0};
    item1.iItem=index1;
    item1.cchTextMax=1024;
    item1.pszText=item1Text;
    item1.mask=LVIF_TEXT|LVIF_PARAM;
    item2.iItem=index2;
    item2.pszText=item2Text;
    item2.cchTextMax=1024;
    item2.mask=LVIF_TEXT|LVIF_PARAM;

    GetListCtrl().GetItem(&item1);
    GetListCtrl().GetItem(&item2);

    BOOL check1=GetListCtrl().GetCheck(item1.iItem);
    BOOL check2=GetListCtrl().GetCheck(item2.iItem);

    item1.iItem=index2;
    item2.iItem=index1;

    GetListCtrl().SetItem(&item1);
    GetListCtrl().SetItem(&item2);
    GetListCtrl().SetCheck(item1.iItem,check1);
    GetListCtrl().SetCheck(item2.iItem,check2);
    if(finallySelected!=-1)		{GetListCtrl().SetItemState(finallySelected,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);}
    if(finallyUnselected!=-1)	{GetListCtrl().SetItemState(finallyUnselected,LVIS_SELECTED|LVIS_FOCUSED,0);}

    SaveFilters();
}

void CHighLightPane::OnDestroy() 
{
    int x;
    for(x=0;x<GetListCtrl().GetItemCount();x++)
    {
        CHightLightFilter *pFilter=(CHightLightFilter *)GetListCtrl().GetItemData(x);
        delete pFilter;
    }
    GetListCtrl().DeleteAllItems();

    CListView::OnDestroy();
}

LRESULT CALLBACK CHighLightPane::ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CHighLightPane *pThis=(CHighLightPane *)GetWindowLong(hwnd,GWL_USERDATA);
    if(uMsg==WM_USER+100)
    {
        pThis->SaveFilters();
    }
    if(uMsg==WM_USER+101)
    {
        RECT R={0};
        pThis->m_bDisableColumnResize=true;
        pThis->GetListCtrl().GetClientRect(&R);
        pThis->GetListCtrl().SetColumnWidth(0,R.right-R.left);
        pThis->m_bDisableColumnResize=false;
    }	
    if(uMsg==WM_KEYDOWN)
    {
        bool pushedLControl=(GetKeyState(VK_LCONTROL)>>15)?true:false;
        bool pushedRControl=(GetKeyState(VK_RCONTROL)>>15)?true:false;
        if(wParam==VK_SPACE)
        {
            int sel=pThis->GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
            pThis->GetListCtrl().EditLabel(sel);
            return 0;
        }
        if(wParam==VK_UP && (pushedLControl||pushedRControl))  {pThis->OnUp();return 0;}
        if(wParam==VK_DOWN && (pushedLControl||pushedRControl)){pThis->OnDown();return 0;}
        if(wParam==VK_INSERT){pThis->OnNew();return 0;}
        if(wParam==VK_DELETE)
        {
            pThis->OnRemove();
            return 0;
        }
    }

    return CallWindowProc(pThis->m_OldListViewProc,hwnd,uMsg,wParam,lParam);
}

void CHighLightPane::OnNew()
{
    CHightLightFilter *pFilter=new CHightLightFilter;
    pFilter->SetText(_T("<New Filter>"));
    int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
    int index=GetListCtrl().InsertItem(sel==-1?GetListCtrl().GetItemCount():sel,pFilter->GetText().c_str(),0);
    GetListCtrl().SetItemState(index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    GetListCtrl().SetItemData(index,(DWORD)pFilter);
    GetListCtrl().SetCheck(index,true);
    GetListCtrl().EditLabel(index);
    SaveFilters();
}

void CHighLightPane::OnRemove()
{
    int sel=GetListCtrl().GetNextItem(-1,LVNI_SELECTED);
    if(sel!=-1)
    {
        CHightLightFilter *pFilter=(CHightLightFilter *)GetListCtrl().GetItemData(sel);
        GetListCtrl().DeleteItem(sel);
        delete pFilter;
    }
    int count=GetListCtrl().GetItemCount();
    GetListCtrl().SetItemState(sel<count?sel:count-1,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    SaveFilters();
}

void CHighLightPane::LoadFilters()
{
    unsigned x;
    for(x=0;x<(unsigned)GetListCtrl().GetItemCount();x++)
    {
        CHightLightFilter *pFilter=(CHightLightFilter *)GetListCtrl().GetItemData(x);
        delete pFilter;
    }
    GetListCtrl().DeleteAllItems();

    for(x=0;x<theApp.m_HighLightFilters.size();x++)
    {
        CHightLightFilter *pFilter=new CHightLightFilter;
        *pFilter=theApp.m_HighLightFilters[x];
        int index=GetListCtrl().InsertItem(x,pFilter->GetText().c_str(),0);
        GetListCtrl().SetItemData(index,(DWORD)pFilter);
        GetListCtrl().SetCheck(index,pFilter->GetEnabled());
    }
}

void CHighLightPane::SaveFilters()
{
    theApp.m_HighLightFilters.clear();
    int x;
    for(x=0;x<GetListCtrl().GetItemCount();x++)
    {
        CHightLightFilter filter;
        filter.SetEnabled(GetListCtrl().GetCheck(x)?true:false);

        TCHAR sTempText[1024]={0};
        LVITEM item={0};
        item.iItem=x;
        item.mask=LVIF_IMAGE|LVIF_TEXT|LVIF_PARAM;
        item.pszText=sTempText;
        item.cchTextMax=_countof(sTempText);
        GetListCtrl().GetItem(&item);
        filter.SetText(sTempText);
        CHightLightFilter *pFilter=(CHightLightFilter *)item.lParam;
        filter.SetBkColor(pFilter->GetBkColor());
        filter.SetTextColor(pFilter->GetTextColor());
        theApp.m_HighLightFilters.push_back(filter);
    }

    theApp.UpdateHighLightFilters();
}

void CHighLightPane::OnEndEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = TRUE;
    PostMessage(WM_USER+100);
}

void CHighLightPane::OnFilterClicked(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NMITEMACTIVATE  *pActivate=(NMITEMACTIVATE *)pNMHDR;
    CPoint p=pActivate->ptAction;
    UINT flags=0;
    int index=GetListCtrl().HitTest(p,&flags);
    if(index!=-1 && flags==LVHT_ONITEMSTATEICON )
    {
        PostMessage(WM_USER+100,0,0);
    }
    if(index!=-1 && flags==LVHT_ONITEMICON)
    {
        CHightLightFilter *pFilter=((CHightLightFilter *)GetListCtrl().GetItemData(index));

        RECT R1={0},R2={0};
        GetItemColorRects(index,&R1,&R2);

        if(PtInRect(&R1,pActivate->ptAction))
        {
            CColorDialog colorDialog(pFilter->GetTextColor(),CC_FULLOPEN|CC_ANYCOLOR,this);
            if(colorDialog.DoModal()==IDOK)
            {
                pFilter->SetTextColor(colorDialog.GetColor());
                SaveFilters();
            }
        }
        if(PtInRect(&R2,pActivate->ptAction))
        {
            CColorDialog colorDialog(pFilter->GetBkColor(),CC_FULLOPEN|CC_ANYCOLOR,this);
            if(colorDialog.DoModal()==IDOK)
            {
                pFilter->SetBkColor(colorDialog.GetColor());
                SaveFilters();
            }
        }
        GetListCtrl().RedrawItems(index,index);
    }

    *pResult = 0;
}

void CHighLightPane::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NMITEMACTIVATE *pActivate=(NMITEMACTIVATE *)pNMHDR;
    CPoint p=pActivate->ptAction;
    UINT flags=0;
    int tempIndex=GetListCtrl().HitTest(p,&flags);
    if(tempIndex!=-1 && flags==LVHT_ONITEMSTATEICON)
    {
        PostMessage(WM_USER+100,0,0);
        return;
    }

    CHightLightFilter *pFilter=new CHightLightFilter;
    pFilter->SetText(_T("<New Filter>"));
    int sel=pActivate->iItem;
    if(sel!=-1){GetListCtrl().EditLabel(sel);return;}
    int index=GetListCtrl().InsertItem(sel==-1?GetListCtrl().GetItemCount():sel,pFilter->GetText().c_str(),0);
    GetListCtrl().SetItemState(index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    GetListCtrl().SetItemData(index,(DWORD)pFilter);
    GetListCtrl().SetCheck(index,true);
    GetListCtrl().EditLabel(index);
    *pResult = 0;
}

void CHighLightPane::GetItemColorRects(int index,RECT *pR1,RECT *pR2)
{
    RECT R={0,0,0,0},R1={0},R2={0};
    GetListCtrl().GetItemRect(index,&R,LVIR_ICON);
    R.right=R.left+32;
    R.bottom=R.top+16;
    pR1->left=R.left+1;
    pR1->top=R.top+1;
    pR1->right=pR1->left+14;
    pR1->bottom=pR1->top+14;

    pR2->left=pR1->right+2;
    pR2->top=R.top+1;
    pR2->right=pR2->left+14;
    pR2->bottom=pR2->top+14;
}

void CHighLightPane::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
    LPNMLVCUSTOMDRAW  pDraw = (LPNMLVCUSTOMDRAW)pNMHDR;

    switch(pDraw->nmcd.dwDrawStage) 
    {
    case CDDS_PREPAINT :
        *pResult=CDRF_NOTIFYITEMDRAW;
        return;
    case CDDS_ITEMPREPAINT:
        *pResult=CDRF_NOTIFYSUBITEMDRAW;
        return;
    }

    *pResult = 0;
    if(pDraw->nmcd.dwDrawStage&CDDS_PREPAINT)
    {
        *pResult=CDRF_NOTIFYITEMDRAW;
    }

    if(pDraw->nmcd.dwDrawStage&CDDS_ITEMPREPAINT)
    {	
        if(pDraw->nmcd.lItemlParam!=0)
        {
            CHightLightFilter *pFilter=((CHightLightFilter *)pDraw->nmcd.lItemlParam);
            pDraw->clrText=pFilter->GetTextColor();
            pDraw->clrTextBk=pFilter->GetBkColor();
            *pResult=CDRF_NOTIFYSUBITEMDRAW;
        }
    }

    if(pDraw->nmcd.dwDrawStage==(CDDS_ITEMPREPAINT | CDDS_SUBITEM))
    {
        if(pDraw->iSubItem==0)
        {
            CHightLightFilter *pFilter=((CHightLightFilter *)pDraw->nmcd.lItemlParam);
            if(pFilter)
            {
                RECT R={0,0,0,0},R1={0},R2={0};

                GetItemColorRects(pDraw->nmcd.dwItemSpec,&R1,&R2);
                GetListCtrl().GetItemRect((int)pDraw->nmcd.dwItemSpec,&R,LVIR_ICON);

                ::SetDCBrushColor(pDraw->nmcd.hdc,pFilter->GetTextColor());
                LOGBRUSH LB1,LB2;
                LB1.lbColor=pFilter->GetTextColor();
                LB1.lbHatch=0;
                LB1.lbStyle=BS_SOLID;
                LB2.lbColor=pFilter->GetBkColor();
                LB2.lbHatch=0;
                LB2.lbStyle=BS_SOLID;
                HBRUSH hb1=CreateBrushIndirect(&LB1);
                HBRUSH hb2=CreateBrushIndirect(&LB2);
                HGDIOBJ hOldBrush=SelectObject(pDraw->nmcd.hdc,hb1);
                Rectangle(pDraw->nmcd.hdc,R1.left,R1.top,R1.right,R1.bottom);
                SelectObject(pDraw->nmcd.hdc,hb2);
                Rectangle(pDraw->nmcd.hdc,R2.left,R2.top,R2.right,R2.bottom);
                SelectObject(pDraw->nmcd.hdc,hOldBrush);
                DeleteObject(hb1);
                DeleteObject(hb2);

                ExcludeClipRect(pDraw->nmcd.hdc,R.left,R.top,R.right,R.bottom);
            }
            *pResult = CDRF_DODEFAULT;
        }
    }
}

BOOL CHighLightPane::PreCreateWindow(CREATESTRUCT& cs)
{
    DWORD dwStyle=cs.style;
    cs.style|=LVS_REPORT|LVS_EDITLABELS|LVS_SINGLESEL;
    return CListView::PreCreateWindow(cs);
}

void CHighLightPane::OnSize(UINT nType, int cx, int cy)
{
    CListView::OnSize(nType, cx, cy);

    if(!m_bDisableColumnResize)
    {
        PostMessage(WM_USER+101);
    }
}

BOOL CHighLightPane::OnEraseBkgnd(CDC* pDC)
{
    BOOL bRet=CListView::OnEraseBkgnd(pDC);
    RECT R={0};
    GetClientRect(&R);
    TCHAR *pText=_T("Drag items here");
    DrawText(pDC->m_hDC,pText,_tcslen(pText),&R,DT_WORDBREAK|DT_CENTER|DT_VCENTER);
    return FALSE;
}

void CHighLightPane::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMITEMACTIVATE *pActivate=(NMITEMACTIVATE *)pNMHDR;

    HMENU hMenu=GetSubMenu(LoadMenu(AfxGetResourceHandle(),MAKEINTRESOURCE(IDM_HIGHLIGHT_FILTERS)),0);
    CMenu *pMenu=new CMenu;
    pMenu->Attach(hMenu);
    POINT p={0};
    ::GetCursorPos(&p);
    unsigned res=::TrackPopupMenu(pMenu->m_hMenu,TPM_LEFTALIGN|TPM_RETURNCMD,p.x,p.y,0,m_hWnd,NULL);

    if(res==ID_HIGHLIGHT_FILTERS_ADD)
    {
        OnNew();
    }
    else if(res==ID_HIGHLIGHT_FILTERS_UP)
    {
        OnUp();
    }
    else if(res==ID_HIGHLIGHT_FILTERS_DOWN)
    {
        OnDown();
    }
    else if(res==ID_HIGHLIGHT_FILTERS_DELETE)
    {
        OnRemove();
    }

    delete pMenu;
    *pResult = 0;
}
