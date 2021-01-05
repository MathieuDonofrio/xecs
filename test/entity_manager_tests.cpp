#include <entity_manager.hpp>
#include <gtest/gtest.h>
#include <vector>

using namespace ecs;

TEST(EntityManager, Peek_AfterInitialization_Zero)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  ASSERT_EQ(manager.peek(), 0);
}

TEST(EntityManager, Reusable_AfterInitialization_None)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  ASSERT_EQ(manager.reusable(), 0);
}

TEST(EntityManager, Generate_Single_Increment)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  ASSERT_EQ(manager.generate(), 0);
  ASSERT_EQ(manager.peek(), 1);
}

TEST(EntityManager, Generate_Multiple_Increment)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  size_t amount = 100000;

  for (size_t i = 0; i < amount; i++)
  {
    ASSERT_EQ(manager.generate(), i);
  }

  ASSERT_EQ(manager.peek(), amount);
}

TEST(EntityManager, Release_Single_Increment)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  entity_type entity = manager.generate();

  ASSERT_EQ(manager.reusable(), 0);

  manager.release(entity);

  ASSERT_EQ(manager.reusable(), 1);
}

TEST(EntityManager, Release_Multiple_Increment)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  int amount = 100000;

  std::vector<entity_type> entities;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
    ASSERT_EQ(manager.reusable(), *it + 1);
  }
}

TEST(EntityManager, Generate_SingleReused_Equal)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  entity_type entity = manager.generate();
  manager.release(entity);

  ASSERT_EQ(manager.generate(), entity);
}

TEST(EntityManager, Release_SingleOnStack_Increment)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  manager.release(manager.generate());

  ASSERT_EQ(manager.reusable(), 1);
  ASSERT_EQ(manager.stack_reusable(), 1);
  ASSERT_EQ(manager.heap_reusable(), 0);
}

TEST(EntityManager, Release_FillStackGoToHeap_Increment)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  entity_type entity = manager.generate();

  std::vector<entity_type> entities;

  entities.reserve(manager.stack_capacity);

  for (size_t i = 0; i < manager.stack_capacity; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_EQ(manager.stack_reusable(), manager.reusable());
  ASSERT_EQ(manager.heap_reusable(), 0);

  manager.release(entity);

  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_NE(manager.stack_reusable(), manager.reusable());
  ASSERT_EQ(manager.heap_reusable(), 1);
  ASSERT_EQ(manager.heap_reusable() + manager.stack_reusable(), manager.reusable());
}

TEST(EntityManager, Release_EmptyStackBeforeHeap)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  std::vector<entity_type> entities;

  size_t amount = manager.stack_capacity + 1;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_EQ(manager.stack_reusable(), manager.reusable() - 1);
  ASSERT_EQ(manager.heap_reusable(), 1);

  for (size_t i = manager.stack_reusable() - 1; i != static_cast<size_t>(-1); i--)
  {
    manager.generate();
    ASSERT_EQ(manager.heap_reusable(), 1);
    ASSERT_EQ(manager.stack_reusable(), i);
  }

  ASSERT_EQ(manager.heap_reusable(), 1);
  ASSERT_EQ(manager.reusable(), 1);
  ASSERT_EQ(manager.stack_reusable(), 0);

  manager.generate();

  ASSERT_EQ(manager.reusable(), 0);
  ASSERT_EQ(manager.stack_reusable(), 0);
  ASSERT_EQ(manager.heap_reusable(), 0);
}

TEST(EntityManager, Release_TriggerHeapGrowth)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  std::vector<entity_type> entities;

  size_t amount = manager.stack_capacity + manager.minimum_heap_capacity;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  entity_type entity = manager.generate();

  ASSERT_LT(manager.heap_reusable(), manager.heap_capacity());

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_EQ(manager.heap_reusable(), manager.heap_capacity());
  ASSERT_EQ(manager.reusable(), manager.stack_reusable() + manager.heap_reusable());

  manager.release(entity);

  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_LT(manager.heap_reusable(), manager.heap_capacity());
  ASSERT_EQ(manager.heap_reusable(), manager.minimum_heap_capacity + 1);
  ASSERT_EQ(manager.reusable(), manager.stack_reusable() + manager.heap_reusable());

  for (size_t i = manager.stack_capacity - 1; i != static_cast<size_t>(-1); i--)
  {
    ASSERT_EQ(manager.generate(), i);
    ASSERT_EQ(manager.stack_reusable(), i);
  }

  ASSERT_EQ(manager.stack_reusable(), 0);

  for (size_t i = amount; i >= manager.stack_capacity; i--)
  {
    ASSERT_EQ(manager.generate(), i);
    ASSERT_EQ(manager.heap_reusable(), i - manager.stack_capacity);
  }

  ASSERT_EQ(manager.heap_reusable(), 0);
  ASSERT_EQ(manager.reusable(), 0);
}

TEST(EntityManager, Swap_NothingInHeap_DoNothing)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  std::vector<entity_type> entities;

  size_t amount = manager.stack_capacity / 2;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_EQ(manager.stack_reusable(), amount);
  ASSERT_EQ(manager.heap_reusable(), 0);

  manager.swap();

  ASSERT_EQ(manager.stack_reusable(), amount);
  ASSERT_EQ(manager.heap_reusable(), 0);
}

TEST(EntityManager, Swap_StackAlreadyFull_DoNothing)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  std::vector<entity_type> entities;

  size_t amount = manager.stack_capacity + 10;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_EQ(manager.stack_reusable(), amount - 10);
  ASSERT_EQ(manager.heap_reusable(), 10);

  manager.swap();

  ASSERT_EQ(manager.stack_reusable(), amount - 10);
  ASSERT_EQ(manager.heap_reusable(), 10);
}

TEST(EntityManager, Swap_PutIntoStack)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  std::vector<entity_type> entities;

  size_t amount_to_swap = 100;

  size_t amount = manager.stack_capacity + amount_to_swap;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_EQ(manager.reusable(), amount);
  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_EQ(manager.heap_reusable(), amount_to_swap);

  for (size_t i = 0; i < amount_to_swap; i++)
  {
    manager.generate();
  }

  ASSERT_EQ(manager.reusable(), amount - amount_to_swap);
  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity - amount_to_swap);
  ASSERT_EQ(manager.heap_reusable(), amount_to_swap);

  manager.swap();

  ASSERT_EQ(manager.reusable(), amount - amount_to_swap);
  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_EQ(manager.heap_reusable(), 0);

  for (size_t i = 0; i < amount_to_swap; i++)
  {
    ASSERT_EQ(manager.generate(), amount - 1 - i);
  }
}

TEST(EntityManager, Swap_StackCanOnlyFitHalfOfHeap)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  std::vector<entity_type> entities;

  size_t amount_on_heap = 100;

  size_t amount_to_swap = amount_on_heap / 2;

  size_t amount = manager.stack_capacity + amount_on_heap;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_EQ(manager.reusable(), amount);
  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_EQ(manager.heap_reusable(), amount_on_heap);

  for (size_t i = 0; i < amount_to_swap; i++)
  {
    manager.generate();
  }

  ASSERT_EQ(manager.reusable(), amount - amount_to_swap);
  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity - amount_to_swap);
  ASSERT_EQ(manager.heap_reusable(), amount_on_heap);

  manager.swap();

  ASSERT_EQ(manager.reusable(), amount - amount_to_swap);
  ASSERT_EQ(manager.stack_reusable(), manager.stack_capacity);
  ASSERT_EQ(manager.heap_reusable(), amount_on_heap - amount_to_swap);

  for (size_t i = 0; i < amount_to_swap; i++)
  {
    ASSERT_EQ(manager.generate(), amount - 1 - i);
  }
}

TEST(EntityManager, ShrinkToFit_UnderMinimumCapacity_DontShrink)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  ASSERT_EQ(manager.heap_capacity(), manager.minimum_heap_capacity);

  manager.shrink_to_fit();

  ASSERT_EQ(manager.heap_capacity(), manager.minimum_heap_capacity);
}

TEST(EntityManager, ShrinkToFit_MemoryOverhead)
{
  using entity_type = unsigned int;
  using entity_manager_type = entity_manager<entity_type>;

  entity_manager_type manager;

  ASSERT_EQ(manager.heap_capacity(), manager.minimum_heap_capacity);

  std::vector<entity_type> entities;

  size_t amount = manager.stack_capacity + manager.minimum_heap_capacity + 1;

  entities.reserve(amount);

  for (size_t i = 0; i < amount; i++)
  {
    entities.push_back(manager.generate());
  }

  for (auto it = entities.begin(); it != entities.end(); ++it)
  {
    manager.release(*it);
  }

  ASSERT_GT(manager.heap_capacity(), manager.minimum_heap_capacity);

  manager.shrink_to_fit();

  ASSERT_EQ(manager.heap_capacity(), manager.minimum_heap_capacity + 1);
}