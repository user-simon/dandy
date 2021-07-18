#pragma once
#include <functional>
#include <type_traits>
#include <optional>
#include <ostream>
#include <algorithm>
#include <string>

/*
 *  utility macros
*/

// for explicit return types
#define DD_EXPR_T auto

// loop over component of expression
#define DD_LOOP(i) for (size_t i = 0; i < size; i++)

// wrapper to expr::apply to avoid overload ambiguity
#define DD_APPLY_FN(fn) apply([](const scalar_t& v) { return fn(v); })

/*
 *  macros for defining templates
*/

#define DD_TEMPLATE_EXPR(t)       template<class t, class = std::enable_if_t<dd_traits::is_expr_v<t>>>
#define DD_TEMPLATE_VALID_EXPR(t) template<class t, class = std::enable_if_t<dd_traits::is_same_size_expr_v<CHILD, t>>>
#define DD_TEMPLATE_SCALAR(t)     template<class t, class = std::enable_if_t<std::is_arithmetic_v<t>>>

/*
 *  macros standard class definitions
*/

// defines trait types from expr_traits<t>
#define DD_TRAIT_DEFS(t)                         \
    using traits   = dd_traits::expr_traits<t>;  \
    using scalar_t = typename traits::scalar_t;  \
    using value_t  = typename traits::value_t;   \
    constexpr static size_t size = traits::size;

// defines utility functions for base expression types
#define DD_BASE_EXPR_DEFS                        \
private:                                         \
    DD_TRAIT_DEFS(CHILD);                        \
                                                 \
    constexpr inline const CHILD& _this() const  \
    {                                            \
        return static_cast<const CHILD&>(*this); \
    }                                            \
                                                 \
    constexpr scalar_t _value(size_t i) const    \
    {                                            \
        return _this()[i];                       \
    }                                            \
public:

// standard vector specialization definitions
#define DD_NAME_DEFS(n, names)                                                        \
    private:                                                                          \
        using base = component_names<T, n - 1, true>;                                 \
    public:                                                                           \
       constexpr static char str_component_names[] = names;                           \
       constexpr component_names& operator=(const component_names&) { return *this; }

/*
 *  macros for defining operators
*/

// defines a binary expression operator
#define DD_DEF_BIN_OP(op, strict)                                                                \
    template<class L, class R, class = std::enable_if_t<dd_traits::is_valid_op_v<L, R, strict>>> \
    constexpr auto operator op(const L& l, const R& r)                                           \
    {                                                                                            \
        return dd_impl::op_expr                                                                  \
        {                                                                                        \
            [](const auto& l, const auto& r) { return l op r; },                                 \
            l, r                                                                                 \
        };                                                                                       \
    }                                                                                            \
                                                                                                 \
    template<class L, class R, class = std::enable_if_t<dd_traits::is_valid_op_v<L, R, true>>>   \
    constexpr L& operator op##=(L& l, const R& r)                                                \
    {                                                                                            \
        return l = l op r;                                                                       \
    }

// defines a unary expression operator
#define DD_DEF_UN_OP(op)                                                 \
    template<class T, class = std::enable_if_t<dd_traits::is_expr_v<T>>> \
    constexpr auto operator op (const T& val)                            \
    {                                                                    \
        return dd_impl::op_expr                                          \
        {                                                                \
            [](const auto& v) { return op(v); },                         \
            val                                                          \
        };                                                               \
    }

/*
 *  macros for defining vector type aliases
*/

#define DD_TYPE_DEF(t, t_name, n) using t_name##n##d = dandy_v<t, n>;
#define DD_TYPE_DEFS(n)                    \
    DD_TYPE_DEF(bool,          binary, n); \
    DD_TYPE_DEF(char,          char,   n); \
    DD_TYPE_DEF(unsigned char, uchar,  n); \
    DD_TYPE_DEF(int,           int,    n); \
    DD_TYPE_DEF(unsigned int,  uint,   n); \
    DD_TYPE_DEF(float,         float,  n); \
    DD_TYPE_DEF(double,        double, n);

/*
 *  arithmetic types
*/

#ifndef DD_NO_SCALAR_ALIASES
using uint = unsigned int;
using ulong = unsigned long;
using uchar = unsigned char;
using byte = uchar;
#endif

/*
 *  global settings
*/

#ifdef DD_NO_COMPONENT_NAMES
constexpr bool DD_ENABLE_NAMES = false;
#else
constexpr bool DD_ENABLE_NAMES = true;
#endif

/*
 *  forward declares
 *    used by definitions of traits and operator overloads
*/

namespace dd_impl
{
    template<class>
    struct expr;

    template<class, class...>
    struct op_expr;

    template<class, size_t, bool>
    struct component_names;

    template<class, size_t, bool>
    struct val_expr;
}

/*
 *  expression type traits
*/

namespace dd_traits
{
    /*
     *  is expression
     *    true if T inherits from expr<T>
    */
    template<class T>
    struct is_expr : std::is_base_of<dd_impl::expr<T>, T> {};

    template<class T>
    constexpr bool is_expr_v = is_expr<T>::value;

    /*
     *  is same size expression
     *    true if both types are expressions of the same size
    */
    template<class L, class R, class = void>
    struct is_same_size_expr : is_same_size_expr<L, R, std::bool_constant<is_expr_v<L>&& is_expr_v<R>>> {};

    template<class L, class R>
    struct is_same_size_expr<L, R, std::false_type> : std::false_type {};

    template<class L, class R>
    struct is_same_size_expr<L, R, std::true_type> : std::bool_constant<L::size == R::size> {};

    template<class L, class R>
    constexpr bool is_same_size_expr_v = is_same_size_expr<L, R>::value;

    /*
     *  is valid operation
     *    true if the two types form a valid expression operation.
     *    STRICT_ORDERING = true requries the expression to appear
     *    first in the operation
    */
    template<class L, class R, bool STRICT_ORDERING>
    struct is_valid_op
        : std::bool_constant<(is_same_size_expr_v<L, R>) ||
                             (is_expr_v<L> && std::is_arithmetic_v<R>) ||
                             (std::is_arithmetic_v<L> && is_expr_v<R> && !STRICT_ORDERING)> {};

    template<class L, class R, bool STRICT_ORDERING>
    constexpr bool is_valid_op_v = is_valid_op<L, R, STRICT_ORDERING>::value;

    /*
     *  expression traits
     *    type traits for expression types
    */
    template<class S> // scalar
    struct expr_traits
    {
        static_assert(std::is_arithmetic_v<S>);

        constexpr static size_t size = 1;
        using scalar_t = S;
    };

    template<class OP, class... ARGS> // op_expr
    struct expr_traits<dd_impl::op_expr<OP, ARGS...>>
    {
        constexpr static size_t size = std::max({ expr_traits<ARGS>::size... });
        using scalar_t = std::invoke_result_t<OP, typename expr_traits<ARGS>::scalar_t...>;
        using value_t = dd_impl::val_expr<scalar_t, size, DD_ENABLE_NAMES>;
    };

    template<class T, size_t N, bool ENABLE_NAMES> // val_expr
    struct expr_traits<dd_impl::val_expr<T, N, ENABLE_NAMES>>
    {
    private:
        template<class T, class = void>
        constexpr static bool _has_named_components = false;

        template<class T>
        constexpr static bool _has_named_components<T, std::void_t<decltype(T::str_component_names)>> = true;

    public:
        constexpr static size_t size = N;
        using scalar_t = T;
        using value_t = dd_impl::val_expr<T, N, ENABLE_NAMES>;
        using names_t = dd_impl::component_names<T, N, ENABLE_NAMES>;
        constexpr static bool has_named_components = _has_named_components<names_t>;
    };
}

/*
 *  operator overloads
*/

DD_DEF_BIN_OP(+,  false);
DD_DEF_BIN_OP(-,  true);
DD_DEF_BIN_OP(*,  false);
DD_DEF_BIN_OP(/,  true);
DD_DEF_BIN_OP(%,  true);
DD_DEF_BIN_OP(&,  false);
DD_DEF_BIN_OP(|,  false);
DD_DEF_BIN_OP(^,  false);
DD_DEF_BIN_OP(>>, true);
DD_DEF_BIN_OP(<<, true);

// TODO: specialization for dividing by scalar

DD_DEF_UN_OP(-);
DD_DEF_UN_OP(+);
DD_DEF_UN_OP(~);

/*
 *  expression implementations
*/

namespace dd_impl
{
    /*
     *  expression specialization
     *    for size specific specializations
    */
    template<class CHILD, size_t SIZE>
    struct expr_spec {};

    template<class CHILD>
    struct expr_spec<CHILD, 2>
    {
        DD_BASE_EXPR_DEFS;

        double angle() const
        {
            return std::atan2((double)_value(1), _value(0));
        }

        static value_t from_angle(double angle)
        {
            return value_t
            {
                std::cos(angle),
                std::sin(angle)
            };
        }

#ifdef DD_SFML
        // conversion operator to sf::Vector2
        DD_TEMPLATE_SCALAR(S)
        operator sf::Vector2<S>() const
        {
            return sf::Vector2<S>(_value(0), _value(1));
        }
#endif
    };

    template<class CHILD>
    struct expr_spec<CHILD, 3>
    {
        DD_BASE_EXPR_DEFS;

        DD_TEMPLATE_VALID_EXPR(E)
        constexpr value_t cross(const E& e) const
        {
            return value_t
            {
                _value(1) * e[2] - _value(2) * e[1],
                _value(2) * e[0] - _value(0) * e[2],
                _value(0) * e[1] - _value(1) * e[0]
            };
        }

#ifdef DD_SFML
        // conversion operator to sf::Vector3
        DD_TEMPLATE_SCALAR(S)
        operator sf::Vector3<S>() const
        {
            return sf::Vector3<S>(_value(0), _value(1), _value(2));
        }
#endif
    };

    /*
     *  expression
     *    base class for all expressions. provides in-class functionality
     *    for vectors and op-expressions
    */
    template<class CHILD>
    struct expr : expr_spec<CHILD, dd_traits::expr_traits<CHILD>::size>
    {
        DD_BASE_EXPR_DEFS;

        /*
         *  operators
        */

        DD_TEMPLATE_VALID_EXPR(E)
        constexpr bool operator==(const E& e) const
        {
            DD_LOOP(i)
            {
                if (_value(i) != e[i])
                    return false;
            }
            return true;
        }

        DD_TEMPLATE_VALID_EXPR(E)
        constexpr bool operator!=(const E& e) const
        {
            return !(_this() == e);
        }

        /*
         *  math functions
        */

        constexpr scalar_t sum() const
        {
            scalar_t out = 0;

            DD_LOOP(i)
            {
                out += _value(i);
            }
            return out;
        }

        constexpr scalar_t product() const
        {
            scalar_t out = 1;

            DD_LOOP(i)
            {
                out *= _value(i);
            }
            return out;
        }

        constexpr bool nonzero() const
        {
            DD_LOOP(i)
            {
                if (_value(i))
                    return true;
            }
            return false;
        }

        DD_TEMPLATE_VALID_EXPR(E)
        constexpr scalar_t dot(const E& e) const
        {
            scalar_t out = 0;

            DD_LOOP(i)
            {
                out += _value(i) * e[i];
            }
            return out;
        }

        constexpr scalar_t length2() const
        {
            return dot(_this());
        }

        double length() const
        {
            return std::sqrt((double)length2());
        }

        DD_TEMPLATE_VALID_EXPR(E)
        constexpr scalar_t distance2(const E& e) const
        {
            return (_this() - e).length2();
        }

        DD_TEMPLATE_VALID_EXPR(E)
        constexpr double distance(const E& e) const
        {
            return std::sqrt((double)distance2(e));
        }

        DD_EXPR_T normalize() const
        {
            return _this() / length();
        }

        DD_TEMPLATE_SCALAR(S)
        DD_EXPR_T set_length(S s) const
        {
            return normalize() * s;
        }

        DD_TEMPLATE_VALID_EXPR(E)
        double delta_angle(const E& e) const
        {
            return std::acos(dot(e) / std::sqrt((double)length2() * e.length2()));
        }

        template<class FN>
        constexpr inline DD_EXPR_T apply(const FN& fn) const
        {
            return op_expr{ fn, std::move(_this()) };
        }

        DD_EXPR_T abs() const
        {
            return DD_APPLY_FN(std::abs);
        }

        DD_EXPR_T round() const
        {
            return DD_APPLY_FN(std::round);
        }

        DD_EXPR_T floor() const
        {
            return DD_APPLY_FN(std::floor);
        }

        DD_EXPR_T ceil() const
        {
            return DD_APPLY_FN(std::ceil);
        }

        DD_TEMPLATE_SCALAR(S)
        DD_EXPR_T scalar_cast() const
        {
            return DD_APPLY_FN(S);
        }
    };
    
    /*
     *  operation expression
     *    expressions involving an operation and at least one other
     *    expression. each component is evaluated in operator[]
    */
    template<class OP, class... ARGS>
    struct op_expr : expr<op_expr<OP, ARGS...>>
    {
        DD_TRAIT_DEFS(op_expr);

    private:
        const std::tuple<ARGS...> _args;
        const OP& _op;
        
        template<class T>
        constexpr static inline scalar_t _get_index(const T& value, size_t i)
        {
            if constexpr(dd_traits::is_expr_v<T>)
                return value[i];
            else
                return value;
        }
        
    public:
        constexpr op_expr(const OP& op, ARGS... args)
            : _op(op), _args(args...) {}

        constexpr inline scalar_t operator[](size_t i) const
        {
            auto eval_component = [&](const ARGS&... args)
            {
                return _op(_get_index(args, i)...);
            };
            return std::apply(eval_component, _args);
        }

        // forces evaluation
        constexpr value_t evaluate() const
        {
            return value_t(*this);
        }

        // forces evaluation
        constexpr value_t operator*() const
        {
            return evaluate();
        }

        // serializes result of expression. forces evaluation
        std::string to_string(std::optional<std::string> name = {}) const
        {
            return evaluate().to_string(name);
        }
    };

    /*
     *  component names
     *    provides named components for val_expr. for performance
     *    reasons, can be disabled with ENABLE = false
    */
    template<class T, size_t N, bool ENABLE = true>
    struct component_names
    {
        constexpr component_names(T*) {}
    };

    template<class T>
    struct component_names<T, 2, true>
    {
        DD_NAME_DEFS(2, "xy");

        T& x;
        T& y;

        constexpr component_names(T data[2]) : x(data[0]), y(data[1]) {}
    };

    template<class T>
    struct component_names<T, 3, true> : component_names<T, 2>
    {
        DD_NAME_DEFS(3, "xyz");

        T& z;

        constexpr component_names(T data[3]) : base(data), z(data[2]) {}
    };

    template<class T>
    struct component_names<T, 4, true> : component_names<T, 3>
    {
        DD_NAME_DEFS(4, "xyzw");

        T& w;

        constexpr component_names(T data[4]) : base(data), w(data[3]) {}
    };

    /*
     *  value expression
     *    an expression containing a single vector value
    */
    template<class T, size_t N, bool ENABLE_NAMES>
    struct val_expr : expr<val_expr<T, N, ENABLE_NAMES>>, component_names<T, N, ENABLE_NAMES>
    {
        static_assert(std::is_arithmetic_v<T> && (N > 1), "Invalid scalar_t or size");

        DD_TRAIT_DEFS(val_expr);

    private:
        using names_t = typename traits::names_t;

    public:
        const static val_expr zero;
        T data[N];

        /*
         *  ctors
        */

        // default ctor - inits all components to 0
        constexpr val_expr() : data{ 0 }, names_t(data) {}

        // value ctor - specify a value for each component
        template<class... ARGS, class = std::enable_if_t<sizeof...(ARGS) == N && std::conjunction_v<std::is_arithmetic<ARGS>...>>>
        constexpr val_expr(ARGS... args) : data{ (T)args... }, names_t(data) {}

        // copy ctor - copy values from an instance of the same expr
        //   required explicitly to make sure names_t is initialized correctly
        constexpr val_expr(const val_expr& e) : names_t(data)
        {
            evaluate(e);
        }

        // expr ctor - evaluate or copy values from a different expression
        template<class E, class = std::enable_if_t<dd_traits::is_same_size_expr_v<val_expr, E>>>
        constexpr val_expr(const E& e) : names_t(data)
        {
            evaluate(e);
        }

        /*
         *  operators
        */

        constexpr T operator[](size_t i) const
        {
            return data[i];
        }

        constexpr T& operator[](size_t i)
        {
            return data[i];
        }

        /*
         *  util
        */

        DD_TEMPLATE_EXPR(E)
        val_expr& evaluate(const E& e)
        {
            DD_LOOP(i)
            {
                data[i] = (T)e[i];
            }
            return *this;
        }

        std::string to_string(std::optional<std::string> name = {}) const
        {
            std::string out;

            if (name)
                out = *name + "  ";

            DD_LOOP(i)
            {
                if constexpr(traits::has_named_components)
                    out += names_t::str_component_names[i];
                else
                    out += std::to_string(i);

                out += ": " + std::to_string(data[i]) + "  ";
            }
            return out;
        }
    };
}

/*
 *  interface aliases for the vector
*/

template<class T, size_t N, bool ENABLE_NAMES = DD_ENABLE_NAMES>
using dandy_v = dd_impl::val_expr<T, N, ENABLE_NAMES>;

#ifndef DD_NO_VECTOR_ALIASES
DD_TYPE_DEFS(2);
DD_TYPE_DEFS(3);
DD_TYPE_DEFS(4);
#endif

/*
 *  static member resolves
*/

template<class T, size_t N, bool ENABLE_NAMES>
const dandy_v<T, N, ENABLE_NAMES> dandy_v<T, N, ENABLE_NAMES>::zero;

/*
 *  stl overloads and specializations
*/
namespace std
{
    DD_TEMPLATE_EXPR(E)
    string to_string(const E& e)
    {
        return e.to_string();
    }

    DD_TEMPLATE_EXPR(E)
    ostream& operator<<(ostream& stream, const E& e)
    {
        return stream << e.to_string();
    }

    template<class T, size_t N, bool ENABLE_NAMES>
    struct hash<dandy_v<T, N, ENABLE_NAMES>>
    {
        size_t operator()(const dandy_v<T, N, ENABLE_NAMES>& e) const
        {
            std::string_view byte_data = { (char*)e.data, N * sizeof(T) };
            std::hash<std::string_view> string_hash;
            return string_hash(byte_data);
        }
    };
}
