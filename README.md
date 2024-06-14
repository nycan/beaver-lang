# Beaver language
A compiler for my own (still very minimal) language built as an SDS for CS20. It includes functions, mutable variables and complex expressions. \

Received help from:
- StackOverflow
- LLVM Kaleidoscope tutorial
- LLVM docs
- C++ docs
- Some random blogs

## Installation instructions
Installation is currently only supported on MacOS and Linux.
1. Clone the repository and ``cd`` into the ``scripts`` directory.
2. Run ``linux_deps.sh`` or ``mac_deps.sh`` depending upon your OS. You may need to ``chmod +x`` the file.
3. Run ``cd ..; mkdir build; cd build; cmake -G "Ninja" ..; cmake --build .`` to get everything set up.
4. The executable is ``beaver``.