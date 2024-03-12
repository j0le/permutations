cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(permutationen CXX)

add_executable(permutationen
    permutationen.cpp
    )

target_compile_features(permutationen PUBLIC cxx_std_23)
