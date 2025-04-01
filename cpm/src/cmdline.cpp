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

void cpm::CmdLine::usagePart(const std::string &str)
{

    auto it = commands.find("usage");

    if (it != commands.end())
    {
        it->second({});
    }
    else
    {
        // If no command is provided, display usage
        std::cerr << str << std::endl;
    }
}

int cpm::CmdLine::parse(int argc, char **argv)
{
    // Check if there are any arguments
    if (argc < 2)
    {
        usagePart("No command provided. Use 'help' for usage information.");

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

        try
        {
            it->second(args);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1; // Error occurred
        }

        return 0; // Success
    }
    else
    {
        usagePart("Usage did not work. Use 'help' for usage information.");
        return 1; // Command not found
    }
}