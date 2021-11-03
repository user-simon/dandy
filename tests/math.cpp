#include "common.h"

template<class T>
struct Math : testing::Test {};
TYPED_TEST_SUITE(Math, vector_types);

TYPED_TEST(Math, Algebra)
{
    EXPECT_TRUE(true);
}
