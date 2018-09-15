#include "Utils.h"
#include <TlHelp32.h>
#include <vector>

bool bAllThreadsSuspended = false;
std::vector<DWORD> vecSuspendedThreadIds;

bool Utils::SuspendAllThreadsExceptCurrent()
{
	if (!bAllThreadsSuspended)
	{
		THREADENTRY32 Entry32;
		Entry32.dwSize = sizeof(THREADENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
		DWORD CurrentProcessId = GetCurrentProcessId();
		DWORD CurrentThreadId = GetCurrentThreadId();
		if (hSnapshot)
		{
			vecSuspendedThreadIds.clear();
			BOOL Status = Thread32First(hSnapshot, &Entry32);
			while (Status)
			{
				if (Entry32.th32OwnerProcessID == CurrentProcessId)
				{
					if (Entry32.th32ThreadID != CurrentThreadId)
					{
						vecSuspendedThreadIds.push_back(Entry32.th32ThreadID);
					}
				}
				Status = Thread32Next(hSnapshot, &Entry32);
			}
			CloseHandle(hSnapshot);
			for (DWORD ThreadId : vecSuspendedThreadIds)
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, false, ThreadId);
				SuspendThread(hThread);
				CloseHandle(hThread);
			}
			bAllThreadsSuspended = true;
			return true;
		}
	}
	return false;
}

void Utils::ResumeAllThreadsExceptCurrent()
{
	if (bAllThreadsSuspended)
	{
		for (DWORD ThreadId : vecSuspendedThreadIds)
		{
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, false, ThreadId);
			ResumeThread(hThread);
			CloseHandle(hThread);
		}
		vecSuspendedThreadIds.clear();
		bAllThreadsSuspended = false;
	}
}

RECT Utils::CreateRect(int x, int y, int w, int h)
{
	RECT r;
	r.left = x;
	r.top = y;
	r.right = x + w;
	r.bottom = y + h;
	return r;
}

float Utils::IHypotenuseF(int x, int y, int x1, int y1)
{
	int dx = x1 - x;
	int dy = y1 - y;
	return sqrtf(dx * dx + dy * dy);
}