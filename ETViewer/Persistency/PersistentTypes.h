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

template<typename T1,typename T2> static void PersistencyAsign(T1 *pVar1,T2 *pVar2){(*pVar1)=(*pVar2);}

/////////////////////////////////////////////////////
// Funciones para guardar tipos fundamentales

HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<DWORD> *prop);
HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<int> *prop);
HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<float> *prop);
HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<double> *prop);
HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<bool> *pItem);
HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentReferenceT<std::tstring> *pItem);

HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<bool> *pItem);
HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<int> *pItem);
HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<DWORD> *prop);
HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<float> *prop);
HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<double> *prop);
HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentReferenceT<std::tstring> *pItem);

HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<DWORD> *prop);
HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<int> *prop);
HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<float> *prop);
HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<double> *prop);
HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<bool> *pItem);
HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentReferenceT<std::tstring> *pItem);

void PersistencyInitialize(CPersistentReferenceT<DWORD> *prop);
void PersistencyInitialize(CPersistentReferenceT<int> *prop);
void PersistencyInitialize(CPersistentReferenceT<float> *prop);
void PersistencyInitialize(CPersistentReferenceT<double> *prop);
void PersistencyInitialize(CPersistentReferenceT<bool> *pItem);
void PersistencyInitialize(CPersistentReferenceT<std::tstring> *pItem);

void PersistencyFree(CPersistentReferenceT<DWORD> *prop);
void PersistencyFree(CPersistentReferenceT<int> *prop);
void PersistencyFree(CPersistentReferenceT<float> *prop);
void PersistencyFree(CPersistentReferenceT<double> *prop);
void PersistencyFree(CPersistentReferenceT<bool> *pItem);
void PersistencyFree(CPersistentReferenceT<std::tstring> *pItem);

/////////////////////////////////////////////////////
// Funciones para guardar contenedores no asociativos

template<typename T1,typename CONTAINED_TYPE>
HRESULT PersistLoadFromContainer(IPersistencyNode *piParent,CPersistentReferenceT<T1>*pItem)
{
    IPersistencyNode *piNode=piParent->GetNode(pItem->GetName());
    if(piNode==NULL){return E_FAIL;}
    pItem->GetValueAddress()->clear();

    SPersistencyProperty countProp;
    countProp.name=_T("ItemCount");
    if(!piNode->GetProperty(&countProp)){return E_FAIL;}
    DWORD itemCount=_ttoi(countProp.value.c_str());

    HRESULT hr=S_OK,finalhr=S_OK;
    for(DWORD x=0;x<itemCount;x++)
    {
        TCHAR currentName[512];
        _stprintf_s(currentName,512,_T("Item%d"),x);

        CONTAINED_TYPE var;
        CPersistentSimpleReferenceT<CONTAINED_TYPE> *pRef=PersistCreateReference(&var,currentName);
        PersistencyInitialize(pRef);
        hr=PersistencyLoad(piNode,pRef);
        if(SUCCEEDED(hr)){pItem->GetValueAddress()->insert(pItem->GetValueAddress()->end(),*pRef->GetValueAddress());}
        delete pRef;
        pRef=NULL;
        if(FAILED(hr)){finalhr=hr;}
    }

    return finalhr;
}

template<typename T1,typename CONTAINED_TYPE>
HRESULT PersistSaveToContainer(IPersistencyNode *piParent,CPersistentReferenceT<T1> *pItem)
{
    IPersistencyNode *piNode=piParent->AddNode(pItem->GetName());
    if(piNode==NULL){return E_FAIL;}
    piNode->Clear();

    TCHAR countValue[512];
    _stprintf_s(countValue,512,_T("%d"),pItem->GetValueAddress()->size());
    SPersistencyProperty countProp;
    countProp.name=_T("ItemCount");
    countProp.value=countValue;
    piNode->AddProperty(countProp);

    DWORD itemCount=0;
    HRESULT hr=S_OK,finalhr=S_OK;
    if(SUCCEEDED(finalhr))
    {
        T1::iterator i;
        for(i=pItem->GetValueAddress()->begin();i!=pItem->GetValueAddress()->end();i++)
        {
            TCHAR currentName[512];
            _stprintf_s(currentName,_T("Item%d"),itemCount);
            CPersistentSimpleReferenceT<CONTAINED_TYPE> *pRef=PersistCreateReference(&(*i),currentName);
            hr=PersistencySave(piNode,pRef);
            if(SUCCEEDED(hr)){itemCount++;}
            if(FAILED(hr)){finalhr=hr;}
            delete pRef;
            pRef=NULL;
        }
    }
    return finalhr;
}

/////////////////////////////////////////////////////
// Funciones para guardar contenedores asociativos

template<typename T1,typename KEY_TYPE,typename CONTAINED_TYPE>
HRESULT PersistLoadFromContainer(IPersistencyNode *piParent,CPersistentReferenceT<T1>*pItem)
{
    IPersistencyNode *piNode=piParent->GetNode(pItem->GetName());
    if(piNode==NULL){return E_FAIL;}
    pItem->GetValueAddress()->clear();

    SPersistencyProperty countProp;
    countProp.name=_T("ItemCount");
    if(!piNode->GetProperty(&countProp)){return E_FAIL;}
    DWORD itemCount=atoi(countProp.value.c_str());

    HRESULT hr=S_OK,finalhr=S_OK;
    for(DWORD x=0;x<itemCount;x++)
    {
        TCHAR keyName[512];
        TCHAR valueName[512];
        _stprintf_s(keyName, _T("ItemKey%d"), x);
        _stprintf_s(valueName,_T("ItemValue%d"),x);

        KEY_TYPE        key;
        CONTAINED_TYPE  value;
        CPersistentSimpleReferenceT<KEY_TYPE>         *pKeyRef=PersistCreateReference(&key,keyName);
        CPersistentSimpleReferenceT<CONTAINED_TYPE>   *pValueRef=PersistCreateReference(&value,valueName);
        PersistencyInitialize(pKeyRef);
        PersistencyInitialize(pValueRef);
        hr=PersistencyLoad(piNode,pKeyRef);
        if(SUCCEEDED(hr)){hr=PersistencyLoad(piNode,pValueRef);}
        if(SUCCEEDED(hr)){pItem->GetValueAddress()->insert(T1::value_type(key,value));}
        delete pKeyRef;
        delete pValueRef;
        pKeyRef=NULL;
        pValueRef=NULL;
        if(FAILED(hr)){finalhr=hr;}
    }
    return finalhr;
}

template<typename T1,typename KEY_TYPE,typename CONTAINED_TYPE>
HRESULT PersistSaveToContainer(IPersistencyNode *piParent,CPersistentReferenceT<T1> *pItem)
{
    IPersistencyNode *piNode=piParent->AddNode(pItem->GetName());
    if(piNode==NULL){return E_FAIL;}
    piNode->Clear();

    TCHAR countValue[512];
    _stprintf_s(countValue,_T("%d"),pItem->GetValueAddress()->size());
    SPersistencyProperty countProp;
    countProp.name=_T("ItemCount");
    countProp.value=countValue;
    piNode->AddProperty(countProp);

    DWORD itemCount=0;
    HRESULT hr=S_OK,finalhr=S_OK;
    if(SUCCEEDED(finalhr))
    {
        T1::iterator i;
        for(i=pItem->GetValueAddress()->begin();i!=pItem->GetValueAddress()->end();i++)
        {
            TCHAR keyName[512];
            TCHAR valueName[512];
            _stprintf_s(keyName,_T("ItemKey%d"),itemCount);
            _stprintf_s(valueName,_T("ItemValue%d"),itemCount);
            CPersistentSimpleReferenceT<KEY_TYPE>         *pKeyRef=PersistCreateReference(const_cast<KEY_TYPE *>(&(i->first)),keyName);
            CPersistentSimpleReferenceT<CONTAINED_TYPE>   *pValueRef=PersistCreateReference(&(i->second),valueName);
            hr=PersistencySave(piNode,pKeyRef);
            if(SUCCEEDED(hr)){hr=PersistencySave(piNode,pValueRef);}
            if(SUCCEEDED(hr)){itemCount++;}
            if(FAILED(hr)){finalhr=hr;}
            delete pKeyRef;
            delete pValueRef;
            pKeyRef=NULL;
            pValueRef=NULL;
        }
    }
    return finalhr;
}

/////////////////////////////////////////////////////
// Funciones para guardar contenedores, usan las funciones anteriores
// para guardar todos los contenedores de la misma manera.

template<typename T1>void    PersistencyInitialize(CPersistentReferenceT<std::list<T1> >*pItem){}
template<typename T1>void    PersistencyFree(CPersistentReferenceT<std::list<T1> >*pItem){}
template<typename T1>HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<std::list<T1> >*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}
template<typename T1>HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<std::list<T1> > *pItem){return PersistLoadFromContainer<std::list<T1> , T1 >(piParent,pItem);}
template<typename T1>HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<std::list<T1> > *pItem){return PersistSaveToContainer<std::list<T1> , T1 >(piParent,pItem);}

template<typename T1>void    PersistencyInitialize(CPersistentReferenceT<std::deque<T1> >*pItem){}
template<typename T1>void    PersistencyFree(CPersistentReferenceT<std::deque<T1> >*pItem){}
template<typename T1>HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<std::deque<T1> >*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}
template<typename T1>HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<std::deque<T1> > *pItem){return PersistLoadFromContainer<std::deque<T1> , T1 >(piParent,pItem);}
template<typename T1>HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<std::deque<T1> > *pItem){return PersistSaveToContainer<std::deque<T1> , T1 >(piParent,pItem);}

template<typename T1>void    PersistencyInitialize(CPersistentReferenceT<std::vector<T1> >*pItem){}
template<typename T1>void    PersistencyFree(CPersistentReferenceT<std::vector<T1> >*pItem){}
template<typename T1>HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<std::vector<T1> >*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}
template<typename T1>HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<std::vector<T1> > *pItem){return PersistLoadFromContainer<std::vector<T1> , T1 >(piParent,pItem);}
template<typename T1>HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<std::vector<T1> > *pItem){return PersistSaveToContainer<std::vector<T1> , T1 >(piParent,pItem);}

template<typename T1>void    PersistencyInitialize(CPersistentReferenceT<std::set<T1> >*pItem){}
template<typename T1>void    PersistencyFree(CPersistentReferenceT<std::set<T1> >*pItem){}
template<typename T1>HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<std::set<T1> >*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}
template<typename T1>HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<std::set<T1> > *pItem){return PersistLoadFromContainer<std::set<T1> , T1 >(piParent,pItem);}
template<typename T1>HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<std::set<T1> > *pItem){return PersistSaveToContainer<std::set<T1> , T1 >(piParent,pItem);}

template<typename T1,typename T2>void    PersistencyInitialize(CPersistentReferenceT<std::map<T1,T2> >*pItem){}
template<typename T1,typename T2>void    PersistencyFree(CPersistentReferenceT<std::map<T1,T2> >*pItem){}
template<typename T1,typename T2>HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<std::map<T1,T2> >*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}
template<typename T1,typename T2>HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<std::map<T1,T2> > *pItem){return PersistLoadFromContainer<std::map<T1,T2> , T1, T2 >(piParent,pItem);}
template<typename T1,typename T2>HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<std::map<T1,T2> > *pItem){return PersistSaveToContainer<std::map<T1,T2> , T1 , T2>(piParent,pItem);}

template<typename T1,typename T2>void    PersistencyInitialize(CPersistentReferenceT<std::multimap<T1,T2> >*pItem){}
template<typename T1,typename T2>void    PersistencyFree(CPersistentReferenceT<std::multimap<T1,T2> >*pItem){}
template<typename T1,typename T2>HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<std::multimap<T1,T2> >*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}
template<typename T1,typename T2>HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<std::multimap<T1,T2> > *pItem){return PersistLoadFromContainer<std::multimap<T1,T2> , T1, T2 >(piParent,pItem);}
template<typename T1,typename T2>HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<std::multimap<T1,T2> > *pItem){return PersistSaveToContainer<std::multimap<T1,T2> , T1 , T2>(piParent,pItem);}
