// TraceExample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "WPPGuid.h"
#include "TraceExample.tmh"

int _tmain()
{
    WPP_INIT_TRACING( L"TraceExample" );

    DoTraceMessage(TRACE_DEBUG, _T("Test wide string message %S"), _T("string"));
    DoTraceMessage(TRACE_DEBUG, "Test narrow string message %s", "string");

    DoTraceMessage(TRACE_DEBUG, _T("This is a number %d"), 1);

    //DoTraceMessage(TRACE_DEBUG, _T("Test special type IPADDR %!iPADDR!"), 1);
    DoTraceMessage(TRACE_DEBUG, _T("Test special type PORT %!PORT!"), 1);
    DoTraceMessage(TRACE_DEBUG, _T("Test special type STATUS %!STATUS!"), 1);
    DoTraceMessage(TRACE_DEBUG, _T("Test special type WINERROR %!WINERROR!"), 1);
    DoTraceMessage(TRACE_DEBUG, _T("Test special type HRESULT %!HRESULT!"), (HRESULT)1);
    DoTraceMessage(TRACE_DEBUG, _T("Test special type NDIS_STATUS %!NDIS_STATUS!"), 1);

    WPP_CLEANUP();
    return 0;
}

