
#include <gtest/gtest.h>
#include <storage.hpp>
#include <string>

using namespace xecs;

struct NonTrivialDestructorOnly
{
  int* _destructor_counter;

  ~NonTrivialDestructorOnly()
  {
    (*_destructor_counter)++;
  }
};

struct NonTrivial
{
  int* _constructor_counter;
  int* _destructor_counter;

  NonTrivial() = default;

  NonTrivial(int* constructor_counter, int* destructor_counter)
    : _constructor_counter(constructor_counter), _destructor_counter(destructor_counter)
  {
    (*_constructor_counter)++;
  }

  ~NonTrivial()
  {
    (*_destructor_counter)++;
  }
};

TEST(Storage, Empty_AfterInitialization_True)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  ASSERT_TRUE(storage.empty());
  ASSERT_EQ(storage.size(), 0);
}

TEST(Storage, Insert_Single_SizeIncrease)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
}

TEST(Storage, Contains_WithoutValue_False)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  ASSERT_FALSE(storage.contains(0));
}

TEST(Storage, Contains_WithValue_True)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);

  ASSERT_TRUE(storage.contains(0));
}

TEST(Storage, Contains_LargeUninsertedValue_False)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  ASSERT_FALSE(storage.contains(1000000));
}

TEST(Storage, Erase_Single_SizeDecrease)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);
  storage.erase(0);

  ASSERT_TRUE(storage.empty());
  ASSERT_EQ(storage.size(), 0);
}

TEST(Storage, Clear_Empty_Empty)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.clear();

  ASSERT_TRUE(storage.empty());
  ASSERT_EQ(storage.size(), 0);
}

TEST(Storage, Clear_NotEmpty_Empty)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);

  storage.clear();

  ASSERT_TRUE(storage.empty());
  ASSERT_EQ(storage.size(), 0);
}

TEST(Storage, Insert_Double)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);

  ASSERT_TRUE(storage.contains(0));
  ASSERT_FALSE(storage.contains(1));

  storage.insert(1);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 2);
}

TEST(Storage, Erase_Double)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);
  storage.insert(1);

  storage.erase(0);

  ASSERT_FALSE(storage.contains(0));
  ASSERT_TRUE(storage.contains(1));

  storage.erase(1);

  ASSERT_TRUE(storage.empty());
  ASSERT_EQ(storage.size(), 0);
}

TEST(Storage, Insert_ReinsertionAfterClear)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);
  storage.clear();
  storage.insert(0);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
}

TEST(Storage, Insert_ReinsertionAfterErase)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  storage.insert(0);
  storage.erase(0);
  storage.insert(0);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
}

TEST(Storage, Insert_TriggerSparseGrowth)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  entity_type bigValue = 999999;

  storage.insert(bigValue);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
  ASSERT_TRUE(storage.contains(bigValue));
}

TEST(Storage, Insert_TriggerGrowth)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  entity_type amount = 10000;

  for (entity_type i = 0; i < amount; i++)
  {
    storage.insert(i);
  }

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), amount);

  for (entity_type i = 0; i < amount; i++)
  {
    ASSERT_TRUE(storage.contains(i));
  }

  ASSERT_FALSE(storage.contains(amount));
}

TEST(Storage, ShrinkToFit_MemoryOverhead)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<unsigned int>>;

  storage_type storage;

  entity_type amount = 10000;

  for (entity_type i = 0; i < amount; i++)
  {
    storage.insert(i, i);
  }

  ASSERT_NE(storage.capacity(), storage.size());

  storage.shrink_to_fit();

  ASSERT_EQ(storage.capacity(), storage.size());
}

TEST(Storage, Random)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<>>;

  storage_type storage;

  for (entity_type i = 0; i < 1000; i++)
  {
    storage.insert(i);
  }

  ASSERT_EQ(storage.size(), 1000);

  storage.clear();

  ASSERT_TRUE(storage.empty());

  for (entity_type i = 100; i < 2000; i++)
  {
    storage.insert(i);
  }

  ASSERT_EQ(storage.size(), 1900);

  for (entity_type i = 300; i < 400; i++)
  {
    storage.erase(i);
  }

  storage.shrink_to_fit();

  ASSERT_EQ(storage.size(), 1800);

  for (entity_type i = 325; i < 375; i++)
  {
    storage.insert(i);
  }

  ASSERT_EQ(storage.size(), 1850);

  for (entity_type i = 0; i < 100; i++)
  {
    ASSERT_FALSE(storage.contains(i));
  }

  for (entity_type i = 100; i < 300; i++)
  {
    ASSERT_TRUE(storage.contains(i));
  }

  for (entity_type i = 400; i < 2000; i++)
  {
    ASSERT_TRUE(storage.contains(i));
  }

  for (entity_type i = 0; i < 100; i++)
  {
    ASSERT_FALSE(storage.contains(i));
  }

  for (entity_type i = 300; i < 325; i++)
  {
    ASSERT_FALSE(storage.contains(i));
  }

  for (entity_type i = 375; i < 400; i++)
  {
    ASSERT_FALSE(storage.contains(i));
  }

  for (entity_type i = 325; i < 375; i++)
  {
    ASSERT_TRUE(storage.contains(i));
  }

  storage.insert(53);

  ASSERT_TRUE(storage.contains(53));

  storage.erase(53);

  ASSERT_FALSE(storage.contains(53));

  storage.insert(53);

  ASSERT_TRUE(storage.contains(53));

  storage.clear();

  ASSERT_TRUE(storage.empty());

  ASSERT_FALSE(storage.contains(53));
}

TEST(Storage, Iterator_CorrectIterations)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<unsigned int>>;

  storage_type storage;

  ASSERT_EQ(storage.begin(), storage.end());

  entity_type amount = 10000;

  for (entity_type i = 0; i < amount; i++)
  {
    storage.insert(i, i);
  }

  ASSERT_NE(storage.begin(), storage.end());

  unsigned int iterations = 0;

  for (auto it = storage.begin(); it != storage.end(); ++it)
  {
    ASSERT_TRUE(*it < amount);
    iterations++;
  }

  ASSERT_EQ(iterations, amount);
}

TEST(StorageWithData, Insert_OneComponent)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<int>>;

  storage_type storage;

  storage.insert(0, 99);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
  ASSERT_EQ(storage.unpack<int>(0), 99);
}

TEST(StorageWithData, Insert_OneComponentNonTrival)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<std::string>>;

  storage_type storage;

  storage.insert(0, std::string { "Test0" });

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
  ASSERT_EQ(storage.unpack<std::string>(0), "Test0");
}

TEST(StorageWithData, Insert_TwoComponents)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<int, float>>;

  storage_type storage;

  storage.insert(0, 99, 0.5f);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
  ASSERT_EQ(storage.unpack<int>(0), 99);
  ASSERT_EQ(storage.unpack<float>(0), 0.5f);
}

TEST(StorageWithData, Insert_TwoComponentsOneNonTrivial)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<int, std::string>>;

  storage_type storage;

  storage.insert(0, 99, std::string { "Test0" });

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
  ASSERT_EQ(storage.unpack<int>(0), 99);
  ASSERT_EQ(storage.unpack<std::string>(0), "Test0");
}

TEST(StorageWithData, Insert_TwoComponentsReinsertedAfterErase)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<int, float>>;

  storage_type storage;

  storage.insert(0, 99, 0.5f);
  storage.erase(0);
  storage.insert(0, 98, 0.4f);

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
  ASSERT_EQ(storage.unpack<int>(0), 98);
  ASSERT_EQ(storage.unpack<float>(0), 0.4f);
}

TEST(StorageWithData, Insert_TwoComponentsOnComponentNonTrivalReinsertedAfterErase)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<int, std::string>>;

  storage_type storage;

  storage.insert(0, 99, std::string { "Test0" });
  storage.erase(0);
  storage.insert(0, 98, std::string { "Test1" });

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), 1);
  ASSERT_EQ(storage.unpack<int>(0), 98);
  ASSERT_EQ(storage.unpack<std::string>(0), "Test1");
}

TEST(StorageWithData, Insert_TriggerGrowth)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<unsigned int>>;

  storage_type storage;

  entity_type amount = 10000;

  for (entity_type i = 0; i < amount; i++)
  {
    storage.insert(i, std::move(i));
  }

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), amount);

  for (entity_type i = 0; i < amount; i++)
  {
    ASSERT_TRUE(storage.contains(i));
    ASSERT_EQ(storage.unpack<unsigned int>(i), i);
  }

  ASSERT_FALSE(storage.contains(amount));
}

TEST(StorageWithData, Insert_NonTrivalTriggerGrowth)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<std::string>>;

  storage_type storage;

  entity_type amount = 10000;

  for (entity_type i = 0; i < amount; i++)
  {
    storage.insert(i, "Test" + std::to_string(i));
  }

  ASSERT_FALSE(storage.empty());
  ASSERT_EQ(storage.size(), amount);

  for (entity_type i = 0; i < amount; i++)
  {
    ASSERT_TRUE(storage.contains(i));
    ASSERT_EQ(storage.unpack<std::string>(i), ("Test" + std::to_string(i)));
  }

  ASSERT_FALSE(storage.contains(amount));
}

TEST(StorageWithData, Insert_NonTrivalDestructorOnly_CheckForLeaks)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<NonTrivialDestructorOnly>>;

  int destructor_count = 0;

  entity_type insert_amount = 10000;

  {
    storage_type storage;

    for (entity_type i = 0; i < insert_amount; i++)
    {
      NonTrivialDestructorOnly inserted { &destructor_count };

      storage.insert(i, inserted);

    } // Calls destructor

    ASSERT_EQ(destructor_count, insert_amount);

  } // Calls destructor

  ASSERT_EQ(destructor_count, insert_amount * 2);
}

TEST(StorageWithData, Insert_NonTrival_CheckForLeaks)
{
  using entity_type = unsigned int;
  using storage_type = storage<entity_type, archetype<NonTrivial>>;

  int constructor_count = 0;
  int destructor_count = 0;

  entity_type insert_amount = 10000;

  entity_type reinsert_amount = 500;

  {
    storage_type storage;

    for (entity_type i = 0; i < insert_amount; i++)
    {
      NonTrivial inserted { &constructor_count, &destructor_count }; // Calls constructor

      storage.insert(i, inserted); // Calls copy assignment

    } // Calls destructor

    ASSERT_GE(constructor_count, insert_amount);
    ASSERT_EQ(destructor_count, insert_amount);

    for (entity_type i = 0; i < reinsert_amount; i++)
    {
      storage.erase(i); // Calls destructor
    }

    ASSERT_GE(constructor_count, insert_amount);
    ASSERT_EQ(destructor_count, insert_amount + reinsert_amount);

    for (entity_type i = 0; i < reinsert_amount; i++)
    {
      NonTrivial inserted { &constructor_count, &destructor_count }; // Calls constructor

      storage.insert(i, inserted); // Calls copy assignment

    } // Calls destructor

    ASSERT_GE(constructor_count, insert_amount + reinsert_amount);
    ASSERT_EQ(destructor_count, insert_amount + reinsert_amount + reinsert_amount);

  } // Calls destructor

  ASSERT_GE(constructor_count, insert_amount + reinsert_amount);
  ASSERT_EQ(destructor_count, insert_amount * 2 + reinsert_amount * 2);
}

TEST(StorageSharedSparseArray, Share_SingleStorage_Shared)
{
  using entity_type = unsigned int;
  using sparse_type = sparse_array<entity_type>;
  using storage_type = storage<entity_type, archetype<>>;

  sparse_type shared;

  storage_type storage;

  ASSERT_FALSE(shared.shared());

  storage.share(&shared);
}

TEST(StorageSharedSparseArray, Unshare_SingleStorage_NotShared)
{
  using entity_type = unsigned int;
  using sparse_type = sparse_array<entity_type>;
  using storage_type = storage<entity_type, archetype<>>;

  sparse_type shared;

  {
    storage_type storage;

    storage.share(&shared);

    ASSERT_TRUE(shared.shared());

    // unshares at end of scope
  }

  ASSERT_FALSE(shared.shared());
}

TEST(StorageSharedSparseArray, Share_TwoStorages_Shared)
{
  using entity_type = unsigned int;
  using sparse_type = sparse_array<entity_type>;
  using storage_type = storage<entity_type, archetype<>>;

  sparse_type shared;

  storage_type storage1;
  storage_type storage2;

  storage1.share(&shared);
  storage2.share(&shared);

  ASSERT_TRUE(shared.shared());
  ASSERT_EQ(shared.shared(), 2);
}

TEST(StorageSharedSparseArray, Unshare_SingleStoragesOneUnshare_Shared)
{
  using entity_type = unsigned int;
  using sparse_type = sparse_array<entity_type>;
  using storage_type = storage<entity_type, archetype<>>;

  sparse_type shared;

  storage_type storage1;

  storage1.share(&shared);

  {
    storage_type storage2;

    storage2.share(&shared);

    ASSERT_EQ(shared.shared(), 2);
  }

  ASSERT_EQ(shared.shared(), 1);
}

TEST(StorageSharedSparseArray, Unshare_SingleStoragesOneUnshare_Throws)
{
  using entity_type = unsigned int;
  using sparse_type = sparse_array<entity_type>;
  using storage_type = storage<entity_type, archetype<>>;

  sparse_type shared;

  storage_type storage;

  storage.insert(99);

  void(0);

  ASSERT_FALSE(shared.shared());
}

TEST(StorageSharedSparseArray, Insert_TwoStoragesInsertSingle_ContainsBoth)
{
  using entity_type = unsigned int;
  using sparse_type = sparse_array<entity_type>;
  using storage_type = storage<entity_type, archetype<>>;

  sparse_type shared;

  storage_type storage1;
  storage_type storage2;

  storage1.share(&shared);
  storage2.share(&shared);

  storage1.insert(10);

  ASSERT_TRUE(storage1.contains(10));
  ASSERT_FALSE(storage2.contains(10));

  storage2.insert(20);

  ASSERT_TRUE(storage1.contains(10));
  ASSERT_FALSE(storage2.contains(10));
  ASSERT_FALSE(storage1.contains(20));
  ASSERT_TRUE(storage2.contains(20));
}

TEST(StorageSharedSparseArray, Insert_TwoStoragesInsertSingle_UsingSharedMemory)
{
  using entity_type = unsigned int;
  using sparse_type = sparse_array<entity_type>;
  using storage_type = storage<entity_type, archetype<>>;

  sparse_type shared;

  storage_type storage1;
  storage_type storage2;

  storage1.share(&shared);
  storage2.share(&shared);

  storage1.insert(100);

  ASSERT_TRUE(shared[100] == storage1.size() - 1);

  storage2.insert(100000);

  ASSERT_TRUE(shared[100000] == storage2.size() - 1);
  ASSERT_TRUE(shared[100] == storage1.size() - 1);

  storage1.insert(99999);

  ASSERT_TRUE(shared[100] == storage1.size() - 2);
  ASSERT_TRUE(shared[99999] == storage1.size() - 1);
  ASSERT_TRUE(shared[100000] == storage2.size() - 1);

  storage2.insert(453);

  ASSERT_TRUE(shared[100] == storage1.size() - 2);
  ASSERT_TRUE(shared[99999] == storage1.size() - 1);
  ASSERT_TRUE(shared[100000] == storage2.size() - 2);
  ASSERT_TRUE(shared[453] == storage2.size() - 1);
}
