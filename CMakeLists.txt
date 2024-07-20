cmake_minimum_required(VERSION 3.17)
project(mirrow
    VERSION 0.1.0
    LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

aux_source_directory(src mirrow_src)

file(GLOB_RECURSE mirrow_header_files "./include/*.hpp")

add_library(mirrow STATIC ${mirrow_src} ${mirrow_header_files})
target_include_directories(mirrow PUBLIC ./include)

target_compile_features(mirrow PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(mirrow PUBLIC /std:c++17)
endif()

option(MIRROW_BUILD_TEST "build test" OFF)

if (PROJECT_IS_TOP_LEVEL)
    set(MIRROW_BUILD_TEST ON CACHE BOOL "build test" FORCE)
endif()

if (MIRROW_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()