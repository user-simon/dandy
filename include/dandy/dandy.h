#pragma once
#include <type_traits> // see dd::traits
#include <algorithm>   // std::max
#include <ostream>     // std::ostream
#include <string>      // std::string
#include <optional>    // std::optional
#include <functional>  // std::hash
#include <tuple>       // std::tuple

#define _DD_NAMESPACE_OPEN namespace dd {
#define _DD_NAMESPACE_CLOSE }

#define _DD_OPERATION_T auto // defined for explicit return types
#define _DD_APPLY_WRAPPER(fn) apply([](const scalar_t& v) { return fn(v); }) // wrapper to select the correct function overload
#define _DD_TEMPLATE_CONSTRAINT(ty, constraint) template<class ty, class = std::enable_if_t<constraint>>

#define _DD_COMPONENT_DEFINES(str)                                                               \
        static constexpr char _names_str[] = str;                                                \
        constexpr component_names& operator= (const component_names&) noexcept { return *this; } \

#define _DD_BASE_COMMON_DEFINES                               \
    public:                                                   \
        using result_t = traits::result_t<CHILD>;             \
        using scalar_t = traits::scalar_t<CHILD>;             \
        static constexpr size_t size = traits::size_v<CHILD>; \
    private:                                                  \
        inline constexpr const CHILD& _child() const          \
        {                                                     \
            return static_cast<const CHILD&>(*this);          \
        }                                                     \
                                                              \
        inline constexpr scalar_t _value(size_t i) const      \
        {                                                     \
            return _child()[i];                               \
        }                                                     \
    public:                                                   \

#define _DD_DEFINE_BINARY_OPERATOR(op, strict_ordering)                                                       \
    template<class L, class R, class = std::enable_if_t<traits::is_valid_operation_v<L, R, strict_ordering>>> \
    constexpr inline _DD_OPERATION_T operator op (const L& l, const R& r)                                     \
    {                                                                                                         \
        return expr::operation                                                                                \
        {                                                                                                     \
            [](const auto& a, const auto& b) { return a op b; },                                              \
            l, r                                                                                              \
        };                                                                                                    \
                                                                                                              \
    }                                                                                                         \
    template<class L, class R, class = std::enable_if_t<traits::is_valid_operation_v<L, R, true>>>            \
    constexpr inline _DD_OPERATION_T operator op##=(L& l, const R& r)                                         \
    {                                                                                                         \
        return l = l op r;                                                                                    \
    }                                                                                                         \

#define _DD_DEFINE_UNARY_OPERATOR(op)                                 \
    template<class E, class = std::enable_if_t<traits::is_expr_v<E>>> \
    constexpr inline _DD_OPERATION_T operator op (const E& e)         \
    {                                                                 \
        return expr::operation                                        \
        {                                                             \
            [](const auto& v) { return op(v); },                      \
            e                                                         \
        };                                                            \
    }                                                                 \

#define _DD_DEFINE_VECTOR_ALIAS(ty, ty_name, size) using ty_name##size##d = vector<ty, size>;
#define _DD_DEFINE_VECTOR_SIZE_ALIASES(size)              \
    _DD_DEFINE_VECTOR_ALIAS(bool,          binary, size); \
    _DD_DEFINE_VECTOR_ALIAS(char,          char,   size); \
    _DD_DEFINE_VECTOR_ALIAS(unsigned char, uchar,  size); \
    _DD_DEFINE_VECTOR_ALIAS(int,           int,    size); \
    _DD_DEFINE_VECTOR_ALIAS(unsigned int,  uint,   size); \
    _DD_DEFINE_VECTOR_ALIAS(float,         float,  size); \
    _DD_DEFINE_VECTOR_ALIAS(double,        double, size); \


_DD_NAMESPACE_OPEN


/*
 *  DD_NO_NAMES
 *    define flag to disable named vector components
*/
#ifdef DD_NO_NAMES
inline constexpr size_t ENABLE_NAMES = false;
#else
inline constexpr size_t ENABLE_NAMES = true;
#endif

/*
 *  forward declares
 *    used by traits
*/
namespace expr
{
    template<class>
    struct base;

    template<class, class...>
    struct operation;

    template<class, size_t>
    struct value;
}

/*
 *  converter
 *    interface to enable conversions between vector and arbitrary user types
*/
template<class, class>
struct converter {};

namespace traits
{
    /*
     *  size
     *    gets the vector size of expression T
    */
    template<class>
    struct size : std::integral_constant<size_t, 1> {};

    template<class OP, class... ARGS>
    struct size<expr::operation<OP, ARGS...>> : std::integral_constant<size_t, std::max({ size<ARGS>::value... })> {};

    template<class S, size_t N>
    struct size<expr::value<S, N>> : std::integral_constant<size_t, N> {};

    template<class T>
    inline constexpr size_t size_v = size<T>::value;

    /*
     *  scalar
     *    gets the scalar type of expression T
    */
    template<class S>
    struct scalar : std::common_type<S> {};

    template<class OP, class... ARGS>
    struct scalar<expr::operation<OP, ARGS...>> : std::invoke_result<OP, typename scalar<ARGS>::type...> {};

    template<class S, size_t N>
    struct scalar<expr::value<S, N>> : std::common_type<S> {};

    template<class T>
    using scalar_t = typename scalar<T>::type;
    
    /*
     *  result
     *    gets the resulting vector value of expression T
    */
    template<class>
    struct result {};

    template<class OP, class... ARGS>
    struct result<expr::operation<OP, ARGS...>> : std::common_type<expr::value<scalar_t<expr::operation<OP, ARGS...>>, size_v<expr::operation<OP, ARGS...>>>> {};

    template<class S, size_t N>
    struct result<expr::value<S, N>> : std::common_type<expr::value<S, N>> {};

    template<class T>
    using result_t = typename result<T>::type;

    /*
     *  is_expr
     *    determines if T is a vector expression
    */
    template<class T>
    struct is_expr : std::is_base_of<expr::base<T>, T> {};

    template<class T>
    inline constexpr bool is_expr_v = is_expr<T>::value;

    /*
     *  is_same_size
     *    determines if T and U are both expressions of the same size
    */
    template<class T, class U>
    struct is_same_size : std::conjunction<is_expr<T>, is_expr<U>, std::bool_constant<size_v<T> == size_v<U>>> {};

    template<class T, class U>
    inline constexpr bool is_same_size_v = is_same_size<T, U>::value;

    /*
     *  is valid operation
     *    determines if T and U make up a valid operation
     *    STRICT_ORDERING = true forbids a scalar type from appearing first
    */
    template<class L, class R, bool STRICT_ORDERING>
    struct is_valid_operation
        : std::bool_constant<is_same_size_v<L, R>                                         ||
                             is_expr_v<L> && std::is_arithmetic_v<R>                      ||
                             std::is_arithmetic_v<L> && is_expr_v<R> && !STRICT_ORDERING> {};
    
    template<class L, class R, bool STRICT_ORDERING>
    inline constexpr bool is_valid_operation_v = is_valid_operation<L, R, STRICT_ORDERING>::value;

    /*
     *  is value
     *    determines if T is a value expression
    */
    template<class>
    struct is_value : std::false_type {};

    template<class S, size_t N>
    struct is_value<expr::value<S, N>> : std::true_type {};

    template<class T>
    inline constexpr bool is_value_v = is_value<T>::value;

    /*
     *  has named components
     *    determines if T has named components
    */
    template<class, class = void>
    struct has_named_components : std::false_type {};

    template<class T>
    struct has_named_components<T, std::void_t<decltype(T::_names_str)>> : std::true_type {};

    template<class T>
    inline constexpr bool has_named_components_v = has_named_components<T>::value;
    
    /*
     *  has_converter
     *    determines if there is a converter specialization defined from T to U
    */
    template<class, class, class = void>
    struct has_converter : std::false_type {};

    template<class T, class U>
    struct has_converter<T, U, std::void_t<decltype(converter<T, U>::from(std::declval<T>()))>> : std::true_type {};

    template<class T, class U>
    inline constexpr bool has_converter_v = has_converter<T, U>::value;
}

namespace expr
{
    /*
     *  operator overloads
    */
    _DD_DEFINE_BINARY_OPERATOR(+,  false);
    _DD_DEFINE_BINARY_OPERATOR(-,  false);
    _DD_DEFINE_BINARY_OPERATOR(*,  false);
    _DD_DEFINE_BINARY_OPERATOR(/,  false);
    _DD_DEFINE_BINARY_OPERATOR(%,  false);
    _DD_DEFINE_BINARY_OPERATOR(&,  false);
    _DD_DEFINE_BINARY_OPERATOR(|,  false);
    _DD_DEFINE_BINARY_OPERATOR(^,  false);
    _DD_DEFINE_BINARY_OPERATOR(>>, true);
    _DD_DEFINE_BINARY_OPERATOR(<<, true);

    _DD_DEFINE_UNARY_OPERATOR(+);
    _DD_DEFINE_UNARY_OPERATOR(-);
    _DD_DEFINE_UNARY_OPERATOR(~);

    /*
     *  base specialization
     *    provides size specific functionality to base
    */
    template<class, size_t>
    struct base_spec {};

    template<class CHILD>
    struct base_spec<CHILD, 2>
    {
        _DD_BASE_COMMON_DEFINES

        double angle() const noexcept
        {
            return std::atan2((double)_value(1), _value(0));
        }

        static result_t from_angle(double angle) noexcept
        {
            return
            {
                std::cos(angle),
                std::sin(angle)
            };
        }
    };

    template<class CHILD>
    struct base_spec<CHILD, 3>
    {
        _DD_BASE_COMMON_DEFINES

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<E, CHILD>))
        constexpr result_t cross(const E& e) const noexcept
        {
            return
            {
                _value(1) * e[2] - _value(2) * e[1],
                _value(2) * e[0] - _value(0) * e[2],
                _value(0) * e[1] - _value(1) * e[0]
            };
        }
    };

    /*
     *  base
     *    provides in-class functionality for expressions
    */
    template<class CHILD>
    struct base : base_spec<CHILD, traits::size_v<CHILD>>
    {
        _DD_BASE_COMMON_DEFINES
        
        /*
         *  operators
        */

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<E, CHILD>))
        constexpr bool operator==(const E& e) const noexcept
        {
            for (size_t i = 0; i < size; i++)
            {
                if (_value(i) != e[i])
                    return false;
            }
            return true;
        }

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<E, CHILD>))
        constexpr bool operator!=(const E& e) const noexcept
        {
            return !operator==(e);
        }

        _DD_TEMPLATE_CONSTRAINT(T, (traits::has_converter_v<result_t, T>))
        inline operator T() const
        {
            if constexpr(traits::is_value_v<CHILD>)
                return converter<result_t, T>::from(_child());
            else
                return converter<result_t, T>::from(_child().evaluate());
        }

        constexpr explicit operator bool() const noexcept
        {
            for (size_t i = 0; i < size; i++)
            {
                if (_value(i))
                    return true;
            }
            return false;
        }

        /*
         *  math
        */

        constexpr scalar_t sum() const noexcept
        {
            scalar_t out = 0;

            for (size_t i = 0; i < size; i++)
                out += _value(i);
            return out;
        }

        constexpr scalar_t product() const noexcept
        {
            scalar_t out = 1;

            for (size_t i = 0; i < size; i++)
                out *= _value(i);
            return out;
        }

        constexpr bool nonzero() const noexcept
        {
            return operator bool();
        }

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<E, CHILD>))
        constexpr scalar_t dot(const E& e) const noexcept
        {
            scalar_t out = 0;

            for (size_t i = 0; i < size; i++)
                out += _value(i) * e[i];
            return out;
        }

        constexpr scalar_t length2() const noexcept
        {
            return dot(_child());
        }

        double length() const
        {
            return std::sqrt((double)length2());
        }

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<E, CHILD>))
        constexpr scalar_t distance2(const E& e) const noexcept
        {
            return (_child() - e).length2();
        }

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<E, CHILD>))
        constexpr double distance(const E& e) const
        {
            return std::sqrt((double)distance2(e));
        }

        value<double, size> normalize() const
        {
            return _child() / length();
        }

        value<double, size> set_length(double l) const noexcept
        {
            return normalize() * l;
        }

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<E, CHILD>))
        double delta_angle(const E& e) const
        {
            return std::acos(dot(e) / std::sqrt((double)length2() * e.length2()));
        }

        _DD_TEMPLATE_CONSTRAINT(FN, (std::is_invocable_v<FN, scalar_t>))
        inline constexpr _DD_OPERATION_T apply(const FN& fn) const noexcept
        {
            return operation{ fn, _child() };
        }

        inline constexpr _DD_OPERATION_T abs() const noexcept
        {
            return _DD_APPLY_WRAPPER(std::abs);
        }

        inline constexpr _DD_OPERATION_T round() const noexcept
        {
            return _DD_APPLY_WRAPPER(std::round);
        }

        inline constexpr _DD_OPERATION_T floor() const noexcept
        {
            return _DD_APPLY_WRAPPER(std::floor);
        }

        inline constexpr _DD_OPERATION_T ceil() const noexcept
        {
            return _DD_APPLY_WRAPPER(std::ceil);
        }

        _DD_TEMPLATE_CONSTRAINT(S, std::is_arithmetic_v<S>)
        inline constexpr _DD_OPERATION_T scalar_cast() const noexcept
        {
            return _DD_APPLY_WRAPPER(S);
        }
    };
    
    /*
     *  operation
     *    expressions involving and operation and at least one other expression
     *    each component is evaluated in operator[]
    */
    template<class OP, class... ARGS>
    struct operation : base<operation<OP, ARGS...>>
    {
    private:
        const std::tuple<const ARGS&...> _args;
        const OP& _op;

        template<class T>
        static inline constexpr traits::scalar_t<operation> _get_operand_at(const T& value, size_t i)
        {
            if constexpr(traits::is_expr_v<T>)
                return value[i];
            else
                return value;
        }
    public:
        /*
         *  ctors
        */

        operation(const OP& op, const ARGS&... args) noexcept : _op(op), _args(args...) {}

        /*
         *  operators
        */

        inline constexpr traits::scalar_t<operation> operator[](size_t i) const
        {
            auto evaulate_index = [&](const ARGS&... args)
            {
                return _op(_get_operand_at(args, i)...);
            };
            return std::apply(evaulate_index, _args);
        }

        constexpr traits::result_t<operation> operator*() const noexcept
        {
            return *this;
        }

        /*
         *  utility
        */

        constexpr traits::result_t<operation> evaluate() const noexcept
        {
            return operator*();
        }

        std::string to_string() const noexcept
        {
            return evaluate().to_string();
        }
    };
    
    /*
     *  component_names
     *    provides named components (e.g. x, y) for value expressions
     *    for performance reasons, can be disabled by defining flag DD_NO_NAMES 
    */
    template<class S, size_t, bool = ENABLE_NAMES>
    struct component_names
    {
        constexpr component_names(S*) noexcept {}
    };

    template<class S>
    struct component_names<S, 2, true>
    {
        _DD_COMPONENT_DEFINES("xy")

        S& x;
        S& y;

        constexpr component_names(S data[2]) noexcept : x(data[0]), y(data[1]) {}
    };

    template<class S>
    struct component_names<S, 3, true> : component_names<S, 2>
    {
        _DD_COMPONENT_DEFINES("xyz")

        S& z;

        constexpr component_names(S data[3]) noexcept : component_names<S, 2>(data), z(data[2]) {}
    };

    template<class S>
    struct component_names<S, 4, true> : component_names<S, 3>
    {
        _DD_COMPONENT_DEFINES("xyzw")

        S& w;

        constexpr component_names(S data[4]) noexcept : component_names<S, 3>(data), w(data[3]) {}
    };

    /*
     *  value
     *    an expression containing a single vector value
    */
    template<class S, size_t N>
    struct value : base<value<S, N>>, component_names<S, N>
    {
        static_assert(std::is_arithmetic_v<S> && (N > 1), "Invalid vector scalar_t or size");

        using names_t = component_names<S, N>;

        const static value zero;
        const static value identity;

        S data[N];
        
        /*
         *  ctors
        */

        // default ctor - initializes all components to zero
        constexpr value() noexcept : data{}, names_t(data) {}

        // value ctor (1) - initialize all components individually
        _DD_TEMPLATE_CONSTRAINT(...ARGS, (sizeof...(ARGS) == N && std::conjunction_v<std::is_convertible<ARGS, S>...>))
        constexpr value(const ARGS&... args) noexcept : data{ (S)args... }, names_t(data) {}

        // value ctor (2) - initialize all components to the same value
        constexpr explicit value(S v) noexcept : names_t(data)
        {
            for (size_t i = 0; i < N; i++)
                data[i] = v;
        }

        // copy ctor - copy values from another value expression
        //   required explicitly to ensure names_t is initialized properly
        constexpr value(const value& other) noexcept : names_t(data)
        {
            assign(other);
        }

        // conversion ctor - get values from a different type of vector
        _DD_TEMPLATE_CONSTRAINT(T, (traits::is_same_size_v<value, T> || traits::has_converter_v<value, T>))
        constexpr value(const T& v) : names_t(data)
        {
            if constexpr(traits::is_same_size_v<value, T>)
                assign(v);
            else
                assign(converter<value, T>::from(v));
        }
        
        /*
         *  operators
        */

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<value, E>))
        inline constexpr value& operator=(const E& e)
        {
            assign(e);
            return *this;
        }

        inline constexpr S operator[](size_t i) const noexcept
        {
            return data[i];
        }

        inline constexpr S& operator[](size_t i) noexcept
        {
            return data[i];
        }

        /*
         *  utility
        */

        _DD_TEMPLATE_CONSTRAINT(E, (traits::is_same_size_v<value, E>))
        constexpr value& assign(const E& e) noexcept
        {
            for (size_t i = 0; i < N; i++)
                data[i] = e[i];

            return *this;
        }

        std::string to_string(std::optional<std::string> name = {}) const noexcept
        {
            std::string out;

            if (name)
                out = *name + "  ";

            for (size_t i = 0; i < N; i++)
            {
                if constexpr(traits::has_named_components_v<value>)
                    out += names_t::_names_str[i];
                else
                    out += std::to_string(i);

                out += ": " + std::to_string(data[i]) + "  ";
            }
            return out;
        }
    };
}

/*
 *  vector aliases
*/

template<class SCALAR_T, size_t SIZE>
using vector = expr::value<SCALAR_T, SIZE>;

namespace types
{
    _DD_DEFINE_VECTOR_SIZE_ALIASES(2);
    _DD_DEFINE_VECTOR_SIZE_ALIASES(3);
    _DD_DEFINE_VECTOR_SIZE_ALIASES(4);
}

/*
 *  static member resolves
*/

template<class S, size_t N>
const vector<S, N> vector<S, N>::zero(0);

template<class S, size_t N>
const vector<S, N> vector<S, N>::identity(1);


_DD_NAMESPACE_CLOSE


namespace std
{
    /*
     *  serialization
    */
    
    _DD_TEMPLATE_CONSTRAINT(E, dd::traits::is_expr_v<E>)
    std::ostream& operator<<(std::ostream& stream, const E& e)
    {
        return stream << e.to_string();
    }

    /*
     *  utility
    */

    template<class S, size_t N>
    struct hash<dd::vector<S, N>>
    {
        size_t operator()(const dd::vector<S, N>& v) const
        {
            std::string_view byte_data = { (char*)v.data, N * sizeof(S) };
            std::hash<std::string_view> string_hash;
            return string_hash(byte_data);
        }
    };

    template<class S, size_t N>
    S* begin(dd::vector<S, N>& v)
    {
        return v.data;
    }

    template<class S, size_t N>
    S* end(dd::vector<S, N>& v)
    {
        return v.data + N;
    }

    template<class S, size_t N>
    const S* begin(const dd::vector<S, N>& v)
    {
        return v.data;
    }

    template<class S, size_t N>
    const S* end(const dd::vector<S, N>& v)
    {
        return v.data + N;
    }
}
