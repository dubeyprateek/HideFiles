#ifndef __FILE_SERVICE__H
#define __FILE_SERVICE__H
#include <fltKernel.h>

CONST ULONG HideFilePoolTag = 'EDIH';

typedef struct _LIST
{
    LIST_ENTRY      Link;
    UNICODE_STRING  pFullFilePath;
}LIST, *PLIST;

LIST_ENTRY ExclusionList;
LIST_ENTRY FileListToHide;
LIST_ENTRY FolderListToHide;


NTSTATUS InitializeLists();
NTSTATUS InstertPathInList(PLIST_ENTRY ptargetList, PWCHAR pszFQPN);
NTSTATUS InsertPathInExclusionList(PWCHAR pszFQPN);
NTSTATUS InsertPathInInclusionFileList(PWCHAR pszFQPN);
NTSTATUS InsertPathInInclusionFolderList(PWCHAR pszFQPN);
NTSTATUS IsFileExist(PWCHAR pszFQPN);


#endif