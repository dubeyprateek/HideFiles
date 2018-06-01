#include <fltKernel.h>
#include "Communications.h"

#pragma comment (lib,"FltMgr")

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
    if (NULL != g_FilterInstance
        && NULL != g_pClientPort) {
        FltCloseClientPort(g_FilterInstance, &g_pClientPort);
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

    status = FltBuildDefaultSecurityDescriptor(pSD,
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