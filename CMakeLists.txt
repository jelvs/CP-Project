cmake_minimum_required(VERSION 3.12)
project(project_parallel_patterns C)

set(CMAKE_C_STANDARD 11)

include_directories(src)

add_executable(project_parallel_patterns
        src/debug.c
        src/debug.h
        src/main.c
        src/patterns.c
        src/patterns.h
        src/unit.c
        src/unit.h)
