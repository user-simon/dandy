#include "common.h"

template<class T>
struct ConstructorsAll : testing::Test {};
TYPED_TEST_SUITE(ConstructorsAll, all_vectors);

TYPED_TEST(ConstructorsAll, Default)
{
    USING_TYPE_INFO

    vector_t v;

    for (size_t i = 0; i < vector_t::size; i++)
        EXPECT_EQ(v.at(i), 0);
}

TYPED_TEST(ConstructorsAll, Copy)
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

TYPED_TEST(ConstructorsAll, Value)
{
    USING_TYPE_INFO

    vector_t v = make_index_vector_v<vector_t>;

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(v.at(i), i);
}

TYPED_TEST(ConstructorsAll, Repeated)
{
    USING_TYPE_INFO

    scalar_t s = random_scalar<scalar_t>();
    vector_t v = vector_t(s);

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(v.at(i), s);
}

TEST(Constructors, ComponentNames)
{
    int2d a { 1, 2 };
    int2d b { 3, 4 };

    // make sure components get bound to correct data
    EXPECT_EQ(a.x, 1);
    EXPECT_EQ(a.y, 2);

    a = b;
    b = { 0, 0 };

    // make sure components don't get rebound after assignment
    EXPECT_EQ(a.x, 3);
    EXPECT_EQ(a.y, 4);
}
