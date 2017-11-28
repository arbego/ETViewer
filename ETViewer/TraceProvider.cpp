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
#include ".\traceprovider.h"

#include "cvconst.h"
#include "Strsafe.h"

#include "Dbghelp.h"

using namespace std;

#define PARAMETER_INDEX_BASE 10

struct STypeEnumerationData
{
    bool			b64BitPDB;
    HANDLE          hProcess;
    DWORD64         nBaseAddress;
    CTracePDBReader *pThis;
    STypeEnumerationData(){pThis=NULL;hProcess=NULL;b64BitPDB=false;nBaseAddress=0;}
};

struct SSymbolEnumerationData
{
    bool			b64BitPDB;
    CTracePDBReader *pThis;
    SSymbolEnumerationData(){pThis=NULL;b64BitPDB=false;}
};

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//							STraceFormatEntry
//
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


typedef BOOL (WINAPI *pfSymSearch)(HANDLE hProcess,ULONG64 BaseOfDll,DWORD Index,DWORD SymTag,PCTSTR Mask,DWORD64 Address,PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,PVOID UserContext,DWORD Options);
typedef BOOL (WINAPI *pfSymEnumTypes)(HANDLE hProcess,ULONG64 BaseOfDll,PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,PVOID UserContext);

eTraceFormatElementType GetElementTypeFromString(TCHAR *pString,bool b64BitPDB)
{
    if(_tcscmp(pString,_T("ItemString"))==0){return eTraceFormatElementType_AnsiString;}
    if(_tcscmp(pString,_T("ItemWString"))==0){return eTraceFormatElementType_UnicodeString;}
    if(_tcscmp(pString,_T("ItemLong"))==0){return eTraceFormatElementType_DWord;}
    if(_tcscmp(pString,_T("ItemChar"))==0){return eTraceFormatElementType_Byte;}
    if(_tcscmp(pString,_T("ItemDouble"))==0){return eTraceFormatElementType_Double;}
    if(_tcscmp(pString,_T("ItemFloat"))==0){return eTraceFormatElementType_Float;}
    if(_tcscmp(pString,_T("ItemLongLong"))==0){return eTraceFormatElementType_Quad;}
    if(_tcscmp(pString,_T("ItemULongLong"))==0){return eTraceFormatElementType_Quad;}
    if(_tcscmp(pString,_T("ItemLongLongX"))==0){return eTraceFormatElementType_Quad;}
    if(_tcscmp(pString,_T("ItemLongLongXX"))==0){return eTraceFormatElementType_Quad;}
    if(_tcscmp(pString,_T("ItemULongLongX"))==0){return eTraceFormatElementType_Quad;}
    if(_tcscmp(pString,_T("ItemULongLongXX"))==0){return eTraceFormatElementType_Quad;}
    if(_tcscmp(pString,_T("ItemPtr"))==0){return b64BitPDB?eTraceFormatElementType_QuadPointer:eTraceFormatElementType_Pointer;}
    return eTraceFormatElementType_Unknown;

    /*
    Types that could use handlers
    ItemIPAddr
    ItemPort
    ItemNTSTATUS
    ItemWINERROR
    ItemHRESULT
    ItemNDIS_STATUS
    */
}

STraceFormatEntry::STraceFormatEntry()
{
    m_dwIndex=0;
    m_SourceFileGUID=GUID_NULL;
    m_pSourceFile=NULL;
    m_pProvider=NULL;
}

STraceFormatEntry::~STraceFormatEntry()
{
    unsigned x;
    for(x=0;x<m_vElements.size();x++)
    {
        TCHAR *pFormat=m_vElements[x].pFormatString;
        delete [] pFormat;
    }
    m_vElements.clear();
    m_dwIndex=0;
}

void STraceFormatEntry::InitializeFromBuffer(TCHAR *pBuffer)
{
    int len=(int)_tcslen(pBuffer),tempLen=0;
    TCHAR *pTempBuffer=new TCHAR [len+1];
    int x;
    for(x=0;x<len;)
    {
        if(pBuffer[x]==_T('%') && pBuffer[x+1]!=_T('%'))
        {
            int nParameterIndexLen=0;
            TCHAR sParamIndex[100]={0};

            if(tempLen)
            {
                pTempBuffer[tempLen]=0;

                STraceFormatElement element;
                element.eType=eTraceFormatElementType_ConstantString;
                element.pFormatString=new TCHAR [tempLen+1];
                _tcscpy_s(element.pFormatString,tempLen+1,pTempBuffer);	
                m_vElements.push_back(element);
                tempLen=0;
            }

            STraceFormatElement element;

            //Copy initial '%' TCHARacter
            pTempBuffer[tempLen]=pBuffer[x];x++;tempLen++;
            // Read parameter index.
            while(pBuffer[x]>=_T('0') && pBuffer[x]<=_T('9'))
            {
                sParamIndex[nParameterIndexLen]=pBuffer[x];
                nParameterIndexLen++;
                x++;
            }

            x++; //Skip initial '!' TCHARacter
            while(pBuffer[x]!=_T('!'))
            {
                pTempBuffer[tempLen]=pBuffer[x];
                x++;tempLen++;
            }
            x++; //Skip final '!' TCHARacter
            pTempBuffer[tempLen]=0;
            if(tempLen)
            {
                int nParameterIndex=_ttoi(sParamIndex)-PARAMETER_INDEX_BASE;

                STraceFormatElement element;
                if(nParameterIndex>=0 && nParameterIndex<(int)m_vParams.size())
                {
                    element.eType=m_vParams[nParameterIndex].eType;
                }
                pTempBuffer[tempLen]=0;
                element.pFormatString=new TCHAR [tempLen+1];
                _tcscpy_s(element.pFormatString,tempLen+1,pTempBuffer);	

                if(tempLen)
                {
                    // Modify the format buffers if needed. 

                    //ETW formats 'doubles' as %s instead of '%f'.
                    if(element.eType==eTraceFormatElementType_Float || 
                        element.eType==eTraceFormatElementType_Double)
                    {
                        if(element.pFormatString[tempLen-1]==_T('s')){element.pFormatString[tempLen-1]=_T('f');}
                    }
                    // 64 bit %p must be translated to %I64X
                    if(element.eType==eTraceFormatElementType_QuadPointer) 
                    {
                        if(element.pFormatString[tempLen-1]==_T('p'))
                        {
                            TCHAR *pOld=element.pFormatString;
                            element.pFormatString=new TCHAR [tempLen+6+1];
                            _tcscpy_s(element.pFormatString,tempLen+6+1,pOld);
                            _tcscpy_s(element.pFormatString+(tempLen-1),8,_T("016I64X"));
                            delete [] pOld;
                        }
                    }
                    if(element.eType==eTraceFormatElementType_Unknown) 
                    {
                        if(element.pFormatString[tempLen-1]==_T('s'))
                        {
                            TCHAR *pOld=element.pFormatString;
                            element.pFormatString=new TCHAR [8+1];
                            _tcscpy_s(element.pFormatString,9,_T("(0x%08x)"));
                            delete [] pOld;
                        }
                    }
                }

                m_vElements.push_back(element);
                pTempBuffer[0]=0;
                tempLen=0;
            }
        }
        else
        {
            pTempBuffer[tempLen]=pBuffer[x];
            tempLen++;
            if(pBuffer[x]==_T('%') && pBuffer[x+1]==_T('%'))
            {
                x++;
            }
            x++;
        }
    }
    if(tempLen)
    {
        pTempBuffer[tempLen]=0;

        STraceFormatElement element;
        element.eType=eTraceFormatElementType_ConstantString;
        element.pFormatString=new TCHAR [tempLen+1];
        _tcscpy_s(element.pFormatString,tempLen+1,pTempBuffer);	
        m_vElements.push_back(element);
        tempLen=0;
    }
    delete [] pTempBuffer;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//							CTraceSourceFile
//
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


CTraceSourceFile::CTraceSourceFile(GUID sourceFileGUID, const TCHAR *pSourceFile)
{
    m_pProvider=NULL;
    m_SourceFileGUID=sourceFileGUID;
    _tcscpy_s(m_SourceFileNameWithPath, pSourceFile);
    m_SourceFileName = _tcsrchr(m_SourceFileNameWithPath, _T('\\'))+1;
}

CTraceSourceFile::~CTraceSourceFile(void)
{
}

GUID CTraceSourceFile::GetGUID()
{
    return m_SourceFileGUID;
}

tstring CTraceSourceFile::GetFileName()
{
    return m_SourceFileName;
}

tstring CTraceSourceFile::GetFileNameWithPath()
{
    return m_SourceFileNameWithPath;
}

CTraceProvider* CTraceSourceFile::GetProvider()
{
    return m_pProvider;
}

void CTraceSourceFile::SetProvider(CTraceProvider *pProvider)
{
    m_pProvider=pProvider;
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//							CTraceProvider
//
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


CTraceProvider::CTraceProvider(tstring sComponentName,tstring sFileName)
{
    m_sComponentName=sComponentName;
    m_sFileList.insert(sFileName);
    m_dwAllSupportedFlagsMask=0;
    m_ProviderGUID=GUID_NULL;
}

CTraceProvider::~CTraceProvider(void)
{
    FreeAll();
}

void CTraceProvider::FreeAll(void)
{
    unsigned x;

    map<GUID,CTraceSourceFile *,CGUIDComparer>::iterator i;
    for(i=m_sSourceFiles.begin();i!=m_sSourceFiles.end();i++)
    {
        CTraceSourceFile *pSource=i->second;
        delete pSource;
    }
    m_sSourceFiles.clear();

    for(x=0;x<m_FormatEntries.size();x++)
    {
        STraceFormatEntry *pFormatEntry=m_FormatEntries[x];
        delete pFormatEntry;
    }
    m_FormatEntries.clear();
}

GUID CTraceProvider::GetGUID()
{
    return m_ProviderGUID;
}

void CTraceProvider::SetGUID(GUID guid)
{
    m_ProviderGUID=guid;
}

tstring CTraceProvider::GetComponentName()
{
    return m_sComponentName;
}

std::set<std::tstring>	CTraceProvider::GetFileList()
{
    return m_sFileList;
}

void CTraceProvider::AddFileName(std::tstring sFileName)
{
    m_sFileList.insert(sFileName);
}

void CTraceProvider::AddSourceFile(CTraceSourceFile *pSourceFile)
{
    m_sSourceFiles[pSourceFile->GetGUID()]=pSourceFile;
}

void CTraceProvider::GetSourceFiles(vector<CTraceSourceFile*> *pvSources)
{
    map<GUID,CTraceSourceFile *,CGUIDComparer>::iterator i;
    for(i=m_sSourceFiles.begin();i!=m_sSourceFiles.end();i++)
    {
        pvSources->push_back(i->second);
    }
}

CTraceSourceFile* CTraceProvider::GetSourceFile(GUID sourceGUID)
{
    map<GUID,CTraceSourceFile *,CGUIDComparer>::iterator i;
    i=m_sSourceFiles.find(sourceGUID);
    if(i!=m_sSourceFiles.end()){return i->second;}
    return 0;
}

void CTraceProvider::AddFormatEntry(STraceFormatEntry *pFormatEntry)
{
    m_FormatEntries.push_back(pFormatEntry);
}

void CTraceProvider::GetFormatEntries(vector<STraceFormatEntry*> *pvFormatEntries)
{
    unsigned x;
    for(x=0;x<m_FormatEntries.size();x++)
    {
        STraceFormatEntry *pFormatEntry=m_FormatEntries[x];
        pvFormatEntries->push_back(pFormatEntry);
    }
}

DWORD CTraceProvider::GetAllSupportedFlagsMask(){return m_dwAllSupportedFlagsMask;}
void  CTraceProvider::GetSupportedFlags(map<tstring,DWORD> *pmFlags){*pmFlags=m_TraceFlags;}
void  CTraceProvider::AddSupportedFlag(tstring sName,DWORD dwValue)
{
    m_TraceFlags[sName]=dwValue;
    m_dwAllSupportedFlagsMask|=dwValue;
}
DWORD CTraceProvider::GetSupportedFlagValue(tstring sName)
{
    map<tstring,DWORD>::iterator i;
    i=m_TraceFlags.find(sName);
    if(i!=m_TraceFlags.end()){return i->second;}
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//
//							CTraceReader
//
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

CTraceProvider *CTraceReader::FindOrCreateProvider(tstring sProviderName)
{
    map<tstring,CTraceProvider*>::iterator iProvider;

    iProvider=m_sTempProviders.find(sProviderName);
    if(iProvider!=m_sTempProviders.end())
    {
        return iProvider->second;
    }
    else
    {
        CTraceProvider *pProvider=new CTraceProvider(sProviderName,m_sFileName);
        m_sTempProviders[sProviderName]=pProvider;
        return pProvider;
    }
}
CTraceSourceFile *CTraceReader::FindSourceFile(GUID sourceFile)
{
    map<GUID,CTraceSourceFile*,CGUIDComparer>::iterator iSourceFile;

    iSourceFile=m_sTempSourceFiles.find(sourceFile);
    if(iSourceFile!=m_sTempSourceFiles.end())
    {
        return iSourceFile->second;
    }
    return NULL;
}

CTraceProvider *CTraceReader::FindProviderByFlag(tstring sFlagName)
{
    map<tstring,CTraceProvider*>::iterator iProvider;

    for(iProvider=m_sTempProviders.begin();iProvider!=m_sTempProviders.end();iProvider++)
    {
        CTraceProvider *pProvider=iProvider->second;
        if(pProvider->GetSupportedFlagValue(sFlagName))
        {
            return pProvider;
        }
    }
    return NULL;
}

void CTraceReader::AddSourceFile(GUID guid,tstring sFileName)
{
    map<GUID,CTraceSourceFile*,CGUIDComparer>::iterator iSourceFile;

    iSourceFile=m_sTempSourceFiles.find(guid);
    if(iSourceFile==m_sTempSourceFiles.end())
    {
        CTraceSourceFile *pSourceFile=new CTraceSourceFile(guid,sFileName.c_str());
        m_sTempSourceFiles[guid]=pSourceFile;
    }
}

eTraceReaderError CTracePDBReader::LoadFromPDB(LPCTSTR pPDB,vector<CTraceProvider *> *pvProviders)
{
    eTraceReaderError eResult=eTraceReaderError_Generic;

    m_sFileName=pPDB;

    pfSymSearch pSymSearch=NULL;
    pfSymEnumTypes pSymEnumTypes=NULL;

    HANDLE hFile=CreateFile(pPDB,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile==INVALID_HANDLE_VALUE)
    {
        DWORD dwResult=GetLastError();
        return eTraceReaderError_PDBNotFound;
    }
    DWORD dwFileSize=GetFileSize(hFile,NULL);
    CloseHandle(hFile);
    hFile=NULL;

    HMODULE hDbgHelp=LoadLibrary(_T("dbghelp.dll"));
    if(hDbgHelp)
    {
#ifdef UNICODE
        pSymSearch = (pfSymSearch)GetProcAddress(hDbgHelp, "SymSearchW");
        pSymEnumTypes = (pfSymEnumTypes)GetProcAddress(hDbgHelp, "SymEnumTypesW");
#else
        pSymSearch = (pfSymSearch)GetProcAddress(hDbgHelp, "SymSearch");
        pSymEnumTypes = (pfSymEnumTypes)GetProcAddress(hDbgHelp, "SymEnumTypes");
#endif

        if(pSymSearch && pSymEnumTypes)
        {
            VOID *pVirtual=VirtualAlloc(NULL,dwFileSize,MEM_RESERVE,PAGE_READWRITE);
            if(pVirtual)
            {
                HANDLE hProcess=GetCurrentProcess();
                BOOL bOk=SymInitialize(hProcess,_T(""),FALSE);
                if(bOk)
                {
                    DWORD64 dwModuleBase = SymLoadModuleEx(hProcess, NULL, pPDB, const_cast<LPCTSTR>(pPDB), (DWORD64)pVirtual, dwFileSize, NULL, 0);
                    if(dwModuleBase)
                    {

                        STypeEnumerationData typeData;
                        typeData.b64BitPDB=false;
                        typeData.hProcess=hProcess;
                        typeData.nBaseAddress=dwModuleBase;
                        typeData.pThis=this;

                        // Temporal solution to 64 bit %p parameters.
                        // %p parameters are identified as ItemPtr in both 32 and 64 bit PDBs
                        // We need to distinguish this two cases.
                        // For the time being we have no clean solution.
                        // The current solution is to find some well known pointer 
                        // types such as PVOID and check their sizes.

                        // Enum types to identify 64 bit PDBs
                        SymEnumTypes(hProcess,dwModuleBase,TypeEnumerationCallback,&typeData);
                        
                        SSymbolEnumerationData symbolData;
                        symbolData.b64BitPDB=typeData.b64BitPDB;
                        symbolData.pThis=this;

                        // 8 = SymTagEnum::SymTagAnnotation
                        if(!pSymSearch(hProcess,dwModuleBase,0,8,0,0,SymbolEnumerationCallback,&symbolData,2))
                        {
                            eResult=eTraceReaderError_FailedToEnumerateSymbols;
                        }
                        else if(m_sTempProviders.size())
                        {
                            eResult=eTraceReaderError_Success;
                        }
                        else
                        {
                            eResult=eTraceReaderError_NoProviderFoundInPDB;
                        }
                        SymUnloadModule64(hProcess,dwModuleBase);
                    }
                    else
                    {
                        eResult=eTraceReaderError_SymEngineLoadModule;
                    }
                    SymCleanup(hProcess);
                }
                else
                {
                    eResult=eTraceReaderError_SymEngineInitFailed;
                }

                VirtualFree(pVirtual,0,MEM_RELEASE);
                pVirtual=NULL;
            }
            else
            {
                eResult=eTraceReaderError_NoMemory;
            }
        }
        else
        {
            eResult=eTraceReaderError_DBGHelpWrongVersion;
        }
        FreeLibrary(hDbgHelp);
        hDbgHelp=NULL;
    }
    else
    {
        eResult=eTraceReaderError_DBGHelpNotFound;
    }

    if(eResult!=eTraceReaderError_Success)
    {
        map<tstring,CTraceProvider*>::iterator iProvider;

        for(iProvider=m_sTempProviders.begin();iProvider!=m_sTempProviders.end();iProvider++)
        {
            CTraceProvider *pProvider=iProvider->second;
            delete pProvider;
        }
        m_sTempProviders.clear();
    }
    else
    {
        map<tstring,CTraceProvider*>::iterator iSource;

        for(iSource=m_sTempProviders.begin();iSource!=m_sTempProviders.end();iSource++)
        {
            pvProviders->push_back(iSource->second);
        }		
    }

    unsigned x;

    // Classification and initialization of all read format entries

    for(x=0;x<m_sTempFormatEntries.size();x++)
    {
        STraceFormatEntry *pFormatEntry=m_sTempFormatEntries[x];

        // Search the provider by flag string.

        pFormatEntry->m_pProvider=FindProviderByFlag(pFormatEntry->m_sFlag);
        if(!pFormatEntry->m_pProvider)
        {
            delete pFormatEntry;
            continue;
        }
    
        CTraceSourceFile *pSourceFile=pFormatEntry->m_pProvider->GetSourceFile(pFormatEntry->m_SourceFileGUID);
        if(!pSourceFile)
        {
            // Find the read source and use it as a template to create a new CTraceSource instance,
            // this is done this way because each provider will hace its own source file instance, and 
            // two different providers can trace from the same source file.

            CTraceSourceFile *pSourceFileTemplate=FindSourceFile(pFormatEntry->m_SourceFileGUID);
            if(!pSourceFileTemplate)
            {
                delete pFormatEntry;
                continue;
            }

            pSourceFile=new CTraceSourceFile(pSourceFileTemplate->GetGUID(),pSourceFileTemplate->GetFileNameWithPath().c_str());
            pFormatEntry->m_pProvider->AddSourceFile(pSourceFile);
            pSourceFile->SetProvider(pFormatEntry->m_pProvider);
        }

        pFormatEntry->m_pSourceFile=pSourceFile;
        pFormatEntry->m_pProvider->AddFormatEntry(pFormatEntry);

    }

    // Free the source file instances used as templates

    map<GUID,CTraceSourceFile*,CGUIDComparer>::iterator iProvider;
    for(iProvider=m_sTempSourceFiles.begin();iProvider!=m_sTempSourceFiles.end();iProvider++)
    {
        CTraceSourceFile *pSource=iProvider->second;
        delete pSource;
    }
    m_sTempSourceFiles.clear();
    m_sTempFormatEntries.clear();
    m_sTempProviders.clear();
    return eResult;
}

BOOL CALLBACK CTracePDBReader::TypeEnumerationCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    if(pSymInfo->Name==NULL){return TRUE;}
    if(pSymInfo->Tag!=17 || pSymInfo->Size!=8){return TRUE;}

    STypeEnumerationData *pData=(STypeEnumerationData *)UserContext;
    CTracePDBReader *pThis=pData->pThis;

    if( _tcscmp(pSymInfo->Name, _T("HANDLE"))==0 ||
        _tcscmp(pSymInfo->Name, _T("PBYTE")) == 0 ||
        _tcscmp(pSymInfo->Name, _T("PVOID")) == 0 ||
        _tcscmp(pSymInfo->Name, _T("UINT_PTR")) == 0 ||
        _tcscmp(pSymInfo->Name, _T("ULONG_PTR")) == 0)
    {
        pData->b64BitPDB=true;
        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK CTracePDBReader::SymbolEnumerationCallback(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    SSymbolEnumerationData *pData=(SSymbolEnumerationData*)UserContext;
    CTracePDBReader *pThis=pData->pThis;

    LPTSTR pTemp = pSymInfo->Name;
    LPTSTR pCompName=NULL;
    LPTSTR pGUIDName=NULL;
    BOOL   bOK=FALSE;

    // 8 = SymTagAnnotation
    if(pSymInfo->Tag==SymTagAnnotation)
    {
        if(_tcsncmp(pSymInfo->Name,_T("TMC:"),4)==0)
        {
            GUID providerGUID=GUID_NULL;
            pTemp=pTemp+_tcslen(pTemp)+1;
            pGUIDName=pTemp;
            pTemp=pTemp+_tcslen(pTemp)+1;
            pCompName=pTemp;
            pTemp=pTemp+_tcslen(pTemp)+1;

            WCHAR wsCLSID[100]={0};
#ifdef UNICODE
            StringCbPrintfW(wsCLSID, 100, L"{%s}", pGUIDName);
#else
            StringCbPrintfW(wsCLSID, 100, L"{%S}", pGUIDName);
#endif
            CLSIDFromString(wsCLSID,&providerGUID);
            
            CTraceProvider *pProvider=pThis->FindOrCreateProvider(pCompName);

            pProvider->SetGUID(providerGUID);

            DWORD dwTraceValue=1;

            while(((ULONG)(pTemp-pSymInfo->Name))<pSymInfo->NameLen)
            {
                pProvider->AddSupportedFlag(pTemp,dwTraceValue);
                pTemp=pTemp+_tcslen(pTemp)+1;
                dwTraceValue<<=1;
            }

        }
        if(_tcsncmp(pSymInfo->Name,_T("TMF:"),4)==0 && pSymInfo->NameLen)
        {
            tstring sCompName;
            TCHAR sTraceLevel[MAX_PATH]={0};
            TCHAR sTraceFlag[MAX_PATH]={0};
            GUID sSourceFileGUID=GUID_NULL;
            DWORD dwTraceId=0;
            LPTSTR pFormatString=new TCHAR[pSymInfo->NameLen];
            pFormatString[0]=0;

            SYMBOL_INFO *pFunctionSymbol=(SYMBOL_INFO *)new char[sizeof(SYMBOL_INFO)+(1024*sizeof(TCHAR))];
            memset(pFunctionSymbol, 0, sizeof(SYMBOL_INFO)+(1024*sizeof(TCHAR)));
            pFunctionSymbol->SizeOfStruct=sizeof(SYMBOL_INFO);
            pFunctionSymbol->MaxNameLen=1024;

            DWORD64 dwFunctionDisplacement=0;
            BOOL bFuncionOk=SymFromAddr(GetCurrentProcess(),pSymInfo->Address,&dwFunctionDisplacement,pFunctionSymbol);

            DWORD dwLineDisplacement=0;
            IMAGEHLP_LINE64 lineInfo={0};
            lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            BOOL bLineOk=SymGetLineFromAddr64(GetCurrentProcess(),pSymInfo->Address,&dwLineDisplacement,&lineInfo);

            STraceFormatEntry *pFormatEntry=new STraceFormatEntry;

            int stringIndex=0;
            pTemp=pTemp+_tcslen(pTemp)+1;
            while(((ULONG)(pTemp - pSymInfo->Name)) < pSymInfo->NameLen)
            {
                int nTempLen=(int)_tcslen(pTemp);
                TCHAR *pFullString=new TCHAR [nTempLen+1];
                _tcscpy_s(pFullString,nTempLen+1,pTemp);

                //OutputDebugString(pTemp);
                //OutputDebugString("\n");

                if(stringIndex==0) // <GUID> <module> // SRC=<source>
                {
                    int tokenindex=0;
                    TCHAR *nextToken=NULL;
                    TCHAR *pToken=_tcstok_s(pFullString,_T(" "),&nextToken);
                    if(pToken)
                    {
                        WCHAR wsCLSID[100]={0};
#ifdef UNICODE
                        StringCbPrintfW(wsCLSID, 100, L"{%s}", pToken);
#else
                        StringCbPrintfW(wsCLSID, 100, L"{%S}", pToken);
#endif
                        CLSIDFromString(wsCLSID,&sSourceFileGUID);
                        pToken=_tcstok_s(NULL,_T(" "),&nextToken);
                    }
                    if(pToken)
                    {
                        sCompName=pToken;
                    }
                }
                else if(stringIndex==1) // #typev <source:line> index "<format>" // LEVEL=<level> FLAGS=<flag>
                {	
                    int startIndex=-1,endIndex=-1;

                    // Search format string limits
                    for(int x=0;pFullString[x]!=0;x++)
                    {
                        if(pFullString[x]==_T('"'))
                        {
                            if(startIndex==-1)
                            {
                                startIndex=x+1;
                            }
                            endIndex=x;
                        }					
                    }
                    // Format string is copied to another buffer and cleaned from the original buffer (through memset).
                    // This is done to simplify string parsing.

                    if(startIndex<endIndex)
                    {
                        if((endIndex-startIndex)>=2 && pFullString[startIndex]==_T('%') && pFullString[startIndex+1]==_T('0'))
                        {
                            startIndex+=2;
                        }
                        _tcsncpy_s(pFormatString, pSymInfo->NameLen, pFullString + startIndex, endIndex - startIndex);
                        //memcpy(pFormatString,pFullString+startIndex,(endIndex-startIndex)*sizeof(TCHAR));
                        //pFormatString[endIndex-startIndex]=0;

                        #if defined(_UNICODE)
                            wmemset(pFullString+startIndex-2,_T(' '),((endIndex-startIndex)+2));
                        #else
                            memset(pFullString+startIndex-2,_T(' '),((endIndex-startIndex)+2));
                        #endif
                    }

                    bool bFlagFound=false;

                    // Search for the remaining useful tokens.
                    int tokenIndex=0;
                    TCHAR *nextToken = NULL;
                    TCHAR *pToken=_tcstok_s(pFullString,_T(" "),&nextToken);
                    while(pToken)
                    {
                        if(tokenIndex==2)
                        {
                            dwTraceId=_ttoi(pToken);
                        }
                        else if(_tcsncmp(pToken, _T("LEVEL="), 6) == 0)
                        {
                            _tcscpy_s(sTraceLevel,pToken+6);
                        }
                        else if(_tcsncmp(pToken,_T("FLAGS="),6)==0)
                        {
                            bFlagFound=true;
                            _tcscpy_s(sTraceFlag,pToken+6);
                        }
                        pToken=_tcstok_s(NULL,_T(" "),&nextToken);
                        tokenIndex++;
                    }
                    if(!bFlagFound)
                    {
                        _tcscpy_s(sTraceFlag,sTraceLevel);
                        _tcscpy_s(sTraceLevel,_T("TRACE_LEVEL_NONE"));
                    }
                }
                else if(pTemp[0]!=_T('{') && pTemp[0]!=_T('}'))
                {
                    int x;
                    for(x=nTempLen-(int)_tcslen(_T(", Item"));x>0;x--)
                    {
                        if(_tcsncmp(pFullString+x,_T(", Item"),_tcslen(_T(", Item")))==0)
                        {
                            TCHAR sParamType[MAX_PATH]={0};
                            TCHAR sParamIndex[MAX_PATH]={0};
                            
                            LPTSTR nextToken = NULL;
                            LPTSTR pToken=_tcstok_s(pFullString+x,_T(" ,"),&nextToken);
                            if(pToken){_tcscpy_s(sParamType,pToken);pToken=_tcstok_s(NULL,_T(" ,"),&nextToken);}
                            if(pToken){pToken=_tcstok_s(NULL,_T(" ,"),&nextToken);}
                            if(pToken){_tcscpy_s(sParamIndex,pToken);}
                            STraceFormatParam parameter;

                            int nParamIndex=_ttoi(sParamIndex)-PARAMETER_INDEX_BASE;
                            if(nParamIndex>=0)
                            {
                                if((int)pFormatEntry->m_vParams.size()<nParamIndex+1)
                                {
                                    pFormatEntry->m_vParams.resize(nParamIndex+1);
                                }
                                parameter.eType=GetElementTypeFromString(sParamType,pData->b64BitPDB);
                                pFormatEntry->m_vParams[nParamIndex]=parameter;
                            }
                        }
                    }
                }
                pTemp=pTemp+nTempLen+1;
                stringIndex++;
                delete [] pFullString;
            }

            pFormatEntry->InitializeFromBuffer(pFormatString);
            pFormatEntry->m_SourceFileGUID=sSourceFileGUID;
            pFormatEntry->m_dwIndex=dwTraceId;
            pFormatEntry->m_dwLine=lineInfo.LineNumber;
            pFormatEntry->m_sFunction=pFunctionSymbol->Name;
            pFormatEntry->m_sLevel=sTraceLevel;
            pFormatEntry->m_sFlag=sTraceFlag;

            pThis->m_sTempFormatEntries.push_back(pFormatEntry);

            pThis->AddSourceFile(sSourceFileGUID,lineInfo.FileName);

            delete [] pFormatString;
            delete [] pFunctionSymbol;
        }
    }
    return TRUE;
}	