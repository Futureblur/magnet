#include "CommandHandler.h"

#include "yaml-cpp/yaml.h"

#include "Application.h"

namespace MG
{
	// TODO: This will later be kept track on in a separate dependency.yaml file
	static const std::vector <std::string> dependencyDatabase = {
			"MagnetExample"
	};

	void CommandHandler::HandleNewCommand(CommandLineArguments* args, int index)
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

			Application::Print("Project Wizard",
			                   "What would you like to name your new C++ project?");
			Application::PrintPrompt();
			std::cin >> name;

			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			do
			{
				Application::Print("Project Wizard", "Choose a project type:");
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

					Application::Print("Project Wizard", "Invalid answer.");
				}
				else
					break;
			} while (true);

			CreateNewProject(name, projectType);
		}
	}

	void CommandHandler::HandleGenerateCommand()
	{
		Application::Print("Magnet", "Generating project files...");

		if (!Application::IsRootLevel())
		{
			Application::Print("Magnet",
			                   "In order to generate, run this command at the root of your project, where .magnet can be found.");

			return;
		}

		GenerateRootCMakeFile();
		GenerateCMakeFiles();
		GenerateDependencyCMakeFiles();

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			Application::Print("Magnet", "Generate failed due to unknown project name.");
			return;
		}

		std::string generateCommand = "cmake -S . -B " + projectName +
		                              "/Build -G Xcode -DCMAKE_BUILD_TYPE=Debug";

		int status = std::system(generateCommand.c_str());
		if (status != 0)
		{
			Application::Print("Magnet",
			                   "CMake failed to generate project files. See messages above for more information.");

			return;
		}

		Application::Print("Magnet",
		                   "Successfully generated project files. Run `magnet build` next.");
	}

	void CommandHandler::HandleBuildCommand()
	{
		Application::Print("Magnet", "Building in debug configuration...");

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			Application::Print("Magnet", "Build failed due to missing ProjectName.txt file.");
			return;
		}

		std::string command = "cmake --build " + projectName + "/Build --config Debug";

		int status = std::system(command.c_str());
		if (status != 0)
		{
			Application::Print("Magnet",
			                   "CMake couldn't build the project. See messages above for more information. Have you tried generating your project files first? If not, run `magnet generate`.");

			return;
		}

		Application::Print("Magnet",
		                   "Successfully built project. Run `magnet go` to launch your app.");
	}

	void CommandHandler::HandleGoCommand()
	{
		Application::Print("Magnet", "Launching project...");

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			Application::Print("Magnet", "Launch failed due to unknown project name.");
			return;
		}

		std::string command = "./" + projectName + "/Binaries/Debug/" + projectName;

		std::system(command.c_str());
	}

	void CommandHandler::HandleCleanCommand()
	{
		Application::Print("Magnet", "Clean started...");

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			Application::Print("Magnet", "Clean failed due to unknown project name.");
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
			Application::Print("Magnet", "Looks like your project is already clean. Nice!");
			return;
		}

		std::stringstream message;
		message << "Removed " << removedItems << " item" << (removedItems > 1 ? "s" : "") << ".";

		Application::Print("Magnet", message.str());
	}

	void CommandHandler::HandlePullCommand()
	{
		// TODO: This simulates a git submodule add for now

		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			Application::Print("Magnet", "Install failed due to unknown project name.");
			return;
		}

		std::string dummyPackagePath = "MagnetExample";
		std::string destinationPath = Application::GetCurrentWorkingDirectory()
		                              + "/" + projectName + "/Dependencies/MagnetExample";

		std::filesystem::copy(dummyPackagePath, destinationPath, std::filesystem::copy_options::recursive);

		Application::Print("Magnet", "Installed new dependency.");
	}

	void CommandHandler::CreateNewProject(const std::string& name, const std::string& type)
	{
		Application::Print("Project Wizard", "Creating new C++ project...");

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

		std::ofstream config(name + "/.magnet/config.yaml");
		config << out.c_str();
		config.close();

		Application::Print("Project Wizard", name + " has been created.\nNext steps: `cd " + name +
		                                     " && magnet generate` to generate project files.");
	}

	bool CommandHandler::GenerateRootCMakeFile()
	{
		std::string projectName = Application::GetProjectName();
		if (projectName.empty())
		{
			Application::Print("Magnet", "Generate failed due to unknown project name.");
			return false;
		}

		std::ofstream cmakeFile("CMakeLists.txt");

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
			Application::Print("Magnet", "Generate failed due to unknown project name.");
			return false;
		}

		std::vector <std::string> sourceFiles;

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

		for (const auto& package : dependencyDatabase)
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
			Application::Print("Magnet", "Generate failed due to unknown project name.");
			return false;
		}

		std::ofstream cmakeFile(projectName + "/Dependencies/CMakeLists.txt");

		cmakeFile << "cmake_minimum_required(VERSION 3.16)\n";
		cmakeFile << "project(" << projectName << "_Dependencies)\n";

		cmakeFile << "add_subdirectory(";

		for (const auto& package : dependencyDatabase)
		{
			cmakeFile << " " << package;
		}

		cmakeFile << " )";

		cmakeFile.close();

		return true;
	}
}
