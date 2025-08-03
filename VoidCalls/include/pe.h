#pragma once

#include<windows.h>
#include<winternl.h>

#include "hash.h"

struct PeExports {
	PDWORD NameRvas;
	PWORD Ordinals;
	PDWORD FunctionRvas;
};

class PeImage {
public:
	ULONG_PTR ImageBase;

	PIMAGE_DOS_HEADER DosHeader;
	PIMAGE_NT_HEADERS NtHeaders;

	PIMAGE_EXPORT_DIRECTORY ExportDirectory;
	PIMAGE_RUNTIME_FUNCTION_ENTRY ExceptionDirectory;

	PeExports Exports;

	PeImage(DWORD modHash);

	template<typename T>
	T FromBase(DWORD rva) {
		return (T)(this->ImageBase + rva);
	}

	void InitHeaders();
	void InitDirectories();
};