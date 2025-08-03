#include "../include/factory.h"

ULONG_PTR Factory::GetSyscallInstruction(ULONG_PTR fnAddr) {
	for (size_t x = 0; x < STUB_SIZE; x += 2) {
		ULONG_PTR currAddr = (fnAddr + x);

		if (!memcmp((PVOID)currAddr, signatures::SYSCALL_SIGNATURE, sizeof(signatures::SYSCALL_SIGNATURE))) {
			return currAddr;
		}
	}

	return NULL;
}

WORD Factory::GetSsn(ULONG_PTR fnAddr) {
	if (!memcmp((PVOID)fnAddr, signatures::SYSCALL_START, sizeof(signatures::SYSCALL_START))) {
		BYTE low = DEREF_BYTE(fnAddr + 0x4);
		BYTE high = DEREF_BYTE(fnAddr + 0x5);

		WORD ssn = high << 8 | low;

		return ssn;
	}
	return 0;
}


WORD Factory::GetSsnTraverse(ULONG_PTR fnAddr, DWORD offset) {
	WORD ssn = GetSsn(fnAddr);

	if (ssn != 0) {
		return ssn;
	}
	else {
		int traverse = 0;

		while (true) {
			WORD nextSsn = GetSsn(fnAddr + (traverse * offset));

			if (nextSsn != 0) {
				return nextSsn - traverse;
			}
			else {
				traverse++;
			}
		}
	}
}

InvokerConfig Factory::GetNtdllConfig(DWORD syscallHash) {
	auto pe = this->Pe;

	auto exportDir = pe->ExportDirectory;
	auto exceptionDir = pe->ExceptionDirectory;

	PDWORD nameRvas = pe->Exports.NameRvas;
	PWORD ordinals = pe->Exports.Ordinals;
	PDWORD fnRvas = pe->Exports.FunctionRvas;

	auto config = InvokerConfig{};

	DWORD index = 0;
	WORD ssn = 0;

	while (DWORD beginAddress = exceptionDir[index].BeginAddress) {
		for (DWORD i = 0; i < exportDir->NumberOfFunctions; i++)
		{
			LPCSTR name = pe->FromBase<LPCSTR>(nameRvas[i]);

			if (DEREF_WORD(name) != 'wZ')
				continue;

			WORD ordinal = ordinals[i];
			DWORD fnRva = fnRvas[ordinal];

			if (beginAddress == fnRva) {

				if (syscallHash == hash::Djb2A(name)) {
					config.Ssn = ssn;

					ULONG_PTR fnAddr = pe->FromBase<ULONG_PTR>(fnRva);

					if (this->GetSyscall) {
						config.JumpAddress = GetSyscallInstruction(fnAddr);
					}

				}
				ssn++;
			}

		}

		index++;
	}

	return config;
}

InvokerConfig Factory::GetWin32uConfig(DWORD syscallHash) {
	auto pe = this->Pe;

	auto exportDir = pe->ExportDirectory;

	PDWORD nameRvas = pe->Exports.NameRvas;
	PWORD ordinals = pe->Exports.Ordinals;
	PDWORD fnRvas = pe->Exports.FunctionRvas;

	auto config = InvokerConfig{};

	for (DWORD i = 0; i < exportDir->NumberOfFunctions; i++)
	{
		LPCSTR name = pe->FromBase<LPCSTR>(nameRvas[i]);

		if (DEREF_WORD(name) != 'tN')
			continue;

		if (syscallHash == hash::Djb2A(name)) {

			WORD ordinal = ordinals[i];
			DWORD fnRva = fnRvas[ordinal];

			ULONG_PTR fnAddr = pe->FromBase<ULONG_PTR>(fnRva);

			WORD ssn = GetSsnTraverse(fnAddr, WIN32U_TRAVERSAL);

			config.Ssn = ssn;

			if (this->GetSyscall) {
				config.JumpAddress = GetSyscallInstruction(fnAddr);
			}

		}

	}

	return config;
}