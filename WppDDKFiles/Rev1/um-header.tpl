`**********************************************************************`
`* This is an include template file for tracewpp preprocessor.        *`
`*                                                                    *`
`*    Copyright (c) Microsoft Corporation. All Rights Reserved.       *`
`**********************************************************************`

// template `TemplateFile`
#ifdef  WPP_THIS_FILE
// included twice
#       define  WPP_ALREADY_INCLUDED
#       undef   WPP_THIS_FILE
#endif  // #ifdef WPP_THIS_FILE

#define WPP_THIS_FILE `SourceFile.CanonicalName`

#ifndef WPP_ALREADY_INCLUDED

`* Dump defintions specified via -D on the command line to WPP *`

`FORALL def IN MacroDefinitions`
#define `def.Name` `def.Alias`
`ENDFOR`


#ifndef USER_MODE_NATIVE_ETW
#ifdef BUILD_MINWIN
#include <windowscore.h>
#else
#include <windows.h>
#endif
#endif

#include <wmistr.h>
#include <evntrace.h>
#ifndef WPP_TRACE_W2K_COMPATABILITY
#include <sddl.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef USER_MODE_NATIVE_ETW
#define WPP_USE_NTDLL_FUNCTIONS

typedef 
ULONG 
(__stdcall *WMIDPREQUEST)(
    _In_ WMIDPREQUESTCODE RequestCode,
    _In_ PVOID RequestContext,
    _Inout_ ULONG *BufferSize,
    _Inout_ PVOID Buffer
    );

typedef struct  _TRACE_GUID_REGISTRATION {
    LPCGUID Guid;            // Guid of data block being registered or updated.
    HANDLE RegHandle;        // Guid Registration Handle is returned.
} TRACE_GUID_REGISTRATION, *PTRACE_GUID_REGISTRATION;

#endif

// LEGACY: This structure was used by Win2k RpcRt4 and cluster tracing

typedef struct _WPP_WIN2K_CONTROL_BLOCK {
    TRACEHANDLE Logger;
    ULONG Flags;
    ULONG Level;
    LPCGUID ControlGuid;
} WPP_WIN2K_CONTROL_BLOCK, *PWPP_WIN2K_CONTROL_BLOCK;

#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used nameless struct/union

typedef struct _WPP_TRACE_CONTROL_BLOCK
{
    struct _WPP_TRACE_CONTROL_BLOCK *Next;
    TRACEHANDLE     UmRegistrationHandle;
    union {
        TRACEHANDLE              Logger;
        PWPP_WIN2K_CONTROL_BLOCK Win2kCb;
        PVOID                    Ptr;
        struct _WPP_TRACE_CONTROL_BLOCK *Cb;
    };
    
    UCHAR           FlagsLen; 
    UCHAR           Level; 
    USHORT          Options;
    ULONG           Flags[1];
    LPCGUID         ControlGuid;
} WPP_TRACE_CONTROL_BLOCK, *PWPP_TRACE_CONTROL_BLOCK;
#pragma warning(pop)

#ifdef WPP_USE_NTDLL_FUNCTIONS

ULONG
__stdcall
EtwTraceMessage(
    _In_ TRACEHANDLE LoggerHandle,
    _In_ ULONG MessageFlags,
    _In_ LPCGUID MessageGuid,
    _In_ USHORT MessageNumber,
    ...
    );

TRACEHANDLE
__stdcall
EtwGetTraceLoggerHandle(
    _In_ PVOID Buffer
    );

UCHAR
__stdcall
EtwGetTraceEnableLevel(
    _In_ TRACEHANDLE LoggerHandle
    );

ULONG
__stdcall
EtwGetTraceEnableFlags(
    _In_ TRACEHANDLE LoggerHandle
    );

ULONG 
__stdcall
EtwRegisterTraceGuidsW(
    _In_ WMIDPREQUEST RequestAddress,
    _In_ PVOID RequestContext,
    _In_ LPCGUID ControlGuid,
    _In_ ULONG GuidCount,
    _In_ PTRACE_GUID_REGISTRATION GuidReg,
    _In_opt_ LPCWSTR MofImagePath,
    _In_opt_ LPCWSTR MofResourceName,
    _Out_ PTRACEHANDLE RegistrationHandle
    );

ULONG
__stdcall
EtwUnregisterTraceGuids(
    _In_ TRACEHANDLE RegistrationHandle
    );

#define WPP_TRACE                   EtwTraceMessage
#define WPP_GET_TRACE_LOGGER_HANDLE EtwGetTraceLoggerHandle
#define WPP_GET_TRACE_ENABLE_LEVEL  EtwGetTraceEnableLevel
#define WPP_GET_TRACE_ENABLE_FLAGS  EtwGetTraceEnableFlags
#define WPP_REGISTER_TRACE_GUIDS    EtwRegisterTraceGuidsW
#define WPP_UNREGISTER_TRACE_GUIDS  EtwUnregisterTraceGuids

#endif

#define WPP_IsValidSid IsValidSid
#define WPP_GetLengthSid GetLengthSid

#ifndef WPP_TRACE
#define WPP_TRACE TraceMessage
#endif

#ifndef WPP_UNREGISTER_TRACE_GUIDS
#define WPP_UNREGISTER_TRACE_GUIDS  UnregisterTraceGuids 
#endif


#ifndef WPP_REGISTER_TRACE_GUIDS 
#define WPP_REGISTER_TRACE_GUIDS  RegisterTraceGuids
#endif


#ifndef WPP_GET_TRACE_LOGGER_HANDLE
#define WPP_GET_TRACE_LOGGER_HANDLE  GetTraceLoggerHandle
#endif

#ifndef WPP_GET_TRACE_ENABLE_LEVEL 
#define WPP_GET_TRACE_ENABLE_LEVEL  GetTraceEnableLevel
#endif

#ifndef WPP_GET_TRACE_ENABLE_FLAGS 
#define WPP_GET_TRACE_ENABLE_FLAGS  GetTraceEnableFlags
#endif

enum {
    WPP_VER_WIN2K_CB_FORWARD_PTR    = 0x01,
    WPP_VER_WHISTLER_CB_FORWARD_PTR = 0x02,
    WPP_VER_LH_CB_FORWARD_PTR = 0x03
};

VOID WppCleanupUm( VOID );
#define WPP_CLEANUP() WppCleanupUm()

#define WppLoadTracingSupport 

//
// For getting private callback, implement a function with the signature below.
// To register the function: #define WPP_PRIVATE_ENABLE_CALLBACK MyPrivateCallback
// If multiple control guids are used, be sure to compare Guid to the one of interest.
//

#if defined(WPP_PRIVATE_ENABLE_CALLBACK)

typedef
VOID
(NTAPI *PFN_WPP_PRIVATE_ENABLE_CALLBACK)(
    _In_ LPCGUID Guid,   
    _In_ TRACEHANDLE Logger, 
    _In_ BOOLEAN Enable, 
    _In_ ULONG Flags,    
    _In_ UCHAR Level
    );  

#endif

//
// For generating CAPTURE_STATE callback (Win7+), implement a function with the signature
// below. To register the function: #define WPP_CAPTURE_STATE_CALLBACK MyCaptureStateCallback
// If multiple control guids are used, be sure to compare Guid to the one of interest.
//

#if defined(WPP_CAPTURE_STATE_CALLBACK)

typedef
VOID
(NTAPI *PFN_WPP_CAPTURE_STATE_CALLBACK)(
    _In_ LPCGUID Guid,   
    _In_ TRACEHANDLE Logger, 
    _In_ ULONG Flags,    
    _In_ UCHAR Level
    );

#endif

#if defined(__cplusplus)
};
#endif

#endif  // #ifndef WPP_ALREADY_INCLUDED


