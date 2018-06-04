#ifndef __GLOBAL_EXCLUSION_LIST_H__
#define __GLOBAL_EXCLUSION_LIST_H__
#pragma once
#include<wdm.h>


#define _MAX_GLOBAL_EXCLUSIONS 50

//
// Driver should not block IO to device and system known 
// system files and folders pattern.
// The match will be found by FsRtlIsNameInExpression
//
WCHAR const gGlobalExcusionList[][_MAX_GLOBAL_EXCLUSIONS] = { 
    L"A:?",
    L"B:?",
    L"C:?",
    L"D:?",
    L"E:?",
    L"F:?",
    L"G:?",
    L"H:?",
    L"I:?",
    L"J:?",
    L"K:?",
    L"L:?",
    L"M:?",
    L"N:?",
    L"O:?",
    L"P:?",
    L"Q:?",
    L"R:?",
    L"S:?",
    L"T:?",
    L"U:?",
    L"V:?",
    L"W:?",
    L"X:?",
    L"Y:?",
    L"Z:?",
    L"*PROGRAM FILES*",
    L"*USERS\\",
    L"*WINDOWS*",
};

#endif