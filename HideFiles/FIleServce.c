#include "FileServce.h"
#include "Communications.h"
#include <ntstrsafe.h>
#include <stdlib.h>


#pragma comment (lib,"NtosKrnl")

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS InitializeLists()
{
    PAGED_CODE();

    InitializeListHead(&ExclusionList);
    InitializeListHead(&FileListToHide);
    InitializeListHead(&FolderListToHide);

    return 0;
}


_IRQL_requires_max_(APC_LEVEL)
NTSTATUS FindNode(
    PLIST_ENTRY ptargetList, 
    PWCHAR pszFQPN, 
    PLIST_ENTRY *pNode
)
{
    PAGED_CODE();

    NTSTATUS            status = -1;
    PLIST_ENTRY         pListEntry = NULL;
    BOOLEAN             bFound = FALSE;
    PLIST               pListNode = NULL;
    UNICODE_STRING      uDriverString;

    pListEntry = ptargetList->Flink;

    if (TRUE == IsListEmpty(ptargetList)) {
        status = STATUS_INVALID_PARAMETER;
        goto EXIT;
    }

    RtlInitUnicodeString(&uDriverString, 
        pszFQPN);

    while (pListEntry->Flink != ptargetList)
    {
        pListNode = CONTAINING_RECORD(pListEntry,
            LIST,
            Link);
        
        if (0 == RtlCompareUnicodeString(&uDriverString,
            &pListNode->pFullFilePath, 
            TRUE))
        {
            bFound = TRUE;
            *pNode = pListEntry;
        }
        pListEntry = pListEntry->Flink;
    }
    
    if (FALSE == bFound)
    {
        status = STATUS_NOT_FOUND;
    }

EXIT:
    return status;
}


_IRQL_requires_max_(APC_LEVEL)
NTSTATUS InstertPathInList(
    PLIST_ENTRY ptargetList, 
    PWCHAR pszFQPN
)
{
    PAGED_CODE();

    PLIST           pNewNode = NULL;
    NTSTATUS        status = -1;
    PWCHAR          pszFQPNDriverBuffer = NULL;
    size_t          strLength = 0;
    size_t          maxLength = _MAX_PATH;

    pNewNode = (PLIST)ExAllocatePoolWithTag(PagedPool,
        sizeof(LIST),
        HideFilePoolTag);
    if (NULL == pNewNode) {
        goto EXIT;
    }

    RtlStringCbLengthW(pszFQPN, 
        maxLength, 
        &strLength);

    pszFQPNDriverBuffer = (PWCHAR)ExAllocatePoolWithTag(PagedPool,
        (strLength + 1) * 2,
        HideFilePoolTag);
    if (NULL == pNewNode) {
        goto EXIT;
    }

    RtlZeroMemory(pszFQPNDriverBuffer, strLength * 2);

    status = RtlStringCbCopyExW(pszFQPNDriverBuffer,
        (strLength + 1) * 2,
        pszFQPN,
        NULL,
        NULL,
        STRSAFE_NO_TRUNCATION);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

    RtlInitUnicodeString(&pNewNode->pFullFilePath,
        pszFQPNDriverBuffer);

    InitializeListHead(&pNewNode->Link);
    InsertHeadList(ptargetList, 
        &pNewNode->Link);

EXIT:
    if (!NT_SUCCESS(status)) {
        if (NULL != pszFQPNDriverBuffer) {
            ExFreePoolWithTag(pszFQPNDriverBuffer,
                HideFilePoolTag);
        }
    }
    return status;
}


_IRQL_requires_max_(APC_LEVEL)
NTSTATUS InsertPathInExclusionList(
    PWCHAR pszFQPN
)
{
    PAGED_CODE();

    NTSTATUS status = -1;

    status = InstertPathInList(&ExclusionList, pszFQPN);

    return status;

}


_IRQL_requires_max_(APC_LEVEL)
NTSTATUS InsertPathInInclusionFileList(
    PWCHAR pszFQPN
)
{
    PAGED_CODE();

    NTSTATUS status = -1;

    if(NULL == pszFQPN)

    status = FilterFromGlobalExcusionList(pszFQPN);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

    status = InstertPathInList(&FileListToHide, pszFQPN);

EXIT:
    return status;
}

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS InsertPathInInclusionFolderList(
    PWCHAR pszFQPN
)
{
    PAGED_CODE();
    NTSTATUS status = -1;

    status = InstertPathInList(&FolderListToHide, pszFQPN);

    return status;

}


_IRQL_requires_max_(APC_LEVEL)
NTSTATUS IsPathExists(
    _In_ PFLT_FILTER pFilter,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PWCHAR pszFQPN,
    _In_ BOOLEAN isDirectory
)
{
    PAGED_CODE();

    NTSTATUS                    status = -1;
    HANDLE                      hFileHandle = NULL;
    PSECURITY_DESCRIPTOR        pSD = NULL;
    OBJECT_ATTRIBUTES           oa = { 0 };
    UNICODE_STRING              pFQPN = { 0 };
    IO_STATUS_BLOCK             ioStatusBlock = {0};

    if (NULL == pszFQPN) {
        status = PHCM_ERROR_INVALID_PARAMETER;
        goto EXIT;
    }
    status = FltBuildDefaultSecurityDescriptor(pSD,
        FLT_PORT_ALL_ACCESS);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

    RtlInitUnicodeString(&pFQPN,
        pszFQPN);

    InitializeObjectAttributes(&oa,
        &pFQPN,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        pSD);

    //
    // Try to create a file and send it to low level drivers
    //
    status = FltCreateFile(pFilter,
        FltObjects->Instance,
        &hFileHandle,
        READ_CONTROL,
        &oa,
        &ioStatusBlock,
        0,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        isDirectory ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
    if (!NT_SUCCESS(status)) {
        goto EXIT;
    }

    if (ioStatusBlock.Status == FILE_OPENED ||
        ioStatusBlock.Status == FILE_EXISTS) {
        
        //
        // Close the file handle 
        //
        status = FltClose(hFileHandle);
        if (!NT_SUCCESS(status)) {
            goto EXIT;
        }

        //
        // File exists return success to the caller.
        //
        status = STATUS_SUCCESS;
    }

EXIT:
    if (NULL != pSD) {
        FltFreeSecurityDescriptor(pSD);
    }
    return status;
}