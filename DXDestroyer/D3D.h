#pragma once
#include <d3d11.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace D3D
{
	void Hook();
	void Unhook();
	HRESULT __stdcall PresentHook(IDXGISwapChain* ecx, UINT SyncInterval, UINT Flags);
}