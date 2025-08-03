#pragma once

#include "../../include/factory.h"

class StaticInstance {
private:
    Factory* NtdllInvokerFactory;

    Invoker<NTSTATUS>* NtAllocateVirtualMemory_Invoker;

public:
    StaticInstance();

    NTSTATUS NtAllocateVirtualMemory(
        HANDLE ProcessHandle,
        PVOID* BaseAddress,
        ULONG_PTR ZeroBits,
        PSIZE_T RegionSize,
        ULONG AllocationType,
        ULONG Protect
    );
};
