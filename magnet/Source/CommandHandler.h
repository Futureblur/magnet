#pragma once

namespace MG
{
	class Project;

	struct CommandLineArguments;

	// Represents the properties of a command.
	struct CommandHandlerProps
	{
		Project* project;

		// Returns the argument by index.
		[[nodiscard]] std::string GetArgument(uint32_t index) const;

		// Returns whether there are any arguments left.
		[[maybe_unused]] [[nodiscard]] bool HasArguments() const;

	private:
		std::vector<std::string> nextArguments;

		friend class Application;
	};

	// Responsible for handling all of Magnet's command logic.
	class CommandHandler
	{
	public:
		static void HandleHelpCommand(const CommandHandlerProps& props);
		static void HandleVersionCommand(const CommandHandlerProps& props);
		static void HandleNewCommand(const CommandHandlerProps& props);
		static void HandleGenerateCommand(const CommandHandlerProps& props);
		static void HandleBuildCommand(const CommandHandlerProps& props);
		static void HandleGoCommand(const CommandHandlerProps& props);
		static void HandleCleanCommand(const CommandHandlerProps& props);
		static void HandlePullCommand(const CommandHandlerProps& props);
		static void HandlePullListCommand(const CommandHandlerProps& props);
		static void HandleRemoveCommand(const CommandHandlerProps& props);
		static void HandleSwitchCommand(const CommandHandlerProps& props);

		// Returns whether the given command is global, meaning it doesn't
		// require a project to be present.
		static bool IsCommandGlobal(const std::string& command);
	private:
		// Creates a new project by initializing the template folder and
		// generating a unique config.yaml file inside the .magnet folder.
		static void CreateNewProject(const Project& project);

		// Creates a CMakeLists.txt file at the root of the project.
		static bool GenerateRootCMakeFile(const CommandHandlerProps& props);

		// Scans the Source folder for .h / .cpp files and generates a fresh
		// CMakeLists.txt files based on that.
		static bool GenerateCMakeFiles(const CommandHandlerProps& props);

		// Generates a CMakeLists.txt file inside of Dependencies folder
		// based on installed packages.
		static bool GenerateDependencyCMakeFiles(const CommandHandlerProps& props);

		// Returns the name of the repository from the given URL.
		static std::string ExtractRepositoryName(const std::string& url);

		// Writes the given dependencies to the .magnet/dependencies file.
		// If the path is empty, it will write to the default path.
		static bool WriteDependencyFile(const std::vector<std::string>& dependencies,
		                                const std::string& path = "");

		static bool RequireProjectName(const CommandHandlerProps& props);

		static bool ExecuteCommand(const std::string& command, const std::string& errorMessage);
	};
}
