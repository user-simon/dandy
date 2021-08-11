#include <gtest/gtest.h>
#include "dandy.h"

using namespace dd::types;

/*
 *  boiler-plate to convert vectors. used by test conversions
*/

struct some_vector
{
    int x, y;
};

template<>
struct dd::converter<int2d, some_vector>
{
    // some_vector -> dandy
    static int2d from(const some_vector& v)
    {
        return int2d{ v.x, v.y };
    }

    // dandy -> some_vector
    static some_vector from(const int2d& v)
    {
        return some_vector{ v.x, v.y };
    }
};

/*
 *  tests
*/

TEST(requirements, requirements)
{
    // size and type of vector shouldn't matter so only int2d is tested
    
    EXPECT_TRUE(std::is_default_constructible_v<int2d>);
    EXPECT_TRUE(std::is_move_constructible_v<int2d>);
    EXPECT_TRUE(std::is_copy_constructible_v<int2d>);
    EXPECT_TRUE(std::is_move_assignable_v<int2d>);
    EXPECT_TRUE(std::is_copy_assignable_v<int2d>);
    EXPECT_TRUE(std::is_destructible_v<int2d>);
    EXPECT_TRUE(std::is_swappable_v<int2d>);
}

TEST(structure, components)
{
    // check that all component names exist
    
    int2d a;
    a.x, a.y;

    int3d b;
    b.x, b.y, b.z;

    int4d c;
    c.x, c.y, c.z, c.w;

    // check that they correctly correspond to the data values

    int4d d;
    EXPECT_TRUE(d.x == d[0] && d.y == d[1] && d.z == d[2] && d.w == d[3]);
    d.x = 1; d.y = 2; d.z = 3; d.w = 4;
    EXPECT_TRUE(d.x == d[0] && d.y == d[1] && d.z == d[2] && d.w == d[3]);
}

TEST(constructors, default)
{
    int4d a;
    EXPECT_TRUE(a.x == 0 && a.y == 0 && a.z == 0 && a.w == 0);
}

TEST(constructors, value)
{
    int4d a(1, 2, 3, 4);
    EXPECT_TRUE(a.x == 1 && a.y == 2 && a.z == 3 && a.w == 4);
}

TEST(constructors, copy)
{
    int2d a(1, 2);
    int2d b = a;

    EXPECT_TRUE(a[0] == b[0] && a[1] == b[1]);
    EXPECT_TRUE(a.x == b.x && a.y == b.y);

    // ensure component_names is initialized properly

    a.x = 10;
    EXPECT_TRUE(a.x != b.x);
}

TEST(operators, comparison)
{
    int2d a;
    a.x = 1;
    a.y = 2;

    int2d b;
    b.x = 3;
    b.y = 4;

    EXPECT_FALSE(a == b);
    a = b;
    EXPECT_TRUE(a == b);
}

TEST(expressions, expressions)
{
    double2d a(1.2, 2.3);
    int2d    b(3, 4);
    binary2d c(1, 0);

    double2d d = a + b * c;
    d /= -a;

    EXPECT_DOUBLE_EQ(d.x, (a.x + b.x * c.x) / -a.x);
    EXPECT_DOUBLE_EQ(d.y, (a.y + b.y * c.y) / -a.y);
}

TEST(math, arithmetic)
{
    // sum, product
    int2d a(1, 2);
    EXPECT_EQ(a.sum(), 3);
    EXPECT_EQ(a.product(), 2);

    // nonzero
    EXPECT_TRUE(a.nonzero());
    EXPECT_FALSE(int2d::zero.nonzero());
    
    // scalar modifiers
    double2d b(-2.3, 4.5);
    EXPECT_EQ(b.abs(), double2d(2.3, 4.5));
    EXPECT_EQ(b.round(), int2d(-2, 5));
    EXPECT_EQ(b.floor(), int2d(-3, 4));
    EXPECT_EQ(b.ceil(), int2d(-2, 5));

    // scalar casting
    double2d c = b.scalar_cast<int>();
    EXPECT_EQ(c, int2d(b.x, b.y));
}

TEST(math, length)
{
    uint2d a(1, 1);
    int2d b(-5, -5);

    EXPECT_DOUBLE_EQ(a.length(), std::sqrt(2));
    EXPECT_DOUBLE_EQ(a.distance(b), uint2d(6, 6).length());
    EXPECT_DOUBLE_EQ(a.normalize().length(), 1);
    EXPECT_DOUBLE_EQ(b.set_length(100).length(), 100);
}

TEST(math, linear)
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

TEST(serialization, to_string)
{
    int2d a(1, 2);
    EXPECT_EQ(a.to_string(), "x: 1  y: 2  ");
}

TEST(conversions, conversions)
{
    some_vector v = int2d(1, 2);

    EXPECT_EQ(v.x, 1);
    EXPECT_EQ(v.y, 2);

    int2d w = v;
    EXPECT_EQ(w.x, 1);
    EXPECT_EQ(w.y, 2);
}
