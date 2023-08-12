#include "Application.h"

#include "yaml-cpp/yaml.h"

#include "CommandHandler.h"
#include "Core.h"

namespace MG
{
	void Application::Init(const CommandLineArguments& args)
	{
		m_Arguments = args;
	}

	void Application::Print(const std::string& host, const std::string& message)
	{
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

			if (argument == "new")
			{
				CommandHandler::HandleNewCommand(&m_Arguments, i);
			}
			else if (argument == "generate")
			{
				CommandHandler::HandleGenerateCommand();
			}
			else if (argument == "build")
			{
				CommandHandler::HandleBuildCommand();
			}
			else if (argument == "go")
			{
				CommandHandler::HandleGoCommand();
			}
			else if (argument == "clean")
			{
				CommandHandler::HandleCleanCommand();
			}
			else if (argument == "pull")
			{
				CommandHandler::HandlePullCommand(&m_Arguments, i);
			}
			else if (argument == "remove")
			{
				CommandHandler::HandleRemoveCommand(&m_Arguments, i);
			}
			else if (argument == "run")
			{
				MG_LOG("Invalid command `" + argument +
				       "`. Did you mean `magnet go` to run your app? If not, try `magnet help` for more information.");
			}
			else
			{
				if (argument == "magnet")
					continue;

				if (i == 0)
				{
					MG_LOG("No argument provided. Try `magnet help` for more information.");

					continue;
				}

				std::string previousArgument = m_Arguments.list[i - 1];
				if (previousArgument == "new" || previousArgument == "pull" || previousArgument == "--list" ||
				    previousArgument == "remove")
					continue;

				MG_LOG("Invalid command `" + argument + "`. Try `magnet help` for more information.");
			}
		}
	}

	std::string Application::GetCurrentWorkingDirectory()
	{
		return std::filesystem::current_path();
	}

	std::string Application::GetProjectName()
	{
		if (!IsRootLevel())
			return "";

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");
		if (config["name"])
			return config["name"].as<std::string>();

		return "";
	}

	std::string Application::GetProjectType()
	{
		if (!IsRootLevel())
			return "";

		YAML::Node config = YAML::LoadFile(".magnet/config.yaml");
		if (config["projectType"])
			return config["projectType"].as<std::string>();

		return "";
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
