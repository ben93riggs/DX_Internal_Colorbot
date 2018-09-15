#include "IAT.h"

LPVOID* GetImportAddress(LPCSTR szModuleImporter, LPCSTR szModuleImportee, LPVOID pfnFunc)
{
	HMODULE hModuleImporter = GetModuleHandleA(szModuleImporter);

	if (!hModuleImporter)
		return nullptr;

	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hModuleImporter, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
		
	if (!pImportDesc)
		return nullptr;

	for (; pImportDesc->Name; pImportDesc++)
	{
		LPCSTR pModuleName = (LPCSTR)((PBYTE)hModuleImporter + pImportDesc->Name);

		if (!(strcmp(pModuleName, szModuleImportee) == 0))
			continue;
		
		PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hModuleImporter + pImportDesc->FirstThunk);
		
		for (; pThunk->u1.Function; pThunk++)
		{
			LPVOID* pEntry = (LPVOID*)&pThunk->u1.Function;
			
			if (*pEntry == pfnFunc)
			{
				return pEntry;
			}
		}
		break;
	}
	
	
	return nullptr;
}

BOOL IAT::Hook(LPCSTR szModuleImporter, LPCSTR szModuleImportee, LPCSTR szFuncName, LPVOID pFuncNew)
{
	if (Hooked)
		return FALSE;

	HMODULE hModuleImportee = GetModuleHandleA(szModuleImportee);

	if (!hModuleImportee)
		return FALSE;

	LPVOID pfnFunc = (LPVOID)GetProcAddress(hModuleImportee, szFuncName);

	if (!pfnFunc)
		return FALSE;

	LPVOID* ImportAddr = GetImportAddress(szModuleImporter, szModuleImportee, pfnFunc);

	if (!ImportAddr)
		return FALSE;

	this->pImport = ImportAddr;
	this->pfnNew = pFuncNew;

	memcpy(&this->pfnOld, this->pImport, sizeof(LPVOID));

	DWORD flOldProtect;
	VirtualProtect(this->pImport, sizeof(LPVOID), PAGE_READWRITE, &flOldProtect);

	memcpy(this->pImport, &pFuncNew, sizeof(LPVOID));

	VirtualProtect(this->pImport, sizeof(LPVOID), flOldProtect, &flOldProtect);

	this->Hooked = TRUE;
	return TRUE;
}

BOOL IAT::UnHook()
{
	if (!Hooked)
		return FALSE;

	DWORD flOldProtect;
	VirtualProtect(this->pImport, sizeof(LPVOID), PAGE_READWRITE, &flOldProtect);

	memcpy(this->pImport, &this->pfnOld, sizeof(LPVOID));

	VirtualProtect(this->pImport, sizeof(LPVOID), flOldProtect, &flOldProtect);

	this->pfnNew = nullptr;
	this->pfnOld = nullptr;
	this->pImport = nullptr;
	this->Hooked = FALSE;

	return TRUE;
}

BOOL IAT::IsHooked() const
{
	return this->Hooked;
}
