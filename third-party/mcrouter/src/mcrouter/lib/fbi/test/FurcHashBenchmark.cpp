/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <stdlib.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

#include "mcrouter/lib/fbi/hash.h"

constexpr const char* kLongKey =
    "someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+_sdkfjsdfksjdfasdfaksxxxx";
constexpr size_t kLongKeyLen = 70;
constexpr size_t kShortKeyLen = 12;

void furcHashBench(size_t iters, uint32_t nPart, size_t keyLen) {
  const char* key = kLongKey;
  for (size_t i = 0; i < iters; ++i) {
    folly::doNotOptimizeAway(furc_hash(key, keyLen, nPart));
  }
}

// Production-realistic shard counts with long keys
BENCHMARK_NAMED_PARAM(furcHashBench, nPart_100, 100, kLongKeyLen)
BENCHMARK_NAMED_PARAM(furcHashBench, nPart_1000, 1000, kLongKeyLen)
BENCHMARK_NAMED_PARAM(furcHashBench, nPart_10000, 10000, kLongKeyLen)

BENCHMARK_DRAW_LINE();

// Production-realistic shard counts with short keys
BENCHMARK_NAMED_PARAM(furcHashBench, nPart_100_short, 100, kShortKeyLen)
BENCHMARK_NAMED_PARAM(furcHashBench, nPart_1000_short, 1000, kShortKeyLen)
BENCHMARK_NAMED_PARAM(furcHashBench, nPart_10000_short, 10000, kShortKeyLen)

BENCHMARK_DRAW_LINE();

// Large m values (beyond 2^23, documented (ab)users)
BENCHMARK_NAMED_PARAM(furcHashBench, nPart_100000000, 100000000, kLongKeyLen)

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
