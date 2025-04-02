#pragma once
#include <string>
#include <vector>

namespace cpm
{

    struct ProjectFile;
    struct Project;
    struct BuildConfig;
    struct Dependency;

    struct ProjectFile
    {
        std::string name;
        std::string version;
        std::string description;
        std::string license;
        Project project;
        std::vector<Dependency> dependencies;
    };

    struct Project
    {
        std::string std;
        std::vector<std::string> srcDirs;
        std::vector<std::string> srcFiles;
        std::vector<std::string> includeDirs;
        std::vector<std::string> includeFiles;
        std::vector<std::string> testDirs;
        std::vector<std::string> libDirs;
        BuildConfig build;
        std::string type;
    };

    struct BuildConfig
    {
        std::string warningLevel;
        std::vector<std::string> binDirs;
        std::vector<std::string> macros;
    };

    struct Dependency
    {
    };

    /**
     *
     *
  "name": "",
  "version": "0.0.1",
  "project": {
    "std": "c++20"
    "srcDir": ["./src"],
    "srcFiles": [],
    "includeDir": ["./include"],
    "includeFiles": [],
    "testDir": ["./test"]
    "libDir": ["./lib"],
    "build": {
      "warningLevel": "severe"
      "bin": ["./build/bin"],
      "macros": [],
    },
    "type": "executable"
  },
  "dependencies": {
  }
     *
     */
}