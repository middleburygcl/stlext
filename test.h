#pragma once

#include <chrono>

class Timer {
 public:
  void start() { a = std::chrono::high_resolution_clock::now(); }
  void stop() { b = std::chrono::high_resolution_clock::now(); }
  double seconds() const { return milliseconds() / 1000.0; }
  double milliseconds() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> a, b;
};