#pragma once

#include <cstdlib>
#include <limits>
#include <tuple>
#include <type_traits>

namespace ecs
{

/**
 * @brief list of types.
 * 
 * Similar to a tuple, but cannot contain any data. 
 * This is used heavily for our compile-time meta programming.
 * 
 * @tparam Types 
 */
template<typename... Types>
struct list
{};

/**
 * @brief finds size of a list (amount of types).
 * 
 * @tparam List list of types
 */
template<typename List>
struct size;

template<typename... Types, template<typename...> typename List>
struct size<List<Types...>> : std::integral_constant<size_t, sizeof...(Types)>
{};

template<typename List>
inline constexpr auto size_v = size<List>::value;

/**
 * @brief checks if a list is empty.
 * 
 * @tparam List list of types
 */
template<typename List>
struct empty;

template<typename... Types, template<typename...> typename List>
struct empty<List<Types...>> : std::bool_constant<sizeof...(Types) == 0>
{};

template<typename List>
inline constexpr auto empty_v = empty<List>::value;

/**
 * @brief checks if a list contains a type.
 * 
 * @tparam Type type to check for
 * @tparam List list to check
 */
template<typename Type, typename List>
struct contains;

template<typename Type, typename... Types, template<typename...> typename List>
struct contains<Type, List<Types...>> : std::disjunction<std::is_same<Type, Types>...>
{};

/**
 * @brief utility for contains value.
 * 
 * @tparam Type type to check for
 * @tparam List list to check
 */
template<typename Type, typename List>
inline constexpr auto contains_v = contains<Type, List>::value;

/**
 * @brief checks if list contains atleast all required types.
 * 
 * @tparam List list to check
 * @tparam Types required types
 */
template<typename List, typename... Types>
struct contains_all;

template<typename List, typename... Types>
struct contains_all : std::conjunction<contains<Types, List>...>
{};

template<typename List, typename... Types>
inline constexpr auto contains_all_v = contains_all<List, Types...>::value;

/**
 * @brief checks if all types are unique.
 * 
 * Types are determined not to be the same by std::is_same<T1, T2>.
 * 
 * @tparam Types types to check
 */
template<typename... Types>
struct unique_types : std::true_type
{};

template<typename Type, typename... Types>
struct unique_types<Type, Types...>
  : std::conjunction<std::negation<std::disjunction<std::is_same<Type, Types>...>>, unique_types<Types...>>
{};

template<typename... Types>
inline constexpr auto unique_types_v = unique_types<Types...>::value;

/**
 * @brief checks if two lists contain the same types.
 * 
 * Compares the size of both lists and checks if Blist contains all the types of AList.
 * 
 * @tparam AList a list of types
 * @tparam BList a list of types
 */
template<typename AList, typename BList>
struct is_same_types;

template<typename... ATypes, template<typename...> typename AList, typename... BTypes, template<typename...> typename BList>
struct is_same_types<AList<ATypes...>, BList<BTypes...>>
  : std::conjunction<std::bool_constant<sizeof...(ATypes) == sizeof...(BTypes)>, contains_all<AList<ATypes...>, BTypes...>>
{};

template<typename AList, typename BList>
inline constexpr auto is_same_types_v = is_same_types<AList, BList>::value;

/**
 * @brief checks if all lists are unique in terms of types.
 * 
 * This is different than unique_types because it ignores the order of
 * types contained by a list. unique_types would evaluate a list with the same types but in a
 * different order as a different list.
 * 
 * @tparam Lists a list of lists
 */
template<typename... Lists>
struct unique_lists : std::true_type
{};

template<typename List, typename... Lists>
struct unique_lists<List, Lists...>
  : std::conjunction<std::negation<std::disjunction<is_same_types<List, Lists>...>>, unique_lists<Lists...>>
{};

template<typename... Lists>
inline constexpr auto unique_lists_v = unique_lists<Lists...>::value;

/**
 * @brief concatenates a list and a type.
 * 
 * @tparam Type type to concatenate (add)
 * @tparam List list to concatenante type to
 */
template<typename Type, typename List>
struct concat;

template<typename Type, typename... Types, template<typename...> typename List>
struct concat<Type, List<Types...>>
{
  using type = List<Type, Types...>;
};

template<typename Type, typename List>
using concat_t = typename concat<Type, List>::type;

/**
 * @brief finds the first occurence of a list that.
 * 
 * This uses is_same_types, so the list to be found must contain all the specified types,
 * and only those types, nothing more, nothing less.
 * 
 * This is used to find the correct type of list with the specified components for any given order.
 * 
 * @tparam ListOfLists list of lists
 * @tparam Types types of the list to find.
 */
template<typename ListOfLists, typename... Types>
struct find_for;

template<typename... Lists, template<typename...> typename ListOfLists, typename... Types>
struct find_for<ListOfLists<Lists...>, Types...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> typename ListOfLists, typename... Types>
struct find_for<ListOfLists<HeadList, Lists...>, Types...>
{
private:
  using next = typename find_for<ListOfLists<Lists...>, Types...>::type;

public:
  using type = typename std::conditional_t<is_same_types_v<HeadList, list<Types...>>, HeadList, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using find_for_t = typename find_for<ListOfLists, RequiredTypes...>::type;

/**
 * @brief removes all lists that do not contains the required types.
 * 
 * This is used to create views at compile time.
 * 
 * @tparam ListOfLists list of lists
 * @tparam RequiredTypes types that must be present in the list
 */
template<typename ListOfLists, typename... RequiredTypes>
struct prune_for;

template<typename... Lists, template<typename...> typename ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<Lists...>, RequiredTypes...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> typename ListOfLists, typename... RequiredTypes>
struct prune_for<ListOfLists<HeadList, Lists...>, RequiredTypes...>
{
private:
  using next = typename prune_for<ListOfLists<Lists...>, RequiredTypes...>::type;
  using accept = typename concat<HeadList, next>::type;

public:
  using type = typename std::conditional_t<contains_all<HeadList, RequiredTypes...>::value, accept, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using prune_for_t = typename prune_for<ListOfLists, RequiredTypes...>::type;

/**
 * @brief finds the type at the specified index in the list.
 * 
 * @tparam I index in list
 * @tparam List list to operate on
 */
template<size_t I, typename List>
struct at;

template<size_t I, typename... Types, template<typename...> typename List>
struct at<I, List<Types...>>
{
  using type = List<>;
};

template<size_t I, typename Type, typename... Types, template<typename...> typename List>
struct at<I, List<Type, Types...>>
{
private:
  using next = typename at<I - 1, List<Types...>>::type;

public:
  using type = typename std::conditional_t<I == 0, Type, next>;
};

template<size_t I, typename List>
using at_t = typename at<I, List>::type;

/**
 * @brief assert's a component to verify that is is valid.
 * 
 * A valid component cannot be cv-qualified, that means it cant be const or volatile. And most importantly,
 * a component needs to be trivial, that means it cant contains constructors, destructors... EVERYTHING must
 * be trivial.
 * 
 * Components must be strictly data. And we restrict at compile-time going beyond that.
 * These restrictions allows us to make some extra optimizations and enforces a performance by default philosophie.
 * 
 * @tparam Component 
 */
template<typename Component>
struct verify_component
{
  static_assert(std::is_same_v<Component, std::remove_cv_t<Component>>, "Component cannot be cv-qualified (const or volatile)");
  static_assert(std::is_trivial_v<Component>, "Component must be trival (JUST DATA. no constructors, destructors...)");
};

template<typename... Components>
using archetype = list<Components...>;

/**
 * @brief assert's a archetype to verify that it is valid.
 * 
 * A valid archetype must contain only valid components determined by verify_component. Additionnaly,
 * all components in an archetype must be unique.
 * 
 * Archetypes are essentially sets of components.
 * 
 * @tparam Archetype 
 */
template<typename Archetype>
struct verify_archetype;

template<typename... Components>
struct verify_archetype<archetype<Components...>> : verify_component<Components>...
{
  static_assert(unique_types_v<Components...>, "Every components must be unique (archetype is a SET of components)");
};

/**
 * @brief assert's a list of archetypes.
 * 
 * A valid list of archetypes must contain only valid archetypes determined by verify_archetype. Additionnaly,
 * all archetypes in the list must be unique.
 * 
 * For two archetypes to be different, they must be of different sizes or contains different components.
 * Archetypes with the same components but in different orders do not count as unique.
 * 
 * @tparam ArchetypeList 
 */
template<typename ArchetypeList>
struct verify_archetype_list;

template<typename... Archetypes>
struct verify_archetype_list<list<Archetypes...>> : verify_archetype<Archetypes>...
{
  static_assert(unique_lists_v<Archetypes...>, "Every archetype must be unique (regardless of component order)");
};

namespace internal
{
  template<typename... Archetypes>
  struct archetype_list_builder
  {
    template<typename Archetype>
    using add = archetype_list_builder<Archetype, Archetypes...>;
    using build = list<Archetypes...>;
  };
} // namespace internal

/**
 * @brief compile-time builder pattern for creating lists of archetypes.
 * 
 * Alternative to building archetype lists manually that may provide better readability.
 */
struct archetype_list_builder : internal::archetype_list_builder<>
{};

} // namespace ecs
