#include "common.h"

template<class T>
struct Constructors : testing::Test {};
TYPED_TEST_SUITE(Constructors, vector_types);

TYPED_TEST(Constructors, Default)
{
    USING_TYPE_INFO

    vector_t v;

    for (size_t i = 0; i < vector_t::size; i++)
        EXPECT_EQ(v.at(i), 0);
}

TYPED_TEST(Constructors, Copy)
{
    USING_TYPE_INFO
    
    vector_t a = random_vector<vector_t>();
    vector_t b = a;
    unique_scalar_vector_t c = b;

    for (size_t i = 0; i < size; i++)
    {
        EXPECT_EQ(a.at(i), b.at(i));
        EXPECT_EQ((unique_scalar_t)b.at(i), c.at(i));
    }
}

TYPED_TEST(Constructors, Value)
{
    USING_TYPE_INFO

    vector_t v = make_index_vector_v<vector_t>;

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(v.at(i), i);
}

TYPED_TEST(Constructors, Repeated)
{
    USING_TYPE_INFO

    scalar_t s = random_scalar<scalar_t>();
    vector_t v = vector_t(s);

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(v.at(i), s);
}
