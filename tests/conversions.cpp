#include "common.h"
#include <array>

template<class T>
struct Conversions : testing::Test {};
TYPED_TEST_SUITE(Conversions, vector_types);

template<class T, size_t N>
struct converter<vector<T, N>, std::array<T, N>>
{
    using dd_vector = vector<T, N>;
    using array = std::array<T, N>;

    constexpr static void convert(const dd_vector& from, array& to)
    {
        for (size_t i = 0; i < N; i++)
            to[i] = from[i];
    }

    constexpr static void convert(const array& from, dd_vector& to)
    {
        for (size_t i = 0; i < N; i++)
            to[i] = from[i];
    }
};

TYPED_TEST(Conversions, Array_conversion)
{
    USING_TYPE_INFO

    vector_t vec = random_vector<vector_t>();
    std::array<scalar_t, size> arr = vec;

    // expect array to be equal to vector and double all indices
    for (size_t i = 0; i < size; i++)
    {
        EXPECT_EQ(arr[i], vec[i]);
        arr[i] *= 2;
    }

    vec = arr;

    for (size_t i = 0; i < size; i++)
    {
        EXPECT_EQ(vec[i], arr[i]);
    }
}
