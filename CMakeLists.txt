cmake_minimum_required(VERSION 3.7...3.29)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(testlang VERSION 0.1
                 DESCRIPTION "compiler testing"
                 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES main.cpp)
add_executable(testlang ${SOURCE_FILES})
