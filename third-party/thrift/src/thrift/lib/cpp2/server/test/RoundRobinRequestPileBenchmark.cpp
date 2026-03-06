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

#include <vector>

#include <folly/Benchmark.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/init/Init.h>
#include <folly/system/HardwareConcurrency.h>

#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>
#include <thrift/lib/cpp2/server/test/RequestPileTestUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::test;

namespace {

constexpr unsigned kNumRounds = 10'000;
constexpr unsigned kPreFillCount = 100'000;
constexpr unsigned kRequestCountIters = 100'000;

RoundRobinRequestPile::Options makeOpts(std::vector<uint32_t> shape) {
  return RoundRobinRequestPile::Options(
      std::move(shape), RequestPileTestUtils::makePileSelectionFunction());
}

// Runs a concurrent producer/consumer benchmark.
//
// Setup (including consumer startup) is excluded from timing via the
// suspender. Measurement covers producer dispatch through completion
// of all enqueue+dequeue work.
template <typename ProducerFunc>
void runConcurrentBenchmark(
    folly::BenchmarkSuspender& suspender,
    RoundRobinRequestPile& pile,
    unsigned numThreads,
    unsigned totalExpected,
    const ProducerFunc& producerFunc) {
  folly::CPUThreadPoolExecutor producer(numThreads);
  folly::CPUThreadPoolExecutor consumer(numThreads);
  folly::relaxed_atomic<unsigned> counter{0};

  auto consumerFunc = [&]() {
    while (counter.load() != totalExpected) {
      if (auto req = pile.dequeue()) {
        ++counter;
      } else {
        std::this_thread::yield();
      }
    }
  };

  for (unsigned i = 0; i < numThreads; ++i) {
    consumer.add(consumerFunc);
  }

  suspender.dismiss();

  for (unsigned i = 0; i < numThreads; ++i) {
    producer.add(producerFunc);
  }

  producer.join();
  consumer.join();
}

} // namespace

// ============ Group 1: Single-Bucket Concurrent Enqueue+Dequeue ============

BENCHMARK(SingleBucket_NoLimit) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(0, 0));
    }
  });
}

BENCHMARK_RELATIVE(SingleBucket_WithLimit) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1});
  opts.numMaxRequests = 100'000;
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(0, 0));
    }
  });
}

BENCHMARK_DRAW_LINE();

// =========== Group 2: Multi-Bucket Concurrent Enqueue+Dequeue ==============

BENCHMARK(MultiBucket_100Buckets_NoLimit) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 100;
  constexpr unsigned kRoundsPerBucket = kNumRounds / kBuckets;

  auto opts = makeOpts({kBuckets});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kRoundsPerBucket * kBuckets;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kRoundsPerBucket; ++i) {
      for (unsigned j = 0; j < kBuckets; ++j) {
        pile.enqueue(state.makeServerRequestForBucket(0, j));
      }
    }
  });
}

BENCHMARK_RELATIVE(MultiBucket_100Buckets_WithLimit) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 100;
  constexpr unsigned kRoundsPerBucket = kNumRounds / kBuckets;

  auto opts = makeOpts({kBuckets});
  opts.numMaxRequests = 100'000;
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kRoundsPerBucket * kBuckets;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kRoundsPerBucket; ++i) {
      for (unsigned j = 0; j < kBuckets; ++j) {
        pile.enqueue(state.makeServerRequestForBucket(0, j));
      }
    }
  });
}

BENCHMARK_RELATIVE(MultiBucket_4Buckets_NoLimit) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 4;
  constexpr unsigned kRoundsPerBucket = kNumRounds / kBuckets;

  auto opts = makeOpts({kBuckets});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kRoundsPerBucket * kBuckets;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kRoundsPerBucket; ++i) {
      for (unsigned j = 0; j < kBuckets; ++j) {
        pile.enqueue(state.makeServerRequestForBucket(0, j));
      }
    }
  });
}

BENCHMARK_DRAW_LINE();

// ================== Group 3: Enqueue-Only Throughput =======================

BENCHMARK(EnqueueOnly_SingleBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  folly::CPUThreadPoolExecutor pool(numThreads);

  suspender.dismiss();

  for (unsigned i = 0; i < numThreads; ++i) {
    pool.add([&]() {
      for (unsigned j = 0; j < kNumRounds; ++j) {
        pile.enqueue(state.makeServerRequestForBucket(0, 0));
      }
    });
  }
  pool.join();

  suspender.rehire();
  while (pile.dequeue()) {
  }
}

BENCHMARK_RELATIVE(EnqueueOnly_MultiBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 100;
  constexpr unsigned kRoundsPerBucket = kNumRounds / kBuckets;

  auto opts = makeOpts({kBuckets});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  folly::CPUThreadPoolExecutor pool(numThreads);

  suspender.dismiss();

  for (unsigned i = 0; i < numThreads; ++i) {
    pool.add([&]() {
      for (unsigned j = 0; j < kRoundsPerBucket; ++j) {
        for (unsigned k = 0; k < kBuckets; ++k) {
          pile.enqueue(state.makeServerRequestForBucket(0, k));
        }
      }
    });
  }
  pool.join();

  suspender.rehire();
  while (pile.dequeue()) {
  }
}

BENCHMARK_DRAW_LINE();

// ================== Group 4: Dequeue-Only Throughput =======================

BENCHMARK(DequeueOnly_SingleBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1});
  RoundRobinRequestPile pile(opts);

  for (unsigned i = 0; i < kPreFillCount; ++i) {
    pile.enqueue(state.makeServerRequestForBucket(0, 0));
  }

  suspender.dismiss();
  while (pile.dequeue()) {
  }
}

BENCHMARK_RELATIVE(DequeueOnly_MultiBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 100;
  constexpr unsigned kPerBucket = kPreFillCount / kBuckets;

  auto opts = makeOpts({kBuckets});
  RoundRobinRequestPile pile(opts);

  for (unsigned i = 0; i < kPerBucket; ++i) {
    for (unsigned j = 0; j < kBuckets; ++j) {
      pile.enqueue(state.makeServerRequestForBucket(0, j));
    }
  }

  suspender.dismiss();
  while (pile.dequeue()) {
  }
}

BENCHMARK_DRAW_LINE();

// ==================== Group 5: Multiple Priorities =========================

BENCHMARK(TwoPriorities_SingleBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1, 1});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(i % 2, 0));
    }
  });
}

BENCHMARK_RELATIVE(TwoPriorities_MultiBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 10;

  auto opts = makeOpts({kBuckets, kBuckets});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(i % 2, i % kBuckets));
    }
  });
}

BENCHMARK_RELATIVE(FivePriorities_SingleBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1, 1, 1, 1, 1});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(i % 5, 0));
    }
  });
}

BENCHMARK_DRAW_LINE();

// ======================== Group 6: Mixed Shape =============================

BENCHMARK(MixedShape_1_10) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1, 10});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      unsigned pri = i % 2;
      unsigned bucket = (pri == 0) ? 0 : (i % 10);
      pile.enqueue(state.makeServerRequestForBucket(pri, bucket));
    }
  });
}

BENCHMARK_RELATIVE(MixedShape_1_10_1) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1, 10, 1});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      unsigned pri = i % 3;
      unsigned bucket = (pri == 1) ? (i % 10) : 0;
      pile.enqueue(state.makeServerRequestForBucket(pri, bucket));
    }
  });
}

BENCHMARK_DRAW_LINE();

// ============= Group 7: Enqueue Rejection Under Contention =================

BENCHMARK(Rejection_SingleBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1});
  opts.numMaxRequests = 100;
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  folly::CPUThreadPoolExecutor pool(numThreads);

  suspender.dismiss();

  for (unsigned i = 0; i < numThreads; ++i) {
    pool.add([&]() {
      for (unsigned j = 0; j < kNumRounds; ++j) {
        pile.enqueue(state.makeServerRequestForBucket(0, 0));
      }
    });
  }
  pool.join();

  suspender.rehire();
  while (pile.dequeue()) {
  }
}

BENCHMARK_RELATIVE(Rejection_MultiBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 10;
  constexpr unsigned kRoundsPerBucket = kNumRounds / kBuckets;

  auto opts = makeOpts({kBuckets});
  opts.numMaxRequests = 10;
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  folly::CPUThreadPoolExecutor pool(numThreads);

  suspender.dismiss();

  for (unsigned i = 0; i < numThreads; ++i) {
    pool.add([&]() {
      for (unsigned j = 0; j < kRoundsPerBucket; ++j) {
        for (unsigned k = 0; k < kBuckets; ++k) {
          pile.enqueue(state.makeServerRequestForBucket(0, k));
        }
      }
    });
  }
  pool.join();

  suspender.rehire();
  while (pile.dequeue()) {
  }
}

BENCHMARK_DRAW_LINE();

// =================== Group 8: requestCount() Cost ==========================

BENCHMARK(RequestCount_SingleBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1, 1, 1, 1, 1});
  RoundRobinRequestPile pile(opts);

  for (unsigned pri = 0; pri < 5; ++pri) {
    for (unsigned i = 0; i < 100; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(pri, 0));
    }
  }

  suspender.dismiss();

  for (unsigned i = 0; i < kRequestCountIters; ++i) {
    folly::doNotOptimizeAway(pile.requestCount());
  }

  suspender.rehire();
  while (pile.dequeue()) {
  }
}

BENCHMARK_RELATIVE(RequestCount_MultiBucket) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  constexpr unsigned kBuckets = 10;

  auto opts = makeOpts({kBuckets, kBuckets, kBuckets, kBuckets, kBuckets});
  RoundRobinRequestPile pile(opts);

  for (unsigned pri = 0; pri < 5; ++pri) {
    for (unsigned j = 0; j < kBuckets; ++j) {
      for (unsigned i = 0; i < 10; ++i) {
        pile.enqueue(state.makeServerRequestForBucket(pri, j));
      }
    }
  }

  suspender.dismiss();

  for (unsigned i = 0; i < kRequestCountIters; ++i) {
    folly::doNotOptimizeAway(pile.requestCount());
  }

  suspender.rehire();
  while (pile.dequeue()) {
  }
}

BENCHMARK_DRAW_LINE();

// =============== Group 9: Pre-Enqueue Filter Overhead ======================

BENCHMARK(PreEnqueueFilter_NoFilter) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1});
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(0, 0));
    }
  });
}

BENCHMARK_RELATIVE(PreEnqueueFilter_AlwaysAccept) {
  folly::BenchmarkSuspender suspender;
  RequestPileTestState state;

  auto opts = makeOpts({1});
  opts.setPreEnqueueFilter(
      [](const ServerRequest&) -> std::optional<ServerRequestRejection> {
        return std::nullopt;
      });
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  auto totalExpected = numThreads * kNumRounds;

  runConcurrentBenchmark(suspender, pile, numThreads, totalExpected, [&]() {
    for (unsigned i = 0; i < kNumRounds; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(0, 0));
    }
  });
}

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
