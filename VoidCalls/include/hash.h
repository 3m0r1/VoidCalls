#pragma once

#include<windows.h>

namespace hash {
	DWORD Djb2A(LPCSTR str);
	DWORD Djb2W(LPCWSTR str);
}