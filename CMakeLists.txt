set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_minimum_required(VERSION 3.10)
project(network-c C CXX)

file(GLOB_RECURSE LIB_SOURCES "src/*.c")
list(FILTER LIB_SOURCES EXCLUDE REGEX ".*/main\\.c$")

add_library(network-c-lib STATIC ${LIB_SOURCES})
target_include_directories(network-c-lib PUBLIC include)
target_compile_options(network-c-lib PRIVATE -Wall -Wextra)

add_executable(main src/main.c)
target_link_libraries(main PRIVATE network-c-lib)

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()