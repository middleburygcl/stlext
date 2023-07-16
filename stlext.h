#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <execution>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

#if HAVE_OMP
#include <omp.h>
#endif

namespace std {

/**
 * allows hashing a pair in order to use as a key in unordered_map
 *
 * found here:
 * https://stackoverflow.com/questions/28367913/how-to-stdhash-an-unordered-stdpair
 */
template <typename T, typename... Rest>
void hash_combine(std::size_t& seed, const T& v, const Rest&... rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}

template <typename T1, typename T2>
struct hash<std::pair<T1, T2>> {
  std::size_t operator()(std::pair<T1, T2> const& p) const {
    std::size_t seed1(0);
    hash_combine(seed1, p.first);
    hash_combine(seed1, p.second);

    std::size_t seed2(0);
    hash_combine(seed2, p.second);
    hash_combine(seed2, p.first);

    return std::min(seed1, seed2);
  }
};

template <typename T>
struct hash<std::array<T, 2>> {
  std::size_t operator()(std::array<T, 2> const& p) const {
    std::size_t seed1(0);
    hash_combine(seed1, p[0]);
    hash_combine(seed1, p[1]);

    std::size_t seed2(0);
    hash_combine(seed2, p[1]);
    hash_combine(seed2, p[0]);

    return std::min(seed1, seed2);
  }
};

template <typename T>
struct hash<std::array<T, 3>> {
  std::size_t operator()(std::array<T, 3> const& p) const {
    std::size_t h = 0;
    hash_combine(h, p[0], p[1], p[2]);
    return h;
  }
};

#if defined(__cpp_lib_parallel_algorithm) && !defined(FORCE_STLEXT)

template <class RandomAccessIterator,
          class Compare = std::less<
              typename std::iterator_traits<RandomAccessIterator>::value_type>>
void parasort(RandomAccessIterator first, RandomAccessIterator last,
              Compare cmp = Compare{}) {
  std::sort(std::execution::par, first, last, cmp);
}

#else

template <class RandomAccessIterator,
          class Compare = std::less<
              typename std::iterator_traits<RandomAccessIterator>::value_type>>
void parasort(RandomAccessIterator first, RandomAccessIterator last,
              Compare cmp = Compare{}) {
  using T = typename std::iterator_traits<RandomAccessIterator>::value_type;
  size_t n = std::distance(first, last);

  size_t n_thread = std::thread::hardware_concurrency();
  if (n < 5 * n_thread) {
    std::sort(first, last, cmp);
    return;
  }

  // create the sorting threads
  std::vector<T> y(first, last);
  const size_t n_arrays = n_thread;
  const size_t m = n / n_thread;
  std::vector<std::function<void(void)>> sort_threads(n_thread);
  std::vector<std::future<void>> sort_futures(n_thread);
  std::vector<size_t> leading(n_thread), ends(n_thread);
  for (size_t i = 0; i < n_thread; i++) {
    const size_t start = i * m;
    size_t end = start + m;
    if (i + 1 == n_thread) end = n;
    leading[i] = start;
    ends[i] = end;
    sort_threads[i] = [&y, start, end, &cmp]() {
      std::sort(y.begin() + start, y.begin() + end, cmp);
    };
    sort_futures[i] = std::async(std::launch::async, sort_threads[i]);
  }

  // wait for the subarrays to be sorted
  for (size_t i = 0; i < n_thread; i++) sort_futures[i].get();

  // merge the subarrays
  const size_t n_half = std::floor((n + 1) / 2);
  for (size_t i = 0; i < n_half; ++i) {
    // find the min and max in the subarrays
    int jmin = -1, jmax = -1;
    T xmin, xmax;
    bool init = false;
#pragma unroll
    for (int j = 0; j < n_arrays; ++j) {
      if (leading[j] >= ends[j]) continue;
      const T& xl = y[leading[j]];
      const T& xe = y[ends[j] - 1];
      if (!init) {
        jmin = j;
        jmax = j;
        xmin = xl;
        xmax = xe;
        init = true;
        continue;
      }
      if (cmp(xl, xmin)) {
        jmin = j;
        xmin = xl;
      }
      if (!cmp(xe, xmax)) {
        jmax = j;
        xmax = xe;
      }
    }
    if (jmin < 0 || jmax < 0)
      std::cout << "i = " << i << ", jmin = " << jmin << ", jmax = " << jmax
                << std::endl;
    assert(jmin >= 0 && jmax >= 0);
    *(first + i) = xmin;
    *(first + n - i - 1) = xmax;
    ++leading[jmin];
    --ends[jmax];
  }
}

#endif

template <class RandomAccessIterator, class Function>
void parafor(RandomAccessIterator first, RandomAccessIterator last,
             Function fn) {
  size_t n = std::distance(first, last);
  size_t n_thread = std::thread::hardware_concurrency();
  if (n < 5 * n_thread) {
    for (; first != last; ++first) fn(0, *first);
    return;
  }

  // create the execution threads
  const size_t m = n / n_thread;
  std::vector<std::function<void(void)>> exec_threads(n_thread);
  std::vector<std::future<void>> exec_futures(n_thread);
  for (size_t i = 0; i < n_thread; i++) {
    auto start = first + i * m;
    auto end = start + m;
    if (i + 1 == n_thread) end = last;
    exec_threads[i] = [start, end, &fn, m, i]() {
      auto it = start;
      size_t j = i * m;
      for (; it != end; ++it) fn(i, j++);
    };
    exec_futures[i] = std::async(std::launch::async, exec_threads[i]);
  }

  // wait for all threads to finish
  for (size_t i = 0; i < n_thread; i++) exec_futures[i].get();
}

template <class Function>
void parafor_i(size_t first, size_t last, Function fn) {
#if HAVE_OMP
#pragma omp parallel for
  for (size_t i = first; i < last; ++i) fn(omp_get_thread_num(), i);
  return;
#endif

  size_t n = last - first;
  size_t n_thread = std::thread::hardware_concurrency();
  if (n < 5 * n_thread) {
    for (auto i = first; i < last; ++i) fn(0, i);
    return;
  }

  // create the execution threads
  const size_t m = n / n_thread;
  std::vector<std::function<void(void)>> exec_threads(n_thread);
  std::vector<std::future<void>> exec_futures(n_thread);
  for (size_t i = 0; i < n_thread; i++) {
    auto start = first + i * m;
    auto end = first + start + m;
    if (i + 1 == n_thread) end = last;
    exec_threads[i] = [i, start, end, &fn]() {
      for (auto j = start; j < end; ++j) fn(i, j);
    };
    exec_futures[i] = std::async(std::launch::async, exec_threads[i]);
  }

  // wait for all threads to finish
  for (size_t i = 0; i < n_thread; i++) exec_futures[i].get();
}

}  // namespace std
