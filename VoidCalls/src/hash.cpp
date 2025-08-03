#include<windows.h>

#include "../include/hash.h"

DWORD hash::Djb2A(LPCSTR str) {
    unsigned long hash = 5381;
    char c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    
    return hash;
}

DWORD hash::Djb2W(LPCWSTR str) {
    unsigned long hash = 5381;
    wchar_t c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}