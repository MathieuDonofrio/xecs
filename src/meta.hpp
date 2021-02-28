#ifndef XECS_META_HPP
#define XECS_META_HPP

#include <cstdlib>
#include <cstring>
#include <limits>
#include <string_view>
#include <type_traits>

namespace xecs
{
namespace internal
{
  template<typename Type>
  constexpr std::string_view function_name()
  {
#ifdef __clang__
    return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
    return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
    return __FUNCSIG__;
#else
#error "Compiler not supported!"
#endif
  }

  constexpr auto prober = function_name<void>();
  constexpr auto start = prober.find("void");
  constexpr auto offset = prober.length() - 4;

  template<typename Type>
  constexpr std::string_view type_name()
  {
    constexpr auto raw = function_name<Type>();

    return raw.substr(start, raw.length() - offset);
  }

} // namespace internal

namespace internal
{
  static constexpr uint64_t basis = 14695981039346656037ull;
  static constexpr uint64_t prime = 1099511628211ull;

  // FNV v1a

  constexpr uint64_t hash(uint64_t n, const char* string, uint64_t h = basis)
  {
    return n > 0 ? hash(n - 1, string + 1, (h ^ *string) * prime) : h;
  }

  constexpr size_t hash(const std::string_view view)
  {
    return hash(view.length() - 1, view.data());
  }

} // namespace internal

template<typename Type>
struct type_id
{
  static constexpr std::string_view name() { return internal::type_name<Type>(); }

  static constexpr size_t index() { return internal::hash(name()); }
};

namespace internal
{
  template<typename T1, typename... Types>
  struct min
  {
    using type = T1;
  };

  template<typename T1, typename T2, typename... Types>
  struct min<T1, T2, Types...>
  {
  private:
    static constexpr bool smaller = type_id<T2>::index() < type_id<T1>::index();

  public:
    using type = typename std::conditional_t<smaller,
      typename min<T2, Types...>::type,
      typename min<T1, Types...>::type>;
  };

  template<typename T1, typename... Types>
  using min_t = typename min<T1, Types...>::type;

  template<typename Type, typename List>
  struct remove;

  template<typename Type, typename... Types, template<typename...> class List>
  struct remove<Type, List<Types...>>
  {
    using type = List<>;
  };

  template<typename T1, typename... Types, template<typename...> class List>
  struct remove<T1, List<T1, Types...>>
  {
    using type = List<Types...>;
  };

  template<typename Type, typename T1, typename... Types, template<typename...> class List>
  struct remove<Type, List<T1, Types...>>
  {
    using type = typename remove<Type, List<Types..., T1>>::type;
  };

  template<typename Type, typename List>
  using remove_t = typename remove<Type, List>::type;

  template<typename Sorted, typename Unsorted>
  struct sort_into;

  template<typename... SortedTypes, template<typename...> class Sorted, typename... Types, template<typename...> class Unsorted>
  struct sort_into<Sorted<SortedTypes...>, Unsorted<Types...>>
  {
    using type = Sorted<>;
  };

  template<typename... SortedTypes, template<typename...> class Sorted, typename T1, typename... Types, template<typename...> class Unsorted>
  struct sort_into<Sorted<SortedTypes...>, Unsorted<T1, Types...>>
  {
    using type = Sorted<T1>;
  };

  template<typename... SortedTypes, template<typename...> class Sorted, typename T1, typename T2, typename... Types, template<typename...> class Unsorted>
  struct sort_into<Sorted<SortedTypes...>, Unsorted<T1, T2, Types...>>
  {
  private:
    static constexpr bool swap = type_id<T2>::index() < type_id<T1>::index();

  public:
    using type = std::conditional_t<swap, Sorted<SortedTypes..., T2, T1>, Sorted<SortedTypes..., T1, T2>>;
  };

  // TODO Optimize sort for 3 types

  template<typename... SortedTypes, template<typename...> class Sorted, typename T1, typename T2, typename T3, typename... Types, template<typename...> class Unsorted>
  struct sort_into<Sorted<SortedTypes...>, Unsorted<T1, T2, T3, Types...>>
  {
    using next_smallest = typename min<T1, T2, T3, Types...>::type;
    using next_sorted = Sorted<SortedTypes..., next_smallest>;
    using next_unsorted = remove_t<next_smallest, Unsorted<T1, T2, T3, Types...>>;

  public:
    using type = typename sort_into<next_sorted, next_unsorted>::type;
  };

}; // namespace internal

template<typename... Types>
struct combined
{
  using sorted = typename internal::sort_into<combined<>, combined<Types...>>::type;
};

} // namespace xecs

#endif