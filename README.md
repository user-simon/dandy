<h1 align="center" width="100%">dandy</h1>

**dandy** is an easy to integrate, single-header linear algebra vector library making use of [expression templates](https://en.wikipedia.org/wiki/Expression_templates) for compile-time expression parsing -- as such, operations can be performed on vector expressions without having to allocate a new vector value for each intermediary operation.

* **lightweight**: a single header file with no non-standard external dependencies
* **easy to integrate**: all vector types are exposed in the `dd::types` namespace which contains no other symbols
* **supports conversions to and from arbitrary foreign types**: in order to more easily integrate with other libraries
* **expression templates**: all vector expressions are parsed at compile-time
* **STL integration**: overloads for various standard functions

## Requirements

dandy requires C++17 or newer and has been formally tested on MSVC and CLANG.

## Wiki

Read the documentation [here]()

## Building

Use the provided [CMakeLists.txt](CMakeLists.txt) file to generate a project. Alternatively, if you're on Windows, you can use the provided [make_vs.bat](make_vs.bat) script to create the project. 