#include "common.h"

template<class T>
struct MathAll : testing::Test {};
TYPED_TEST_SUITE(MathAll, all_vectors);

template<class T>
struct MathSigned : testing::Test {};
TYPED_TEST_SUITE(MathSigned, signed_vectors);

template<class T>
struct MathFloating : testing::Test {};
TYPED_TEST_SUITE(MathFloating, floating_vectors);

TYPED_TEST(MathAll, Expressions)
{
    USING_TYPE_INFO

    auto a = random_vector<vector_t>();
    auto b = random_vector<vector_t>();
    auto c = random_vector<unique_scalar_vector_t>();

    auto d = *(1 + -(a + b) * c * 0.5);

    EXPECT_EQ(decltype(d)::size, size);
    testing::StaticAssertTypeEq<decltype(d)::scalar_t, double>();

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(1 - (a[i] + b[i]) * c[i] * 0.5, d[i]);
}

TYPED_TEST(MathAll, Util)
{
    USING_TYPE_INFO

    vector_t a;

    EXPECT_FALSE(a.nonzero());
    EXPECT_FALSE(a.contains(1));
    EXPECT_EQ(a.sum(), 0);
    EXPECT_EQ(a.product(), 0);
    
    a = make_index_vector_v<vector_t>;

    EXPECT_TRUE(a.nonzero());
    EXPECT_TRUE(a.contains(1));
    EXPECT_EQ(a.sum(), (size - 1) * 0.5 * size);
    EXPECT_EQ(a.product(), 0);

    a[0]++;

    EXPECT_EQ(a.sum(), 1 + (size - 1) * 0.5 * size);
    EXPECT_NE(a.product(), 0);
}

TYPED_TEST(MathSigned, Algebra)
{
    USING_TYPE_INFO

    vector_t a = random_vector<vector_t>();
    a[0] = -1;

    vector_t b = a.abs();

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(b[i], std::abs(a[i]));
}

TYPED_TEST(MathFloating, Algebra)
{
    
}

TYPED_TEST(MathAll, LinearAlgebra)
{
    USING_TYPE_INFO
    
    auto a = vector_t::zero;
    auto b = vector_t::identity;

    EXPECT_EQ(a.distance2(b), size);
}
