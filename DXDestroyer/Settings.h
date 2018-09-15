#pragma once
#include <Windows.h>
#include <vector>

namespace Settings
{
	extern int PixelStep;
	extern int SleepFrames;
	extern RECT ScanRect;
	extern std::vector<RECT> ExclusionRects;
	extern int AimOffsetX;
	extern int AimOffsetY;
	extern COLORREF AimColor;
	extern byte AimKey;
	extern float AimRadius;
	extern float AimSpeedX;
	extern float AimSpeedY;
}