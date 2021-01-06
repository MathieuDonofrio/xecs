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
template<typename Registry, typename... Components>
class basic_view;

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
  using shared_type = std::shared_ptr<internal::sparse_array<entity_type>>;

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

  void optimize();

  template<typename... Components>
  auto view() const { return basic_view<registry_type, Components...> { const_cast<registry_type*>(this) }; }

  template<typename... Components>
  auto view() { return basic_view<registry_type, Components...> { this }; }

  template<typename... Components>
  bool has_components(const entity_type entity) const { return view<Components...>().contains(entity); }

  template<typename... Components>
  size_t size() const { return view<Components...>().size(); }

private:
  friend basic_view;

  template<size_t I = 0>
  void setup_shared_memory();

  template<typename Archetype>
  const storage<entity_type, Archetype>& access() const { return std::get<storage<entity_type, Archetype>>(_pool); }

  template<typename Archetype>
  storage<entity_type, Archetype>& access() { return std::get<storage<entity_type, Archetype>>(_pool); }

private:
  manager_type _manager;
  pool_type _pool;
  shared_type _shared;
};

template<typename Registry, typename... Components>
class basic_view
{
public:
  using Registry_type = Registry;
  using entity_type = typename Registry_type::entity_type;
  using archetype_list_type = typename prune_for_t<Registry_type::template archetype_list_type, Components...>;

public:
  basic_view(Registry_type* registry) : _registry { registry } {}

  template<size_t I = 0, typename Action>
  void storage_action(const entity_type entity, const Action action);

  template<size_t I = 0, typename Func>
  void for_each(Func func);

  template<size_t I = 0>
  bool contains(const entity_type entity) const;

  template<size_t I = 0>
  size_t size() const;

private:
  Registry_type* _registry;
};

template<typename Registry, typename... Components>
template<size_t I, typename Action>
void basic_view<Registry, Components...>::storage_action(const entity_type entity, const Action action)
{
  using current = typename at_t<I, archetype_list_type>;

  if constexpr (I == size_v<archetype_list_type>) throw std::invalid_argument("Entity does not exist!");
  else
  {
    auto& storage = _registry->template access<current>();

    if (storage.contains(entity)) action(storage);
    else
      storage_action<I + 1, Action>(entity, action);
  }
  (void)entity; // Suppress warning
  (void)action; // Suppress warning
}

template<typename Registry, typename... Components>
template<size_t I, typename Func>
void basic_view<Registry, Components...>::for_each(const Func func)
{
  using current = typename at_t<I, archetype_list_type>;

  if constexpr (I < size_v<archetype_list_type>)
  {
    auto& storage = _registry->template access<current>();

    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
      std::apply(func, std::make_tuple(*it, it.template get<Components>()...));
    }

    for_each<I + 1>(func);
  }
  else
    (void)func; // Suppress warning
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

template<typename Entity, typename... Archetypes>
registry<Entity, list<Archetypes...>>::registry()
{
  _shared = std::make_shared<internal::sparse_array<entity_type>>();
  setup_shared_memory();
}

template<typename Entity, typename... Archetypes>
template<size_t I>
void registry<Entity, list<Archetypes...>>::setup_shared_memory()
{
  using current = typename at_t<I, archetype_list_type>;

  if constexpr (I < size_v<archetype_list_type>)
  {
    access<current>().make_shared(_shared);
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

  view<Components...>().storage_action(entity, [entity](auto& storage)
    { storage.erase(entity); });

  _manager.release(entity);
}

template<typename Entity, typename... Archetypes>
void registry<Entity, list<Archetypes...>>::optimize()
{
  ((access<Archetypes>().shrink_to_fit()), ...);

  _manager.swap();
  _manager.shrink_to_fit();
}

} // namespace ecs
