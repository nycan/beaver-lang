# Beaver language
A language built with LLVM for CS20. Still in early stages of development. \\
Originally based on the LLVM Kaleidoscope tutorial

## Installation instructions
1. Make sure CMake and Clang++ are installed.
2. Download the contents of the repository
3. Run ``mkdir build && cd build``
4. Run ``cmake -G "Ninja" ..`` to create the build files
5. Run ``cmake --build .`` to compile
6. The executable is ``beaver``