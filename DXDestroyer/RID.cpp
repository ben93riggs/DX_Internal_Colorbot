#include "RID.h"
#include "IAT.h"
#include "CLog.h"

typedef UINT(__stdcall *TNTUserGetRawInputData)(HRAWINPUT hRawInput, UINT Command, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
IAT IATNTUserGetRID;

namespace RID
{
	void Hook()
	{
		if (!IATNTUserGetRID.Hook("User32.dll", "win32u.dll", "NtUserGetRawInputData", NtUserGetRawInputDataHook))
			LOG << "Failed to hook NTUserGetRID.";
	}

	void Unhook()
	{
		if (!IATNTUserGetRID.UnHook())
			LOG << "Failed to unhook NTUserGetRID";
	}

	UINT __stdcall NtUserGetRawInputDataHook(HRAWINPUT hRawInput, UINT Command, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
	{
		UINT ret = ((TNTUserGetRawInputData)IATNTUserGetRID.pfnOld)(hRawInput, Command, pData, pcbSize, cbSizeHeader);

		RAWINPUT* pRawInput = (RAWINPUT*)pData;

		if (pRawInput->header.dwType == RIM_TYPEMOUSE)
		{
			if (pRawInput->header.hDevice == nullptr)
			{
				pRawInput->header.hDevice = (HANDLE)0x1000;
			}
		}

		return ret;
	}
}
