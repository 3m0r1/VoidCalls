#include "memory.h"

void run_memory_example() {
    SysCtx* ctx = INIT_CTX_NTDLL();
    SysConfig* config = GET_SYS_CONFIG(ctx, ZwAllocateVirtualMemory);

    PRINT_CONFIG(config);

    PVOID base_address = NULL;
    SIZE_T size = 32;

    NTSTATUS status;
    INVOKE_INDIRECT(
        config,
        GetCurrentProcess(),
        &base_address,
        0,
        &size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );

    CHECK_NTSTATUS(
        ZwAllocateVirtualMemory,
        status,
        "Successfully allocated memory at 0x%p\n",
        base_address
    );

    free_ctx(ctx);
    free_config(config);
}