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

#include <windows.h>
#include <stack>
#include <map>
#include <list>
#include <set>
#include <deque>
#include "..\tstring.h"
#include <strsafe.h>
#include <vector>

#define PF_READ					0x0001
#define PF_WRITE				0x0002
#define PF_OPTIONAL				0x0004
#define PF_NORMAL				PF_READ|PF_WRITE

struct SPersistencyProperty
{
    std::tstring name;
    std::tstring value;
};

class IPersistencyNode
{
public:
    virtual void Clear()=0;
    virtual bool AddProperty(SPersistencyProperty property)=0;
    virtual bool GetProperty(SPersistencyProperty *pProperty)=0;
    virtual bool RemoveProperty(SPersistencyProperty property)=0;

    virtual IPersistencyNode *AddNode(std::tstring id)=0;
    virtual IPersistencyNode *GetNode(std::tstring id)=0;
    virtual void DeleteNode(std::tstring id)=0;
};

class IPersistencyItem
{
public:

    virtual void Initialize()=0;
    virtual void Free()=0;
    virtual void SetDefaultValue()=0;

    virtual HRESULT Load(IPersistencyNode *)=0;
    virtual HRESULT Save(IPersistencyNode *)=0;
    virtual HRESULT Remove(IPersistencyNode *)=0;

    virtual TCHAR *GetName()=0;
};


template<typename T1> 
class CPersistentReferenceT:public IPersistencyItem
{
protected:

    DWORD		m_dwFlags;

    T1			*m_pValue;
    TCHAR		m_sName[200];

public:

    TCHAR *GetName(){return m_sName;}
    T1	 *GetValueAddress(){return m_pValue;}
    void SetDefaultValue(){}

    CPersistentReferenceT(T1 *pValue,TCHAR *pName,DWORD flags)
    {
        m_dwFlags=flags;
        m_pValue=pValue;
        _tcscpy_s(m_sName,200,pName);
    }
    virtual ~CPersistentReferenceT(){}
};


template<typename T1> 
class CPersistentSimpleReferenceT:public CPersistentReferenceT<T1>
{
public:

    void	Initialize(){PersistencyInitialize(this);}
    void	Free(){PersistencyFree(this);}
    void	SetDefaultValue(){}
    HRESULT Load(IPersistencyNode *piNode){HRESULT hr=S_OK;if(m_dwFlags&PF_READ){hr=PersistencyLoad(piNode,this);}if(m_dwFlags&PF_OPTIONAL){return S_OK;}return hr;}
    HRESULT Save(IPersistencyNode *piNode){HRESULT hr=S_OK;if(m_dwFlags&PF_WRITE){hr=PersistencySave(piNode,this);}if(m_dwFlags&PF_OPTIONAL){return S_OK;}return hr;}
    HRESULT Remove(IPersistencyNode *piNode){HRESULT hr=S_OK;if(m_dwFlags&PF_WRITE){hr=PersistencyRemove(piNode,this);}if(m_dwFlags&PF_OPTIONAL){return S_OK;}return hr;}

    CPersistentSimpleReferenceT(T1 *pValue,TCHAR *pName,DWORD flags):CPersistentReferenceT<T1>(pValue,pName,flags){}
    virtual ~CPersistentSimpleReferenceT(){}
};


template<typename T1> 
class CPersistentValueReferenceT:public CPersistentReferenceT<T1>
{
protected:
    T1 m_DefValue;

public:
    CPersistentValueReferenceT<T1> *SetDefaultValueAndReturnThis(T1 def)
    {
        PersistencyAsign(&m_DefValue,&def);
        return this;
    }

    CPersistentValueReferenceT(T1 *pValue,TCHAR *pName,DWORD flags)
        :CPersistentReferenceT<T1>(pValue,pName,flags)
    {
    }
    void	Initialize(){PersistencyInitialize(this);}
    void	Free(){PersistencyFree(this);}
    void	SetDefaultValue(){PersistencyAsign(m_pValue,&m_DefValue);}

    HRESULT Load(IPersistencyNode *piNode){HRESULT hr=S_OK;if(m_dwFlags&PF_READ){hr=PersistencyLoad(piNode,this);}if(m_dwFlags&PF_OPTIONAL){return S_OK;}return hr;}
    HRESULT Save(IPersistencyNode *piNode){HRESULT hr=S_OK;if(m_dwFlags&PF_WRITE){hr=PersistencySave(piNode,this);}if(m_dwFlags&PF_OPTIONAL){return S_OK;}return hr;}
    HRESULT Remove(IPersistencyNode *piNode){HRESULT hr=S_OK;if(m_dwFlags&PF_WRITE){hr=PersistencyRemove(piNode,this);}if(m_dwFlags&PF_OPTIONAL){return S_OK;}return hr;}

    ~CPersistentValueReferenceT(){}
};

template<typename T1> CPersistentValueReferenceT<T1> *PersistCreateReferenceWithDefaultValue(T1 *pVar,TCHAR *name,DWORD flags=PF_NORMAL){return new CPersistentValueReferenceT<T1>(pVar,name,flags);}
template<typename T1> CPersistentSimpleReferenceT<T1> *PersistCreateReference(T1 *pVar,TCHAR *name,DWORD flags=PF_NORMAL){return new CPersistentSimpleReferenceT<T1>(pVar,name,flags);}

#define DECLARE_SERIALIZABLE(className)\
    static HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<className>*pItem)\
    {\
        IPersistencyNode *piNode=piParent->GetNode(pItem->GetName());\
        if(piNode==NULL){return E_FAIL;}else {return pItem->GetValueAddress()->PersistencyLoad(piNode);}\
    }\
    static HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<className>*pItem)\
    {\
        IPersistencyNode *piNode=piParent->AddNode(pItem->GetName());\
        if(piNode==NULL){return E_FAIL;}else {return pItem->GetValueAddress()->PersistencySave(piNode);}\
    }\
    static HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<className>*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}\
    static void PersistencyInitialize(CPersistentReferenceT<className>*pItem){pItem->GetValueAddress()->PersistencyInitialize();}\
    static void PersistencyFree(CPersistentReferenceT<className>*pItem){pItem->GetValueAddress()->PersistencyFree();}

#define DECLARE_SERIALIZABLE_ENUMERATION(enumeration)\
    static HRESULT PersistencyLoad(IPersistencyNode *piParent,CPersistentReferenceT<enumeration>*pItem){return PersistencyLoad(piParent,(CPersistentReferenceT<int>*)pItem);}\
    static HRESULT PersistencySave(IPersistencyNode *piParent,CPersistentReferenceT<enumeration>*pItem){return PersistencySave(piParent,(CPersistentReferenceT<int>*)pItem);}\
    static HRESULT PersistencyRemove(IPersistencyNode *piParent,CPersistentReferenceT<enumeration>*pItem){piParent->DeleteNode(pItem->GetName());return S_OK;}\
    static void PersistencyInitialize(CPersistentReferenceT<enumeration>*pItem){return PersistencyInitialize((CPersistentReferenceT<int>*)pItem);}\
    static void PersistencyFree(CPersistentReferenceT<enumeration>*pItem){}

#define BEGIN_PERSIST_MAP(className)	\
    IPersistencyItem **PersistGetPropertyMap(TCHAR *pMapName,TCHAR *persistNamePrefix)\
    {												\
        bool bInSpecifiedSubMap=(pMapName==NULL);\
        className *pInstance=this;						\
        std::list<IPersistencyItem *> items;				\
        TCHAR szPrefix[1024]={0};						\
        TCHAR szvarName[1024]={0};						\
        if (persistNamePrefix != NULL){ _stprintf_s(szPrefix, 1024, _T("%s"), persistNamePrefix); }

#define PERSIST(var,str) 							if(bInSpecifiedSubMap){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReference(&pInstance->var,szvarName));}
#define PERSIST_FLAGS(var,str,flags) 				if(bInSpecifiedSubMap){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReference(&pInstance->var,szvarName,flags));}
#define PERSIST_VALUE(var,str,value) 				if(bInSpecifiedSubMap){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReferenceWithDefaultValue(&pInstance->var,szvarName)->SetDefaultValueAndReturnThis(value));}
#define PERSIST_VALUE_FLAGS(var,str,value,flags) 	if(bInSpecifiedSubMap){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReferenceWithDefaultValue(&pInstance->var,szvarName,flags)->SetDefaultValueAndReturnThis(value));}
#define PERSIST_POINTER(var,str) 					        if(bInSpecifiedSubMap && var){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReference(pInstance->var,szvarName));}
#define PERSIST_POINTER_FLAGS(var,str,flags) 				if(bInSpecifiedSubMap && var){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReference(pInstance->var,szvarName,flags));}
#define PERSIST_POINTER_VALUE(var,str,value) 				if(bInSpecifiedSubMap && var){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReferenceWithDefaultValue(pInstance->var,szvarName)->SetDefaultValueAndReturnThis(value));}
#define PERSIST_POINTER_VALUE_FLAGS(var,str,value,flags) 	if(bInSpecifiedSubMap && var){StringCbPrintf(szvarName,1024,_T("%s%s"),szPrefix,str);items.push_back(PersistCreateReferenceWithDefaultValue(pInstance->var,szvarName,flags)->SetDefaultValueAndReturnThis(value));}

#define BEGIN_PERSIST_SUBMAP(str) 	if(pMapName && strcmp(pMapName,str)==0){bInSpecifiedSubMap=true;
#define END_PERSIST_SUBMAP(str) 	bInSpecifiedSubMap=false;}

#define PERSIST_CLASS_CHAIN(otherClass)\
if(bInSpecifiedSubMap){\
    IPersistencyItem **ppOtherClassItems=otherClass::PersistGetPropertyMap(NULL,szPrefix);\
    int x=0;\
    while(ppOtherClassItems[x]!=NULL){items.push_back(ppOtherClassItems[x]);x++;}\
    delete [] ppOtherClassItems;\
}
#define PERSIST_CLASS_CHAIN_SUBMAP(otherClass,subMap)\
if(bInSpecifiedSubMap){\
    IPersistencyItem **ppOtherClassItems=otherClass::PersistGetPropertyMap(subMap,szPrefix);\
    int x=0;\
    while(ppOtherClassItems[x]!=NULL){items.push_back(ppOtherClassItems[x]);x++;}\
    delete [] ppOtherClassItems;\
}

#define PERSIST_CLASS_CHAIN_PREFIX(otherClass,chainPrefix)\
if(bInSpecifiedSubMap){\
    TCHAR szChainPrefix[STRING_SIZE]={0};\
    _stprintf_s(szChainPrefix, szPrefix); \
    strcat(szChainPrefix,chainPrefix);\
    IPersistencyItem **ppOtherClassItems=otherClass::PersistGetPropertyMap(pMapName,szChainPrefix);\
    int x=0;\
    while(ppOtherClassItems[x]!=NULL){items.push_back(ppOtherClassItems[x]);x++;}\
    delete [] ppOtherClassItems;\
}

void	_PersistencyDefaultValue(IPersistencyItem **ppiList,TCHAR *pPrefixName=NULL);
HRESULT _PersistencySave(IPersistencyItem **ppiList,IPersistencyNode *piNode,TCHAR *pPrefixName=NULL);
HRESULT _PersistencyLoad(IPersistencyItem **ppiList,IPersistencyNode *piNode,TCHAR *pPrefixName=NULL);
HRESULT _PersistencyRemove(IPersistencyItem **ppiList,IPersistencyNode *piNode,TCHAR *pPrefixName=NULL);
void	_PersistencyInitialize(IPersistencyItem **ppiList,TCHAR *pPrefixName=NULL);
void	_PersistencyFree(IPersistencyItem **ppiList,TCHAR *pPrefixName=NULL);
void	_FreePersistencyPropertyMap(IPersistencyItem ***ppiList);

#define END_PERSIST_MAP()\
    int x;\
    std::list<IPersistencyItem *>::iterator i;	\
    IPersistencyItem **pList=new IPersistencyItem *[(items.size()+1)];\
    for(x=0,i=items.begin();i!=items.end();i++,x++){pList[x]=(*i);}\
    pList[items.size()]=NULL;\
    return pList;\
};     \
    virtual void PersistencyDefaultValue(TCHAR *pMapName=NULL,TCHAR *pPrefixName=NULL)\
{\
    IPersistencyItem **ppiList=PersistGetPropertyMap(pMapName,pPrefixName);\
    _PersistencyDefaultValue(ppiList, pPrefixName);\
    _FreePersistencyPropertyMap(&ppiList);\
}\
    virtual HRESULT PersistencySave(IPersistencyNode *piNode,TCHAR *pMapName=NULL,TCHAR *pPrefixName=NULL)\
{\
    if(piNode==NULL){return E_FAIL;}\
    IPersistencyItem **ppiList=PersistGetPropertyMap(pMapName,pPrefixName);\
    HRESULT finalhr=_PersistencySave(ppiList,piNode,pPrefixName);\
    _FreePersistencyPropertyMap(&ppiList);\
    return finalhr;\
} \
    virtual HRESULT PersistencyLoad(IPersistencyNode *piNode,TCHAR *pMapName=NULL,TCHAR *pPrefixName=NULL)\
{\
    if(piNode==NULL){return E_FAIL;}\
    IPersistencyItem **ppiList=PersistGetPropertyMap(pMapName,pPrefixName);\
    HRESULT finalhr=_PersistencyLoad(ppiList,piNode,pPrefixName);\
    _FreePersistencyPropertyMap(&ppiList);\
    return finalhr;\
}\
    virtual HRESULT PersistencyRemove(IPersistencyNode *piNode,TCHAR *pMapName=NULL,TCHAR *pPrefixName=NULL)\
{\
    if(piNode==NULL){return E_FAIL;}\
    IPersistencyItem **ppiList=PersistGetPropertyMap(pMapName,pPrefixName);\
    HRESULT finalhr=_PersistencyRemove(ppiList,piNode,pPrefixName);\
    _FreePersistencyPropertyMap(&ppiList);\
    return finalhr;\
}\
    virtual void PersistencyInitialize(TCHAR *pMapName=NULL,TCHAR *pPrefixName=NULL)\
{\
    IPersistencyItem **ppiList=PersistGetPropertyMap(pMapName,pPrefixName);\
    _PersistencyInitialize(ppiList,pPrefixName);\
    _FreePersistencyPropertyMap(&ppiList);\
}\
    virtual void PersistencyFree(TCHAR *pMapName=NULL,TCHAR *pPrefixName=NULL)\
{\
    IPersistencyItem **ppiList=PersistGetPropertyMap(pMapName,pPrefixName);\
    _PersistencyFree(ppiList,pPrefixName);\
    _FreePersistencyPropertyMap(&ppiList);\
}

#define BEGIN_STRUCT_PERSISTS(className)			\
    static IPersistencyItem **PersistGetPropertyMap(className *pInstance);\
    static HRESULT PersistencySave(IPersistencyNode *piNode,CPersistentSimpleReferenceT<className> *PERSIST)\
    {\
        if(piNode==NULL){return E_FAIL;}\
        HRESULT hr=S_OK,finalhr=S_OK;\
        IPersistencyNode *piStructNode=piNode->AddNode(PERSIST->GetName());\
        if(piStructNode==NULL){return E_FAIL;}\
        IPersistencyItem **ppiList=PersistGetPropertyMap(PERSIST->GetValueAddress());\
        int x=0;\
        while(ppiList[x]!=NULL){hr=ppiList[x]->Save(piStructNode);if(FAILED(hr)){finalhr=hr;};delete ppiList[x];ppiList[x]=NULL;x++;}\
        delete [] ppiList;ppiList=NULL;\
        return finalhr;\
    }\
    static HRESULT PersistencyLoad(IPersistencyNode *piNode,CPersistentSimpleReferenceT<className> *PERSIST)\
    {\
        if(piNode==NULL){return E_FAIL;}\
        HRESULT hr=S_OK,finalhr=S_OK;\
        IPersistencyNode *piStructNode=piNode->GetNode(PERSIST->GetName());\
        if(piStructNode==NULL){return E_FAIL;}\
        IPersistencyItem **ppiList=PersistGetPropertyMap(PERSIST->GetValueAddress());\
        int x=0;\
        while(ppiList[x]!=NULL){hr=ppiList[x]->Load(piStructNode);if(FAILED(hr)){finalhr=hr;};delete ppiList[x];ppiList[x]=NULL;x++;}\
        delete [] ppiList;ppiList=NULL;\
        return finalhr;\
    }\
    static HRESULT PersistencyRemove(IPersistencyNode *piNode,CPersistentSimpleReferenceT<className> *PERSIST)\
    {\
        if(piNode==NULL){return E_FAIL;}\
        piNode->DeleteNode(PERSIST->GetName());\
        return S_OK;\
    }\
    static void PersistencyInitialize(CPersistentSimpleReferenceT<className> *PERSIST)\
    {\
        HRESULT hr=S_OK,finalhr=S_OK;\
        IPersistencyItem **ppiList=PersistGetPropertyMap(PERSIST->GetValueAddress());\
        int x=0;\
        while(ppiList[x]!=NULL){ppiList[x]->Initialize();if(FAILED(hr)){finalhr=hr;};delete ppiList[x];ppiList[x]=NULL;x++;}\
        delete [] ppiList;ppiList=NULL;\
    }\
    static void PersistencyFree(CPersistentSimpleReferenceT<className> *PERSIST)\
    {\
        HRESULT hr=S_OK,finalhr=S_OK;\
        IPersistencyItem **ppiList=PersistGetPropertyMap(PERSIST->GetValueAddress());\
        int x=0;\
        while(ppiList[x]!=NULL){ppiList[x]->Free();if(FAILED(hr)){finalhr=hr;};delete ppiList[x];ppiList[x]=NULL;x++;}\
        delete [] ppiList;ppiList=NULL;\
    }\
    static IPersistencyItem **PersistGetPropertyMap(className *pInstance)	\
    {													\
        bool bInSpecifiedSubMap=true;\
        std::list<IPersistencyItem *> items;			\
        TCHAR szPrefix[1024]={0};						\
        TCHAR szvarName[1024]={0};						

#define END_STRUCT_PERSISTS()						\
        int x;\
        std::list<IPersistencyItem *>::iterator i;	\
        IPersistencyItem **pList=new IPersistencyItem *[(items.size()+1)];\
        for(x=0,i=items.begin();i!=items.end();i++,x++){pList[x]=(*i);}\
        pList[items.size()]=NULL;\
        return pList;\
    };

