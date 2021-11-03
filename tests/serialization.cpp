#include "common.h"

template<class T>
struct Serialization : testing::Test {};
TYPED_TEST_SUITE(Serialization, vector_types);

TYPED_TEST(Serialization, To_string)
{
    EXPECT_TRUE(true);
}
