#ifdef _WIN32

#include "Platform.h"

#include <Windows.h>

namespace MG
{
	void Platform::Initialize()
	{
		SetConsoleOutputCP(CP_UTF8);
	}

	std::filesystem::path Platform::GetExecutablePath()
	{
		char path[MAX_PATH];
		GetModuleFileName(NULL, path, MAX_PATH);
		std::filesystem::path p = path;

		p.remove_filename();
		return p;
	}
}

#endif