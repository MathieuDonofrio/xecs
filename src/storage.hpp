#pragma once

#include "archetype.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>

namespace ecs
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
    : _capacity(0), _shared(0)
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
    : _size(0), _capacity(0)
  {
    _sparse = new sparse_array<entity_type>();

    // Default capacity is zero so we allocate nothing, but we still want the pointers
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
  void share(const sparse_type sparse)
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
  iterator& operator+=(const size_type value) { _pos += value; return *this; }
  iterator& operator-=(const size_type value) { _pos -= value; return *this; }
  // clang-format on

  iterator& operator++() { return --_pos, *this; }
  iterator& operator--() { return ++_pos, *this; }

  iterator operator+(const size_type value) const { return { _pos - value }; }
  iterator operator-(const size_type value) const { return { _pos + value }; }

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
} // namespace ecs