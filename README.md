<h1 align="center" width="100%">dandy</h1>

**dandy** is an easy to integrate, single-header linear algebra vector library making 
use of [expression templates](https://en.wikipedia.org/wiki/Expression_templates) for compile-time expression parsing -- as such, 
operations can be performed on complex vector expressions without the need for
intermediary vector allocations.

* **lightweight**: a single header file with no non-standard external dependencies
* **easy to integrate**: all vector types can be accessed
either under the `dd` namespace or in the global namespace by adding `using namespace dd::types`
to your source (note that this does not leak any additional symbols). [See all types](https://user-simon.github.io/dandy/pages/basics.html#vector-types)
* **supports conversions to and from arbitrary foreign types**: in order to more easily integrate with other libraries. [Read more](https://user-simon.github.io/dandy/pages/conversions.html)
* **expression templates**: all vector expressions are parsed at compile-time. [Read more](https://user-simon.github.io/dandy/pages/basics.html#expression-templates)
* **STL integration**: overloads for `std::ostream operator<<`, `std::hash::operator()` and the inclusion of methods `begin()` and `end()` allows the vector to be used in a multitude of standard containers and algorithms

## Requirements

dandy requires C++17 or newer and has been formally tested on MSVC and CLANG.

## Wiki

Read the documentation [here](https://user-simon.github.io/dandy/).

## Building

Use the provided [CMakeLists.txt](CMakeLists.txt) file to generate a project. Alternatively, if you're on Windows, you can use the provided [make_vs.bat](make_vs.bat) script to create the project under `./build`
