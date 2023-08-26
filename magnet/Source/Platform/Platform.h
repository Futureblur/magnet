#pragma once

namespace MG
{
	class Platform
	{
	public:
		// Returns the real path to the executable. Does not include the executable name.
		static std::filesystem::path GetExecutablePath();
	};
}