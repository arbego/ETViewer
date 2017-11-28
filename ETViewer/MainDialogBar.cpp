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
#include "MainDialogBar.h"
#include ".\maindialogbar.h"
#include ".\MainFrm.h"


// CMainDialogBar

IMPLEMENT_DYNAMIC(CMainDialogBar, CDialogBar)
CMainDialogBar::CMainDialogBar()
{
}

CMainDialogBar::~CMainDialogBar()
{
}


BEGIN_MESSAGE_MAP(CMainDialogBar, CDialogBar)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

void CMainDialogBar::InitDialogBar() 
{
    m_BTStartStop.Attach(::GetDlgItem(m_hWnd,IDC_BT_START_STOP_CAPTURE));
    m_BTOpenFile.Attach(::GetDlgItem(m_hWnd,IDC_BT_OPEN_FILE));
    m_BTShowSourceContainer.Attach(::GetDlgItem(m_hWnd,IDC_BT_SHOW_SOURCE_CONTAINER));
    m_BTEnsureVisible.Attach(::GetDlgItem(m_hWnd,IDC_BT_ENSURE_VISIBLE));
    m_BTSave.Attach(::GetDlgItem(m_hWnd,IDC_BT_SAVE));
    m_BTHighlightFilters.Attach(::GetDlgItem(m_hWnd,IDC_BT_HIGHLIGHT_FILTERS));
    m_BTFind.Attach(::GetDlgItem(m_hWnd,IDC_BT_FIND));
    m_BTClear.Attach(::GetDlgItem(m_hWnd,IDC_BT_CLEAR));
    m_BTClearSelected.Attach(::GetDlgItem(m_hWnd,IDC_BT_CLEAR_SELECTED));
    m_EDErrorLookup.Attach(::GetDlgItem(m_hWnd,IDC_ED_ERRORLOOKUP));
    m_BTErrorLookup.Attach(::GetDlgItem(m_hWnd,IDC_BT_ERRORLOOKUP));

    UpdateBitmaps();
}

void CMainDialogBar::OnDestroy()
{
    m_BTStartStop.Detach();
    m_BTOpenFile.Detach();
    m_BTShowSourceContainer.Detach();
    m_BTEnsureVisible.Detach();
    m_BTSave.Detach();
    m_BTHighlightFilters.Detach();
    m_BTFind.Detach();
    m_BTClear.Detach();
    m_BTClearSelected.Detach();
    m_EDErrorLookup.Detach();
    m_BTErrorLookup.Detach();

    CDialogBar::OnDestroy();
}

void CMainDialogBar::UpdateBitmaps()
{
    CMainFrame *pFrame=dynamic_cast<CMainFrame *>(GetParent()->GetParent());
    if(!pFrame){return;}

    m_BTStartStop		.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(theApp.m_Controller.IsPaused()?IDI_CAPTURE_STOP:IDI_CAPTURE_START)));
    m_BTOpenFile		.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_OPEN_FILE)));
    m_BTShowSourceContainer	.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_SOURCE_FILE)));
    m_BTEnsureVisible	.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(pFrame->GetTracePane()->IsShowLastTraceEnabled()?IDI_SCROLL_ENABLED:IDI_SCROLL_DISABLED)));
    m_BTClear			.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_CLEAR_EVENTS)));
    m_BTClearSelected	.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_CLEAR_SELECTED_EVENTS)));
    m_BTSave			.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_SAVE)));
    m_BTFind			.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_FIND)));
    m_BTHighlightFilters.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_HIGHLIGHT_FILTERS)));
    m_BTErrorLookup		.SetIcon(LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ERRORLOOKUP)));
}

void CMainDialogBar::OnSessionTypeChanged()
{
}