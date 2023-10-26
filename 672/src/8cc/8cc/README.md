# 8CC Compiler

## Overview

8cc is a C programming language compiler designed to support all C11 language features while maintaining minimal code complexity. The compiler is capable of compiling itself, serving as both an implementation of the C language and an example of what the compiler can handle.

The source code for 8cc is written with a focus on conciseness and readability. This makes it an excellent resource for learning about various compiler techniques. You can explore the lexer, preprocessor, and parser to understand how C source code is processed at each stage.

Please note that 8cc is not an optimizing compiler as of now. The generated code is typically two or more times slower than GCC currently. There are plans to implement optimization features in the future.

## How to build the compiler executable?
Simply run the following command:
```sh
make
```

## Prerequests (required to be installed before u can build 8cc)
`gcc`


# ToDo List

## 1. Proper Deallocation of Memory
It is essential to implement proper memory deallocation for instances of 
functions such as `underscore()` when they are no longer in use.
- **underscore() function**
  - **parse.c**:**line 213**: `lvarNode.varname = underscore(name);`
  - **parse.c**:**line 229**: `gvarNode.glabel = underscore(name);`
  - **parse.c**:**line 298**: `Node tmpNode = { .kind = AST_FUNCDESG, ty, .fname = underscore(fname) };`
  - **parse.c**:**line 319**: `funcNode.fname = underscore(fname);`
  - **parse.c**:**line 389**: `structRefNode.field = underscore(name);`
  - **parse.c**:**line 418**: `labelNode.goto_label = underscore(label);`
  - **parse.c**:**line 434**: `labeladdrNode.goto_label = underscore(label);`
