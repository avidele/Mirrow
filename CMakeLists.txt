cmake_minimum_required(VERSION 3.17)
project(mirror
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE mirror_src "./src/*.cpp")
file(GLOB_RECURSE mirror_header_files "./include/*.hpp")

if(NOT mirror_src)
    file(WRITE ${CMAKE_BINARY_DIR}/dummy.cpp "// Dummy source file\n")
    list(APPEND mirror_src ${CMAKE_BINARY_DIR}/dummy.cpp)
endif()

add_library(mirror STATIC ${mirror_src} ${mirror_header_files})
target_include_directories(mirror PUBLIC ./include)

target_compile_features(mirror PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(mirror PUBLIC /std:c++20)
endif()

option(mirror_BUILD_TEST "build test" OFF)

if (PROJECT_IS_TOP_LEVEL)
    set(mirror_BUILD_TEST ON CACHE BOOL "build test" FORCE)
endif()

if (mirror_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()
