#ifndef XECS_CONTEXT_HPP
#define XECS_CONTEXT_HPP

#include "container.hpp"
#include "meta.hpp"
#include "type_map.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

namespace xecs
{
using cid = uint32_t;

namespace internal
{
  template<typename Component>
  static constexpr cid component_id()
  {
    return static_cast<cid>(type_id<Component>::index());
  }

  template<typename List>
  struct component_sequence;

  template<typename... Components, template<typename...> class List>
  struct component_sequence<List<Components...>>
  {
    static std::vector<cid> runtime()
    {
      std::vector<cid> components;
      components.reserve(sizeof...(Components));

      (components.push_back(component_id<Components>()), ...);

      return components;
    }
  };

  template<typename... Components>
  static std::vector<cid> component_ids()
  {
    return component_sequence<combined<Components...>::sorted_t>::runtime();
  }

  template<typename Tag = void>
  class indexor
  {
  public:
    template<typename... Components>
    void initialize()
    {
      auto id = safe_id<Components...>();

      if (id >= _signatures.size()) _signatures.resize(id + 1);

      _signatures[id] = component_ids<Components...>();

      _mappings.template raw_access<Initializer>(id).initialized = true;
    }

    template<typename... Components>
    cid unsafe_id() const
    {
      return static_cast<cid>(decltype(_mappings)::key<combined<Components...>::sorted_t>());
    }

    template<typename... Components>
    cid safe_id()
    {
      return static_cast<cid>(_mappings.template assure_key<combined<Components...>::sorted_t>());
    }

    const std::vector<cid>& signature(const cid id) const
    {
      return _signatures[id];
    }

    bool initialized(const cid id)
    {
      return _mappings.safe(id) && _mappings.template raw_access<Initializer>(id).initialized;
    }

    size_t size() const
    {
      return _signatures.size();
    }

  private:
    struct Initializer
    {
      bool initialized = false;
    };

    type_map<Tag, Initializer> _mappings;
    std::vector<std::vector<cid>> _signatures;
  };
} // namespace internal

class context
{
public:
  template<typename... Components>
  cid assure_archetype()
  {
    const cid id = _archetypes.unsafe_id<Components...>();

    if (!_archetypes.initialized(id))
    {
      _archetypes.initialize<Components...>();
      add_archetype(id);
    }

    return id;
  }

  template<typename... Components>
  cid assure_view()
  {
    const cid id = _views.unsafe_id<Components...>();

    if (!_views.initialized(id))
    {
      _views.initialize<Components...>();
      add_view(id);
    }

    return id;
  }

  const std::vector<cid>& view_archetypes(cid view) const
  {
    return _view_to_archetypes[view];
  }

private:
  void add_archetype(cid id)
  {
    auto& archetype_components = _archetypes.signature(id);

    for (cid i = 0; i < _views.size(); i++)
    {
      if (_views.initialized(i))
      {
        auto& view_components = _views.signature(i);

        if (std::includes(archetype_components.begin(), archetype_components.end(), view_components.begin(), view_components.end()))
        {
          _view_to_archetypes[i].push_back(id);

          if (view_components.size() == archetype_components.size())
          {
            std::swap(_view_to_archetypes[i].front(), _view_to_archetypes[i].back());
          }
        }
      }
    }
  }

  void add_view(cid id)
  {
    if (id >= _view_to_archetypes.size()) _view_to_archetypes.resize(id + 1);

    auto& view_components = _views.signature(id);

    for (cid i = 0; i < _archetypes.size(); i++)
    {
      if (_archetypes.initialized(i))
      {
        auto& archetype_components = _archetypes.signature(i);

        if (std::includes(archetype_components.begin(), archetype_components.end(), view_components.begin(), view_components.end()))
        {
          _view_to_archetypes[id].push_back(i);

          if (view_components.size() == archetype_components.size())
          {
            std::swap(_view_to_archetypes[id].front(), _view_to_archetypes[id].back());
          }
        }
      }
    }
  }

  struct archetype_tag
  {};
  struct view_tag
  {};

  internal::indexor<archetype_tag> _archetypes;
  internal::indexor<view_tag> _views;

  std::vector<std::vector<cid>> _view_to_archetypes;
};

} // namespace xecs

#endif