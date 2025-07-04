#include "./includes/syscalls.h"

const BYTE SYSCALL_SIGNATURE[] = { 0x0f, 0x05 };
const BYTE SYSCALL_START[] = { 0x4c, 0x8b, 0xd1, 0xb8 };

SysCtx* init_ctx(ModuleType module_type, BOOL get_syscall_addr) {
    SysCtx* ctx = (SysCtx*)calloc(1, sizeof(SysCtx));

    if (!ctx)
        return NULL;

    ctx->type = module_type;
    ctx->get_syscall_addr = get_syscall_addr;
    ctx->pe = parse_image(ctx->type);

    return ctx;
}

PVOID get_syscall_addr(DWORD_PTR fn_addr) {
    for (size_t x = 0; x < STUB_SIZE; x += 2) {
        PVOID curr_addr = (PVOID)(fn_addr + x);

        if (!memcmp(curr_addr, SYSCALL_SIGNATURE, sizeof(SYSCALL_SIGNATURE))) {
            return curr_addr;
        }
    }

    return NULL;
}

DWORD find_ssn(DWORD_PTR stub_address) {
    if (!memcmp((PVOID)stub_address, SYSCALL_START, sizeof(SYSCALL_START))) {

        BYTE high = *(PBYTE)(stub_address + 0x5);
        BYTE low = *(PBYTE)(stub_address + 0x4);

        return (high << 8) | low;
    }
    else {
        return 0;
    }
}

SysConfig* get_sys_config(SysCtx* ctx, DWORD fn_hash) {
    SysConfig* config = (SysConfig*)calloc(1, sizeof(SysConfig));

    if (!config)
        return NULL;

    Pe pe = ctx->pe;

    DWORD_PTR image_base = pe.headers.nt_hdr->OptionalHeader.ImageBase;
    PIMAGE_EXPORT_DIRECTORY export_dir = pe.directories.export_dir;
    PIMAGE_RUNTIME_FUNCTION_ENTRY exception_dir = pe.directories.exception_dir;

    PDWORD name_rvas = (PDWORD)(image_base + export_dir->AddressOfNames);
    PWORD ordinals = (PWORD)(image_base + export_dir->AddressOfNameOrdinals);
    PDWORD function_rvas = (PDWORD)(image_base + export_dir->AddressOfFunctions);

    if (ctx->type == Win32u) {
        for (size_t i = 0; i < export_dir->NumberOfFunctions; i++) {
            LPCSTR fn_name = (LPCSTR)(image_base + name_rvas[i]);

            if (strncmp(fn_name, "Nt", 2))
                continue;

            WORD fn_ord = ordinals[i];
            DWORD fn_rva = function_rvas[fn_ord];

            if (djb2A(fn_name) == fn_hash) {
                DWORD_PTR fn_addr = (DWORD_PTR)(image_base + fn_rva);

                int traverse = 0;
                WORD next_ssn = 0;

                while (TRUE) {
                    DWORD_PTR next_stub = (DWORD_PTR)(fn_addr + (traverse * (WIN32U_TRAVERSAL_OFFSET)));
                    next_ssn = find_ssn(next_stub);

                    if (next_ssn != 0) {
                        break;
                    }

                    traverse++;
                }

                WORD ssn = next_ssn - traverse;
                config->ssn = ssn;

                if (ctx->get_syscall_addr) {
                    config->jump_addr = get_syscall_addr(fn_addr);
                }
            }
        }
    }
    else if (ctx->type == Ntdll) {
        DWORD index = 0;
        WORD ssn = 0;

        while (exception_dir[index].BeginAddress) {
            for (size_t i = 0; i < export_dir->NumberOfFunctions; i++) {
                LPCSTR fn_name = (LPCSTR)(image_base + name_rvas[i]);

                if (strncmp(fn_name, "Zw", 2))
                    continue;

                WORD fn_ord = ordinals[i];
                DWORD fn_rva = function_rvas[fn_ord];

                if (fn_rva == exception_dir[index].BeginAddress) {
                    if (djb2A(fn_name) == fn_hash) {
                        config->ssn = ssn;

                        if (ctx->get_syscall_addr) {
                            DWORD_PTR fn_addr = (DWORD_PTR)(image_base + fn_rva);
                            config->jump_addr = get_syscall_addr(fn_addr);
                        }

                        break;
                    }

                    ssn++;
                }
            }

            index++;
        }
    }

    return config;
}

void free_ctx(SysCtx* ctx) {
    free(ctx);
}

void free_config(SysConfig* config) {
    free(config);
}