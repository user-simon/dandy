Disabling Prefabricated Vectors
===============================

There are a number of prefabricated vector values available for convenience such as :cpp:var:`detail::value::zero` and :cpp:var:`detail::value::identity`.
All of these will be allocated per vector template instantiation. As such, if memory overhead is an issue and if they won't be needed, there is an option to disable
them.

.. doxygendefine:: DD_DISABLE_PREFABS

Add the define to your source before the header is included and prefabricated vector values will no longer be compiled.
