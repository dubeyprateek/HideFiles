#include <fltKernel.h>
#include "Communications.h"

#pragma comment (lib,"FltMgr")

//
//  Callback to notify a filter it has received a message from a user App
//

NTSTATUS HideMessageNotifyCallback(
    _In_opt_ PVOID PortCookie,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
)
{
    NTSTATUS status = -1;

    return status;
}

//
//  Callback to notify a filter when a new connection to a port is established
//

NTSTATUS HideConnectCallback(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
)
{
    NTSTATUS status = -1;

    return status;
}

//
//  Callback to notify a filter when a connection to a port is being torn down
//
VOID HideDisconnectCallback(
    _In_opt_ PVOID ConnectionCookie
)
{
    NTSTATUS status = -1;

    return status;
}


NTSTATUS CreateCommunicationPort(PFLT_FILTER Filter)
{
    PAGED_CODE();

    PSECURITY_DESCRIPTOR        pSD = NULL;
    OBJECT_ATTRIBUTES           oa = {0};
    NTSTATUS                    status = -1;


    if (NULL == Filter) {
        status = STATUS_INVALID_PARAMETER;
        goto EXIT;
    }

    status = FltBuildDefaultSecurityDescriptor(pSD,
        FLT_PORT_ALL_ACCESS);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

    InitializeObjectAttributes(&oa,
        g_pszCommunicationPortName,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        pSD);

    status = FltCreateCommunicationPort(Filter,
        g_pServerPort,
        &oa,
        NULL,
        HideConnectCallback,
        HideDisconnectCallback,
        HideMessageNotifyCallback,
        1);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

EXIT:
    if (NULL != pSD) {
        FltFreeSecurityDescriptor(pSD);
    }
    return status;
}


NTSTATUS CloseCommunicationPort(PFLT_PORT pFltPort)
{
    NTSTATUS                    status = STATUS_SUCCESS;
    
    if (NULL == pFltPort)
    {
        status = STATUS_INVALID_PARAMETER;
        goto EXIT;
    }
    FltCloseCommunicationPort(pFltPort);

EXIT:
    return status;
}