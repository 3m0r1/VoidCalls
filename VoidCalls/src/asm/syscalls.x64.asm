PUBLIC SysSetConfig
PUBLIC SysSetResult
PUBLIC SysInvokeDirect
PUBLIC SysInvokeIndirect

.code

SysSetConfig PROC
	mov r12, rcx
	ret
SysSetConfig ENDP

SysSetResult PROC
	mov [rcx], rax
	ret
SysSetResult ENDP

SysInvokeDirect PROC
	mov r10, rcx
	mov eax, dword ptr [r12]
	syscall
	ret
SysInvokeDirect ENDP

SysInvokeIndirect PROC
	mov r10, rcx
	mov eax, dword ptr [r12]
	jmp qword ptr [r12 + 8]
SysInvokeIndirect ENDP

END