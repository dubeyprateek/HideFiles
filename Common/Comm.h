#ifndef __COMM__H
#define __COMM__H
#pragma once

typedef struct _DATA_BUFFER{
    int iControlCode;
    int bufferlength;
    void* pBuffer;
}DATA_BUFFER, *PDATA_BUFFER;

#endif