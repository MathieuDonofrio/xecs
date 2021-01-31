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

template<typename... Types, template<typename...> typename List>
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

template<typename... Types, template<typename...> typename List>
struct empty<List<Types...>> : std::bool_constant<sizeof...(Types) == 0>
{};

template<typename List>
constexpr auto empty_v = empty<List>::value;

/**
 * @brief Checks if a list contains a type.
 * 
 * @tparam Type Type to check for
 * @tparam List List to check
 */
template<typename Type, typename List>
struct contains;

template<typename Type, typename... Types, template<typename...> typename List>
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

template<typename... ATypes, template<typename...> typename AList, typename... BTypes, template<typename...> typename BList>
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
 * @brief Concatenates a list and a type.
 * 
 * @tparam Type Type to concatenate (add)
 * @tparam List List to concatenante type to
 */
template<typename Type, typename List>
struct concat;

template<typename Type, typename... Types, template<typename...> typename List>
struct concat<Type, List<Types...>>
{
  using type = List<Type, Types...>;
};

template<typename Type, typename List>
using concat_t = typename concat<Type, List>::type;

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

template<typename... Lists, template<typename...> typename ListOfLists, typename... Types>
struct find_for<ListOfLists<Lists...>, Types...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> typename ListOfLists, typename... Types>
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

template<typename... Lists, template<typename...> typename ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<Lists...>, RequiredTypes...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> typename ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<HeadList, Lists...>, RequiredTypes...>
{
private:
  using next = typename prune_for<ListOfLists<Lists...>, RequiredTypes...>::type;
  using accept = typename concat<HeadList, next>::type;

public:
  using type = typename std::conditional_t<contains_all<HeadList, RequiredTypes...>::value, accept, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using prune_for_t = typename prune_for<ListOfLists, RequiredTypes...>::type;

/**
 * @brief Finds the type at the specified index in the list.
 * 
 * @tparam I Index in the list
 * @tparam List The list to operate on
 */
template<size_t I, typename List>
struct at;

template<size_t I, typename... Types, template<typename...> typename List>
struct at<I, List<Types...>>
{
  using type = List<>;
};

template<size_t I, typename Type, typename... Types, template<typename...> typename List>
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
