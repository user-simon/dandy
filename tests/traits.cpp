#include "common.h"

template<class T>
struct Traits : testing::Test {};
TYPED_TEST_SUITE(Traits, vector_types);

TYPED_TEST(Traits, Type_requirements)
{
    USING_TYPE_INFO
    
    EXPECT_TRUE(std::is_default_constructible_v<vector_t>);
    EXPECT_TRUE(std::is_move_constructible_v<vector_t>);
    EXPECT_TRUE(std::is_copy_constructible_v<vector_t>);
    EXPECT_TRUE(std::is_move_assignable_v<vector_t>);
    EXPECT_TRUE(std::is_copy_assignable_v<vector_t>);
    EXPECT_TRUE(std::is_destructible_v<vector_t>);
    EXPECT_TRUE(std::is_swappable_v<vector_t>);
}

TYPED_TEST(Traits, Categories)
{
    USING_TYPE_INFO

    EXPECT_TRUE(traits::is_value_v<vector_t>);
    EXPECT_FALSE(traits::is_operation_v<vector_t>);

    EXPECT_FALSE(traits::is_value_v<operation_t>);
    EXPECT_TRUE(traits::is_operation_v<operation_t>);

    EXPECT_TRUE(traits::is_expression_v<vector_t>);
    EXPECT_TRUE(traits::is_expression_v<operation_t>);
}

TYPED_TEST(Traits, Type_relationships)
{
    USING_TYPE_INFO

    EXPECT_TRUE((traits::is_same_size_v<vector_t, operation_t>));
    EXPECT_FALSE((traits::is_same_size_v<vector_t, unique_size_vector_t>));

    EXPECT_TRUE((traits::is_valid_operation_v<vector_t, vector_t, true>));
    EXPECT_FALSE((traits::is_valid_operation_v<operation_t, unique_size_vector_t, true>));

    EXPECT_FALSE((traits::is_valid_operation_v<scalar_t, vector_t, true>));
    EXPECT_FALSE((traits::is_valid_operation_v<scalar_t, operation_t, true>));
}
