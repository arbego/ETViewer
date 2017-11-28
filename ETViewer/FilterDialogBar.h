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

class CFilterDialogBar : public CDialogBar
{
    DECLARE_DYNAMIC(CFilterDialogBar)


public:
    CFilterDialogBar();
    virtual ~CFilterDialogBar();
    
    COLORREF	m_FilterChangedColor;
    HBRUSH		m_hFilterChangedBrush;

    CComboBox m_CBIncludeFilter;
    CComboBox m_CBExcludeFilter;

    CEdit	m_EDIncludeEdit;
    CEdit	m_EDExcludeEdit;

    WNDPROC		m_OldIncludeEditProc;
    WNDPROC		m_OldExcludeEditProc;

    void OnChangedInstantFilters();

    static LRESULT CALLBACK InstantEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    void InitDialogBar();
    void OnOk();
    void OnCancel();
    void OnSessionTypeChanged();

    //{{AFX_DATA(CFilterDialogBar)
    enum { IDD = IDD_FILTER_DIALOG_BAR };
    //}}AFX_DATA
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
    afx_msg void OnCbnSelchangeCbIncludeFilter();
    afx_msg void OnCbnSelchangeCbExcludeFilter();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};


