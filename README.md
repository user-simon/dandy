<h1 align="center" width="100%">dandy</h1>

**dandy** is an easy to integrate, single-header linear algebra vector library making use of [expression templates](https://en.wikipedia.org/wiki/Expression_templates) for compile-time expression parsing -- as such, operations can be performed on vector expressions without having to allocate a new vector value for each intermediary operation.

**dandy** requires C++17 or newer and has been tested on MSVC and CLANG.

---

* **lightweight**: a single header file with no non-standard external dependencies
* **easy to integrate**: all vector types are exposed in the `dd::types` namespace which contains no other symbols
* **supports conversions to and from foreign vector types**: in order to more easily integrate with other libraries. [Read more](#user-defined-conversions)
* **expression templates**: all vector expressions are parsed at compile-time
* **STL integration**: overloads for various standard functions

---

## Types

The generic vector template is defined as `vector<[scalar type], [size]>` in namespace `dd`. For convenience, vectors sizes 2-4 have aliases of the form `[scalar type][size]d` in the `dd::types` namespace:

| vector<T, 2> | vector<T, 3> | vector<T, 4> |
| --: | --: | --: |
| binary2d | binary3d | binary4d |
| char2d | char3d | char4d |
| uchar2d | uchar3d | uchar4d |
| int2d | int3d | int42d |
| uint2d | uint3d | uint4d |
| float2d | float3d | float4d |
| double2d | double3d | double4d |

## Basic usage

Simply include the header file and optionally use namespace the `dd::types`.

## Operators
NOTE: all operators require the argument vector expressions (if more than one) to be of the same size
* the binary operators `+`, `-`, `*`, `/`, `%`, `&`, `|`, `^`, `>>`, `<<`, and their respective assignment counterparts are all overloaded for expressions of the pattern:
  ```cpp
  [vector expression] [operation] [vector expression]
  [vector expression] [operation] [scalar]
  ```
* all binary operators **except** `>>`, `<<`, and the assignment operators are overloaded for expressions of the pattern:
  ```cpp
  [scalar] [operation] [vector expression]
  ```
* the unary operators `+`, `-`, and `~` are overloaded for all vector expressions and are evaluated on a component-basis
* is [`EqualityComparable`](https://en.cppreference.com/w/cpp/named_req/EqualityComparable):
  ```cpp
  float3d a { 1, 2, 3 };
  float3d b = a;
  float3d c { 4, 5, 6 };
  
  assert(a == b);
  assert(a != (b + c));
  ```

## Construction

* is [`DefaultConstructible`](https://en.cppreference.com/w/cpp/named_req/DefaultConstructible):
  ```cpp
  float3d v; // v contains 0, 0, 0
  ```
* is constructable from individual component values:
  ```cpp
  float3d v { 1.0, 2.0, 3.0 }; // v contains 1.0, 2.0, 3.0
  ```
* is **explicitly** constructable from a single value:
  ```cpp
  float3d v(1.0); // v contains 1.0, 1.0, 1.0
  ```
* is [`CopyConstructible`](https://en.cppreference.com/w/cpp/named_req/CopyConstructible) and [`CopyAssignable`](https://en.cppreference.com/w/cpp/named_req/CopyAssignable):
  ```cpp
  float3d a { 1.0, 2.0, 3.0 };
  float3d b = a; // b contains 1.0, 2.0, 3.0
  float3d c;
  c = a; // c contains 1.0, 2.0, 3.0
  ```
* is constructable from a vector expression **of the same size**:
  ```cpp
  float3d a { 1.1, 2.2, 3.3 };
  int3d b = a; // b contains 1, 2, 3
  int3d c;
  c = a + b; // c contains 2, 4, 6
  
  int2d d = a; // error
  ```
* **[2D only]** is constructable from an angle specified in radians:
  ```cpp
  float2d v = float2d::from_angle(0); // v contains 1, 0
  ```

## Expression templates

* operations aren't evaluated until applied to a value:
  ```cpp
  auto v = int3d(1, 2, 3) + int3d(1, 2, 3); // v is of the intermediary type "operation"
  ```
* operations can be evaluated fully **implicitly**:
  ```cpp
  int3d v = int3d(1, 2, 3) + int3d(1, 2, 3); // vector v contains 2, 4, 6
  ```
* or **explicitly**:
  ```cpp
  auto v = (int3d(1, 2, 3) + int3d(1, 2, 3)).evaluate(); // v is a vector and contains 2, 4, 6

  // alternate syntax:
  auto v = *(int3d(1, 2, 3) + int3d(1, 2, 3)); // v is a vector and contains 2, 4, 6
  ```
* or evaluated component-wise through the index operators (NOTE: only the specified index is evaluated):
  ```cpp
  int x = (int3d(1, 2, 3) + int3d(1, 2, 3))[0];
  ```

## Indexing
  
* [by default](#options), components in a vector value can be accessed either by index or by name:
  ```cpp
  float3d v { 1, 2, 3 };
  float x = v[0]; // x is 1
  float y = v.y; // y is 2
  ```
* components in a vector operation can only be accessed by index:
  ```cpp
  int x = (int3d(1, 2, 3) + int3d(1, 2, 3))[0]; // x is 2
  ```

## Functions

NOTE: here, `scalar_t` refers to the scalar type of the vector expression on which the function is called (`*this`).
| Function | Description |
| --- | --- |
| **Math functions:** | 
| `sum` -> `scalar_t` | calculates the sum of the vector |
| `product` -> `scalar_t` | calculates the product of the vector |
| `nonzero` -> `bool` | true iff all components are 0 |
| `dot(vector)` -> `scalar_t` | calculates the dot product with `vector` |
| `length2` -> `scalar_t` | calculates the length **squared** |
| `length` -> `double` | calculates the length. Equivalent to `std::sqrt(length2())` |
| `distance2(vector)` -> `scalar_t` | calculates the Euclidian distance **squared** to `vector` |
| `distance(vector)` -> `double` | calculates the Euclidian distance to `vector`. Equivalent to `std::sqrt(distance2(vector))` |
| `normalize` -> `vector<double>` | normalizes the vector. Equivalent to dividing by it's length |
| `set_length(length)` -> `vector<double>` | sets the length of the vector. Equivalent to `normalize() * length` |
| `delta_angle(vector)` -> `double` | calculates the angle to `vector` |
| `apply(fn)` -> `operation` | creates an operation which applies the function `fn` to each component |
| `abs` -> `operation` | calculates the absolute values of each component |
| `round` -> `operation` | calculates the rounded values of each component |
| `floor` -> `operation` | calculates the floored values of each component |
| `ceil` -> `operation` | calculates the ceiling values of each component |
| `scalar_cast<T>` -> `operation` | casts each component to T |
| `angle` -> `double` | **[2D only]** calculates the angle represented the 2D vector |
| `from_angle(angle)` -> `vector` | **[2D only]** **[static]** creates the vector representation of the angle on the unit circle |
| `cross(vector)` -> `vector` | **[3D only]** calculates the cross product with `vector` |
| **Utility functions:** |
| `evaulate(vector)` -> `vector` | **[Value only]** evaulates/copies components from `vector` |
| `to_string(optional: name)` -> `std::string` | **[Value only]** creates string representation of the vector |
| `evaluate` -> `vector` | **[Operation only]** forces evaluation |
| `to_string(optional: name)` -> `std::string` | **[Operation only]** forces evaluation and creates string representation of resulting vector |

### STL integration
 
| Function(s) | Description |
| --- | --- |
| `std::ostream operator<<` | overload to serialize vector data |
| `std::hash::operator()` | specialization to allow use as keys in std::unordered_* containers |
| `std::begin` and `std::end` | overloads to enable range-for loops |

## Options

* `DD_NO_NAMES`: define to disable component names in vector values if they're not needed. Can be done for performance reasons 
  
## User defined conversions

To enable **implicit** conversions between **dandy** and an arbitrary foreign vector type, start by specializing the `dd::converter` class to desired **dandy** vector and desired foreign vector, **in that order**. Then implement two static methods, both named `from`, in the class: each taking one of the vectors as parameter and returning the other.

See below for examples:

#### Simple conversion:
```cpp
struct some_vector
{
  int x, y;
}

template<>
struct dd::converter<int2d, some_vector>
{
    // some_vector -> dandy
    static int2d from(const some_vector& v)
    {
        return int2d{ v.x, v.y };
    }
    
    // dandy -> some_vector
    static some_vector from(const int2d& v)
    {
        return some_vector{ v.x, v.y };
    }
}
```

#### Scalar type agnostic vector conversion:
```cpp
template<class S>
struct some_vector
{
    S x, y;
};

template<class S1, class S2>
struct dd::converter<dd::vector<S1, 2>, some_vector<S2>>
{
    // some_vector -> dandy
    static dd::vector<S1, 2> from(const some_vector<S2>& v)
    {
        return dd::vector<S1, 2>{ v.x, v.y }; // the dd::vector constructor handles scalar conversions
    }

    // dandy -> some_vector
    static some_vector<S2> from(const dd::vector<S1, 2>& v)
    {
        return some_vector<S2>{ (S2)v.x, (S2)v.y }; // the some_vector does not
    }
};
```
