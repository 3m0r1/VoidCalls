#include "./includes/hash.h"

DWORD djb2A(LPCSTR str) {
    unsigned long hash = 5381;
    char c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}
DWORD djb2W(LPCWSTR str) {
    unsigned long hash = 5381;
    wchar_t c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}
