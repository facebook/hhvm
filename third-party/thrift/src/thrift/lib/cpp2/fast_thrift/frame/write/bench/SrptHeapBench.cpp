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
 * SrptHeap Benchmarks
 *
 * Measures the core operations of the 4-ary SRPT min-heap:
 *   - insert: add new stream
 *   - peekMin + extractMin: SRPT flush cycle
 *   - update (decrease key): partial flush
 *   - erase (cancel stream): mid-heap removal
 *   - steady-state: interleaved insert/flush/complete cycle
 */

#include <thrift/lib/cpp2/fast_thrift/frame/write/SrptHeap.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

#include <cstdint>
#include <random>

namespace apache::thrift::fast_thrift::frame::write {
namespace {

struct BenchEntry {
  size_t remaining{0};
};

struct BenchKeyFn {
  size_t operator()(const BenchEntry& e) const noexcept { return e.remaining; }
};

using BenchHeap = SrptHeap<BenchEntry, BenchKeyFn>;

// ============================================================================
// Insert
// ============================================================================

void BM_Insert(uint32_t iters, size_t heapSize) {
  // Pre-populate heap to target size, then benchmark inserting one more.
  BenchHeap heap;
  for (uint32_t i = 1; i <= static_cast<uint32_t>(heapSize); ++i) {
    heap.insert(i, BenchEntry{i * 100});
  }

  uint32_t nextId = static_cast<uint32_t>(heapSize) + 1;
  for (uint32_t i = 0; i < iters; ++i) {
    heap.insert(nextId, BenchEntry{50});
    heap.erase(nextId);
    ++nextId;
  }
}

BENCHMARK_PARAM(BM_Insert, 1)
BENCHMARK_PARAM(BM_Insert, 10)
BENCHMARK_PARAM(BM_Insert, 100)
BENCHMARK_PARAM(BM_Insert, 1000)

BENCHMARK_DRAW_LINE();

// ============================================================================
// ExtractMin (SRPT flush)
// ============================================================================

void BM_ExtractMin(uint32_t iters, size_t heapSize) {
  for (uint32_t i = 0; i < iters; ++i) {
    folly::BenchmarkSuspender suspender;
    BenchHeap heap;
    for (uint32_t j = 1; j <= static_cast<uint32_t>(heapSize); ++j) {
      heap.insert(j, BenchEntry{j * 100});
    }
    suspender.dismiss();

    heap.extractMin();
  }
}

BENCHMARK_PARAM(BM_ExtractMin, 1)
BENCHMARK_PARAM(BM_ExtractMin, 10)
BENCHMARK_PARAM(BM_ExtractMin, 100)
BENCHMARK_PARAM(BM_ExtractMin, 1000)

BENCHMARK_DRAW_LINE();

// ============================================================================
// Update (decrease key — partial flush simulation)
// ============================================================================

void BM_UpdateDecreaseKey(uint32_t iters, size_t heapSize) {
  BenchHeap heap;
  // Insert with ascending priorities so the last entry has the highest key.
  for (uint32_t j = 1; j <= static_cast<uint32_t>(heapSize); ++j) {
    heap.insert(j, BenchEntry{j * 1000});
  }

  // Repeatedly decrease the max entry's key, then restore it.
  uint32_t targetId = static_cast<uint32_t>(heapSize);
  for (uint32_t i = 0; i < iters; ++i) {
    auto* ptr = heap.find(targetId);
    size_t orig = ptr->remaining;
    ptr->remaining = 1; // decrease to min
    heap.update(targetId);
    // Restore
    ptr = heap.find(targetId);
    ptr->remaining = orig;
    heap.update(targetId);
  }
}

BENCHMARK_PARAM(BM_UpdateDecreaseKey, 10)
BENCHMARK_PARAM(BM_UpdateDecreaseKey, 100)
BENCHMARK_PARAM(BM_UpdateDecreaseKey, 1000)

BENCHMARK_DRAW_LINE();

// ============================================================================
// Erase (cancel mid-heap)
// ============================================================================

void BM_EraseMidHeap(uint32_t iters, size_t heapSize) {
  for (uint32_t i = 0; i < iters; ++i) {
    folly::BenchmarkSuspender suspender;
    BenchHeap heap;
    for (uint32_t j = 1; j <= static_cast<uint32_t>(heapSize); ++j) {
      heap.insert(j, BenchEntry{j * 100});
    }
    suspender.dismiss();

    // Erase the middle element
    heap.erase(static_cast<uint32_t>(heapSize) / 2);
  }
}

BENCHMARK_PARAM(BM_EraseMidHeap, 10)
BENCHMARK_PARAM(BM_EraseMidHeap, 100)
BENCHMARK_PARAM(BM_EraseMidHeap, 1000)

BENCHMARK_DRAW_LINE();

// ============================================================================
// Steady-state: SRPT flush cycle
// Simulates realistic usage: insert N streams, SRPT flush all to completion.
// ============================================================================

void BM_SrptFlushCycle(uint32_t iters, size_t numStreams) {
  constexpr size_t kFragmentSize = 16384; // 16KB fragments

  std::mt19937 rng(42);
  std::uniform_int_distribution<size_t> sizeDist(1000, 500000);

  for (uint32_t i = 0; i < iters; ++i) {
    BenchHeap heap;

    // Insert N streams with random sizes
    for (uint32_t j = 1; j <= static_cast<uint32_t>(numStreams); ++j) {
      heap.insert(j, BenchEntry{sizeDist(rng)});
    }

    // SRPT flush all to completion
    while (!heap.empty()) {
      uint32_t id = heap.peekMinStreamId();
      auto& min = heap.peekMin();
      if (min.remaining <= kFragmentSize) {
        heap.extractMin();
      } else {
        min.remaining -= kFragmentSize;
        heap.update(id);
      }
    }
  }
}

BENCHMARK_PARAM(BM_SrptFlushCycle, 1)
BENCHMARK_PARAM(BM_SrptFlushCycle, 5)
BENCHMARK_PARAM(BM_SrptFlushCycle, 10)
BENCHMARK_PARAM(BM_SrptFlushCycle, 50)
BENCHMARK_PARAM(BM_SrptFlushCycle, 100)

BENCHMARK_DRAW_LINE();

// ============================================================================
// Comparison: round-robin vs SRPT mean completion time
// Not a perf benchmark — validates that SRPT produces lower avg completion.
// ============================================================================

void BM_SrptMeanCompletion(uint32_t iters, size_t numStreams) {
  constexpr size_t kFragmentSize = 16384;

  std::mt19937 rng(42);
  std::uniform_int_distribution<size_t> sizeDist(1000, 500000);

  for (uint32_t i = 0; i < iters; ++i) {
    BenchHeap heap;
    for (uint32_t j = 1; j <= static_cast<uint32_t>(numStreams); ++j) {
      heap.insert(j, BenchEntry{sizeDist(rng)});
    }

    size_t clock = 0;
    size_t totalCompletion = 0;
    size_t completed = 0;

    while (!heap.empty()) {
      uint32_t id = heap.peekMinStreamId();
      auto& min = heap.peekMin();
      ++clock;
      if (min.remaining <= kFragmentSize) {
        heap.extractMin();
        totalCompletion += clock;
        ++completed;
      } else {
        min.remaining -= kFragmentSize;
        heap.update(id);
      }
    }

    folly::doNotOptimizeAway(totalCompletion);
    folly::doNotOptimizeAway(completed);
  }
}

BENCHMARK_PARAM(BM_SrptMeanCompletion, 10)
BENCHMARK_PARAM(BM_SrptMeanCompletion, 100)

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
