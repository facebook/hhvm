/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <folly/Range.h>
#include <folly/init/Init.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/RendezvousHashFunc.h"

constexpr folly::StringPiece kKey =
    "someKey_ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+_sdkfjsdfksjdfasdfaksxxx";

using facebook::memcache::RendezvousHashFunc;

namespace {

std::vector<folly::StringPiece> makeEndpoints(size_t n) {
  static std::vector<std::string> storage;
  storage.clear();
  storage.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    storage.push_back("endpoint" + std::to_string(i) + ".example.com:1234");
  }
  std::vector<folly::StringPiece> endpoints;
  endpoints.reserve(n);
  for (const auto& s : storage) {
    endpoints.emplace_back(s);
  }
  return endpoints;
}

} // namespace

void rendezvousHashBench(size_t iters, size_t poolSize) {
  RendezvousHashFunc func(
      std::vector<folly::StringPiece>(), folly::dynamic::object);
  BENCHMARK_SUSPEND {
    auto endpoints = makeEndpoints(poolSize);
    func = RendezvousHashFunc(endpoints, folly::dynamic::object);
  }
  for (size_t i = 0; i < iters; ++i) {
    folly::doNotOptimizeAway(func(kKey));
  }
}

BENCHMARK_NAMED_PARAM(rendezvousHashBench, pool_10, 10)
BENCHMARK_NAMED_PARAM(rendezvousHashBench, pool_100, 100)
BENCHMARK_NAMED_PARAM(rendezvousHashBench, pool_500, 500)
BENCHMARK_NAMED_PARAM(rendezvousHashBench, pool_1000, 1000)

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv, true);

  folly::runBenchmarks();
  return 0;
}
