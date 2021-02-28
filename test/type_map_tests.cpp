
#include <gtest/gtest.h>
#include <string>
#include <type_map.hpp>

TEST(TypeMap, Safe_NoAccess_False)
{
  xecs::type_map<int> container;

  ASSERT_FALSE(container.safe(container.key<int>()));
  ASSERT_FALSE(container.safe(container.key<double>()));
}

TEST(TypeMap, Safe_WithAccess_True)
{
  xecs::type_map<int> container;

  container.template access<int, int>() = 10;
  container.template access<double, int>() = 10;

  ASSERT_TRUE(container.safe(container.key<int>()));
  ASSERT_TRUE(container.safe(container.key<double>()));
}

TEST(TypeMap, Access_CorrectValue)
{
  xecs::type_map<int> container;

  container.template access<int, int>() = 10;

  ASSERT_EQ((container.template access<int, int>()), 10);

  container.template access<double, int>() = 15;

  ASSERT_EQ((container.template access<int, int>()), 10);
  ASSERT_EQ((container.template access<double, int>()), 15);
}

TEST(TypeMap, RawAccess_CorrectValue)
{
  xecs::type_map<int> container;

  container.template access<int, int>();
  container.template access<double, int>();

  container.template raw_access<int>(container.template key<int>()) = 10;

  ASSERT_EQ((container.template raw_access<int>(container.template key<int>())), 10);
  ASSERT_EQ((container.template access<int, int>()), 10);

  container.template raw_access<int>(container.template key<double>()) = 15;

  ASSERT_EQ((container.template raw_access<int>(container.template key<int>())), 10);
  ASSERT_EQ((container.template access<int, int>()), 10);
  ASSERT_EQ((container.template raw_access<int>(container.template key<double>())), 15);
  ASSERT_EQ((container.template access<double, int>()), 15);
}