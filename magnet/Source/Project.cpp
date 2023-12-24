#include "Project.h"

namespace MG
{
	std::string Configuration::ToString() const
	{
		switch (m_Mode)
		{
			case ConfigurationMode::Debug:
				return "Debug";
			case ConfigurationMode::Release:
				return "Release";
			default:
				return "";
		}
	}

	void Configuration::SetMode(const std::string& mode)
	{
		std::string lowerCaseMode = mode;
		std::transform(lowerCaseMode.begin(), lowerCaseMode.end(), lowerCaseMode.begin(),
		               [](unsigned char c) -> unsigned char
		               {
			               return static_cast<unsigned char>(std::tolower(c));
		               });

		if (lowerCaseMode == "debug")
			m_Mode = ConfigurationMode::Debug;
		else if (lowerCaseMode == "release")
			m_Mode = ConfigurationMode::Release;
	}

	bool Configuration::IsValid() const
	{
		return m_Mode != ConfigurationMode::Unknown;
	}

	Configuration Configuration::FromString(const std::string& string)
	{
		Configuration configuration;
		configuration.SetMode(string);
		return configuration;
	}

	Project Project::GetDefault()
	{
		Project project;

		project.m_Name = "MyProject";
		project.m_Type = ProjectType::Executable;
		project.m_CppVersion = CppVersion::Cpp17;
		project.m_CmakeVersion = "3.20";
		project.m_Configuration = Configuration::FromString("Debug");

		return project;
	}

	const std::string& Project::GetName() const
	{
		return m_Name;
	}

	void Project::SetName(const std::string& name)
	{
		if (name.empty())
			return;

		m_Name = name;
	}

	const ProjectType& Project::GetType() const
	{
		return m_Type;
	}

	std::string Project::GetTypeString() const
	{
		switch (m_Type)
		{
			case ProjectType::Executable:
				return "Executable";
			case ProjectType::StaticLibrary:
				return "StaticLibrary";
			case ProjectType::DynamicLibrary:
				return "DynamicLibrary";
			default:
				return "";
		}
	}

	std::string Project::GetCmakeTypeString() const
	{
		switch (m_Type)
		{
			case ProjectType::StaticLibrary:
				return "STATIC";
			case ProjectType::DynamicLibrary:
				return "SHARED";
			default:
				return "";
		}
	}

	void Project::SetType(const ProjectType& type)
	{
		if (type == ProjectType::Unknown)
			return;

		m_Type = type;
	}

	void Project::SetType(const std::string& type)
	{
		std::string lowerCaseType = type;
		std::transform(lowerCaseType.begin(), lowerCaseType.end(), lowerCaseType.begin(),
		               [](unsigned char c) -> unsigned char
		               {
			               return static_cast<unsigned char>(std::tolower(c));
		               });

		if (lowerCaseType == "executable")
			m_Type = ProjectType::Executable;
		else if (lowerCaseType == "staticlibrary")
			m_Type = ProjectType::StaticLibrary;
		else if (lowerCaseType == "dynamiclibrary")
			m_Type = ProjectType::DynamicLibrary;
	}

	int Project::GetCppVersion() const
	{
		switch (m_CppVersion)
		{
			case CppVersion::Cpp11:
				return 11;
			case CppVersion::Cpp14:
				return 14;
			case CppVersion::Cpp17:
				return 17;
			case CppVersion::Cpp20:
				return 20;
			default:
				return -1;
		}
	}

	void Project::SetCppVersion(int version)
	{
		switch (version)
		{
			case 11:
				m_CppVersion = CppVersion::Cpp11;
				break;
			case 14:
				m_CppVersion = CppVersion::Cpp14;
				break;
			case 17:
				m_CppVersion = CppVersion::Cpp17;
				break;
			case 20:
				m_CppVersion = CppVersion::Cpp20;
				break;
			default:
				break;
		}
	}

	const std::string& Project::GetCmakeVersion() const
	{
		return m_CmakeVersion;
	}

	void Project::SetCmakeVersion(const std::string& version)
	{
		if (version.empty())
			return;

		m_CmakeVersion = version;
	}

	const Configuration& Project::GetConfiguration() const
	{
		return m_Configuration;
	}

	void Project::SetConfiguration(const MG::Configuration& configuration)
	{
		if (!configuration.IsValid())
			return;

		m_Configuration = configuration;
	}

	bool Project::IsValid() const
	{
		return !m_Name.empty() && GetType() != ProjectType::Unknown && GetCppVersion() != -1 &&
		       m_Configuration.IsValid() && !m_CmakeVersion.empty();
	}
}
