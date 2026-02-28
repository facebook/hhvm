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

#include <deque>
#include <memory>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

#include <thrift/lib/cpp2/transport/rocket/core/RingBuffer.h>

using namespace apache::thrift::rocket;

// Simple struct to test with non-trivial types
struct TestObject {
  explicit TestObject(int v = 0) : value(v) {}
  TestObject(const TestObject& other) : value(other.value) {}
  TestObject(TestObject&& other) noexcept : value(other.value) {}
  ~TestObject() = default;

  int value;
};

// Benchmark for emplace_back operation
template <typename T>
void benchmarkEmplaceBack(size_t iters, size_t size) {
  folly::BenchmarkSuspender suspender;

  // RingBuffer setup
  RingBuffer<T> ringBuffer(10); // 2^10 = 1024 capacity

  suspender.dismiss();

  // Benchmark RingBuffer emplace_back
  for (size_t i = 0; i < iters; ++i) {
    if (i % size == 0) {
      suspender.rehire();
      // Clear and reset
      while (!ringBuffer.empty()) {
        ringBuffer.pop_front();
      }
      suspender.dismiss();
    }

    if constexpr (std::is_same_v<T, int>) {
      ringBuffer.emplace_back(static_cast<T>(i));
    } else {
      ringBuffer.emplace_back(T(static_cast<int>(i)));
    }
  }
}

template <typename T>
void benchmarkStdDequeEmplaceBack(size_t iters, size_t size) {
  folly::BenchmarkSuspender suspender;

  // std::deque setup
  std::deque<T> stdDeque;

  suspender.dismiss();

  // Benchmark std::deque emplace_back
  for (size_t i = 0; i < iters; ++i) {
    if (i % size == 0) {
      suspender.rehire();
      // Clear and reset
      stdDeque.clear();
      suspender.dismiss();
    }

    if constexpr (std::is_same_v<T, int>) {
      stdDeque.emplace_back(static_cast<T>(i));
    } else {
      stdDeque.emplace_back(T(static_cast<int>(i)));
    }
  }
}

// Benchmark for front + pop_front operations
template <typename T>
void benchmarkFrontAndPop(size_t iters, size_t batchSize) {
  folly::BenchmarkSuspender suspender;

  // RingBuffer setup
  RingBuffer<T> ringBuffer(10); // 2^10 = 1024 capacity

  // Fill the buffer
  for (size_t i = 0; i < batchSize; ++i) {
    ringBuffer.emplace_back(static_cast<T>(i));
  }

  suspender.dismiss();

  // Benchmark RingBuffer front + pop_front
  for (size_t i = 0; i < iters; ++i) {
    if (i % batchSize == 0) {
      suspender.rehire();
      // Refill
      for (size_t j = 0; j < batchSize; ++j) {
        if constexpr (std::is_same_v<T, int>) {
          ringBuffer.emplace_back(static_cast<T>(j));
        } else {
          ringBuffer.emplace_back(T(static_cast<int>(j)));
        }
      }
      suspender.dismiss();
    }

    T value = ringBuffer.front();
    ringBuffer.pop_front();
    folly::doNotOptimizeAway(value);
  }
}

template <typename T>
void benchmarkStdDequeFrontAndPop(size_t iters, size_t batchSize) {
  folly::BenchmarkSuspender suspender;

  // std::deque setup
  std::deque<T> stdDeque;

  // Fill the deque
  for (size_t i = 0; i < batchSize; ++i) {
    if constexpr (std::is_same_v<T, int>) {
      stdDeque.emplace_back(static_cast<T>(i));
    } else {
      stdDeque.emplace_back(T(static_cast<int>(i)));
    }
  }

  suspender.dismiss();

  // Benchmark std::deque front + pop_front
  for (size_t i = 0; i < iters; ++i) {
    if (i % batchSize == 0) {
      suspender.rehire();
      // Refill
      stdDeque.clear();
      for (size_t j = 0; j < batchSize; ++j) {
        if constexpr (std::is_same_v<T, int>) {
          stdDeque.emplace_back(static_cast<T>(j));
        } else {
          stdDeque.emplace_back(T(static_cast<int>(j)));
        }
      }
      suspender.dismiss();
    }

    T value = stdDeque.front();
    stdDeque.pop_front();
    folly::doNotOptimizeAway(value);
  }
}

// Benchmark for batch consumption
template <typename T>
void benchmarkBatchConsume(size_t iters, size_t batchSize) {
  folly::BenchmarkSuspender suspender;

  // RingBuffer setup
  RingBuffer<T> ringBuffer(10); // 2^10 = 1024 capacity

  suspender.dismiss();

  // Benchmark RingBuffer batch consume
  for (size_t i = 0; i < iters; ++i) {
    suspender.rehire();
    // Fill the buffer
    for (size_t j = 0; j < batchSize; ++j) {
      if constexpr (std::is_same_v<T, int>) {
        ringBuffer.emplace_back(static_cast<T>(j));
      } else {
        ringBuffer.emplace_back(T(static_cast<int>(j)));
      }
    }
    suspender.dismiss();

    ringBuffer.consume(
        [&](T& val) { folly::doNotOptimizeAway(val); }, batchSize);
  }
}

template <typename T>
void benchmarkStdDequeBatchConsume(size_t iters, size_t batchSize) {
  folly::BenchmarkSuspender suspender;

  // std::deque setup
  std::deque<T> stdDeque;

  suspender.dismiss();

  // Benchmark std::deque batch consume (manual implementation)
  for (size_t i = 0; i < iters; ++i) {
    suspender.rehire();
    // Fill the deque
    for (size_t j = 0; j < batchSize; ++j) {
      if constexpr (std::is_same_v<T, int>) {
        stdDeque.emplace_back(static_cast<T>(j));
      } else {
        stdDeque.emplace_back(T(static_cast<int>(j)));
      }
    }
    suspender.dismiss();

    size_t count = std::min(batchSize, stdDeque.size());
    for (size_t j = 0; j < count; ++j) {
      folly::doNotOptimizeAway(stdDeque.front());
      stdDeque.pop_front();
    }
  }
}

// Mixed workload benchmark (realistic usage pattern)
template <typename T>
void benchmarkMixedWorkload(size_t iters, size_t batchSize) {
  folly::BenchmarkSuspender suspender;

  // RingBuffer setup
  RingBuffer<T> ringBuffer(10); // 2^10 = 1024 capacity

  suspender.dismiss();

  // Benchmark RingBuffer with mixed operations
  for (size_t i = 0; i < iters; ++i) {
    // Add elements
    for (size_t j = 0; j < batchSize; ++j) {
      if constexpr (std::is_same_v<T, int>) {
        ringBuffer.emplace_back(static_cast<T>(j));
      } else {
        ringBuffer.emplace_back(T(static_cast<int>(j)));
      }
    }

    // Read some elements
    ringBuffer.consume(
        [](T& val) { folly::doNotOptimizeAway(val); }, batchSize / 2);

    // Add more elements
    for (size_t j = 0; j < batchSize / 2; ++j) {
      if constexpr (std::is_same_v<T, int>) {
        ringBuffer.emplace_back(static_cast<T>(j + 1000));
      } else {
        ringBuffer.emplace_back(T(static_cast<int>(j + 1000)));
      }
    }

    // Consume remaining elements
    size_t consumed = ringBuffer.consume(
        [](T& val) { folly::doNotOptimizeAway(val); }, ringBuffer.size());

    folly::doNotOptimizeAway(consumed);
  }
}

template <typename T>
void benchmarkStdDequeMixedWorkload(size_t iters, size_t batchSize) {
  folly::BenchmarkSuspender suspender;

  // std::deque setup
  std::deque<T> stdDeque;

  suspender.dismiss();

  // Benchmark std::deque with mixed operations
  for (size_t i = 0; i < iters; ++i) {
    // Add elements
    for (size_t j = 0; j < batchSize; ++j) {
      if constexpr (std::is_same_v<T, int>) {
        stdDeque.emplace_back(static_cast<T>(j));
      } else {
        stdDeque.emplace_back(T(static_cast<int>(j)));
      }
    }

    // Read some elements
    for (size_t j = 0; j < batchSize / 2; ++j) {
      T value = stdDeque.front();
      stdDeque.pop_front();
      folly::doNotOptimizeAway(value);
    }

    // Add more elements
    for (size_t j = 0; j < batchSize / 2; ++j) {
      if constexpr (std::is_same_v<T, int>) {
        stdDeque.emplace_back(static_cast<T>(j + 1000));
      } else {
        stdDeque.emplace_back(T(static_cast<int>(j + 1000)));
      }
    }

    // Consume remaining elements (manually)
    size_t count = stdDeque.size();
    for (size_t j = 0; j < count; ++j) {
      T value = stdDeque.front();
      stdDeque.pop_front();
      folly::doNotOptimizeAway(value);
    }

    folly::doNotOptimizeAway(count);
  }
}

// Define non-template wrapper functions for benchmarks with external linkage

// EmplaceBack benchmarks
static void benchmarkEmplaceBackInt_10(size_t iters) {
  benchmarkEmplaceBack<int>(iters, 10);
}
void benchmarkStdDequeEmplaceBackInt_10(size_t iters) {
  benchmarkStdDequeEmplaceBack<int>(iters, 10);
}
void benchmarkEmplaceBackInt_100(size_t iters) {
  benchmarkEmplaceBack<int>(iters, 100);
}
void benchmarkStdDequeEmplaceBackInt_100(size_t iters) {
  benchmarkStdDequeEmplaceBack<int>(iters, 100);
}
void benchmarkEmplaceBackInt_1000(size_t iters) {
  benchmarkEmplaceBack<int>(iters, 1000);
}
void benchmarkStdDequeEmplaceBackInt_1000(size_t iters) {
  benchmarkStdDequeEmplaceBack<int>(iters, 1000);
}

// Front and Pop benchmarks
void benchmarkFrontAndPopInt_10(size_t iters) {
  benchmarkFrontAndPop<int>(iters, 10);
}
void benchmarkStdDequeFrontAndPopInt_10(size_t iters) {
  benchmarkStdDequeFrontAndPop<int>(iters, 10);
}
void benchmarkFrontAndPopInt_100(size_t iters) {
  benchmarkFrontAndPop<int>(iters, 100);
}
void benchmarkStdDequeFrontAndPopInt_100(size_t iters) {
  benchmarkStdDequeFrontAndPop<int>(iters, 100);
}
void benchmarkFrontAndPopInt_1000(size_t iters) {
  benchmarkFrontAndPop<int>(iters, 1000);
}
void benchmarkStdDequeFrontAndPopInt_1000(size_t iters) {
  benchmarkStdDequeFrontAndPop<int>(iters, 1000);
}

// Batch Consume benchmarks
void benchmarkBatchConsumeInt_10(size_t iters) {
  benchmarkBatchConsume<int>(iters, 10);
}
void benchmarkStdDequeBatchConsumeInt_10(size_t iters) {
  benchmarkStdDequeBatchConsume<int>(iters, 10);
}
void benchmarkBatchConsumeInt_100(size_t iters) {
  benchmarkBatchConsume<int>(iters, 100);
}
void benchmarkStdDequeBatchConsumeInt_100(size_t iters) {
  benchmarkStdDequeBatchConsume<int>(iters, 100);
}
void benchmarkBatchConsumeInt_1000(size_t iters) {
  benchmarkBatchConsume<int>(iters, 1000);
}
void benchmarkStdDequeBatchConsumeInt_1000(size_t iters) {
  benchmarkStdDequeBatchConsume<int>(iters, 1000);
}

// Mixed Workload benchmarks
void benchmarkMixedWorkloadInt_10(size_t iters) {
  benchmarkMixedWorkload<int>(iters, 10);
}
void benchmarkStdDequeMixedWorkloadInt_10(size_t iters) {
  benchmarkStdDequeMixedWorkload<int>(iters, 10);
}
void benchmarkMixedWorkloadInt_100(size_t iters) {
  benchmarkMixedWorkload<int>(iters, 100);
}
void benchmarkStdDequeMixedWorkloadInt_100(size_t iters) {
  benchmarkStdDequeMixedWorkload<int>(iters, 100);
}
void benchmarkMixedWorkloadInt_1000(size_t iters) {
  benchmarkMixedWorkload<int>(iters, 1000);
}
void benchmarkStdDequeMixedWorkloadInt_1000(size_t iters) {
  benchmarkStdDequeMixedWorkload<int>(iters, 1000);
}

// Non-trivial type benchmarks
void benchmarkEmplaceBackTestObject_100(size_t iters) {
  benchmarkEmplaceBack<TestObject>(iters, 100);
}
void benchmarkStdDequeEmplaceBackTestObject_100(size_t iters) {
  benchmarkStdDequeEmplaceBack<TestObject>(iters, 100);
}
void benchmarkFrontAndPopTestObject_100(size_t iters) {
  benchmarkFrontAndPop<TestObject>(iters, 100);
}
void benchmarkStdDequeFrontAndPopTestObject_100(size_t iters) {
  benchmarkStdDequeFrontAndPop<TestObject>(iters, 100);
}
void benchmarkBatchConsumeTestObject_100(size_t iters) {
  benchmarkBatchConsume<TestObject>(iters, 100);
}
void benchmarkStdDequeBatchConsumeTestObject_100(size_t iters) {
  benchmarkStdDequeBatchConsume<TestObject>(iters, 100);
}
void benchmarkMixedWorkloadTestObject_100(size_t iters) {
  benchmarkMixedWorkload<TestObject>(iters, 100);
}
void benchmarkStdDequeMixedWorkloadTestObject_100(size_t iters) {
  benchmarkStdDequeMixedWorkload<TestObject>(iters, 100);
}

// Register benchmarks
BENCHMARK(benchmarkEmplaceBackInt_10) {
  benchmarkEmplaceBackInt_10(10);
}
BENCHMARK_RELATIVE(benchmarkStdDequeEmplaceBackInt_10) {
  benchmarkStdDequeEmplaceBackInt_10(10);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkEmplaceBackInt_100) {
  benchmarkEmplaceBackInt_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeEmplaceBackInt_100) {
  benchmarkStdDequeEmplaceBackInt_100(100);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkEmplaceBackInt_1000) {
  benchmarkEmplaceBackInt_1000(1000);
}
BENCHMARK_RELATIVE(benchmarkStdDequeEmplaceBackInt_1000) {
  benchmarkStdDequeEmplaceBackInt_1000(1000);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkFrontAndPopInt_10) {
  benchmarkFrontAndPopInt_10(10);
}
BENCHMARK_RELATIVE(benchmarkStdDequeFrontAndPopInt_10) {
  benchmarkStdDequeFrontAndPopInt_10(10);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkFrontAndPopInt_100) {
  benchmarkFrontAndPopInt_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeFrontAndPopInt_100) {
  benchmarkStdDequeFrontAndPopInt_100(100);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkFrontAndPopInt_1000) {
  benchmarkFrontAndPopInt_1000(1000);
}
BENCHMARK_RELATIVE(benchmarkStdDequeFrontAndPopInt_1000) {
  benchmarkStdDequeFrontAndPopInt_1000(1000);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkBatchConsumeInt_10) {
  benchmarkBatchConsumeInt_10(10);
}
BENCHMARK_RELATIVE(benchmarkStdDequeBatchConsumeInt_10) {
  benchmarkStdDequeBatchConsumeInt_10(10);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkBatchConsumeInt_100) {
  benchmarkBatchConsumeInt_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeBatchConsumeInt_100) {
  benchmarkStdDequeBatchConsumeInt_100(100);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkBatchConsumeInt_1000) {
  benchmarkBatchConsumeInt_1000(1000);
}
BENCHMARK_RELATIVE(benchmarkStdDequeBatchConsumeInt_1000) {
  benchmarkStdDequeBatchConsumeInt_1000(1000);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkMixedWorkloadInt_10) {
  benchmarkMixedWorkloadInt_10(10);
}
BENCHMARK_RELATIVE(benchmarkStdDequeMixedWorkloadInt_10) {
  benchmarkStdDequeMixedWorkloadInt_10(10);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkMixedWorkloadInt_100) {
  benchmarkMixedWorkloadInt_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeMixedWorkloadInt_100) {
  benchmarkStdDequeMixedWorkloadInt_100(100);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkMixedWorkloadInt_1000) {
  benchmarkMixedWorkloadInt_1000(1000);
}
BENCHMARK_RELATIVE(benchmarkStdDequeMixedWorkloadInt_1000) {
  benchmarkStdDequeMixedWorkloadInt_1000(1000);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkEmplaceBackTestObject_100) {
  benchmarkEmplaceBackTestObject_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeEmplaceBackTestObject_100) {
  benchmarkStdDequeEmplaceBackTestObject_100(100);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkFrontAndPopTestObject_100) {
  benchmarkFrontAndPopTestObject_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeFrontAndPopTestObject_100) {
  benchmarkStdDequeFrontAndPopTestObject_100(100);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkBatchConsumeTestObject_100) {
  benchmarkBatchConsumeTestObject_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeBatchConsumeTestObject_100) {
  benchmarkStdDequeBatchConsumeTestObject_100(100);
}
BENCHMARK_DRAW_LINE();

BENCHMARK(benchmarkMixedWorkloadTestObject_100) {
  benchmarkMixedWorkloadTestObject_100(100);
}
BENCHMARK_RELATIVE(benchmarkStdDequeMixedWorkloadTestObject_100) {
  benchmarkStdDequeMixedWorkloadTestObject_100(100);
}
BENCHMARK_DRAW_LINE();

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
