cmake_minimum_required(VERSION 3.16)
project(os4)

set(CMAKE_CXX_STANDARD 14)

include_directories("EX4 Resources")

add_executable(os4
        "EX4 Resources/MemoryConstants.h"
        "EX4 Resources/PhysicalMemory.cpp"
        "EX4 Resources/PhysicalMemory.h"
        "EX4 Resources/SimpleTest.cpp"
        "EX4 Resources/VirtualMemory.h")
