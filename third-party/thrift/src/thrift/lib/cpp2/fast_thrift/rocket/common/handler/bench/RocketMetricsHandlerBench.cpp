/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/handler/RocketMetricsHandler.h>

using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::channel_pipeline;

struct BenchStats {
  struct BenchCounter {
    void incrementValue(int64_t delta) noexcept { value_ += delta; }
    int64_t value() const noexcept { return value_; }
    int64_t value_{0};
  };

  BenchCounter rocketInbound;
  BenchCounter rocketOutbound;
  BenchCounter rocketErrors;
  BenchCounter rocketActive;
  BenchCounter thriftInbound;
  BenchCounter thriftOutbound;
  BenchCounter thriftErrors;
  BenchCounter thriftActive;
};

struct BenchContext {
  Result fireRead(TypeErasedBox&&) noexcept { return Result::Success; }
  Result fireWrite(TypeErasedBox&&) noexcept { return Result::Success; }
  void fireException(folly::exception_wrapper&&) noexcept {}
};

BENCHMARK(Baseline_DirectFireRead, n) {
  BenchContext ctx;
  for (unsigned i = 0; i < n; ++i) {
    TypeErasedBox box(uint64_t{42});
    folly::doNotOptimizeAway(ctx.fireRead(std::move(box)));
  }
}

BENCHMARK_RELATIVE(RocketMetricsHandler_OnRead, n) {
  auto stats = std::make_shared<BenchStats>();
  RocketMetricsHandler<Direction::Server, BenchStats> handler(std::move(stats));
  BenchContext ctx;
  for (unsigned i = 0; i < n; ++i) {
    TypeErasedBox box(uint64_t{42});
    folly::doNotOptimizeAway(handler.onRead(ctx, std::move(box)));
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Baseline_DirectFireWrite, n) {
  BenchContext ctx;
  for (unsigned i = 0; i < n; ++i) {
    TypeErasedBox box(uint64_t{42});
    folly::doNotOptimizeAway(ctx.fireWrite(std::move(box)));
  }
}

BENCHMARK_RELATIVE(RocketMetricsHandler_OnWrite, n) {
  auto stats = std::make_shared<BenchStats>();
  RocketMetricsHandler<Direction::Server, BenchStats> handler(std::move(stats));
  BenchContext ctx;
  for (unsigned i = 0; i < n; ++i) {
    TypeErasedBox box(uint64_t{42});
    folly::doNotOptimizeAway(handler.onWrite(ctx, std::move(box)));
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(RocketMetricsHandler_FullCycle, n) {
  auto stats = std::make_shared<BenchStats>();
  RocketMetricsHandler<Direction::Server, BenchStats> handler(std::move(stats));
  BenchContext ctx;
  for (unsigned i = 0; i < n; ++i) {
    TypeErasedBox readBox(uint64_t{42});
    folly::doNotOptimizeAway(handler.onRead(ctx, std::move(readBox)));
    TypeErasedBox writeBox(uint64_t{42});
    folly::doNotOptimizeAway(handler.onWrite(ctx, std::move(writeBox)));
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
