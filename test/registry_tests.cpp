#include <gtest/gtest.h>
#include <registry.hpp>

using namespace ecs;

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
  ASSERT_EQ(registry.size<double>(), 0);
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

  static registry<entity_type, registered_archetypes> registry;

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