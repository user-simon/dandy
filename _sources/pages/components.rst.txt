Named Components
================

Named components are achieved through anonymous structs of component variables inside
an anonymous union also containing the data array, such that the component variables
and the data array share the same memory location.

Anoymous structs are technically non-ISO C++, meaning there might be compilers that do
not support it. If one of these compilers should be used with this library, the
specializations which provide the component variables should get SFINAE'd out and the
default case of only having a data array is used.

According to my testing, GCC, CLANG, and MSVC should all support anonymous structs.
