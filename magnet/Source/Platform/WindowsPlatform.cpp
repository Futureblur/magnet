#ifdef _WIN32

#include "Platform.h"

namespace MG
{
	std::filesystem::path Platform::GetExecutablePath()
	{
		return std::filesystem::current_path();
	}
}

#endif