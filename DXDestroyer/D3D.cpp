#include "D3D.h"
#include "Settings.h"
#include "Utils.h"


#include "VMT.h"
#include "CLog.h"



#define INRECT(x, y, r) (x <= r.right && x >= r.left && y <= r.bottom && y >= r.top)

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
static VMT VMTPresentHook;

namespace D3D
{
	int FrameSleepCounter = 0;
	bool bD3DInitialized = false;
	ID3D11Device* Device;
	ID3D11DeviceContext* Context;
	IDXGIResource* Resource;
	ID3D11Texture2D* StagingTexture;
	ID3D11Texture2D* DestTexture;
	D3D11_MAPPED_SUBRESOURCE MappedSubresource;
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;

	int BufferCX = 0;
	int BufferCY = 0;
	int CrosshairX = 0;
	int CrosshairY = 0;

	struct Target
	{
		POINT Origin;
		RECT HealthExclusion;
		float DeltaCrosshair;

		Target(int healthX, int healthY)
		{
			Origin.x = healthX + Settings::AimOffsetX;
			Origin.y = healthY + Settings::AimOffsetY;
			HealthExclusion.left = healthX - 50;
			HealthExclusion.top = healthY - 10;
			HealthExclusion.right = healthX + 150;
			HealthExclusion.bottom = healthY + 50;
			DeltaCrosshair = Utils::IHypotenuseF(CrosshairX, CrosshairY, Origin.x, Origin.y);
		}
		Target()
		{
			ZeroMemory(this, sizeof(Target));
		}
	} bestTarget;

	std::vector<Target> TargetList;

	bool PointExcluded(int x, int y)
	{
		for (Target target : TargetList)
		{
			if (INRECT(x, y, target.HealthExclusion))
				return true;
		}
		return false;
	}

	void* CreateDummySwap()
	{
		DXGI_SWAP_CHAIN_DESC swapDesc;
		ZeroMemory(&swapDesc, sizeof(swapDesc));
		swapDesc.BufferCount = 2;
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.Width = 2;
		swapDesc.BufferDesc.Height = 2;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.OutputWindow = FindWindowA(NULL, "ProcessWindowNameHere");
		swapDesc.SampleDesc.Count = 1;
		swapDesc.Windowed = TRUE;
		IDXGISwapChain* TempSwap;
		ID3D11Device* TempDevice;
		ID3D11DeviceContext* TempContext;
		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
		D3D10_DRIVER_TYPE driverType10 = D3D10_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL desiredLevels[6] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};
		D3D_FEATURE_LEVEL receivedLevel;
		HRESULT hErr = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, 0, desiredLevels, 6, D3D11_SDK_VERSION, &swapDesc, &TempSwap, &TempDevice, &receivedLevel, &TempContext);
		if (SUCCEEDED(hErr))
		{
			DWORD64* pDws = (DWORD64*)TempSwap;
			LPVOID pVtable = (LPVOID)(pDws[0]);
			TempSwap->Release();
			TempDevice->Release();
			TempContext->Release();
			return pVtable;
		}
		return nullptr;
	}

	void Hook()
	{
		void* SwapChainVTable = CreateDummySwap();

		if (!SwapChainVTable)
		{
			LOG << "Failed to get swap table";
			return;
		}
		
		if (VMTPresentHook.IsHooked())
		{
			LOG << "D3D Present is already hooked!";
			return;
		}
			
		if (!VMTPresentHook.Hook((PDWORD64)SwapChainVTable, 8, PresentHook))
			LOG << "Failed to hook D3D Present.";
	}

	void Unhook()
	{
		if (VMTPresentHook.IsHooked())
		{
			bD3DInitialized = false;
			VMTPresentHook.UnHook();
		}
	}

	void InitD3D(IDXGISwapChain* swapchain)
	{
		if (SUCCEEDED(swapchain->GetDevice(__uuidof(ID3D11Device), (void**)&Device)))
		{
			Device->GetImmediateContext(&Context);
			DXGI_SWAP_CHAIN_DESC SwapDesc;
			if (SUCCEEDED(swapchain->GetDesc(&SwapDesc)))
			{
				BufferCX = SwapDesc.BufferDesc.Width;
				BufferCY = SwapDesc.BufferDesc.Height;
				CrosshairX = BufferCX / 2;
				CrosshairY = BufferCY / 2;
				D3D11_TEXTURE2D_DESC texDesc;
				texDesc.Width = SwapDesc.BufferDesc.Width;
				texDesc.Height = SwapDesc.BufferDesc.Height;
				texDesc.Format = SwapDesc.BufferDesc.Format;
				texDesc.ArraySize = 1;
				texDesc.BindFlags = 0;
				texDesc.MiscFlags = 0;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.MipLevels = 1;
				texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
				texDesc.Usage = D3D11_USAGE_STAGING;
				if (SUCCEEDED(Device->CreateTexture2D(&texDesc, NULL, &DestTexture)))
				{
					bD3DInitialized = true;
				}
			}
		}
	}

	HRESULT __stdcall PresentHook(IDXGISwapChain* ecx, UINT SyncInterval, UINT Flags)
	{
		if (!bD3DInitialized)
		{
			InitD3D(ecx);
			return ((D3D11PresentHook)VMTPresentHook.pfnOld)(ecx, SyncInterval, Flags);
		}

		if (FrameSleepCounter >= Settings::SleepFrames)
		{
			if (SUCCEEDED(ecx->GetBuffer(0, __uuidof(IDXGIResource), (void**)&Resource)))
			{
				if (SUCCEEDED(Resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&StagingTexture)))
				{
					Context->CopyResource(DestTexture, StagingTexture);
					if (SUCCEEDED(Context->Map(DestTexture, 0, D3D11_MAP_READ, 0, &MappedSubresource)))
					{
						TargetList.clear();

						COLORREF* pPixels = (COLORREF*)MappedSubresource.pData;

						for (int i = 0; i < BufferCX * BufferCY; i += Settings::PixelStep)
						{
							if (pPixels[i] != Settings::AimColor)
								continue;

							int nx = i % BufferCX;
							int ny = i / BufferCX;

							if (!PointExcluded(nx, ny))
								TargetList.push_back(Target(nx, ny));
						}

						if (TargetList.size() > 0)
						{
							bestTarget = TargetList[0];

							for (Target target : TargetList)
							{
								if (target.DeltaCrosshair < bestTarget.DeltaCrosshair)
									bestTarget = target;
							}

							if (bestTarget.DeltaCrosshair <= Settings::AimRadius)
							{
								if (GetAsyncKeyState(Settings::AimKey) & 0x8000)
								{
									mouse_event(0x01, (bestTarget.Origin.x - CrosshairX) * Settings::AimSpeedX, (bestTarget.Origin.y - CrosshairY) * Settings::AimSpeedY, 0, 0);
								}
							}
						}

						Context->Unmap(DestTexture, 0);
					}
				}
			}
			FrameSleepCounter = 0;
		}

		FrameSleepCounter++;
		return ((D3D11PresentHook)VMTPresentHook.pfnOld)(ecx, SyncInterval, Flags);
	}
}
