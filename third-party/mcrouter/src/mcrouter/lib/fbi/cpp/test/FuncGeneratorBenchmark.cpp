/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <functional>

#include <folly/Benchmark.h>
#include <folly/Conv.h>
#include <folly/init/Init.h>

#include "mcrouter/lib/fbi/cpp/FuncGenerator.h"

using namespace facebook::memcache;

namespace {

template <class Iterator>
void run(Iterator begin, Iterator end) {
  size_t n = std::distance(begin, end);
  size_t sum = 0;
  for (auto it = begin; it != end; ++it) {
    sum += (*it)();
  }
  if (sum != n * (n - 1) / 2) {
    throw std::runtime_error(folly::to<std::string>(
        "Bad sum: ", sum, ", expected: ", n * (n + 1) / 2));
  }
  folly::doNotOptimizeAway(sum);
}

void runFuncRange(size_t n) {
  auto v = makeFuncGenerator([](size_t id) { return id; }, n);

  run(v.begin(), v.end());
}

void runVector(size_t n) {
  std::vector<std::function<size_t()>> v;
  v.reserve(n);

  for (size_t i = 0; i < n; ++i) {
    v.emplace_back([i]() { return i; });
  }

  run(v.begin(), v.end());
}

} // anonymous namespace

BENCHMARK(FuncRange_2, n) {
  for (auto i = 0u; i < n; ++i) {
    runFuncRange(2);
  }
}

BENCHMARK_RELATIVE(Vector_2, n) {
  for (auto i = 0u; i < n; ++i) {
    runVector(2);
  }
}

BENCHMARK(FuncRange_5, n) {
  for (auto i = 0u; i < n; ++i) {
    runFuncRange(5);
  }
}

BENCHMARK_RELATIVE(Vector_5, n) {
  for (auto i = 0u; i < n; ++i) {
    runVector(5);
  }
}

BENCHMARK(FuncRange_10, n) {
  for (auto i = 0u; i < n; ++i) {
    runFuncRange(10);
  }
}

BENCHMARK_RELATIVE(Vector_10, n) {
  for (auto i = 0u; i < n; ++i) {
    runVector(10);
  }
}

BENCHMARK(FuncRange_100, n) {
  for (auto i = 0u; i < n; ++i) {
    runFuncRange(100);
  }
}

BENCHMARK_RELATIVE(Vector_100, n) {
  for (auto i = 0u; i < n; ++i) {
    runVector(100);
  }
}

int main(int argc, char** argv) {
  folly::init(&argc, &argv, true /* removeFlags */);
  folly::runBenchmarks();
  return 0;
}

/**
 * ============================================================================
 * FuncGeneratorBenchmark                                    time/iter  iters/s
 * ============================================================================
 * FuncRange_2                                                333.37ps    3.00G
 * Vector_2                                           0.30%   110.81ns    9.02M
 * FuncRange_5                                                333.37ps    3.00G
 * Vector_5                                           0.15%   229.09ns    4.37M
 * FuncRange_10                                               333.37ps    3.00G
 * Vector_10                                          0.08%   417.91ns    2.39M
 * FuncRange_100                                               54.59ns   18.32M
 * Vector_100                                         1.51%     3.61us  276.74K
 * ============================================================================
 */
