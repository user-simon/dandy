#include "common.h"
#include <array>

template<class T>
struct ConversionsAll : testing::Test {};
TYPED_TEST_SUITE(ConversionsAll, all_vectors);

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

TYPED_TEST(ConversionsAll, Array_conversion)
{
    USING_TYPE_INFO

    vector_t vec = random_vector<vector_t>();

    // cast result to avoid type-promotion issues when multiplying a char
    std::array<scalar_t, size> arr = (2 * vec).scalar_cast<scalar_t>();

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(arr[i], (scalar_t)(2 * vec[i]));

    vec = arr;

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(vec[i], arr[i]);
}
