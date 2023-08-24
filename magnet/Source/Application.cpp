#include "Application.h"

#include "yaml-cpp/yaml.h"

#include "CommandHandler.h"
#include "Core.h"
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
		uint32_t skipCounter = 0;

		for (int i = 0; i < m_Arguments.count; i++)
		{
			if (skipCounter > 0)
			{
				skipCounter--;
				continue;
			}

			std::string argument = m_Arguments.list[i];

			bool hasNext = i + 1 < m_Arguments.count;

			// Capture all arguments after the current one in a vector.
			std::vector<std::string> nextArguments;
			if (hasNext)
			{
				for (int j = i + 1; j < m_Arguments.count; j++)
					nextArguments.emplace_back(m_Arguments.list[j]);
			}

			if (argument == "magnet")
			{
				if (!hasNext)
					MG_LOG("No argument provided. Try `magnet help` for more information.");

				continue;
			}

			skipCounter = (int) nextArguments.size();

			CommandHandlerProps props;

			Project project;
			project.SetName(Application::GetProjectName());
			project.SetType(Application::GetProjectType());
			project.SetCppVersion(Application::GetCppVersion());
			project.SetCmakeVersion(Application::GetCmakeVersion());
			project.SetConfiguration(Configuration::FromString(Application::GetDefaultConfiguration()));

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

		auto cppDialectNode = config["cppVersion"];
		if (cppDialectNode)
			return cppDialectNode.as<int>();

		return -1;
	}

	std::string Application::GetCmakeVersion()
	{
		if (!IsRootLevel())
			return "";

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");

		auto cmakeVersionNode = config["cmakeVersion"];
		if (cmakeVersionNode)
			return cmakeVersionNode.as<std::string>();

		return "";
	}

	[[maybe_unused]] void Application::SetCmakeVersion(const std::string& version)
	{
		if (!IsRootLevel())
			return;

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");
		config["cmakeVersion"] = version;

		std::ofstream file(".magnet/config.yaml");
		file << config;
		file.close();
	}

	std::string Application::GetDefaultConfiguration()
	{
		if (!IsRootLevel())
			return "";

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");

		auto defaultConfigurationNode = config["defaultConfiguration"];
		if (defaultConfigurationNode)
			return defaultConfigurationNode.as<std::string>();

		return "";
	}

	void Application::SetDefaultConfiguration(const Configuration& configuration)
	{
		if (!IsRootLevel())
			return;

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");
		config["defaultConfiguration"] = configuration.ToString();

		std::ofstream file(".magnet/config.yaml");
		file << config;
		file.close();
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
