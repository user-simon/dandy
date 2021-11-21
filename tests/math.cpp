#include "common.h"

template<class T>
struct MathAll : testing::Test {};
TYPED_TEST_SUITE(MathAll, all_vectors);

TYPED_TEST(MathAll, Expressions)
{
    USING_TYPE_INFO

    auto a = random_vector<vector_t>();
    auto b = random_vector<vector_t>();
    auto c = random_vector<unique_scalar_vector_t>();

    auto d = *(1 + -(a + b) * c * 0.5);

    EXPECT_EQ(decltype(d)::size, size);
    testing::StaticAssertTypeEq<typename decltype(d)::scalar_t, double>();

    for (size_t i = 0; i < size; i++)
        EXPECT_EQ(1 + -(a[i] + b[i]) * c[i] * 0.5, d[i]);
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

TYPED_TEST(MathAll, LinearAlgebra)
{
    USING_TYPE_INFO
    
    auto a = vector_t::zero;
    auto b = vector_t::identity;

    EXPECT_EQ(a.distance2(b), size);
}

TEST(Math, Length)
{
    uint2d a(1, 1);
    int2d b(-5, -5);

    EXPECT_DOUBLE_EQ(a.length(), std::sqrt(2));
    EXPECT_DOUBLE_EQ(a.distance(b), uint2d(6, 6).length());
    EXPECT_DOUBLE_EQ(a.normalize().length(), 1);
    EXPECT_DOUBLE_EQ(b.set_length(100).length(), 100);
}

TEST(Math, LinearAlgebra)
{
    double2d a(1, 0);
    double2d b(0, 1);

    // angles
    EXPECT_DOUBLE_EQ(std::cos(a.angle()), 1);
    EXPECT_DOUBLE_EQ(std::sin(a.angle()), 0);
    EXPECT_DOUBLE_EQ(a.delta_angle(b), b.angle());

    // dot p
    EXPECT_DOUBLE_EQ(a.dot(b), 0);

    double2d c(1.2, 3.4);
    double2d d(5.6, 7.8);
    EXPECT_DOUBLE_EQ(c.dot(d), 1.2 * 5.6 + 3.4 * 7.8);

    // cross p
    int3d e(1, 2, 3);
    int3d f(4, 5, 6);

    EXPECT_EQ(e.cross(f), int3d(-3, 6, -3));

    // from angle
    EXPECT_EQ(double2d::from_angle(a.angle()), a);
}
