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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CETViewerDoc

IMPLEMENT_DYNCREATE(CETViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CETViewerDoc, CDocument)
END_MESSAGE_MAP()


// Construcción o destrucción de CETViewerDoc

CETViewerDoc::CETViewerDoc()
{
    // TODO: agregar aquí el código de construcción único

}

CETViewerDoc::~CETViewerDoc()
{
}

BOOL CETViewerDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    // TODO: agregar aquí código de reinicio
    // (los documentos SDI volverán a utilizar este documento)

    return TRUE;
}



// Diagnósticos de CETViewerDoc

#ifdef _DEBUG
void CETViewerDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CETViewerDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG


// Comandos de CETViewerDoc
