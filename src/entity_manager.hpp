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
