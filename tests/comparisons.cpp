#include "common.h"

template<class T>
struct ComparisonsAll : testing::Test {};
TYPED_TEST_SUITE(ComparisonsAll, all_vectors);

TYPED_TEST(ComparisonsAll, Equal)
{
    USING_TYPE_INFO

    vector_t a = make_index_vector_v<vector_t>;
    vector_t b;

    for (size_t i = 0; i < size; i++)
        b[i] = a[i];

    EXPECT_EQ(a, b);

    a[0]++;

    EXPECT_NE(a, b);
}

TYPED_TEST(ComparisonsAll, LessThan)
{
    USING_TYPE_INFO

    vector_t a = make_index_vector_v<vector_t>;
    vector_t b = make_index_vector_v<vector_t>;

    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a < b);

    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(a <= b);

    a[0]++;

    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a < b);

    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a <= b);

    for (size_t i = 1; i < size; i++)
        a[i]++;

    EXPECT_TRUE(a > b);
    EXPECT_FALSE(a < b);

    EXPECT_TRUE(a >= b);
    EXPECT_FALSE(a <= b);
}
