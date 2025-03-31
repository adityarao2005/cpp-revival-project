#include "cmdline.hpp"
#include <iostream>

void cpm::CmdLine::onCommand(const std::function<void(std::vector<std::string>)> &callback, std::vector<std::string> command)
{
    // Store the command and its associated callback
    for (const auto &cmd : command)
    {
        commands[cmd] = callback;
    }
}

int cpm::CmdLine::parse(int argc, char **argv)
{
    // Check if there are any arguments
    if (argc < 2)
    {
        auto it = commands.find("usage");

        if (it != commands.end())
        {
            it->second({});
        }
        else
        {
            // If no command is provided, display usage
            std::cerr << "No command provided. Use 'help' for usage information." << std::endl;
        }

        return 1; // No command provided
    }

    // Get the command from the arguments
    std::string command = argv[1];

    // Check if the command exists in the map
    auto it = commands.find(command);
    if (it != commands.end())
    {
        // Call the associated callback function with the remaining arguments
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i)
        {
            args.push_back(argv[i]);
        }
        it->second(args);

        return 0; // Success
    }
    else
    {
        return 1; // Command not found
    }
}