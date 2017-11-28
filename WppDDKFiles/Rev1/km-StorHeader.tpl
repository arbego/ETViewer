`**********************************************************************`
`* This is an include template file for tracewpp preprocessor.        *`
`*                                                                    *`
`*    Copyright 1999-2000 Microsoft Corporation. All Rights Reserved. *`
`**********************************************************************`

// template `TemplateFile`

`* Dump the definitions specified via -D on the command line to WPP *`

`FORALL def IN MacroDefinitions`
#define `def.Name` `def.Alias`
`ENDFOR`

#define WPP_THIS_FILE `SourceFile.CanonicalName`

#include <stddef.h>
#include <stdarg.h>
#include <wmistr.h>

#if defined(__cplusplus)
extern "C" {
#endif

ULONG
__inline
NullTraceFunc (
    IN ULONG64  LoggerHandle,
    IN ULONG   MessageFlags,
    IN LPCGUID  MessageGuid,
    IN USHORT  MessageNumber,
    IN ...
    )
{
   UNREFERENCED_PARAMETER(LoggerHandle);
   UNREFERENCED_PARAMETER(MessageFlags);
   UNREFERENCED_PARAMETER(MessageGuid);
   UNREFERENCED_PARAMETER(MessageNumber);
   return 0;
}
       
__declspec(selectany) PFN_WPPTRACEMESSAGE    pfnWppTraceMessage = NullTraceFunc;

#if defined(__cplusplus)
};
#endif

#if !defined(_NTRTL_) 
// fake RTL_TIME_ZONE_INFORMATION //
typedef int RTL_TIME_ZONE_INFORMATION;
#define _WMIKM_
#endif

#ifndef WPP_TRACE
#define WPP_TRACE pfnWppTraceMessage
#endif

#ifndef WPP_OLDCC
#define WPP_OLDCC
#endif

///////////////////////////////////////////////////////////////////////////////
//
// B O R R O W E D  D E F I N I T I O N S
//
///////////////////////////////////////////////////////////////////////////////

# define WPP_LOGGER_ARG ULONG64 Logger,

#ifndef NTAPI
#if ((_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)) && !defined(_M_AMD64)
#define NTAPI __stdcall
#else
#define _cdecl
#define NTAPI
#endif
#endif


//
// Define API decoration for direct importing system DLL references.
//

#if !defined(_NTSYSTEM_)
#define NTSYSAPI     DECLSPEC_IMPORT
#define NTSYSCALLAPI DECLSPEC_IMPORT
#else
#define NTSYSAPI
#if defined(_NTDLLBUILD_)
#define NTSYSCALLAPI
#else
#define NTSYSCALLAPI DECLSPEC_ADDRSAFE
#endif

#endif

#if !defined(_NTDEF_)
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#endif

#define TRACE_MESSAGE_SEQUENCE                1      // Message should include a sequence number
#define TRACE_MESSAGE_GUID                    2      // Message includes a GUID
#define TRACE_MESSAGE_COMPONENTID             4      // Message has no GUID, Component ID instead
#define TRACE_MESSAGE_TIMESTAMP               8      // Message includes a timestamp
#define TRACE_MESSAGE_PERFORMANCE_TIMESTAMP   16     // *Obsolete* Clock type is controlled by
                                                     // the logger
#define TRACE_MESSAGE_SYSTEMINFO              32     // Message includes system information TID,PID
#define TRACE_MESSAGE_FLAG_MASK               0xFFFF // Only the lower 16 bits of flags are
                                                     // placed in the message those above 16
                                                     // bits are reserved for local processing
#ifndef TRACE_MESSAGE_MAXIMUM_SIZE
#define TRACE_MESSAGE_MAXIMUM_SIZE  8*1024           // the maximum size allowed for a single trace
#endif                                               // message

#ifndef RtlFillMemory
#define RtlFillMemory(Destination,Length,Fill) StorMemSet((Destination),(Fill),(Length))
#endif

#ifndef RtlZeroMemory
#define RtlZeroMemory(Destination,Length) StorMemSet((Destination),0,(Length))
#endif

#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L) // ntsubauth
#define STATUS_WMI_GUID_NOT_FOUND        ((NTSTATUS)0xC0000295L)
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)
#define STATUS_INVALID_DEVICE_REQUEST    ((NTSTATUS)0xC0000010L)


typedef ULONG64 TRACEHANDLE, *PTRACEHANDLE;

#ifndef TRACE_INFORMATION_CLASS_DEFINE
typedef enum _TRACE_INFORMATION_CLASS {
    TraceIdClass,
    TraceHandleClass,
    TraceEnableFlagsClass,
    TraceEnableLevelClass,
    GlobalLoggerHandleClass,
    EventLoggerHandleClass,
    AllLoggerHandlesClass,
    TraceHandleByNameClass
} TRACE_INFORMATION_CLASS;
#endif

//
// Action code for IoWMIRegistrationControl api
//

#define WMIREG_ACTION_REGISTER      1
#define WMIREG_ACTION_DEREGISTER    2
#define WMIREG_ACTION_REREGISTER    3
#define WMIREG_ACTION_UPDATE_GUIDS  4
#define WMIREG_ACTION_BLOCK_IRPS    5

///////////////////////////////////////////////////////////////////////////////


__inline TRACEHANDLE WppQueryLogger(_In_opt_ PWSTR LoggerName)
{
    ULONG ReturnLength;
    NTSTATUS Status;
    TRACEHANDLE TraceHandle;
    UNICODE_STRING Buffer;

    StorRtlInitUnicodeString(&Buffer, LoggerName ? LoggerName : L"stdout");

    if ((Status = StorWmiQueryTraceInformation(TraceHandleByNameClass,
                                               (PVOID)&TraceHandle,
                                               sizeof(TraceHandle),
                                               &ReturnLength,
                                               (PVOID)&Buffer)) != STATUS_SUCCESS) {
       return 0;
    }

    return TraceHandle;
}

typedef NTSTATUS (*WMIENTRY_NEW)(
    IN UCHAR ActionCode,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN OUT PVOID Buffer,
    IN PVOID Context,
    OUT PULONG Size
    );

typedef struct _WPP_TRACE_CONTROL_BLOCK
{
    WMIENTRY_NEW                     Callback;
    struct _WPP_TRACE_CONTROL_BLOCK *Next;

    __int64 Logger;
    UCHAR FlagsLen; UCHAR Level; USHORT Reserved;
    ULONG  Flags[1];
} WPP_TRACE_CONTROL_BLOCK, *PWPP_TRACE_CONTROL_BLOCK;

typedef struct _WPP_REGISTRATION_BLOCK
{
    WMIENTRY_NEW                    Callback;
    struct _WPP_REGISTRATION_BLOCK *Next;

    LPCGUID ControlGuid;
    LPCWSTR  FriendlyName;
    LPCWSTR  BitNames;
    PUNICODE_STRING RegistryPath;

    UCHAR   FlagsLen, RegBlockLen;
} WPP_REGISTRATION_BLOCK, *PWPP_REGISTRATION_BLOCK;



