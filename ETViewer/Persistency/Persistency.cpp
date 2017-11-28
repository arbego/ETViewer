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

void _PersistencyDefaultValue(IPersistencyItem **ppiList,TCHAR *pPrefixName)
{
    int x=0;
    if(ppiList)
    {
        while(ppiList[x]!=NULL)
        {
            ppiList[x]->SetDefaultValue();
            x++;
        }
    }
}
HRESULT _PersistencySave(IPersistencyItem **ppiList,IPersistencyNode *piNode,TCHAR *pPrefixName)
{
    HRESULT hr=S_OK,finalhr=S_OK;
    if(pPrefixName==NULL){piNode->Clear();}
    int x=0;
    if(ppiList)
    {
        while(ppiList[x]!=NULL)
        {
            hr=ppiList[x]->Remove(piNode);
            hr=ppiList[x]->Save(piNode);
            if(FAILED(hr))
            {
                finalhr=hr;
            }
            x++;
        }
    }
    return finalhr;
} 
HRESULT _PersistencyLoad(IPersistencyItem **ppiList,IPersistencyNode *piNode,TCHAR *pPrefixName)
{
    HRESULT hr=S_OK,finalhr=S_OK;
    int x=0;
    if(ppiList)
    {
        while(ppiList[x]!=NULL)
        {
            hr=ppiList[x]->Load(piNode);
            if(FAILED(hr))
            {
                //RTTRACE("GameRunTimeLib::PersistencyLoad-> Failed To Load item %s, result 0x%08x",ppiList[x]->GetName(),hr);
                finalhr=hr;
            }
            x++;
        }
    }
    return finalhr;
}
HRESULT _PersistencyRemove(IPersistencyItem **ppiList,IPersistencyNode *piNode,TCHAR *pPrefixName)
{
    HRESULT hr=S_OK,finalhr=S_OK;
    int x=0;
    if(ppiList)
    {
        while(ppiList[x]!=NULL)
        {
            hr=ppiList[x]->Remove(piNode);
            if(FAILED(hr))
            {
                finalhr=hr;
            }
            x++;
        }
    }
    return finalhr;
}
void _PersistencyInitialize(IPersistencyItem **ppiList,TCHAR *pPrefixName)
{
    int x=0;
    if(ppiList)
    {
        while(ppiList[x]!=NULL)
        {
            ppiList[x]->Initialize();
            x++;
        }
    }
}
void _PersistencyFree(IPersistencyItem **ppiList,TCHAR *pPrefixName)
{
    int x=0;
    if(ppiList)
    {
        while(ppiList[x]!=NULL)
        {
            ppiList[x]->Free();
            x++;
        }
    }
}

void _FreePersistencyPropertyMap(IPersistencyItem ***ppiList)
{
    int x=0;
    if(ppiList)
    {
        while((*ppiList)[x]!=NULL){delete ((*ppiList)[x]);x++;}
        delete [] ((*ppiList));
    }
}