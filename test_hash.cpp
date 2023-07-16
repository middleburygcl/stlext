#include <cassert>
#include <cstdint>
#include <iostream>
#include <unordered_set>

#include "stlext.h"
#include "test.h"

int main(int argc, char** argv) {
  std::unordered_set<std::pair<int, int>> set1;
  set1.insert({1, 2});
  assert(set1.find({1, 2}) != set1.end());
  assert(set1.find({2, 3}) == set1.end());

  using ivec2 = std::array<int, 2>;
  std::unordered_set<ivec2> set2;
  set2.insert({2, 3});
  assert(set2.find({1, 2}) == set2.end());
  assert(set2.find({2, 3}) != set2.end());

  std::cout << "all checks passed!" << std::endl;

  using ivec3 = std::array<uint64_t, 3>;
  std::unordered_set<ivec3> set3;
  set3.insert({2, 4, 6});
  assert(set3.find({2, 4, 6}) != set3.end());
  assert(set3.find({1, 2, 3}) == set3.end());

  return 0;
}