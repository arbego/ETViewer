#pragma once

#ifdef WPP_ETW_PROVIDER

#include <wmistr.h>
#include <evntrace.h>
#include "evntprov.h"


#if !defined(ETW_INLINE)
#define ETW_INLINE DECLSPEC_NOINLINE __inline
#endif

#if defined(__cplusplus)
extern "C" {
#endif


#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION
#if  !defined(McGenDebug)
#define McGenDebug(a,b)
#endif 


#if !defined(MCGEN_TRACE_CONTEXT_DEF)
#define MCGEN_TRACE_CONTEXT_DEF
typedef struct _MCGEN_TRACE_CONTEXT
{
    TRACEHANDLE     RegistrationHandle;
    TRACEHANDLE     Logger;
    ULONGLONG       MatchAnyKeyword;
    ULONGLONG       MatchAllKeyword;
    ULONG           Flags;
    ULONG           IsEnabled;
    UCHAR           Level; 
    UCHAR           Reserve;
} MCGEN_TRACE_CONTEXT, *PMCGEN_TRACE_CONTEXT;
#endif

#if !defined(MCGEN_EVENT_ENABLED_DEF)
#define MCGEN_EVENT_ENABLED_DEF
FORCEINLINE
BOOLEAN
McGenEventEnabled(
    __in PMCGEN_TRACE_CONTEXT EnableInfo,
    __in PCEVENT_DESCRIPTOR EventDescriptor
    )
{
    //
    // Check if the event Level is lower than the level at which
    // the channel is enabled.
    // If the event Level is 0 or the channel is enabled at level 0,
    // all levels are enabled.
    //

    if ((EventDescriptor->Level <= EnableInfo->Level) || // This also covers the case of Level == 0.
        (EnableInfo->Level == 0)) {

        //
        // Check if Keyword is enabled
        //

        if ((EventDescriptor->Keyword == (ULONGLONG)0) ||
            ((EventDescriptor->Keyword & EnableInfo->MatchAnyKeyword) &&
             ((EventDescriptor->Keyword & EnableInfo->MatchAllKeyword) == EnableInfo->MatchAllKeyword))) {

            return TRUE;
        }
    }
    return FALSE;
}
#endif


//
// EnableCheckMacro
//
#ifndef MCGEN_ENABLE_CHECK
#define MCGEN_ENABLE_CHECK(Context, Descriptor) (Context.IsEnabled &&  McGenEventEnabled(&Context, &Descriptor))
#endif

#if !defined(MCGEN_CONTROL_CALLBACK)
#define MCGEN_CONTROL_CALLBACK

DECLSPEC_NOINLINE __inline
VOID
__stdcall
McGenControlCallbackV2(
    __in LPCGUID SourceId,
    __in ULONG ControlCode,
    __in UCHAR Level,
    __in ULONGLONG MatchAnyKeyword,
    __in ULONGLONG MatchAllKeyword,
    __in_opt PEVENT_FILTER_DESCRIPTOR FilterData,
    __inout_opt PVOID CallbackContext
    )
/*++

Routine Description:

    This is the notification callback for Vista.

Arguments:

    SourceId - The GUID that identifies the session that enabled the provider. 

    ControlCode - The parameter indicates whether the provider 
                  is being enabled or disabled.

    Level - The level at which the event is enabled.

    MatchAnyKeyword - The bitmask of keywords that the provider uses to 
                      determine the category of events that it writes.

    MatchAllKeyword - This bitmask additionally restricts the category 
                      of events that the provider writes. 

    FilterData - The provider-defined data.

    CallbackContext - The context of the callback that is defined when the provider 
                      called EtwRegister to register itself.

Remarks:

    ETW calls this function to notify provider of enable/disable

--*/
{
    PMCGEN_TRACE_CONTEXT Ctx = (PMCGEN_TRACE_CONTEXT)CallbackContext;
#ifndef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(FilterData);
#endif

    if (Ctx == NULL) {
        return;
    }

    switch (ControlCode) {

        case EVENT_CONTROL_CODE_ENABLE_PROVIDER:
            Ctx->Level = Level;
            Ctx->MatchAnyKeyword = MatchAnyKeyword;
            Ctx->MatchAllKeyword = MatchAllKeyword;
            Ctx->IsEnabled = EVENT_CONTROL_CODE_ENABLE_PROVIDER;
            break;

        case EVENT_CONTROL_CODE_DISABLE_PROVIDER:
            Ctx->IsEnabled = EVENT_CONTROL_CODE_DISABLE_PROVIDER;
            Ctx->Level = 0;
            Ctx->MatchAnyKeyword = 0;
            Ctx->MatchAllKeyword = 0;
            break;
 
        default:
            break;
    }

#ifdef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    //
    // Call user defined callback
    //
    MCGEN_PRIVATE_ENABLE_CALLBACK_V2(
        SourceId,
        ControlCode,
        Level,
        MatchAnyKeyword,
        MatchAllKeyword,
        FilterData,
        CallbackContext
        );
#endif
   
    return;
}

#endif
#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION


EXTERN_C __declspec(selectany) const GUID `Provider.ProviderSymbol`;// = `Provider.ProviderGuidStructForm`;

#define `Provider.ProviderSymbol`_CHANNEL_C1 0x9

`FORALL Msg IN Messages WHERE MsgIsEtw`
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR `Msg.Func`_`Msg.EtwId`;
#define `Msg.Name`_Value `Msg.EtwId`

`ENDFOR`

#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION


EXTERN_C __declspec(selectany) REGHANDLE `Provider.ProviderVarName`Handle;// = (REGHANDLE)0;

EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT `Provider.ProviderSymbol`_Context;// = {0};

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    __in LPCGUID ProviderId,
    __in_opt PENABLECALLBACK EnableCallback,
    __in_opt PVOID CallbackContext,
    __inout PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW USER mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG Error;


    if (*RegHandle) {
        //
        // already registered
        //
        return ERROR_SUCCESS;
    }

    Error = EventRegister( ProviderId, EnableCallback, CallbackContext, RegHandle); 

    return Error;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(__inout PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW USER mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG Error;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return ERROR_SUCCESS;
    }

    Error = EventUnregister(*RegHandle); 
    *RegHandle = (REGHANDLE)0;
    
    return Error;
}
#endif

#ifdef WPP_LOGPAIR_SEPARATOR
# undef WPP_LOGPAIR_SEPARATOR
# define WPP_LOGPAIR_SEPARATOR
#endif

#ifdef WPP_LOGPAIR_SIZET
# undef WPP_LOGPAIR_SIZET
# define WPP_LOGPAIR_SIZET ULONG
#endif

`FORALL i IN TypeSigSet WHERE !UnsafeArgs && Etw`
#ifndef EtwppTemplate_`i.Name`_def
#define EtwppTemplate_`i.Name`_def
ETW_INLINE
ULONG
EtwppTemplate_`i.Name`(
    __in REGHANDLE RegHandle,
    __in PCEVENT_DESCRIPTOR Descriptor`i.EtwHasArguments`
    `i.EtwArguments`
    )
{
#define ARGUMENT_COUNT_`i.Name` `i.Count`

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_`i.Name` + 1];

    `i.EtwFillDescriptors`

    ULONG Result = EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_`i.Name`, (ARGUMENT_COUNT_`i.Name` == 0) ? NULL : EventData);
    return Result;
}

#endif
`ENDFOR`

#ifndef EventRegister`Provider.ProviderVarName`
#define EventRegister`Provider.ProviderVarName`() McGenEventRegister(&`Provider.ProviderSymbol`, McGenControlCallbackV2, &`Provider.ProviderSymbol`_Context, &`Provider.ProviderVarName`Handle) 
#endif

#ifndef EventUnregister`Provider.ProviderVarName`
#define EventUnregister`Provider.ProviderVarName`() McGenEventUnregister(&`Provider.ProviderVarName`Handle) 
#endif

#define ETW_START EventRegister`Provider.ProviderVarName`
#define ETW_STOP EventUnregister`Provider.ProviderVarName`

`FORALL Msg IN Messages WHERE MsgIsEtw`
#define EventWrite_`Msg.Name`(`Msg.FixedArgs``Msg.MacroArgs`)\
        MCGEN_ENABLE_CHECK(`Provider.ProviderSymbol`_Context, `Msg.Func`_`Msg.EtwId`) ?\
        Etwpp`Msg.EtwTemplateSymbol`(`Provider.ProviderVarName`Handle, &`Msg.Func`_`Msg.EtwId``Msg.MacroExprs`)\
        : ERROR_SUCCESS\

`ENDFOR`

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION


#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

#ifndef ETW_LOGPAIR
#define ETW_LOGPAIR(_Size, _Addr) WPP_LOGPAIR(_Size, _Addr)
#endif

#define ETW_LOGTYPEVAL(_Type, _Value) ETW_LOGPAIR(sizeof(_Type), &(_Value))
#define ETW_LOGTYPEPTR(_Value) ETW_LOGPAIR(sizeof(*(_Value)), (_Value))

#endif

#define ETW_THIS_FILE `SourceFile.CanonicalName`

#define ETW_EVAL(_value_) _value_
#define Event(Id) ETW_EVAL(Event) ## ETW_EVAL(Id) ## ETW_EVAL(_) ## ETW_EVAL(ETW_THIS_FILE) ## ETW_EVAL(__LINE__)

#if defined(__cplusplus)
};
#endif

#define MSG_event_10                         0x0000000AL
#define MSG_providermessage                  0x90000001L

#endif
