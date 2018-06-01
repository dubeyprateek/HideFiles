#include "FIleServce.h"
#include <Wdm.h> 
#include <Ntddk.h>
#include <wdmsec.h>
#include <ntstrsafe.h>
#include <stdlib.h>


#pragma comment (lib,"NtosKrnl")

NTSTATUS InitializeLists()
{
    PAGED_CODE();

    InitializeListHead(&ExclusionList);
    InitializeListHead(&FileListToHide);
    InitializeListHead(&FolderListToHide);

    return 0;
}

NTSTATUS FindNode(PLIST_ENTRY ptargetList, PWCHAR pszFQPN, PLIST_ENTRY *pNode)
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

NTSTATUS InstertPathInList(PLIST_ENTRY ptargetList, PWCHAR pszFQPN)
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
        ExFreePoolWithTag(pszFQPNDriverBuffer,
            HideFilePoolTag);
    }
    return status;
}

NTSTATUS InsertPathInExclusionList(PWCHAR pszFQPN)
{
    PAGED_CODE();

    NTSTATUS status = -1;

    status = InstertPathInList(&ExclusionList, pszFQPN);

    return status;

}

NTSTATUS InsertPathInInclusionFileList(PWCHAR pszFQPN)
{
    PAGED_CODE();

    NTSTATUS status = -1;

    status = InstertPathInList(&FileListToHide, pszFQPN);

    return status;
}

NTSTATUS InsertPathInInclusionFolderList(PWCHAR pszFQPN)
{
    PAGED_CODE();
    NTSTATUS status = -1;

    status = InstertPathInList(&FolderListToHide, pszFQPN);

    return status;

}