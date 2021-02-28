#ifndef XECS_TYPE_MAP_HPP
#define XECS_TYPE_MAP_HPP

#include "container.hpp"

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>

namespace xecs
{
template<typename... Types>
struct combine_types
{};

template<typename... Types>
class type_map
{
public:
  template<typename Key>
  static size_t key()
  {
    static const size_t value = next_index();
    return value;
  }

  template<typename Key>
  size_t assure_key()
  {
    const size_t index = key<Key>();

    if (index >= _container.size())
    {
      _container.resize(index + 1);
    }

    return index;
  }

  template<typename Key, typename Type>
  Type& access()
  {
    return raw_access<Type>(assure_key<Key>());
  }

  template<typename Type>
  Type& raw_access(size_t key)
  {
    return _container.template access<Type>()[key];
  }

  bool safe(size_t key) const
  {
    return key < _container.size();
  }

private:
  static size_t next_index()
  {
    static size_t sequence = 0;
    return sequence++;
  }

private:
  container<Types...> _container;
};
} // namespace xecs

#endif