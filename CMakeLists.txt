cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(permutationen CXX)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics -fansi-escape-codes)
endif()

add_executable(permutationen
    permutationen.cpp
    )

target_compile_features(permutationen PUBLIC cxx_std_23)

add_executable(experiment
    experiment.cpp
    )

target_compile_features(experiment PUBLIC cxx_std_23)
