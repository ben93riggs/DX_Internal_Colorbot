#pragma once
#include <string>
#include <ctime>

#define LOG CLog()

inline std::string GetTime()
{
	time_t rawtime;
	tm* timeinfo = nullptr;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", timeinfo);
	return std::string(buffer);
}

class CLog
{
public:
	static void Log(const std::string& text);

	template<typename T>
	CLog &operator << (const T &t)
	{
		Log(t);
		return *this;
	}
};
