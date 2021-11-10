Disabling Component Names
=========================

Component names might incur a slight performance penalty due to increased complexity when constructing
a vector value since each named component variable has to bind to their respective indices in the data array.
As such, if one should not need them and is content to access components only by index, there is an
option to disable them.

.. doxygendefine:: DD_DISABLE_NAMES

Add the define to your source before the header is included and vector values will no longer be compiled with
named component variables.
