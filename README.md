# Beaver language
A language built with LLVM for CS20. Still in early stages of development. \
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