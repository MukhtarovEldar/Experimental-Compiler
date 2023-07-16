cmake_minimum_required(VERSION 3.14)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD_14) # CXX instead of C, 14 instead of 11

project(Experimental)

add_executable(
    func
    main.cpp # not src/main.c
)
