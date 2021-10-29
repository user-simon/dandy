#pragma once
#include <string>
#include <string_view>
#include <ostream>
#include <type_traits>
#include <tuple>      // std::tuple, std::apply
#include <functional> // std::hash
#include <algorithm>  // std::max
#include <cstdint>    // fixed width integer types

#define _DD_NAMESPACE_OPEN namespace dd {
#define _DD_NAMESPACE_CLOSE }
#define _DD_OPERATION_T auto
#define _DD_DEFINE_BINARY_OPERATOR(op)                                                              \
    template<class L, class R, class = std::enable_if_t<traits::is_valid_operation_v<L, R, false>>> \
    constexpr inline _DD_OPERATION_T operator op (const L& l, const R& r)                           \
    {                                                                                               \
        return detail::operation                                                                    \
        {                                                                                           \
            [](const auto& a, const auto& b) { return a op b; },                                    \
            l, r                                                                                    \
        };                                                                                          \
                                                                                                    \
    }                                                                                               \
    template<class L, class R, class = std::enable_if_t<traits::is_valid_operation_v<L, R, true>>>  \
    constexpr inline _DD_OPERATION_T operator op##=(L& l, const R& r)                               \
    {                                                                                               \
        return l = l op r;                                                                          \
    }                                                                                               \

#define _DD_DEFINE_UNARY_OPERATOR(op)                                             \
    template<class Expr, class = std::enable_if_t<traits::is_expression_v<Expr>>> \
    constexpr inline _DD_OPERATION_T operator op (const Expr& expr)               \
    {                                                                             \
        return detail::operation                                                  \
        {                                                                         \
            [](const auto& v) { return op(v); },                                  \
            expr                                                                  \
        };                                                                        \
    }                                                                             \


_DD_NAMESPACE_OPEN


/// @brief Define as 0 to disable named vector components
#ifndef DD_ENABLE_NAMES
#define DD_ENABLE_NAMES 1
#endif

namespace detail
{
    template<class>
    struct expression_base;

    template<class, size_t>
    struct expression;

    template<class, class...>
    struct operation;

    template<class, size_t, bool>
    struct component_names;

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
    template<class T>
    struct type_identity
    {
        using type = T;
    };

    /// @var is_value_v
    /// @brief Determines if a type is a vector value type
    template<class T>
    struct is_value : std::false_type {};

    template<class Scalar, size_t Size>
    struct is_value<detail::value<Scalar, Size>> : std::true_type {};

    template<class T>
    inline constexpr bool is_value_v = is_value<T>::value;

    /// @var is_operation_v
    /// @brief Determines if a type is a vector operation type
    template<class T>
    struct is_operation : std::false_type {};

    template<class Op_fn, class... Operands>
    struct is_operation<detail::operation<Op_fn, Operands...>> : std::true_type {};

    template<class T>
    inline constexpr bool is_operation_v = is_operation<T>::value;

    /// @var is_expression_v
    /// @brief Determines if a type is a vector operation type
    template<class T>
    struct is_expression : std::disjunction<is_value<T>, is_operation<T>> {};

    template<class T>
    inline constexpr bool is_expression_v = is_expression<T>::value;

    /// @typedef scalar_t
    /// @brief Gets the scalar type of the vector expression
    /// @details For vector operations, this is set to the result type of the operation function
    ///          when called with the operation operands
    template<class Scalar>
    struct scalar : type_identity<Scalar> {};

    template<class Op_fn, class... Operands>
    struct scalar<detail::operation<Op_fn, Operands...>> : std::invoke_result<Op_fn, typename scalar<Operands>::type...> {};

    template<class Scalar, size_t Size>
    struct scalar<detail::value<Scalar, Size>> : type_identity<Scalar> {};

    template<class Expr>
    using scalar_t = typename scalar<Expr>::type;

    /// @var size_v
    /// @brief Gets the size of the vector expression
    /// @details For vector operations, this is set to the max size of its operands
    template<class>
    struct size : std::integral_constant<size_t, 1> {};

    template<class Op_fn, class... Operands>
    struct size<detail::operation<Op_fn, Operands...>> : std::integral_constant<size_t, std::max({ size<Operands>::value... })> {};

    template<class Scalar, size_t Size>
    struct size<detail::value<Scalar, Size>> : std::integral_constant<size_t, Size> {};

    template<class Expr>
    inline constexpr size_t size_v = size<Expr>::value;
    
    /// @typedef vector_t
    /// @brief Gets the resulting vector type of the vector expression
    template<class Expr, class = std::enable_if_t<is_expression_v<Expr>>>
    struct _vector : type_identity<detail::value<scalar_t<Expr>, size_v<Expr>>> {};

    template<class Expr>
    struct vector : _vector<Expr> {};

    template<class Expr>
    using vector_t = typename vector<Expr>::type;

    /// @var is_same_size_v
    /// @brief Determines if two types are vector expressions of the same size
    template<class T, class U>
    struct is_same_size : std::conjunction<is_expression<T>, is_expression<U>, std::bool_constant<size_v<T> == size_v<U>>> {};

    template<class T, class U>
    inline constexpr bool is_same_size_v = is_same_size<T, U>::value;

    /// @var is_valid_operation_v
    /// @brief Determines if two types form a valid vector operation
    /// @param Strict_ordering A value of true forbids a scalar type from appearing first in
    ///                        the operation
    template<class L, class R, bool Strict_ordering>
    struct is_valid_operation
        : std::bool_constant<(is_same_size_v<L, R>)                                               ||
                             (is_expression_v<L> && std::is_arithmetic_v<R>)                      ||
                             (std::is_arithmetic_v<L> && is_expression_v<R> && !Strict_ordering)> {};
    
    template<class L, class R, bool Strict_ordering>
    inline constexpr bool is_valid_operation_v = is_valid_operation<L, R, Strict_ordering>::value;

    /// @var has_named_components_v
    /// @brief Determines if a vector value has named vector components
    template<class>
    struct has_named_components : std::false_type {};

    template<class Scalar, size_t Size>
    struct has_named_components<detail::value<Scalar, Size>> : std::is_base_of<detail::component_names<Scalar, Size, true>, detail::value<Scalar, Size>> {};
    
    template<class Expr>
    inline constexpr bool has_named_components_v = has_named_components<Expr>::value;
    
    /// @var has_converter_v
    /// @brief Determines if a there is a converter specilization defined between two types
    template<class, class, class = void>
    struct has_converter : std::false_type {};

    template<class T, class U>
    struct has_converter<T, U, std::void_t<
        decltype(converter<T, U>::convert(std::declval<T>(), std::declval<U&>())),  // has T -> U
        decltype(converter<T, U>::convert(std::declval<U>(), std::declval<T&>()))>> // has U -> T
    : std::true_type {};

    template<class T, class U>
    inline constexpr bool has_converter_v = has_converter<T, U>::value;
}

namespace detail
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
    
    /// @brief Provides all size-agnostic in-class functionality for vector expressions
    /// @param Child The child vector expression type, for use in CRTP
    template<class Child>
    struct expression_base
    {
    protected:
        using scalar_t = traits::scalar_t<Child>;
        using vector_t = traits::vector_t<Child>;
        constexpr static size_t size = traits::size_v<Child>;
    public:
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<Expr, Child>>>
        constexpr bool operator==(const Expr& expr) const noexcept
        {
            for (size_t i = 0; i < size; i++)
            {
                if (get(i) != expr[i])
                    return false;
            }
            return true;
        }

        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<Expr, Child>>>
        constexpr bool operator!=(const Expr& expr) const noexcept
        {
            return !operator==(expr);
        }

        template<class Other, class = std::enable_if_t<traits::has_converter_v<vector_t, Other>>>
        inline operator Other() const
        {
            Other other;

            if constexpr(traits::is_value_v<Child>)
                converter<vector_t, Other>::convert(_child(), other);
            else
                converter<vector_t, Other>::convert(_child().evaluate(), other);
            return other;
        }
        
        constexpr explicit operator bool() const noexcept
        {
            return nonzero();
        }

        ///  @brief Gets the component at specified index
        constexpr inline scalar_t at(size_t index) const
        {
            return _child()[index];
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

        /// @brief Determines if vector is non-zero, that is if the vector has a non-zero component
        constexpr bool nonzero() const noexcept
        {
            for (size_t i = 0; i < size; i++)
            {
                if (at(i))
                    return true;
            }
            return false;
        }

        /// @brief Calculates the dot product with another vector expression
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<Expr, Child>>>
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
            return dot(_child());
        }

        /// @brief Calculates the Euclidian length
        /// @details Analagous to writing `std::sqrt(vector.length2())`
        double length() const
        {
            return std::sqrt((double)length2());
        }

        /// @brief Calculates the Euclidian distance to another vector expression squared
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<Expr, Child>>>
        constexpr scalar_t distance2(const Expr& expr) const noexcept
        {
            return (_child() - expr).length2();
        }

        /// @brief Calculates the Euclidian distance to another vector expression
        /// @details Analagous to writing `std::sqrt(vector.distance2())`
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<Expr, Child>>>
        constexpr double distance(const Expr& expr) const
        {
            return std::sqrt((double)distance2(expr));
        }

        /// @brief Calculates the normalized vector
        /// @details Analagous to writing `vector / vector.length()``
        value<double, size> normalize() const
        {
            return _child() / length();
        }

        /// @brief Sets the Euclidian length
        /// @details Analogous to writing `vector.normalize() * length`
        value<double, size> set_length(double length) const noexcept
        {
            return normalize() * length;
        }

        /// @brief Calculates the delta angle to another vector
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<Expr, Child>>>
        double delta_angle(const Expr& expr) const
        {
            return std::acos(dot(expr) / std::sqrt((double)length2() * expr.length2()));
        }

        /// @brief Creates an operation to apply a function to all components
        template<class Fn, class = std::enable_if_t<std::is_invocable_v<Fn, scalar_t>>>
        inline constexpr _DD_OPERATION_T apply(const Fn& fn) const noexcept
        {
            return operation{ fn, _child() };
        }

        /// @brief Creates an operation to take the absolute value of each component
        inline constexpr _DD_OPERATION_T abs() const noexcept
        {
            return apply([](scalar_t v) { return std::abs(v); });
        }

        /// @brief Creates an operation to round each component
        inline constexpr _DD_OPERATION_T round() const noexcept
        {
            return apply([](scalar_t v) { return std::round(v); });
        }

        /// @brief Creates an operation to floor each component
        inline constexpr _DD_OPERATION_T floor() const noexcept
        {
            return apply([](scalar_t v) { return std::floor(v); });
        }

        /// @brief Creates an operation to ceil each component
        inline constexpr _DD_OPERATION_T ceil() const noexcept
        {
            return apply([](scalar_t v) { return std::ceil(v); });
        }

        /// @brief Creates an operation to cast each component
        template<class Scalar, class = std::enable_if_t<std::is_arithmetic_v<Scalar>>>
        inline constexpr _DD_OPERATION_T scalar_cast() const noexcept
        {
            return apply([](scalar_t v) { return (Scalar)v; });
        }

        /// @brief Serializes the vector to a string
        /// @param name Can be provided to differentiate between multiple vector values
        std::string to_string(const std::string& name = "") const noexcept
        {
            std::string out = name + "(";

            for (size_t i = 0; i < size; i++)
            {
                out += std::to_string(at(i));

                if (i < (size - 1)) [[likely]]
                    out += ", ";
            }
            return out + ")";
        }
    protected:
        constexpr inline const Child& _child() const noexcept
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
        /// @brief Calculates the angle represented by the vector expression
        double angle() const noexcept
        {
            return std::atan2(base::at(1), base::at(0));
        }

        /// @brief Constructs the vector value representation of an angle
        static vector_t from_angle(double angle) noexcept
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
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<Expr, Child>>>
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
        /// @brief The scalar type result of the operation
        using typename base::scalar_t;

        /// @brief The vector type result of the operation
        using typename base::vector_t;

        /// @brief The size of the vectors in the operation
        using base::size;
        
        /// @brief Constructs a vector operation from a function and operands
        operation(const Op_fn& op, const Operands&... args) noexcept : _operands(args...), _op(op) {}

        /// @brief Evaluates component at an index
        inline constexpr scalar_t operator[](size_t index) const
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
            return (vector_t)*this; // call the value constructor
        }
    private:
        const std::tuple<const Operands&...> _operands;
        const Op_fn& _op;

        template<class T>
        static inline constexpr scalar_t _get_operand_at(const T& value, size_t index)
        {
            if constexpr(traits::is_expression_v<T>)
                return value[index];
            else
                return value;
        }
    };
    
    /// @defgroup ComponentNames
    /// @brief These specializations provide the named vector components to vector values
    ///        (e.g. vector.x, vector.y).
    /// @details Each component is a reference to the value at its associated index in the
    ///          `value::data` array
    template<class Scalar, size_t Size, bool = DD_ENABLE_NAMES>
    struct component_names
    {
        constexpr component_names(Scalar*) noexcept {}
    };

    /// @ingroup ComponentNames
    template<class Scalar>
    struct component_names<Scalar, 2, true>
    {
        /// @brief The `X` component
        Scalar& x;

        /// @brief The `Y` component
        Scalar& y;

        constexpr component_names(Scalar data[2]) noexcept : x(data[0]), y(data[1]) {}
        constexpr component_names& operator=(const component_names&) noexcept { return *this; }
    };

    /// @ingroup ComponentNames
    template<class Scalar>
    struct component_names<Scalar, 3, true> : component_names<Scalar, 2>
    {
        /// @brief The `Z` component
        Scalar& z;

        constexpr component_names(Scalar data[3]) noexcept : component_names<Scalar, 2>(data), z(data[2]) {}
        constexpr component_names& operator=(const component_names&) noexcept { return *this; }
    };

    /// @ingroup ComponentNames
    template<class Scalar>
    struct component_names<Scalar, 4, true> : component_names<Scalar, 3>
    {
        /// @brief The `W` component
        Scalar& w;

        constexpr component_names(Scalar data[4]) noexcept : component_names<Scalar, 3>(data), w(data[3]) {}
        constexpr component_names& operator=(const component_names&) noexcept { return *this; }
    };
    
    /// @brief A vector expression containing a single vector value
    /// @param Scalar The scalar type of the vector
    /// @param Size The size of the vector
    template<class Scalar, size_t Size>
    struct value : expression<value<Scalar, Size>>, component_names<Scalar, Size>
    {
        static_assert(std::is_arithmetic_v<Scalar> && (Size > 1), "Invalid vector scalar_t or size");
    private:
        using base = expression<value>;
        using component_names = component_names<Scalar, Size>;
    public:
        /// @brief The scalar type of the value
        using typename base::scalar_t;

        /// @brief The vector size of the value
        using base::size;

        /// @brief Vector value filled with zeroes
        const static value zero;

        /// @brief Vector value filled with ones
        const static value identity;
              
        /// @brief Houses the data for the vector value
        scalar_t data[size];
        
        /// @brief Default constructs the vector value
        /// @details All components will be initialized to 0
        constexpr value() noexcept : component_names(data), data{} {}

        /// @brief Constructs a vector value from individual component values
        /// @details Expects as many arguments as the vector size and that each argument
        ///          is convertible to the vector scalar type
        template<class... Scalars, class = std::enable_if_t<sizeof...(Scalars) == Size && std::conjunction_v<std::is_convertible<Scalars, scalar_t>...>>>
        constexpr value(const Scalars&... args) noexcept : component_names(data), data{ (scalar_t)args... } {}

        /// @brief Constructs a vector value from a single repeated value
        /// @details All components will be initialized to v
        constexpr explicit value(scalar_t scalar) noexcept : component_names(data)
        {
            for (size_t i = 0; i < size; i++)
                data[i] = scalar;
        }

        /// @brief Copies component values from another vector value of the same type
        /// @details This function is defined explicitly to ensure component_names is
        ///          initialized properly
        constexpr value(const value& other) noexcept : component_names(data)
        {
            assign(other);
        }

        /// @brief Copies component values from a different type
        /// @details Vector values are copied normally. Vector operations are evaluated and copied.
        ///          Foreign types with a converter specialization defined are converted and then copied.
        template<class Other, class = std::enable_if_t<traits::is_same_size_v<value, Other> || traits::has_converter_v<value, Other>>>
        constexpr value(const Other& other) : component_names(data)
        {
            if constexpr(traits::is_same_size_v<value, Other>)
                assign(other);
            else
                converter<value, Other>::convert(other, *this);
        }

        /// @brief Copy assignment operator
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<value, Expr>>>
        inline constexpr value& operator=(const Expr& expr)
        {
            assign(expr);
            return *this;
        }

        /// @brief Gets the component value at an index
        inline constexpr scalar_t operator[](size_t index) const noexcept
        {
            return data[index];
        }

        /// @brief Gets a reference to the component at an index
        inline constexpr scalar_t& operator[](size_t index) noexcept
        {
            return data[index];
        }

        /// @brief Copies component values from another vector expression
        /// @details Vector values are copied normally. Vector operations are evaluated and copied.
        template<class Expr, class = std::enable_if_t<traits::is_same_size_v<value, Expr>>>
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
using vector = detail::value<Scalar, Size>;

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

template<class Scalar, size_t Size>
const vector<Scalar, Size> vector<Scalar, Size>::zero(0);

template<class Scalar, size_t Size>
const vector<Scalar, Size> vector<Scalar, Size>::identity(1);


_DD_NAMESPACE_CLOSE


namespace std
{
    /// @brief Serializes vector expression to a `std::ostream`
    /// @details Allows for e.g. `std::cout << vector`
    template<class Expr, class = std::enable_if_t<dd::traits::is_expression_v<Expr>>>
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

            std::string_view byte_data = { (char*)v.data, Size * sizeof(Scalar) };
            std::hash<std::string_view> string_hash;
            return string_hash(byte_data);
        }
    };
}
