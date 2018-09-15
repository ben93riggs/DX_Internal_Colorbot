#pragma once
#include <Windows.h>

namespace RID
{
	void Hook();
	void Unhook();
	UINT __stdcall NtUserGetRawInputDataHook(HRAWINPUT hRawInput, UINT Command, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
}