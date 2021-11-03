#include "common.h"

template<class T>
struct Expressions : testing::Test {};
TYPED_TEST_SUITE(Expressions, vector_types);

TYPED_TEST(Expressions, Equality)
{
    USING_TYPE_INFO

    vector_t a = random_vector<vector_t>();
    vector_t b = a;

    EXPECT_EQ(a, b);

    a[0]++;

    EXPECT_NE(a, b);
}

TYPED_TEST(Expressions, Addition)
{
    EXPECT_TRUE(true);
}
