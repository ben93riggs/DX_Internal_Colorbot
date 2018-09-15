#include "CLog.h"
#include <fstream>
#include <iostream>

void CLog::Log(const std::string& text)
{
	printf("%s\n", text.c_str());

	try
	{
		std::ofstream log_file("C:\\users\\ben93\\Desktop\\log.txt", std::ios_base::out | std::ios_base::app);
		if (log_file.is_open())
		{
			log_file << GetTime() + " | " + text << std::endl;
			log_file.close();
		}
		else
		{
			throw std::exception(std::string("Log file specified not found.").c_str());
		}
	}
	catch (std::exception& e)
	{
		printf("%s\n", e.what());
	}
}
