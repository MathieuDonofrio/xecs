
#include <container.hpp>
#include <gtest/gtest.h>
#include <string>

TEST(Container, Empty_AfterInitialization)
{
  xecs::container<int> container;

  ASSERT_TRUE(container.empty());
  ASSERT_EQ(container.size(), 0);
  ASSERT_EQ(container.capacity(), 0);
}

TEST(Container, Empty_NonTrivial_AfterInitialization)
{
  xecs::container<std::string> container;

  ASSERT_TRUE(container.empty());
  ASSERT_EQ(container.size(), 0);
  ASSERT_EQ(container.capacity(), 0);
}

TEST(Container, Reserve_CapacityIncrease)
{
  xecs::container<int> container;

  container.reserve(15);

  ASSERT_EQ(container.capacity(), 15);

  container.reserve(50);

  ASSERT_EQ(container.capacity(), 50);
}

TEST(Container, Reserve_NonTrivial_CapacityIncrease)
{
  xecs::container<std::string> container;

  container.reserve(15);

  ASSERT_EQ(container.capacity(), 15);

  container.reserve(50);

  ASSERT_EQ(container.capacity(), 50);
}

TEST(Container, Reserve_Smaller_DoNothing)
{
  xecs::container<int> container;

  container.reserve(15);

  ASSERT_EQ(container.capacity(), 15);

  container.reserve(5);

  ASSERT_EQ(container.capacity(), 15);
}

TEST(Container, Reserve_NonTrivial_Smaller_DoNothing)
{
  xecs::container<std::string> container;

  container.reserve(15);

  ASSERT_EQ(container.capacity(), 15);

  container.reserve(5);

  ASSERT_EQ(container.capacity(), 15);
}

TEST(Container, PushBack_SizeIncrease)
{
  xecs::container<int> container;
  container.reserve(10);

  container.push_back(15);

  ASSERT_FALSE(container.empty());
  ASSERT_EQ(container.size(), 1);

  container.push_back(50);

  ASSERT_FALSE(container.empty());
  ASSERT_EQ(container.size(), 2);
}

TEST(Container, PushBack_NonTrivial_SizeIncrease)
{
  xecs::container<std::string> container;
  container.reserve(10);

  container.push_back(std::string { "1" });

  ASSERT_FALSE(container.empty());
  ASSERT_EQ(container.size(), 1);

  container.push_back(std::string { "2" });

  ASSERT_FALSE(container.empty());
  ASSERT_EQ(container.size(), 2);
}

TEST(Container, PushBack_CorrectValue)
{
  xecs::container<int> container;
  container.reserve(10);

  container.push_back(15);

  ASSERT_EQ(container.template access<int>()[0], 15);

  container.push_back(50);

  ASSERT_EQ(container.template access<int>()[1], 50);
}

TEST(Container, PushBack_NonTrivial_CorrectValue)
{
  xecs::container<std::string> container;
  container.reserve(10);

  container.push_back(std::string { "1" });

  ASSERT_EQ(container.template access<std::string>()[0], std::string { "1" });

  container.push_back(std::string { "2" });

  ASSERT_EQ(container.template access<std::string>()[1], std::string { "2" });
}

TEST(Container, PushBack_BeforeGrow_CorrectValue)
{
  xecs::container<int> container;
  container.reserve(1);

  container.push_back(15);

  ASSERT_EQ(container.template access<int>()[0], 15);

  container.reserve(2);

  ASSERT_EQ(container.template access<int>()[0], 15);

  container.push_back(50);

  ASSERT_EQ(container.template access<int>()[0], 15);
  ASSERT_EQ(container.template access<int>()[1], 50);
}

TEST(Container, PushBack_NonTrivial_BeforeGrow_CorrectValue)
{
  xecs::container<std::string> container;
  container.reserve(1);

  container.push_back(std::string { "15" });

  ASSERT_EQ(container.template access<std::string>()[0], std::string { "15" });

  container.reserve(2);

  ASSERT_EQ(container.template access<std::string>()[0], std::string { "15" });

  container.push_back(std::string { "50" });

  ASSERT_EQ(container.template access<std::string>()[0], std::string { "15" });
  ASSERT_EQ(container.template access<std::string>()[1], std::string { "50" });
}

TEST(Container, Clear_Empty)
{
  xecs::container<int> container;
  container.reserve(1);

  container.push_back(15);

  container.clear();

  ASSERT_EQ(container.size(), 0);
  ASSERT_EQ(container.capacity(), 1);
}

TEST(Container, Clear_NonTrivial_Empty)
{
  xecs::container<std::string> container;
  container.reserve(1);

  container.push_back(std::string { "15" });

  container.clear();

  ASSERT_EQ(container.size(), 0);
  ASSERT_EQ(container.capacity(), 1);
}

TEST(Container, ShrinkToFit_Shrinked)
{
  xecs::container<int> container;
  container.reserve(10);

  container.push_back(15);
  container.push_back(20);

  container.shrink_to_fit();

  ASSERT_EQ(container.size(), 2);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<int>()[0], 15);
  ASSERT_EQ(container.template access<int>()[1], 20);
}

TEST(Container, ShrinkToFit_NonTrivial_Shrinked)
{
  xecs::container<std::string> container;
  container.reserve(10);

  container.push_back(std::string { "15" });
  container.push_back(std::string { "20" });

  container.shrink_to_fit();

  ASSERT_EQ(container.size(), 2);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<std::string>()[0], std::string { "15" });
  ASSERT_EQ(container.template access<std::string>()[1], std::string { "20" });
}

TEST(Container, Resize_Empty_SizeAndCapacityIncrease)
{
  xecs::container<int> container;

  container.resize(10);

  ASSERT_EQ(container.size(), 10);
  ASSERT_EQ(container.capacity(), 10);
}

TEST(Container, Resize_NonTrivial_Empty_SizeAndCapacityIncrease)
{
  xecs::container<std::string> container;

  container.resize(10);

  ASSERT_EQ(container.size(), 10);
  ASSERT_EQ(container.capacity(), 10);
}

TEST(Container, Resize_Bigger_SizeAndCapacityIncrease)
{
  xecs::container<int> container;

  container.reserve(1);

  container.push_back(10);

  ASSERT_EQ(container.template access<int>()[0], 10);

  container.resize(10);

  ASSERT_EQ(container.size(), 10);
  ASSERT_EQ(container.capacity(), 10);
  ASSERT_EQ(container.template access<int>()[0], 10);
}

TEST(Container, Resize_NonTrivial_Bigger_SizeAndCapacityIncrease)
{
  xecs::container<std::string> container;

  container.reserve(1);

  container.push_back(std::string { "10" });

  ASSERT_EQ(container.template access<std::string>()[0], std::string { "10" });

  container.resize(10);

  ASSERT_EQ(container.size(), 10);
  ASSERT_EQ(container.capacity(), 10);
  ASSERT_EQ(container.template access<std::string>()[0], std::string { "10" });
}

TEST(Container, Resize_Smaller_SizeDecrease)
{
  xecs::container<int> container;

  container.reserve(2);

  container.push_back(10);
  container.push_back(15);

  container.resize(1);

  ASSERT_EQ(container.size(), 1);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<int>()[0], 10);
}

TEST(Container, Resize_NonTrivial_Smaller_SizeDecrease)
{
  xecs::container<std::string> container;

  container.reserve(2);

  container.push_back(std::string { "10" });
  container.push_back(std::string { "15" });

  container.resize(1);

  ASSERT_EQ(container.size(), 1);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<std::string>()[0], std::string { "10" });
}

TEST(Container, PopBack_SizeDecrease)
{
  xecs::container<int> container;

  container.reserve(2);

  container.push_back(10);
  container.push_back(15);

  container.pop_back();

  ASSERT_EQ(container.size(), 1);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<int>()[0], 10);
}

TEST(Container, PopBack_NonTrivial_SizeDecrease)
{
  xecs::container<std::string> container;

  container.reserve(2);

  container.push_back(std::string { "10" });
  container.push_back(std::string { "15" });

  container.pop_back();

  ASSERT_EQ(container.size(), 1);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<std::string>()[0], std::string { "10" });
}

TEST(Container, Erase_SizeDecreaseAndCorrectValue)
{
  xecs::container<int> container;

  container.reserve(2);

  container.push_back(10);
  container.push_back(15);

  container.erase(0);

  ASSERT_EQ(container.size(), 1);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<int>()[0], 15);
}

TEST(Container, Erase_NonTrivial_SizeDecreaseAndCorrectValue)
{
  xecs::container<std::string> container;

  container.reserve(2);

  container.push_back(std::string { "10" });
  container.push_back(std::string { "15" });

  container.erase(0);

  ASSERT_EQ(container.size(), 1);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<std::string>()[0], std::string { "15" });
}

TEST(Container, Insert_CorrectValue)
{
  xecs::container<int> container;

  container.resize(2);

  container.insert(0, 10);

  ASSERT_EQ(container.size(), 2);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<int>()[0], 10);

  container.insert(1, 15);

  ASSERT_EQ(container.size(), 2);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<int>()[0], 10);
  ASSERT_EQ(container.template access<int>()[1], 15);
}

TEST(Container, Insert_NonTrivial_CorrectValue)
{
  xecs::container<std::string> container;

  container.resize(2);

  container.insert(0, std::string { "10" });

  ASSERT_EQ(container.size(), 2);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<std::string>()[0], std::string { "10" });

  container.insert(1, std::string { "15" });

  ASSERT_EQ(container.size(), 2);
  ASSERT_EQ(container.capacity(), 2);
  ASSERT_EQ(container.template access<std::string>()[0], std::string { "10" });
  ASSERT_EQ(container.template access<std::string>()[1], std::string { "15" });
}