# cpp-revival-project
C++ Revival project. To revive C++ programming in a safe, scalable, and feature-friendly way.

## Introduction
C++ has come a long way from being the unsafe object-oriented version of C that people used to know. With languages like Java, C#, Python, and JavaScript in the view for much of the market in programming, many people have forgotten the power that C++ has and the programs that you can build in C++. The goal of this project is to provide a framework which will help power up C++ such that it will be as programmatically friendly, safe, and scalable as the languages most commonly used today.

## Tools Provided

The tools provided in this project are what C++ has been missing that Java, C#, Python, JS, Rust, etc have.
These features are the following:

- A package manager and project orchestrator tool. CMake sucks. Maven, Gradle, npm, and cargo are so much better. When I started getting into C++, creating a C++ project caused me with CMake caused me the most issues rather than developing and building the project. - **Craft**
- An application (web) framework. C++ does not have a standard and lightweight one which implements the best of its features (C++ 20 and beyond). Although Boost is good, it does not give developers the same programmatic feel as when developing applications with Spring or ASP.NET Core or Express or Django. - **WebCraft**

## craft - The C++ Package Manager

Will support package management for C++. Will help with compilation, building, testing, packaging, and dependency management for the project's lifecycle.

## Project Orchestration
### Initialize project command
```bash
craft init
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
      "warningLevel": "severe"
      "bin": "./build/bin",
      "macros": [],
      "target": [ "current" ]
    },
    "type": "executable"
  },
  "dependencies": {
  }
}
```

### Build project command
```bash
craft build <flags>
```
Builds the project and puts the output objects and binaries in the $project.build.bin folder.

Will run either one of the following commands based on the compiler detected:
**MSVC** (cl.exe):
```bash
cl /I include /link /LIBPATH:lib /Fe:build\my_program.exe src\*.cpp /std:c++20 /W4
```

**GCC** (g++):
```bash
g++ -I include -L lib -o build/my_program src/*.cpp -std=c++20 -Wall -Wextra -Wpedantic
```

**Clang** (clang++):
```bash
clang++ -I include -L lib -o build/my_program src/*.cpp -std=c++20 -Wall -Wextra -Wpedantic
```

### Run project command
**NOTE: This will only "run" the project if the type of the project is an 'executable'**

```bash
craft run <cmd line arguments>
```

Faster way of running the project rather than doing:
```bash
craft build
$executable <cmd line arguments>
```

### Test project command
```bash
craft test
```

Runs all test cases (unit tests, integration tests, and system tests).
**NOTE: I have not decided how the testing for this should be implemented**

### Export project command
```bash
craft export
```

Will provide a command line interface for exporting the project.
Will build the project in all the provided target machines and architectures in the project.json file.
For example:
If the target is the following: target: [ "windows:x86", "windows:x64", "linux:arm64" ]
Then the file tree will be generated after running export:
```
 + build
 +-- bin/
 +-- target/
 |    +-- windows
 |    |    +-- x86
 |    |    +-- x64
 |    +-- linux/arm64
```
Target can be 'OS:architecture', 'current', or 'all'. By default and for simplicity, it is 'current'
A feature to push these binaries to a platform will be added soon.
If the project is a library then it will bundle the binaries and header files into a zip file to be sent over to some platform.
If the project is an executable then it will bundle the binaries with the executable together.

## Package Management

### Clean project command
```bash
craft clean
```

Cleans up the dependencies and build files of the project

### Install project command
```bash
craft install
```

Fetches dependencies and compiles them into the lib folder for use.
