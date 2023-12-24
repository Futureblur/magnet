#pragma once

// Creates the function declaration for a given command.
#define MG_DEFINE_COMMAND(name) static void Handle##name##Command(const CommandHandlerProps& props)

namespace MG
{
	class Project;

	struct CommandLineArguments;

	// Represents the properties of a command.
	struct CommandHandlerProps
	{
		Project* project;

		// Returns the argument by index.
		[[maybe_unused]] [[nodiscard]] std::string GetArgument(uint32_t index) const;

		// Consolidates all the arguments into a single string.
		[[nodiscard]] std::string ConvertArgumetsToString() const;

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
		MG_DEFINE_COMMAND(Help);
		MG_DEFINE_COMMAND(Version);
		MG_DEFINE_COMMAND(Config);
		MG_DEFINE_COMMAND(New);
		MG_DEFINE_COMMAND(Generate);
		MG_DEFINE_COMMAND(Build);
		MG_DEFINE_COMMAND(Go);
		MG_DEFINE_COMMAND(Clean);
		MG_DEFINE_COMMAND(Pull);
		MG_DEFINE_COMMAND(PullList);
		MG_DEFINE_COMMAND(Remove);
		MG_DEFINE_COMMAND(Switch);

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

		// Returns the custom CMake code specified by the user from a CMakeLists.txt file path.
		static std::string ExtractCustomCMakeCode(const std::filesystem::path& path);

		// Returns the name of the repository from the given URL.
		static std::string ExtractRepositoryName(const std::string& url);

		// Writes the given dependencies to the .magnet/dependencies file.
		// If the path is empty, it will write to the default path.
		static bool WriteDependencyFile(const std::vector<std::string>& dependencies,
		                                const std::filesystem::path& path = "");


		// Returns whether the given project name is valid.
		static bool RequireProjectName(const CommandHandlerProps& props);

		// Executes the given command and returns whether it was successful.
		static bool ExecuteCommand(const std::string& command, const std::string& errorMessage);
	};
}
