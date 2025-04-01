#include "cpm.hpp"
#include <stdexcept>
#include <iostream>
#include <string>
#include <filesystem>
#include "cmdline.hpp"

void cpm::init()
{
    std::cout << "[init]" << std::endl;
    // throw std::runtime_error("init not implemented");
}

void cpm::build(std::vector<std::string> args)
{
    std::cout << "[build]" << std::endl;
}

void cpm::run(std::vector<std::string> args)
{
    std::cout << "[run]" << std::endl;
}

void cpm::test(std::vector<std::string> args)
{
    std::cout << "[test]" << std::endl;
}

void cpm::_export()
{
    std::cout << "[export]" << std::endl;
}

void cpm::clean()
{
    std::cout << "[clean]" << std::endl;
}

void cpm::install()
{
    std::cout << "[install]" << std::endl;
}

void cpm::help()
{
    std::cout << "[help]" << std::endl;
}

void cpm::usage()
{
    std::cout << "Usage: cpm [command] [options]" << std::endl;
}

void version()
{
    std::cout << "CPM version 1.0.0" << std::endl;
}

int cpm::handle_cmdline(int argc, char **argv)
{
    CREATE_COMMAND_LINE;
    PARSE_ON([](std::vector<std::string> args)
             { cpm::init(); },
             "init");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::build(args); },
             "build", "b", "-b");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::run(args); },
             "run", "r", "-r");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::test(args); },
             "test", "t", "-t");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::_export(); },
             "export", "e", "-e");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::clean(); },
             "clean", "c", "-c");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::install(); },
             "install", "i", "-i");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::help(); },
             "help", "h", "-h");
    PARSE_ON([](std::vector<std::string> args)
             { cpm::usage(); },
             "usage", "u", "-u");
    PARSE_ON([](std::vector<std::string> args)
             { version(); },
             "version", "v", "-v");

    return PARSE(argc, argv);
}