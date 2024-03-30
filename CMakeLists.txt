cmake_minimum_required(VERSION 3.15)

option(PERMUTATIONS_IGNORE_ENVVAR_CXXFLAGS "Ignore the environment variables CFLAGS and CXXFLAGS" OFF)

if(PERMUTATIONS_IGNORE_ENVVAR_CXXFLAGS)
    set(ENV{CXXFLAGS} "")
    set(ENV{CFLAGS} "")
endif()

option(PERMUTATIONS_CREATE_PDB "Create a .pdb file with debug information" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(permutationen CXX)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND
        ("${CMAKE_GENERATOR}" STREQUAL "Ninja" OR "${CMAKE_GENERATOR}" STREQUAL "Ninja Multi-Config")
  )
    add_compile_options(-fcolor-diagnostics -fansi-escape-codes -fdiagnostics-absolute-paths)
endif()

add_executable(permutationen
    permutationen.cpp
    )

if(PERMUTATIONS_CREATE_PDB)
    target_compile_options(permutationen PRIVATE
        -gcodeview
    )

    target_link_options(permutationen PRIVATE
        "LINKER:--pdb="
    )
endif()

target_compile_features(permutationen PUBLIC cxx_std_23)

add_executable(experiment
    experiment.cpp
    )

target_compile_features(experiment PUBLIC cxx_std_23)
