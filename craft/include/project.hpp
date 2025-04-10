#pragma once
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <memory>
#include <filesystem>

namespace craft
{
	/// @brief The project details of the project.
	namespace project
	{
		/// @brief dependency namespace which represents the dependencies of a project.
		namespace dependency
		{
			/// @brief source namespace which represents the dependency sources.
			namespace source
			{
				/// @brief DependencySource is a base class that represents a dependency source.
				class DependencySource
				{
				public:
					/// @brief Extracts the library from the source.
					virtual void extractLibrary() = 0;
				};

				/// @brief VcpkgSource is a class that represents a vcpkg dependency source.
				class VcpkgSource : public DependencySource
				{
				public:
					/// @brief The name of the package in vcpkg.
					std::string packageName;

					/// @brief Extracts the library from the source. Uses the vcpkg tool to install the package.
					void extractLibrary() override;
				};

				/// @brief GitSource is a class that represents a git dependency source.
				class GitSource : public DependencySource
				{
				public:
					/// @brief The URL of the git repository.
					std::string cloneUrl;

					/// @brief Extracts the library from the source. Uses git to clone the repository.
					void extractLibrary() override;
				};

				/// @brief FolderSource is a class that represents a folder dependency source.
				class FolderSource : public DependencySource
				{
				public:
					/// @brief The path to the folder.
					std::string path;

					/// @brief Extracts the library from the source. Copies the library contents into the lib folder.
					void extractLibrary() override;
				};

				/// @brief ZipSource is a class that represents a zip file dependency source.
				class ZipSource : public DependencySource
				{
				public:
					/// @brief The path to the zip file.
					std::string file;

					/// @brief Extracts the library from the source. Extracts the zip library into the lib folder.
					void extractLibrary() override;
				};

				/// @brief SystemSource is a class that represents a system dependency source.
				class SystemLibrarySource : public DependencySource
				{
				public:
					/// @brief The name of the library in the system.
					std::string libName;

					/// @brief Extracts the library from the source. Does nothing as the library is already installed in the system.
					/// @note This is a placeholder implementation. In a real-world scenario, this would check if the library is installed.
					/// and if not, it would install it somehow.
					void extractLibrary() override;
				};
			}

			/// @brief Dependency is a base class that represents a dependency.
			class Dependency
			{
			public:
				/// @brief The includePath of the dependency.
				/// @note Not all the include paths are required. For example, a system library dependency may not have an include path.
				std::optional<std::string> includePath;
				/// @brief The source of the dependency.
				/// @note The source can be a vcpkg, git, folder, zip, or system library.
				std::unique_ptr<source::DependencySource> source;

				/// @brief Builds the dependency.
				virtual void build() = 0;
			};

			/// @brief HeaderOnlyDependency is a class that represents a header-only dependency.
			class HeaderOnlyDependency : public Dependency
			{
			public:
				/// @brief Builds the dependency.
				/// @note This does nothing as header-only dependencies do not require building.
				void build() override;
			};

			/// @brief LibraryType is an enum class that represents the type of library.
			enum class LibraryType
			{
				/// @brief Static library.
				STATIC,
				/// @brief Dynamic library.
				DYNAMIC
			};

			/// @brief LibraryDependency is a class that represents a library dependency.
			class LibraryDependency : public Dependency
			{
			public:
				/// @brief The type of library.
				LibraryType type;
				/// @brief The path to the library.
				/// @note This is optional as the library might be a system or vcpkg library.
				std::optional<std::string> libPath;

				/// @brief Builds the dependency.
				/// @note This does nothing as the library dependencies do not require building as they are already built.
				void build() override;
			};

			/// @brief SourceCodeDependency is a class that represents a source code dependency.
			class SourceCodeDependency : public Dependency
			{
			public:
				/// @brief The path to the source code.
				std::string srcPath;
				/// @brief The build command to build the source code.
				std::string buildCommand;
				/// @brief The path of the library outputed.
				std::string libOutput;

				/// @brief Builds the dependency.
				/// @note This uses the build command to build the source code.
				void build() override;
			};
		}

		/// @brief WarningLevel is an enum class that represents the warning level.
		enum class WarningLevel
		{
			/// @brief No warnings.
			W0,
			/// @brief All warnings.
			W1,
			/// @brief All warnings and errors.
			W2,
			/// @brief All warnings, errors and extra warnings.
			W3,
			/// @brief All warnings, errors, extra warnings and extra extra warnings.
			W4
		};

		/// @brief Build is a struct that represents the build configuration.
		struct BuildConfig
		{
			/// @brief The build directory.
			std::string binDir;
			/// @brief List of macros to define.
			std::vector<std::string> marcos;
			/// @brief warning level of the build.
			WarningLevel warningLevel;
		};

		/// @brief ProjectType is an enum class that represents the type of project.
		enum class ProjectType
		{
			/// @brief Executable project.
			EXECUTABLE,
			/// @brief Static library project.
			STATIC_LIBRARY,
			/// @brief Dynamic library project.
			DYNAMIC_LIBRARY,
			/// @brief header only library project.
			HEADER_ONLY_LIBRARY,
		};

		/// @brief CXXVersion is an enum class that represents the C++ version.
		enum class CXXVersion
		{
			/// @brief C++ 11
			CXX11,
			/// @brief C++ 14
			CXX14,
			/// @brief C++ 17
			CXX17,
			/// @brief C++ 20
			CXX20,
			/// @brief C++ 23
			CXX23,
		};

		/// @brief Project is a class that represents a project. It can be a library or an executable.
		struct Project
		{
			/// @brief the test directories
			std::vector<std::string> testDirs;
			/// @brief the library directories
			std::vector<std::string> libDirs;
			/// @brief the type of the project
			ProjectType type;
			/// @brief the source files
			std::vector<std::string> srcFiles;
			/// @brief the source directories
			std::vector<std::string> srcDirs;
			/// @brief the build configuration
			std::unique_ptr<BuildConfig> build;
			/// @brief the include directories
			std::vector<std::string> includeDirs;
			/// @brief the include files
			std::vector<std::string> includeFiles;
			/// @brief the C++ version
			CXXVersion cxxVersion;
		};

		/// @brief ProjectFile is a class that represents a project file. It can be JSON or YAML or any other format.
		struct ProjectFile
		{
			/// @brief The name of the project
			std::string name;
			/// @brief The version of the project
			std::string version;
			/// @brief The description of the project
			std::optional<std::string> description;
			/// @brief The author of the project
			std::optional<std::string> author;
			/// @brief The license of the project
			std::optional<std::string> license;
			/// @brief The project details of the project
			std::unique_ptr<Project> project;
			/// @brief The dependencies of the project
			std::vector<std::unique_ptr<dependency::Dependency>> dependencies;
		};

		namespace file
		{

			enum class ProjectFileType
			{
				/// @brief JSON project file.
				JSON,
				/// @brief YAML project file.
				YAML,
				/// @brief XML project file.
				XML,
			};

			/// @brief Identifies the project file path and type.
			/// @return an optional of both
			std::optional<std::pair<std::filesystem::path, ProjectFileType>> identifyProjectFile();
			/// @brief Reads the project file from the given path.
			/// @param projectPath the project path of the project file.
			/// @param type the type of the project file.
			/// @return A pair of the project file and the type of the project file.
			std::optional<std::pair<std::unique_ptr<ProjectFile>, ProjectFileType>> readProjectFile(std::filesystem::path projectPath, ProjectFileType type);

			/// @brief Writes the project file to the given path.
			/// @param projectPath the project path of the project file.
			/// @param type the type of the project file.
			/// @param projectFile the project file to write.
			void writeProjectFile(std::filesystem::path projectPath, ProjectFileType type, std::unique_ptr<ProjectFile> projectFile);
		};
	}
}
