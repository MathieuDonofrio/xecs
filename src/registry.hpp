#pragma once

#include "archetype.hpp"
#include "entity_manager.hpp"
#include "storage.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace ecs
{
template<typename Entity, typename ArchetypeList>
class registry;

template<typename Entity, typename... Archetypes>
class registry<Entity, list<Archetypes...>> : verify_archetype_list<list<Archetypes...>>
{
public:
  using entity_type = Entity;
  using archetype_list_type = list<Archetypes...>;
  using registry_type = registry<entity_type, archetype_list_type>;
  using pool_type = std::tuple<storage<entity_type, Archetypes>...>;
  using shared_type = sparse_array<entity_type>;
  using manager_type = entity_manager<entity_type>;

  static_assert(sizeof...(Archetypes) > 0, "Registry must contain atleast one archetype");

private:
  template<typename... Components>
  class basic_view;

public:
  registry();
  registry(const registry&) = delete;
  registry(registry&&) = delete;

  registry& operator=(const registry&) = delete;

  template<typename... Components>
  entity_type create(const Components&... components);

  template<typename... Components>
  void destroy(const entity_type entity);

  void destroy_all();

  void optimize();

  template<typename... Components, typename Func>
  void for_each(const Func& func) { view<Components...>().for_each(func); }

  template<typename Component>
  Component& unpack(const entity_type entity) { return view<Component>().template unpack<Component>(entity); }

  template<typename... Components>
  bool has(const entity_type entity) { return view<Components...>().contains(entity); }

  template<typename... Components>
  size_t size() { return view<Components...>().size(); }

  template<typename... Components>
  bool empty() { return size<Components...>() == 0; }

  template<typename... Components>
  auto view() { return basic_view<Components...> { this }; }

  size_t storages() { return _shared.shared(); }

private:
  template<typename Archetype>
  storage<entity_type, Archetype>& access() { return std::get<storage<entity_type, Archetype>>(_pool); }

  template<size_t I = 0>
  void setup_shared_memory();

private:
  pool_type _pool;
  shared_type _shared;
  manager_type _manager;
};

template<typename Entity, typename... Archetypes>
template<typename... Components>
class registry<Entity, list<Archetypes...>>::basic_view
{
public:
  using archetype_list_view_type = prune_for_t<archetype_list_type, Components...>;

  static_assert(size_v<archetype_list_view_type> > 0, "View must contain atleast one archetype");

public:
  basic_view(registry_type* registry) : _registry { registry } {}

  template<size_t I = 0, typename Func>
  void for_each(const Func& func);

  template<typename Component, size_t I = 0>
  Component& unpack(const entity_type entity);

  template<size_t I = 0>
  bool contains(const entity_type entity);

  template<size_t I = 0>
  size_t size();

private:
  friend registry_type;

  template<size_t I = 0>
  void erase(const entity_type entity);

private:
  registry_type* _registry;
};

template<typename Entity, typename... Archetypes>
template<typename... Components>
template<size_t I, typename Func>
void registry<Entity, list<Archetypes...>>::basic_view<Components...>::for_each(const Func& func)
{
  using current = at_t<I, archetype_list_view_type>;

  auto& storage = _registry->template access<current>();

  for (auto it = storage.begin(); it != storage.end(); ++it)
  {
    func(*it, it.template unpack<Components>()...);
  }

  if constexpr (I + 1 < size_v<archetype_list_view_type>) for_each<I + 1>(func);
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
template<typename Component, size_t I>
Component& registry<Entity, list<Archetypes...>>::basic_view<Components...>::unpack(const entity_type entity)
{
  using current = at_t<I, archetype_list_view_type>;

  if constexpr (I == size_v<archetype_list_view_type>)
    throw std::invalid_argument("Entity " + std::to_string(entity) + " does not exist in the view!");
  else
  {
    auto& storage = _registry->template access<current>();

    if (storage.contains(entity)) return storage.template unpack<Component>(entity);
    else
      return unpack<Component, I + 1>(entity);
  }
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
template<size_t I>
bool registry<Entity, list<Archetypes...>>::basic_view<Components...>::contains(const entity_type entity)
{
  using current = at_t<I, archetype_list_view_type>;

  if (_registry->template access<current>().contains(entity)) return true;

  if constexpr (I + 1 == size_v<archetype_list_view_type>) return false;
  else
    return contains<I + 1>(entity);
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
template<size_t I>
size_t registry<Entity, list<Archetypes...>>::basic_view<Components...>::size()
{
  using current = at_t<I, archetype_list_view_type>;

  if constexpr (I == size_v<archetype_list_view_type>) return 0;
  else
    return _registry->template access<current>().size() + size<I + 1>();
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
template<size_t I>
void registry<Entity, list<Archetypes...>>::basic_view<Components...>::erase(const entity_type entity)
{
  using current = at_t<I, archetype_list_view_type>;

  if constexpr (I == size_v<archetype_list_view_type>)
    throw std::invalid_argument("Entity " + std::to_string(entity) + " does not exist in the view!");
  else
  {
    auto& storage = _registry->template access<current>();

    if (storage.contains(entity)) storage.erase(entity);
    else
      erase<I + 1>(entity);
  }
}

template<typename Entity, typename... Archetypes>
registry<Entity, list<Archetypes...>>::registry()
{
  setup_shared_memory();
}

template<typename Entity, typename... Archetypes>
template<size_t I>
void registry<Entity, list<Archetypes...>>::setup_shared_memory()
{
  using current = at_t<I, archetype_list_type>;

  if constexpr (I < size_v<archetype_list_type>)
  {
    access<current>().share(&_shared);
    setup_shared_memory<I + 1>();
  }
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
Entity registry<Entity, list<Archetypes...>>::create(const Components&... components)
{
  using current = find_for_t<list<Archetypes...>, Components...>;

  static_assert(size_v<current> == sizeof...(Components),
    "Registry does not contain suitable archetype for provided components");

  const entity_type entity = _manager.generate();

  access<current>().insert(entity, components...);

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
