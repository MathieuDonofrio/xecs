#pragma once

#include <cstdlib>
#include <limits>
#include <tuple>
#include <type_traits>

namespace std
{
template<typename T, typename Tuple>
struct find_index;

template<typename T, typename... Types>
struct find_index<T, std::tuple<T, Types...>> : std::integral_constant<size_t, 0>
{};

template<typename T, typename U, typename... Types>
struct find_index<T, std::tuple<U, Types...>> : std::integral_constant<size_t, 1 + find_index<T, std::tuple<Types...>>::value>
{};

template<typename Type, typename Tuple>
constexpr Type& get(const Tuple& tuple) { return std::get<find_index<Type, Tuple>::value>(tuple); }
} // namespace std

namespace ecs
{
template<typename... Types>
struct list
{};

template<typename List>
struct size;

template<typename... Types, template<typename...> typename List>
struct size<List<Types...>> : std::integral_constant<size_t, sizeof...(Types)>
{};

template<typename List>
inline constexpr auto size_v = size<List>::value;

template<typename List>
struct empty;

template<typename... Types, template<typename...> typename List>
struct empty<List<Types...>> : std::bool_constant<sizeof...(Types) == 0>
{};

template<typename List>
inline constexpr auto empty_v = empty<List>::value;

template<typename Type, typename List>
struct contains;

template<typename Type, typename... Types, template<typename...> typename List>
struct contains<Type, List<Types...>> : std::disjunction<std::is_same<Type, Types>...>
{};

template<typename Type, typename List>
inline constexpr auto contains_v = contains<Type, List>::value;

template<typename List, typename... Types>
struct contains_all;

template<typename List, typename... Types>
struct contains_all : std::conjunction<contains<Types, List>...>
{};

template<typename List, typename... Types>
inline constexpr auto contains_all_v = contains_all<List, Types...>::value;

template<typename... Types>
struct unique_types : std::true_type
{};

template<typename Type, typename... Types>
struct unique_types<Type, Types...>
  : std::conjunction<std::negation<std::disjunction<std::is_same<Type, Types>...>>, unique_types<Types...>>
{};

template<typename... Types>
inline constexpr auto unique_types_v = unique_types<Types...>::value;

template<typename AList, typename BList>
struct is_same_types;

template<typename... ATypes, template<typename...> typename AList, typename... BTypes, template<typename...> typename BList>
struct is_same_types<AList<ATypes...>, BList<BTypes...>>
  : std::conjunction<std::bool_constant<sizeof...(ATypes) == sizeof...(BTypes)>, contains_all<AList<ATypes...>, BTypes...>>
{};

template<typename AList, typename BList>
inline constexpr auto is_same_types_v = is_same_types<AList, BList>::value;

template<typename Type, typename List>
struct concat;

template<typename Type, typename... Types, template<typename...> typename List>
struct concat<Type, List<Types...>>
{
  using type = List<Type, Types...>;
};

template<typename Type, typename List>
using concat_t = typename concat<Type, List>::type;

template<typename ListOfLists, typename... RequiredTypes>
struct find_for;

template<typename... Lists, template<typename...> typename ListOfLists, typename... RequiredTypes>
struct find_for<ListOfLists<Lists...>, RequiredTypes...>
{
  using type = list<>;
};

template<typename HeadList, typename... Lists, template<typename...> typename ListOfLists, typename... RequiredTypes>
struct find_for<ListOfLists<HeadList, Lists...>, RequiredTypes...>
{
private:
  using next = typename find_for<ListOfLists<Lists...>, RequiredTypes...>::type;

public:
  using type = typename std::conditional_t<contains_all<HeadList, RequiredTypes...>::value, HeadList, next>;
};

template<typename ListOfLists, typename... RequiredTypes>
using find_for_t = typename find_for<ListOfLists, RequiredTypes...>::type;

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

template<typename... Components>
struct archetype : list<Components...>
{
  static_assert(unique_types_v<Components...>, "Archetypes cannot contain duplicate components");
  static_assert(std::conjunction_v<std::is_same<Components, std::remove_cv_t<Components>>...>, "Archetype components cannot be cv-qualified (const or volatile)");
  static_assert(std::conjunction_v<std::is_default_constructible<Components>...>, "Archetype components must be default constructible");
  static_assert(std::conjunction_v<std::is_move_assignable<Components>...>, "Archetype components must be move assignable");
  static_assert(std::conjunction_v<std::is_move_constructible<Components>...>, "Archetype components must be move constructible");
  static_assert(std::conjunction_v<std::is_copy_assignable<Components>...>, "Archetype components must be copy assignable");
  static_assert(std::conjunction_v<std::is_copy_constructible<Components>...>, "Archetype components myst be copy constructible");
};

namespace internal
{
  template<typename... Archetypes>
  struct registry_builder
  {
    template<typename Archetype>
    using add = registry_builder<Archetype, Archetypes...>;
    using build = list<Archetypes...>;
  };
} // namespace internal

using registry_builder = internal::registry_builder<>;

} // namespace ecs
