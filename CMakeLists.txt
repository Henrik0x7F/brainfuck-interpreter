cmake_minimum_required(VERSION 3.11)
project(bf6)

set(CMAKE_CXX_STANDARD 17)

add_executable(bf
    src/main.cpp
    src/bf_interpreter.cpp
    src/bf_interpreter.hpp
    )
