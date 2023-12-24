#pragma once

namespace MG
{
	// Represents the type of project.
	enum class ProjectType
	{
		Unknown,
		Executable,
		StaticLibrary,
		DynamicLibrary
	};

	// A list of all supported C++ versions.
	enum class CppVersion
	{
		Cpp11,
		Cpp14,
		Cpp17,
		Cpp20
	};

	// Represents the configuration mode.
	enum class ConfigurationMode
	{
		Unknown,
		Debug,
		Release
	};

	// Represents the configuration mode.
	struct Configuration
	{
		ConfigurationMode m_Mode = ConfigurationMode::Unknown;

		// Returns the configuration mode as a string.
		[[nodiscard]] std::string ToString() const;

		// Sets the configuration mode from a string, but only if it's valid.
		void SetMode(const std::string& mode);

		// Returns whether the configuration is valid, meaning it has a mode.
		[[nodiscard]] bool IsValid() const;

		// Returns a configuration from a string.
		[[nodiscard]] static Configuration FromString(const std::string& string);
	};

	// Represents a project with attributes such as name, type and C++ version.
	class Project
	{
	public:
		// Returns a project with default settings.
		static Project GetDefault();

		[[nodiscard]] const std::string& GetName() const;
		void SetName(const std::string& name);

		[[nodiscard]] const ProjectType& GetType() const;
		[[nodiscard]] std::string GetTypeString() const;

		// Returns the type as a string that can be used in CMake.
		// Converts StaticLibrary to STATIC and DynamicLibrary to SHARED.
		[[nodiscard]] std::string GetCmakeTypeString() const;
		void SetType(const ProjectType& type);
		void SetType(const std::string& type);

		[[nodiscard]] int GetCppVersion() const;
		void SetCppVersion(int version);

		[[nodiscard]] const std::string& GetCmakeVersion() const;
		void SetCmakeVersion(const std::string& version);

		[[nodiscard]] const Configuration& GetConfiguration() const;
		void SetConfiguration(const Configuration& configuration);

		// Returns whether the project is valid, meaning it has a name, type and C++ version.
		[[nodiscard]] bool IsValid() const;

	private:
		std::string m_Name;
		ProjectType m_Type = ProjectType::Unknown;
		CppVersion m_CppVersion = CppVersion::Cpp17;
		std::string m_CmakeVersion;
		Configuration m_Configuration;
	};
}
