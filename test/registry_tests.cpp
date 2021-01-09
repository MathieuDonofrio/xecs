#include <gtest/gtest.h>
#include <registry.hpp>

using namespace ecs;

TEST(Registry, Unpack_SetWithMultipleArchetypes_SameValues)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<int, float>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  auto entity1 = registry.create(5);
  auto entity2 = registry.create(8, 0.5f);

  registry.unpack<int>(entity2) = 99;

  ASSERT_EQ(registry.unpack<int>(entity2), 99);
  ASSERT_EQ(registry.unpack<float>(entity2), 0.5f);
  ASSERT_EQ(registry.unpack<int>(entity1), 5);

  registry.unpack<int>(entity1) = 10;

  ASSERT_EQ(registry.unpack<int>(entity1), 10);
  ASSERT_EQ(registry.unpack<int>(entity2), 99);
  ASSERT_EQ(registry.unpack<float>(entity2), 0.5f);
}

TEST(Registry, Unpack_EntityThatDoesntExist_Throw)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  auto entity1 = registry.create(10);
  auto entity2 = registry.create(0.5f);

  try
  {
    registry.unpack<float>(entity1) = 2.5f;
    ASSERT_TRUE(false);
  }
  catch (const std::exception&)
  {};

  ASSERT_EQ(registry.unpack<int>(entity1), 10);

  try
  {
    registry.unpack<int>(entity2);
    ASSERT_TRUE(false);
  }
  catch (const std::exception&)
  {};

  ASSERT_EQ(registry.unpack<float>(entity2), 0.5f);
}

TEST(Registry, ForEach_Single_SameValues)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  auto e = registry.create(5);

  registry.for_each<int>([e](auto entity, auto i)
    {
      ASSERT_EQ(entity, e);
      ASSERT_EQ(i, 5);
    });
}

TEST(Registry, ForEach_SingleSet_SameValues)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  auto e = registry.create(5);

  registry.for_each<int>([](auto, auto& i)
    { i = 10; });

  ASSERT_EQ(registry.unpack<int>(e), 10);
}

TEST(Registry, ForEach_Multiple_CorrectIterations)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  int amount = 10000;

  std::vector<int> entities;

  for (int i = 0; i < amount; i++)
  {
    entities.push_back(registry.create(i));
  }

  size_t count = 0;

  registry.for_each([&count](auto)
    { count++; });

  ASSERT_EQ(amount, count);
}

TEST(Registry, ForEach_MultipleTwoArchetypes_CorrectIterations)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<int, float>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  int amount = 10000;

  std::vector<int> entities;

  for (int i = 0; i < amount; i++)
  {
    if (i % 2 == 0)
      entities.push_back(registry.create(i));
    else
      entities.push_back(registry.create(i, static_cast<float>(i)));
  }

  size_t intview = 0;

  registry.for_each<int>([&intview](auto, auto)
    { intview++; });

  ASSERT_EQ(amount, intview);

  size_t floatview = 0;

  registry.for_each<float>([&floatview](auto, auto)
    { floatview++; });

  ASSERT_EQ(amount / 2, floatview);
}

