#include <archetype.hpp>

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

static_assert(std::is_same_v<list<int>, find_for_t<list<list<int>>, int>>, "");
static_assert(std::is_same_v<list<int>, find_for_t<list<list<float>, list<int>>, int>>, "");
static_assert(std::is_same_v<list<int>, find_for_t<list<list<int, float>, list<int>>, int>>, "");
static_assert(std::is_same_v<list<int, float>, find_for_t<list<list<int, float>, list<int>>, int, float>>, "");
static_assert(std::is_same_v<list<int, float>, find_for_t<list<list<int, float>, list<int>>, float, int>>, "");

static_assert(std::is_same_v<int, at_t<0, list<int>>>, "");
static_assert(std::is_same_v<float, at_t<0, list<float, int>>>, "");
static_assert(std::is_same_v<int, at_t<1, list<float, int>>>, "");
static_assert(std::is_same_v<double, at_t<0, list<double, float, int>>>, "");
static_assert(std::is_same_v<float, at_t<1, list<double, float, int>>>, "");
static_assert(std::is_same_v<int, at_t<2, list<double, float, int>>>, "");
} // namespace ecs