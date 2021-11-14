API
===

* :cpp:struct:`impl::expression_base` defines methods callable on all vector expressions
* :cpp:struct:`impl::expression` extends :cpp:struct:`impl::expression_base` by adding methods callable on vector expressions of specific sizes
* :cpp:struct:`impl::operation` defines methods callable on all vector operations
* :cpp:struct:`impl::value` defines methods callable on all vector values

Vector expressions
------------------

This class defines methods callable on any vector expression.

.. doxygenstruct:: impl::expression_base
    :members:
    
This class defines methods callable vector expression of specific sizes.

.. doxygengroup:: Expressions 
    :members:

Vector operations
-----------------

This class defines the methods and properties available to all vector operations.

.. doxygenstruct:: impl::operation
    :members:

Vector values
-------------

These classes define the methods and properties available to all vector values.

.. note::
    
    It is not intended for the user to interface with vector values directly, but rather through one of
    :ref:`the defined aliases <vector-aliases>`
    
.. doxygenstruct:: impl::value
    :members:

.. doxygengroup:: ValueData
    :members:

STL integration
---------------

.. doxygenfunction:: std::operator<<
.. doxygenstruct:: std::hash< dd::vector< Scalar, Size > >
.. doxygengroup:: Iterators
