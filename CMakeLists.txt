cmake_minimum_required(VERSION 3.7...3.29)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(testlang VERSION 0.1
                 DESCRIPTION "compiler testing"
                 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(testlang src/main.cpp)

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(testlang ${llvm_libs})