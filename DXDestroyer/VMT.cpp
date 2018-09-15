#include "VMT.h"

BOOL VMT::Hook(PDWORD64 Table, INT funcIndex, PVOID funcNew)
{
	if (this->Hooked)
		return FALSE;
	
	this->pVTable = Table;
	this->pfnNew = funcNew;
	this->indexHook = funcIndex;

	if (IsBadReadPtr(this->pVTable, sizeof(DWORD64) * (funcIndex + 1)))
		return FALSE;
	
	DWORD OldProtect;
	VirtualProtect(&this->pVTable[funcIndex], sizeof(DWORD64), PAGE_READWRITE, &OldProtect);

	this->pfnOld = (PVOID)this->pVTable[funcIndex];
	this->pVTable[funcIndex] = (DWORD64)this->pfnNew;

	VirtualProtect(&this->pVTable[funcIndex], sizeof(DWORD64), OldProtect, &OldProtect);

	this->Hooked = TRUE;
	return TRUE;
}

BOOL VMT::UnHook()
{
	if (!this->Hooked)
		return FALSE;
	
	DWORD OldProtect;
	VirtualProtect(&this->pVTable[this->indexHook], sizeof(DWORD64), PAGE_READWRITE, &OldProtect);

	memcpy(&this->pVTable[this->indexHook], &this->pfnOld, sizeof(DWORD64));

	//this->pVTable[this->indexHook] = (DWORD64)this->pfnOld;

	VirtualProtect(&this->pVTable[this->indexHook], sizeof(DWORD64), OldProtect, &OldProtect);

	this->Hooked = FALSE;
	return TRUE;
}

BOOL VMT::IsHooked() const
{
	return this->Hooked;
}
