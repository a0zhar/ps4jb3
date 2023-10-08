# 8CC Compiler

## Overview

8cc is a C programming language compiler designed to support all C11 language features while maintaining minimal code complexity. 
The compiler is capable of compiling itself, serving as both an implementation of the C language and an example of what the compiler can handle.

The source code for 8cc is written with a focus on conciseness and readability. 
This makes it an excellent resource for learning about various compiler techniques. 
You can explore the lexer, preprocessor, and parser to understand how C source code is processed at each stage.

Please note that 8cc is not an optimizing compiler. The generated code is typically two or more times slower than GCC. 
There are plans to implement optimization features in the future.

## Build

To build 8cc, simply run the following command:
```sh
make
```
