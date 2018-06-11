#include <Windows.h>
#include <Fltuser.h>
#include<stdio.h>
#include<malloc.h>
#include"..\Common\Comm.h"

#pragma comment (lib, "FltLib")


int __cdecl main()
{
    HRESULT hResult                 = -1;
    HANDLE communicationPort        = INVALID_HANDLE_VALUE;
    PDATA_BUFFER pData              = NULL;
    DWORD dwBytesRerurned           = 0;

    //
    // Connect to the communication port with the driver
    //
    hResult = FilterConnectCommunicationPort(L"\\HideFilePort",
        0,
        NULL,
        0,
        NULL,
        &communicationPort);

    if (!SUCCEEDED(hResult)) {
        goto EXIT;
    }

    
    pData = (PDATA_BUFFER)malloc(sizeof(DATA_BUFFER));
    if (!pData) {
        goto EXIT;
    }
    ZeroMemory(pData,sizeof(DATA_BUFFER));

    pData->pBuffer = malloc(MAX_PATH);
    pData->bufferlength = MAX_PATH;
    pData->iControlCode = 0;

    wcscpy_s((PWCHAR)pData->pBuffer, (SIZE_T)(MAX_PATH / 2), L"C:\\Temp\\");

    //
    // Send message 
    //
    hResult = FilterSendMessage(communicationPort,
        (LPVOID)pData,
        sizeof(DATA_BUFFER),
        NULL,
        0,
        &dwBytesRerurned);
    if (!SUCCEEDED(hResult)) {
        goto EXIT;
    }


    //
    // Recive message
    //

EXIT:
    if (INVALID_HANDLE_VALUE != communicationPort) {
        CloseHandle(communicationPort);
    }
    if (NULL != pData) {
        if (pData->pBuffer) {
            free(pData->pBuffer);
            ZeroMemory(pData, sizeof(DATA_BUFFER));
        }
        free(pData);
        pData = NULL;
    }
    return 0;
}
