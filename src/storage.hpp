#pragma once

#include "archetype.hpp"

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <tuple>
#include <utility>

#define SPARSE_ARRAY_PAGE_SIZE 4096 // This should not be changed unless you know what your doing

static_assert((SPARSE_ARRAY_PAGE_SIZE & (SPARSE_ARRAY_PAGE_SIZE - 1)) == 0, "SPARSE_ARRAY_PAGE_SIZE must be a power of two");

#define STORAGE_MINIMUM_CAPACITY 8

static_assert(STORAGE_MINIMUM_CAPACITY != 0, "STORAGE_MINIMUM_CAPACITY cannot be 0");

namespace ecs
{
namespace internal
{
  template<typename Entity>
  class sparse_array;
}
template<typename Entity, typename Archetype>
struct storage;

template<typename Entity, typename... Components>
class storage<Entity, archetype<Components...>> final
{
public:
  using entity_type = Entity;
  using size_type = size_t;

private:
  using dense_type = entity_type*;
  using page_type = entity_type*;
  using sparse_type = std::shared_ptr<internal::sparse_array<Entity>>;
  using storage_type = std::tuple<Components*...>;

public:
  class iterator;

  storage();
  storage(const storage&) = delete;
  storage(storage&&) = delete;
  ~storage();

  storage& operator=(const storage&) = delete;

  void assure(const size_type page);

  template<typename... IncludedComponents>
  void insert(const entity_type value, IncludedComponents&&... components);
  void erase(const entity_type value);
  bool contains(const entity_type value) const;

  void shrink_to_fit();

  void clear() { _size = 0; }

  iterator begin() { return { this, _size - 1 }; }
  iterator end() { return { this, static_cast<size_type>(-1) }; }

  void make_shared(const sparse_type& sparse) { _sparse = sparse; }

  size_type size() const { return _size; }
  size_type capacity() const { return _capacity; }
  bool empty() const { return _size == 0; }

  template<typename Component>
  Component& unpack(const entity_type value) { return access<Component>()[_sparse->index(value)]; }

private:
  template<typename Component>
  Component*& access() { return std::get<Component*>(_storage); }

private:
  dense_type _dense;
  sparse_type _sparse;
  storage_type _storage;

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
  {
  }

  iterator& move(const size_type value)
  {
    _pos += value;
    return *this;
  }

  iterator& operator+=(const size_type value) { return move(value); }
  iterator& operator-=(const size_type value) { return move(-value); }

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

  const entity_type operator*() const { return _ptr->_dense[_pos]; }

  template<typename Component> const Component& unpack() const { return _ptr->access<Component>()[_pos]; }
  template<typename Component> Component& unpack() { return _ptr->access<Component>()[_pos]; }

private:
  storage* const _ptr;
  size_type _pos;
};

namespace internal
{
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

  template<typename Entity>
  class sparse_array final
  {
  public:
    using entity_type = Entity;
    using size_type = size_t;
    using page_type = entity_type*;
    using array_type = page_type*;

    static constexpr size_type entites_per_page = SPARSE_ARRAY_PAGE_SIZE / sizeof(entity_type);
    static constexpr size_type page_shift = log2(entites_per_page);
    static constexpr size_type offset_mask = ((1 << page_shift) - 1);

    sparse_array();
    sparse_array(const sparse_array&) = delete;
    sparse_array(sparse_array&&) = delete;
    ~sparse_array();

    sparse_array& operator=(const sparse_array&) = delete;

    void assure(const size_type page);

    page_type operator[](const size_type page) const { return _array[page]; }
    page_type& operator[](const size_type page) { return _array[page]; }

    size_type page(const entity_type entity) const { return entity >> page_shift; };
    size_type offset(const entity_type entity) const { return entity & offset_mask; }
    size_type index(const entity_type entity) const { return _array[page(entity)][offset(entity)]; }

    size_type pages() const { return _pages; };

  private:
    array_type _array;
    size_type _pages;
  };

  template<typename Entity>
  sparse_array<Entity>::sparse_array()
    : _pages(4)
  {
    // This needs to be calloc because 0 pointers are used to determine if the page is allocated
    _array = static_cast<array_type>(std::calloc(_pages, sizeof(page_type)));
  }

  template<typename Entity>
  sparse_array<Entity>::~sparse_array()
  {
    for (size_type i = 0; i < _pages; i++)
      if (_array[i]) free(_array[i]);
    free(_array);
  }

  template<typename Entity>
  void sparse_array<Entity>::assure(const size_type page)
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
} // namespace internal

template<typename Entity, typename... Components>
storage<Entity, archetype<Components...>>::storage()
  : _size { 0 }, _capacity { STORAGE_MINIMUM_CAPACITY }
{
  _sparse = std::make_shared<internal::sparse_array<entity_type>>();
  _dense = static_cast<dense_type>(std::malloc(_capacity * sizeof(entity_type)));
  ((access<Components>() = static_cast<Components*>(std::malloc(_capacity * sizeof(Components)))), ...);
}

template<typename Entity, typename... Components>
storage<Entity, archetype<Components...>>::~storage()
{
  free(_dense);
  (free(access<Components>()), ...);
}

template<typename Entity, typename... Components>
void storage<Entity, archetype<Components...>>::assure(const size_type page)
{
  if (_size == _capacity)
  {
    // This is essentially _capacity * 1.5 + a small linear amount
    _capacity = (_capacity * 3) / 2 + 8;

    _dense = static_cast<dense_type>(std::realloc(_dense, _capacity * sizeof(entity_type)));
    ((access<Components>() = static_cast<Components*>(std::realloc(access<Components>(), _capacity * sizeof(Components)))), ...);
  }

  _sparse->assure(page);
}

template<typename Entity, typename... Components>
template<typename... IncludedComponents>
void storage<Entity, archetype<Components...>>::insert(const entity_type value, IncludedComponents&&... components)
{
  // undefined behaviour if the entity already exists

  const auto p = _sparse->page(value);

  assure(p);

  _dense[_size] = value;

  ((access<IncludedComponents>()[_size] = std::move(components)), ...);

  (*_sparse)[p][_sparse->offset(value)] = static_cast<entity_type>(_size++);
}

template<typename Entity, typename... Components>
void storage<Entity, archetype<Components...>>::erase(const entity_type value)
{
  // undefined behaviour if the entity does not exist

  const auto back = _dense[--_size];

  const auto index = _sparse->index(value);

  (*_sparse)[_sparse->page(back)][_sparse->offset(back)] = static_cast<entity_type>(index);

  _dense[index] = back;

  ((access<Components>()[index] = std::move(access<Components>()[_size])), ...);
}

template<typename Entity, typename... Components>
bool storage<Entity, archetype<Components...>>::contains(const entity_type value) const
{
  using page_type = entity_type*;

  const auto pageIndex = _sparse->page(value);

  page_type page;
  size_type index;

  // We must access the dense array here because our sparse arrays may be shared, therefor we need
  // to make sure entity index is valid.
  return pageIndex < _sparse->pages() && (page = (*_sparse)[pageIndex]) != 0
         && (index = page[_sparse->offset(value)]) < _size && _dense[index] == value;
}

template<typename Entity, typename... Components>
void storage<Entity, archetype<Components...>>::shrink_to_fit()
{
  if (_size != _capacity && _size > STORAGE_MINIMUM_CAPACITY)
  {
    _capacity = _size;

    _dense = static_cast<dense_type>(std::realloc(_dense, _capacity * sizeof(entity_type)));
    ((access<Components>() = static_cast<Components*>(std::realloc(access<Components>(), _capacity * sizeof(Components)))), ...);
  }
}
} // namespace ecs