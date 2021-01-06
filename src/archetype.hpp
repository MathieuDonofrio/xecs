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

namespace std
{
static_assert(find_index<int, std::tuple<int>>::value == 0, "");
static_assert(find_index<int, std::tuple<int, float>>::value == 0, "");
static_assert(find_index<int, std::tuple<float, int>>::value == 1, "");
static_assert(find_index<int, std::tuple<float, int, char>>::value == 1, "");
static_assert(find_index<int, std::tuple<float, bool, int>>::value == 2, "");
static_assert(find_index<float, std::tuple<double, float, bool, int>>::value == 1, "");
} // namespace std

namespace ecs
{
static_assert(size_v<list<>> == 0, "");
static_assert(size_v<list<float>> == 1, "");
static_assert(size_v<list<int, float>> == 2, "");
static_assert(size_v<list<int, float, bool>> == 3, "");

static_assert(empty_v<list<>> == true, "");
static_assert(empty_v<list<int>> == false, "");
static_assert(empty_v<list<float, int>> == false, "");

static_assert(contains_v<int, list<>> == false, "");
static_assert(contains_v<int, list<int>> == true, "");
static_assert(contains_v<int, list<int, float>> == true, "");
static_assert(contains_v<int, list<float, int>> == true, "");
static_assert(contains_v<float, list<float, int>> == true, "");
static_assert(contains_v<bool, list<float, int>> == false, "");
static_assert(contains_v<bool, list<float, bool, int>> == true, "");
static_assert(contains_v<int, list<float, bool, int>> == true, "");

static_assert(contains_all_v<list<>> == true, "");
static_assert(contains_all_v<list<>, int> == false, "");
static_assert(contains_all_v<list<int>, int> == true, "");
static_assert(contains_all_v<list<int>, int, float> == false, "");
static_assert(contains_all_v<list<int, float>, int> == true, "");
static_assert(contains_all_v<list<int, float>, int, float> == true, "");
static_assert(contains_all_v<list<float, int>, int, float> == true, "");
static_assert(contains_all_v<list<float, int>, float, int> == true, "");
static_assert(contains_all_v<list<float, int>, float, int, bool> == false, "");
static_assert(contains_all_v<list<float, int, bool>, float, int> == true, "");
static_assert(contains_all_v<list<float, int, bool>, float, int, bool> == true, "");
static_assert(contains_all_v<list<float, bool, int>, float, int, bool> == true, "");

static_assert(unique_types_v<> == true, "");
static_assert(unique_types_v<int> == true, "");
static_assert(unique_types_v<int, double> == true, "");
static_assert(unique_types_v<int, int> == false, "");
static_assert(unique_types_v<int, double, float> == true, "");
static_assert(unique_types_v<int, double, int> == false, "");
static_assert(unique_types_v<int, double, float, bool> == true, "");
static_assert(unique_types_v<int, double, bool, bool> == false, "");
static_assert(unique_types_v<int, double, float, bool, char> == true, "");
static_assert(unique_types_v<int, double, float, bool, int, char> == false, "");

static_assert(is_same_types_v<list<>, list<>> == true, "");
static_assert(is_same_types_v<list<int>, list<>> == false, "");
static_assert(is_same_types_v<list<int>, list<int>> == true, "");
static_assert(is_same_types_v<list<float>, list<int>> == false, "");
static_assert(is_same_types_v<list<float>, list<float>> == true, "");
static_assert(is_same_types_v<list<int>, list<int, float>> == false, "");
static_assert(is_same_types_v<list<int, float>, list<int, float>> == true, "");
static_assert(is_same_types_v<list<float, int>, list<int, float>> == true, "");
static_assert(is_same_types_v<list<float, int>, list<int, bool>> == false, "");
static_assert(is_same_types_v<list<float, int>, list<int, bool, float>> == false, "");
static_assert(is_same_types_v<list<bool, float, int>, list<int, bool, float>> == true, "");
static_assert(is_same_types_v<list<float, bool, int>, list<int, float, bool>> == true, "");
static_assert(is_same_types_v<list<bool, float, int, double>, list<int, bool, float>> == false, "");

static_assert(std::is_same_v<list<int>, concat_t<int, list<>>>, "");
static_assert(std::is_same_v<list<int, float>, concat_t<int, list<float>>>, "");
static_assert(std::is_same_v<list<double, bool, float>, concat_t<double, list<bool, float>>>, "");

static_assert(std::is_same_v<list<>, prune_for_t<list<list<int>>, float>>, "");
static_assert(std::is_same_v<list<list<int>>, prune_for_t<list<list<int>>, int>>, "");
static_assert(std::is_same_v<list<list<float>>, prune_for_t<list<list<int>, list<float>>, float>>, "");
static_assert(std::is_same_v<list<list<int>>, prune_for_t<list<list<int>, list<float>>, int>>, "");
static_assert(std::is_same_v<list<list<int>, list<float, int>>, prune_for_t<list<list<int>, list<float, int>>, int>>, "");
static_assert(std::is_same_v<list<list<int>, list<float, int>>, prune_for_t<list<list<int>, list<float, int>>, int>>, "");
static_assert(std::is_same_v<list<list<int>, list<float, int>>, prune_for_t<list<list<int>, list<float, int>, list<bool>>, int>>, "");
static_assert(std::is_same_v<list<list<bool>>, prune_for_t<list<list<int>, list<float, int>, list<bool>>, bool>>, "");
static_assert(std::is_same_v<list<list<float, int>>, prune_for_t<list<list<int, bool>, list<float, int>, list<bool>>, float>>, "");
static_assert(std::is_same_v<list<list<int, bool>, list<float, int>>, prune_for_t<list<list<int, bool>, list<float, int>, list<bool>>, int>>, "");
static_assert(std::is_same_v<list<list<int, bool>>, prune_for_t<list<list<int, bool>, list<float, int>, list<bool>>, int, bool>>, "");
static_assert(std::is_same_v<list<>, prune_for_t<list<list<int, bool>, list<float, int>, list<bool>>, int, bool, float>>, "");
static_assert(std::is_same_v<list<list<float, int>>, prune_for_t<list<list<int, bool>, list<float, int>, list<bool>>, float, int>>, "");
static_assert(std::is_same_v<list<list<int, bool, float>, list<float, int>>, prune_for_t<list<list<int, bool, float>, list<float, int>, list<bool>>, float, int>>, "");

static_assert(std::is_same_v<int, at_t<0, list<int>>>, "");
static_assert(std::is_same_v<float, at_t<0, list<float, int>>>, "");
static_assert(std::is_same_v<int, at_t<1, list<float, int>>>, "");
static_assert(std::is_same_v<double, at_t<0, list<double, float, int>>>, "");
static_assert(std::is_same_v<float, at_t<1, list<double, float, int>>>, "");
static_assert(std::is_same_v<int, at_t<2, list<double, float, int>>>, "");
} // namespace ecs
