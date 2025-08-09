#include "structs.h"
class Instance {
    private:
        Factory* NtdllFactory;
        Invoker<NTSTATUS>* NtAllocateVirtualMemory_Invoker;
        Invoker<NTSTATUS>* NtWriteVirtualMemory_Invoker;
        Invoker<NTSTATUS>* NtProtectVirtualMemory_Invoker;
        Invoker<NTSTATUS>* NtCreateThreadEx_Invoker;
    public:
        Instance() {
            this->NtdllFactory = new Factory(ModuleType::Ntdll, true);
            this->NtAllocateVirtualMemory_Invoker = this->NtdllFactory->CreateInvoker<NTSTATUS>(hash::Djb2A("ZwAllocateVirtualMemory"));
            this->NtWriteVirtualMemory_Invoker = this->NtdllFactory->CreateInvoker<NTSTATUS>(hash::Djb2A("ZwWriteVirtualMemory"));
            this->NtProtectVirtualMemory_Invoker = this->NtdllFactory->CreateInvoker<NTSTATUS>(hash::Djb2A("ZwProtectVirtualMemory"));
            this->NtCreateThreadEx_Invoker = this->NtdllFactory->CreateInvoker<NTSTATUS>(hash::Djb2A("ZwCreateThreadEx"));
        }
        NTSTATUS NtAllocateVirtualMemory(
            _In_ HANDLE ProcessHandle,
            _Inout_ PVOID * BaseAddress,
            _In_ ULONG ZeroBits,
            _Inout_ PSIZE_T RegionSize,
            _In_ ULONG AllocationType,
            _In_ ULONG Protect
        ) {
            auto invoker = this->NtAllocateVirtualMemory_Invoker;
            return invoker->InvokeIndirect (
                ProcessHandle,
                BaseAddress,
                ZeroBits,
                RegionSize,
                AllocationType,
                Protect
            );
        }
        NTSTATUS NtWriteVirtualMemory(
            _In_ HANDLE ProcessHandle,
            _In_ PVOID BaseAddress,
            _In_ PVOID Buffer,
            _In_ SIZE_T NumberOfBytesToWrite,
            _Out_opt_ PSIZE_T NumberOfBytesWritten
        ) {
            auto invoker = this->NtWriteVirtualMemory_Invoker;
            return invoker->InvokeIndirect (
                ProcessHandle,
                BaseAddress,
                Buffer,
                NumberOfBytesToWrite,
                NumberOfBytesWritten
            );
        }
        NTSTATUS NtProtectVirtualMemory(
            _In_ HANDLE ProcessHandle,
            _Inout_ PVOID * BaseAddress,
            _Inout_ PSIZE_T RegionSize,
            _In_ ULONG NewProtect,
            _Out_ PULONG OldProtect
        ) {
            auto invoker = this->NtProtectVirtualMemory_Invoker;
            return invoker->InvokeIndirect (
                ProcessHandle,
                BaseAddress,
                RegionSize,
                NewProtect,
                OldProtect
            );
        }
        NTSTATUS NtCreateThreadEx(
            _Out_ PHANDLE ThreadHandle,
            _In_ ACCESS_MASK DesiredAccess,
            _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
            _In_ HANDLE ProcessHandle,
            _In_ PVOID StartRoutine,
            _In_opt_ PVOID Argument,
            _In_ ULONG CreateFlags,
            _In_ SIZE_T ZeroBits,
            _In_ SIZE_T StackSize,
            _In_ SIZE_T MaximumStackSize,
            _In_opt_ PPS_ATTRIBUTE_LIST AttributeList
        ) {
            auto invoker = this->NtCreateThreadEx_Invoker;
            return invoker->InvokeIndirect (
                ThreadHandle,
                DesiredAccess,
                ObjectAttributes,
                ProcessHandle,
                StartRoutine,
                Argument,
                CreateFlags,
                ZeroBits,
                StackSize,
                MaximumStackSize,
                AttributeList
            );
        }
};
