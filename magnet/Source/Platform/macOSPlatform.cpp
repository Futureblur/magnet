#ifdef __APPLE__

#include "Platform.h"

#include <mach-o/dyld.h>

namespace MG
{
	void Platform::Initialize()
	{
	}

	std::filesystem::path Platform::GetExecutablePath()
	{
		char path[PATH_MAX];
		uint32_t size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0)
		{
			// Expand the path to a full pathname.
			char* realPath = realpath(path, nullptr);
			if (realPath)
			{
				strncpy(path, realPath, size);
				free(realPath);
			}
		}

		std::filesystem::path p = path;
		p.remove_filename();

		return p;
	}

	std::string Platform::GetGenerator()
	{
		return "Xcode";
	}

	std::string Platform::GetGenerateCommand(const std::string& configuration)
	{
		return "-DCMAKE_BUILD_TYPE=" + configuration + " ";
	}

	std::string Platform::GetGoCommand(const std::string& appPath)
	{
		return "./" + appPath;
	}
}

#endif
