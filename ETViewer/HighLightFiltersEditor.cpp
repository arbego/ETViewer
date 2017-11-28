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
#include "HighLightFiltersEditor.h"

#define MARGIN 4

/////////////////////////////////////////////////////////////////////////////
// CHighLightFiltersEditor dialog


CHighLightFiltersEditor::CHighLightFiltersEditor(CWnd* pParent /*=NULL*/)
    : CDialog(CHighLightFiltersEditor::IDD, pParent)
{
    //{{AFX_DATA_INIT(CHighLightFiltersEditor)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_OldListViewProc=0;
    m_hImageList=ImageList_Create(32,16,ILC_COLOR4|ILC_MASK,0,10);
}


void CHighLightFiltersEditor::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CHighLightFiltersEditor)
    DDX_Control(pDX, IDCANCEL, m_BTCancel);
    DDX_Control(pDX, IDOK, m_BTOk);
    DDX_Control(pDX, IDC_BT_UP, m_BTUp);
    DDX_Control(pDX, IDC_BT_DOWN, m_BTDown);
    DDX_Control(pDX, IDC_LW_FILTERS, m_LWFilters);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHighLightFiltersEditor, CDialog)
    //{{AFX_MSG_MAP(CHighLightFiltersEditor)
    ON_BN_CLICKED(IDC_BT_DOWN, OnDown)
    ON_BN_CLICKED(IDC_BT_UP, OnUp)
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LW_FILTERS, OnEndLabelEdit)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LW_FILTERS, OnCustomDraw)
    ON_NOTIFY(NM_CLICK, IDC_LW_FILTERS, OnFilterClicked)
    ON_NOTIFY(NM_DBLCLK, IDC_LW_FILTERS, OnDoubleClick)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHighLightFiltersEditor message handlers

void CHighLightFiltersEditor::OnCancel() 
{
    CDialog::OnCancel();
}

void CHighLightFiltersEditor::OnDown() 
{
    int sel=m_LWFilters.GetNextItem(-1,LVNI_SELECTED);
    if(sel==-1 || sel>=m_LWFilters.GetItemCount()-1){return;}

    SwapItems(sel,sel+1);
}

void CHighLightFiltersEditor::OnUp() 
{
    int sel=m_LWFilters.GetNextItem(-1,LVNI_SELECTED);
    if(sel<1){return;}

    SwapItems(sel,sel-1);
}

void CHighLightFiltersEditor::SwapItems(int index1,int index2) 
{

    int finallySelected=-1;
    int finallyUnselected=-1;
    int sel=m_LWFilters.GetNextItem(-1,LVNI_SELECTED);
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

    m_LWFilters.GetItem(&item1);
    m_LWFilters.GetItem(&item2);

    BOOL check1=m_LWFilters.GetCheck(item1.iItem);
    BOOL check2=m_LWFilters.GetCheck(item2.iItem);

    item1.iItem=index2;
    item2.iItem=index1;

    m_LWFilters.SetItem(&item1);
    m_LWFilters.SetItem(&item2);
    m_LWFilters.SetCheck(item1.iItem,check1);
    m_LWFilters.SetCheck(item2.iItem,check2);
    if(finallySelected!=-1)		{m_LWFilters.SetItemState(finallySelected,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);}
    if(finallyUnselected!=-1)	{m_LWFilters.SetItemState(finallyUnselected,LVIS_SELECTED|LVIS_FOCUSED,0);}

}

void CHighLightFiltersEditor::OnOK() 
{
    SaveFilters();
    CDialog::OnOK();
}

void CHighLightFiltersEditor::OnSize(UINT nType, int cx, int cy) 
{
    CDialog::OnSize(nType, cx, cy);
    if(m_LWFilters.m_hWnd){SetMetrics();}
}

void CHighLightFiltersEditor::OnDestroy() 
{
    int x;
    for(x=0;x<m_LWFilters.GetItemCount();x++)
    {
        CHightLightFilter *pFilter=(CHightLightFilter *)m_LWFilters.GetItemData(x);
        delete pFilter;
    }
    m_LWFilters.DeleteAllItems();

    CDialog::OnDestroy();
}

LRESULT CALLBACK CHighLightFiltersEditor::ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CHighLightFiltersEditor *pThis=(CHighLightFiltersEditor *)GetWindowLong(hwnd,GWL_USERDATA);
    if(uMsg==WM_KEYDOWN)
    {
        bool pushedLControl=(GetKeyState(VK_LCONTROL)>>15)?true:false;
        bool pushedRControl=(GetKeyState(VK_RCONTROL)>>15)?true:false;
        if(wParam==VK_SPACE)
        {
            int sel=pThis->m_LWFilters.GetNextItem(-1,LVNI_SELECTED);
            pThis->m_LWFilters.EditLabel(sel);
            return 0;
        }
        if(wParam==VK_UP && (pushedLControl||pushedRControl))  {pThis->OnUp();return 0;}
        if(wParam==VK_DOWN && (pushedLControl||pushedRControl)){pThis->OnDown();return 0;}
        if(wParam==VK_INSERT)
        {
            CHightLightFilter *pFilter=new CHightLightFilter;
            pFilter->SetText(_T("<New Filter>"));
            int sel=pThis->m_LWFilters.GetNextItem(-1,LVNI_SELECTED);
            int index=pThis->m_LWFilters.InsertItem(sel==-1?pThis->m_LWFilters.GetItemCount():sel,pFilter->GetText().c_str(),0);
            pThis->m_LWFilters.SetItemState(index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
            pThis->m_LWFilters.SetItemData(index,(DWORD)pFilter);
            pThis->m_LWFilters.SetCheck(index,true);
            pThis->m_LWFilters.EditLabel(index);
            return 0;
        }
        if(wParam==VK_DELETE)
        {
            int sel=pThis->m_LWFilters.GetNextItem(-1,LVNI_SELECTED);
            if(sel!=-1)
            {
                CHightLightFilter *pFilter=(CHightLightFilter *)pThis->m_LWFilters.GetItemData(sel);
                pThis->m_LWFilters.DeleteItem(sel);
                delete pFilter;
            }
            int count=pThis->m_LWFilters.GetItemCount();
            pThis->m_LWFilters.SetItemState(sel<count?sel:count-1,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
            return 0;
        }
    }

    return CallWindowProc(pThis->m_OldListViewProc,hwnd,uMsg,wParam,lParam);
}

BOOL CHighLightFiltersEditor::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    m_BTUp	.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_UP)));
    m_BTDown.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_DOWN)));

    m_LWFilters.InsertColumn(1,_T("Filter"),LVCFMT_LEFT,400,0);
    m_LWFilters.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_CHECKBOXES);

    m_OldListViewProc=(WNDPROC)GetWindowLong(m_LWFilters.m_hWnd,GWL_WNDPROC);
    SetWindowLong(m_LWFilters.m_hWnd,GWL_STYLE,m_LWFilters.GetStyle()|LVS_EDITLABELS);
    SetWindowLong(m_LWFilters.m_hWnd,GWL_USERDATA,(DWORD)this);
    SetWindowLong(m_LWFilters.m_hWnd,GWL_WNDPROC,(DWORD)ListViewProc);
    ListView_SetImageList(m_LWFilters.m_hWnd,m_hImageList,LVSIL_SMALL);
    
    SetMetrics();
    LoadFilters();

    SetIcon((HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_HIGHLIGHT_FILTERS),IMAGE_ICON,16,16,0),FALSE);
    SetIcon((HICON)LoadImage(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_HIGHLIGHT_FILTERS),IMAGE_ICON,32,32,0),TRUE);

    m_LWFilters.SetFocus();
    return TRUE; 
}

void CHighLightFiltersEditor::SetMetrics()
{
    RECT clientRect,listRect,upRect,downRect,okRect,cancelRect;
    GetClientRect(&clientRect);
    m_LWFilters.GetClientRect(&listRect);
    m_BTUp.GetClientRect(&upRect);
    m_BTDown.GetClientRect(&downRect);
    m_BTOk.GetClientRect(&okRect);
    m_BTCancel.GetClientRect(&cancelRect);

    SIZE listSize=GetRectSize(listRect);
    SIZE okSize=GetRectSize(okRect);
    SIZE cancelSize=GetRectSize(cancelRect);
    SIZE upSize=GetRectSize(upRect);
    SIZE downSize=GetRectSize(downRect);
    SIZE clientSize=GetRectSize(clientRect);
    POINT listPos,okPos,cancelPos,upPos,downPos;

    listPos.x=MARGIN;
    listPos.y=MARGIN;
    listSize.cx=clientSize.cx-(upSize.cx+MARGIN*3);
    listSize.cy=clientSize.cy-(okSize.cy+MARGIN*3);

    upPos.x=clientSize.cx-(MARGIN+upSize.cx);
    upPos.y=MARGIN;

    downPos.x=clientSize.cx-(MARGIN+downSize.cx);
    downPos.y=clientSize.cy-(MARGIN*2+downSize.cy+okSize.cy);
    
    okPos.x=MARGIN;
    okPos.y=clientSize.cy-(MARGIN+okSize.cy);

    cancelPos.x=clientSize.cx-(MARGIN+cancelSize.cx);
    cancelPos.y=clientSize.cy-(MARGIN+okSize.cy);

    m_LWFilters.SetWindowPos(NULL,listPos.x,listPos.y,listSize.cx,listSize.cy,SWP_NOZORDER);
    m_BTUp.SetWindowPos(NULL,upPos.x,upPos.y,upSize.cx,upSize.cy,SWP_NOZORDER);
    m_BTDown.SetWindowPos(NULL,downPos.x,downPos.y,downSize.cx,downSize.cy,SWP_NOZORDER);
    m_BTOk.SetWindowPos(NULL,okPos.x,okPos.y,okSize.cx,okSize.cy,SWP_NOZORDER);
    m_BTCancel.SetWindowPos(NULL,cancelPos.x,cancelPos.y,cancelSize.cx,cancelSize.cy,SWP_NOZORDER);
}

void CHighLightFiltersEditor::LoadFilters()
{
    m_LWFilters.DeleteAllItems();

    DWORD x;
    for(x=0;x<theApp.m_HighLightFilters.size();x++)
    {
        CHightLightFilter *pFilter=new CHightLightFilter;
        *pFilter=theApp.m_HighLightFilters[x];
        int index=m_LWFilters.InsertItem(x,pFilter->GetText().c_str(),0);
        m_LWFilters.SetItemData(index,(DWORD)pFilter);
        m_LWFilters.SetCheck(index,pFilter->GetEnabled());
    }
}

void CHighLightFiltersEditor::SaveFilters()
{
    theApp.m_HighLightFilters.clear();
    int x;
    for(x=0;x<m_LWFilters.GetItemCount();x++)
    {
        CHightLightFilter filter;
        filter.SetEnabled(m_LWFilters.GetCheck(x)?true:false);

        TCHAR sTempText[1024]={0};
        LVITEM item={0};
        item.iItem=x;
        item.mask=LVIF_IMAGE|LVIF_TEXT|LVIF_PARAM;
        item.pszText=sTempText;
        item.cchTextMax=_countof(sTempText);
        m_LWFilters.GetItem(&item);
        filter.SetText(sTempText);
        CHightLightFilter *pFilter=(CHightLightFilter *)item.lParam;
        filter.SetBkColor(pFilter->GetBkColor());
        filter.SetTextColor(pFilter->GetTextColor());
        theApp.m_HighLightFilters.push_back(filter);
    }

    theApp.UpdateHighLightFilters();
}

void CHighLightFiltersEditor::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult){*pResult = TRUE;}
void CHighLightFiltersEditor::OnFilterClicked(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NMITEMACTIVATE  *pActivate=(NMITEMACTIVATE *)pNMHDR;
    CPoint p=pActivate->ptAction;
    UINT flags=0;
    int index=m_LWFilters.HitTest(p,&flags);
    if(index!=-1 && flags==LVHT_ONITEMICON)
    {
        CHightLightFilter *pFilter=((CHightLightFilter *)m_LWFilters.GetItemData(index));

        RECT R1={0},R2={0};
        GetItemColorRects(index,&R1,&R2);

        if(PtInRect(&R1,pActivate->ptAction))
        {
            CColorDialog colorDialog(pFilter->GetTextColor(),CC_FULLOPEN|CC_ANYCOLOR,this);
            if(colorDialog.DoModal()==IDOK){pFilter->SetTextColor(colorDialog.GetColor());}
        }
        if(PtInRect(&R2,pActivate->ptAction))
        {
            CColorDialog colorDialog(pFilter->GetBkColor(),CC_FULLOPEN|CC_ANYCOLOR,this);
            if(colorDialog.DoModal()==IDOK){pFilter->SetBkColor(colorDialog.GetColor());}
        }
        m_LWFilters.RedrawItems(index,index);
    }

    *pResult = 0;
}

void CHighLightFiltersEditor::GetItemColorRects(int index,RECT *pR1,RECT *pR2)
{
    RECT R={0,0,0,0},R1={0},R2={0};
    m_LWFilters.GetItemRect(index,&R,LVIR_ICON);
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

void CHighLightFiltersEditor::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
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
                m_LWFilters.GetItemRect((int)pDraw->nmcd.dwItemSpec,&R,LVIR_ICON);

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
void CHighLightFiltersEditor::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NMITEMACTIVATE *pActivate=(NMITEMACTIVATE *)pNMHDR;
    CHightLightFilter *pFilter=new CHightLightFilter;
    pFilter->SetText(_T("<New Filter>"));
    int sel=pActivate->iItem;
    if(sel!=-1){m_LWFilters.EditLabel(sel);return;}
    int index=m_LWFilters.InsertItem(sel==-1?m_LWFilters.GetItemCount():sel,pFilter->GetText().c_str(),0);
    m_LWFilters.SetItemState(index,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
    m_LWFilters.SetItemData(index,(DWORD)pFilter);
    m_LWFilters.SetCheck(index,true);
    m_LWFilters.EditLabel(index);
    *pResult = 0;
}
