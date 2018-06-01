#ifndef __FILE_COMMUNICATIONS__H
#define __FILE_COMMUNICATIONS__H
#pragma once
#include <Wdm.h>

PWCHAR g_pszCommunicationPortName = L"\\HidePort";
PFLT_PORT *g_pServerPort = NULL;

NTSTATUS CreateCommunicationPort(
    PFLT_FILTER Filter
);

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
);

//
//  Callback to notify a filter when a new connection to a port is established
//

NTSTATUS HideConnectCallback(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
);

//
//  Callback to notify a filter when a connection to a port is being torn down
//
VOID HideDisconnectCallback(
    _In_opt_ PVOID ConnectionCookie
);

#endif