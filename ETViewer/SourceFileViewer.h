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

#if !defined(AFX_SOURCEFILEVIEWER_H__D9C826CC_F6C0_46E5_8824_E5DE26B60136__INCLUDED_)
#define AFX_SOURCEFILEVIEWER_H__D9C826CC_F6C0_46E5_8824_E5DE26B60136__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSourceFileContainer;
#include "FindDialog.h"

class CSourceFileViewer : public CDialog, public CFindDialogClient
{
    DWORD				m_OldEditProc;
    HFONT				m_hFileFont;
    CHAR				*m_pFileBuffer;
    CHAR				*m_pFileBufferUpper;
    TCHAR               *m_pFileBufferWide;
    DWORD				m_FileBufferLength;

    TCHAR m_SourceFile[MAX_PATH];
    int  m_SourceLine;
    CSourceFileContainer	*m_pContainer;

// Construction
public:
    CSourceFileViewer(CSourceFileContainer* pParent = NULL);   // standard constructor

    void ShowLine(int line);
    std::tstring GetFile();

// Dialog Data
    //{{AFX_DATA(CSourceFileViewer)
    enum { IDD = IDD_SOURCE_FILE_VIEWER };
    CEdit	m_EDLine;
    CEdit	m_EDFullPath;
    CRichEditCtrl	m_EDFile;
    //}}AFX_DATA

    DWORD OpenFile(const TCHAR *pFile,int line,bool bShowErrorIfFailed=true);
    void Reload();
    bool FindNext(const TCHAR *pText);
    void Copy();
    void ShowFindDialog();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSourceFileViewer)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL


    bool FindAndDeleteAll(const TCHAR *pText);
    bool FindAndMarkAll(const TCHAR *pText);
    void SetFocusOnOwnerWindow();

    static LRESULT CALLBACK FileEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    void SetMetrics();
    void UpdateLine();
    void OnFind();

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSourceFileViewer)
    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnUpdateSelectedLine();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCEFILEVIEWER_H__D9C826CC_F6C0_46E5_8824_E5DE26B60136__INCLUDED_)
