#include "common.h"

template<class T>
struct TraitsAll : testing::Test {};
TYPED_TEST_SUITE(TraitsAll, all_vectors);

TYPED_TEST(TraitsAll, Type_requirements)
{
    USING_TYPE_INFO

    EXPECT_TRUE(std::is_default_constructible_v<vector_t>);
    EXPECT_TRUE(std::is_move_constructible_v<vector_t>);
    EXPECT_TRUE(std::is_copy_constructible_v<vector_t>);
    EXPECT_TRUE(std::is_move_assignable_v<vector_t>);
    EXPECT_TRUE(std::is_copy_assignable_v<vector_t>);
    EXPECT_TRUE(std::is_destructible_v<vector_t>);
    EXPECT_TRUE(std::is_swappable_v<vector_t>);

    // make sure the vector constructors are compile-time evaluatable

    constexpr static vector_t a = make_index_vector_v<vector_t>;
    constexpr static vector_t b = a;
    constexpr static vector_t c = a + b;

    for (size_t i = 0; i < size; i++)
    {
        EXPECT_EQ(a[i], i);
        EXPECT_EQ(b[i], i);
        EXPECT_EQ(c[i], 2 * i);
    }
}

TYPED_TEST(TraitsAll, Categories)
{
    USING_TYPE_INFO

    EXPECT_TRUE(traits::is_value_v<vector_t>);
    EXPECT_FALSE(traits::is_operation_v<vector_t>);

    EXPECT_FALSE(traits::is_value_v<operation_t>);
    EXPECT_TRUE(traits::is_operation_v<operation_t>);

    EXPECT_TRUE(traits::is_expression_v<vector_t>);
    EXPECT_TRUE(traits::is_expression_v<operation_t>);
}

TYPED_TEST(TraitsAll, Type_relationships)
{
    USING_TYPE_INFO

    EXPECT_TRUE((traits::is_same_size_v<vector_t, operation_t>));
    EXPECT_FALSE((traits::is_same_size_v<vector_t, unique_size_vector_t>));

    EXPECT_TRUE((traits::is_valid_operation_v<vector_t, vector_t, true>));
    EXPECT_FALSE((traits::is_valid_operation_v<operation_t, unique_size_vector_t, true>));

    EXPECT_FALSE((traits::is_valid_operation_v<scalar_t, vector_t, true>));
    EXPECT_FALSE((traits::is_valid_operation_v<scalar_t, operation_t, true>));
}

TYPED_TEST(TraitsAll, Prefabs)
{
    USING_TYPE_INFO

    for (scalar_t s : vector_t::zero)
        EXPECT_EQ(s, 0);

    for (scalar_t s : vector_t::identity)
        EXPECT_EQ(s, 1);
}
