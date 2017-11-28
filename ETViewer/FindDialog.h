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
#include "afxwin.h"

class CFindDialog;
class CFindDialogClient
{
public:

    CFindDialogClient();

    HWND			m_hFindOwner;
    bool			m_bFindDirectionUp;
    bool			m_bMatchCaseInFind;
    bool			m_bHideTracingOptions;
    bool			m_bHideDeleteButtons;
    bool			m_bHideMarkButtons;
    bool			m_bFindInPIDName;
    bool			m_bFindInTraceText;
    static std::tstring	m_LastTextToFind;

    CFindDialog  *m_pFindDialog;

    virtual void BeginFind(CWnd *pParent,HWND owner,const TCHAR *pTextToFind=NULL);

    virtual bool FindAndDeleteAll(const TCHAR *pText)=0;
    virtual bool FindAndMarkAll(const TCHAR *pText)=0;
    virtual bool FindNext(const TCHAR *pText)=0;
    virtual void SetFocusOnOwnerWindow()=0;
};

// CFindDialog dialog

class CFindDialog : public CFindReplaceDialog
{
    DECLARE_DYNAMIC(CFindDialog)

    CFindDialogClient *m_pFindClient;

public:
    CFindDialog(CFindDialogClient *pFindClient);   // standard constructor
    virtual ~CFindDialog();

// Dialog Data
    //{{AFX_DATA(CFindDialog)
    enum { IDD = IDD_FIND_DIALOG };
    CEdit	m_EDTextToFind;
    CButton m_BTUp;
    CButton m_BTDown;
    CButton m_BTFind;
    CButton m_CBMatchCase;
    CButton m_BTDeleteAll;
    CButton m_BTMarkAll;
    CButton m_CBFindInPIDName;
    CButton m_CBFindInTraceText;
    CComboBox m_COTextToFind;
    //}}AFX_DATA

    bool UpdateOptions();

    std::deque<std::tstring> m_TextList;

    void Save();
    void SetText(const TCHAR *pTextToFind);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    //{{AFX_MSG(CFindDialog)
    afx_msg void OnDeleteAll();
    afx_msg void OnMarkAll();
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnChangedText();
    afx_msg void OnFind();
    afx_msg void OnCancel();
    afx_msg void OnTextSelected();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
};
