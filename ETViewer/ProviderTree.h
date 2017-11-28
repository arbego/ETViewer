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

class CETViewerDoc;

class CProviderTree : public CTreeView
{
protected: // Crear sólo a partir de serialización
    CProviderTree();
    DECLARE_DYNCREATE(CProviderTree)

    HIMAGELIST	m_hImageList;
    HICON		m_hPlayIcon;
    HICON		m_hPlayBlockedIcon;
    HICON		m_hRecordIcon;
    HICON		m_hPauseIcon;
    HICON       m_hModuleIcon;
    HICON		m_hLevelIcon;
    HICON		m_hFlagsIcon;
    HICON		m_hCheckedIcon;
    HICON		m_hUncheckedIcon;
    HICON		m_hSelectedIcon;

    int		m_iPlayIcon;
    int		m_iPlayBlockedIcon;
    int		m_iRecordIcon;
    int		m_iPauseIcon;
    int     m_iModuleIcon;
    int		m_iLevelIcon;
    int		m_iFlagsIcon;
    int		m_iCheckedIcon;
    int		m_iUncheckedIcon;
    int		m_iSelectedIcon;

    void UpdateProviderIcons(HTREEITEM hItem);
    void UpdateProviderSubTree(HTREEITEM hProviderItem);

// Atributos
public:
    CETViewerDoc* GetDocument();

// Operaciones
public:

// Reemplazos
    public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual void OnInitialUpdate(); // Se llama la primera vez después de la construcción

// Implementación
public:
    virtual ~CProviderTree();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

    void OnAddProvider(CTraceProvider *pProvider);
    void OnRemoveProvider(CTraceProvider *pProvider);
    void OnReplaceProvider(CTraceProvider *pOldProvider,CTraceProvider *pNewProvider);
    void OnProvidersModified();
    void OnSessionTypeChanged();
    void OnRemoveSelectedProvider();
    void SetAllProviderLevel(DWORD dwLevel);

protected:

// Funciones de asignación de mensajes generadas
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // Versión de depuración en ProviderTree.cpp
inline CETViewerDoc* CProviderTree::GetDocument()
   { return reinterpret_cast<CETViewerDoc*>(m_pDocument); }
#endif

