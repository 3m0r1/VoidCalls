#include "clipboard.h"

void run_clipboard_example() {
    // VoidCalls leaves module-loading up to the user, For simplicity here we use LoadLibraryA.
    LoadLibraryA("win32u.dll");

    OpenClipboard(
        NULL
    );

    SysCtx* ctx = INIT_CTX_WIN32U();
    SysConfig* config = GET_SYS_CONFIG(ctx, NtUserEmptyClipboard);

    PRINT_CONFIG(config);

    NTSTATUS status;
    INVOKE_INDIRECT(
        config,
        NULL
    );

    CHECK_NTSTATUS(
        NtUserEmptyClipboard,
        status,
        "Successfully emptied user clipboard\n"
    );

    free_ctx(ctx);
    free_config(config);
}