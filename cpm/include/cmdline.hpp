#pragma once
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>

namespace cpm
{

    class CmdLine
    {
    private:
        std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> commands;

    public:
        void onCommand(const std::function<void(std::vector<std::string>)> &callback, std::vector<std::string> command);
        int parse(int argc, char **argv);
    };

}

#define CREATE_COMMAND_LINE cpm::CmdLine cmdline
#define PARSE_ON(function, args...) cmdline.onCommand(function, {args})
#define PARSE(argc, argv) cmdline.parse(argc, argv)