#include "project.hpp"

using namespace cpm::project;
using namespace cpm::project::file;

std::optional<std::pair<std::filesystem::path, ProjectFileType>> cpm::project::file::identifyProjectFile()
{
    throw std::runtime_error("identifyProjectFile not implemented");
}

std::optional<std::pair<std::unique_ptr<ProjectFile>, ProjectFileType>> cpm::project::file::readProjectFile(std::filesystem::path projectPath, ProjectFileType type)
{
    throw std::runtime_error("readProjectFile not implemented");
}

void cpm::project::file::writeProjectFile(std::filesystem::path projectPath, ProjectFileType type, std::unique_ptr<ProjectFile> projectFile)
{
    throw std::runtime_error("writeProjectFile not implemented");
}