#pragma once
#include <dandy/dandy.h>
#include <gtest/gtest.h>
#include <random>
using namespace dd;

template<size_t Size, class T, class U = T>
using operation_t = decltype(std::declval<vector<T, Size>>() + std::declval<vector<T, Size>>());

template<class Scalar>
inline Scalar random_scalar()
{
    //static std::random_device _device;
    static std::mt19937 rng = std::mt19937(std::random_device()());
    static std::uniform_int_distribution<std::mt19937::result_type> dist;
    auto scalar = dist(rng);

    return *(Scalar*)&scalar;
}

template<class Vector>
inline Vector random_vector()
{
    Vector out;

    for (size_t i = 0; i < Vector::size; i++)
        out[i] = random_scalar<typename Vector::scalar_t>();
    return out;
}

using vector_types = testing::Types<
    vector<int32_t,  2>,
    vector<uint32_t, 2>,
    vector<double,   2>,
    
    vector<uint32_t, 3>,
    vector<uint32_t, 4>,
    vector<uint32_t, 5>
>;

template<class Vector, class Indices = std::make_index_sequence<Vector::size>>
struct make_index_vector;

template<class Vector, size_t... Indices>
struct make_index_vector<Vector, std::index_sequence<Indices...>>
{
    constexpr static Vector value { Indices... };
};

template<class Vector>
constexpr inline const Vector& make_index_vector_v = make_index_vector<Vector>::value;

#define USING_TYPE_INFO                                                                          \
    using vector_t = TypeParam;                                                                  \
    using scalar_t = typename vector_t::scalar_t;                                                \
    constexpr static size_t size = vector_t::size;                                               \
    using operation_t = operation_t<size, scalar_t>;                                             \
                                                                                                 \
    constexpr static size_t unique_size = size == 2 ? 3 : 2;                                     \
    using unique_scalar_t = std::conditional_t<std::is_same_v<scalar_t, double>, float, double>; \
                                                                                                 \
    using unique_size_vector_t   = vector<scalar_t,        unique_size>;                         \
    using unique_scalar_vector_t = vector<unique_scalar_t, size>;                                \
    using unique_vector_t        = vector<unique_scalar_t, unique_size>;