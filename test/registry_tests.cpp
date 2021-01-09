#include <gtest/gtest.h>
#include <registry.hpp>

using namespace ecs;

TEST(Registry, Storages_OneArchetype_OneStorages)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  ASSERT_EQ(registry.storages(), 1);
}

TEST(Registry, Storages_TwoArchetypes_TwoStorages)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  ASSERT_EQ(registry.storages(), 2);
}

TEST(Registry, Storages_ThreeArchetypes_ThreeStorages)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float>>::
        add<archetype<int, float>>::
          build;

  registry<entity_type, registered_archetypes> registry;

  ASSERT_EQ(registry.storages(), 3);
}

TEST(Registry, Storages_FiveArchetypes_FiveStorages)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float>>::
        add<archetype<int, float>>::
          add<archetype<double>>::
            add<archetype<float, double>>::
              build;

  registry<entity_type, registered_archetypes> registry;

  ASSERT_EQ(registry.storages(), 5);
}

TEST(Registry, Empty_AfterInitialization_True)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, Create_Single_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  registry.create();

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), 1);
}

TEST(Registry, Has_WithoutValue_False)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  ASSERT_FALSE(registry.has(0));
}

TEST(Registry, Has_WithValue_True)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create();

  ASSERT_TRUE(registry.has(entity));
}

TEST(Registry, Has_WithIncorrectValue_True)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create();

  ASSERT_FALSE(registry.has(entity + 1));
}

TEST(Registry, Destroy_Single_SizeDecrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create();
  registry.destroy(entity);

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, DestroyAll_Empty_Empty)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  registry.destroy_all();

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, DestroyAll_Single_Empty)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  registry.create();
  registry.destroy_all();

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, Create_Multiple_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<>>::build;

  registry<entity_type, registered_archetypes> registry;

  size_t amount = 1000;

  for (size_t i = 0; i < amount; i++)
  {
    registry.create();
  }

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), amount);
}

TEST(Registry, Create_TwoArchetypes_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  registry.create(10);
  registry.create(0.5f);

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), 2);
  ASSERT_EQ(registry.size<int>(), 1);
  ASSERT_EQ(registry.size<float>(), 1);
}

TEST(Registry, Has_TwoArchetypes)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  auto entity1 = registry.create(10);

  ASSERT_TRUE(registry.has(entity1));
  ASSERT_TRUE(registry.has<int>(entity1));
  ASSERT_FALSE(registry.has<float>(entity1));

  auto entity2 = registry.create(0.5f);

  ASSERT_TRUE(registry.has(entity1));
  ASSERT_TRUE(registry.has<int>(entity1));
  ASSERT_FALSE(registry.has<float>(entity1));

  ASSERT_TRUE(registry.has(entity2));
  ASSERT_FALSE(registry.has<int>(entity2));
  ASSERT_TRUE(registry.has<float>(entity2));
}

TEST(Registry, Destroy_TwoArchetypes_SizeDecrease)
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
    registry.destroy<float>(entity1);
  }
  catch (const std::exception&)
  {};

  ASSERT_EQ(registry.size(), 2);

  try
  {
    registry.destroy<int>(entity2);
  }
  catch (const std::exception&)
  {};

  ASSERT_EQ(registry.size(), 2);

  registry.destroy<int>(entity1);

  ASSERT_EQ(registry.size(), 1);
  ASSERT_EQ(registry.size<int>(), 0);
  ASSERT_EQ(registry.size<float>(), 1);

  registry.destroy<float>(entity2);

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
  ASSERT_EQ(registry.size<int>(), 0);
  ASSERT_EQ(registry.size<float>(), 0);
}

TEST(Registry, Create_TwoArchetypesMultipleComponents_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float, int, bool>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  registry.create(10);
  registry.create(0.5f, 5, true);

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), 2);
  ASSERT_EQ(registry.size<int>(), 2);
  ASSERT_EQ(registry.size<bool>(), 1);
  ASSERT_EQ(registry.size<float>(), 1);
  ASSERT_EQ((registry.size<float, int>()), 1);
  ASSERT_EQ((registry.size<int, float>()), 1);
  ASSERT_EQ((registry.size<int, bool>()), 1);
  ASSERT_EQ((registry.size<bool, int>()), 1);
  ASSERT_EQ((registry.size<float, bool>()), 1);
  ASSERT_EQ((registry.size<bool, float>()), 1);
  ASSERT_EQ((registry.size<float, int, bool>()), 1);
  ASSERT_EQ((registry.size<int, float, bool>()), 1);
  ASSERT_EQ((registry.size<bool, int, float>()), 1);
  ASSERT_EQ((registry.size<bool, float, int>()), 1);
  ASSERT_EQ((registry.size<float, bool, int>()), 1);
}

TEST(Registry, Create_TwoArchetypesMultipleComponentsDifferentOrder_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<float, int, bool>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  registry.create(0.5f, true, 5);

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), 1);
  ASSERT_EQ((registry.size<float, bool>()), 1);

  registry.create(5, 0.5f, true);

  ASSERT_EQ(registry.size(), 2);
  ASSERT_EQ((registry.size<float, bool>()), 2);

  registry.create(5, true, 0.5f);

  ASSERT_EQ(registry.size(), 3);
  ASSERT_EQ((registry.size<float, bool>()), 3);

  registry.create(true, 5, 0.5f);

  ASSERT_EQ(registry.size(), 4);
  ASSERT_EQ((registry.size<float, bool>()), 4);
}

TEST(Registry, Unpack_SingleAfterInitialization_SameValue)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create(5);

  ASSERT_EQ(registry.unpack<int>(entity), 5);
  ASSERT_NE(registry.unpack<int>(entity), 4);
}

TEST(Registry, Unpack_MultipleAfterInitialization_SameValues)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  int amount = 1000;

  std::vector<int> entities;

  for (int i = 0; i < amount; i++)
  {
    entities.push_back(registry.create(i));
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    ASSERT_EQ(registry.unpack<int>(*it), *it);
  }
}

TEST(Registry, Unpack_SingleSet_SameValue)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create(5);

  registry.unpack<int>(entity) = 4;

  ASSERT_EQ(registry.unpack<int>(entity), 4);
  ASSERT_NE(registry.unpack<int>(entity), 5);
}

TEST(Registry, Unpack_DoubleSet_SameValue)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create(5);

  registry.unpack<int>(entity) = 4;

  registry.unpack<int>(entity) = 99;

  ASSERT_EQ(registry.unpack<int>(entity), 99);
}

TEST(Registry, Unpack_MultipleSets_SameValues)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      build;

  registry<entity_type, registered_archetypes> registry;

  int amount = 1000;

  std::vector<int> entities;

  for (int i = 0; i < amount; i++)
  {
    entities.push_back(registry.create(i));
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    int changed_value = *it * 2 + 1;
    registry.unpack<int>(*it) = changed_value;
    ASSERT_EQ(registry.unpack<int>(*it), changed_value);
  }
}

TEST(Registry, Unpack_MultipleArchetypes_SameValues)
{
  using entity_type = unsigned int;
  using registered_archetypes = archetype_list_builder::
    add<archetype<int>>::
      add<archetype<int, float>>::
        build;

  registry<entity_type, registered_archetypes> registry;

  auto entity1 = registry.create(5);

  ASSERT_EQ(registry.unpack<int>(entity1), 5);

  auto entity2 = registry.create(8, 0.5f);

  ASSERT_EQ(registry.unpack<int>(entity2), 8);
  ASSERT_EQ(registry.unpack<float>(entity2), 0.5f);
}

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
    registry.unpack<float>(entity1) = 15;
  }
  catch (const std::exception&)
  {};

  ASSERT_EQ(registry.unpack<int>(entity1), 10);

  try
  {
    registry.unpack<int>(entity2);
  }
  catch (const std::exception&)
  {};

  ASSERT_EQ(registry.unpack<float>(entity2), 95.5f);
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

