#pragma once

#include "archetype.hpp"

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>

#define SPARSE_ARRAY_PAGE_SIZE 4096 // This should not be changed unless you know what your doing

static_assert((SPARSE_ARRAY_PAGE_SIZE & (SPARSE_ARRAY_PAGE_SIZE - 1)) == 0, "SPARSE_ARRAY_PAGE_SIZE must be a power of two");

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
 * To make dynamically resizing this array cheaper as well as to reduce the amount of memory used 
 * in certain cases, this array implements paging. A page is a pointer to a fixed size array where 
 * the size is equal to the SPARSE_ARRAY_PAGE_SIZE.
 * 
 * @tparam Entity unsigned int entity identifier
 */
template<typename Entity>
class sparse_array final
{
private:
  /**
   * @brief Utility method for computing base 2 log at compile-time
   * 
   * Does not need to be efficient, this is just used at compile-time. 
   * Simply needs to work and be readable.
   * 
   * @param x Value to compute base 2 log for
   * 
   * @return constexpr size_t Base 2 log of x
   */
  static constexpr size_t log2(const size_t x)
  {
    size_t bit = 0;
    while (bit != (8 * sizeof(size_t)))
    {
      if ((x >> bit) & 1) return bit;
      ++bit;
    }
    return 0;
  }

public:
  using entity_type = Entity;
  using size_type = size_t;
  using page_type = entity_type*;
  using array_type = page_type*;
  using shared_count_type = uint16_t;

  static_assert(std::numeric_limits<entity_type>::is_integer && !std::numeric_limits<entity_type>::is_signed,
    "Entity type must be an unsigned integer");

  /**
   * @brief Amount of entities that can fit in one page.
   * 
   * This is the SPARSE_ARRAY_PAGE_SIZE divided by the size of the entity type.
   */
  static constexpr size_type entites_per_page = SPARSE_ARRAY_PAGE_SIZE / sizeof(entity_type);

  /**
   * @brief Right bit shift amount to obtain an entities page.
   * 
   * This is the base 2 log of the amount of entities per page.
   */
  static constexpr size_type page_shift = log2(entites_per_page);

  /**
   * @brief Bit mask to obtain the offset of an entity in its page.
   * 
   * Obtained using the page shift.
   */
  static constexpr size_type offset_mask = ((1 << page_shift) - 1);

  /**
   * @brief Construct a new sparse array object
   */
  sparse_array()
    : _pages(8), _shared(0)
  {
    // This needs to be calloc because 0 pointers are used to determine if the page is allocated
    _array = static_cast<array_type>(std::calloc(_pages, sizeof(page_type)));
  }

  /**
   * @brief Destroy the sparse array object
   */
  ~sparse_array()
  {
    for (size_type i = 0; i < _pages; i++)
      if (_array[i]) free(_array[i]);
    free(_array);
  }

  sparse_array(const sparse_array&) = delete;
  sparse_array(sparse_array&&) = delete;
  sparse_array& operator=(const sparse_array&) = delete;

  /**
   * @brief Assures that the sparse array has a given page
   * 
   * First, this method checks if the page pointer even exists. It it doesn't
   * it resizes the page pointer array. Then, this method checks if the requested
   * page exists, if it doesn't it allocates the new page.
   * 
   * @param page Page to assure
   */
  void assure(const size_type page)
  {
    if (page >= _pages)
    {
      // Make required amount the requested page + 1
      // There does not seem to be any apperent performance gain by incrementing more than 1
      const auto required = page + 1;

      _array = static_cast<array_type>(std::realloc(_array, required * sizeof(page_type)));

      // We need to set the new memory to 0 here because 0 pointers are used to determine if the page is allocated
      std::memset(_array + _pages, 0, (required - _pages) * sizeof(page_type));

      _pages = required;
    }

    if (!_array[page])
    {
      _array[page] = static_cast<page_type>(std::malloc(SPARSE_ARRAY_PAGE_SIZE));
    }
  }

  /**
   * @brief Returns the page for the page index.
   * 
   * @param page Page index
   * @return page_type Array of indexes
   */
  page_type operator[](const size_type page) const { return _array[page]; }

  /**
   * @brief Returns the page for the page index.
   * 
   * @param page Page index
   * @return page_type Array of indexes
   */
  page_type& operator[](const size_type page) { return _array[page]; }

  /**
   * @brief Returns the page index of an entity.
   * 
   * Simply a very cheap bitshift.
   * 
   * @param entity Entity to get the page index for
   * @return size_type Page index for entity
   */
  size_type page(const entity_type entity) const { return entity >> page_shift; }

  /**
   * @brief Returns the page offset of an entity.
   * 
   * The offet is where the entity would be located in its page.
   * 
   * Simply a very cheap bitmask.
   * 
   * @param entity Entity to get the page offer for
   * @return size_type Page offer for entity
   */
  size_type offset(const entity_type entity) const { return entity & offset_mask; }

  /**
   * @brief Returns the index mapping for an entity.
   * 
   * @param entity Entity to get the page offer for
   * @return size_type Page offer for entity
   */
  size_type index(const entity_type entity) const { return _array[page(entity)][offset(entity)]; }

  /**
   * @brief Returns the amount of pages in the sparse_array.
   * 
   * This includes both allocated and unallocated pages.
   * 
   * @return size_type Amount of pages
   */
  size_type pages() const { return _pages; }

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
  size_type _pages;
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
    (new_array<Components>(), ...);
  }

  /**
   * @brief Destroy the storage object
   */
  ~storage()
  {
    // We only manage our sparse_array memory if its not shared
    if (!_sparse->shared()) delete _sparse;

    free(_dense);
    (delete_array<Components>(), ...);
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
   * You can directly add the components with this method. This is not nessesary and can
   * be done later very efficiently using the unpack method. Also, the order of the
   * components dont matter, as long as they are all unique and part of the archetype.
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

    const auto p = _sparse->page(entity);

    if (_size == _capacity) grow();
    _sparse->assure(p);

    _dense[_size] = entity;

    ((access<IncludedComponents>()[_size] = components), ...);

    (*_sparse)[p][_sparse->offset(entity)] = static_cast<entity_type>(_size++);
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
    const auto back = _dense[--_size];

    const auto index = _sparse->index(entity);

    (*_sparse)[_sparse->page(back)][_sparse->offset(back)] = static_cast<entity_type>(index);

    _dense[index] = back;

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
    using page_type = entity_type*;

    const auto pageIndex = _sparse->page(entity);

    page_type page;
    size_type index;

    // We must access the dense array here because our sparse arrays may be shared, therefor we need
    // to make sure entity index is valid.
    return pageIndex < _sparse->pages() && (page = (*_sparse)[pageIndex]) != 0
           && (index = page[_sparse->offset(entity)]) < _size && _dense[index] == entity;
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

    return access<Component>()[_sparse->index(entity)];
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
      (resize_array<Components>(), ...);
    }
  }

  /**
   * @brief Binds the shared sparse_array to this storage.
   * 
   * Sharing sparse_arrays between storages that operate on entities distributed by the
   * same entity_manager is recommended for performance and memory usage.
   * 
   * @throws std::exception If the storage is not empty.
   * 
   * @param sparse The sparse_array to share with this storage
   */
  void share(const sparse_type sparse)
  {
    if (_size != 0)
      throw new std::logic_error("You cannot share a sparse_array with a storage that is not empty");

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
   * Growth is exponential with a small linear amount.
   */
  void grow()
  {
    // This is essentially _capacity * 1.5 + 8
    // There may be a better way to grow
    _capacity = (_capacity * 3) / 2 + 8;

    _dense = static_cast<dense_type>(std::realloc(_dense, _capacity * sizeof(entity_type)));
    (resize_array<Components>(), ...);
  }

  template<typename Component>
  void resize_array()
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

  template<typename Component>
  void new_array()
  {
    if constexpr (std::is_trivial_v<Component>)
      access<Component>() = static_cast<Component*>(std::malloc(_capacity * sizeof(Component)));
    else
      access<Component>() = new Component[_capacity];
  }

  template<typename Component>
  void delete_array()
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