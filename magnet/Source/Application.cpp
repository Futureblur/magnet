#include "Application.h"

#include "yaml-cpp/yaml.h"

#include "CommandHandler.h"
#include "Core.h"
#include "Platform/Platform.h"
#include "Project.h"

namespace MG
{
	// Map commands to Handler functions.
	static const std::unordered_map<std::string, void (*)(const CommandHandlerProps&)> m_Commands = {
			{"help",     CommandHandler::HandleHelpCommand},
			{"version",  CommandHandler::HandleVersionCommand},
			{"config",   CommandHandler::HandleConfigCommand},
			{"new",      CommandHandler::HandleNewCommand},
			{"generate", CommandHandler::HandleGenerateCommand},
			{"build",    CommandHandler::HandleBuildCommand},
			{"go",       CommandHandler::HandleGoCommand},
			{"clean",    CommandHandler::HandleCleanCommand},
			{"pull",     CommandHandler::HandlePullCommand},
			{"remove",   CommandHandler::HandleRemoveCommand},
			{"switch",   CommandHandler::HandleSwitchCommand},
	};

	static const std::unordered_map<std::string, std::string> m_SimilarCommands = {
			{"--help",    "help"},
			{"--version", "version"},
			{"run",       "go"},
			{"launch",    "go"},
			{"get",       "pull"},
			{"add",       "pull"},
			{"install",   "pull"},
			{"delete",    "remove"},
	};

	void Application::Initialize(const CommandLineArguments& args)
	{
		m_Arguments = args;
		Platform::Initialize();
	}

	void Application::Print(const std::string& host, const std::string& message)
	{
		if (message.empty())
			return;

		if (host.empty())
		{
			std::cout << message << "\n";
			return;
		}

		std::cout << "[🧲 " << host << "] " << message << "\n";
	}

	void Application::PrintPrompt()
	{
		std::cout << "> ";
	}

	void Application::Run()
	{
		uint32_t skipCounter = 0;
		bool skipFirst = true;

		for (int i = 0; i < m_Arguments.count; i++)
		{
			bool hasNext = i + 1 < m_Arguments.count;
			if (skipFirst)
			{
				if (!hasNext)
				{
					MG_LOG("It seems like you forgot to specify a command. Try `magnet help` for more information.");
					break;
				}

				skipFirst = false;
				continue;
			}

			if (skipCounter > 0)
			{
				skipCounter--;
				continue;
			}

			std::string argument = m_Arguments.list[i];

			std::vector<std::string> nextArguments;
			PopulateNextArguments(&nextArguments, hasNext, i + 1);

			skipCounter = (int) nextArguments.size();

			CommandHandlerProps props;
			auto project = CreateConfiguredProject();
			props.project = &project;
			props.nextArguments = nextArguments;

			bool commandExists = m_Commands.find(argument) != m_Commands.end();
			if (commandExists)
			{
				if (!props.project->IsValid() && !CommandHandler::IsCommandGlobal(argument))
				{
					MG_LOG("It seems like there is no project in this folder, or the current configuration is corrupted. Try `magnet help` for more information.");
					break;
				}

				m_Commands.at(argument)(props);
				continue;
			}

			if (CheckTypo(argument))
				break;

			MG_LOG("Invalid command `" + argument + "`. Try `magnet help` for more information.");
		}
	}

	std::filesystem::path Application::GetCurrentWorkingDirectory()
	{
		return std::filesystem::current_path();
	}

	std::string Application::GetProjectName()
	{
		if (!IsRootLevel())
			return "";

		return GetYamlString(s_ConfigPath, "name");
	}

	std::string Application::GetProjectType()
	{
		if (!IsRootLevel())
			return "";

		return GetYamlString(s_ConfigPath, "projectType");
	}

	int Application::GetCppVersion()
	{
		if (!IsRootLevel())
			return -1;

		return GetYamlInt(s_ConfigPath, "cppVersion");
	}

	[[maybe_unused]] void Application::SetCppVersion(int version)
	{
		if (!IsRootLevel())
			return;

		SetYamlInt(s_ConfigPath, "cppVersion", version);
	}

	std::string Application::GetCmakeVersion()
	{
		if (!IsRootLevel())
			return "";

		return GetYamlString(s_ConfigPath, "cmakeVersion");
	}

	[[maybe_unused]] void Application::SetCmakeVersion(const std::string& version)
	{
		if (!IsRootLevel())
			return;

		SetYamlString(s_ConfigPath, "cmakeVersion", version);
	}

	std::string Application::GetDefaultConfiguration()
	{
		if (!IsRootLevel())
			return "";

		return GetYamlString(s_ConfigPath, "defaultConfiguration");
	}

	void Application::SetDefaultConfiguration(const Configuration& configuration)
	{
		if (!IsRootLevel())
			return;

		SetYamlString(s_ConfigPath, "defaultConfiguration", configuration.ToString());
	}

	std::vector<std::string> Application::GetDependencies()
	{
		if (!IsRootLevel())
			return {};

		YAML::Node dependencies = YAML::LoadFile(".magnet/dependencies.yaml");
		if (dependencies["dependencies"])
			return dependencies["dependencies"].as<std::vector<std::string>>();

		return {};
	}

	bool Application::IsRootLevel()
	{
		return std::filesystem::exists(".magnet");
	}

	std::string Application::GetYamlString(const std::string& path, const std::string& key)
	{
		YAML::Node config = YAML::LoadFile(path);

		auto node = config[key];
		if (node)
			return node.as<std::string>();

		return "";
	}

	void Application::SetYamlString(const std::string& path, const std::string& key, const std::string& value)
	{
		YAML::Node config = YAML::LoadFile(path);
		config[key] = value;

		std::ofstream file(path);
		file << config;
		file.close();
	}

	int Application::GetYamlInt(const std::string& path, const std::string& key)
	{
		YAML::Node config = YAML::LoadFile(path);

		auto node = config[key];
		if (node)
			return node.as<int>();

		return -1;
	}

	void Application::SetYamlInt(const std::string& path, const std::string& key, int value)
	{
		YAML::Node config = YAML::LoadFile(path);
		config[key] = value;

		std::ofstream file(path);
		file << config;
		file.close();
	}

	Project Application::CreateConfiguredProject()
	{
		Project project;

		project.SetName(Application::GetProjectName());
		project.SetType(Application::GetProjectType());
		project.SetCppVersion(Application::GetCppVersion());
		project.SetCmakeVersion(Application::GetCmakeVersion());
		project.SetConfiguration(Configuration::FromString(Application::GetDefaultConfiguration()));

		return project;
	}

	void Application::PopulateNextArguments(std::vector<std::string>* arguments, bool hasNext, int startIndex)
	{
		if (hasNext)
		{
			for (int j = startIndex; j < m_Arguments.count; j++)
				arguments->emplace_back(m_Arguments.list[j]);
		}
	}

	bool Application::CheckTypo(const std::string& argument)
	{
		bool similarCommandExists = m_SimilarCommands.find(argument) != m_SimilarCommands.end();
		if (similarCommandExists)
		{
			const std::string& similarCommand = m_SimilarCommands.at(argument);
			MG_LOG("Did you mean `" + similarCommand + "`?");
			return true;
		}

		return false;
	}
}
