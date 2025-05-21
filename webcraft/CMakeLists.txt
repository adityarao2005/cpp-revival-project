cmake_minimum_required(VERSION 3.10)

project(WebCraft)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)


if (NOT APPLE)
    find_package(fmt CONFIG REQUIRED)
endif()

find_path(CPP_ASYNC_INCLUDE_DIRS "async/task.h")

if (NOT CPP_ASYNC_INCLUDE_DIRS)
    set(CPP_ASYNC_INCLUDE_DIRS "build/vcpkg_installed/arm64-osx/include")
endif()

if (WIN32)
elseif (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(uring REQUIRED IMPORTED_TARGET liburing)
elseif (UNIX AND APPLE)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()


file(GLOB_RECURSE WebCraft_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(WebCraft ${WebCraft_SOURCES})

if (NOT APPLE)
    target_link_libraries(WebCraft PRIVATE fmt::fmt)
endif()

if (WIN32)
    target_link_libraries(WebCraft PRIVATE wsock32 ws2_32)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(WebCraft PRIVATE PkgConfig::uring)
elseif (UNIX AND APPLE)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_include_directories(WebCraft PRIVATE ${CPP_ASYNC_INCLUDE_DIRS})
target_include_directories(WebCraft PRIVATE include)