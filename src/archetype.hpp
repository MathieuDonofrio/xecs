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
struct find_index<T, std::tuple<T, Types...>>
  : std::integral_constant<size_t, 0>
{
};

template<typename T, typename U, typename... Types>
struct find_index<T, std::tuple<U, Types...>>
  : std::integral_constant<size_t, 1 + find_index<T, std::tuple<Types...>>::value>
{
};

static_assert(find_index<int, std::tuple<int>>::value == 0, "");
static_assert(find_index<int, std::tuple<int, float>>::value == 0, "");
static_assert(find_index<int, std::tuple<float, int>>::value == 1, "");
static_assert(find_index<int, std::tuple<float, int, char>>::value == 1, "");
static_assert(find_index<int, std::tuple<float, bool, int>>::value == 2, "");
static_assert(find_index<float, std::tuple<double, float, bool, int>>::value == 1, "");

template<typename Type, typename Tuple>
constexpr Type& get(const Tuple& tuple)
{
  return std::get<find_index<Type, Tuple>::value>(tuple);
}
} // namespace std

namespace ecs
{

template<typename... Components>
struct unique : std::true_type
{
};

template<typename T, typename... Components>
struct unique<T, Components...>
  : std::conjunction<std::negation<std::disjunction<std::is_same<T, Components>...>>, unique<Components...>>
{
};

static_assert(unique<>::value == true, "");
static_assert(unique<int>::value == true, "");
static_assert(unique<int, double>::value == true, "");
static_assert(unique<int, int>::value == false, "");
static_assert(unique<int, double, float>::value == true, "");
static_assert(unique<int, double, int>::value == false, "");
static_assert(unique<int, double, float, bool>::value == true, "");
static_assert(unique<int, double, bool, bool>::value == false, "");
static_assert(unique<int, double, float, bool, char>::value == true, "");
static_assert(unique<int, double, float, bool, int, char>::value == false, "");

struct archetype_base
{
};

template<typename... Components>
struct archetype : archetype_base
{
public:
  static_assert(unique<Components...>::value,
    "Archetype components must be unique");
  static_assert(std::conjunction<std::is_same<Components, std::remove_cv_t<Components>>...>::value,
    "Archetype components cannot be cv-qualified (const or volatile)");
  static_assert(std::conjunction<std::is_default_constructible<Components>...>::value,
    "Archetype components must be default constructible");
  static_assert(std::conjunction<std::is_move_assignable<Components>...>::value,
    "Archetype components must be move assignable");
  static_assert(std::conjunction<std::is_move_constructible<Components>...>::value,
    "Archetype components must be move constructible");
  static_assert(std::conjunction<std::is_copy_assignable<Components>...>::value,
    "Archetype components must be copy assignable");
  static_assert(std::conjunction<std::is_copy_constructible<Components>...>::value,
    "Archetype components myst be copy constructible");

  struct empty : std::bool_constant<sizeof...(Components) == 0>
  {};
  struct size : std::integral_constant<size_t, sizeof...(Components)>
  {};

  template<typename Component>
  struct contains : std::disjunction<std::is_same<Component, Components>...>
  {};

  template<typename... R>
  struct contains_all : std::conjunction<contains<R>...>
  {};
};

static_assert(archetype<>::contains<int>::value == false, "");
static_assert(archetype<bool>::contains<int>::value == false, "");
static_assert(archetype<float>::contains<float>::value == true, "");

static_assert(archetype<>::contains_all<int>::value == false, "");
static_assert(archetype<float>::contains_all<int>::value == false, "");
static_assert(archetype<int>::contains_all<int>::value == true, "");
static_assert(archetype<int, float>::contains_all<float>::value == true, "");
static_assert(archetype<int, float>::contains_all<float, double>::value == false, "");
static_assert(archetype<int, float>::contains_all<int, float>::value == true, "");
static_assert(archetype<int, float, bool>::contains_all<float, bool, int>::value == true, "");

static_assert(archetype<>::empty::value == true, "");
static_assert(archetype<int>::empty::value == false, "");

static_assert(archetype<>::size::value == 0, "");
static_assert(archetype<int>::size::value == 1, "");
static_assert(archetype<int, float, bool>::size::value == 3, "");

template<typename Type>
struct is_archetype : std::is_base_of<archetype_base, Type>
{};

static_assert(is_archetype<int>::value == false, "");
static_assert(is_archetype<archetype<>>::value == true, "");
static_assert(is_archetype<archetype<int, float>>::value == true, "");

template<typename... Archetypes>
struct archetype_list
{
  struct empty : std::bool_constant<true>
  {};
  struct size : std::integral_constant<size_t, 0>
  {};

  template<typename Archetype>
  struct contains : std::false_type
  {
  };

  template<typename... Components>
  struct purge_for
  {
    using type = archetype_list<>;
  };

  template<typename... Components>
  using purge_for_t = typename purge_for<Components...>::type;
};

template<typename T, typename... Archetypes>
struct archetype_list<T, Archetypes...>
{
  static_assert(std::conjunction<is_archetype<T>, is_archetype<Archetypes>...>::value,
    "Archetype List must contain only Archetypes");

  struct empty : std::bool_constant<false>
  {};
  struct size : std::integral_constant<size_t, 1 + sizeof...(Archetypes)>
  {};

  template<typename Archetype>
  struct contains : std::disjunction<std::is_same<Archetype, T>, std::is_same<Archetype, Archetypes>...>
  {};

  template<typename... Components>
  struct purge_for
  {
  private:
    template<typename, typename>
    struct construct;

    template<typename Head, typename... Tail>
    struct construct<Head, archetype_list<Tail...>>
    {
      using type = archetype_list<Head, Tail...>;
    };

    using next = typename archetype_list<Archetypes...>::template purge_for<Components...>::type;

  public:
    using type = typename std::conditional<
      T::template contains_all<Components...>::value,
      typename construct<T, next>::type,
      next>::type;
  };

  template<typename... Components>
  using purge_for_t = typename purge_for<Components...>::type;
};

static_assert(archetype_list<>::contains<archetype<int>>::value == false, "");
static_assert(archetype_list<archetype<int>>::contains<archetype<int>>::value == true, "");
static_assert(archetype_list<archetype<float>>::contains<archetype<int>>::value == false, "");
static_assert(archetype_list<archetype<float, int>, archetype<bool, float>>::contains<archetype<bool, float>>::value == true, "");

template<size_t I, typename List>
struct archetype_at;

template<size_t I, typename... Archetypes>
struct archetype_at<I, archetype_list<Archetypes...>>
{
  using type = archetype<>;
};

template<size_t I, typename T, typename... Archetypes>
struct archetype_at<I, archetype_list<T, Archetypes...>>
{
  using type = typename std::conditional<
    I == 0,
    T,
    typename archetype_at<I - 1, archetype_list<Archetypes...>>::type>::type;
};

static_assert(std::is_same<
                archetype_at<0, archetype_list<archetype<int>, archetype<float>>>::type,
                archetype<int>>::value,
  "");
static_assert(std::is_same<
                archetype_at<1, archetype_list<archetype<int>, archetype<float>>>::type,
                archetype<float>>::value,
  "");
static_assert(std::is_same<
                archetype_at<1, archetype_list<archetype<int>, archetype<float>, archetype<char>>>::type,
                archetype<float>>::value,
  "");
static_assert(std::is_same<
                archetype_at<2, archetype_list<archetype<int>, archetype<float>, archetype<char>>>::type,
                archetype<char>>::value,
  "");
} // namespace ecs