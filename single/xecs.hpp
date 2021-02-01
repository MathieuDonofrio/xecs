#ifndef XECS_ARCHETYPE_HPP
#define XECS_ARCHETYPE_HPP

#include <cstdlib>
#include <limits>
#include <tuple>
#include <type_traits>

namespace xecs
{
/**
 * @brief List of types.
 * 
 * Similar to a tuple, but cannot contain any data. 
 * This is used heavily for our compile-time meta programming.
 * 
 * @tparam Types All the types to be contained by the list
 */
template<typename... Types>
struct list
{};

/**
 * @brief Finds size of a list (amount of types).
 * 
 * @tparam List List of types
 */
template<typename List>
struct size;

template<typename... Types, template<typename...> class List>
struct size<List<Types...>> : std::integral_constant<size_t, sizeof...(Types)>
{};

template<typename List>
constexpr auto size_v = size<List>::value;

/**
 * @brief Checks if a list is empty.
 * 
 * @tparam List List of types
 */
template<typename List>
struct empty;

template<typename... Types, template<typename...> class List>
struct empty<List<Types...>> : std::bool_constant<sizeof...(Types) == 0>
{};

template<typename List>
constexpr auto empty_v = empty<List>::value;

/**
 * @brief Finds the index of the specified type in the list.
 * 
 * Returns size of list if nothing was found.
 * 
 * @tparam Type the type to find
 * @tparam List The list to operate on
 * @tparam I Current search index (default start at 0)
 */
template<typename Type, typename List, size_t I = 0>
struct find;

template<typename Type, typename... Types, template<typename...> class List, size_t I>
struct find<Type, List<Types...>, I> : std::integral_constant<size_t, I>
{};

template<typename Type, typename Head, typename... Types, template<typename...> class List, size_t I>
struct find<Type, List<Head, Types...>, I>
  : std::conditional_t<std::is_same_v<Type, Head>, std::integral_constant<size_t, I>, find<Type, List<Types...>, I + 1>>
{};

template<typename Type, typename List>
constexpr auto find_v = find<Type, List, 0>::value;

/**
 * @brief Finds the type at the specified index in the list.
 * 
 * @tparam I Index in the list
 * @tparam List The list to operate on
 */
template<size_t I, typename List>
struct at;

template<size_t I, typename... Types, template<typename...> class List>
struct at<I, List<Types...>>
{
  using type = List<>;
};

template<size_t I, typename Type, typename... Types, template<typename...> class List>
struct at<I, List<Type, Types...>>
{
private:
  using next = typename at<I - 1, List<Types...>>::type;

public:
  using type = typename std::conditional_t<I == 0, Type, next>;
};

template<size_t I, typename List>
using at_t = typename at<I, List>::type;

/**
 * @brief Checks if a list contains a type.
 * 
 * @tparam Type Type to check for
 * @tparam List List to check
 */
template<typename Type, typename List>
struct contains;

template<typename Type, typename... Types, template<typename...> class List>
struct contains<Type, List<Types...>> : std::disjunction<std::is_same<Type, Types>...>
{};

/**
 * @brief Utility for contains value.
 * 
 * @tparam Type Type to check for
 * @tparam List List to check
 */
template<typename Type, typename List>
constexpr auto contains_v = contains<Type, List>::value;

/**
 * @brief Checks if list contains atleast all required types.
 * 
 * @tparam List List to check
 * @tparam Types Required types
 */
template<typename List, typename... Types>
struct contains_all;

template<typename List, typename... Types>
struct contains_all : std::conjunction<contains<Types, List>...>
{};

template<typename List, typename... Types>
constexpr auto contains_all_v = contains_all<List, Types...>::value;

/**
 * @brief Checks if all types are unique.
 * 
 * Types are determined not to be the same by std::is_same<T1, T2>.
 * 
 * @tparam Types Types to check
 */
template<typename... Types>
struct unique_types : std::true_type
{};

template<typename Type, typename... Types>
struct unique_types<Type, Types...>
  : std::conjunction<std::negation<std::disjunction<std::is_same<Type, Types>...>>, unique_types<Types...>>
{};

template<typename... Types>
constexpr auto unique_types_v = unique_types<Types...>::value;

/**
 * @brief Checks if two lists contain the same types.
 * 
 * Compares the size of both lists and checks if Blist contains all the types of AList.
 * 
 * @tparam AList First list of types
 * @tparam BList Second list of types
 */
template<typename AList, typename BList>
struct is_same_types;

template<typename... ATypes, template<typename...> class AList, typename... BTypes, template<typename...> class BList>
struct is_same_types<AList<ATypes...>, BList<BTypes...>>
  : std::conjunction<std::bool_constant<sizeof...(ATypes) == sizeof...(BTypes)>, contains_all<AList<ATypes...>, BTypes...>>
{};

template<typename AList, typename BList>
constexpr auto is_same_types_v = is_same_types<AList, BList>::value;

/**
 * @brief Checks if all lists are unique in terms of types.
 * 
 * This is different than unique_types because it ignores the order of
 * types contained by a list. unique_types would evaluate a list with the same types but in a
 * different order as a different list.
 * 
 * @tparam Lists A list of lists
 */
template<typename... Lists>
struct unique_lists : std::true_type
{};

template<typename List, typename... Lists>
struct unique_lists<List, Lists...>
  : std::conjunction<std::negation<std::disjunction<is_same_types<List, Lists>...>>, unique_lists<Lists...>>
{};

template<typename... Lists>
constexpr auto unique_lists_v = unique_lists<Lists...>::value;

/**
 * @brief Adds a type to the front of a list.
 * 
 * @tparam Type Type to add
 * @tparam List List to add type to
 */
template<typename Type, typename List>
struct push_front;

template<typename Type, typename... Types, template<typename...> class List>
struct push_front<Type, List<Types...>>
{
  using type = List<Type, Types...>;
};

template<typename Type, typename List>
using push_front_t = typename push_front<Type, List>::type;

/**
 * @brief Adds a type to the back of a list.
 * 
 * @tparam Type Type to add
 * @tparam List List to add type to
 */
template<typename Type, typename List>
struct push_back;

template<typename Type, typename... Types, template<typename...> class List>
struct push_back<Type, List<Types...>>
{
  using type = List<Types..., Type>;
};

template<typename Type, typename List>
using push_back_t = typename push_back<Type, List>::type;

/**
 * @brief Finds the first occurence of a list that.
 * 
 * This uses is_same_types, so the list to be found must contain all the specified types,
 * and only those types, nothing more, nothing less.
 * 
 * This is used to find the correct type of list with the specified components for any given order.
 * 
 * @tparam ListOfLists A List of lists to search
 * @tparam Types Exact contained types of the list to find.
 */
template<typename ListOfLists, typename... Types>
struct find_for;

template<typename... Lists, template<typename...> class ListOfLists, typename... Types>
struct find_for<ListOfLists<Lists...>, Types...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> class ListOfLists, typename... Types>
struct find_for<ListOfLists<HeadList, Lists...>, Types...>
{
private:
  using next = typename find_for<ListOfLists<Lists...>, Types...>::type;

public:
  using type = typename std::conditional_t<is_same_types_v<HeadList, list<Types...>>, HeadList, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using find_for_t = typename find_for<ListOfLists, RequiredTypes...>::type;

/**
 * @brief Removes all lists that do not contains the required types.
 * 
 * This is used to create views at compile time.
 * 
 * @tparam ListOfLists A List of lists to prune
 * @tparam RequiredTypes types that must be present in the list
 */
template<typename ListOfLists, typename... RequiredTypes>
struct prune_for;

template<typename... Lists, template<typename...> class ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<Lists...>, RequiredTypes...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> class ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<HeadList, Lists...>, RequiredTypes...>
{
private:
  using next = typename prune_for<ListOfLists<Lists...>, RequiredTypes...>::type;
  using to_front = typename push_front<HeadList, next>::type; // Can make certain operations O(1)
  using to_back = typename push_back<HeadList, next>::type;
  using accept = typename std::conditional_t<size_v<HeadList> == sizeof...(RequiredTypes), to_front, to_back>;

public:
  using type = typename std::conditional_t<contains_all<HeadList, RequiredTypes...>::value, accept, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using prune_for_t = typename prune_for<ListOfLists, RequiredTypes...>::type;

/**
 * @brief Tries to place the closes list to the specified types in front.
 * 
 * This can allow certain operations to be made O(1).
 * 
 * @warning Does not do a compleate sort.
 * 
 * @tparam ListOfLists A List of lists arrange
 * @tparam Types Types that the lists of lists gets arranged for.
 */
template<typename ListOfLists, typename... Types>
struct optimize_order;

/**
 * @brief Assert's a component to verify that is is valid.
 * 
 * A valid component cannot be cv-qualified, that means it cant be const or volatile. And most importantly,
 * a component needs to be trivial, that means it cant contains constructors, destructors... EVERYTHING must
 * be trivial.
 * 
 * Components must be strictly data. And we restrict at compile-time going beyond that.
 * These restrictions allows us to make some extra optimizations and enforces a performance by default philosophie.
 * 
 * @tparam Component The component to verify
 */
template<typename Component>
struct verify_component
{
  static_assert(std::is_same_v<Component, std::remove_cv_t<Component>>, "Component cannot be cv-qualified (const or volatile)");
};

template<typename... Components>
using archetype = list<Components...>;

/**
 * @brief Assert's a archetype to verify that it is valid.
 * 
 * A valid archetype must contain only valid components determined by verify_component. Additionnaly,
 * all components in an archetype must be unique.
 * 
 * Archetypes are essentially sets of components.
 * 
 * @tparam Archetype The archetype to verify
 */
template<typename Archetype>
struct verify_archetype;

template<typename... Components>
struct verify_archetype<archetype<Components...>> : verify_component<Components>...
{
  static_assert(unique_types_v<Components...>, "Every component must be unique (archetype is a SET of components)");
};

/**
 * @brief Assert's a list of archetypes.
 * 
 * A valid list of archetypes must contain only valid archetypes determined by verify_archetype. Additionnaly,
 * all archetypes in the list must be unique.
 * 
 * For two archetypes to be different, they must be of different sizes or contains different components.
 * Archetypes with the same components but in different orders do not count as unique.
 * 
 * @tparam ArchetypeList The archetype list to verify
 */
template<typename ArchetypeList>
struct verify_archetype_list;

template<typename... Archetypes>
struct verify_archetype_list<list<Archetypes...>> : verify_archetype<Archetypes>...
{
  static_assert(unique_lists_v<Archetypes...>, "Every archetype must be unique (regardless of component order)");
};

namespace internal
{
  template<typename... Archetypes>
  struct archetype_list_builder
  {
    template<typename Archetype>
    using add = archetype_list_builder<Archetype, Archetypes...>;
    using build = list<Archetypes...>;
  };
} // namespace internal

/**
 * @brief Compile-time builder pattern for creating lists of archetypes.
 * 
 * Alternative to building archetype lists manually. This may provide better readability.
 */
struct archetype_list_builder : internal::archetype_list_builder<>
{};
} // namespace xecs

#endif

#ifndef XECS_ENTITY_MANAGER_HPP
#define XECS_ENTITY_MANAGER_HPP

#include <array>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>

#define ENTITY_MANAGER_STACK_SIZE 16384 // This should not be changed unless you know what your doing

static_assert((ENTITY_MANAGER_STACK_SIZE & (ENTITY_MANAGER_STACK_SIZE - 1)) == 0,
  "ENTITY_MANAGER_STACK_SIZE must be a power of two");

namespace xecs
{
/**
 * @brief Manager responsible for distributing entities.
 * 
 * An entity manager is essentially a class responsible for generating and recycling entities.
 * Entities are simply just identifiers.
 * 
 * For an enitity to be valid is must be an unsigned integer.
 * 
 * The maximum amount of entities that can exist is equal to the maximum value of the entity. 16 bit identifiers
 * can allow for a maximum of 65535 entities. A 32 bit identifier is much larger and can support 4,294,967,295 entities.
 * Anything more than 32 bit is probably overkill for the any game or simulation.
 * 
 * The entity manager is just a counter with a stack to be able to recycle entities.
 * 
 * This particular entity manager optimizes to use stack memory (could also be static memory). It contains 
 * a small stack allocated on the stack and a bigger stack allocated on the heap. The size of the stack memory stack
 * is defined by ENTITY_MANAGER_STACK_SIZE. When the size of recycled entities exceed this amount, the entity manager
 * will then go to the heap. The manager will always priorize fetching from the stack. It is possible to swap recycled values
 * accumulated in the heap memory stack into the stack memory stack
 * 
 * @tparam Entity unsigned integer type to represent entity
 */
template<typename Entity>
class entity_manager
{
public:
  using entity_type = Entity;
  using size_type = size_t;

  static_assert(std::numeric_limits<entity_type>::is_integer && !std::numeric_limits<entity_type>::is_signed,
    "Entity type must be an unsigned integer");

  /**
   * @brief Fixed capacity of entities for stack memory stack.
   * 
   */
  static constexpr size_type stack_capacity = ENTITY_MANAGER_STACK_SIZE / sizeof(entity_type);

  /**
   * @brief Minimum capacity on entities for the heap memory stack.
   * 
   * Twice as big as stack.
   */
  static constexpr size_type minimum_heap_capacity = stack_capacity * 2;

public:
  using stack_buffer_type = entity_type[stack_capacity];
  using heap_buffer_type = entity_type*;

  /**
   * @brief Construct a new entity manager object
   * 
   */
  entity_manager()
    : _current(0), _stack_reusable(0), _heap_reusable(0), _heap_capacity(minimum_heap_capacity), _stack_buffer()
  {
    _heap_buffer = static_cast<heap_buffer_type>(std::malloc(minimum_heap_capacity * sizeof(entity_type)));
  }

  /**
   * @brief Destroy the entity manager object
   */
  ~entity_manager()
  {
    free(_heap_buffer);
  }

  entity_manager(const entity_manager&) = delete;
  entity_manager(entity_manager&&) = delete;
  entity_manager& operator=(const entity_manager&) = delete;
  entity_manager& operator=(entity_manager&&) = delete;

  /**
   * @brief Generates a unique entity.
   * 
   * This will try to obtain a recycled entity, but if none are available then the
   * internal counter will be incremented.
   * 
   * @return entity_type The entity identifier generated
   */
  entity_type generate()
  {
    if (_stack_reusable) return _stack_buffer[--_stack_reusable];
    else if (_heap_reusable)
      return _heap_buffer[--_heap_reusable];
    else
      return _current++;
  }

  /**
   * @brief Allows an entity to be reused.
   * 
   * This will add the entity to pools of reusable entities.
   * 
   * @param entity Entity to release
   */
  void release(entity_type entity)
  {
    if (_stack_reusable < stack_capacity) _stack_buffer[_stack_reusable++] = entity;
    else
    {
      // Heap resizing should not happen very often
      if (_heap_reusable == _heap_capacity)
      {
        // Grow by a factor of 1.25
        // This is ok since we know the heap capacity starts off as a large amount
        _heap_capacity = (_heap_capacity * 5) / 3;
        _heap_buffer = static_cast<heap_buffer_type>(std::realloc(_heap_buffer, _heap_capacity * sizeof(entity_type)));
      }
      _heap_buffer[_heap_reusable++] = entity;
    }
  }

  /**
   * @brief releases all entities at once.
   * 
   * Resets the internal counter and clears reusable entities.
   * 
   * This is a very cheap O(1) operation.
   */
  void release_all()
  {
    _stack_reusable = 0;
    _heap_reusable = 0;
    _current = 0;
  }

  /**
   * @brief moves reusable heap memory entites into stack memory as best as possible.
   * 
   * This can be good to call every once in a while to insure that we use stack memory
   * as much as possible by moving the entities accumulated in the heap.
   */
  void swap()
  {
    if (_heap_reusable && _stack_reusable != stack_capacity)
    {
      const auto stack_space = stack_capacity - _stack_reusable;
      const auto swap_amount = _heap_reusable < stack_space ? _heap_reusable : stack_space;

      void* dst_stack = static_cast<void*>(static_cast<entity_type*>(_stack_buffer) + _stack_reusable);
      void* src_heap = _heap_buffer + _heap_reusable - swap_amount;

      std::memcpy(dst_stack, src_heap, swap_amount * sizeof(entity_type));

      _stack_reusable += swap_amount;
      _heap_reusable -= swap_amount;
    }
  }

  /**
   * @brief Resizes the heap memory stack to be as small possible.
   * 
   * The heap memory cannot be smaller than minimum_heap_capacity.
   * 
   * This is good to call every once in a while to optimize memory usage.
   */
  void shrink_to_fit()
  {
    if (_heap_reusable != _heap_capacity && _heap_reusable > minimum_heap_capacity)
    {
      _heap_capacity = _heap_reusable;

      _heap_buffer = static_cast<heap_buffer_type>(std::realloc(_heap_buffer, _heap_capacity * sizeof(entity_type)));
    }
  }

  /**
   * @brief Reveals the current value of the internal counter.
   * 
   * @warning This value is not guaranted to be the next value to be generated.
   * 
   * @return entity_type Next entity for internal counter
   */
  [[nodiscard]] entity_type peek() const { return _current; }

  /**
   * @brief Returns the amount of reusable entities that are in stack memory.
   * 
   * @return size_type Amount of reusable entites in stack memory
   */
  [[nodiscard]] size_type stack_reusable() const { return _stack_reusable; }

  /**
   * @brief Returns the amount of reusable entities that are in heap memory.
   * 
   * @return size_type amount of reusable entites in heap memory
   */
  [[nodiscard]] size_type heap_reusable() const { return _heap_reusable; }

  /**
   * @brief Returns the total amount of reusable entities.
   * 
   * @return size_type Reusable entities
   */
  [[nodiscard]] size_type reusable() const { return _stack_reusable + _heap_reusable; }

  /**
   * @brief Returns the current capacity of the heap memory stack.
   * 
   * @return size_type Heap memory stack capacity
   */
  [[nodiscard]] size_type heap_capacity() const { return _heap_capacity; }

private:
  entity_type _current;

  size_type _stack_reusable;
  size_type _heap_reusable;
  size_type _heap_capacity;

  heap_buffer_type _heap_buffer;
  stack_buffer_type _stack_buffer;
};
} // namespace xecs

#endif

#ifndef XECS_STORAGE_HPP
#define XECS_STORAGE_HPP

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>

namespace xecs
{
/**
 * @brief Array that sparsely stores indexes towards another array.
 * 
 * You could think of the sparse array as an unordered map where the key is an unsigned int entity
 * and the value is a unsigned int index (size_t). The sparse_array uses more memory 
 * than a unordered map but is much faster.
 * 
 * This class is used by the storage (a sparse set) but is implemented seperatly to be able to
 * have multiple storages that share the same sparse_array. This reduces memory usage when there are
 * multiple storages, making them more scalable and efficient. All storages that use entites generated
 * by the same entity manager can share the same sparse_array (one registry has one sparse_array).
 * 
 * Paging is not nessesary here because if implmented correctly there should only be one sparse_array
 * per entity_manager.
 * 
 * @tparam Entity unsigned int entity identifier
 */
template<typename Entity>
class sparse_array final
{
public:
  using entity_type = Entity;
  using size_type = size_t;
  using array_type = entity_type*;
  using shared_count_type = uint16_t;

  static_assert(std::numeric_limits<entity_type>::is_integer && !std::numeric_limits<entity_type>::is_signed,
    "Entity type must be an unsigned integer");

  /**
   * @brief Construct a new sparse array object
   */
  sparse_array()
    : _capacity(32), _shared(0)
  {
    _array = static_cast<array_type>(std::malloc(_capacity * sizeof(entity_type)));
  }

  /**
   * @brief Destroy the sparse array object
   */
  ~sparse_array()
  {
    free(_array);
  }

  sparse_array(const sparse_array&) = delete;
  sparse_array(sparse_array&&) = delete;
  sparse_array& operator=(const sparse_array&) = delete;
  sparse_array& operator=(sparse_array&&) = delete;

  /**
   * @brief Assures that the sparse array can contain the entity.
   * 
   * If the sparse_array cannot contain the entity, this will trigger
   * a resize.
   * 
   * @param page Page to assure
   */
  void assure(const entity_type entity)
  {
    if (entity >= _capacity)
    {
      const auto linear = entity + (1024 / sizeof(entity_type)); // 1kb
      const auto exponential = _capacity << 1; // Double capacity

      _capacity = entity >= exponential ? linear : exponential;

      _array = static_cast<array_type>(std::realloc(_array, _capacity * sizeof(entity_type)));
    }
  }

  /**
   * @brief Returns the page for the page index.
   * 
   * @param page Page index
   * @return page_type Array of indexes
   */
  entity_type operator[](const entity_type entity) const { return _array[entity]; }

  /*! @copydoc operator[] */
  entity_type& operator[](const entity_type entity) { return _array[entity]; }

  /**
   * @brief Returns the capacity of the sparse_array.
   * 
   * If an entity identifier bigger than this amount is to be inserted into the
   * sparse_array, a resize will be required.
   * 
   * @return size_type Capacity of the sparse_array
   */
  size_type capacity() const { return _capacity; }

  /**
   * @brief Signals that a storage is sharing this sparse_array
   */
  void share() { ++_shared; }

  /**
   * @brief Signals that a storage is unsharing this sparse_array
   */
  void unshare() { --_shared; }

  /**
   * @brief Returns the amount of storages that are sharing this sparse_array
   * @return shared_count_type Amount of storages that share this sparse_array
   */
  shared_count_type shared() const { return _shared; }

private:
  array_type _array;
  size_type _capacity;
  shared_count_type _shared;
};

/**
 * @brief Collection of entites of an archetype and its components.
 * 
 * The storage is essentially a sparse set fine-tuned for our needs. This is not a general
 * purpose sparse set.
 * 
 * Sparse sets are a well documented data structure. Basically, it allows us to have very fast O(1) random insertion,
 * deletion and much more. But most importantly, our data is stored contigioustly in memory, so we can iterate
 * on it as fast as you can a vector.
 * 
 * Sparse sets work by having two arrays, a densly packed one and the sparse one (sparse_array). The sparse array 
 * is used access data with a level of indirection and is used to make operations like insert and erase O(1). The
 * dense array is what we iterate on and it is always densly packed. For more information on a sparse set just look it
 * up, its very widely used.
 * 
 * Our storage is built on top of a sparse set and allows us to store all the components of an archetype.
 * So instead of having one sparse set per component, we have one sparse set per archetype. This means that all the
 * components of an archetype are stored in the correct order. This is a huge part of our ecs implementation,
 * this allows us to always iterate on multiple components perfectly contiguously without any holes or branching checks.
 * There is very little extra cost for iterating over more than one component.
 * 
 * @note Supports storing non-trivial types, however this is not recommended for performance.
 * 
 * @warning Order is never guaranted.
 * 
 * @tparam Entity unsigned integer entity identifier to store
 * @tparam Archetype list of components to store
 */
template<typename Entity, typename Archetype>
class storage;

template<typename Entity, typename... Components>
class storage<Entity, archetype<Components...>> final
{
public:
  using entity_type = Entity;
  using size_type = size_t;

private:
  using dense_type = entity_type*;
  using page_type = entity_type*;
  using sparse_type = sparse_array<Entity>*;
  using component_pool_type = std::tuple<Components*...>;

  static_assert(std::numeric_limits<entity_type>::is_integer && !std::numeric_limits<entity_type>::is_signed,
    "Entity type must be an unsigned integer");

public:
  class iterator;

  /**
   * @brief Construct a new storage object
   */
  storage()
    : _size(0), _capacity(4)
  {
    _sparse = new sparse_array<entity_type>();

    _dense = static_cast<dense_type>(std::malloc(_capacity * sizeof(entity_type)));
    (alloc_array<Components>(), ...);
  }

  /**
   * @brief Destroy the storage object
   */
  ~storage()
  {
    // We only manage our sparse_array memory if its not shared
    if (_sparse->shared()) _sparse->unshare();
    else
      delete _sparse;

    free(_dense);
    (free_array<Components>(), ...);
  }

  storage(const storage&) = delete;
  storage(storage&&) = delete;
  storage& operator=(const storage&) = delete;
  storage& operator=(storage&&) = delete;

  /**
   * @brief Inserts a entity and all its components at once.
   * 
   * Entities are always inserted in the back of the array, however they are not
   * guarented to stay there, and can be moved around by other operations.
   * 
   * You can directly add the components with this method. However, components will be copied. 
   * This is not nessesary and can be done later very efficiently using the unpack method. 
   * Also, the order of the components dont matter, as long as they are all unique and part of the archetype.
   * 
   * This operation is usually O(1) and is pretty cheap. Some insert operations may be slower
   * if any internal array need a resize.
   * 
   * @warning Undefined behaviour if the entity already exists. If you dont know
   * if the entity exists, call the contains method first.
   * 
   * @tparam IncludedComponents Types of components to insert with (optional).
   * @param entity Entity to insert
   * @param components Components to insert alongside entity
   */
  template<typename... IncludedComponents>
  void insert(const entity_type entity, const IncludedComponents&... components)
  {
    static_assert(contains_all_v<list<Components...>, IncludedComponents...>,
      "One or more included components do not belong to the archetype");
    static_assert(unique_types_v<IncludedComponents...>,
      "Included components are not unique");

    if (_size == _capacity) grow();
    _sparse->assure(entity);

    _dense[_size] = entity;

    ((access<IncludedComponents>()[_size] = components), ...);

    (*_sparse)[entity] = static_cast<entity_type>(_size++);
  }

  /**
   * @brief Erases an entity from the storage.
   * 
   * Erasing entities is essentially just poping an entity from the back of the array and moving
   * it to the location of the entity to erase.
   * 
   * This is a very cheap O(1) operation.
   * 
   * @warning Undefined behaviour if the entity does not exist. If you dont know
   * if the entity exists, call the contains method first.
   * 
   * @param entity Entity to erase
   */
  void erase(const entity_type entity)
  {
    const auto back_entity = _dense[--_size];
    const auto index = (*_sparse)[entity];

    (*_sparse)[back_entity] = index;

    _dense[index] = back_entity;

    ((access<Components>()[index] = std::move(access<Components>()[_size])), ...);
  }

  /**
   * @brief Returns whether or not an entity is present in the storage.
   * 
   * This operation is a very cheap O(1) operation.
   * 
   * @param entity Entity to check
   * @return true If entity is present in storage, false otherwise
   */
  bool contains(const entity_type entity) const
  {
    size_type index;

    // We must access the dense array here because our sparse arrays may be shared, therefor we need
    // to make sure entity index is valid.
    return (index = (*_sparse)[entity]) < _size && _dense[index] == entity;
  }

  /**
   * @brief Returns a reference of the stored component for the specified entity and component type.
   * 
   * This operation simply optains the index of the entity and access the dense array that holds
   * the component of requested type.
   * 
   * Very cheap operation, however unpacking from the iterator doesn't require recalculating the index
   * every time, so try to prioritize that (even if its a very cheap to find the index).
   * 
   * @tparam Component Type of component to unpack
   * @param entity Entity to unpack component for
   * @return Component& Reference to component belonging to the entity
   */
  template<typename Component>
  Component& unpack(const entity_type entity)
  {
    static_assert(contains_v<Component, list<Components...>>,
      "The component your trying to unpack does not belong to the archetype");

    return access<Component>()[(*_sparse)[entity]];
  }

  /**
   * @brief Resizes every internal dense array to be as small as possible.
   * 
   * This is good to call every once in a while to optimize memory usage.
   * 
   * @note This does not resize the sparse_array
   */
  void shrink_to_fit()
  {
    if (_size != _capacity)
    {
      _capacity = _size;

      _dense = static_cast<dense_type>(std::realloc(_dense, _capacity * sizeof(entity_type)));
      (realloc_array<Components>(), ...);
    }
  }

  /**
   * @brief Binds the shared sparse_array to this storage.
   * 
   * Sharing sparse_arrays between storages that operate on entities distributed by the
   * same entity_manager is recommended for performance and memory usage.
   * 
   * @warning Attemping to share a sparse array while already containing entities
   * will no nothing.
   * 
   * @param sparse The sparse_array to share with this storage
   */
  void share(sparse_type sparse)
  {
    if (_size != 0) return;

    if (_sparse->shared()) _sparse->unshare();
    else
      // If the sparse_array is not shared that means we current have the default sparse_array
      // allocated at initialization and we must delete it.
      delete _sparse;

    _sparse = sparse;
    sparse->share();
  }

  /**
   * @brief clears the entire storage
   * 
   * As cheap of an operation as you can get (sets size to zero).
   */
  void clear() { _size = 0; }

  /**
   * @brief Returns an iterator of the first entity of the dense array.
   * 
   * If the storage is empty the begining with be equal to the end.
   * 
   * @return iterator Dense array iterator begining
   */
  iterator begin() { return { this, _size - 1 }; }

  /**
   * @brief Returns an iterator at the last entity of the dense array.
   * 
   * If the storage is empty the begining with be equal to the end.
   * 
   * @return iterator Dense array iterator end
   */
  iterator end() { return { this, static_cast<size_type>(-1) }; }

  /**
   * @brief Returns the amount of entites current held by the storage.
   * 
   * @return size_type Amount of entities currently in storage
   */
  [[nodiscard]] size_type size() const { return _size; }

  /**
   * @brief Returns the current entity capacity of the storage.
   * 
   * If this is equal to the size, the next insert operation if no entites
   * are removed will result in a resize of all internal dense arrays.
   * 
   * @return size_type Current capacity of entities in storage
   */
  [[nodiscard]] size_type capacity() const { return _capacity; }

  /**
   * @brief Returns whether or not the storage is empty.
   * 
   * The storage is empty when there are not entities currently being
   * stored in it.
   * 
   * @return true If the storage is empty, false otherwise
   */
  [[nodiscard]] bool empty() const { return _size == 0; }

private:
  /**
   * @brief Grows the sparse set allocated space.
   * 
   * This grows the dense entity array and all the dense component arrays.
   * 
   * Growth is exponential with a small linear amount.
   */
  void grow()
  {
    // This is essentially _capacity * 1.5 + 8
    // There may be a better way to grow
    _capacity = (_capacity * 3) / 2 + 8;

    _dense = static_cast<dense_type>(std::realloc(_dense, _capacity * sizeof(entity_type)));
    (realloc_array<Components>(), ...);
  }

  /**
   * @brief Resizes the dense array for the specfied component type to the current capacity.
   * 
   * Uses realloc if the component type is trivial, otherwise will use new and moves data using
   * std::copy and move iterators.
   * 
   * @note Using realloc is faster.
   * 
   * @tparam Component The component type of the dense array to resize.
   */
  template<typename Component>
  void realloc_array()
  {
    if constexpr (std::is_trivial_v<Component>)
      access<Component>() = static_cast<Component*>(std::realloc(access<Component>(), _capacity * sizeof(Component)));
    else
    {
      Component* new_array = new Component[_capacity];
      std::copy(std::make_move_iterator(access<Component>()), std::make_move_iterator(access<Component>() + _size), new_array);
      delete[] access<Component>();
      access<Component>() = new_array;
    }
  }

  /**
   * @brief Allocates a new dense array for the specified component type.
   * 
   * Only used by the constructor.
   * 
   * @note Even if the default capacity is 0 this should be called.
   * 
   * @tparam Component The component type of the dense array to allocate.
   */
  template<typename Component>
  void alloc_array()
  {
    if constexpr (std::is_trivial_v<Component>)
      access<Component>() = static_cast<Component*>(std::malloc(_capacity * sizeof(Component)));
    else
      access<Component>() = new Component[_capacity];
  }

  /**
   * @brief Deallocates the dense array for the specified component type.
   * 
   * Only used by the destructor.
   * 
   * @tparam Component The component type of the dense array to deallocate.
   */
  template<typename Component>
  void free_array()
  {
    if constexpr (std::is_trivial_v<Component>) free(access<Component>());
    else
      delete[] access<Component>();
  }

  /**
   * @brief Accesses the component dense array for the specified component type.
   * 
   * @note Uses the tuple std::get method.
   * 
   * @tparam Component Type of component to access dense array for.
   * @return Component*& Dense array of component
   */
  template<typename Component>
  Component*& access()
  {
    static_assert(contains_v<Component, list<Components...>>,
      "The component type your trying to access does not belong to the archetype");

    return std::get<Component*>(_pool);
  }

private:
  dense_type _dense;
  sparse_type _sparse;
  component_pool_type _pool;

  size_type _size;
  size_type _capacity;
};

template<typename Entity, typename... Components>
class storage<Entity, archetype<Components...>>::iterator final
{
public:
  using iterator_category = std::random_access_iterator_tag;

  iterator(storage* const ptr, const size_type pos)
    : _ptr { ptr }, _pos { pos }
  {}

  // clang-format off
  iterator& operator+=(const size_type value) { _pos -= value; return *this; }
  iterator& operator-=(const size_type value) { _pos += value; return *this; }
  // clang-format on

  iterator& operator++() { return --_pos, *this; }
  iterator& operator--() { return ++_pos, *this; }

  iterator operator+(const size_type value) const { return { _ptr, _pos - value }; }
  iterator operator-(const size_type value) const { return { _ptr, _pos + value }; }

  bool operator==(const iterator other) const { return other._pos == _pos; }
  bool operator!=(const iterator other) const { return other._pos != _pos; }

  bool operator<(const iterator other) const { return other._pos > _pos; }
  bool operator>(const iterator other) const { return other._pos < _pos; }

  bool operator<=(const iterator other) const { return other._pos >= _pos; }
  bool operator>=(const iterator other) const { return other._pos <= _pos; }

  /**
   * @brief Returns the entity identifier for the current position of the iterator.
   * 
   * @note You can obtain the component with the unpack<Component> method.
   * 
   * @return const entity_type Entity at current iterator position
   */
  [[nodiscard]] entity_type operator*() const { return _ptr->_dense[_pos]; }

  /**
   * @brief Returns a reference the the component for current entity and specified component type.
   * 
   * @note You can obtain the current entity with the operator*.
   * 
   * Unpacking using this method is recommended above all other unpack methods. Even if performance is 
   * going to be similar, unpacking with this method is the cheapest because it doesn't need to calculate
   * the index.
   * 
   * @tparam Component Type of component to unpack
   * @return Component& Reference to component belonging to the entity
   */
  template<typename Component>
  [[nodiscard]] const Component& unpack() const
  {
    static_assert(contains_v<Component, list<Components...>>,
      "The component your trying to unpack does not belong to the archetype");

    return _ptr->access<Component>()[_pos];
  }

  /*! @copydoc unpack */
  template<typename Component>
  [[nodiscard]] Component& unpack()
  {
    return const_cast<Component&>(const_cast<const iterator*>(this)->unpack<Component>());
  }

private:
  storage* const _ptr;
  size_type _pos;
};
} // namespace xecs

#endif

#ifndef XECS_REGISTRY_HPP
#define XECS_REGISTRY_HPP

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

namespace xecs
{
/**
 * @brief Entity-component system core contaner.
 * 
 * A registry contains a entity_manager, one or many storages depending on the amount of archetypes, and one
 * sparse_array that is shared with all storages.
 * 
 * The registry is the glue that puts everything together in a wonderfull way and abstracts away
 * much of the managment of your archetypes. You can create views of this registry for the components
 * you need, and operate on those views.
 * 
 * Because of the usage of archetypes, this entity-component system is very scalable and does not need
 * any special optimizations to run as fast as possible. Everything is efficient by default.
 * 
 * This registry leverages its knowledge of all achetypes at compile time, to reduce 
 * the complexity of many operations, who often times can be reduced to nearly no overhead.
 * 
 * @tparam Entity The unsigned integer entity type
 * @tparam ArchetypeList The list of all archetypes to be used by this registry
 */
template<typename Entity, typename ArchetypeList>
class registry;

template<typename Entity, typename... Archetypes>
class registry<Entity, list<Archetypes...>> : verify_archetype_list<list<Archetypes...>>
{
public:
  using entity_type = Entity;
  using archetype_list_type = list<Archetypes...>;
  using registry_type = registry<entity_type, archetype_list_type>;
  using pool_type = std::tuple<storage<entity_type, Archetypes>...>;
  using shared_type = sparse_array<entity_type>;
  using manager_type = entity_manager<entity_type>;

  static_assert(std::numeric_limits<entity_type>::is_integer && !std::numeric_limits<entity_type>::is_signed,
    "Entity type must be an unsigned integer");
  static_assert(sizeof...(Archetypes) > 0, "Registry must contain atleast one archetype");

private:
  template<typename... Components>
  class basic_view;

public:
  /**
   * @brief Construct a new registry object
   */
  registry()
  {
    setup_shared_memory();
  }

  ~registry()
  {}

  registry(const registry&) = delete;
  registry(registry&&) = delete;
  registry& operator=(const registry&) = delete;
  registry& operator=(registry&&) = delete;

  /**
   * @brief Create an entity and initializes it with the given components.
   * 
   * In order for the create method to know what archetype is going to be created, all components
   * of the archetype must be specified and initialized in the create method. The order of the components
   * doesn't matter as long as the components match the exact components of one of the registry archetypes.
   * 
   * This operation is always O(1) and is very fast.
   * 
   * @tparam Components The exact component types of one of the registry archetypes
   * @param components The components to initialize with
   * @return entity_type The created entity's identifier
   */
  template<typename... Components>
  entity_type create(const Components&... components)
  {
    using current = find_for_t<list<Archetypes...>, Components...>;

    static_assert(size_v<current> == sizeof...(Components),
      "Registry does not contain suitable archetype for provided components");

    const entity_type entity = _manager.generate();

    access<current>().insert(entity, components...);

    return entity;
  }

  /**
   * @brief Destroys the specified entity.
   * 
   * This operation is very cheap but has a complexity of O(N) where N is the amount
   * of archetypes. This complexity can be compleatly or partially reduced by specifying
   * all or some of the types of the entity's archetype. You should probably be doing this.
   * 
   * @warning Attempting to destroy an entity that doesn't exist in the view results
   * in undefined behaviour
   * 
   * @tparam Components Component types that you know this entity's archetype has
   * @param entity The entity to destroy
   */
  template<typename... Components>
  void destroy(const entity_type entity)
  {
    static_assert(size_v<prune_for_t<list<Archetypes...>, Components...>> > 0,
      "Registry does not contain suitable archetype for provided components");

    view<Components...>().erase(entity);

    _manager.release(entity);
  }

  /**
   * @brief Destroys all entites in the registry.
   * 
   * This is a very cheap way to destroy all entities. Much faster than destroying each one
   * individually. You should use this if you can.
   */
  void destroy_all()
  {
    ((access<Archetypes>().clear()), ...);

    _manager.release_all();
  }

  /**
   * @brief Performs certain optimizations on the registry such as memory optimization.
   * 
   * @note This will mainly do memory optimizations, but in the future this may change.
   * 
   * This can be quite expensive to call often. Not recommended to call every frame.
   * However, every once in a while, it can be usefull to call this method in a clever way
   * to recalibrate everything and make sure your ressources are being use optimally.
   * 
   * @note You should consider using this if your application is running for a long time.
   */
  void optimize()
  {
    ((access<Archetypes>().shrink_to_fit()), ...);

    _manager.swap();
    _manager.shrink_to_fit();
  }

  /**
   * @brief Iterates over every entity that has the specified components and calls the given function.
   * 
   * Same thing as creating a view with the components you need and calling for_each.
   * 
   * @tparam Components The components types to form the view for
   * @tparam Func The function type
   * @param func The function to call on every iteration
   */
  template<typename... Components, typename Func>
  void for_each(const Func& func) { view<Components...>().for_each(func); }

  /**
   * @brief Returns a reference of the stored component for the specified entity and component type.
   * 
   * Unpacking components this way is the simplest, but the most expensive way to do it. A view with
   * only the specified component is created, so this method may need to check multiple storages
   * to find where the entity is stored. A better alternative is to create the view yourself, with
   * all the components you know the entity's archetype contains, this can significantly improve
   * performance. However, obtaining the components from iteration is always the best way and has no cost.
   * 
   * @warning Attempting to unpack an entity that doesn't contain the component results in
   * undefined behaviour
   * 
   * @tparam Component The component type to unpack
   * @param entity Entity to unpack component for
   * @return Component& Reference to component belonging to the entity
   */
  template<typename Component>
  Component& unpack(const entity_type entity) { return view<Component>().template unpack<Component>(entity); }

  /**
   * @brief Returns whether or not the entity has all the specified components.
   * 
   * Not very expensive to do, but you shouldn't need to call this method in most cases.
   * If your finding yourself calling this method often, then your probably doing something wrong.
   * 
   * If you want to make this method faster, you can specify the types that you know the 
   * component has, this can reduce some of the complexity.
   * 
   * @note Specifying more component types does not make this method slower, it can even
   * make it faster in some cases.
   * 
   * @tparam Components The component types to check for
   * @param entity The entity to check for
   * @return true If the entity has all the specified component types, false otherwise
   */
  template<typename... Components>
  bool has(const entity_type entity) { return view<Components...>().contains(entity); }

  /**
   * @brief Returns the amount of entities contained by this registry who have the specified components if any.
   * 
   * If no components are specified, this will the amount of all entities in the registry.
   * 
   * @note The more components are specified, the faster this method becomes.
   * 
   * @tparam Components The required components to be counted in the returned size
   * @return size_t The amount of entities in the registry that have the specified components
   */
  template<typename... Components>
  size_t size() { return view<Components...>().size(); }

  /**
   * @brief Returns whether or not the registry contains any entities with the specified components if any.
   * 
   * If no components are specified, this will return whether or not the registry contains ANY entities.
   * 
   * @note Uses the registry size method under the hood.
   * 
   * @tparam Components The required components to be counted in the check
   * @return true If the registry contains any entities that have the specified components.
   */
  template<typename... Components>
  bool empty() { return size<Components...>() == 0; }

  /**
   * @brief Returns a view of the registry for the specified components.
   * 
   * This is the main operation of the registry. Almost every method in the registry
   * uses a view under the hood.
   * 
   * Views find all the archetypes that contain the specified components at compile-time.
   * 
   * Views are essentially free to create.
   * 
   * @tparam Components The component types to include in the view
   * @return auto A view of the registry for the specified components
   */
  template<typename... Components>
  auto view() { return basic_view<Components...> { this }; }

  /**
   * @brief Returns the amount of storages in the registry.
   * 
   * @return size_t Amount of storages.
   */
  size_t storages() const { return _shared.shared(); }

  /**
   * @brief Accesses the storage for the specified archetype.
   * 
   * Getting the archetype's storage is done at compile-time.
   * 
   * Obtaining the storage of an archetype is essentially free.
   * 
   * @warning You should not directly access the storage unless you
   * know what your doing.
   * 
   * @tparam Archetype The archetype to get the storage for
   * @return auto& The storage of the specified archetype
   */
  template<typename Archetype>
  auto& access() { return std::get<storage<entity_type, Archetype>>(_pool); }

private:
  /**
   * @brief Set the up shared sparse_set
   * 
   * Called during the construction of the registry.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the registry.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   */
  template<size_t I = 0>
  void setup_shared_memory()
  {
    using current = at_t<I, archetype_list_type>;

    if constexpr (I < size_v<archetype_list_type>)
    {
      access<current>().share(&_shared);
      setup_shared_memory<I + 1>();
    }
  }

private:
  pool_type _pool;
  shared_type _shared;
  manager_type _manager;
};

template<typename Entity, typename... Archetypes>
template<typename... Components>
class registry<Entity, list<Archetypes...>>::basic_view
{
public:
  using archetype_list_view_type = prune_for_t<archetype_list_type, Components...>;

  static_assert(size_v<archetype_list_view_type> > 0, "View must contain atleast one archetype");

public:
  explicit basic_view(registry_type* registry) : _registry { registry } {}

  /**
   * @brief Iterates over every entity that has the specified components and calls the given function.
   * 
   * There is not much of a cost outside of the unpacking cost for iterating over multiple components. 
   * Actually, specifing more components may even lead to better results in some cases. Go crazy...
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @tparam Components The components types to form the view for
   * @tparam Func The function type
   * @param func The function to call on every iteration
   */
  template<size_t I = 0, typename Func>
  void for_each(const Func& func)
  {
    using current = at_t<I, archetype_list_view_type>;

    auto& storage = _registry->template access<current>();

    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
      func(*it, it.template unpack<Components>()...);
    }

    if constexpr (I + 1 < size_v<archetype_list_view_type>) for_each<I + 1>(func);
  }

  /**
   * @brief Returns a reference of the stored component for the specified entity and component type.
   * 
   * You should always prioritize getting your components from iteration.
   * 
   * @warning Attempting to unpack an entity that is not in the view results in
   * undefined behaviour
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam Component The component type to unpack
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @param entity Entity to unpack component for
   * @return Component& Reference to component belonging to the entity
   */
  template<typename Component, size_t I = 0>
  Component& unpack(const entity_type entity)
  {
    if constexpr (I == 0)
      static_assert(size_v<prune_for_t<archetype_list_view_type, Component>> > 0,
        "You cannot unpack a component type that is not included in the view");

    using current = at_t<I, archetype_list_view_type>;

    auto& storage = _registry->template access<current>();

    if constexpr (I + 1 == size_v<archetype_list_view_type>)
      return storage.template unpack<Component>(entity);
    else if (storage.contains(entity))
      return storage.template unpack<Component>(entity);
    else
      return unpack<Component, I + 1>(entity);
  }

  /**
   * @brief Returns whether or not the view contains the specified entity.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @param entity The entity to check for
   * @return true If the view contains the entity
   */
  template<size_t I = 0>
  bool contains(const entity_type entity)
  {
    using current = at_t<I, archetype_list_view_type>;

    if (_registry->template access<current>().contains(entity)) return true;

    if constexpr (I + 1 == size_v<archetype_list_view_type>) return false;
    else
      return contains<I + 1>(entity);
  }

  /**
   * @brief Returns the amount of entities in the view.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @return size_t The amount of entities in the view
   */
  template<size_t I = 0>
  size_t size()
  {
    using current = at_t<I, archetype_list_view_type>;

    if constexpr (I == size_v<archetype_list_view_type>) return 0;
    else
      return _registry->template access<current>().size() + size<I + 1>();
  }

private:
  friend registry_type;

  /**
   * @brief Erases an entity from the correct storage in the view.
   * 
   * Used internally by the registry to destroy entities. Unsafe and cannot be used externally
   * because this method removes an entity without releasing the entity back into
   * the entity_manager.
   * 
   * @warning Attempting to erase an entity that doesn't exist in the view results
   * in undefined behaviour
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @param entity The entity to erase
   */
  template<size_t I = 0>
  void erase(const entity_type entity)
  {
    using current = at_t<I, archetype_list_view_type>;

    auto& storage = _registry->template access<current>();

    if constexpr (I + 1 == size_v<archetype_list_view_type>) storage.erase(entity);
    else if (storage.contains(entity))
      storage.erase(entity);
    else
      erase<I + 1>(entity);
  }

private:
  registry_type* _registry;
};
} // namespace xecs

#endif
