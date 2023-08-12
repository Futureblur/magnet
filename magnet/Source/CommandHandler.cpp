#include "CommandHandler.h"

#include "yaml-cpp/yaml.h"

#include "Application.h"
#include "Core.h"

namespace MG
{
	void CommandHandler::HandleNewCommand(const CommandLineArguments* args, int index)
	{
		bool hasNext = index + 1 < args->count;
		bool hasNextButOne = index + 2 < args->count;
		if (hasNext && hasNextButOne)
		{
			std::string nextArgument = args->list[index + 1];
			std::string nextButOneArgument = args->list[index + 2];
			CreateNewProject(nextArgument, nextButOneArgument);
		}
		else
		{
			std::string name;
			std::string projectType = "Application";

			MG_LOG_HOST("Project Wizard", "What would you like to name your new C++ project?");
			Application::PrintPrompt();
			std::cin >> name;

			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			do
			{
				MG_LOG_HOST("Project Wizard", "Choose a project type:");
				std::cout << "- Application (default)\n";
				std::cout << "- StaticLibrary\n";
				std::cout << "- SharedLibrary\n";
				Application::PrintPrompt();

				std::string input;
				std::getline(std::cin, input);

				if (!input.empty())
				{
					std::istringstream stream(input);
					stream >> projectType;

					if (projectType == "Application" || projectType == "StaticLibrary" ||
					    projectType == "SharedLibrary")
					{
						break;
					}

					MG_LOG_HOST("Project Wizard", "Invalid answer.");
				}
				else
					break;
			} while (true);

			CreateNewProject(name, projectType);
		}
	}

	void CommandHandler::HandleGenerateCommand()
	{
		MG_LOG("Generating project files...");

		if (!Application::IsRootLevel())
		{
			MG_LOG("In order to generate, run this command at the root of your project, where .magnet can be found.");

			return;
		}

		GenerateRootCMakeFile();
		GenerateCMakeFiles();
		GenerateDependencyCMakeFiles();

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Generate failed due to unknown project name.");
			return;
		}

		std::string generateCommand = "cmake -S . -B " + projectName +
		                              "/Build -G Xcode -DCMAKE_BUILD_TYPE=Debug";

		int status = std::system(generateCommand.c_str());
		if (status != 0)
		{
			MG_LOG("CMake failed to generate project files. See messages above for more information.");

			return;
		}

		MG_LOG("Successfully generated project files. Run `magnet build` next.");
	}

	void CommandHandler::HandleBuildCommand()
	{
		MG_LOG("Building in debug configuration...");

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Build failed due to missing ProjectName.txt file.");
			return;
		}

		std::string command = "cmake --build " + projectName + "/Build --config Debug";

		int status = std::system(command.c_str());
		if (status != 0)
		{
			MG_LOG("CMake couldn't build the project. See messages above for more information. Have you tried generating your project files first? If not, run `magnet generate`.");

			return;
		}

		MG_LOG("Successfully built project. Run `magnet go` to launch your app.");
	}

	void CommandHandler::HandleGoCommand()
	{
		MG_LOG("Launching project...");

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Launch failed due to unknown project name.");
			return;
		}

		std::string command = "./" + projectName + "/Binaries/Debug/" + projectName;

		int result = std::system(command.c_str());
		if (result != 0)
		{
			MG_LOG("Failed to launch project. See messages above for more information.");
			return;
		}
	}

	void CommandHandler::HandleCleanCommand()
	{
		MG_LOG("Clean started...");

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Clean failed due to unknown project name.");
			return;
		}

		std::array<std::string, 4> removeTargets = {
				"/Build/cmake_install.cmake",
				"/Build/CMakeCache.txt",
				"/Build/CMakeFiles",
				"/Build/Makefile"
		};

		int removedItems = 0;
		for (const auto& target : removeTargets)
		{
			std::string path = projectName + target;
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

	void CommandHandler::HandlePullCommand(const CommandLineArguments* args, int index)
	{
		bool hasNext = index + 1 < args->count;
		if (!hasNext)
		{
			int status = std::system("git submodule update --init --recursive");
			if (status != 0)
			{
				MG_LOG("Failed to install dependencies. See messages above for more information.");
				return;
			}

			MG_LOG("Successfully installed all dependencies.");
			HandleGenerateCommand();
			return;
		}

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Install failed due to unknown project name.");
			return;
		}

		std::string nextArgument = args->list[index + 1];

		if (nextArgument == "--list")
		{
			HandlePullListCommand();
			return;
		}

		if (nextArgument == "--help")
		{
			MG_LOG("Usage: magnet pull <url>");
			MG_LOG("       magnet pull --list");
			return;
		}

		if (nextArgument.find("https://") != 0)
		{
			nextArgument = "https://github.com/" + nextArgument;
		}

		std::string name = ExtractRepositoryName(nextArgument);
		std::string installPath = projectName + "/Dependencies/" + name;
		std::string command = "git submodule add " + nextArgument + " " + installPath;

		int status = std::system(command.c_str());
		if (status != 0)
		{
			MG_LOG("Failed to install dependency. See messages above for more information.");
			return;
		}

		auto dependencies = Application::GetDependencies();
		dependencies.push_back(name);
		WriteDependencyFile(dependencies);

		MG_LOG("Installed new dependency: " + name);

		HandleGenerateCommand();
	}

	void CommandHandler::HandlePullListCommand()
	{
		auto dependencies = Application::GetDependencies();

		if (dependencies.empty())
		{
			MG_LOG("No dependencies installed.");
			return;
		}

		MG_LOG("Here are all the installed dependencies:");

		for (auto& package : dependencies)
			std::cout << package << "\n";
	}

	void CommandHandler::CreateNewProject(const std::string& name, const std::string& type)
	{
		MG_LOG_HOST("Project Wizard", "Creating new C++ project...");

		std::string templatePath = "magnet/magnet/Templates/MAGNET_NEW_PROJECT";
		std::string newPath = Application::GetCurrentWorkingDirectory() + "/" + name;

		std::filesystem::copy(templatePath, newPath, std::filesystem::copy_options::recursive);

		std::filesystem::rename(name + "/MAGNET_NEW_PROJECT", name + "/" + name);

		std::filesystem::remove(name + "/Template_CMakeLists.txt");
		std::filesystem::remove(name + "/" + name + "/Source/Template_CMakeLists.txt");
		std::filesystem::remove(name + "/" + name + "/Dependencies/Template_CMakeLists.txt");

		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "name";
		out << YAML::Value << name;
		out << YAML::Key << "projectType";
		out << YAML::Value << type;
		out << YAML::Key << "cppDialect";
		out << YAML::Value << "20";
		out << YAML::Key << "cmakeVersion";
		out << YAML::Value << "3.16";
		out << YAML::Key << "defaultConfiguration";
		out << YAML::Value << "Debug";
		out << YAML::EndMap;

		// Create config.yaml file in .magnet folder which does not exist yet
		std::filesystem::create_directory(name + "/.magnet");
		std::ofstream config(name + "/.magnet/config.yaml");
		config << out.c_str();

		if (!config)
		{
			MG_LOG_HOST("Project Wizard", "Failed to create config.yaml file.");
			MG_LOG_HOST("Project Wizard", "Error: " + std::string(std::strerror(errno)));
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

	bool CommandHandler::GenerateRootCMakeFile()
	{
		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Generate failed due to unknown project name.");
			return false;
		}

		std::ofstream cmakeFile("CMakeLists.txt");

		cmakeFile << "# Generated by Magnet v" << MG_VERSION << "\n\n";
		cmakeFile << "cmake_minimum_required(VERSION 3.16)\n";
		cmakeFile << "project(" << projectName << ")\n";
		cmakeFile << "set(CMAKE_CXX_STANDARD 17)\n";

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

	bool CommandHandler::GenerateCMakeFiles()
	{
		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Generate failed due to unknown project name.");
			return false;
		}

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

		std::ofstream cmakeFile(projectName + "/Source/CMakeLists.txt");

		cmakeFile << "# Generated by Magnet v" << MG_VERSION << "\n\n";
		cmakeFile << "cmake_minimum_required(VERSION 3.16)\n";
		cmakeFile << "project(" << projectName << ")\n";
		cmakeFile << "set(CMAKE_CXX_STANDARD 17)\n";
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

		cmakeFile << "target_link_libraries(${PROJECT_NAME} PUBLIC";

		auto dependencies = Application::GetDependencies();
		for (const auto& package : dependencies)
		{
			cmakeFile << " " << package;
		}

		cmakeFile << ")";

		cmakeFile.close();

		return true;
	}

	bool CommandHandler::GenerateDependencyCMakeFiles()
	{
		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			MG_LOG("Generate failed due to unknown project name.");
			return false;
		}

		std::ofstream cmakeFile(projectName + "/Dependencies/CMakeLists.txt");

		cmakeFile << "# Generated by Magnet v" << MG_VERSION << "\n\n";
		cmakeFile << "cmake_minimum_required(VERSION 3.16)\n";
		cmakeFile << "project(" << projectName << "_Dependencies)\n";

		cmakeFile << "add_subdirectory(";

		auto dependencies = Application::GetDependencies();
		for (const auto& package : dependencies)
		{
			cmakeFile << " " << package;
		}

		cmakeFile << " )";

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
}
