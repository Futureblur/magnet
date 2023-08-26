#ifdef __APPLE__

#include "Platform.h"

#include <mach-o/dyld.h>

namespace MG
{
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
}

#endif