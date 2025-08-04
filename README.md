
# VoidCalls

Modern C++ windows system call framework.

## Background

Due to the lack of c++ windows system call frameworks with neat interfaces and decent evasion, I decided to make VoidCalls.

Part of it was out of necessity as well, since I required a good framework to use in the my C++ implants.

I plan on writing some PoCs based on my own research for [Abyss](https://github.com/AbyssFramework) using this framework.

## Features

- Clean C++ API
- Support for NTDLL, WIN32U
- Support for Direct / Indirect system calls

## Example

```cpp
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
} else {
    printf("[-] Failed to allocate memory, status: 0x%x\n", status);
}

instance->Destroy();
```

## Notes
``win32u.dll`` has to be loaded to create invokers with a Win32u factory.

## TODO

- x86 support