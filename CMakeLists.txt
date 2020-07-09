cmake_minimum_required(VERSION 3.10)
project(RISC-V)

set(CMAKE_CXX_STANDARD 14)

add_executable(code main.cpp ClassInstruction.hpp)
