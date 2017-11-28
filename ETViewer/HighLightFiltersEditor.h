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

#if !defined(AFX_VAHIGHLIGHTFILTERSEDITOR_H__9E949CE9_D21C_4D45_B4CC_6FAEA9BD8B01__INCLUDED_)
#define AFX_VAHIGHLIGHTFILTERSEDITOR_H__9E949CE9_D21C_4D45_B4CC_6FAEA9BD8B01__INCLUDED_

#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VAHighLightFiltersEditor.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHighLightFiltersEditor dialog

class CHighLightFiltersEditor : public CDialog
{
// Construction
public:
    CHighLightFiltersEditor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CHighLightFiltersEditor)
    enum { IDD = IDD_HIGHLIGHT_FILTERS_EDITOR };
    CButton	m_BTCancel;
    CButton	m_BTOk;
    CButton	m_BTDown;
    CButton	m_BTUp;
    CListCtrl	m_LWFilters;
    //}}AFX_DATA

    WNDPROC				m_OldListViewProc;
    HIMAGELIST			m_hImageList;

    static LRESULT CALLBACK ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    void SwapItems(int index1,int index2);
    void GetItemColorRects(int index,RECT *pR1,RECT *pR2);

    void LoadFilters();
    void SaveFilters();
    void SetMetrics();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHighLightFiltersEditor)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CHighLightFiltersEditor)
    virtual void OnCancel();
    afx_msg void OnDown();
    afx_msg void OnUp();
    virtual void OnOK();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
    afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnFilterClicked(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VAHIGHLIGHTFILTERSEDITOR_H__9E949CE9_D21C_4D45_B4CC_6FAEA9BD8B01__INCLUDED_)
