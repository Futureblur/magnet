#include "Application.h"

#include "yaml-cpp/yaml.h"

#include "CommandHandler.h"
#include "Core.h"

namespace MG
{
	// Map commands to Handler functions.
	static const std::unordered_map<std::string, void (*)(const CommandHandlerProps&)> m_Commands = {
			{"help",     CommandHandler::HandleHelpCommand},
			{"new",      CommandHandler::HandleNewCommand},
			{"generate", CommandHandler::HandleGenerateCommand},
			{"build",    CommandHandler::HandleBuildCommand},
			{"go",       CommandHandler::HandleGoCommand},
			{"clean",    CommandHandler::HandleCleanCommand},
			{"pull",     CommandHandler::HandlePullCommand},
			{"remove",   CommandHandler::HandleRemoveCommand}
	};

	void Application::Init(const CommandLineArguments& args)
	{
		m_Arguments = args;
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
		for (int i = 0; i < m_Arguments.count; i++)
		{
			std::string argument = m_Arguments.list[i];

			std::string projectName = Application::GetProjectName();
			std::string projectType = Application::GetProjectType();
			int cppVersion = Application::GetCppVersion();

			bool hasNext = i + 1 < m_Arguments.count;
			std::string nextArgument = hasNext ? m_Arguments.list[i + 1] : "";

			if (argument == "magnet" && !hasNext)
			{
				MG_LOG("No argument provided. Try `magnet help` for more information.");
				continue;
			}

			CommandHandlerProps props;
			props.projectName = projectName;
			props.projectType = projectType;
			props.cppVersion = cppVersion;
			props.nextArgument = nextArgument;

			bool commandExists = m_Commands.find(argument) != m_Commands.end();
			if (commandExists)
			{
				if (!props.IsValid() && !CommandHandler::IsCommandGlobal(argument))
				{
					MG_LOG("It seems like there is no project in this folder, or the current configuration is corrupted. Try `magnet help` for more information.");
					break;
				}
				
				m_Commands.at(argument)(props);
				continue;
			}

			bool hasPrevious = i - 1 >= 0;
			if (!hasPrevious)
				continue;

			std::string previousArgument = m_Arguments.list[i - 1];
			if (previousArgument == "pull" || previousArgument == "--list" || previousArgument == "remove")
				continue;

			MG_LOG("Invalid command `" + argument + "`. Try `magnet help` for more information.");
		}
	}

	std::string Application::GetCurrentWorkingDirectory()
	{
		return std::filesystem::current_path().string();
	}

	std::string Application::GetProjectName()
	{
		if (!IsRootLevel())
			return "";

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");

		auto nameNode = config["name"];
		if (nameNode)
			return nameNode.as<std::string>();

		return "";
	}

	std::string Application::GetProjectType()
	{
		if (!IsRootLevel())
			return "";

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");

		auto projectTypeNode = config["projectType"];
		if (projectTypeNode)
			return projectTypeNode.as<std::string>();

		return "";
	}

	int Application::GetCppVersion()
	{
		if (!IsRootLevel())
			return -1;

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");

		auto cppDialectNode = config["cppDialect"];
		if (cppDialectNode)
			return cppDialectNode.as<int>();

		return 0;
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
}
