#pragma once

#include<windows.h>

#include "pe.h"

#define NT_SUCCESS(status) (((NTSTATUS)(status)) >= 0)

#define CHECK_NTSTATUS(fn, status, message, ...) \
    if (NT_SUCCESS(status)) { \
		printf(message, ##__VA_ARGS__); \
	} else { \
		printf("[-] %s failed with status code 0x%x\n", #fn, status); \
	} \
		
#define STUB_SIZE 23

#define INIT_CTX_NTDLL() init_ctx(Ntdll, TRUE)
#define INIT_CTX_WIN32U() init_ctx(Win32u, TRUE)

#define GET_SYS_CONFIG(ctx, fn) \
	get_sys_config(ctx, djb2A(#fn))

#define INVOKE_INDIRECT(config, ...) \
     SysSetConfig(config); \
     status = SysInvokeIndirect(__VA_ARGS__);

#define INVOKE_DIRECT(config, ...) \
     SysSetConfig(config); \
     status = SysInvokeDirect(__VA_ARGS__);

typedef struct SysConfig {
	WORD ssn;
	PVOID jump_addr;
} SysConfig;

typedef struct SysCtx {
	ModuleType type;
	BOOL get_syscall_addr;
	Pe pe;
} SysCtx;

SysCtx* init_ctx(ModuleType module_type, BOOL get_syscall_addr);
SysConfig* get_sys_config(SysCtx* ctx, DWORD fn_hash);

void free_ctx(SysCtx* ctx);
void free_config(SysConfig* config);

void SysSetConfig(SysConfig* config);

NTSTATUS SysInvokeIndirect(void*, ...);
NTSTATUS SysInvokeDirect(void*, ...);
