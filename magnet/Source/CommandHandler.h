#pragma once

#include <iostream>

namespace MG
{
	struct CommandLineArguments;

	// Responsible for handling all of Magnet's command logic.
	class CommandHandler
	{
	public:
		static void HandleNewCommand(const CommandLineArguments* args, int index);
		static void HandleGenerateCommand();
		static void HandleBuildCommand();
		static void HandleGoCommand();
		static void HandleCleanCommand();
		static void HandlePullCommand(const CommandLineArguments* args, int index);
		static void HandlePullListCommand();
		static void HandleRemoveCommand(const CommandLineArguments* args, int index);
	private:
		// Creates a new project by initializing the template folder and
		// generating a unique config.yaml file inside the .magnet folder.
		static void CreateNewProject(const std::string& name, const std::string& type);

		// Creates a CMakeLists.txt file at the root of the project.
		static bool GenerateRootCMakeFile();

		// Scans the Source folder for .h / .cpp files and generates a fresh
		// CMakeLists.txt files based on that.
		static bool GenerateCMakeFiles();

		// Generates a CMakeLists.txt file inside of Dependencies folder
		// based on installed packages.
		static bool GenerateDependencyCMakeFiles();

		// Returns the name of the repository from the given URL.
		static std::string ExtractRepositoryName(const std::string& url);

		// Writes the given dependencies to the .magnet/dependencies file.
		// If the path is empty, it will write to the default path.
		static bool WriteDependencyFile(const std::vector<std::string>& dependencies,
		                                const std::string& path = "");
	};
}
