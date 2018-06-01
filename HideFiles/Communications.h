#ifndef __FILE_COMMUNICATIONS__H
#define __FILE_COMMUNICATIONS__H
#pragma once
#include <Wdm.h>

PWCHAR g_pszCommunicationPortName = L"\\HideFilePort";
PFLT_PORT g_pServerPort = NULL;
PFLT_PORT g_pClientPort = NULL;
EX_PUSH_LOCK g_ClientCommPortLock;
PFLT_FILTER g_FilterInstance = NULL;

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS CreateCommunicationPort(
    _In_ PFLT_FILTER Filter
);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS CloseCommunicationPort(
    _In_ PFLT_PORT pFltPort
);

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
);

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
);

//
//  Callback to notify a filter when a connection to a port is being torn down
//
_IRQL_requires_max_(APC_LEVEL)
VOID HideDisconnectCallback(
    _In_opt_ PVOID ConnectionCookie
);

#endif