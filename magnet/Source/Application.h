#pragma once

#include <iostream>

namespace MG
{
	struct CommandLineArguments
	{
		int count;
		const char** list;
	};

	// The heart of Magnet.
	class Application
	{
	public:
		Application() = default;
		~Application() = default;

		static void Init(const CommandLineArguments& args);
		static void Print(const std::string& host, const std::string& message);
		static void PrintPrompt();
		static void Run();

		static std::string GetCurrentWorkingDirectory();
		static std::string GetProjectName();
		static std::string GetProjectType();
		static int GetCppVersion();
		static std::string GetDefaultConfiguration();
		static std::vector<std::string> GetDependencies();
		static bool IsRootLevel();

	private:
		static inline CommandLineArguments m_Arguments;
	};
}

