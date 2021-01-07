#pragma once

#include "archetype.hpp"
#include "entity_manager.hpp"
#include "storage.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace ecs
{
namespace internal
{
  template<typename Registry, typename... Components>
  class basic_view;
}

template<typename Entity, typename ArchetypeRegistry>
class registry;

template<typename Entity, typename... Archetypes>
class registry<Entity, list<Archetypes...>>
{
public:
  using entity_type = Entity;
  using archetype_list_type = list<Archetypes...>;
  using registry_type = registry<entity_type, archetype_list_type>;
  using manager_type = entity_manager<entity_type>;
  using pool_type = std::tuple<storage<entity_type, Archetypes>...>;
  using shared_type = internal::sparse_array<entity_type>;

  static_assert(std::numeric_limits<entity_type>::is_integer && !std::numeric_limits<entity_type>::is_signed,
    "Entity must be an unsigned integer");
  static_assert(sizeof...(Archetypes) > 0,
    "Registry must contain atleast one archetype");

public:
  registry();
  registry(const registry&) = delete;
  registry(registry&&) = delete;

  registry& operator=(const registry&) = delete;

  template<typename... Components>
  entity_type create(Components&&... components);

  template<typename... Components>
  void destroy(const entity_type entity);

  void destroy_all();

  void optimize();

  template<typename... Components, typename Func>
  void for_each(Func& func) { view<Components...>().for_each(func); }

  template<typename... Components>
  void set(const entity_type entity, Components&&... components) { return view<Components...>().set(entity, components...); }

  template<typename Component>
  Component& get(const entity_type entity) { return view<Component>()::template get<Component>(); }

  template<typename... Components>
  bool has(const entity_type entity) { return view<Components...>().contains(entity); }

  template<typename... Components>
  size_t size() { return view<Components...>().size(); }

  template<typename... Components>
  bool empty() { return size<Components...>() == 0; }

private:
  template<typename... Components>
  auto view() { return internal::basic_view<registry_type, Components...> { this }; }

private:
  friend internal::basic_view;

  template<size_t I = 0>
  void setup_shared_memory();

  template<typename Archetype>
  storage<entity_type, Archetype>& access() { return std::get<storage<entity_type, Archetype>>(_pool); }

private:
  manager_type _manager;
  pool_type _pool;
  shared_type _shared;
};

namespace internal
{
  template<typename Registry, typename... Components>
  class basic_view
  {
  public:
    using Registry_type = Registry;
    using entity_type = typename Registry_type::entity_type;
    using archetype_list_type = typename prune_for_t<Registry_type::template archetype_list_type, Components...>;

  public:
    basic_view(Registry_type* registry) : _registry { registry } {}

    template<size_t I = 0, typename Func>
    void for_each(const Func& func);

    template<size_t I = 0>
    void erase(const entity_type entity);

    template<size_t I = 0>
    void set(const entity_type entity, Components&&... components);

    template<size_t I = 0, typename Component>
    Component& get(const entity_type entity);

    template<size_t I = 0>
    bool contains(const entity_type entity) const;

    template<size_t I = 0>
    size_t size() const;

  private:
    Registry_type* _registry;
  };

  template<typename Registry, typename... Components>
  template<size_t I, typename Func>
  void basic_view<Registry, Components...>::for_each(const Func& func)
  {
    using current = typename at_t<I, archetype_list_type>;

    if constexpr (I < size_v<archetype_list_type>)
    {
      auto& storage = _registry->template access<current>();

      for (auto it = storage.begin(); it != storage.end(); ++it)
      {
        std::apply(func, std::make_tuple(*it, it.template unpack<Components>()...));
      }

      for_each<I + 1>(func);
    }
  }

  template<typename Registry, typename... Components>
  template<size_t I>
  void basic_view<Registry, Components...>::erase(const entity_type entity)
  {
    using current = typename at_t<I, archetype_list_type>;

    if constexpr (I == size_v<archetype_list_type>) throw std::invalid_argument("Entity does not exist!");
    else
    {
      auto& storage = _registry->template access<current>();

      if (storage.contains(entity)) storage.erase(entity);
      else
        erase<I + 1>(entity);
    }
  }

  template<typename Registry, typename... Components>
  template<size_t I>
  void basic_view<Registry, Components...>::set(const entity_type entity, Components&&... components)
  {
    using current = typename at_t<I, archetype_list_type>;

    if constexpr (I == size_v<archetype_list_type>) throw std::invalid_argument("Entity does not exist!");
    else
    {
      auto& storage = _registry->template access<current>();

      if (storage.contains(entity))
      {
        ((storage::template unpack<Components>() = std::move(components)), ...);
      }
      else
        set<I + 1>(entity, std::move(components)...);
    }
  }

  template<typename Registry, typename... Components>
  template<size_t I, typename Component>
  Component& basic_view<Registry, Components...>::get(const entity_type entity)
  {
    using current = typename at_t<I, archetype_list_type>;

    if constexpr (I == size_v<archetype_list_type>) throw std::invalid_argument("Entity does not exist!");
    else
    {
      auto& storage = _registry->template access<current>();

      if (storage.contains(entity)) return storage::template unpack<Component>();
      else
        return get<I + 1>(entity);
    }
  }

  template<typename Registry, typename... Components>
  template<size_t I>
  bool basic_view<Registry, Components...>::contains(const entity_type entity) const
  {
    using current = typename at_t<I, archetype_list_type>;

    if constexpr (I == size_v<archetype_list_type>) return false;
    else if (_registry->template access<current>().contains(entity))
      return true;
    else
      return contains<I + 1>(entity);
  }

  template<typename Registry, typename... Components>
  template<size_t I>
  size_t basic_view<Registry, Components...>::size() const
  {
    using current = typename at_t<I, archetype_list_type>;

    if constexpr (I == size_v<archetype_list_type>) return 0;
    else
      return _registry->template access<current>().size() + size<I + 1>();
  }
} // namespace internal

template<typename Entity, typename... Archetypes>
registry<Entity, list<Archetypes...>>::registry()
{
  setup_shared_memory();
}

template<typename Entity, typename... Archetypes>
template<size_t I>
void registry<Entity, list<Archetypes...>>::setup_shared_memory()
{
  using current = typename at_t<I, archetype_list_type>;

  if constexpr (I < size_v<archetype_list_type>)
  {
    access<current>().share(&_shared);
    setup_shared_memory<I + 1>();
  }
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
Entity registry<Entity, list<Archetypes...>>::create(Components&&... components)
{
  static_assert(size_v<prune_for_t<list<Archetypes...>, Components...>> > 0,
    "Registry does not contain suitable archetype for provided components");

  using current = typename find_for_t<list<Archetypes...>, Components...>;

  const entity_type entity = _manager.generate();

  access<current>().insert(entity, std::move(components)...);

  return entity;
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
void registry<Entity, list<Archetypes...>>::destroy(const entity_type entity)
{
  static_assert(size_v<prune_for_t<list<Archetypes...>, Components...>> > 0,
    "Registry does not contain suitable archetype for provided components");

  view<Components...>().erase(entity);

  _manager.release(entity);
}

template<typename Entity, typename... Archetypes>
void registry<Entity, list<Archetypes...>>::destroy_all()
{
  ((access<Archetypes>().clear()), ...);

  _manager.release_all();
}

template<typename Entity, typename... Archetypes>
void registry<Entity, list<Archetypes...>>::optimize()
{
  ((access<Archetypes>().shrink_to_fit()), ...);

  _manager.swap();
  _manager.shrink_to_fit();
}

} // namespace ecs
