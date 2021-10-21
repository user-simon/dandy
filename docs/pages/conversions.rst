Custom Type Conversions
===========================

.. doxygenstruct:: converter

Adding a conversion
-------------------

Start by specializing the :cpp:class:`converter` struct for the desired dandy type and desired
foreign type *in that order*. Then add two static methods named ``convert``, each taking as
parameters one of the types to convert *from* and a reference to the other type to convert *to*.

Examples
--------

Specialization to convert between ``int3d`` and ``std::array<int, 3>``:

.. code-block:: C

    template<>
    struct dd::converter<int3d, std::array<int, 3>>
    {
        static void convert(const int3d& from, std::array<int, 3>& to)
        {
            to[0] = from.x;
            to[1] = from.y;
            to[2] = from.z;
        }

        static void convert(const std::array<int, 3>& from, int3d& to)
        {
            to.x = from[0];
            to.y = from[1];
            to.z = from[2];
        }
    };

Specialization to convert between ``dd::vector<T, 2>`` and a user defined vector type ``some_vector<T>``:

.. code-block:: C

    template<class T>
    struct some_vector
    {
        T x, y;
    };

    template<class T>
    struct dd::converter<dd::vector<T, 2>, some_vector<T>>
    {
        using dandy = dd::vector<T, 2>;
        using foreign = some_vector<T>;

        static void convert(const dandy& from, foreign& to)
        {
            to.x = from.x;
            to.y = from.y;
        }

        static void convert(const foreign& from, dandy& to)
        {
            to.x = from.x;
            to.y = from.y;
        }
    };
