#include "project.hpp"

using namespace craft::project;
using namespace craft::project::file;

std::optional<std::pair<std::filesystem::path, ProjectFileType>> craft::project::file::identifyProjectFile()
{
    throw std::runtime_error("identifyProjectFile not implemented");
}

std::optional<std::pair<std::unique_ptr<ProjectFile>, ProjectFileType>> craft::project::file::readProjectFile(std::filesystem::path projectPath, ProjectFileType type)
{
    throw std::runtime_error("readProjectFile not implemented");
}

void craft::project::file::writeProjectFile(std::filesystem::path projectPath, ProjectFileType type, std::unique_ptr<ProjectFile> projectFile)
{
    throw std::runtime_error("writeProjectFile not implemented");
}

void craft::project::dependency::source::VcpkgSource::extractLibrary()
{
}

void craft::project::dependency::source::GitSource::extractLibrary()
{
}

void craft::project::dependency::source::FolderSource::extractLibrary()
{
}

void craft::project::dependency::source::ZipSource::extractLibrary()
{
}

void craft::project::dependency::source::SystemLibrarySource::extractLibrary()
{
}

void craft::project::dependency::HeaderOnlyDependency::build()
{
}

void craft::project::dependency::LibraryDependency::build()
{
}

void craft::project::dependency::SourceCodeDependency::build()
{
}
