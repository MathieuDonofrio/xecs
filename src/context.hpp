#ifndef XECS_CONTEXT_HPP
#define XECS_CONTEXT_HPP

#include "container.hpp"
#include "type_map.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace xecs
{
using cid = uint32_t;

struct ComponentTag
{};
struct archetype_tag
{};
struct view_tag
{};

template<typename... Components>
static std::vector<cid> component_ids()
{
  std::vector<cid> components;
  components.reserve(sizeof...(Components));

  (components.push_back(component_id<Components>()), ...);

  std::sort(components.begin(), components.end());

  return components;
}

template<typename Component>
static cid component_id()
{
  return static_cast<cid>(type_map<ComponentTag>::key<Component>());
}

class context
{
public:
  template<typename... Components>
  size_t assure_archetype()
  {
    const size_t id = _archetypes.unsafe_id<Components...>();

    if (!_archetypes.initialized(id))
    {
      if (_archetypes.initialize<Components...>())
      {
        add_archetype(id);
      }
    }

    return _archetypes.lookup_index(id);
  }

  template<typename... Components>
  size_t assure_view()
  {
    const size_t id = _views.unsafe_id<Components...>();

    if (!_views.initialized(id))
    {
      if (_views.initialize<Components...>())
      {
        add_view(id);
      }
    }

    return _views.lookup_index(id);
  }

  const std::vector<size_t>& view_archetypes(size_t view) const
  {
    return _view_to_archetypes[view];
  }

private:
  struct Initializer
  {
    bool initialized = false;
  };

  template<typename Tag>
  class indexor
  {
  public:
    template<typename... Components>
    bool initialize()
    {
      auto id = safe_id<Components...>();

      auto signature = component_ids<Components...>();

      for (size_t index = 0; index < _signatures.size(); index++)
      {
        auto& other_signature = _signatures[index];

        if (signature.size() == other_signature.size()
            && std::includes(signature.begin(), signature.end(), other_signature.begin(), other_signature.end()))
        {
          _mappings.template raw_access<cid>(id) = index;
          return false;
        }
      }

      auto index = _signatures.size();

      _mappings.template raw_access<cid>(id) = index;

      _signatures.push_back(std::move(signature));

      _mappings.template raw_access<Initializer>(id).initialized = true;

      return true;
    }

    template<typename... Components>
    size_t unsafe_id()
    {
      return decltype(_mappings)::key<combine_types<Components...>>();
    }

    template<typename... Components>
    size_t safe_id()
    {
      return _mappings.template assure_key<combine_types<Components...>>();
    }

    cid lookup_index(const size_t id)
    {
      return _mappings.template raw_access<cid>(id);
    }

    bool initialized(const size_t id)
    {
      return _mappings.safe(id) && _mappings.template raw_access<Initializer>(id).initialized;
    }

    std::vector<cid>& signature(const size_t index)
    {
      return _signatures[index];
    }

    size_t size() const { return _signatures.size(); }

  private:
    type_map<Tag, cid, Initializer> _mappings;
    std::vector<std::vector<cid>> _signatures;
  };

  void add_archetype(size_t id)
  {
    const size_t index = _archetypes.lookup_index(id);

    auto& archetype_components = _archetypes.signature(index);

    for (size_t i = 0; i < _views.size(); i++)
    {
      auto& view_components = _views.signature(i);

      if (std::includes(archetype_components.begin(), archetype_components.end(), view_components.begin(), view_components.end()))
      {
        _view_to_archetypes[i].push_back(index);

        if (view_components.size() == archetype_components.size())
        {
          std::swap(_view_to_archetypes[i].front(), _view_to_archetypes[i].back());
        }
      }
    }
  }

  void add_view(size_t id)
  {
    const size_t index = _views.lookup_index(id);

    if (index >= _view_to_archetypes.size())
    {
      _view_to_archetypes.resize(index + 1);
    }

    auto& view_components = _views.signature(index);

    for (size_t i = 0; i < _archetypes.size(); i++)
    {
      auto& archetype_components = _archetypes.signature(i);

      if (std::includes(archetype_components.begin(), archetype_components.end(), view_components.begin(), view_components.end()))
      {
        _view_to_archetypes[index].push_back(i);

        if (view_components.size() == archetype_components.size())
        {
          std::swap(_view_to_archetypes[index].front(), _view_to_archetypes[index].back());
        }
      }
    }
  }

  indexor<archetype_tag> _archetypes;
  indexor<view_tag> _views;

  std::vector<std::vector<size_t>> _view_to_archetypes;
};

} // namespace xecs

#endif