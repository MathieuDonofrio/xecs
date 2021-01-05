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

template<typename Entity, typename ArchetypeList>
class registry;

template<typename Entity, typename... Archetypes>
class registry<Entity, archetype_list<Archetypes...>>
{
public:
  using entity_type = Entity;
  using archetype_list_type = archetype_list<Archetypes...>;
  using registry_type = registry<entity_type, archetype_list_type>;
  using manager_type = entity_manager<entity_type>;
  using pool_type = std::tuple<storage<entity_type, Archetypes>...>;
  using shared_type = std::shared_ptr<sparse_array<entity_type>>;

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
  using registry_type = Registry;
  using entity_type = typename registry_type::entity_type;
  using archetype_list_type = typename registry_type::archetype_list_type::template purge_for_t<Components...>;

public:
  basic_view(registry_type* registry) : _registry { registry } {}

  template<size_t I = 0, typename Action>
  void storage_action(const entity_type entity, const Action action);

  template<size_t I = 0, typename Func>
  void each(Func func);

  template<size_t I = 0>
  bool contains(const entity_type entity) const;

  template<size_t I = 0>
  size_t size() const;

private:
  registry_type* _registry;
};

template<typename Registry, typename... Components>
template<size_t I, typename Action>
void basic_view<Registry, Components...>::storage_action(const entity_type entity, const Action action)
{
  using current = typename archetype_at<I, archetype_list_type>::type;

  if constexpr (I == archetype_list_type::size::value)
  {
    throw std::invalid_argument("Entity does not exist!");
  }
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
  using current = typename archetype_at<I, archetype_list_type>::type;

  if constexpr (I < archetype_list_type::size::value)
  {
    auto& storage = _registry->template access<current>();

    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
      std::apply(func, std::make_tuple(*it, it.template get<Components>()...));
    }

    each<I + 1>(func);
  }
  else
    (void)func; // Suppress warning
}

template<typename Registry, typename... Components>
template<size_t I>
bool basic_view<Registry, Components...>::contains(const entity_type entity) const
{
  using current = typename archetype_at<I, archetype_list_type>::type;

  if constexpr (I == archetype_list_type::size::value) return false;
  else if (_registry->template access<current>().contains(entity))
    return true;
  else
    return contains<I + 1>(entity);
}

template<typename Registry, typename... Components>
template<size_t I>
size_t basic_view<Registry, Components...>::size() const
{
  using current = typename archetype_at<I, archetype_list_type>::type;

  if constexpr (I == archetype_list_type::size::value) return 0;
  else
    return _registry->template access<current>().size() + size<I + 1>();
}

template<typename Entity, typename... Archetypes>
registry<Entity, archetype_list<Archetypes...>>::registry()
{
  _shared = std::make_shared<sparse_array<entity_type>>();
  setup_shared_memory();
}

template<typename Entity, typename... Archetypes>
template<size_t I>
void registry<Entity, archetype_list<Archetypes...>>::setup_shared_memory()
{
  using current = typename archetype_at<I, archetype_list_type>::type;

  if constexpr (I < archetype_list_type::size::value)
  {
    access<current>().make_shared(_shared);
    setup_shared_memory<I + 1>();
  }
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
Entity registry<Entity, archetype_list<Archetypes...>>::create(Components&&... components)
{
  static_assert(archetype_list<Archetypes...>::template contains<archetype<Components...>>::value,
    "Registry does not contain suitable archetype for provided components");

  const entity_type entity = _manager.generate();

  access<archetype<Components...>>().insert(entity, std::move(components)...);

  return entity;
}

template<typename Entity, typename... Archetypes>
template<typename... Components>
void registry<Entity, archetype_list<Archetypes...>>::destroy(const entity_type entity)
{
  static_assert(archetype_list<Archetypes...>::template purge_for_t<Components...>::size::value > 0,
    "Registry does not contain any archetypes with all provided components");

  view<Components...>().storage_action(entity, [entity](auto& storage)
    { storage.erase(entity); });

  _manager.release(entity);
}

template<typename Entity, typename... Archetypes>
void registry<Entity, archetype_list<Archetypes...>>::optimize()
{
  ((access<Archetypes>().shrink_to_fit()), ...);

  _manager.swap();
  _manager.shrink_to_fit();
}

} // namespace ecs
