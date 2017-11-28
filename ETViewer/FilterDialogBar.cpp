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
#include "FilterDialogBar.h"
#include ".\filterdialogbar.h"

#define MAX_INSTANT_FILTERS		20

// CFilterDialogBar

IMPLEMENT_DYNAMIC(CFilterDialogBar, CDialogBar)
CFilterDialogBar::CFilterDialogBar()
{
    m_FilterChangedColor=RGB(255,255,0);
    m_hFilterChangedBrush=CreateSolidBrush(m_FilterChangedColor);
    m_OldIncludeEditProc=NULL;
    m_OldExcludeEditProc=NULL;
}

CFilterDialogBar::~CFilterDialogBar()
{
}


BEGIN_MESSAGE_MAP(CFilterDialogBar, CDialogBar)
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE(IDC_CB_INCLUDE_FILTER, OnCbnSelchangeCbIncludeFilter)
    ON_CBN_SELCHANGE(IDC_CB_EXCLUDE_FILTER, OnCbnSelchangeCbExcludeFilter)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CFilterDialogBar message handlers

void CFilterDialogBar::InitDialogBar() 
{
    unsigned x;

    m_CBIncludeFilter.Attach(::GetDlgItem(m_hWnd,IDC_CB_INCLUDE_FILTER));
    m_CBExcludeFilter.Attach(::GetDlgItem(m_hWnd,IDC_CB_EXCLUDE_FILTER));

    m_EDIncludeEdit.Attach(m_CBIncludeFilter.GetWindow(GW_CHILD)->m_hWnd);
    m_EDExcludeEdit.Attach(m_CBExcludeFilter.GetWindow(GW_CHILD)->m_hWnd);


    for(x=0;x<theApp.m_InstantIncludeFilterList.size();x++)
    {
        m_CBIncludeFilter.AddString(theApp.m_InstantIncludeFilterList[x].c_str());
    }
    for(x=0;x<theApp.m_InstantExcludeFilterList.size();x++)
    {
        m_CBExcludeFilter.AddString(theApp.m_InstantExcludeFilterList[x].c_str());
    }

    m_OldIncludeEditProc=(WNDPROC)GetWindowLong(m_EDIncludeEdit.m_hWnd,GWL_WNDPROC);
    SetWindowLong(m_EDIncludeEdit.m_hWnd,GWL_USERDATA,(DWORD)this);
    SetWindowLong(m_EDIncludeEdit.m_hWnd,GWL_WNDPROC,(DWORD)InstantEditProc);

    m_OldExcludeEditProc=(WNDPROC)GetWindowLong(m_EDExcludeEdit.m_hWnd,GWL_WNDPROC);
    SetWindowLong(m_EDExcludeEdit.m_hWnd,GWL_USERDATA,(DWORD)this);
    SetWindowLong(m_EDExcludeEdit.m_hWnd,GWL_WNDPROC,(DWORD)InstantEditProc);

    m_EDIncludeEdit.SetWindowText(theApp.m_InstantIncludeFilter.c_str());
    m_EDExcludeEdit.SetWindowText(theApp.m_InstantExcludeFilter.c_str());
}

LRESULT CALLBACK CFilterDialogBar::InstantEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CFilterDialogBar *pThis=(CFilterDialogBar *)GetWindowLong(hwnd,GWL_USERDATA);
    if(uMsg==WM_MOUSEWHEEL){return 0L;}
    if(uMsg==WM_KEYDOWN || uMsg==WM_SYSKEYDOWN)
    {
        /*CEdit *pEdit=NULL;
        if(hwnd==pThis->m_EDExcludeEdit.m_hWnd){pEdit=&pThis->m_EDExcludeEdit;}
        if(hwnd==pThis->m_EDIncludeEdit.m_hWnd){pEdit=&pThis->m_EDIncludeEdit;}
        if(pEdit)
        {
            bool pushedLControl=(GetKeyState(VK_LCONTROL)>>15)?true:false;
            bool pushedRControl=(GetKeyState(VK_RCONTROL)>>15)?true:false;
            bool pushedControl=(pushedLControl||pushedRControl);
            if(wParam=='F' && (pushedControl))
            {
                int begin=0,end=0;
                TCHAR text[1024]={0};
                pEdit->GetSel(begin,end);
                if(begin!=end)
                {
                    pEdit->GetWindowText(text,1024);
                    if(strlen(text)!=0)
                    {
                        int len=min(end-begin,sizeof(pThis->m_LastTextToFind)-1);
                        strncpy((TCHAR*)pThis->m_LastTextToFind,text+begin,len);
                        pThis->m_LastTextToFind[len]=0;
                    }
                }
            }
        }
        pThis->ProcessSpecialKeyStroke(wParam);*/
    }

    if(hwnd==pThis->m_EDExcludeEdit.m_hWnd){return CallWindowProc(pThis->m_OldExcludeEditProc,hwnd,uMsg,wParam,lParam);}
    if(hwnd==pThis->m_EDIncludeEdit.m_hWnd){return CallWindowProc(pThis->m_OldIncludeEditProc,hwnd,uMsg,wParam,lParam);}
    return 0L;
}

void CFilterDialogBar::OnDestroy()
{
    TCHAR sTempText[1024]={0};

    theApp.m_InstantIncludeFilterList.clear();
    int x;
    for(x=0;x<m_CBIncludeFilter.GetCount();x++)
    {
        m_CBIncludeFilter.GetLBText(x,sTempText);
        theApp.m_InstantIncludeFilterList.push_back(sTempText);
    }
    theApp.m_InstantExcludeFilterList.clear();
    for(x=0;x<m_CBExcludeFilter.GetCount();x++)
    {
        m_CBExcludeFilter.GetLBText(x,sTempText);
        theApp.m_InstantExcludeFilterList.push_back(sTempText);
    }

    m_CBIncludeFilter.Detach();
    m_CBExcludeFilter.Detach();
    m_EDIncludeEdit.Detach();
    m_EDExcludeEdit.Detach();
    CDialogBar::OnDestroy();
}
void CFilterDialogBar::OnOk()
{
    bool		 bAddComboString=false;
    CComboBox   *pCombo=NULL;
    CWnd		*pEdit=NULL;
    TCHAR		newText[2048];

    if(GetFocus()==&m_CBIncludeFilter || GetFocus()==&m_EDIncludeEdit)
    {
        TCHAR sTemp[1024]={0};
        m_CBIncludeFilter.GetWindowText(sTemp,_countof(sTemp));
        theApp.m_InstantIncludeFilter=sTemp;
        _tcscpy_s(newText,theApp.m_InstantIncludeFilter.c_str());
        bAddComboString=true;
        pCombo=&m_CBIncludeFilter;
        pEdit=&m_EDIncludeEdit;

        theApp.UpdateInstantFilters();
    }
    if(GetFocus()==&m_CBExcludeFilter || GetFocus()==&m_EDExcludeEdit)
    {
        TCHAR sTemp[1024]={0};
        m_CBExcludeFilter.GetWindowText(sTemp,_countof(sTemp));
        theApp.m_InstantExcludeFilter=sTemp;
        _tcscpy_s(newText,theApp.m_InstantExcludeFilter.c_str());
        bAddComboString=true;
        pCombo=&m_CBExcludeFilter;
        pEdit=&m_EDExcludeEdit;

        theApp.UpdateInstantFilters();
    }
    if(bAddComboString)
    {
        if(_tcscmp(newText,_T(""))!=0)
        {
            TCHAR existingText[2048]={0};
            TCHAR tempText[2048]={0};

            _tcscpy_s(tempText,newText);
            _tcsupr_s(tempText);

            int x;
            for(x=0;x<pCombo->GetCount();x++)
            {
                pCombo->GetLBText(x,existingText);
                _tcsupr_s(existingText);
                if(_tcscmp(existingText,tempText)==0)
                {
                    pCombo->DeleteString(x);
                    break;
                }
            }
            pCombo->InsertString(0,newText);
            if(pCombo->GetCount()>MAX_INSTANT_FILTERS){pCombo->DeleteString(pCombo->GetCount()-1);}
        }
        pEdit->SetWindowText(newText);
        OnChangedInstantFilters();
    }
}

void CFilterDialogBar::OnCancel()
{
    if(GetFocus()==&m_CBIncludeFilter || GetFocus()==&m_EDIncludeEdit)
    {
        m_CBIncludeFilter.SetWindowText(theApp.m_InstantIncludeFilter.c_str());
        OnChangedInstantFilters();
    }

    if(GetFocus()==&m_CBExcludeFilter || GetFocus()==&m_EDExcludeEdit)
    {
        m_CBExcludeFilter.SetWindowText(theApp.m_InstantExcludeFilter.c_str());
        OnChangedInstantFilters();
    }
}

void CFilterDialogBar::OnCbnSelchangeCbIncludeFilter()
{
    TCHAR newFilter[2048];
    int index=m_CBIncludeFilter.GetCurSel();
    if(index!=-1)
    {
        m_CBIncludeFilter.GetLBText(index,newFilter);
        m_EDIncludeEdit.SetWindowText(newFilter);
        theApp.m_InstantIncludeFilter=newFilter;
        OnChangedInstantFilters();
        theApp.UpdateInstantFilters();
    }
}

void CFilterDialogBar::OnCbnSelchangeCbExcludeFilter()
{
    TCHAR newFilter[2048];
    int index=m_CBExcludeFilter.GetCurSel();
    if(index!=-1)
    {
        m_CBExcludeFilter.GetLBText(index,newFilter);
        m_EDExcludeEdit.SetWindowText(newFilter);
        theApp.m_InstantExcludeFilter=newFilter;
        OnChangedInstantFilters();
        theApp.UpdateInstantFilters();
    }
}

HBRUSH CFilterDialogBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogBar::OnCtlColor(pDC, pWnd, nCtlColor);
    if(pWnd==&m_CBIncludeFilter || pWnd==&m_EDIncludeEdit)
    {
        TCHAR sTemp[1024]={0};
        m_EDIncludeEdit.GetWindowText(sTemp,1024);
        if(_tcscmp(sTemp,theApp.m_InstantIncludeFilter.c_str())!=0)
        {
            hbr=m_hFilterChangedBrush;
            pDC->SetBkColor(m_FilterChangedColor);
        }
    }
    if(pWnd==&m_CBExcludeFilter || pWnd==&m_EDExcludeEdit)
    {
        TCHAR sTemp[1024]={0};
        m_EDExcludeEdit.GetWindowText(sTemp,1024);
        if(_tcscmp(sTemp,theApp.m_InstantExcludeFilter.c_str())!=0)
        {
            hbr=m_hFilterChangedBrush;
            pDC->SetBkColor(m_FilterChangedColor);
        }
    }
    return hbr;
}

void CFilterDialogBar::OnChangedInstantFilters()
{
    m_CBIncludeFilter.RedrawWindow();
    m_CBExcludeFilter.RedrawWindow();
}

void CFilterDialogBar::OnSessionTypeChanged()
{
    if(theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_RealTime || 
        theApp.m_Controller.GetSessionType()==eTraceControllerSessionType_CreateLog)
    {
        m_CBIncludeFilter.EnableWindow(TRUE);
        m_CBExcludeFilter.EnableWindow(TRUE);
    }
    else
    {
        m_CBIncludeFilter.EnableWindow(FALSE);
        m_CBExcludeFilter.EnableWindow(FALSE);
    }
}