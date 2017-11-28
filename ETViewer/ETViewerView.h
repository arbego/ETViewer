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

#include "FindDialog.h"
#include "TraceController.h"

#define CAPTURE_TIMER			1
#define SHOW_LAST_TRACE_TIMER	2

struct CColumnInfo
{
    int					width;
    bool				visible;
    std::tstring        name;
    DWORD				format;
    int					id;
    int					iSubItem;
    int					iOrder;

    BEGIN_PERSIST_MAP(CColumnInfo)
        PERSIST(width,_T("Width"))
        PERSIST(visible,_T("Visible"))
        PERSIST(name,_T("Name"))
        PERSIST(format,_T("Format"))
        PERSIST(id,_T("Id"))
        PERSIST(iOrder,_T("Order"))
    END_PERSIST_MAP();

    CColumnInfo(){width=100;visible=false;}
    CColumnInfo(int _id,TCHAR* n,int fmt,int w,bool v,int o){id=_id;format=fmt;width=w;visible=v;name=n;iSubItem=-1;iOrder=o;}
};

DECLARE_SERIALIZABLE(CColumnInfo);

struct SETViewerTrace
{
    STraceEvenTracingNormalizedData trace;

    int		iImage;

    SETViewerTrace()
    {
        iImage=0;
    }
};

class CETViewerView : public CListView, public CFindDialogClient, public ITraceEvents
{
protected: // Crear sólo a partir de serialización
    CETViewerView();
    DECLARE_DYNCREATE(CETViewerView)

// Atributos
public:
    CETViewerDoc* GetDocument() const;

// Operaciones
public:

// Reemplazos
    public:
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
    virtual void OnInitialUpdate(); // Se llama la primera vez después de la construcción

// Implementación
public:
    virtual ~CETViewerView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

    std::tstring		m_LastTextToFind;
    bool				m_bFindDirectionUp;

    std::deque<CColumnInfo>		m_ColumnInfo;
    std::map<int, CColumnInfo*>	m_mVisibleColumns;

    WNDPROC		m_OldListEditProc;
    WNDPROC		m_OldListViewProc;

    HIMAGELIST	m_hImageList;
    HICON		m_hHollowIcon;
    HICON		m_hMarkerIcon;

    std::deque<SETViewerTrace *>	m_lTraces;
    HANDLE					        m_hTracesMutex;
    bool					        m_bShowLastTrace;
    
    int m_iHollowImage;
    int m_iMarkerImage;

    int m_iEditSubItem;
    CSize m_ListEditSize;
    CPoint m_ListEditPos;
    CEdit *m_pEdit;

    CFont *m_pTraceFont;
    DWORD m_dwTraceFontSize;
    std::tstring m_sTraceFont;

    COLORREF m_cNormalTextColor;
    COLORREF m_cNormalBkColor;
    COLORREF m_cSelectedTextColor;
    COLORREF m_cSelectedBkColor;

    HBRUSH m_hNormalBrush;
    HBRUSH m_hSelectedBrush;
    HBRUSH m_hHollowBrush;

    HPEN m_hNormalPen;
    HPEN m_hSelectedPen;

    int m_SortColumn;
    int m_SortDirection;

    int m_nLastFocusedSequenceIndex;
    int m_nUnformattedTraces;

    void UpdateFont();
    void UpdateColumns();
    void AddColumn(int id);
    void RemoveColumn(int id);

    static LRESULT CALLBACK ListViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static LRESULT CALLBACK ListEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    void ProcessSpecialKeyStroke(WORD key);

    int FindText(const TCHAR* pTextToFind,int baseIndex,bool up,bool stopAtEnd=false);

    void SetFocusOnOwnerWindow();

    TCHAR *GetTraceText(SETViewerTrace *pTrace,CColumnInfo *pColumn,TCHAR *pAuxBuffer,unsigned nAuxLen);

    BEGIN_PERSIST_MAP(CETViewerView)
        PERSIST(m_ColumnInfo,_T("Columns"))
        PERSIST(m_dwTraceFontSize,_T("FontSize"));
        PERSIST(m_sTraceFont,_T("FontFamily"));
        PERSIST(m_bShowLastTrace,_T("ShowLastTrace"));
    END_PERSIST_MAP();

    DECLARE_CONFIG_FILE_MEDIA();

    
public:

    bool Load(CConfigFile *pFile);
    bool Save(CConfigFile *pFile);

    void OnFontBigger();
    void OnFontSmaller();

    bool IsShowLastTraceEnabled();
    void EnableShowLastTrace(bool bEnable);
    void ResetShowLastTrace();

    bool FindNext();
    bool FindNext(const TCHAR *pText);
    bool FindAndMarkAll(const TCHAR *pText);
    bool FindAndDeleteAll(const TCHAR *pText);

    void Copy(bool bAllTraces);
    void Clear();
    void ClearSelected();

    void GetTransferBuffer(int *pSize,TCHAR **buffer,bool bAllTraces);
    void GetTraceColors(SETViewerTrace *pTrace,COLORREF *pTextColor,COLORREF *pBkColor,HPEN *phPen,HBRUSH *phBrush);

    // ITraceEvents
    void ProcessTrace(STraceEvenTracingNormalizedData *pTraceData);
    void ProcessUnknownTrace(STraceEvenTracingNormalizedData *pTraceData);

    void OnAddProvider(CTraceProvider *pProvider);
    void OnRemoveProvider(CTraceProvider *pProvider);
    void OnReplaceProvider(CTraceProvider *pOldProvider,CTraceProvider *pNewProvider);
    void OnProvidersModified();
    void OnSessionTypeChanged();

    void SetTraceFont(std::tstring sTraceFont, DWORD dwFontSize);
    void GetTraceFont(std::tstring *psTraceFont, DWORD *pdwFontSize);

    void SortItems(CColumnInfo *pColumn);

// Funciones de asignación de mensajes generadas
protected:
    afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
    afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEditFind();
    afx_msg void OnFind();
    afx_msg void OnHighLightFilters();
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnClear();
    afx_msg void OnCut();
    afx_msg void OnCopy();
    afx_msg void OnSave();
    afx_msg void OnBeginEdit(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEndEdit(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnGetItemInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // Versión de depuración en ETViewerView.cpp
inline CETViewerDoc* CETViewerView::GetDocument() const
   { return reinterpret_cast<CETViewerDoc*>(m_pDocument); }
#endif

