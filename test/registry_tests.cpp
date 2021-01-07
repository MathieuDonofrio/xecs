#include <gtest/gtest.h>
#include <registry.hpp>

using namespace ecs;

TEST(Registry, Empty_AfterInitialization_True)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, Create_Single_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  registry.create();

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), 1);
}

TEST(Registry, Has_WithoutValue_False)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  ASSERT_FALSE(registry.has(0));
}

TEST(Registry, Has_WithValue_True)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create();

  ASSERT_TRUE(registry.has(entity));
}

TEST(Registry, Has_WithIncorrectValue_True)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create();

  ASSERT_FALSE(registry.has(entity + 1));
}

TEST(Registry, Destroy_Single_SizeDecrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  auto entity = registry.create();
  registry.destroy(entity);

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, DestroyAll_Empty_Empty)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  registry.destroy_all();

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, DestroyAll_Single_Empty)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  registry.create();
  registry.destroy_all();

  ASSERT_TRUE(registry.empty());
  ASSERT_EQ(registry.size(), 0);
}

TEST(Registry, Create_Multiple_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<>>::build;

  static registry<entity_type, registered_archetypes> registry;

  size_t amount = 1000;

  for(size_t i = 0; i < amount; i++)
  {
    registry.create();
  }

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), amount);
}

TEST(Registry, Create_TwoArchetypes_SizeIncrease)
{
  using entity_type = unsigned int;
  using registered_archetypes = registry_builder::
    add<archetype<int>>::
    add<archetype<float>>::
    build;

  static registry<entity_type, registered_archetypes> registry;

  registry.create(10);
  registry.create(0.5f);

  ASSERT_FALSE(registry.empty());
  ASSERT_EQ(registry.size(), 2);
  ASSERT_EQ(registry.size<int>(), 1);
  ASSERT_EQ(registry.size<float>(), 1);
}
