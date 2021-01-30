#ifndef _BENCHMARK_HPP_
#define _BENCHMARK_HPP_

#include <chrono>
#include <iostream>
#include <vector>

namespace benchmark
{
using clock_t = std::chrono::high_resolution_clock;
using time_point_t = clock_t::time_point;
using rep_t = double;
using elapsed_t = std::chrono::duration<rep_t, std::micro>;
using average_t = std::chrono::duration<rep_t, std::nano>;

// do_not_optimize works very well for gcc and clang
// msvc works aswell, however results are not always accurate

#if __GNUC__ || __clang__
template<typename T>
inline void do_not_optimize(T const& value)
{
  asm volatile(""
               :
               : "r,m"(value)
               : "memory");
}
#elif _MSC_VER
#include <intrin.h> // For _ReadWriteBarrier
template<typename T>
inline void do_not_optimize(T const& value)
{
  char const volatile* ptr = &reinterpret_cast<char const volatile&>(value);
  _ReadWriteBarrier();
  (void)(ptr);
}
#else
template<typename T>
inline void do_not_optimize(T const&)
{}
#endif

} // namespace benchmark

#define BEGIN_BENCHMARK(__NAME)                        \
  std::cout << "[=========] " << std::endl;            \
  std::cout << "[ RUN     ] " << #__NAME << std::endl; \
  benchmark::time_point_t __benchmark_start = benchmark::clock_t::now();

#define END_BENCHMARK(__ITERATIONS, __OPERATIONS_PER_ITERATION)                                                                             \
  benchmark::time_point_t __benchmark_end = benchmark::clock_t::now();                                                                      \
  benchmark::elapsed_t __benchmark_elapsed = __benchmark_end - __benchmark_start;                                                           \
  benchmark::average_t __benchmark_average = __benchmark_end - __benchmark_start;                                                           \
  auto __benchmark_iterations = __ITERATIONS * __OPERATIONS_PER_ITERATION;                                                                  \
  std::cout << "[ ELAPSED ] " << (__benchmark_elapsed.count() * 0.001) << " ms (" << __benchmark_iterations << " iterations)" << std::endl; \
  std::cout << "[ AVERAGE ] " << (__benchmark_average.count() / __benchmark_iterations) << " ns" << std::endl;

#endif