#include "./includes/pe.h"

PVOID custom_get_module(DWORD module_hash) {
    PPEB peb = (PPEB)__readgsqword(0x60);

    PLIST_ENTRY head = &peb->Ldr->InMemoryOrderModuleList;
    PLIST_ENTRY entry = head->Flink;

    while (entry != head) {
        PLDR_DATA_TABLE_ENTRY data = (PLDR_DATA_TABLE_ENTRY)entry;

        if (module_hash == djb2W(data->FullDllName.Buffer)) {
            return data->Reserved2[0];
        }

        entry = entry->Flink;
    }

    return NULL;
}

Pe parse_image(ModuleType type) {
    Pe pe = { 0 };
    PVOID image_base = NULL;

    if (type == Ntdll) {
        image_base = custom_get_module(djb2W(L"ntdll.dll"));
    }
    else if (type == Win32u) {
        image_base = custom_get_module(djb2W(L"win32u.dll"));
    }

    if (!image_base) {
        return pe;
    }

    DWORD_PTR pe_base = (DWORD_PTR)image_base;

    PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)image_base;
    PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)(pe_base + dos_header->e_lfanew);

    IMAGE_OPTIONAL_HEADER optional_header = nt_headers->OptionalHeader;

    PIMAGE_EXPORT_DIRECTORY export_directory = (PIMAGE_EXPORT_DIRECTORY)(
        pe_base + optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    PIMAGE_RUNTIME_FUNCTION_ENTRY exception_directory = (PIMAGE_RUNTIME_FUNCTION_ENTRY)(
        pe_base + optional_header.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);

    PeHeaders headers;
    headers.dos_hdr = dos_header;
    headers.nt_hdr = nt_headers;

    PeDirectories directories;
    directories.export_dir = export_directory;
    directories.exception_dir = exception_directory;

    pe.headers = headers;
    pe.directories = directories;

    return pe;
}
