#pragma once

// WPP stuff

//CCFAFF9A-C537-4F81-81C3-9CC0F3792823
#ifndef WPP_CONTROL_GUIDS
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(ETV_TEST_WPP_GUID,(CCFAFF9A,C537,4F81,81C3,9CC0F3792823), \
        WPP_DEFINE_BIT(TRACE_DEBUG) )
#endif //WPP_CONTROL_GUIDS

// allow all non-metro platforms to use the global logger
#ifdef WINAPI_FAMILY
#   define ALLOW_GLOBALLOGGER WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#else
#   define ALLOW_GLOBALLOGGER 1
#endif

//adding this define if you want to get boot time log
#if ((!defined WPP_GLOBALLOGGER) && (1 == ALLOW_GLOBALLOGGER))
#   define WPP_GLOBALLOGGER 
#endif

