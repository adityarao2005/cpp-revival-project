cmake_minimum_required(VERSION 3.10)

project(WebCraft)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt CONFIG REQUIRED)
find_path(CPP_ASYNC_INCLUDE_DIRS "async/task.h" REQUIRED)

file(GLOB_RECURSE WebCraft_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(WebCraft ${WebCraft_SOURCES})

target_link_libraries(WebCraft PRIVATE fmt::fmt)
target_include_directories(WebCraft PRIVATE ${CPP_ASYNC_INCLUDE_DIRS})
target_include_directories(WebCraft PRIVATE include)