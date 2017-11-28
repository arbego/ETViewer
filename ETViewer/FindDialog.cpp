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
#include "FindDialog.h"

#define MAX_FIND_TEXTS 20

std::tstring	CFindDialogClient::m_LastTextToFind;

CFindDialogClient::CFindDialogClient()
{
    m_pFindDialog=NULL;
    m_hFindOwner=NULL;
    m_bFindDirectionUp=false;
    m_bMatchCaseInFind=false;
    m_bHideTracingOptions=true;
    m_bHideDeleteButtons=false;
    m_bHideMarkButtons=false;
    m_bFindInPIDName=false;
    m_bFindInTraceText=true;
}

void CFindDialogClient::BeginFind(CWnd *pParent,HWND owner,const TCHAR *pTextToFind)
{
    if(!m_pFindDialog)
    {
        m_hFindOwner=owner;
        m_pFindDialog=new CFindDialog(this);
        m_pFindDialog->Create(TRUE,_T(""),NULL,m_bFindDirectionUp?0:FR_DOWN,pParent);

    }
    m_pFindDialog->ShowWindow(SW_SHOW);
    m_pFindDialog->SetText(pTextToFind);
}

// CFindDialog dialog

IMPLEMENT_DYNAMIC(CFindDialog, CFindReplaceDialog)

CFindDialog::CFindDialog(CFindDialogClient *pFindClient)
{
    m_pFindClient=pFindClient;
    m_fr.hwndOwner=m_pFindClient->m_hFindOwner;
    m_fr.Flags|=FR_HIDEWHOLEWORD|FR_ENABLETEMPLATE;
    m_fr.Flags|=m_pFindClient->m_bMatchCaseInFind?FR_MATCHCASE:0;
    m_fr.lpstrFindWhat=const_cast<TCHAR*>(m_pFindClient->m_LastTextToFind.c_str());
    m_fr.hInstance=AfxGetResourceHandle();
    m_fr.lpTemplateName=MAKEINTRESOURCE(IDD_FIND_DIALOG);
}

CFindDialog::~CFindDialog()
{
}

void CFindDialog::DoDataExchange(CDataExchange* pDX)
{
    CFindReplaceDialog::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CFindDialog)
    DDX_Control(pDX, 1056,						m_BTUp);
    DDX_Control(pDX, 1057,						m_BTDown);
    DDX_Control(pDX, IDOK,						m_BTFind);
    DDX_Control(pDX, 1041,						m_CBMatchCase);
    DDX_Control(pDX, IDC_BT_DELETE_ALL,			m_BTDeleteAll);
    DDX_Control(pDX, IDC_BT_MARK_ALL,			m_BTMarkAll);
    DDX_Control(pDX, IDC_CB_FIND_PID_NAME,		m_CBFindInPIDName);
    DDX_Control(pDX, IDC_CB_FIND_IN_TRACE_TEXT,	m_CBFindInTraceText);
    DDX_Control(pDX, IDC_CO_TEXT_TO_FIND,		m_COTextToFind);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindDialog, CFindReplaceDialog)
    //{{AFX_MSG_MAP(CFindDialog)
    ON_CBN_EDITCHANGE(IDC_CO_TEXT_TO_FIND, OnChangedText)
    ON_CBN_SELCHANGE(IDC_CO_TEXT_TO_FIND, OnTextSelected)
    ON_BN_CLICKED(IDOK, OnFind)
    ON_BN_CLICKED(IDCANCEL, OnCancel)
    ON_BN_CLICKED(IDC_BT_DELETE_ALL, OnDeleteAll)
    ON_BN_CLICKED(IDC_BT_MARK_ALL, OnMarkAll)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CFindDialog message handlers


void CFindDialog::OnDeleteAll() 
{
    if(UpdateOptions())
    {
        if(m_pFindClient->FindAndDeleteAll(m_pFindClient->m_LastTextToFind.c_str())){m_pFindClient->SetFocusOnOwnerWindow();EndDialog(IDOK);}
    }
}

void CFindDialog::OnMarkAll() 
{
    if(UpdateOptions())
    {
        if(m_pFindClient->FindAndMarkAll(m_pFindClient->m_LastTextToFind.c_str())){m_pFindClient->SetFocusOnOwnerWindow();EndDialog(IDOK);}
    }
}

BOOL CFindDialog::OnInitDialog() 
{
    CFindReplaceDialog::OnInitDialog();

    unsigned x;
    m_CBFindInPIDName.SetCheck(m_pFindClient->m_bFindInPIDName?BST_CHECKED:BST_UNCHECKED);
    m_CBFindInPIDName.ShowWindow(m_pFindClient->m_bHideTracingOptions?SW_HIDE:SW_SHOW);
    m_CBFindInTraceText.SetCheck(m_pFindClient->m_bFindInTraceText?BST_CHECKED:BST_UNCHECKED);
    m_CBFindInTraceText.ShowWindow(m_pFindClient->m_bHideTracingOptions?SW_HIDE:SW_SHOW);

    m_BTDeleteAll.ShowWindow(m_pFindClient->m_bHideDeleteButtons?SW_HIDE:SW_SHOW);
    m_BTMarkAll.ShowWindow(m_pFindClient->m_bHideMarkButtons?SW_HIDE:SW_SHOW);

    m_EDTextToFind.Attach(m_COTextToFind.GetWindow(GW_CHILD)->m_hWnd);
    for(x=0;x<m_TextList.size();x++){m_COTextToFind.AddString(m_TextList[x].c_str());}

    m_EDTextToFind.SetWindowText(m_pFindClient->m_LastTextToFind.c_str());
    OnChangedText();

    m_EDTextToFind.SetFocus();
    return FALSE;
}

void CFindDialog::OnFind() 
{
    bool		 bAddComboString=false;
    CComboBox   *pCombo=NULL;
    CWnd		*pEdit=NULL;
    TCHAR		newText[2048];

    m_COTextToFind.GetWindowText(newText,_countof(newText));
    bAddComboString=true;
    pCombo=&m_COTextToFind;
    pEdit=&m_EDTextToFind;

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
            if(pCombo->GetCount()>MAX_FIND_TEXTS){pCombo->DeleteString(pCombo->GetCount()-1);}
        }
        pEdit->SetWindowText(newText);

    }

    Save();

    if(UpdateOptions())
    {
        if(m_pFindClient->FindNext(newText))
        {
            m_pFindClient->SetFocusOnOwnerWindow();
            EndDialog(IDOK);
        }
    }
}

bool CFindDialog::UpdateOptions() 
{
    bool res=true;

    TCHAR sTemp[1024]={0};
    m_EDTextToFind.GetWindowText(sTemp,1024);
    m_pFindClient->m_LastTextToFind=sTemp;
    m_pFindClient->m_bFindDirectionUp=(m_BTUp.GetCheck()==BST_CHECKED);
    m_pFindClient->m_bMatchCaseInFind=(m_CBMatchCase.GetCheck()==BST_CHECKED);
    m_pFindClient->m_bFindInPIDName=(m_CBFindInPIDName.GetCheck()==BST_CHECKED);
    m_pFindClient->m_bFindInTraceText=(m_CBFindInTraceText.GetCheck()==BST_CHECKED);

    if(!m_pFindClient->m_bFindInPIDName && !m_pFindClient->m_bFindInTraceText)
    {
        MessageBox(_T("At least one search option must be selected\r\n\r\n\"Find in Process Id / Name\" \r\n\"Find in Trace Text\""),_T("ETViewer"),MB_ICONSTOP|MB_OK);
        res=false;
    }
    return res;
}

void CFindDialog::Save() 
{
    m_TextList.clear();
    int x;
    for(x=0;x<m_COTextToFind.GetCount();x++)
    {
        TCHAR sTemp[1024]={0};
        m_COTextToFind.GetLBText(x,sTemp);
        m_TextList.push_back(sTemp);
    }
}

void CFindDialog::OnDestroy() 
{
    m_EDTextToFind.Detach();
    m_pFindClient->m_pFindDialog=NULL;
    CFindReplaceDialog::OnDestroy();
    m_pFindClient->SetFocusOnOwnerWindow();
}

void CFindDialog::OnCancel()
{
    Save();
    CFindReplaceDialog::EndDialog(IDCANCEL);
}

void CFindDialog::OnChangedText() 
{
    if(m_EDTextToFind.m_hWnd==NULL){return;}

    TCHAR A[200];
    m_EDTextToFind.GetWindowText(A,200);
    bool anyText=_tcscmp(A,_T(""))!=0;

    m_BTDeleteAll.EnableWindow(anyText);
    m_BTMarkAll.EnableWindow(anyText);
    m_BTFind.EnableWindow(anyText);
}

void CFindDialog::OnTextSelected()
{
    TCHAR newText[2048];
    int index=m_COTextToFind.GetCurSel();
    if(index!=-1)
    {
        m_COTextToFind.GetLBText(index,newText);
        m_EDTextToFind.SetWindowText(newText);
    }
    OnChangedText();
}

void CFindDialog::SetText(const TCHAR *pTextToFind)
{
    m_EDTextToFind.SetWindowText(pTextToFind);
    OnChangedText();
    m_EDTextToFind.SetSel(0,m_EDTextToFind.GetWindowTextLength());
    m_COTextToFind.SetFocus();
}