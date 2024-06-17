# Beaver language
A compiler for my own (still very minimal) language built as an SDS for CS20. It includes functions, mutable variables and complex expressions. \
The most difficult part was creating the code generation for conditional statements - I probably spent a few good weeks debugging it. \
I am proud of the fact that I did this project using much better programming practices than I have been previously. However, there is definitely still room for improvement.

Received help from:
- StackOverflow
- LLVM Kaleidoscope tutorial
- LLVM docs
- C++ docs
- Some random blogs

## Installation instructions
Installation is currently only supported on MacOS and Linux. Installation may work on Windows, but it is recommended to use WSL.
1. Clone the repository and ``cd`` into the ``scripts`` directory.
2. Run ``linux_deps.sh`` or ``mac_deps.sh`` depending upon your OS. You may need to ``chmod +x`` the file.
3. Run ``cd ..; mkdir build; cd build; cmake -G "Ninja" ..; cmake --build .`` to get everything set up.
4. The executable is ``beaver``.