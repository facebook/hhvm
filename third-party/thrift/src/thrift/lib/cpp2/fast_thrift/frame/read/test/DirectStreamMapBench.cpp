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

/**
 * DirectStreamMap Microbenchmarks
 *
 * Standalone benchmarks for the direct-mapped stream ID table.
 * Measures insert, find, erase, and interleaved patterns that
 * mirror real FrameDefragmentationHandler usage.
 */

#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

#include <memory>

namespace apache::thrift::fast_thrift::frame::read {
namespace {

// ~40 byte value type to mimic FragmentState
struct FakeState {
  uint32_t type{0};
  uint16_t flags{0};
  uint32_t streamId{0};
  uint16_t metadataSize{0};
  size_t accumulatedBytes{0};
  std::unique_ptr<char[]> payload;

  FakeState() = default;
  explicit FakeState(uint32_t id)
      : streamId(id), payload(std::make_unique<char[]>(64)) {}
  FakeState(FakeState&&) = default;
  FakeState& operator=(FakeState&&) = default;
};

// ============================================================================
// Single-stream: insert + find + erase (mimics 2-fragment reassembly)
// ============================================================================

BENCHMARK(SingleStream_InsertFindErase, iters) {
  DirectStreamMap<FakeState> map;
  for (size_t i = 0; i < iters; ++i) {
    uint32_t id = static_cast<uint32_t>(i * 2 + 1);
    map.emplace(id, FakeState(id));
    auto it = map.find(id);
    folly::doNotOptimizeAway(it->second.streamId);
    map.erase(it);
  }
}

// ============================================================================
// Find in empty map (passthrough fast path)
// ============================================================================

BENCHMARK(FindMiss_EmptyMap, iters) {
  DirectStreamMap<FakeState> map;
  for (size_t i = 0; i < iters; ++i) {
    uint32_t id = static_cast<uint32_t>(i * 2 + 1);
    auto it = map.find(id);
    folly::doNotOptimizeAway(it);
  }
}

// ============================================================================
// Interleaved: 10 concurrent streams
// ============================================================================

BENCHMARK(Interleaved_10Streams, iters) {
  folly::BenchmarkSuspender susp;
  constexpr int kStreams = 10;
  DirectStreamMap<FakeState> map;
  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t base = static_cast<uint32_t>((i * kStreams) % 100000) * 2 + 1;
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      map.emplace(id, FakeState(id));
    }
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      auto it = map.find(id);
      folly::doNotOptimizeAway(it->second.streamId);
      map.erase(it);
    }
  }
}

// ============================================================================
// Interleaved: 100 concurrent streams
// ============================================================================

BENCHMARK(Interleaved_100Streams, iters) {
  folly::BenchmarkSuspender susp;
  constexpr int kStreams = 100;
  DirectStreamMap<FakeState> map;
  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t base = static_cast<uint32_t>((i * kStreams) % 100000) * 2 + 1;
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      map.emplace(id, FakeState(id));
    }
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      auto it = map.find(id);
      folly::doNotOptimizeAway(it->second.streamId);
      map.erase(it);
    }
  }
}

// ============================================================================
// Interleaved: 1000 concurrent streams
// ============================================================================

BENCHMARK(Interleaved_1000Streams, iters) {
  folly::BenchmarkSuspender susp;
  constexpr int kStreams = 1000;
  DirectStreamMap<FakeState> map;
  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t base = static_cast<uint32_t>((i * kStreams) % 1000000) * 2 + 1;
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      map.emplace(id, FakeState(id));
    }
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      auto it = map.find(id);
      folly::doNotOptimizeAway(it->second.streamId);
      map.erase(it);
    }
  }
}

// ============================================================================
// Steady state: map stays at ~10 entries, continuous insert/erase
// ============================================================================

BENCHMARK(SteadyState_10Pending, iters) {
  folly::BenchmarkSuspender susp;
  DirectStreamMap<FakeState> map;

  // Seed with 10 entries
  for (uint32_t s = 0; s < 10; ++s) {
    map.emplace(s * 2 + 1, FakeState(s * 2 + 1));
  }
  uint32_t nextId = 21;
  uint32_t eraseId = 1;
  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    map.emplace(nextId, FakeState(nextId));
    auto it = map.find(eraseId);
    folly::doNotOptimizeAway(it->second.streamId);
    map.erase(it);
    nextId += 2;
    eraseId += 2;
  }
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::read

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
