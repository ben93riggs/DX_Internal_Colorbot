#pragma once
#include <Windows.h>

namespace Utils
{
	extern bool SuspendAllThreadsExceptCurrent();
	extern void ResumeAllThreadsExceptCurrent();
	extern RECT CreateRect(int x, int y, int w, int h);
	extern float IHypotenuseF(int x, int y, int x1, int y1);
}