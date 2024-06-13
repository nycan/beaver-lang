cmake_minimum_required(VERSION 3.7...3.29)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
set(LLVM_DIR /usr/local/Cellar/llvm/18.1.4/lib/cmake/llvm)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

project(beaverlang VERSION 0.1
                 DESCRIPTION "beaver"
                 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(
  llvm_libs
  ${LLVM_TARGETS_TO_BUILD}
  core
  native
  mcjit
)

add_executable(beaver
  src/main.cpp
  src/lexer.cpp
  src/parser.cpp
  src/syntaxtree.cpp
  src/operations.cpp
  src/generator.cpp
  # src/jit.cpp
)
target_link_libraries(beaver ${llvm_libs} ${targets})