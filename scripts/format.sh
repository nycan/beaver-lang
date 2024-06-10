#!/bin/bash
for file in ../src/*; do
    clang-format -i "$file"
done