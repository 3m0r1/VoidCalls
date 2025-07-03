## VoidCalls
Windows system call framework with evasion capabilities.

## Explanation
VoidCalls allows the user to make direct or indirect system calls with easy-to-use macros while also not doing anything suspicious.

VoidCalls resolves System Call Service Numbers using the Exception Directory, meaning it doesn't need to read NTDLL's system call stubs (which might have been tampered with).  

## Example
```c
    SysCtx* ctx = INIT_CTX_NTDLL();
    SysConfig* config = GET_SYS_CONFIG(ctx, ZwAllocateVirtualMemory);

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

```

## TODO
- Add x86 support