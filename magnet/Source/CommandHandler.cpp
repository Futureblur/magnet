#include "CommandHandler.h"

#include "yaml-cpp/yaml.h"

#include "Application.h"
#include "Core.h"
#include "Project.h"

#include <regex>

namespace MG
{
	std::string CommandHandlerProps::GetArgument(uint32_t index) const
	{
		if (index >= nextArguments.size())
			return "";

		return nextArguments[index];
	}

	bool CommandHandlerProps::HasArguments() const
	{
		return !nextArguments.empty();
	}

	void CommandHandler::HandleHelpCommand([[maybe_unused]] const CommandHandlerProps& props)
	{
		MG_LOG("Usage: magnet <command> [options]\n");
		MG_LOGNH("Commands:");
		MG_LOGNH("  help                         Shows this message.");
		MG_LOGNH("  version                      Shows the current version of Magnet.");
		MG_LOGNH("  new                          Creates a new C++ project.");
		MG_LOGNH("  generate                     Generates project files.");
		MG_LOGNH("  build                        Builds the project.");
		MG_LOGNH("  go                           Launches the project.");
		MG_LOGNH("  clean                        Cleans the project.");
		MG_LOGNH("  pull <url>                   Installs a new dependency.");
		MG_LOGNH("  pull --list                  Lists all installed dependencies.");
		MG_LOGNH("  pull --help                  Shows more information.");
		MG_LOGNH("  remove <dependency>          Removes a dependency.");
		MG_LOGNH("  switch <dependency> <branch> Switches a dependency branch.");
	}

	void CommandHandler::HandleVersionCommand([[maybe_unused]] const CommandHandlerProps& props)
	{
		MG_LOG("Magnet v" + std::string(MG_VERSION));
	}

	void CommandHandler::HandleNewCommand([[maybe_unused]] const CommandHandlerProps& props)
	{
		Project project = Project::GetDefault();

		std::string name;

		MG_LOG_HOST("Project Wizard", "What would you like to name your new C++ project?");
		Application::PrintPrompt();
		std::cin >> name;

		project.SetName(name);

		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		do
		{
			MG_LOG_HOST("Project Wizard", "Choose a project type:");
			MG_LOGNH("  1. Executable (default)");
			MG_LOGNH("  2. StaticLibrary");
			MG_LOGNH("  3. DynamicLibrary");
			Application::PrintPrompt();

			std::string input;
			std::getline(std::cin, input);

			if (!input.empty())
			{
				switch (input[0])
				{
					case '1':
						project.SetType(ProjectType::Executable);
						break;
					case '2':
						project.SetType(ProjectType::StaticLibrary);
						break;
					case '3':
						project.SetType(ProjectType::DynamicLibrary);
						break;
					default:
						MG_LOG_HOST("Project Wizard", "Invalid answer.");
						continue;
				}

				break;
			}
			else
				break;
		} while (true);

		CreateNewProject(project);
	}

	void CommandHandler::HandleGenerateCommand(const CommandHandlerProps& props)
	{
		MG_LOG("Generating project files...");

		if (!Application::IsRootLevel())
		{
			MG_LOG("In order to generate, run this command at the root of your project, where .magnet can be found.");
			return;
		}

		if (!RequireProjectName(props))
			return;

		std::string projectName = props.project->GetName();

		const std::filesystem::path dependenciesPath = projectName + "/Dependencies";
		bool hasMissingDependencies = false;
		if (std::filesystem::exists(dependenciesPath))
		{
			auto dependencies = Application::GetDependencies();
			for (const auto& package : dependencies)
			{
				std::filesystem::path path = dependenciesPath / package;
				if (!std::filesystem::exists(path))
				{
					MG_LOG("Missing dependency: " + path.string());
					hasMissingDependencies = true;
				}
			}
		}

		if (hasMissingDependencies)
		{
			MG_LOG("Generate failed due to missing dependencies. Run `magnet pull` to install them.");
			return;
		}

		GenerateRootCMakeFile(props);
		GenerateCMakeFiles(props);
		GenerateDependencyCMakeFiles(props);

		std::string generateCommand = "cmake -S . -B " + projectName + "/Build ";

#if _WIN32
		generateCommand += "-G \"Visual Studio 16 2019\"";
#elif __APPLE__
		generateCommand += "-G Xcode";
#elif __linux__
		generateCommand += "-G \"Unix Makefiles\"";
#endif

		if (!ExecuteCommand(generateCommand,
		                    "CMake failed to generate project files. See messages above for more information."))
			return;

		MG_LOG("Successfully generated project files. Run `magnet build` next.");
	}

	void CommandHandler::HandleBuildCommand(const CommandHandlerProps& props)
	{
		std::string configuration = props.project->GetConfiguration().ToString();
		MG_LOG("Building in " + configuration + " configuration...");

		if (!RequireProjectName(props))
			return;

		std::string command = "cmake --build " + props.project->GetName() + "/Build --config " +
		                      configuration;
		if (!ExecuteCommand(command,
		                    "CMake couldn't build the project. See messages above for more information. Have you tried generating your project files first? If not, run `magnet generate`."))
			return;

		MG_LOG("Build successful. Run `magnet go` to launch your app.");
	}

	void CommandHandler::HandleGoCommand(const CommandHandlerProps& props)
	{
		MG_LOG("Launching project...");

		if (!RequireProjectName(props))
			return;

		std::string projectName = props.project->GetName();
		std::string configuration = props.project->GetConfiguration().ToString();
		std::string command = "./" + projectName + "/Binaries/" + configuration + "/" + projectName;
		if (!ExecuteCommand(command, "Failed to launch project. See messages above for more information."))
			return;
	}

	void CommandHandler::HandleCleanCommand(const CommandHandlerProps& props)
	{
		MG_LOG("Clean started...");

		if (!RequireProjectName(props))
			return;

		std::array<std::string, 4> removeTargets = {
				"/Build/cmake_install.cmake",
				"/Build/CMakeCache.txt",
				"/Build/CMakeFiles",
				"/Build/Makefile"
		};

		int removedItems = 0;
		for (const auto& target : removeTargets)
		{
			std::string path = props.project->GetName() + target;
			removedItems += (int) std::filesystem::remove_all(path);
		}

		if (removedItems == 0)
		{
			MG_LOG("Looks like your project is already clean. Nice!");
			return;
		}

		std::stringstream message;
		message << "Removed " << removedItems << " item" << (removedItems > 1 ? "s" : "") << ".";

		MG_LOG(message.str());
	}

	void CommandHandler::HandlePullCommand(const CommandHandlerProps& props)
	{
		std::string nextArgument = props.GetArgument(0);
		if (nextArgument.empty())
		{
			std::string command = "git submodule update --init --recursive";
			if (!ExecuteCommand(command,
			                    "Failed to install dependencies. See messages above for more information."))
				return;

			MG_LOG("Successfully installed all dependencies.");
			HandleGenerateCommand(props);
			return;
		}

		if (!RequireProjectName(props))
			return;

		if (nextArgument == "--list")
		{
			HandlePullListCommand(props);
			return;
		}

		if (nextArgument == "--help")
		{
			MG_LOGNH("Usage: magnet pull <url>");
			MG_LOGNH("       magnet pull --list");
			return;
		}

		// If the user didn't provide a full URL, we'll assume it's a GitHub repository.
		if (nextArgument.find("https://") != 0)
		{
			nextArgument = "https://github.com/" + nextArgument;
		}

		std::string name = ExtractRepositoryName(nextArgument);
		std::string installPath = props.project->GetName() + "/Dependencies/" + name;
		std::string command = "git submodule add " + nextArgument + " " + installPath;

		if (!ExecuteCommand(command,
		                    "Failed to install dependency. See messages above for more information."))
			return;

		auto dependencies = Application::GetDependencies();
		dependencies.push_back(name);
		WriteDependencyFile(dependencies);

		MG_LOG("Installed new dependency: " + name);

		HandleGenerateCommand(props);
	}

	void CommandHandler::HandlePullListCommand([[maybe_unused]] const CommandHandlerProps& props)
	{
		auto dependencies = Application::GetDependencies();

		if (dependencies.empty())
		{
			MG_LOG("No dependencies installed.");
			return;
		}

		MG_LOG("Here are all the installed dependencies:");

		for (auto& package : dependencies)
			MG_LOGNH(package);
	}

	void CommandHandler::HandleRemoveCommand(const CommandHandlerProps& props)
	{
		std::string dependency = props.GetArgument(0);
		if (dependency.empty())
		{
			MG_LOG("Usage: magnet remove <dependency>");
			return;
		}

		if (!RequireProjectName(props))
			return;

		std::string installPath = props.project->GetName() + "/Dependencies/" + dependency;
		std::string deinitCommand = "git submodule deinit -f " + installPath;

		if (!ExecuteCommand(deinitCommand,
		                    "Failed to remove dependency. See messages above for more information."))
			return;

		std::string gitRemoveCommand = "git rm -f " + installPath;
		if (!ExecuteCommand(gitRemoveCommand,
		                    "Failed to remove dependency. See messages above for more information."))
			return;

		std::string removeGitModuleCommand = "rm -rf .git/modules/" + installPath;
		if (!ExecuteCommand(removeGitModuleCommand,
		                    "Failed to remove dependency. See messages above for more information."))
			return;

		auto dependencies = Application::GetDependencies();
		dependencies.erase(std::remove(dependencies.begin(), dependencies.end(), dependency),
		                   dependencies.end());
		WriteDependencyFile(dependencies);

		MG_LOG("Removed dependency: " + dependency);

		HandleGenerateCommand(props);
	}

	void CommandHandler::HandleSwitchCommand(const CommandHandlerProps& props)
	{
		std::string dependency = props.GetArgument(0);
		std::string branch = props.GetArgument(1);

		if (dependency.empty() || branch.empty())
		{
			MG_LOG("Usage: magnet switch <dependency> <branch>");
			return;
		}

		if (!RequireProjectName(props))
			return;

		std::filesystem::path installPath = std::filesystem::path(props.project->GetName()) / "Dependencies" /
		                                    dependency;
		std::string command = "git -C " + installPath.string() + " checkout " + branch;
		if (!ExecuteCommand(command,
		                    "Failed to switch dependency branch. See messages above for more information."))
			return;

		std::string gitAddCommand = "git add " + installPath.string();
		if (!ExecuteCommand(gitAddCommand,
		                    "Failed to switch dependency branch. See messages above for more information."))
			return;

		MG_LOG("Switched " + dependency + " branch to: " + branch);

		HandleGenerateCommand(props);
	}

	bool CommandHandler::IsCommandGlobal(const std::string& command)
	{
		return command == "new" || command == "help" || command == "version";
	}

	void CommandHandler::CreateNewProject(const Project& project)
	{
		MG_LOG_HOST("Project Wizard", "Creating new C++ project...");

		const std::string& name = project.GetName();

		std::string templatePath = "magnet/magnet/Templates/MAGNET_NEW_PROJECT";
		std::string newPath = Application::GetCurrentWorkingDirectory() + "/" + name;

		std::filesystem::copy(templatePath, newPath, std::filesystem::copy_options::recursive);

		std::filesystem::rename(name + "/MAGNET_NEW_PROJECT", name + "/" + name);

		// Read .gitignore file and replace MAGNET_NEW_PROJECT with project name
		std::ifstream gitignore(name + "/.gitignore");
		std::string gitignoreContent((std::istreambuf_iterator<char>(gitignore)),
		                             std::istreambuf_iterator<char>());
		gitignore.close();

		std::ofstream newGitignore(name + "/.gitignore");
		std::string newGitignoreContent = std::regex_replace(gitignoreContent,
		                                                     std::regex("MAGNET_NEW_PROJECT"), name);
		newGitignore << newGitignoreContent;
		newGitignore.close();

		// Read README.md file and replace MAGNET_NEW_PROJECT with project name
		std::ifstream readme(name + "/README.md");
		std::string readmeContent((std::istreambuf_iterator<char>(readme)),
		                          std::istreambuf_iterator<char>());
		readme.close();

		std::ofstream newReadme(name + "/README.md");
		std::string newReadmeContent = std::regex_replace(readmeContent,
		                                                  std::regex("MAGNET_NEW_PROJECT"), name);
		newReadme << newReadmeContent;
		newReadme.close();

		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "name";
		out << YAML::Value << name;
		out << YAML::Key << "projectType";
		out << YAML::Value << project.GetTypeString();
		out << YAML::Key << "cppVersion";
		out << YAML::Value << project.GetCppVersion();
		out << YAML::Key << "cmakeVersion";
		out << YAML::Value << project.GetCmakeVersion();
		out << YAML::Key << "defaultConfiguration";
		out << YAML::Value << project.GetConfiguration().ToString();
		out << YAML::EndMap;

		// Create config.yaml file in .magnet folder which does not exist yet
		std::filesystem::create_directory(name + "/.magnet");
		std::ofstream config(name + "/.magnet/config.yaml");
		config << out.c_str();

		if (!config)
		{
			MG_LOG_HOST("Project Wizard", "Failed to create config.yaml file.");
			return;
		}

		WriteDependencyFile({}, name + "/.magnet/dependencies.yaml");

		std::string gitCommand = "git init " + name;
		int status = std::system(gitCommand.c_str());
		if (status != 0)
		{
			MG_LOG_HOST("Project Wizard", "Failed to initialize git repository.");
			return;
		}

		MG_LOG_HOST("Project Wizard", name + " has been created.\nNext steps: `cd " + name +
		                              " && magnet generate` to generate project files.");
	}

	bool CommandHandler::GenerateRootCMakeFile(const CommandHandlerProps& props)
	{
		if (!RequireProjectName(props))
			return false;

		std::ofstream cmakeFile("CMakeLists.txt");

		cmakeFile << "# Generated by Magnet v" << MG_VERSION << "\n\n";
		cmakeFile << "cmake_minimum_required(VERSION 3.16)\n";
		cmakeFile << "project(" << props.projectName << ")\n";
		cmakeFile << "set(CMAKE_CXX_STANDARD " << props.cppVersion << ")\n";

		cmakeFile << R"""(
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries")

add_subdirectory("${PROJECT_NAME}/Source")
add_subdirectory("${PROJECT_NAME}/Dependencies")

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Source")
	 )""";

		cmakeFile.close();

		return true;
	}

	bool CommandHandler::GenerateCMakeFiles(const CommandHandlerProps& props)
	{
		if (!RequireProjectName(props))
			return false;

		std::string projectName = props.project->GetName();
		std::vector<std::string> sourceFiles;

		std::string sourceFilesPath = projectName + "/Source";
		for (auto& path : std::filesystem::recursive_directory_iterator(sourceFilesPath))
		{
			if (path.path().extension() == ".cpp" || path.path().extension() == ".h" ||
			    path.path().extension() == ".hpp")
			{
				sourceFiles.push_back(path.path().filename().string());
			}
		}

		std::ofstream cmakeFile(props.projectName + "/Source/CMakeLists.txt");

		cmakeFile << "# Generated by Magnet v" << MG_VERSION << "\n\n";
		cmakeFile << "cmake_minimum_required(VERSION 3.16)\n";
		cmakeFile << "project(" << props.projectName << ")\n";
		cmakeFile << "set(CMAKE_CXX_STANDARD 17)\n";

		if (Application::GetProjectType() == "StaticLibrary")
			cmakeFile << "add_library(${PROJECT_NAME} STATIC";
		else if (Application::GetProjectType() == "SharedLibrary")
			cmakeFile << "add_library(${PROJECT_NAME} SHARED";
		else
			cmakeFile << "add_executable(${PROJECT_NAME}";

		for (const auto& sourceFile : sourceFiles)
		{
			cmakeFile << " " << sourceFile;
		}

		cmakeFile << ")\n";

		cmakeFile << R"""(
# Set rpath relative to app
if (NOT MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")
else ()
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/Binaries/Debug")
endif ()

# Precompiled headers
# target_precompile_headers(${PROJECT_NAME} PUBLIC PCH.h)
)""";

		auto dependencies = Application::GetDependencies();
		if (!dependencies.empty())
		{
			cmakeFile << "target_link_libraries(${PROJECT_NAME}";

			for (const auto& package : dependencies)
			{
				cmakeFile << " " << package;
			}

			cmakeFile << ")";
		}

		cmakeFile.close();
		return true;
	}

	bool CommandHandler::GenerateDependencyCMakeFiles(const CommandHandlerProps& props)
	{
		if (!RequireProjectName(props))
			return false;

		std::string projectName = props.project->GetName();

		CmakeEmitter emitter(projectName + "/Dependencies/CMakeLists.txt");

		cmakeFile << "# Generated by Magnet v" << MG_VERSION << "\n\n";
		cmakeFile << "cmake_minimum_required(VERSION 3.16)\n";
		cmakeFile << "project(" << props.projectName << ")\n";

		auto dependencies = Application::GetDependencies();
		if (!dependencies.empty())
		{
			for (const auto& package : dependencies)
			{
				cmakeFile << "add_subdirectory(" << package << ")\n";
			}

			cmakeFile << "target_include_directories(" << props.projectName << " PUBLIC\n";

			for (const auto& package : dependencies)
			{
				std::string includePath = props.projectName + "/Dependencies/" + package;
				if (!std::filesystem::exists(includePath))
					continue;

				cmakeFile << "      \"" << package;

				if (std::filesystem::exists(includePath + "/include"))
					cmakeFile << "/include";

				cmakeFile << "\"\n";
			}

			cmakeFile << ")\n";
		}

		cmakeFile.close();
		return true;
	}

	std::string CommandHandler::ExtractRepositoryName(const std::string& url)
	{
		std::string name = url;
		name = url.substr(url.find_last_of('/') + 1);
		name = name.substr(0, name.find_last_of('.'));
		return name;
	}

	bool CommandHandler::WriteDependencyFile(const std::vector<std::string>& dependencies,
	                                         const std::string& path)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "dependencies";
		out << YAML::Value << dependencies;
		out << YAML::EndMap;

		std::ofstream file(path.empty() ? ".magnet/dependencies.yaml" : path);
		file << out.c_str();

		if (!file)
		{
			MG_LOG("Failed to update dependencies.yaml file.");
			return false;
		}

		return true;
	}

	bool CommandHandler::RequireProjectName(const CommandHandlerProps& props)
	{
		if (props.project->GetName().empty())
		{
			MG_LOG("Command failed due to unknown project name.");
			return false;
		}

		return true;
	}

	bool CommandHandler::ExecuteCommand(const std::string& command, const std::string& errorMessage)
	{
		int status = std::system((command).c_str());
		if (status != 0)
		{
			MG_LOG(errorMessage);
			return false;
		}

		return true;
	}
}
