#include "./include/static.h"

StaticInstance::StaticInstance() {
    this->NtdllInvokerFactory = new Factory(ModuleType::Ntdll, true);

    this->NtAllocateVirtualMemory_Invoker = this->NtdllInvokerFactory->CreateInvoker<NTSTATUS>(hash::Djb2A("ZwAllocateVirtualMemory"));
}

NTSTATUS StaticInstance::NtAllocateVirtualMemory(
    HANDLE ProcessHandle,
    PVOID* BaseAddress,
    ULONG_PTR ZeroBits,
    PSIZE_T RegionSize,
    ULONG AllocationType,
    ULONG Protect
) {
    auto invoker = this->NtAllocateVirtualMemory_Invoker;

    return invoker->InvokeIndirect(
        ProcessHandle,
        BaseAddress,
        ZeroBits,
        RegionSize,
        AllocationType,
        Protect
    );
}