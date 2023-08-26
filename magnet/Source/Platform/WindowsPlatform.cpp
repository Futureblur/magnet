#ifdef _WIN32

#include "Platform.h"

#include <windows.h>

namespace MG
{
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