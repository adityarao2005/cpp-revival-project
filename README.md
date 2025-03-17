# cpp-revival-project
C++ Revival project. To revive C++ programming in a safe, scalable, and feature-friendly way.

## Introduction
C++ has come a long way from being the unsafe object-oriented version of C. With languages like Java, C#, Python, and JavaScript in the view for much of the market in programming, many people have forgotten the power that C++ has and the programs that you can build in C++. The goal of this project is to provide a framework which will help power up C++ such that it will be as programmatically friendly, safe, and scalable as the languages most commonly used today.

## Tools Provided

The tools provided in this project are what C++ has been missing that Java, C#, Python, JS, Rust, etc have.
These features are the following:

- A package manager and project orchestrator tool. CMake sucks. Maven, Gradle, npm, and cargo are so much better. When I started getting into C++, creating a C++ project caused me with CMake caused me the most issues rather than developing and building the project.
- An application (web) framework. C++ does not have a standard and lightweight one which implements the best of its features (C++ 20 and beyond). Although Boost is good, it does not give developers the same programmatic feel as when developing applications with Spring or ASP.NET Core or Express or Django.

## CPM - The C++ Package Manager

Will support package management for C++. Will help with compilation, building, testing, packaging, and dependency management for the project's lifecycle.
The commands would look like this:

### Initialize project command
```bash
cpm init
```
Creates a new project in the current directory.
**File structure**
```
 - (current directory)
 +----- src/ (created)
 +----- test/ (created)
 +----- include/ (created)
 +----- build/ (created)
 |       +----- bin/ (created)
 |       +----- generated/ (created)
 |       +----- obj/ (created)
 +----- lib/ (created)
 +----- project.json
```

**project.json content**
```json
{
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
      "bin": ["./build/bin"],
      "outputName": "a.out",
      "macros": [],
    },
  },
  "dependencies": {
  }
}
```

