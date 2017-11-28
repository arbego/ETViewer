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

#include <windows.h>
#include ".\persistency.h"
#include ".\persistenttypes.h"

HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<DWORD> *pItem)
{
    TCHAR sTemp[1024]={0};
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    _stprintf_s(sTemp,_T("%d"),*pItem->GetValueAddress());
    prop.value=sTemp;
    return piNode->AddProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<DWORD> *pItem)
{
    pItem->SetDefaultValue();
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    if(piNode->GetProperty(&prop)){(*pItem->GetValueAddress())=_ttoi(prop.value.c_str());return S_OK;}
    return E_FAIL;
}

HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<DWORD> *pItem)
{
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    return piNode->RemoveProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<int> *pItem)
{
    TCHAR sTemp[1024]={0};
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    _stprintf_s(sTemp,_T("%d"),*pItem->GetValueAddress());
    prop.value=sTemp;
    return piNode->AddProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<int> *pItem)
{
    pItem->SetDefaultValue();
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    if(piNode->GetProperty(&prop)){(*pItem->GetValueAddress())=_ttoi(prop.value.c_str());return S_OK;}
    return E_FAIL;
}

HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<int> *pItem)
{
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    return piNode->RemoveProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<bool> *pItem)
{
    TCHAR sTemp[1024]={0};
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    _stprintf_s(sTemp,_T("%d"),(int)(*pItem->GetValueAddress()));
    prop.value=sTemp;
    return piNode->AddProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<bool> *pItem)
{
    pItem->SetDefaultValue();
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    if(piNode->GetProperty(&prop)){(*pItem->GetValueAddress())=_ttoi(prop.value.c_str())?true:false;return S_OK;}
    return E_FAIL;
}

HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<bool> *pItem)
{
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    return piNode->RemoveProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<float> *pItem)
{
    TCHAR sTemp[1024]={0};
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    _stprintf_s(sTemp,_T("%f"),*pItem->GetValueAddress());
    prop.value=sTemp;
    return piNode->AddProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<float> *pItem)
{
    pItem->SetDefaultValue();
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    if(piNode->GetProperty(&prop)){(*pItem->GetValueAddress())=(float)_ttof(prop.value.c_str());return S_OK;}
    return E_FAIL;
}
HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<float> *pItem)
{
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    return piNode->RemoveProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<double> *pItem)
{
    TCHAR sTemp[1024]={0};
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    _stprintf_s(sTemp,_T("%f"),*pItem->GetValueAddress());
    prop.value=sTemp;
    return piNode->AddProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<double> *pItem)
{
    pItem->SetDefaultValue();
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    if(piNode->GetProperty(&prop)){(*pItem->GetValueAddress())=_ttof(prop.value.c_str());return S_OK;}
    return E_FAIL;
}
HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<double> *pItem)
{
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    return piNode->RemoveProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<std::tstring>*pItem)
{
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    prop.value=*pItem->GetValueAddress();
    return piNode->AddProperty(prop)?S_OK:E_FAIL;
}

HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<std::tstring>*pItem)
{
    pItem->SetDefaultValue();
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    if(piNode->GetProperty(&prop)){*pItem->GetValueAddress()=prop.value;return S_OK;}
    return E_FAIL;
}

HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<std::tstring> *pItem)
{
    SPersistencyProperty prop;
    prop.name=pItem->GetName();
    return piNode->RemoveProperty(prop)?S_OK:E_FAIL;
}

void PersistencyInitialize(CPersistentReferenceT<DWORD> *pItem)			{(*pItem->GetValueAddress())=0;}
void PersistencyInitialize(CPersistentReferenceT<int> *pItem)			{(*pItem->GetValueAddress())=0;}
void PersistencyInitialize(CPersistentReferenceT<float> *pItem)			{(*pItem->GetValueAddress())=0;}
void PersistencyInitialize(CPersistentReferenceT<double> *pItem)		{(*pItem->GetValueAddress())=0;}
void PersistencyInitialize(CPersistentReferenceT<bool> *pItem)			{(*pItem->GetValueAddress())=0;}
void PersistencyInitialize(CPersistentReferenceT<std::tstring> *pItem)   {(*pItem->GetValueAddress())=_T("");}


void PersistencyFree(CPersistentReferenceT<DWORD> *prop){}
void PersistencyFree(CPersistentReferenceT<int> *prop){}
void PersistencyFree(CPersistentReferenceT<float> *prop){}
void PersistencyFree(CPersistentReferenceT<double> *prop){}
void PersistencyFree(CPersistentReferenceT<bool> *pItem){}
void PersistencyFree(CPersistentReferenceT<std::tstring> *pItem){}

