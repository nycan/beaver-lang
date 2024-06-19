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

## Usage instructions
Here is a sample program:
```
# comments are as in python

# declare functions similar to rust
fn doSmthn(x,y) {
    # no brackets needed!
    if x>y {
        # no need for 3 extra letters
        ret 1;
    } elif x==y { # uses "elif" like python
        ret 2;
    } else {
        ret 3;
    };

    # C-style for loops and while loops are also supported
}

fn mutable() {
    # there is support for mutable variables
    let a = 3;
    # complex expressions and assignment operators are supported
    a += 3*(doSmthn(a,2)+1)-a/32;
    ret 3*a-2;
}

# entry point function is main, it takes no arguments
fn main() {
    # just return the thing you want to print
    # I have not yet added printing as it is OS-specific
    # the POSIX interface is relatively straightforward but converting a double to a string isn't.
    ret doSmthn(3,4);
}

# note: all types are doubles currently. I was planning on adding types but didn't end up having time.
# error handling is not great but there is some
# There are probably still some bugs

# I hope to keep working on this over the summer and add the features I missed adding because I spent too long debugging.
```