#pragma once
#include <string>
#include <string_view>
#include <ostream>
#include <type_traits>
#include <tuple>      // std::tuple, std::apply
#include <functional> // std::hash
#include <algorithm>  // std::max
#include <cstdint>    // fixed width integer types
#include <cmath>      // std::sqrt, std::sin, std::acos, std::atan2

#define _DD_NAMESPACE_OPEN namespace dd {
#define _DD_NAMESPACE_CLOSE }
#define _DD_OPERATION_T auto
#define _DD_DEFINE_BINARY_OPERATOR(op)                                                         \
    template<class L, class R, traits::require<traits::is_valid_operation_v<L, R, false>> = 1> \
    inline constexpr _DD_OPERATION_T operator op (const L& l, const R& r)                      \
    {                                                                                          \
        return impl::operation                                                                 \
        {                                                                                      \
            [](const auto& a, const auto& b) { return a op b; },                               \
            l, r                                                                               \
        };                                                                                     \
                                                                                               \
    }                                                                                          \
    template<class L, class R, traits::require<traits::is_valid_operation_v<L, R, true>> = 1>  \
    inline constexpr _DD_OPERATION_T operator op##=(L& l, const R& r)                          \
    {                                                                                          \
        return l = l op r;                                                                     \
    }

#define _DD_DEFINE_UNARY_OPERATOR(op)                                        \
    template<class Expr, traits::require<traits::is_expression_v<Expr>> = 1> \
    inline constexpr _DD_OPERATION_T operator op (const Expr& expr)          \
    {                                                                        \
        return impl::operation                                               \
        {                                                                    \
            [](const auto& v) { return op(v); },                             \
            expr                                                             \
        };                                                                   \
    }


_DD_NAMESPACE_OPEN


namespace impl
{
    template<class>
    struct expression_base;

    template<class, size_t>
    struct expression;

    template<class, class...>
    struct operation;

    template<class, size_t>
    struct value;
}

/// @brief Interface to convert between a dandy vector type and an arbitrary foreign type
/// @param Dandy A dandy vector type
/// @param Foreign A foreign type
/// 
/// @note
///     - The Foreign type has to be default constructable
///     - There is currently no mechanism to allow only one way of the conversion
template<class Dandy, class Foreign>
struct converter;

namespace traits
{
    /// @brief Helper metafunction to return a template type unchanged
    template<class T>
    struct type_identity
    {
        using type = T;
    };

    /// @brief Wrapper for `std::enable_if_t` with second parameter defaulted to `bool`
    /// @details Renamed for brevity and to be closer to C++20 syntax
    template<bool Condition, class Ty = bool>
    using require = std::enable_if_t<Condition, Ty>;

    /// @struct is_value
    /// @brief Determines if a type is a vector value type
    template<class T>
    struct _is_value : std::false_type {};

    template<class Scalar, size_t Size>
    struct _is_value<impl::value<Scalar, Size>> : std::true_type {};
    
    template<class T>
    struct is_value : _is_value<T> {};

    template<class T>
    inline constexpr bool is_value_v = is_value<T>::value;
    
    /// @struct is_operation
    /// @brief Determines if a type is a vector operation
    template<class T>
    struct _is_operation : std::false_type {};

    template<class Op_fn, class... Operands>
    struct _is_operation<impl::operation<Op_fn, Operands...>> : std::true_type {};

    template<class T>
    struct is_operation : _is_operation<T> {};

    template<class T>
    inline constexpr bool is_operation_v = is_operation<T>::value;

    /// @struct is_expression
    /// @brief Determines if a type is a vector expression
    /// @details Returns true iff a type is a value or an operation
    template<class T>
    struct _is_expression : std::disjunction<is_value<T>, is_operation<T>> {};

    template<class T>
    struct is_expression : _is_expression<T> {};

    template<class T>
    inline constexpr bool is_expression_v = is_expression<T>::value;

    /// @struct scalar
    /// @brief Gets the scalar type of a vector expression
    /// @details 
    ///  - *Values*: returns scalar type of the value
    ///  - *Operations*: returns the resulting scalar type of the operation
    template<class Scalar>
    struct _scalar_impl : type_identity<Scalar> {};

    template<class Op_fn, class... Operands>
    struct _scalar_impl<impl::operation<Op_fn, Operands...>> : std::invoke_result<Op_fn, typename _scalar_impl<Operands>::type...> {};

    template<class Scalar, size_t Size>
    struct _scalar_impl<impl::value<Scalar, Size>> : type_identity<Scalar> {};

    template<class Expr, require<is_expression_v<Expr>> = 1>
    struct _scalar : _scalar_impl<Expr> {};

    template<class Expr>
    struct scalar : _scalar<Expr> {};

    template<class Expr>
    using scalar_t = typename scalar<Expr>::type;

    /// @struct size
    /// @brief Gets the size of a vector expression
    /// @details 
    ///  - *Values*: returns size of the value
    ///  - *Operations*: returns the greatest size of its operands
    template<class>
    struct _size_impl : std::integral_constant<size_t, 1> {};

    template<class Op_fn, class... Operands>
    struct _size_impl<impl::operation<Op_fn, Operands...>> : std::integral_constant<size_t, std::max({ _size_impl<Operands>::value... })> {};

    template<class Scalar, size_t Size>
    struct _size_impl<impl::value<Scalar, Size>> : std::integral_constant<size_t, Size> {};

    template<class Expr, require<is_expression_v<Expr>> = 1>
    struct _size : _size_impl<Expr> {};

    template<class Expr>
    struct size : _size<Expr> {};

    template<class Expr>
    inline constexpr size_t size_v = size<Expr>::value;
    
    /// @struct vector
    /// @brief Gets the resulting vector type of a vector expression
    /// @detail Behaviour undefined for non-expression types
    template<class Expr, require<is_expression_v<Expr>> = 1>
    struct _vector : type_identity<impl::value<scalar_t<Expr>, size_v<Expr>>> {};

    template<class Expr>
    struct vector : _vector<Expr> {};

    template<class Expr>
    using vector_t = typename vector<Expr>::type;

    /// @struct is_same_size
    /// @brief Determines if two types are vector expressions of the same size
    template<class T, class U, bool AreExpressions>
    struct _is_same_size : std::false_type {};

    template<class T, class U>
    struct _is_same_size<T, U, true> : std::bool_constant<size_v<T> == size_v<U>> {};

    template<class T, class U>
    struct is_same_size : _is_same_size<T, U, is_expression_v<T> && is_expression_v<U>> {};

    template<class T, class U>
    inline constexpr bool is_same_size_v = is_same_size<T, U>::value;

    /// @struct is_valid_operation
    /// @brief Determines if two types form a valid vector operation
    /// @param Strict_ordering A value of true forbids a scalar type from appearing first in
    ///                        the operation
    template<class L, class R, bool Strict_ordering>
    struct _is_valid_operation
        : std::bool_constant<(is_same_size_v<L, R>)                                               ||
                             (is_expression_v<L> && std::is_arithmetic_v<R>)                      ||
                             (std::is_arithmetic_v<L> && is_expression_v<R> && !Strict_ordering)> {};
    
    template<class L, class R, bool Strict_ordering>
    struct is_valid_operation : _is_valid_operation<L, R, Strict_ordering> {};

    template<class L, class R, bool Strict_ordering>
    inline constexpr bool is_valid_operation_v = is_valid_operation<L, R, Strict_ordering>::value;

    /// @struct has_converter
    /// @brief Determines if there is a converter specilization defined between two types
    template<class, class, class = void>
    struct _has_converter : std::false_type {};

    template<class T, class U>
    struct _has_converter<T, U, std::void_t<decltype(
        converter<T, U>::convert(std::declval<T>(), std::declval<U&>()), // has T -> U
        converter<T, U>::convert(std::declval<U>(), std::declval<T&>())  // has U -> T
    )>> : std::true_type {};

    template<class T, class U>
    struct has_converter : _has_converter<T, U> {};

    template<class T, class U>
    inline constexpr bool has_converter_v = has_converter<T, U>::value;
}

namespace impl
{
    _DD_DEFINE_BINARY_OPERATOR(+);
    _DD_DEFINE_BINARY_OPERATOR(-);
    _DD_DEFINE_BINARY_OPERATOR(*);
    _DD_DEFINE_BINARY_OPERATOR(/);
    _DD_DEFINE_BINARY_OPERATOR(%);
    _DD_DEFINE_BINARY_OPERATOR(&);
    _DD_DEFINE_BINARY_OPERATOR(|);
    _DD_DEFINE_BINARY_OPERATOR(^);
    _DD_DEFINE_BINARY_OPERATOR(>>);
    _DD_DEFINE_BINARY_OPERATOR(<<);

    _DD_DEFINE_UNARY_OPERATOR(+);
    _DD_DEFINE_UNARY_OPERATOR(-);
    _DD_DEFINE_UNARY_OPERATOR(~);

    template<class Expr1, class Expr2, traits::require<traits::is_same_size_v<Expr1, Expr2>> = 1>
    inline constexpr bool operator==(const Expr1& expr1, const Expr2& expr2) noexcept
    {
        for (size_t i = 0; i < Expr1::size; i++)
        {
            if (expr1[i] != expr2[i])
                return false;
        }
        return true;
    }

    template<class Expr1, class Expr2, traits::require<traits::is_same_size_v<Expr1, Expr2>> = 1>
    inline constexpr bool operator!=(const Expr1& expr1, const Expr2& expr2) noexcept
    {
        for (size_t i = 0; i < Expr1::size; i++)
        {
            if (expr1[i] != expr2[i])
                return true;
        }
        return false;
    }

    template<class Expr1, class Expr2, traits::require<traits::is_same_size_v<Expr1, Expr2>> = 1>
    inline constexpr bool operator<(const Expr1& expr1, const Expr2& expr2) noexcept
    {
        for (size_t i = 0; i < Expr1::size; i++)
        {
            if (expr1[i] >= expr2[i])
                return false;
        }
        return true;
    }

    template<class Expr1, class Expr2, traits::require<traits::is_same_size_v<Expr1, Expr2>> = 1>
    inline constexpr bool operator<=(const Expr1& expr1, const Expr2& expr2) noexcept
    {
        for (size_t i = 0; i < Expr1::size; i++)
        {
            if (expr1[i] > expr2[i])
                return false;
        }
        return true;
    }

    template<class Expr1, class Expr2, traits::require<traits::is_same_size_v<Expr1, Expr2>> = 1>
    inline constexpr bool operator>(const Expr1& expr1, const Expr2& expr2) noexcept
    {
        return expr2 < expr1;
    }

    template<class Expr1, class Expr2, traits::require<traits::is_same_size_v<Expr1, Expr2>> = 1>
    inline constexpr bool operator>=(const Expr1& expr1, const Expr2& expr2) noexcept
    {
        return expr2 <= expr1;
    }

    /// @brief Provides all size-agnostic in-class functionality for vector expressions
    /// @param Child The child vector expression type, for use in CRTP
    template<class Child>
    struct expression_base
    {
    protected:
        using scalar_t = traits::scalar_t<Child>;
        using vector_t = traits::vector_t<Child>;
        static constexpr size_t size = traits::size_v<Child>;
    public:
        template<class Other, traits::require<traits::has_converter_v<vector_t, Other>> = 1>
        operator Other() const
        {
            Other other;
            converter<vector_t, Other>::convert(_child(), other);
            return other;
        }
        
        constexpr explicit operator bool() const noexcept
        {
            return nonzero();
        }

        ///  @brief Gets the component at specified index
        constexpr scalar_t at(const size_t index) const
        {
            return _child()[index];
        }

        /// @brief Determines if at least one component is non-zero
        constexpr bool nonzero() const noexcept
        {
            for (size_t i = 0; i < size; i++)
            {
                if (at(i))
                    return true;
            }
            return false;
        }

        /// @brief Determines if at least one component is equal to a scalar value
        template<class T, traits::require<std::is_arithmetic_v<T>> = 1>
        constexpr bool contains(const T value) const noexcept
        {
            for (size_t i = 0; i < size; i++)
            {
                if (at(i) == value)
                    return true;
            }
            return false;
        }
        
        /// @brief Sums all components
        constexpr scalar_t sum() const noexcept
        {
            scalar_t out = 0;

            for (size_t i = 0; i < size; i++)
                out += at(i);
            return out;
        }

        /// @brief Multiplies all components
        constexpr scalar_t product() const noexcept
        {
            scalar_t out = 1;

            for (size_t i = 0; i < size; i++)
                out *= at(i);
            return out;
        }

        /// @brief Calculates the dot product with another vector expression
        template<class Expr, traits::require<traits::is_same_size_v<Expr, Child>> = 1>
        constexpr scalar_t dot(const Expr& expr) const noexcept
        {
            scalar_t out = 0;

            for (size_t i = 0; i < size; i++)
                out += at(i) * expr[i];
            return out;
        }

        /// @brief Calculates the Euclidian length squared
        constexpr scalar_t length2() const noexcept
        {
            const vector_t& value = _child(); // implicitly evaluate if Child is an operation
            return value.dot(value);
        }

        /// @brief Calculates the Euclidian length
        /// @details Analagous to writing `std::sqrt(vector.length2())`
        double length() const
        {
            return std::sqrt((double)length2());
        }

        /// @brief Calculates the Euclidian distance to another vector expression squared
        template<class Expr, traits::require<traits::is_same_size_v<Expr, Child>> = 1>
        constexpr scalar_t distance2(const Expr& expr) const noexcept
        {
            return (_child() - expr).length2();
        }

        /// @brief Calculates the Euclidian distance to another vector expression
        /// @details Analagous to writing `std::sqrt(vector.distance2())`
        template<class Expr, traits::require<traits::is_same_size_v<Expr, Child>> = 1>
        constexpr double distance(const Expr& expr) const
        {
            return std::sqrt((double)distance2(expr));
        }

        /// @brief Calculates the normalized vector
        /// @details Analagous to writing `vector / vector.length()`
        value<double, size> normalize() const
        {
            return _child() / length();
        }

        /// @brief Sets the Euclidian length
        /// @details Analogous to writing `vector.normalize() * length`
        value<double, size> set_length(const double length) const noexcept
        {
            return normalize() * length;
        }

        /// @brief Calculates the delta angle to another vector expression
        template<class Expr, traits::require<traits::is_same_size_v<Expr, Child>> = 1>
        double delta_angle(const Expr& expr) const
        {
            return std::acos(dot(expr) / std::sqrt((double)length2() * expr.length2()));
        }

        /// @brief Creates an operation to apply a function to all components
        template<class Fn, traits::require<std::is_invocable_v<Fn, scalar_t>> = 1>
        constexpr _DD_OPERATION_T apply(const Fn& fn) const noexcept
        {
            return operation{ fn, _child() };
        }

        /// @brief Creates an operation to take the absolute value of each component
        constexpr _DD_OPERATION_T abs() const noexcept
        {
           static_assert(std::is_signed_v<scalar_t>, "Cannot take the absolute value of an unsigned vector type");
           return apply([](scalar_t v) { return std::abs(v); });
        }

        /// @brief Creates an operation to round each component
        constexpr _DD_OPERATION_T round() const noexcept
        {
            static_assert(std::is_floating_point_v<scalar_t>, "Cannot round an integer");
            return apply([](scalar_t v) { return std::round(v); });
        }

        /// @brief Creates an operation to floor each component
        constexpr _DD_OPERATION_T floor() const noexcept
        {
            static_assert(std::is_floating_point_v<scalar_t>, "Cannot floor an integer");
            return apply([](scalar_t v) { return std::floor(v); });
        }

        /// @brief Creates an operation to ceil each component
        constexpr _DD_OPERATION_T ceil() const noexcept
        {
            static_assert(std::is_floating_point_v<scalar_t>, "Cannot take the ceiling of an integer");
            return apply([](scalar_t v) { return std::ceil(v); });
        }

        /// @brief Creates an operation to cast each component
        template<class Scalar>
        constexpr _DD_OPERATION_T scalar_cast() const noexcept
        {
            static_assert(std::is_arithmetic_v<Scalar>, "Cannot scalar cast vector to non-arithmetic type");
            return apply([](scalar_t v) { return static_cast<Scalar>(v); });
        }

        /// @brief Serializes the vector to a string
        /// @param name Can be provided to differentiate between multiple vectors
        std::string to_string(const std::string& name = "") const noexcept
        {
            std::string out = name + "(";

            for (size_t i = 0; i < size; i++)
            {
                out += std::to_string(at(i));

                if (i < (size - 1))
                    out += ", ";
            }
            return out + ")";
        }
    protected:
        constexpr const Child& _child() const noexcept
        {
            return static_cast<const Child&>(*this);
        }
    };
    
    /// @defgroup Expressions
    /// @brief Provides all in-class functionality to vector expressions
    /// @param Child The child vector expression type, for use in CRTP
    template<class Child, size_t Child_size = traits::size_v<Child>>
    struct expression : expression_base<Child> {};

    /// @ingroup Expressions
    /// @brief Adds functionality for 2D vector expressions
    template<class Child>
    struct expression<Child, 2> : expression_base<Child>
    {
    protected:
        using base = expression_base<Child>;
        using typename base::vector_t;
    public:
        /// @brief Calculates the angle represented by the components
        double angle() const noexcept
        {
            return std::atan2(base::at(1), base::at(0));
        }

        /// @brief Constructs the vector value representation of an angle
        static vector_t from_angle(const double angle) noexcept
        {
            return
            {
                std::cos(angle),
                std::sin(angle)
            };
        }
    };

    /// @ingroup Expressions
    /// @brief Adds functionality for 3D vector expressions
    template<class Child>
    struct expression<Child, 3> : expression_base<Child>
    {
    protected:
        using base = expression_base<Child>;
        using typename base::vector_t;
    public:
        /// @brief Calculates the cross product with another vector expression
        template<class Expr, traits::require<traits::is_same_size_v<Expr, Child>> = 1>
        constexpr vector_t cross(const Expr& expr) const noexcept
        {
            return
            {
                base::at(1) * expr[2] - base::at(2) * expr[1],
                base::at(2) * expr[0] - base::at(0) * expr[2],
                base::at(0) * expr[1] - base::at(1) * expr[0]
            };
        }
    };
    
    /// @brief Acts as an intermediary type for operations involving expressions
    /// @param Op_fn The function type to apply to the operands
    /// @param Operands Types of the operands
    template<class Op_fn, class... Operands>
    struct operation : expression<operation<Op_fn, Operands...>>
    {
    private:
        using base = expression<operation>;
    public:
        /// @brief The scalar type of the value
        using scalar_t = typename base::scalar_t;

        /// @brief The vector size of the value
        static constexpr size_t size = base::size;

        /// @brief The vector result type of the operation
        using vector_t = typename base::vector_t;

        /// @brief Constructs a vector operation from a function and operands
        constexpr operation(const Op_fn op, const Operands&... args) noexcept : _op(op), _operands(args...) {}

        /// @brief Evaluates component at an index
        constexpr scalar_t operator[](const size_t index) const
        {
            auto evaulate_index = [&](const Operands&... args)
            {
                return _op(_get_operand_at(args, index)...);
            };
            return std::apply(evaulate_index, _operands);
        }

        /// @brief Evaluates vector operation to a vector value
        constexpr vector_t operator*() const noexcept
        {
            return evaluate();
        }

        /// @brief Evaluates vector operation to a vector value
        constexpr vector_t evaluate() const noexcept
        {
            return vector_t(*this);
        }
    private:
        template<class T>
        static constexpr scalar_t _get_operand_at(const T& value, const size_t index)
        {
            if constexpr(traits::is_expression_v<T>)
                return value[index];
            else
                return value;
        }

        const Op_fn _op;
        const std::tuple<const Operands&...> _operands;
    };
    
    /// @defgroup ValueData
    /// @brief Defines the vector value data
    /// @details The data is stored as a `Scalar data[Size]`. There are also specializations
    ///          defined which add named components if a compatible compiler is used

    /// @ingroup ValueData
    /// @brief Default case: only contains a `Scalar data[Size]` array
    template<class Scalar, size_t Size>
    struct value_data
    {
        Scalar data[Size];
    protected:
        template<class... Scalars>
        constexpr value_data(Scalars... scalars) noexcept : data{ scalars... } {}
    };

    /// @ingroup ValueData
    /// @brief Specialization for 2D vectors: adds component names `x`, `y`
    template<class Scalar>
    struct value_data<Scalar, 2>
    {
        union
        {
            Scalar data[2];

            struct
            {
                Scalar x, y;
            };
        };
    protected:
        template<class... Scalars>
        constexpr value_data(Scalars... scalars) noexcept : data{ scalars... } {}
    };

    /// @ingroup ValueData
    /// @brief Specialization for 3D vectors: adds component names `x`, `y`, `z`
    template<class Scalar>
    struct value_data<Scalar, 3>
    {
        union
        {
            Scalar data[3];

            struct
            {
                Scalar x, y, z;
            };
        };
    protected:
        template<class... Scalars>
        constexpr value_data(Scalars... scalars) noexcept : data{ scalars... } {}
    };

    /// @ingroup ValueData
    /// @brief Specialization for 4D vectors: adds component names `x`, `y`, `z`, `w`
    template<class Scalar>
    struct value_data<Scalar, 4>
    {
        union
        {
            Scalar data[4];

            struct
            {
                Scalar x, y, z, w;
            };
        };
    protected:
        template<class... Scalars>
        constexpr value_data(Scalars... scalars) noexcept : data{ scalars... } {}
    };

    /// @brief A vector expression containing a single vector value
    /// @param Scalar The scalar type of the vector
    /// @param Size The size of the vector
    template<class Scalar, size_t Size>
    struct value : expression<value<Scalar, Size>>, value_data<Scalar, Size>
    {
        static_assert(std::is_arithmetic_v<Scalar> && (Size > 1), "Invalid vector scalar_t or size");
    private:
        using base = expression<value>;
        using value_data = value_data<Scalar, Size>;
    public:
        using value_data::data;
        
        /// @brief The scalar type of the value
        using scalar_t = typename base::scalar_t;

        /// @brief The vector size of the value
        static constexpr size_t size = base::size;
        
        /// @defgroup Prefabs
        /// @brief Prefabricated vector values

        /// @ingroup Prefabs
        /// @brief Vector value filled with zeroes
        const static value zero;

        /// @ingroup Prefabs
        /// @brief Vector value filled with ones
        const static value identity;

        /// @brief Default constructs the vector value
        /// @details All components will be initialized to 0
        constexpr value() = default;

        /// @brief Constructs a vector value from individual component values
        /// @details Expects as many arguments as the vector size and that each argument
        ///          is convertible to the vector scalar type
        template<class... Scalars, traits::require<sizeof... (Scalars) == size && std::conjunction_v<std::is_convertible<Scalars, scalar_t>...>> = 1>
        constexpr value(const Scalars&... scalars) noexcept : value_data(static_cast<scalar_t>(scalars)...) {}

        /// @brief Constructs a vector value from a single repeated value
        /// @details All components will be initialized to `scalar`
        constexpr explicit value(const scalar_t scalar) noexcept
        {
            for (size_t i = 0; i < size; i++)
                data[i] = scalar;
        }

        /// @brief Copies component values from a different vector expression of the same size
        template<class Other, traits::require<traits::is_same_size_v<value, Other>> = 1>
        constexpr value(const Other& other)
        {
            assign(other);
        }

        /// @brief Create value from foreign type through converter
        /// @details `Other` must have a converter specialization defined
        template<class Other, traits::require<traits::has_converter_v<value, Other>> = 1>
        constexpr value(const Other& other)
        {
            converter<value, Other>::convert(other, *this);
        }

        /// @brief Copy assignment operator
        template<class Expr, traits::require<traits::is_same_size_v<value, Expr>> = 1>
        constexpr value& operator=(const Expr& expr)
        {
            return assign(expr);
        }

        /// @brief Gets the component value at an index
        constexpr scalar_t operator[](const size_t index) const noexcept
        {
            return data[index];
        }

        /// @brief Gets a reference to the component at an index
        constexpr scalar_t& operator[](const size_t index) noexcept
        {
            return data[index];
        }

        /// @brief Copies component values from another vector expression
        /// @details Vector values are copied normally. Vector operations are evaluated and copied.
        template<class Expr, traits::require<traits::is_same_size_v<value, Expr>> = 1>
        constexpr value& assign(const Expr& expr) noexcept
        {
            for (size_t i = 0; i < size; i++)
                data[i] = expr[i];
            return *this;
        }

        /// @defgroup Iterators
        /// @brief These allow the use of vector values in STL algorithms and ranged-for loops

        /// @ingroup Iterators
        Scalar* begin()
        {
            return data;
        }

        /// @ingroup Iterators
        Scalar* end()
        {
            return data + size;
        }

        /// @ingroup Iterators
        const Scalar* begin() const
        {
            return data;
        }

        /// @ingroup Iterators
        const Scalar* end() const
        {
            return data + size;
        }
    };
}

/// @param Scalar The scalar type of the vector (e.g. `int` or `float`)
/// @param Size The size of the vector
template<class Scalar, size_t Size>
using vector = impl::value<Scalar, Size>;

template<class S, size_t N>
const vector<S, N> vector<S, N>::zero(0);

template<class S, size_t N>
const vector<S, N> vector<S, N>::identity(1);

namespace types
{
    // 2D
    using bool2d   = vector<bool,     2>;
    using char2d   = vector<int8_t,   2>;
    using uchar2d  = vector<uint8_t,  2>;
    using int2d    = vector<int32_t,  2>;
    using uint2d   = vector<uint32_t, 2>;
    using long2d   = vector<int64_t,  2>;
    using ulong2d  = vector<uint64_t, 2>;
    using float2d  = vector<float,    2>;
    using double2d = vector<double,   2>;

    // 3D
    using bool3d   = vector<bool,     3>;
    using char3d   = vector<int8_t,   3>;
    using uchar3d  = vector<uint8_t,  3>;
    using int3d    = vector<int32_t,  3>;
    using uint3d   = vector<uint32_t, 3>;
    using long3d   = vector<int64_t,  3>;
    using ulong3d  = vector<uint64_t, 3>;
    using float3d  = vector<float,    3>;
    using double3d = vector<double,   3>;

    // 4D
    using bool4d   = vector<bool,     4>;
    using char4d   = vector<int8_t,   4>;
    using uchar4d  = vector<uint8_t,  4>;
    using int4d    = vector<int32_t,  4>;
    using uint4d   = vector<uint32_t, 4>;
    using long4d   = vector<int64_t,  4>;
    using ulong4d  = vector<uint64_t, 4>;
    using float4d  = vector<float,    4>;
    using double4d = vector<double,   4>;
}

// bring the vector types to the dd namespace
using namespace types;


_DD_NAMESPACE_CLOSE


namespace std
{
    /// @brief Serializes vector expression to a `std::ostream`
    /// @details Allows for e.g. `std::cout << vector`
    template<class Expr, dd::traits::require<dd::traits::is_expression_v<Expr>> = 1>
    std::ostream& operator<<(std::ostream& stream, const Expr& e)
    {
        return stream << e.to_string();
    }

    /// @brief Hash specialization for use in `std::unordered_*` containers
    template<class Scalar, size_t Size>
    struct hash<dd::vector<Scalar, Size>>
    {
        size_t operator()(const dd::vector<Scalar, Size>& v) const
        {
            // interpret data as a string_view and use the string_view hasher

            const std::string_view byte_data = { reinterpret_cast<const char*>(v.data), Size * sizeof(Scalar) };
            const std::hash<std::string_view> string_hash;
            return string_hash(byte_data);
        }
    };
}
