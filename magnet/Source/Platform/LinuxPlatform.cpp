#ifdef __linux__

#include "Platform.h"

#include <limits.h>
#include <unistd.h>

namespace MG
{
	void Platform::Initialize()
	{
	}

	std::filesystem::path Platform::GetExecutablePath()
	{
		char path[PATH_MAX];
		uint32_t size = sizeof(path);

		if (readlink("/proc/self/exe", path, size) == -1)
		{
			return "";
		}

		char* realPath = realpath(path, nullptr);
		if (realPath)
		{
			strncpy(path, realPath, size);
			free(realPath);
		}

		std::filesystem::path p = path;
		p.remove_filename();

		return p;
	}

	std::string Platform::GetGenerateCommand([[maybe_unused]] const std::string& configuration)
	{
		return "-G \"Ninja\" -DCMAKE_BUILD_TYPE=" + configuration;
	}

	std::string Platform::GetGoCommand(const std::string& appPath)
	{
		return "./" + appPath;
	}
}

#endif