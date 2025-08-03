#pragma once

#include<windows.h>

#include "pe.h"

#define STUB_SIZE 23
#define WIN32U_TRAVERSAL 32

#define DEREF_BYTE(x) *(BYTE*)(x)
#define DEREF_WORD(x) *(WORD*)(x)

namespace signatures {
	constexpr BYTE SYSCALL_SIGNATURE[] = { 0x0f, 0x05 };
	constexpr BYTE SYSCALL_START[] = { 0x4c, 0x8b, 0xd1, 0xb8 };
}

namespace hashes {
	constexpr DWORD Ntdll = 0x22d3b5ed;
	constexpr DWORD Win32u = 0x34c755b7;
}

enum class ModuleType {
	Ntdll,
	Win32u
};

struct InvokerConfig {
	WORD Ssn;
	ULONG_PTR JumpAddress;
};

extern "C" {
	void SysSetConfig(InvokerConfig config);
	void SysSetResult(PVOID output);

	NTSTATUS SysInvokeDirect(...);
	NTSTATUS SysInvokeIndirect(...);
}

template<typename Ret>
class Invoker {
public:
	InvokerConfig Config;

	Invoker(InvokerConfig config) : Config{ config } {}

	template <typename... Args>
	Ret InvokeDirect(Args... args) {
		Ret result{ 0 };

		SysSetConfig(this->Config);
		SysInvokeDirect(args...);
		SysSetResult(&result);

		return result;
	}

	template <typename... Args>
	Ret InvokeIndirect(Args... args) {
		Ret result{ 0 };

		SysSetConfig(this->Config);
		SysInvokeIndirect(args...);
		SysSetResult(&result);

		return result;
	}

};

class Factory {
private:
	InvokerConfig GetNtdllConfig(DWORD syscallHash);
	InvokerConfig GetWin32uConfig(DWORD syscallHash);

public:
	bool GetSyscall;
	ModuleType Module;
	PeImage* Pe;

	Factory(ModuleType moduleType, bool getSyscall) {
		this->Module = moduleType;
		this->GetSyscall = getSyscall;

		if (this->Module == ModuleType::Ntdll) {
			this->Pe = new PeImage(hashes::Ntdll);
		}
		else if (this->Module == ModuleType::Win32u) {
			this->Pe = new PeImage(hashes::Win32u);
		}
		else {
			this->Pe = new PeImage(hashes::Ntdll);
		}
	}

	static ULONG_PTR GetSyscallInstruction(ULONG_PTR fnAddr);
	static WORD GetSsn(ULONG_PTR fnAddr);
	static WORD GetSsnTraverse(ULONG_PTR fnAddr, DWORD offset);

	template <typename Ret>
	Invoker<Ret>* CreateInvoker(DWORD syscallHash) {
		if (this->Module == ModuleType::Ntdll) {
			auto config = this->GetNtdllConfig(syscallHash);
			return new Invoker<Ret>(config);
		}
		else if (this->Module == ModuleType::Win32u) {
			auto config = this->GetWin32uConfig(syscallHash);
			return new Invoker<Ret>(config);
		}
		else {
			auto config = this->GetNtdllConfig(syscallHash);
			return new Invoker<Ret>(config);
		}
	}
};