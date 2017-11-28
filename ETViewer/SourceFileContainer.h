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

#if !defined(AFX_SOURCEFILECONTAINER_H__9B0F676E_E7A4_4EB7_9C2C_6632F6172D3A__INCLUDED_)
#define AFX_SOURCEFILECONTAINER_H__9B0F676E_E7A4_4EB7_9C2C_6632F6172D3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SourceFileViewer.h"

class CSourceFileContainer : public CDialog
{
// Construction
public:
    CSourceFileContainer(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CSourceFileContainer)
    enum { IDD = IDD_SOURCE_FILE_CONTAINER };
    CButton	m_BTRecentFiles;
    CButton	m_BTFind;
    CButton	m_BTCopy;
    CButton	m_BTOpenFile;
    CTabCtrl	m_TCSourceFiles;
    CButton		m_BTCloseFile;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSourceFileContainer)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    CSourceFileViewer *GetViewerAt(int index);

    public:

    void GetFiles(std::set<std::tstring> *psFiles);
    void ReloadFile(const TCHAR *sFile);

    bool ShowFile(const TCHAR *pFile,int line,bool bShowErrorIfFailed=true);
    void BrowseForAndShowFile();
    void ShowSelectedFile();
    void SetMetrics();

    void SelectNext();
    void SelectPrevious();

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSourceFileContainer)
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnCloseFile();
    afx_msg void OnSourceFileChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnOpenFile();
    afx_msg void OnCopy();
    afx_msg void OnFind();
    afx_msg void OnRecentFile();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCEFILECONTAINER_H__9B0F676E_E7A4_4EB7_9C2C_6632F6172D3A__INCLUDED_)
