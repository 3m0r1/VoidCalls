#include "./includes/syscalls.h"

const BYTE SYSCALL_SIGNATURE[] = { 0x0f, 0x05 };

SysCtx* init_ctx(ModuleType module_type, BOOL get_syscall_addr) {
    SysCtx* ctx = (SysCtx*)calloc(1, sizeof(SysCtx));

    if (!ctx)
        return NULL;

    ctx->type = module_type;
    ctx->get_syscall_addr = get_syscall_addr;
    ctx->pe = parse_image(ctx->type);

    return ctx;
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

    DWORD exception_index = 0;
    WORD ssn = 0;

    while (exception_dir[exception_index].BeginAddress) {
        for (size_t i = 0; i < export_dir->NumberOfFunctions; i++) {
            LPCSTR fn_name = (LPCSTR)(image_base + name_rvas[i]);

            if (strncmp(fn_name, "Zw", 2))
                continue;

            WORD fn_ord = ordinals[i];
            DWORD fn_rva = function_rvas[fn_ord];

            if (fn_rva == exception_dir[exception_index].BeginAddress) {
                if (djb2A(fn_name) == fn_hash) {
                    config->ssn = ssn;

                    // required for indirect syscalls
                    if (ctx->get_syscall_addr) {
                        PVOID fn_addr = (PVOID)(image_base + fn_rva);

                        for (size_t x = 0; x < STUB_SIZE; x += 2) {
                            PVOID curr_addr = (PVOID)((DWORD_PTR)fn_addr + x);

                            if (!memcmp(curr_addr, SYSCALL_SIGNATURE, sizeof(SYSCALL_SIGNATURE))) {
                                config->jump_addr = curr_addr;
                                break;
                            }
                        }
                    }
                    else {
                        break;
                    }
                }

                ssn++;
            }
        }

        exception_index++;
    }

    return config;
}

void free_ctx(SysCtx* ctx) {
    free(ctx);
}

void free_config(SysConfig* config) {
    free(config);
}