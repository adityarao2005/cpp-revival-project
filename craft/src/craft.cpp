#include "craft.hpp"
#include <stdexcept>
#include <iostream>
#include <string>
#include <filesystem>
#include "cmdline.hpp"
#include "project.hpp"

// Singleton object of the project file
std::unique_ptr<craft::project::ProjectFile> projectFile = nullptr;
std::optional<std::pair<std::filesystem::path, craft::project::file::ProjectFileType>> projectFilePath = std::nullopt;

// TODO: Implement this function
void craft::init()
{
    std::cout << "Project initialization beginning" << std::endl;

    throw std::runtime_error("not implemented");
}

/// @brief Initializes the project variables.
/// @note This function is called when the project is initialized.
/// @throws std::runtime_error if the project file is not found or cannot be read.
void initializeProjectVariables()
{
    // Check if the project file exists
    projectFilePath = craft::project::file::identifyProjectFile();
    if (projectFilePath)
    {
        // Read the project file
        auto projectFileData = craft::project::file::readProjectFile(projectFilePath->first, projectFilePath->second);
        if (projectFileData)
        {
            // Set the project file
            projectFile = std::move(projectFileData->first);
            std::cout << "Project file loaded successfully" << std::endl;
        }
        else
        {
            throw std::runtime_error("Failed to read project file");
        }
    }
    else
    {
        throw std::runtime_error("Project file not found");
    }
}

// TODO: Implement this function
void craft::build(std::vector<std::string> args)
{
    std::cout << "[build]" << std::endl;
    initializeProjectVariables();

    throw std::runtime_error("not implemented");
}

// TODO: Implement this function
void craft::run(std::vector<std::string> args)
{
    std::cout << "[run]" << std::endl;
    initializeProjectVariables();

    throw std::runtime_error("not implemented");
}

// TODO: Implement this function
void craft::test(std::vector<std::string> args)
{
    std::cout << "[test]" << std::endl;
    initializeProjectVariables();

    throw std::runtime_error("not implemented");
}

// TODO: Implement this function
void craft::_export()
{
    std::cout << "[export]" << std::endl;
    initializeProjectVariables();

    throw std::runtime_error("not implemented");
}

// TODO: Implement this function
void craft::clean()
{
    std::cout << "[clean]" << std::endl;
    initializeProjectVariables();

    throw std::runtime_error("not implemented");
}

// TODO: Implement this function
void craft::install()
{
    std::cout << "[install]" << std::endl;
    initializeProjectVariables();

    throw std::runtime_error("not implemented");
}

void craft::help()
{
    std::cout << "[help]" << std::endl;
}

void craft::usage()
{
    std::cout << "Usage: craft [command] [options]" << std::endl;
}

void version()
{
    std::cout << "craft version 1.0.0" << std::endl;
}

int craft::handle_cmdline(int argc, char **argv)
{
    CREATE_COMMAND_LINE;
    PARSE_ON([](std::vector<std::string> args)
             { craft::init(); },
             "init");
    PARSE_ON([](std::vector<std::string> args)
             { craft::build(args); },
             "build", "b", "-b");
    PARSE_ON([](std::vector<std::string> args)
             { craft::run(args); },
             "run", "r", "-r");
    PARSE_ON([](std::vector<std::string> args)
             { craft::test(args); },
             "test", "t", "-t");
    PARSE_ON([](std::vector<std::string> args)
             { craft::_export(); },
             "export", "e", "-e");
    PARSE_ON([](std::vector<std::string> args)
             { craft::clean(); },
             "clean", "c", "-c");
    PARSE_ON([](std::vector<std::string> args)
             { craft::install(); },
             "install", "i", "-i");
    PARSE_ON([](std::vector<std::string> args)
             { craft::help(); },
             "help", "h", "-h");
    PARSE_ON([](std::vector<std::string> args)
             { craft::usage(); },
             "usage", "u", "-u");
    PARSE_ON([](std::vector<std::string> args)
             { version(); },
             "version", "v", "-v");

    return PARSE(argc, argv);
}