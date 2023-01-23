cmake_minimum_required(VERSION 3.22)
project(tcl)

find_package(TCL)
find_package(Boost REQUIRED system json)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory(test)
add_subdirectory(example)