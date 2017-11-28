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
#include <io.h>
#include ".\Persistency.h"
#include ".\configfile.h"

#define CONFIG_FILE_DELIMITER _T(" \t:\r\n\"=")

CConfigFileNode::~CConfigFileNode()
{
    std::multimap<std::tstring,CConfigFileNode*>::iterator i;
    for(i=m_mNodes.begin();i!=m_mNodes.end();i++)
    {
        CConfigFileNode *pNode=i->second;
        delete pNode;
    }
}

std::tstring		CConfigFileNode::GetValue(std::tstring sValue){return m_mData[sValue];}
bool			CConfigFileNode::HasValue(std::tstring sValue){return m_mData.find(sValue)!=m_mData.end();}

CConfigFileNode *CConfigFileNode::GetAddNode_Internal(std::tstring sNodePath,bool bAdd)
{
    const TCHAR *pNodePath=sNodePath.c_str();
    const TCHAR *pSubNodeEnd=_tcschr(pNodePath,_T('\\'));
    if(pSubNodeEnd)
    {
        TCHAR sSubNodeName[MAX_PATH]={0};
        _tcsncpy_s(sSubNodeName,pNodePath,(DWORD)(pSubNodeEnd-pNodePath));
        std::multimap<std::tstring,CConfigFileNode*>::iterator i=m_mNodes.find((std::tstring)sSubNodeName);
        if(i!=m_mNodes.end())
        {
            CConfigFileNode *pNode=i->second;
            return pNode->GetAddNode_Internal(pSubNodeEnd+1,bAdd);
        }
        else
        {
            if(bAdd)
            {
                CConfigFileNode *pNode=new CConfigFileNode;
                pNode->m_sName=sSubNodeName;
                m_mNodes.insert(std::pair<std::tstring,CConfigFileNode*>(sSubNodeName,pNode));
                return pNode->GetAddNode_Internal((pSubNodeEnd+1),bAdd);
            }
            else
            {
                return NULL;
            }
        }
    }
    else
    {
        std::multimap<std::tstring,CConfigFileNode*>::iterator i=m_mNodes.find((std::tstring)pNodePath);
        if(i!=m_mNodes.end())
        {
            CConfigFileNode *pNode=i->second;
            return pNode;
        }
        else
        {
            if(bAdd)
            {
                CConfigFileNode *pNode=new CConfigFileNode;
                pNode->m_sName=sNodePath;
                m_mNodes.insert(std::pair<std::tstring,CConfigFileNode*>(sNodePath,pNode));
                return pNode;
            }
            else
            {
                return NULL;
            }
        }
    }
}

IPersistencyNode *CConfigFileNode::AddNode(std::tstring id){return GetAddNode_Internal(id,true);}
IPersistencyNode *CConfigFileNode::GetNode(std::tstring id){return GetAddNode_Internal(id,false);}

void CConfigFileNode::DeleteNode(std::tstring sNodePath)
{
    const TCHAR *pNodePath=sNodePath.c_str();
    const TCHAR *pSubNodeEnd=_tcschr(pNodePath,_T('\\'));
    if(pSubNodeEnd)
    {
        TCHAR sSubNodeName[MAX_PATH]={0};
        _tcsncpy_s(sSubNodeName,pNodePath,(DWORD)(pSubNodeEnd-pNodePath));
        std::multimap<std::tstring,CConfigFileNode*>::iterator i=m_mNodes.find((std::tstring)sSubNodeName);
        if(i!=m_mNodes.end())
        {
            CConfigFileNode *pNode=i->second;
            pNode->DeleteNode(pSubNodeEnd+1);
        }
    }
    else
    {
        std::multimap<std::tstring,CConfigFileNode*>::iterator i=m_mNodes.find(sNodePath);
        if(i!=m_mNodes.end())
        {
            CConfigFileNode *pNode=i->second;
            m_mNodes.erase(i);
            delete pNode;
        }
    }
}

// IPersistencyNode
void CConfigFileNode::Clear()
{
    m_mData.clear();
    std::multimap<std::tstring,CConfigFileNode*>::iterator i;
    for(i=m_mNodes.begin();i!=m_mNodes.end();i++)
    {
        CConfigFileNode *pNode=i->second;
        delete pNode;
    }
    m_mNodes.clear();
}

bool CConfigFileNode::AddProperty(SPersistencyProperty prop){m_mData[prop.name]=prop.value;return true;}
bool CConfigFileNode::GetProperty(SPersistencyProperty *pProp)
{
    std::map<std::tstring,std::tstring>::iterator i=m_mData.find(pProp->name);
    if(i==m_mData.end()){return false;}
    pProp->value=i->second;
    return true;
}
bool CConfigFileNode::RemoveProperty(SPersistencyProperty prop)
{
    std::map<std::tstring,std::tstring>::iterator i=m_mData.find(prop.name);
    if(i==m_mData.end()){return false;}
    m_mData.erase(i);
    return true;
}


CConfigFile::CConfigFile(void)
{
    m_dwSaveTabCount=0;
    m_RootNode.m_sName=_T("*ROOT*");
    m_hFile=INVALID_HANDLE_VALUE;
    m_pBuffer=NULL;
}

CConfigFile::~CConfigFile(void)
{
}


bool CConfigFile::Open(std::tstring sFileName)
{
    DWORD dwFileLength=0;
    
    m_RootNode.Clear();

    m_hFile=CreateFile(sFileName.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(m_hFile==INVALID_HANDLE_VALUE){return false;}

    dwFileLength=GetFileSize(m_hFile,NULL);
    if(!dwFileLength){return false;}
    
    DWORD  dwRead=0;
    m_pBuffer=new TCHAR [dwFileLength+1];
    ReadFile(m_hFile,m_pBuffer,dwFileLength,&dwRead,NULL);
    m_pBuffer[dwFileLength]=0;

    // Se leen los nodos del archivo
    CConfigFileNode *pCurrentNode=&m_RootNode;
    std::stack<CConfigFileNode*> sNodeStack;
    sNodeStack.push(pCurrentNode);

    // usando pSearchBuffer podemos poner pToken a cero para seguir la busqueda en otro punto
    TCHAR *nextToken=NULL;
    TCHAR *pToken=NULL;
    TCHAR *pSearchBuffer=m_pBuffer; 
    while(pToken = _tcstok_s(pToken?NULL:pSearchBuffer, CONFIG_FILE_DELIMITER, &nextToken))
    {
        if(pToken[0] ==_T('*') && _tcschr(CONFIG_FILE_DELIMITER,pToken[1])==NULL)
        {
            std::tstring sToken=pToken+1;
            CConfigFileNode *pNode=new CConfigFileNode;
            pNode->m_sName=sToken; // se esquiva el asterisco de bloque
            pCurrentNode->m_mNodes.insert(std::pair<std::tstring,CConfigFileNode*>(sToken,pNode));
            pCurrentNode=pNode;
        }
        else if(pToken[0]==_T('{'))
        {
            sNodeStack.push(pCurrentNode);
        }
        else if(pToken[0]==_T('}'))
        {
            sNodeStack.pop();
            pCurrentNode=sNodeStack.top();
        }
        else
        {
            int nLen=(int)_tcslen(pToken);
            int pos=nLen;
            // Se esquivan los espacios y el igual entre el nombre del dato y el valor
            while (_tcschr(_T("\t ="), pToken[pos])){ pos++; }
            TCHAR *pBuf=NULL;
            // Los strings y los demas valores se tratan de diferente manera
            if(pToken[pos]==_T('"'))
            {
                pos++;
                if (_tcschr(_T("\"\r\n"), pToken[pos])) // esquivar cadenas vacias o no terminadas
                {
                    pos++;
                }
                else
                {
                    pBuf = _tcstok_s(pToken + pos, _T("\"\r\n"), &nextToken);
                }
            }
            else
            {
                if (_tcschr(_T("\r\n"), pToken[pos])) // esquivar valores vacios
                {
                    pos++;
                }
                else
                {
                    pBuf = _tcstok_s(pToken + pos, _T("\t\r\n "), &nextToken);
                }
            }
            if(pBuf)
            {
                pSearchBuffer=pBuf+_tcslen(pBuf)+1;
                pCurrentNode->m_mData[pToken]=pBuf;
            }
            else
            {
                pSearchBuffer=pToken+pos;
                pCurrentNode->m_mData[pToken]=_T("");
            }
        }
    }
    if(m_pBuffer){delete [] m_pBuffer;m_pBuffer=NULL;}
    CloseHandle(m_hFile);
    m_hFile=INVALID_HANDLE_VALUE;
    m_pBuffer=NULL;
    return true;
}

bool CConfigFile::Save(std::tstring sFileName)
{
    m_hFile=CreateFile(sFileName.c_str(),GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(m_hFile!=INVALID_HANDLE_VALUE)
    {
        m_dwSaveTabCount=0;
        SaveNode(&m_RootNode);
        CloseHandle(m_hFile);
        m_hFile=INVALID_HANDLE_VALUE;
        return true;
    }
    return false;
}

void CConfigFile::SaveNode(CConfigFileNode *pNode)
{
    if(pNode!=&m_RootNode)
    {
        SaveBeginSection(pNode->m_sName);
        std::map<std::tstring,std::tstring>::iterator iData;
        for(iData=pNode->m_mData.begin();iData!=pNode->m_mData.end();iData++)
        {
            SaveValue(iData->first,iData->second);
        }    
    }
    std::multimap<std::tstring,CConfigFileNode*>::iterator iChild;
    for(iChild=pNode->m_mNodes.begin();iChild!=pNode->m_mNodes.end();iChild++)
    {
        CConfigFileNode *pChild=iChild->second;
        SaveNode(pChild);
    }
    if(pNode!=&m_RootNode)
    {
        SaveEndSection();
    }
}

void CConfigFile::SaveLine(std::tstring sValue)
{
    DWORD dwWritten=0;
    TCHAR sTemp[512]={0};
#if defined(_UNICODE)
    wmemset(sTemp, _T('\t'), m_dwSaveTabCount);
#else
    memset(sTemp, _T('\t'), m_dwSaveTabCount);
#endif
    _tcscat_s(sTemp, _countof(sTemp), sValue.c_str());
    _tcscat_s(sTemp, _countof(sTemp), _T("\r\n"));
    WriteFile(m_hFile, sTemp, (DWORD)_tcslen(sTemp)*sizeof(TCHAR), &dwWritten, NULL);
}

void CConfigFile::SaveValue(std::tstring sName, std::tstring sValue){ TCHAR sTemp[512] = { 0 }; _stprintf_s(sTemp, _T("%s=\"%s\""), sName.c_str(), sValue.c_str()); SaveLine(sTemp); }

void CConfigFile::SaveBeginSection(std::tstring sName)
{
    SaveLine((std::tstring)_T("*")+sName);
    SaveLine(_T("{"));
    m_dwSaveTabCount++;
}

void CConfigFile::SaveEndSection()
{
    m_dwSaveTabCount--;
    SaveLine(_T("}"));
}

IPersistencyNode *CConfigFile::GetRoot(){return &m_RootNode;}
IPersistencyNode *CConfigFile::GetNode(std::tstring id){return m_RootNode.GetNode(id);}
IPersistencyNode *CConfigFile::AddNode(std::tstring id){return m_RootNode.AddNode(id);}
