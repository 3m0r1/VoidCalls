#include<stdio.h>
#include<windows.h>

#include "examples/include/static.h"

int main()
{
    auto instance = new StaticInstance();

    PVOID baseAddr = NULL;
    SIZE_T regSize = 32;

    NTSTATUS status = instance->NtAllocateVirtualMemory(
        GetCurrentProcess(),
        &baseAddr,
        0,
        &regSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    if (NT_SUCCESS(status)) {
        printf("[+] Allocated 32 bytes at 0x%p\n", baseAddr);
    }
    else {
        printf("[-] Failed to allocate memory, status: 0x%x\n", status);
    }

    instance->Destroy();
}