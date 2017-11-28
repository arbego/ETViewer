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
#include "SaveAllTracesQuestionDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveAllTracesQuestionDialog dialog


CSaveAllTracesQuestionDialog::CSaveAllTracesQuestionDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSaveAllTracesQuestionDialog::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSaveAllTracesQuestionDialog)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CSaveAllTracesQuestionDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSaveAllTracesQuestionDialog)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveAllTracesQuestionDialog, CDialog)
    //{{AFX_MSG_MAP(CSaveAllTracesQuestionDialog)
    ON_BN_CLICKED(IDC_BT_SELECTED, OnSelectedTraces)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveAllTracesQuestionDialog message handlers

void CSaveAllTracesQuestionDialog::OnSelectedTraces() 
{
    EndDialog(IDC_BT_SELECTED);
}
