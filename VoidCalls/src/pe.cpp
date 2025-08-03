#include "../include/pe.h"

ULONG_PTR CustomGetModule(DWORD hash) {
    PPEB peb = (PPEB)__readgsqword(0x60);

    PLIST_ENTRY head = &peb->Ldr->InMemoryOrderModuleList;
    PLIST_ENTRY entry = head->Flink;

    while (entry != head) {
        PLDR_DATA_TABLE_ENTRY data = (PLDR_DATA_TABLE_ENTRY)entry;

        if (hash == hash::Djb2W(data->FullDllName.Buffer)) {
            return reinterpret_cast<ULONG_PTR>(data->Reserved2[0]);
        }

        entry = entry->Flink;
    }

    return NULL;
}

void PeImage::InitHeaders() {
    this->DosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(this->ImageBase);
    this->NtHeaders = this->FromBase<PIMAGE_NT_HEADERS>(this->DosHeader->e_lfanew);
}

void PeImage::InitDirectories() {
    this->ExportDirectory = this->FromBase<PIMAGE_EXPORT_DIRECTORY>(this->NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    this->ExceptionDirectory = this->FromBase<PIMAGE_RUNTIME_FUNCTION_ENTRY>(this->NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);

    this->Exports = PeExports{ 0 };

    this->Exports.NameRvas = this->FromBase<PDWORD>(this->ExportDirectory->AddressOfNames);
    this->Exports.Ordinals = this->FromBase<PWORD>(this->ExportDirectory->AddressOfNameOrdinals);
    this->Exports.FunctionRvas = this->FromBase<PDWORD>(this->ExportDirectory->AddressOfFunctions);
}

PeImage::PeImage(DWORD modHash) {
    this->ImageBase = CustomGetModule(modHash);
    this->InitHeaders();
    this->InitDirectories();
}