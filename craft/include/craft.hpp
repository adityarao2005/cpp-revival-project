#pragma once
#include <string>
#include <vector>

namespace craft
{

    /// @brief Initialize the C++ project in the current directory.
    /// @details This function sets up the necessary files and directories for the C++ project.
    /// @note This function should be called only once to set up the project.
    void init();

    /// @brief Builds the C++ project.
    /// @details This function compiles the source code and generates the executable or library.
    /// @note This function should be called after the project is initialized and source code is added.
    /// @param args build flags
    void build(std::vector<std::string> args);

    /// @brief Runs the C++ project.
    /// @details This function executes the compiled project.
    /// @note This function should be called after the project is built. This function should only be used if the project is an executable
    /// @param args Runs the project with the given arguments.
    void run(std::vector<std::string> args);

    /// @brief Tests the C++ project.
    /// @details This function runs the test cases for the project.
    /// @note This function should be called after the project is built and test cases are added.
    /// @param args Test flags
    void test(std::vector<std::string> args);

    /// @brief Exports the project.
    /// @param args Arguments
    void _export();

    /// @brief Cleans the project
    /// @details This function removes the build artifacts and temporary files.
    /// @note This function should be called when you want to clean the project.
    void clean();

    /// @brief Installs all the required dependencies for the project.
    void install();

    /// @brief Displays the help message.
    /// @details This function provides information about the available commands and their usage.
    void help();

    /// @brief Displays the usage message.
    /// @details This function provides information about the command line arguments and their usage.
    void usage();

    /// @brief Handles the command line arguments.
    /// @param argc Count of arguments
    /// @param argv Number of arguments
    /// @return 0 on success, 1 on failure
    int handle_cmdline(int argc, char **argv);
}