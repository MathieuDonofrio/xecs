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
#include <string>
#include <tuple>
#include <utility>

namespace ecs
{
/**
 * @brief Entity-component system core contaner.
 * 
 * A registry contains a entity_manager, one or many storages depending on the amount of archetypes, and one
 * sparse_array that is shared with all storages.
 * 
 * The registry is the glue that puts everything together in a wonderfull way and abstracts away
 * much of the managment of your archetypes. You can create views of this registry for the components
 * you need, and operate on those views.
 * 
 * Because of the usage of archetypes, this entity-component system is very scalable and does not need
 * any special optimizations to run as fast as possible. Everything is efficient by default.
 * 
 * This registry leverages its knowledge of all achetypes at compile time, to reduce 
 * the complexity of many operations, who often times can be reduced to nearly no overhead.
 * 
 * @tparam Entity The unsigned integer entity type
 * @tparam ArchetypeList The list of all archetypes to be used by this registry
 */
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

  static_assert(std::numeric_limits<entity_type>::is_integer && !std::numeric_limits<entity_type>::is_signed,
    "Entity type must be an unsigned integer");
  static_assert(sizeof...(Archetypes) > 0, "Registry must contain atleast one archetype");

private:
  template<typename... Components>
  class basic_view;

public:
  /**
   * @brief Construct a new registry object
   */
  registry()
  {
    setup_shared_memory();
  }

  registry(const registry&) = delete;
  registry(registry&&) = delete;
  registry& operator=(const registry&) = delete;

  /**
   * @brief Create an entity and initializes it with the given components.
   * 
   * In order for the create method to know what archetype is going to be created, all components
   * of the archetype must be specified and initialized in the create method. The order of the components
   * doesn't matter as long as the components match the exact components of one of the registry archetypes.
   * 
   * This operation is always O(1) and is very fast.
   * 
   * @tparam Components The exact component types of one of the registry archetypes
   * @param components The components to initialize with
   * @return entity_type The created entity's identifier
   */
  template<typename... Components>
  entity_type create(const Components&... components)
  {
    using current = find_for_t<list<Archetypes...>, Components...>;

    static_assert(size_v<current> == sizeof...(Components),
      "Registry does not contain suitable archetype for provided components");

    const entity_type entity = _manager.generate();

    access<current>().insert(entity, components...);

    return entity;
  }

  /**
   * @brief Destroys the specified entity.
   * 
   * This operation is very cheap but has a complexity of O(N) where N is the amount
   * of archetypes. This complexity can be compleatly or partially reduced by specifying
   * all or some of the types of the entity's archetype. You should probably be doing this.
   * 
   * @throws std::invalid_argument if the entity with the specified components does not 
   * exist in the registry.
   * 
   * @tparam Components Component types that you know this entity's archetype has
   * @param entity The entity to destroy
   */
  template<typename... Components>
  void destroy(const entity_type entity)
  {
    static_assert(size_v<prune_for_t<list<Archetypes...>, Components...>> > 0,
      "Registry does not contain suitable archetype for provided components");

    view<Components...>().erase(entity);

    _manager.release(entity);
  }

  /**
   * @brief Destroys all entites in the registry.
   * 
   * This is a very cheap way to destroy all entities. Much faster than destroying each one
   * individually. You should use this if you can.
   */
  void destroy_all()
  {
    ((access<Archetypes>().clear()), ...);

    _manager.release_all();
  }

  /**
   * @brief Performs certain optimizations on the registry such as memory optimization.
   * 
   * @note This will mainly do memory optimizations, but in the future this may change.
   * 
   * This can be quite expensive to call often. Not recommended to call every frame.
   * However, every once in a while, it can be usefull to call this method in a clever way
   * to recalibrate everything and make sure your ressources are being use optimally.
   * 
   * @note You should consider using this if your application is running for a long time.
   */
  void optimize()
  {
    ((access<Archetypes>().shrink_to_fit()), ...);

    _manager.swap();
    _manager.shrink_to_fit();
  }

  /**
   * @brief Iterates over every entity that has the specified components and calls the given function.
   * 
   * Same thing as creating a view with the components you need and calling for_each.
   * 
   * @tparam Components The components types to form the view for
   * @tparam Func The function type
   * @param func The function to call on every iteration
   */
  template<typename... Components, typename Func>
  void for_each(const Func& func) { view<Components...>().for_each(func); }

  /**
   * @brief Returns a reference of the stored component for the specified entity and component type.
   * 
   * Unpacking components this way is the simplest, but the most expensive way to do it. A view with
   * only the specified component is created, so this method may need to check multiple storages
   * to find where the entity is stored. A better alternative is to create the view yourself, with
   * all the components you know the entity's archetype contains, this can significantly improve
   * performance. However, obtaining the components from iteration is always the best way and has no cost.
   * 
   * @throws std::invalid_argument if the entity with the specified component does not 
   * exist in the registry.
   * 
   * @tparam Component The component type to unpack
   * @param entity Entity to unpack component for
   * @return Component& Reference to component belonging to the entity
   */
  template<typename Component>
  Component& unpack(const entity_type entity) { return view<Component>().template unpack<Component>(entity); }

  /**
   * @brief Returns whether or not the entity has all the specified components.
   * 
   * Not very expensive to do, but you shouldn't need to call this method in most cases.
   * If your finding yourself calling this method often, then your probably doing something wrong.
   * 
   * If you want to make this method faster, you can specify the types that you know the 
   * component has, this can reduce some of the complexity.
   * 
   * @note Specifying more component types does not make this method slower, it can even
   * make it faster in some cases.
   * 
   * @tparam Components The component types to check for
   * @param entity The entity to check for
   * @return true If the entity has all the specified component types, false otherwise
   */
  template<typename... Components>
  bool has(const entity_type entity) { return view<Components...>().contains(entity); }

  /**
   * @brief Returns the amount of entities contained by this registry who have the specified components if any.
   * 
   * If no components are specified, this will the amount of all entities in the registry.
   * 
   * @note The more components are specified, the faster this method becomes.
   * 
   * @tparam Components The required components to be counted in the returned size
   * @return size_t The amount of entities in the registry that have the specified components
   */
  template<typename... Components>
  size_t size() { return view<Components...>().size(); }

  /**
   * @brief Returns whether or not the registry contains any entities with the specified components if any.
   * 
   * If no components are specified, this will return whether or not the registry contains ANY entities.
   * 
   * @note Uses the registry size method under the hood.
   * 
   * @tparam Components The required components to be counted in the check
   * @return true If the registry contains any entities that have the specified components.
   */
  template<typename... Components>
  bool empty() { return size<Components...>() == 0; }

  /**
   * @brief Returns a view of the registry for the specified components.
   * 
   * This is the main operation of the registry. Almost every method in the registry
   * uses a view under the hood.
   * 
   * Views find all the archetypes that contain the specified components at compile-time.
   * 
   * Views are essentially free to create.
   * 
   * @tparam Components The component types to include in the view
   * @return auto A view of the registry for the specified components
   */
  template<typename... Components>
  auto view() { return basic_view<Components...> { this }; }

  size_t storages() { return _shared.shared(); }

private:
  /**
   * @brief Accesses the storage for the specified archetype.
   * 
   * Getting the archetype's storage is done at compile-time.
   * 
   * Obtaining the storage of an archetype is essentially free.
   * 
   * @tparam Archetype The archetype to get the storage for
   * @return auto& The storage of the specified archetype
   */
  template<typename Archetype>
  auto& access() { return std::get<storage<entity_type, Archetype>>(_pool); }

  /**
   * @brief Set the up shared sparse_set
   * 
   * Called during the construction of the registry.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the registry.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   */
  template<size_t I = 0>
  void setup_shared_memory()
  {
    using current = at_t<I, archetype_list_type>;

    if constexpr (I < size_v<archetype_list_type>)
    {
      access<current>().share(&_shared);
      setup_shared_memory<I + 1>();
    }
  }

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

  /**
   * @brief Iterates over every entity that has the specified components and calls the given function.
   * 
   * There is not much of a cost outside of the unpacking cost for iterating over multiple components. 
   * Actually, specifing more components may even lead to better results in some cases. Go crazy...
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @tparam Components The components types to form the view for
   * @tparam Func The function type
   * @param func The function to call on every iteration
   */
  template<size_t I = 0, typename Func>
  void for_each(const Func& func)
  {
    using current = at_t<I, archetype_list_view_type>;

    auto& storage = _registry->template access<current>();

    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
      func(*it, it.template unpack<Components>()...);
    }

    if constexpr (I + 1 < size_v<archetype_list_view_type>) for_each<I + 1>(func);
  }

  /**
   * @brief Returns a reference of the stored component for the specified entity and component type.
   * 
   * You should always prioritize getting your components from iteration.
   * 
   * @warning You cannot unpack a component that is not included in the view.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam Component The component type to unpack
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @param entity Entity to unpack component for
   * @return Component& Reference to component belonging to the entity
   */
  template<typename Component, size_t I = 0>
  Component& unpack(const entity_type entity)
  {
    if constexpr (I == 0)
      static_assert(size_v<prune_for_t<archetype_list_view_type, Component>> > 0,
        "You cannot unpack a component type that is not included in the view");

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

  /**
   * @brief Returns whether or not the view contains the specified entity.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @param entity The entity to check for
   * @return true If the view contains the entity
   */
  template<size_t I = 0>
  bool contains(const entity_type entity)
  {
    using current = at_t<I, archetype_list_view_type>;

    if (_registry->template access<current>().contains(entity)) return true;

    if constexpr (I + 1 == size_v<archetype_list_view_type>) return false;
    else
      return contains<I + 1>(entity);
  }

  /**
   * @brief Returns the amount of entities in the view.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @return size_t The amount of entities in the view
   */
  template<size_t I = 0>
  size_t size()
  {
    using current = at_t<I, archetype_list_view_type>;

    if constexpr (I == size_v<archetype_list_view_type>) return 0;
    else
      return _registry->template access<current>().size() + size<I + 1>();
  }

private:
  friend registry_type;

  /**
   * @brief Erases an entity from the correct storage in the view.
   * 
   * Used internally by the registry to destroy entities. Unsafe and cannot be used externally
   * because this method removes an entity without releasing the entity back into
   * the entity_manager.
   * 
   * @note This method uses recusion to iterate over all the archetypes in the view.
   * 
   * @tparam I Archetype index used during recursion, always leave it at 0
   * @param entity The entity to erase
   */
  template<size_t I = 0>
  void erase(const entity_type entity)
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

private:
  registry_type* _registry;
};
} // namespace ecs
