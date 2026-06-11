cmake_minimum_required(VERSION 3.10)
project(network-c C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE include)

target_compile_options(main PRIVATE -Wall -Wextra)