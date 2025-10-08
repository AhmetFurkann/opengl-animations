cmake_minimum_required(VERSION 3.16)

project(Animations)
set(CMAKE_CXX_FLAGS "-fPIC")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O2 -fsanitize=address,undefined")
set(CMAKE_LINKER_FLAGS_RELEASE "${CMAKE_LINKER_FLAGS_RELEASE} -fsanitize=address,undefined")

add_subdirectory(shared)
add_subdirectory(TrianglePoints)
add_subdirectory(TriangleLines)