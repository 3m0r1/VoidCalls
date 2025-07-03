#pragma once

#include<windows.h>
#include<winternl.h>

#include "hash.h"

typedef enum ModuleType {
	Ntdll,
	Win32u,
} ModuleType;

typedef struct PeHeaders {
	PIMAGE_DOS_HEADER dos_hdr;
	PIMAGE_NT_HEADERS nt_hdr;
} PeHeaders;

typedef struct PeDirectories {
	PIMAGE_EXPORT_DIRECTORY export_dir;
	PIMAGE_RUNTIME_FUNCTION_ENTRY exception_dir;
} PeDirectories;

typedef struct Pe {
	PeHeaders headers;
	PeDirectories directories;
} Pe;

Pe parse_image(ModuleType module_type);