cmake_minimum_required(VERSION 3.10)
project(eq VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/contrib)

include(cmake/cpm.cmake)

CPMAddPackage(
    NAME JUCE
    GITHUB_REPOSITORY juce-framework/JUCE
    GIT_TAG 8.0.7
    VERSION 8.0.7 #if in repo tag is like v8.0.7 the GIT_TAG is not nesessarely
    SOURCE_DIR ${LIB_DIR}/juce
)

CPMAddPackage(
    NAME GOOGLETEST
    GITHUB_REPOSITORY google/googletest
    VERSION 1.16.0
    SOURCE_DIR ${LIB_DIR}/googletest
    OPTIONS 
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)

enable_testing()

if(GCC)
    add_compile_options(
        -Wall
        -Wextra
        -pedantic
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -pedantic
    )
endif()


add_subdirectory(my_plagin)
add_subdirectory(play_audio)
add_subdirectory(Audio_Plagin_Host)
add_subdirectory(test)
