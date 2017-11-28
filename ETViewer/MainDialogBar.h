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

#pragma once
#include "resource.h"

class CMainDialogBar : public CDialogBar
{
    DECLARE_DYNAMIC(CMainDialogBar)

public:
    CMainDialogBar();
    virtual ~CMainDialogBar();

    //{{AFX_DATA(CMainDialogBar)
    enum { IDD = IDD_MAIN_DIALOG_BAR };
    //}}AFX_DATA

    CButton	m_BTStartStop;
    CButton	m_BTOpenFile;
    CButton	m_BTShowSourceContainer;
    CButton	m_BTEnsureVisible;
    CButton	m_BTSave;
    CButton	m_BTHighlightFilters;
    CButton	m_BTFind;
    CButton	m_BTClear;
    CButton	m_BTClearSelected;
    CEdit	m_EDErrorLookup;
    CButton	m_BTErrorLookup;

    void InitDialogBar();
    void UpdateBitmaps();
    void OnSessionTypeChanged();

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
};


