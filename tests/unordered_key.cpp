#include "common.h"
#include <unordered_set>

TEST(unordered, set)
{
    std::unordered_set<float2d> vectors;

    for (uint32_t i = 0; i < 10; i++)
        vectors.insert({ i / 3, i / 5 });

    for (uint32_t i = 0; i < 10; i++)
        EXPECT_EQ(vectors.count({ i / 3, i / 5 }), 1);
}
