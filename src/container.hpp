#ifndef XECS_CONTAINER_HPP
#define XECS_CONTAINER_HPP

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <tuple>
#include <utility>

namespace xecs
{
template<typename... Types>
class container
{
public:
  class iterator;

  container()
    : _size(0), _capacity(0)
  {
    ((std::get<Types*>(_arrays) = nullptr), ...);
  }

  ~container()
  {
    (deallocate<Types>(), ...);
  }

  container(const container&) = delete;
  container(container&&) = delete;
  container& operator=(const container&) = delete;
  container& operator=(container&&) = delete;

  template<typename... InitializedTypes>
  void push_back(const InitializedTypes&... types)
  {
    (construct<Types>(_size), ...);

    ((access<InitializedTypes>()[_size] = types), ...);

    ++_size;
  }

  template<typename... InitializedTypes>
  void insert(size_t index, const InitializedTypes&... types)
  {
    ((access<InitializedTypes>()[index] = types), ...);
  }

  void pop_back()
  {
    --_size;

    (destroy<Types>(_size), ...);
  }

  void erase(const size_t index)
  {
    --_size;

    (destroy<Types>(index), ...);

    // Moves the data to the erased location
    ((access<Types>()[index] = std::move(access<Types>()[_size])), ...);
  }

  void clear()
  {
    (destroyAll<Types>(), ...);
    _size = 0;
  }

  void resize(const size_t size)
  {
    if (size > _size)
    {
      reserve(size);

      for (size_t i = _size; i < size; i++)
      {
        (construct<Types>(i), ...);
      }
    }
    else
    {
      for (size_t i = size; i < _size; i++)
      {
        (destroy<Types>(i), ...);
      }
    }

    _size = static_cast<uint32_t>(size);
  }

  void reserve(const size_t capacity)
  {
    if (_capacity < capacity)
    {
      _capacity = static_cast<uint32_t>(capacity);

      (reallocate<Types>(), ...);
    }
  }

  void shrink_to_fit()
  {
    if (_capacity > _size)
    {
      _capacity = static_cast<uint32_t>(_size);
      (reallocate<Types>(), ...);
    }
  }

  template<typename Type>
  const Type* const access() const { return std::get<Type*>(_arrays); }

  template<typename Type>
  Type* access() { return std::get<Type*>(_arrays); }

  size_t size() const { return _size; }

  size_t capacity() const { return _capacity; }

  bool empty() const { return _size == 0; }

private:
  template<typename Type>
  void deallocate()
  {
    if constexpr (!std::is_trivially_destructible_v<Type>)
    {
      for (size_t i = 0; i < _size; i++)
      {
        access<Type>()[i].~Type();
      }
    }

    if (access<Type>())
    {
      free(access<Type>());
    }
  }

  template<typename Type>
  void reallocate()
  {
    if (std::is_trivially_copyable_v<Type> || std::is_trivially_move_assignable_v<Type>)
    {
      std::get<Type*>(_arrays) = static_cast<Type*>(std::realloc(access<Type>(), _capacity * sizeof(Type)));
    }
    else
    {
      Type* old_array = access<Type>();

      Type* new_array = static_cast<Type*>(std::malloc(_capacity * sizeof(Type)));

      for (size_t i = 0; i < _size; i++)
      {
        if constexpr (!std::is_trivially_constructible_v<Type>)
        {
          new (new_array + i) Type();
        }

        new_array[i] = std::move(old_array[i]);

        old_array[i].~Type();
      }

      free(old_array);

      std::get<Type*>(_arrays) = new_array;
    }
  }

  template<typename Type>
  void construct(const size_t index)
  {
    if constexpr (!std::is_trivially_constructible_v<Type>)
    {
      new (access<Type>() + index) Type(); // Default constructor
    }
    else
      (void)index; // Suppress unused warning
  }

  template<typename Type>
  void destroyAll()
  {
    if constexpr (!std::is_trivially_destructible_v<Type>)
    {
      for (size_t i = 0; i < _size; i++)
      {
        destroy<Type>(i);
      }
    }
  }

  template<typename Type>
  void destroy(const size_t index)
  {
    if constexpr (!std::is_trivially_destructible_v<Type>)
    {
      access<Type>()[index].~Type();
    }
    else
      (void)index; // Suppress unused warning
  }

  std::tuple<Types*...> _arrays;
  uint32_t _size;
  uint32_t _capacity;
};
} // namespace xecs

#endif