cmake_minimum_required(VERSION 3.14)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD_14) # CXX instead of C, 14 instead of 11

project(Experimental-Compiler)

add_executable(
    func
    src/main.cpp
    src/error.cpp
    src/environment.cpp
    src/file_io.cpp
    src/parser.cpp
)

target_include_directories(
  func
  PUBLIC src/
)