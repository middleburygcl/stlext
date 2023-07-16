#define FORCE_STLEXT
#include <cassert>
#include <execution>

#include "stlext.h"
#include "test.h"

int main(int argc, char** argv) {
  int n = 1e5;
  if (argc > 1) n = std::atoi(argv[1]);
  std::cout << "testing n = " << n / 1e6 << "M with "
            << std::thread::hardware_concurrency() << " threads" << std::endl;

  std::vector<size_t> x(n, 0);
  auto fn = [&x](int tid, size_t i) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    assert(i < x.size());
    x[i] = i * i;
  };

  auto check_result = [&x, n]() {
    for (size_t i = 0; i < n; i++) assert(x[i] == i * i);
  };

  Timer timer;

  timer.start();
  std::parafor_i(0, n, fn);
  timer.stop();
  std::cout << "t_parafor_i: " << timer.seconds() << std::endl;
  check_result();

  timer.start();
  std::parafor(x.begin(), x.end(), fn);
  timer.stop();
  std::cout << "t_parafor: " << timer.seconds() << std::endl;
  check_result();

#if defined(__cpp_lib_parallel_algorithm)
  // test parallel stl if available
  timer.start();
  std::fill(x.begin(), x.end(), 0);
  std::for_each(std::execution::par, x.begin(), x.end(),
                [&fn](size_t i) { fn(0, i); });
  timer.stop();
  std::cout << "t_foreach: " << timer.seconds() << std::endl;
#endif

  timer.start();
  for (size_t i = 0; i < n; i++) fn(0, i);
  timer.stop();
  std::cout << "t_serial: " << timer.seconds() << std::endl;

  return 0;
}
