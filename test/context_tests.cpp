
#include <context.hpp>
#include <gtest/gtest.h>
#include <string>

TEST(Context, AssureArchetype_Single_UniqueIndex)
{
  xecs::context context;

  ASSERT_NE(context.assure_archetype<>(), context.assure_archetype<int>());
}

TEST(Context, AssureArchetype_Multiple_UniqueIndex)
{
  xecs::context context;

  ASSERT_EQ(context.assure_archetype<int>(), context.assure_archetype<int>());
  ASSERT_NE(context.assure_archetype<int>(), context.assure_archetype<float>());
  ASSERT_NE(context.assure_archetype<int>(), context.assure_archetype<double>());

  ASSERT_EQ(context.assure_archetype<float>(), context.assure_archetype<float>());
  ASSERT_NE(context.assure_archetype<float>(), context.assure_archetype<int>());
  ASSERT_NE(context.assure_archetype<float>(), context.assure_archetype<double>());

  ASSERT_EQ(context.assure_archetype<double>(), context.assure_archetype<double>());
  ASSERT_NE(context.assure_archetype<double>(), context.assure_archetype<int>());
  ASSERT_NE(context.assure_archetype<double>(), context.assure_archetype<float>());
}

TEST(Context, AssureArchetype_DifferentOrder_UniqueIndex)
{
  xecs::context context;

  ASSERT_EQ((context.assure_archetype<double, int>()), (context.assure_archetype<int, double>()));
  ASSERT_EQ((context.assure_archetype<float, int>()), (context.assure_archetype<int, float>()));
  ASSERT_EQ((context.assure_archetype<double, float>()), (context.assure_archetype<float, double>()));

  xecs::cid c3 = context.assure_archetype<int, float, double>();

  ASSERT_EQ((context.assure_archetype<int, double, float>()), c3);
  ASSERT_EQ((context.assure_archetype<float, int, double>()), c3);
  ASSERT_EQ((context.assure_archetype<float, double, int>()), c3);
  ASSERT_EQ((context.assure_archetype<double, float, int>()), c3);
  ASSERT_EQ((context.assure_archetype<double, int, float>()), c3);
}

TEST(Context, AssureView_Single_UniqueIndex)
{
  xecs::context context;

  ASSERT_NE(context.assure_view<>(), context.assure_view<int>());
}

TEST(Context, AssureView_Multiple_UniqueIndex)
{
  xecs::context context;

  ASSERT_EQ(context.assure_view<int>(), context.assure_view<int>());
  ASSERT_NE(context.assure_view<int>(), context.assure_view<float>());
  ASSERT_NE(context.assure_view<int>(), context.assure_view<double>());

  ASSERT_EQ(context.assure_view<float>(), context.assure_view<float>());
  ASSERT_NE(context.assure_view<float>(), context.assure_view<int>());
  ASSERT_NE(context.assure_view<float>(), context.assure_view<double>());

  ASSERT_EQ(context.assure_view<double>(), context.assure_view<double>());
  ASSERT_NE(context.assure_view<double>(), context.assure_view<int>());
  ASSERT_NE(context.assure_view<double>(), context.assure_view<float>());
}

TEST(Context, AssureView_DifferentOrder_UniqueIndex)
{
  xecs::context context;

  ASSERT_EQ((context.assure_view<double, int>()), (context.assure_view<int, double>()));
  ASSERT_EQ((context.assure_view<float, int>()), (context.assure_view<int, float>()));
  ASSERT_EQ((context.assure_view<double, float>()), (context.assure_view<float, double>()));

  xecs::cid c3 = context.assure_view<int, float, double>();

  ASSERT_EQ((context.assure_view<int, double, float>()), c3);
  ASSERT_EQ((context.assure_view<float, int, double>()), c3);
  ASSERT_EQ((context.assure_view<float, double, int>()), c3);
  ASSERT_EQ((context.assure_view<double, float, int>()), c3);
  ASSERT_EQ((context.assure_view<double, int, float>()), c3);
}

TEST(Context, ViewArchetypes_Single)
{
  xecs::context context;

  xecs::cid archetype_index = context.assure_archetype<int>();

  xecs::cid empty_view_index = context.assure_view<>();
  std::vector<xecs::cid> archetypes = context.view_archetypes(empty_view_index);

  ASSERT_EQ(archetypes.size(), 1);
  ASSERT_EQ(archetypes[0], archetype_index);

  xecs::cid int_view_index = context.assure_view<int>();
  archetypes = context.view_archetypes(int_view_index);

  ASSERT_EQ(archetypes.size(), 1);
  ASSERT_EQ(archetypes[0], archetype_index);

  xecs::cid float_view_index = context.assure_view<float>();
  archetypes = context.view_archetypes(float_view_index);

  ASSERT_EQ(archetypes.size(), 0);
}

#include <iostream>

TEST(Context, ViewArchetypes_Multiple_CorrectSize)
{
  xecs::context context;

  std::vector<xecs::cid> view_archetypes;

  context.assure_archetype<int>();
  context.assure_archetype<float>();
  context.assure_archetype<bool>();
  context.assure_archetype<double>();
  context.assure_archetype<int, float>();
  context.assure_archetype<double, float>();
  context.assure_archetype<float, double>();
  context.assure_archetype<float, double, int>();
  context.assure_archetype<double, float, int>();
  context.assure_archetype<int, float, double>();
  context.assure_archetype<bool, double, int>();

  view_archetypes = context.view_archetypes(context.assure_view());
  ASSERT_EQ(view_archetypes.size(), 8);

  view_archetypes = context.view_archetypes(context.assure_view<int>());
  ASSERT_EQ(view_archetypes.size(), 4);

  view_archetypes = context.view_archetypes(context.assure_view<float>());
  ASSERT_EQ(view_archetypes.size(), 4);

  view_archetypes = context.view_archetypes(context.assure_view<double>());
  ASSERT_EQ(view_archetypes.size(), 4);

  view_archetypes = context.view_archetypes(context.assure_view<bool>());
  ASSERT_EQ(view_archetypes.size(), 2);

  view_archetypes = context.view_archetypes(context.assure_view<bool>());
  ASSERT_EQ(view_archetypes.size(), 2);

  view_archetypes = context.view_archetypes(context.assure_view<int, float>());
  ASSERT_EQ(view_archetypes.size(), 2);

  view_archetypes = context.view_archetypes(context.assure_view<double, int>());
  ASSERT_EQ(view_archetypes.size(), 2);

  view_archetypes = context.view_archetypes(context.assure_view<int, double>());
  ASSERT_EQ(view_archetypes.size(), 2);

  view_archetypes = context.view_archetypes(context.assure_view<float, double, int>());
  ASSERT_EQ(view_archetypes.size(), 1);

  view_archetypes = context.view_archetypes(context.assure_view<double, float, int>());
  ASSERT_EQ(view_archetypes.size(), 1);
}

TEST(Context, ViewArchetypes_AfterView_CorrectSize)
{
  xecs::context context;

  std::vector<xecs::cid> view_archetypes;

  context.assure_archetype<int>();
  context.assure_archetype<float>();
  context.assure_archetype<bool>();
  context.assure_archetype<double>();

  view_archetypes = context.view_archetypes(context.assure_view());
  ASSERT_EQ(view_archetypes.size(), 4);

  view_archetypes = context.view_archetypes(context.assure_view<int>());
  ASSERT_EQ(view_archetypes.size(), 1);

  context.assure_archetype<int, float>();
  context.assure_archetype<double, float>();
  context.assure_archetype<float, double, int>();
  context.assure_archetype<bool, double, int>();

  view_archetypes = context.view_archetypes(context.assure_view());
  ASSERT_EQ(view_archetypes.size(), 8);

  view_archetypes = context.view_archetypes(context.assure_view<int>());
  ASSERT_EQ(view_archetypes.size(), 4);
}

TEST(Context, ViewArchetypes_Multiple_CorrectArchetypes)
{
  xecs::context context;

  std::vector<xecs::cid> archetypes;
  std::vector<xecs::cid> view_archetypes;

  archetypes.push_back(context.assure_archetype<int>());
  context.assure_archetype<float>();
  context.assure_archetype<bool>();
  context.assure_archetype<double>();
  archetypes.push_back(context.assure_archetype<int, float>());
  context.assure_archetype<double, float>();

  size_t view = context.assure_view<int>();

  archetypes.push_back(context.assure_archetype<float, double, int>());
  archetypes.push_back(context.assure_archetype<bool, double, int>());

  view_archetypes = context.view_archetypes(view);

  ASSERT_EQ(view_archetypes.size(), archetypes.size());

  std::sort(archetypes.begin(), archetypes.end());
  std::sort(view_archetypes.begin(), view_archetypes.end());

  ASSERT_TRUE(std::includes(archetypes.begin(), archetypes.end(), view_archetypes.begin(), view_archetypes.end()));
}