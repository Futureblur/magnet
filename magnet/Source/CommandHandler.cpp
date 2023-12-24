#include "CommandHandler.h"

#include "yaml-cpp/yaml.h"

#include "Application.h"
#include "CmakeEmitter.h"
#include "Core.h"
#include "Platform/Platform.h"
#include "Project.h"

namespace MG
{
	std::string CommandHandlerProps::GetArgument(uint32_t index) const
	{
		if (index >= nextArguments.size())
			return "";

		return nextArguments[index];
	}

	std::string CommandHandlerProps::ConvertArgumetsToString() const
	{
		if (nextArguments.empty())
			return "";

		auto arguments = std::accumulate(nextArguments.begin(), nextArguments.end(), std::string(),
		                                 [](const std::string& a, const std::string& b)
		                                 {
			                                 return a + " " + b;
		                                 });
		// Remove the first space.
		return arguments.substr(1);
	}

	[[maybe_unused]] bool CommandHandlerProps::HasArguments() const
	{
		return !nextArguments.empty();
	}

	void CommandHandler::HandleHelpCommand([[maybe_unused]] const CommandHandlerProps& props)
	{
		MG_LOG("Usage: magnet <command> [options]\n");
		MG_LOGNH("Commands:");
		MG_LOGNH("  help                         Shows this message.");
		MG_LOGNH("  version                      Shows the current version of Magnet.");
		MG_LOGNH("  config <configuration>       Changes the default configuration.");
		MG_LOGNH("  new                          Creates a new C++ project.");
		MG_LOGNH("  generate                     Generates project files.");
		MG_LOGNH("  build                        Builds the project.");
		MG_LOGNH("  go                           Launches the project.");
		MG_LOGNH("  clean                        Cleans the project.");
		MG_LOGNH("  pull                         Installs all dependencies.");
		MG_LOGNH("  pull <url>                   Installs a new dependency.");
		MG_LOGNH("  pull --list                  Lists all installed dependencies.");
		MG_LOGNH("  pull --help                  Shows more information.");
		MG_LOGNH("  remove <dependency>          Removes a dependency.");
		MG_LOGNH("  switch <dependency> <branch> Switches a dependency branch.");
	}

	void CommandHandler::HandleConfigCommand(const CommandHandlerProps& props)
	{
		std::string nextArgument = props.GetArgument(0);
		if (nextArgument.empty())
		{
			MG_LOG("Usage: magnet config <configuration>");
			return;
		}

		Configuration configuration = Configuration::FromString(nextArgument);

		if (!configuration.IsValid())
		{
			MG_LOG("Usage: magnet config [Debug/Release]");
			return;
		}

		Application::SetDefaultConfiguration(configuration);

		MG_LOG("Successfully changed default configuration to " + configuration.ToString() + ".");
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

			break;
		}
		while (true);

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

		const std::filesystem::path dependenciesPath = std::filesystem::path(projectName) / "Dependencies";
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

		std::filesystem::path buildPath = std::filesystem::path(projectName) / "Build";
		std::string generateCommand = "cmake -S . -B " + buildPath.string() + " ";

		generateCommand += Platform::GetGenerateCommand(props.project->GetConfiguration().ToString());
		generateCommand += " " + props.ConvertArgumetsToString();

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

		std::filesystem::path buildPath = std::filesystem::path(props.project->GetName()) / "Build";
		std::string command = "cmake --build " + buildPath.string() + " --config " + configuration + " " +
		                      props.ConvertArgumetsToString();

		if (!ExecuteCommand(command,
		                    "CMake couldn't build the project. See messages above for more information. Have you tried generating your project files first? If not, run `magnet generate`."))
			return;

		MG_LOG("Build successful. Run `magnet go` to launch your app.");
	}

	void CommandHandler::HandleGoCommand(const CommandHandlerProps& props)
	{
		HandleGenerateCommand(props);
		HandleBuildCommand(props);

		MG_LOG("Launching project...");

		if (!RequireProjectName(props))
			return;

		std::string projectName = props.project->GetName();
		std::string configuration = props.project->GetConfiguration().ToString();

		auto appPath = std::filesystem::path(projectName) / "Binaries" / configuration / projectName;
		std::string command = Platform::GetGoCommand(appPath.string());
		command += " " + props.ConvertArgumetsToString();

		if (!ExecuteCommand(command, "Failed to launch project. See messages above for more information."))
			return;
	}

	void CommandHandler::HandleCleanCommand(const CommandHandlerProps& props)
	{
		MG_LOG("Clean started...");

		if (!RequireProjectName(props))
			return;

		std::array<std::string, 4> removeTargets =
		{
			"Build/cmake_install.cmake",
			"Build/CMakeCache.txt",
			"Build/CMakeFiles",
			"Build/Makefile"
		};

		int removedItems = 0;
		for (const auto& target : removeTargets)
		{
			std::filesystem::path path = std::filesystem::path(props.project->GetName()) / target;
			removedItems += static_cast<int>(std::filesystem::remove_all(path.generic_string()));
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
		std::filesystem::path installPath = std::filesystem::path(props.project->GetName()) / "Dependencies" /
		                                    name;
		std::string command = "git submodule add " + nextArgument + " " + installPath.string();

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

		std::filesystem::path installPath = std::filesystem::path(props.project->GetName()) / "Dependencies" /
		                                    dependency;
		std::string deinitCommand = "git submodule deinit -f " + installPath.string();

		if (!ExecuteCommand(deinitCommand,
		                    "Failed to remove dependency. See messages above for more information."))
			return;

		std::string gitRemoveCommand = "git rm -f " + installPath.string();
		if (!ExecuteCommand(gitRemoveCommand,
		                    "Failed to remove dependency. See messages above for more information."))
			return;

		std::filesystem::path gitModulesPath = std::filesystem::path(props.project->GetName()) / ".git" /
		                                       "modules" / installPath;
		std::string removeGitModuleCommand = "rm -rf " + gitModulesPath.string();
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

		std::filesystem::path templatePath = Platform::GetExecutablePath() /
		                                     "../../Templates/MAGNET_NEW_PROJECT";
		std::filesystem::path newPath = Application::GetCurrentWorkingDirectory() / name;

		templatePath = templatePath.generic_string();
		newPath = newPath.generic_string();

		std::filesystem::copy(templatePath, newPath, std::filesystem::copy_options::recursive);

		{
			std::filesystem::path fromPath = std::filesystem::path(name) / "MAGNET_NEW_PROJECT";
			std::filesystem::path toPath = std::filesystem::path(name) / name;
			std::filesystem::rename(fromPath, toPath);
		}

		// Read .gitignore file and replace MAGNET_NEW_PROJECT with project name
		std::filesystem::path gitignorePath = std::filesystem::path(name) / ".gitignore";
		std::ifstream gitignore(gitignorePath);
		std::string gitignoreContent((std::istreambuf_iterator<char>(gitignore)),
		                             std::istreambuf_iterator<char>());
		gitignore.close();

		std::ofstream newGitignore(gitignorePath);
		std::string newGitignoreContent = std::regex_replace(gitignoreContent,
		                                                     std::regex("MAGNET_NEW_PROJECT"), name);
		newGitignore << newGitignoreContent;
		newGitignore.close();

		// Read README.md file and replace MAGNET_NEW_PROJECT with project name
		std::filesystem::path readmePath = std::filesystem::path(name) / "README.md";
		std::ifstream readme(readmePath);
		std::string readmeContent((std::istreambuf_iterator<char>(readme)),
		                          std::istreambuf_iterator<char>());
		readme.close();

		std::ofstream newReadme(readmePath);
		std::string newReadmeContent = std::regex_replace(readmeContent,
		                                                  std::regex("MAGNET_NEW_PROJECT"), name);
		newReadme << newReadmeContent;
		newReadme.close();

		// Read .idea/cmake.xml file and replace MAGNET_NEW_PROJECT with project name
		std::filesystem::path cmakePath = std::filesystem::path(name) / ".idea" / "cmake.xml";
		std::ifstream cmake(cmakePath);
		std::string cmakeContent((std::istreambuf_iterator<char>(cmake)),
		                         std::istreambuf_iterator<char>());
		cmake.close();

		std::ofstream newCmake(cmakePath);
		std::string newCmakeContent = std::regex_replace(cmakeContent,
		                                                 std::regex("MAGNET_NEW_PROJECT"), name);
		newCmake << newCmakeContent;
		newCmake.close();

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
		std::filesystem::path magnetPath = std::filesystem::path(name) / ".magnet";
		std::filesystem::create_directory(magnetPath);
		std::ofstream config(magnetPath / "config.yaml");
		config << out.c_str();

		if (!config)
		{
			MG_LOG_HOST("Project Wizard", "Failed to create config.yaml file.");
			return;
		}

		WriteDependencyFile({}, magnetPath / "dependencies.yaml");

		std::filesystem::path dependenciesPath = std::filesystem::path(name) / name / "Dependencies";
		std::filesystem::create_directory(dependenciesPath);

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

		// Before overwriting the CMakeLists.txt file, we'll extract the custom code
		// and add it back after we're done generating.
		std::filesystem::path cmakeFilePath = "CMakeLists.txt";
		std::string customCode = ExtractCustomCMakeCode(cmakeFilePath);

		CmakeEmitter emitter(cmakeFilePath);

		emitter.Add_Header();
		emitter.Add_CmakeMinimumRequired(props.project->GetCmakeVersion());
		emitter.Add_Project(props.project->GetName());

		emitter.Add_Newline();

		emitter.Add_SetCmakeCxxStandard(props.project->GetCppVersion());

		emitter.Add_Newline();

		auto ifTrue = [&emitter]()
		{
			emitter.Add_Indentation();
			emitter.Add_SetCmakeArchiveOutputDirectory(
				"${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries/${CMAKE_BUILD_TYPE}");
			emitter.Add_Indentation();
			emitter.Add_SetCmakeLibraryOutputDirectory(
				"${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries/${CMAKE_BUILD_TYPE}");
			emitter.Add_Indentation();
			emitter.Add_SetCmakeRuntimeOutputDirectory(
				"${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries/${CMAKE_BUILD_TYPE}");
		};

		auto ifFalse = [&emitter]()
		{
			emitter.Add_Indentation();
			emitter.Add_SetCmakeArchiveOutputDirectory("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries");
			emitter.Add_Indentation();
			emitter.Add_SetCmakeLibraryOutputDirectory("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries");
			emitter.Add_Indentation();
			emitter.Add_SetCmakeRuntimeOutputDirectory("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Binaries");
		};

		emitter.Add_IfElse("CMAKE_GENERATOR MATCHES \"Unix Makefiles\" OR CMAKE_GENERATOR MATCHES Ninja",
		                   ifTrue, ifFalse);

		emitter.Add_Newline();

		emitter.Add_AddSubdirectory("${PROJECT_NAME}/Source");
		emitter.Add_AddSubdirectory("${PROJECT_NAME}/Dependencies");

		emitter.Add_Newline();

		emitter.Add_TargetIncludeDirectories(
			props.project->GetName(), "PUBLIC", "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/Source");

		emitter.Add_Newline();

		emitter.Add_If("MSVC", [&]()
		{
			emitter.Add_Indentation();
			emitter.Add_Literal("set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT " +
			                    props.project->GetName() + ")");
			emitter.Add_Newline();
		});

		emitter.Add_Newline(2);
		emitter.Add_Comment("Insert your own CMake commands after this line.", true);

		if (customCode.empty() || customCode == "\n")
		{
			emitter.Add_Newline();
			return true;
		}

		emitter.Add_Literal(customCode);
		return true;
	}

	bool CommandHandler::GenerateCMakeFiles(const CommandHandlerProps& props)
	{
		if (!RequireProjectName(props))
			return false;

		std::string projectName = props.project->GetName();
		std::vector<std::string> sourceFiles;

		std::filesystem::path sourceFilesPath = std::filesystem::path(projectName) / "Source";
		for (auto& path : std::filesystem::recursive_directory_iterator(sourceFilesPath))
		{
			if (path.path().extension() == ".cpp" || path.path().extension() == ".h" ||
			    path.path().extension() == ".hpp")
			{
				sourceFiles.push_back(path.path().filename().string());
			}
		}

		// Before overwriting the CMakeLists.txt file, we'll extract the custom code
		// and add it back after we're done generating.
		std::filesystem::path cmakePath = std::filesystem::path(projectName) / "Source" / "CMakeLists.txt";
		std::string customCode = ExtractCustomCMakeCode(cmakePath);

		CmakeEmitter emitter(cmakePath);

		emitter.Add_Header();
		emitter.Add_CmakeMinimumRequired(props.project->GetCmakeVersion());
		emitter.Add_Project(projectName);
		emitter.Add_SetCmakeCxxStandard(props.project->GetCppVersion());

		bool isExecutable = props.project->GetType() == ProjectType::Executable;
		if (isExecutable)
		{
			emitter.Add_AddExecutable(projectName, sourceFiles);
		}
		else
		{
			std::string type = props.project->GetCmakeTypeString();
			emitter.Add_AddLibrary(projectName, type, sourceFiles);
		}

		auto ifTrue = [&]()
		{
			emitter.Add_Indentation();
			emitter.Add_SetTargetProperties(projectName, "LINK_FLAGS", "\"-Wl,-rpath,./\"");
		};

		auto ifFalse = [&]()
		{
			emitter.Add_Indentation();
			emitter.Add_SetTargetProperties(projectName, "VS_DEBUGGER_WORKING_DIRECTORY",
			                                "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/Binaries/Debug");
		};

		emitter.Add_Newline();

		emitter.Add_Comment("Set rpath relative to app");
		emitter.Add_IfElse("NOT MSVC", ifTrue, ifFalse);

		emitter.Add_Newline();

		emitter.Add_TargetPrecompileHeaders("${PROJECT_NAME}", "PUBLIC", "PCH.h");

		emitter.Add_Newline();

		auto dependencies = Application::GetDependencies();
		if (!dependencies.empty())
		{
			emitter.Add_TargetLinkLibraries(projectName, dependencies);
		}

		emitter.Add_Newline();
		emitter.Add_Comment("Insert your own CMake commands after this line.", true);

		if (customCode.empty() || customCode == "\n")
		{
			emitter.Add_Newline();
			return true;
		}

		emitter.Add_Literal(customCode);
		return true;
	}

	bool CommandHandler::GenerateDependencyCMakeFiles(const CommandHandlerProps& props)
	{
		if (!RequireProjectName(props))
			return false;

		std::string projectName = props.project->GetName();

		// Before overwriting the CMakeLists.txt file, we'll extract the custom code
		// and add it back after we're done generating.
		std::filesystem::path cmakePath = std::filesystem::path(projectName) / "Dependencies" / "CMakeLists.txt";
		std::string customCode = ExtractCustomCMakeCode(cmakePath);

		CmakeEmitter emitter(cmakePath);

		emitter.Add_Header();
		emitter.Add_CmakeMinimumRequired(props.project->GetCmakeVersion());
		emitter.Add_Project(projectName);

		emitter.Add_Newline();

		auto dependencies = Application::GetDependencies();
		if (!dependencies.empty())
		{
			emitter.Add_AddSubdirectory(dependencies);

			emitter.Add_Newline();

			emitter.Begin_TargetIncludeDirectories(projectName, "PUBLIC");

			for (const auto& package : dependencies)
			{
				std::filesystem::path packagePath = std::filesystem::path(projectName) / "Dependencies" /
				                                    package;
				if (!std::filesystem::exists(packagePath))
					continue;

				emitter.Add_Indentation();

				emitter.Add_Literal("\"");
				emitter.Add_Literal(package);

				if (std::filesystem::exists(packagePath / "include"))
					emitter.Add_Literal("/include");

				emitter.Add_Literal("\"");
				emitter.Add_Newline();
			}

			emitter.End_TargetIncludeDirectories();
		}

		emitter.Add_Newline();
		emitter.Add_Comment("Insert your own CMake commands after this line.", true);

		if (customCode.empty() || customCode == "\n")
		{
			emitter.Add_Newline();
			return true;
		}

		emitter.Add_Literal(customCode);
		return true;
	}

	std::string CommandHandler::ExtractCustomCMakeCode(const std::filesystem::path& path)
	{
		std::ifstream cmakeFile(path);

		if (!cmakeFile)
			return "";

		std::string content((std::istreambuf_iterator<char>(cmakeFile)),
		                    std::istreambuf_iterator<char>());

		std::string indicator = "# Insert your own CMake commands after this line.";
		std::string customCode = content.substr(content.find(indicator) + indicator.length());

		return customCode;
	}

	std::string CommandHandler::ExtractRepositoryName(const std::string& url)
	{
		std::string name = url;
		name = url.substr(url.find_last_of('/') + 1);
		name = name.substr(0, name.find_last_of('.'));
		return name;
	}

	bool CommandHandler::WriteDependencyFile(const std::vector<std::string>& dependencies,
	                                         const std::filesystem::path& path)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "dependencies";
		out << YAML::Value << dependencies;
		out << YAML::EndMap;

		std::filesystem::path dependenciesPath = std::filesystem::path(".magnet") / "dependencies.yaml";
		std::ofstream file(path.empty() ? dependenciesPath : path);
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
