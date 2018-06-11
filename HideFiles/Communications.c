#include "Communications.h"
#include "GlobalExclusionList.h"
#include"..\Common\Comm.h"

#pragma comment (lib,"FltMgr")

PWCHAR               g_pszCommunicationPortName = L"\\HideFilePort";
PFLT_PORT            g_pServerPort = NULL;
PFLT_PORT            g_pClientPort = NULL;
EX_PUSH_LOCK         g_ClientCommPortLock;
extern ULONG         HideFilePoolTag;


extern PFLT_FILTER gpFilterHandle;

//
//  Callback to notify a filter it has received a message from a user App
//
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS HideMessageNotifyCallback(
    _In_opt_ PVOID PortCookie,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
)
{
    UNREFERENCED_PARAMETER(PortCookie);
    UNREFERENCED_PARAMETER(InputBuffer);
    UNREFERENCED_PARAMETER(InputBufferLength);
    UNREFERENCED_PARAMETER(OutputBuffer);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ReturnOutputBufferLength);

    NTSTATUS status = -1;

    return status;
}

//
//  Callback to notify a filter when a new connection to a port is established
//
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS HideConnectCallback(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
)
{
    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    UNREFERENCED_PARAMETER(ConnectionPortCookie);

    NTSTATUS status = STATUS_SUCCESS;

    FltAcquirePushLockExclusive(&g_ClientCommPortLock);
    g_pClientPort = ClientPort;
    FltReleasePushLock(&g_ClientCommPortLock);

    return status;
}

//
//  Callback to notify a filter when a connection to a port is being torn down
//
_IRQL_requires_max_(APC_LEVEL)
VOID HideDisconnectCallback(
    _In_opt_ PVOID ConnectionCookie
)
{
    UNREFERENCED_PARAMETER(ConnectionCookie);

    FltAcquirePushLockExclusive(&g_ClientCommPortLock);
    if (NULL != gpFilterHandle
        && NULL != g_pClientPort) {
        FltCloseClientPort(gpFilterHandle, &g_pClientPort);
        g_pClientPort = NULL;
    }

    FltReleasePushLock(&g_ClientCommPortLock);
}

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS CreateCommunicationPort(
    _In_ PFLT_FILTER Filter
)
{
    PAGED_CODE();

    PSECURITY_DESCRIPTOR        pSD = NULL;
    OBJECT_ATTRIBUTES           oa = {0};
    NTSTATUS                    status = -1;
    UNICODE_STRING              commPortName = {0};

    if (NULL == Filter) {
        status = STATUS_INVALID_PARAMETER;
        goto EXIT;
    }

    ////
    //// Allocate the SD
    ////

    //pSD = ExAllocatePoolWithTag(PagedPool,
    //    sizeof(SECURITY_DESCRIPTOR),
    //    HideFilePoolTag);
    //if (NULL == pSD) {
    //    goto EXIT;
    //}

    status = FltBuildDefaultSecurityDescriptor(&pSD,
        FLT_PORT_ALL_ACCESS);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

    RtlInitUnicodeString(&commPortName,
        g_pszCommunicationPortName);

    InitializeObjectAttributes(&oa,
        &commPortName,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        pSD);

    status = FltCreateCommunicationPort(Filter,
        &g_pServerPort,
        &oa,
        NULL,
        HideConnectCallback,
        HideDisconnectCallback,
        HideMessageNotifyCallback,
        1);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

    FltInitializePushLock(&g_ClientCommPortLock);
EXIT:
    if (NULL != pSD) {
        FltFreeSecurityDescriptor(pSD);
    }
    return status;
}

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS CloseCommunicationPort(
    _In_ PFLT_PORT pFltPort
)
{
    NTSTATUS          status = STATUS_SUCCESS;
    
    if (NULL == pFltPort)
    {
        status = STATUS_INVALID_PARAMETER;
        goto EXIT;
    }
    FltCloseCommunicationPort(pFltPort);

EXIT:
    return status;
}

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS FilterFromGlobalExcusionList(
    const PWCHAR pszFilePath
)
{
    NTSTATUS            status = STATUS_SUCCESS;
    int                 iCount = 0;
    UNICODE_STRING      unicodeExclusionPath = {0};
    UNICODE_STRING      unicodePath = {0};

    RtlInitUnicodeString(&unicodePath, 
        pszFilePath);

    while (gGlobalExcusionList[iCount] != NULL)
    {
        RtlInitUnicodeString(&unicodeExclusionPath, 
            gGlobalExcusionList[iCount]);

        if (TRUE == FsRtlIsNameInExpression(
            &unicodeExclusionPath,
            &unicodeExclusionPath,
            TRUE,
            0))
        {
            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    return status;
}