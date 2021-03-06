Basics
======

**dandy** is an easy to integrate, single-header linear algebra vector library making 
use of expression templates for compile-time expression parsing -- as such, 
operations can be performed on complex vector expressions without the need for
intermediary vector allocations.

.. _vector-aliases:

Vector types
------------

.. doxygentypedef:: vector

.. doxygennamespace:: types
    :members:

.. note:: Throughout this documentation, a "vector value" will refer to any of the types defined above

Construction
------------

- Is `DefaultConstructible <https://en.cppreference.com/w/cpp/named_req/DefaultConstructible>`_

  .. code-block:: C

    float3d v; // v contains 0, 0, 0

- Is constructable from individual component values:

  .. code-block:: C

    float3d v { 1.0, 2.0, 3.0 }; // v contains 1.0, 2.0, 3.0

- Is **explicitly** constructable from a single value:
  
  .. code-block:: C

    float3d v(1.0); // v contains 1.0, 1.0, 1.0

- Is `CopyConstructible <https://en.cppreference.com/w/cpp/named_req/CopyConstructible>`_ and `CopyAssignable <https://en.cppreference.com/w/cpp/named_req/CopyAssignable>`_:
  
  .. code-block:: C

    float3d a { 1.0, 2.0, 3.0 };
    float3d b = a; // b contains 1.0, 2.0, 3.0
    float3d c;
    c = a; // c contains 1.0, 2.0, 3.0

- Is constructable from a vector expression **of the same size**:
  
  .. code-block:: C
  
    float3d a { 1.1, 2.2, 3.3 };
    int3d b = a; // b contains 1, 2, 3
    int3d c;
    c = a + b; // c contains 2, 4, 6
    
    int2d d = a; // error

- **2D only**: Is constructable from an angle specified in radians
  
  .. code-block:: C

    float2d v = float2d::from_angle(0); // v contains 1, 0

Expression templates
--------------------

- Operations aren't evaluated until applied to a value:

  .. code-block:: C
  
    auto v = int3d{ 1, 2, 3 } + int3d{ 1, 2, 3 }; // v is of the intermediary type "operation"

- Operations can be evaluated fully **implicitly**:

  .. code-block:: C
    
    int3d v = int3d{ 1, 2, 3 } + int3d{ 1, 2, 3 }; // vector v contains 2, 4, 6

- Or **explicitly**:

  .. code-block:: C

    auto v = (int3d{ 1, 2, 3 } + int3d{ 1, 2, 3 }).evaluate(); // v is a vector and contains 2, 4, 6

    // alternate syntax:
    auto v = *(int3d{ 1, 2, 3 } + int3d{ 1, 2, 3 }); // v is a vector and contains 2, 4, 6

.. note::
    
    Throughout this documentation, ``vector expression`` refers to an instance of either a vector value
    (:cpp:struct:`impl::value`) or a vector operation (:cpp:struct:`impl::operation`) 

Operator overloads
------------------

.. note::
    - All operators require the argument vector expressions (if more than one) to be of the same size
    - All operators evaluate the expressions component-wise

- All binary arithmetic operators (``+``, ``-``, ``*``, ``/``, ``%``, ``&``, ``|``, ``^``, ``>>``, ``<<``), including their corresponding assignment operators,
  are overloaded for expressions of the pattern:

    .. code-block:: C

        [vector expression] [operator] [vector expression]
        [vector expression] [operator] [scalar]

- All binary arithmetic operators, exluding their corresponding assignment operators, are overloaded for expressions of the pattern:

  .. code-block:: C
  
      [scalar] [operator] [vector expression]

- The unary operators ``+``, ``-``, and ``~`` are overloaded for all vector expressions
- Is `EqualityComparable <https://en.cppreference.com/w/cpp/named_req/EqualityComparable>`_: Two vectors are equal if all components are equal

  .. code-block:: C
  
    float3d a { 1, 2, 3 };
    float3d b = a;
    float3d c { 4, 5, 6 };
    
    assert(a == b);
    assert(a != (b + c));
  
- Is `LessThanComparable <https://en.cppreference.com/w/cpp/named_req/LessThanComparable>`_: A vector ``a`` is "lesser than" a vector ``b`` if
  all components in ``a`` are lesser than the corresponding components in ``b``
  
  .. note:: ``!(a < b)`` does not imply that ``a >= b``

  .. code-block:: C

    int2d a { 1, 2 };
    int2d b { 2, 3 };

    assert(a < b);

    a.x = 2;

    assert_false(a < b); // since a.x >= b.x

- Is **explicitly** convertable to a bool:

  .. code-block:: C
  
    if (double2d{ 3, 4 })
        std::cout << "This will run";
    if (double2d::zero)
        std::cout << "This will not";
        
    // explicit cast required
    bool is_nonzero = (bool)double2d{ 1, 0 }; // is_nonzero is true

Indexing
--------

- Components in all vector expressions can be retrieved either with :cpp:func:`impl::expression_base::at`:

  .. code-block:: C

    float3d v { 1, 2, 3 };
    float x = v.at(0); // x is 1
    float y = v.at(1); // y is 2

  Or with the index operators:

  .. code-block:: C
  
    float3d v { 1, 2, 3 };
    float x = v[0]; // x is 1
    float y = v[1]; // y is 2


- :doc:`If a compatible compiler is used <components>`, components in a vector value can also be accessed by name:
  
  .. code-block:: C

    float3d v { 1, 2, 3 };
    float x = v.x; // x is 1
    float y = v.y; // y is 2

  Or with a range-for loop:

  .. code-block:: C

    int sum = 0;
  
    for (double v : double3d{ 1, 2, 3 })
        sum += v;

    // sum is 6
