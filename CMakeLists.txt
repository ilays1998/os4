cmake_minimum_required(VERSION 3.16)
project(os4)

set(CMAKE_CXX_STANDARD 14)

include_directories("EX4_Resources")

add_executable(os4
        "MemoryConstants.h"
        "PhysicalMemory.cpp"
        "PhysicalMemory.h"
        "SimpleTest.cpp"
        "VirtualMemory.h")
