# Filinator

**Filinator** is a minimal GNU99 C program that recursively encodes and decodes file and directory names. It supports both in-place transformations and an output mode that copies files with "flattened" encoded names. Filinator is designed to be simple, efficient, and easily optimizable using Clang and LLVM.

## Features

- **In-Place Encoding:**  
  Converts spaces in directory names to a special encoded character (represented as `SEC_CHAR`, i.e., `§`), and for files, it transforms the absolute file path by replacing `/` with `@`, spaces with `_`, and `SEC_CHAR` with a space. The files and directories are then renamed accordingly.

- **Output Mode Encoding:**  
  When using the `-output` flag, instead of renaming files in place, Filinator copies each file from the input tree into a separate output directory with its encoded name, effectively producing a flattened file structure.

- **Decoding:**  
  Reverses the encoding transformations, restoring the original file and directory names in place.

## Compilation

Filinator is written using GNU99 standards. To compile with Clang and benefit from LLVM optimizations, run:

```bash
clang -std=gnu99 -O3 -march=native -flto -o filinator filinator.c
