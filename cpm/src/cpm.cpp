#include "cpm.hpp"
#include <stdexcept>
#include <iostream>
#include <string>
#include <filesystem>
#include "cmdline.hpp"

void cpm::init()
{
    throw std::runtime_error("init not implemented");
}

void cpm::build(std::vector<std::string> args)
{
    throw std::runtime_error("build not implemented");
}

void cpm::run(std::vector<std::string> args)
{
    throw std::runtime_error("run not implemented");
}

void cpm::test(std::vector<std::string> args)
{
    throw std::runtime_error("test not implemented");
}

void cpm::_export()
{
    throw std::runtime_error("export not implemented");
}

void cpm::clean()
{
    throw std::runtime_error("clean not implemented");
}

void cpm::install()
{
    throw std::runtime_error("install not implemented");
}

void cpm::help()
{
    throw std::runtime_error("help not implemented");
}

void cpm::usage()
{
    throw std::runtime_error("usage not implemented");
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
             { std::cout << "CPM version 1.0.0" << std::endl; },
             "version", "v", "-v");

    return PARSE(argc, argv);
}