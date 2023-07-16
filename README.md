### **about**

`stlext` is a single-header set of utilities to extend the `C++` Standard Library (using only `C++11`) to provide `C++17`-like features. It is primarily intended to be used with compilers that don't fully support parallel algorithms, like `std::sort` and `std::for_each`, but there are some other utilities that may be useful, like hashing pairs or arrays of integers (perhaps to hash edges/faces in a mesh, or when storing a sparse matrix using triplets).

To use the extensions, simply include `stlext.h` in your project. The `CMake` configuration is only provided to build and run a few tests. If you have an `OpenMP`-supported compiler, define `HAVE_OMP=1` if you'd like to use `OpenMP` instead of the `C++` `<thread>` library.

#### **requirements**

- A compiler which supports the `C++11` standard.
- (optional) `CMake` to build the tests.

To run the tests, please configure the project with a `C++` compiler of your choice by specifying `-DCMAKE_CXX_COMPILER=[compiler]`. The default is `g++-13` because that's what I have on my system. All the tests can be run with `make testall`.

### **why?**

I personally don't find compiler support for `C++17` parallel algorithms (`__cpp_lib_parallel_algorithm`) to be very robust yet. GNU supports `__cpp_lib_parallel_algorithm` (using Intel's `TBB` as well as an `OpenMP`-supported compiler). `Clang` also supports it if you use the `g++` `stdlib` (via `-stdlib=libstdc++`), but I haven't been able to get this to work with `Clang` on Mac (maybe this works now with a newer version of XCode).

Also, I generally like to know the thread index when running a `for`-loop in parallel (similar to `omp_get_thread_num()` with `OpenMP`), usually if the `for`-loop requires a workspace buffer when performing the parallel computation. The functions `std::parafor` and `std::parafor_i` allow parallel execution of a `for`-loop with information about which element is being processed (by index) as well as which thread is invoking the callback.

### **main features:**

- `std::parasort`: sorts a range of values; call just as you would call `std::sort` (but don't specify an execution policy).
- `std::parafor`: run a `for`-loop in parallel; similar signature to `std::for_each` (without specifying execution policy), however, the `UnaryFunction` object should accept two arguments: `(tid, i)` where `tid` will be the thread id and `i` is the distance from the first iterator in the range to the current element being processed.
- `std::parafor_i`: run a `for`-loop in parallel: similar to `std::parafor` except it accepts the integer range of the `for`-loop instead of iterators.
- hash functions for `std::pair<T1, T2>`, `std::array<T, 2>` and `std::array<T, 3>`.

#### **`std::parasort` benchmark**

Here is a comparison of `std::parasort` with the `C++17` Parallel STL (using `g++-13`) on an Apple M1 Pro (2021) using 10 threads. The number of values (each a `float`) being sorted is `n`, and the sorting is performed 50 times and then averaged to produce the table below. All cell values are in seconds and the values in parentheses represent the speedup over sorting sequentially.

| n      | std::sort (serial) | std::sort (parallel) | std::parasort |
| ------ | ------------------ | -------------------- | ------------- |
| $10^5$ | 0.005              | 0.000 (35.86)        | 0.005 (1.00)  |
| $10^6$ | 0.067              | 0.009 (7.35)         | 0.023 (2.92)  |
| $10^7$ | 0.775              | 0.110 (7.03)         | 0.210 (3.69)  |
| $10^8$ | 8.684              | 1.287 (6.75)         | 2.162 (4.02)  |

As you can see, `std::parasort` is not as fast as the `C++17` Parallel STL, however, it's at least faster (in general) than sorting sequentially. It divides and sorts (in parallel) chunks of the input array and then merges the result (sequentially). I'm sure a different algorithm like parallel radix sort would be faster.

### **examples**

#### **`std::parasort`**

```c++
size_t n = 1e8;
std::vector<float> values(n);
for (size_t i = 0; i < n; i++)
  values[i] = double(rand()) / double(RAND_MAX);
std::parasort(values.begin(), values.end());
```

or with a custom comparator:

```c++
std::parasort(values.begin(), values.end(), [](float a, float b) { return a > b; });
```

### **`std::parafor` and `std::parafor_i`**

```c++
size_t n = 1e9;
std::vector<size_t> x(n, 0);
auto fn = [&x](int tid, size_t i) {
  x[i] = i * i;
};

std::parafor(x.begin(), x.end(), fn);
```

Alternatively, the last line could be:

```c++
std::parafor_i(0, n, fn);
```

### **license**

Copyright 2023 Philip Claude Caplan

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
