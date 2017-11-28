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
#include "afxcmn.h"
#include "afxwin.h"

class CSettingsDialog : public CDialog
{
    CFont  m_TraceFont;
    std::tstring m_sTraceFont;
    DWORD  m_dwTraceFontSize;
    WNDPROC	m_OldListViewProc;

    DECLARE_DYNAMIC(CSettingsDialog)

    void UpdateFont();

    void SwapItems(int index1,int index2);
    void AddSourcePath(int index);
    void OnInsertSourcePath();

    static LRESULT CALLBACK ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

public:
    CSettingsDialog(CWnd* pParent = NULL);   // standard constructor
    virtual ~CSettingsDialog();

// Dialog Data
    enum { IDD = IDD_SETTINGS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CListCtrl m_LWSourcePaths;
    CButton m_BTAdd;
    CButton m_BTRemove;
    CButton m_BTUp;
    CButton m_BTDown;
    CButton m_CBAssociatePDB;
    CButton m_CBAssociateETL;
    CButton m_CBAssociateSources;
    CStatic m_STFontSample;
    CButton m_BTSelectFont;
    CButton m_BTOk;
    CButton m_BTCancel;
    CButton m_RBPDBReloadAuto;
    CButton m_RBPDBReloadAsk;
    CButton m_RBPDBReloadDisabled;
    CButton m_RBSourceReloadAuto;
    CButton m_RBSourceReloadAsk;
    CButton m_RBSourceReloadDisabled;
    afx_msg void OnAddSourcePath();
    afx_msg void OnRemoveSourcePath();
    afx_msg void OnMoveSourceUp();
    afx_msg void OnMoveSourceDown();
    afx_msg void OnSelectFont();
    afx_msg void OnPDBReloadChanged();
    afx_msg void OnSourceReloadChanged();
    afx_msg void OnOk();
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
};
