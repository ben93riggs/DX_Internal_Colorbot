#pragma once
#include <Windows.h>

typedef void** PPVOID;
class VMT
{
	BOOL Hooked = FALSE;

public:
	PVOID pfnOld, pfnNew;
	PDWORD64 pVTable;
	INT indexHook;

	BOOL Hook(PDWORD64 Table, INT funcIndex, PVOID funcNew);
	BOOL UnHook();

	BOOL IsHooked() const;
};

