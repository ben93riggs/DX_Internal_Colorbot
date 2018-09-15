#pragma once
#include <Windows.h>
#include <DbgHelp.h>

#pragma comment(lib, "DbgHelp.lib")

class IAT
{
	BOOL Hooked = FALSE;

public:

	LPVOID pfnOld = nullptr;
	LPVOID pfnNew = nullptr;
	LPVOID* pImport = nullptr;

	BOOL IsHooked() const;
	BOOL Hook(LPCSTR szModuleImporter, LPCSTR szModuleImportee, LPCSTR szFuncName, LPVOID pFuncNew);
	BOOL UnHook();

};

