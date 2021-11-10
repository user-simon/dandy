#include "common.h"
#include <sstream>

TEST(Serialization, ToString)
{
    int2d a;

    EXPECT_EQ(a.to_string(), "(0, 0)");

    auto b = make_index_vector_v<dd::vector<uint32_t, 5>>;

    EXPECT_EQ(b.to_string("vector"), "vector(0, 1, 2, 3, 4)");
}

TEST(Serialization, Stream)
{
    std::stringstream stream;

    int2d a;
    stream << a;

    EXPECT_EQ(stream.str(), "(0, 0)");
    stream.str(std::string()); // clear stream

    a = make_index_vector_v<int2d>;
    stream << -(a + a);

    EXPECT_EQ(stream.str(), "(0, -2)");
}
