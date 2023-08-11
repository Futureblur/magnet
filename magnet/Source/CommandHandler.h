#pragma once

#include <iostream>

namespace MG
{
	// Responsible for handling all of Magnet's command logic.
	class CommandHandler
	{
	public:
		static void HandleNewCommand(struct CommandLineArguments* args, int index);
		static void HandleGenerateCommand();
		static void HandleBuildCommand();
		static void HandleGoCommand();
		static void HandleCleanCommand();
		static void HandlePullCommand();
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
	};
}
