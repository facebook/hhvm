/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <folly/Random.h>
#include <folly/Range.h>
#include <folly/init/Init.h>

#include "mcrouter/lib/WeightedCh3HashFunc.h"
#include "mcrouter/lib/WeightedCh4HashFunc.h"

constexpr folly::StringPiece kKey =
    "someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+_sdkfjsdfksjdfasdfaksxxx";

using facebook::memcache::WeightedCh3HashFunc;
using facebook::memcache::WeightedCh4HashFunc;

void weightedCh3Bench(size_t iters, size_t size, double weight, size_t keyLen) {
  std::vector<double> weights;
  BENCHMARK_SUSPEND {
    weights.resize(size, weight);
  }
  folly::StringPiece key =
      kKey.subpiece(0, keyLen ? keyLen : folly::StringPiece::npos);
  WeightedCh3HashFunc func(std::move(weights));
  for (size_t i = 0; i < iters; ++i) {
    func(key);
  }
}

void weightedCh4Bench(size_t iters, size_t size, double weight, size_t keyLen) {
  std::vector<double> weights;
  BENCHMARK_SUSPEND {
    weights.resize(size, weight);
  }
  folly::StringPiece key =
      kKey.subpiece(0, keyLen ? keyLen : folly::StringPiece::npos);
  WeightedCh4HashFunc func(std::move(weights));
  for (size_t i = 0; i < iters; ++i) {
    func(key);
  }
}

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_100, 100, 1.0, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_100, 100, 1.0, 0)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_1000, 1000, 1.0, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_1000, 1000, 1.0, 0)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_10000, 10000, 1.0, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_10000, 10000, 1.0, 0)

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_100_05, 100, 0.5, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_100_05, 100, 0.5, 0)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_1000_05, 1000, 0.5, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_1000_05, 1000, 0.5, 0)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_10000_05, 10000, 0.5, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_10000_05, 10000, 0.5, 0)

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_100_02, 100, 0.2, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_100_02, 100, 0.2, 0)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_1000_02, 1000, 0.2, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_1000_02, 1000, 0.2, 0)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_10000_02, 10000, 0.2, 0)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_10000_02, 10000, 0.2, 0)

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_100_short, 100, 1.0, 12)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_100_short, 100, 1.0, 12)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_1000_short, 1000, 1.0, 12)
BENCHMARK_RELATIVE_NAMED_PARAM(weightedCh4Bench, size_1000_short, 1000, 1.0, 12)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_10000_short, 10000, 1.0, 12)
BENCHMARK_RELATIVE_NAMED_PARAM(
    weightedCh4Bench,
    size_10000_short,
    10000,
    1.0,
    12)

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_100_short_05, 100, 0.5, 12)
BENCHMARK_RELATIVE_NAMED_PARAM(
    weightedCh4Bench,
    size_100_short_05,
    100,
    0.5,
    12)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_1000_short_05, 1000, 0.5, 12)
BENCHMARK_RELATIVE_NAMED_PARAM(
    weightedCh4Bench,
    size_1000_short_05,
    1000,
    0.5,
    12)

BENCHMARK_NAMED_PARAM(weightedCh3Bench, size_10000_short_05, 10000, 0.5, 12)
BENCHMARK_RELATIVE_NAMED_PARAM(
    weightedCh4Bench,
    size_10000_short_05,
    10000,
    0.5,
    12)

BENCHMARK_DRAW_LINE();

int main(int argc, char** argv) {
  folly::init(&argc, &argv, true);

  folly::runBenchmarks();
  return 0;
}
