#pragma once

#include<windows.h>

#include "pe.h"

#define STUB_SIZE 23
#define WIN32U_TRAVERSAL_OFFSET 32

#define NT_SUCCESS(status) (((NTSTATUS)(status)) >= 0)

#define CHECK_NTSTATUS(fn, status, success, ...) \
    if (NT_SUCCESS(status)) { \
		printf(success, ##__VA_ARGS__); \
	} else { \
		printf("[-] %s failed with status code: 0x%x\n", #fn, status); \
	} \

#define PRINT_CONFIG(config) printf("[CONFIG] Syscall Id: 0x%x, Jump Address: 0x%p\n", config->ssn, config->jump_addr)

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
