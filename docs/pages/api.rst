API
===

Vector expressions
------------------

These classes define the methods and properties available to all vector expressions.

.. doxygenstruct:: detail::expression_base
    :members:
    
.. doxygengroup:: Expressions 
    :members:

Vector operations
-----------------

This class defines the methods and properties available to all vector operations.

.. doxygenstruct:: detail::operation
    :members:

Vector values
-----------------

These classes define the methods and properties available to all vector values.

.. note::
    
    It is not intended for the user to interface with vector values directly, but rather through one of
    :ref:`the defined aliases <vector-aliases>`
    
.. doxygenstruct:: detail::value
    :members:

.. doxygengroup:: ComponentNames
    :members:

STL integration
---------------

.. doxygenfunction:: std::operator<<
.. doxygenstruct:: std::hash< dd::vector< Scalar, Size > >
.. doxygengroup:: Iterators
