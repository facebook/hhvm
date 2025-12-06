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

BENCHMARK(DefaultPerf) {
  RequestPileTestState state;

  folly::BenchmarkSuspender suspender;
  suspender.dismiss();

  RoundRobinRequestPile::Options opts(
      {1}, RequestPileTestUtils::makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  auto numThreads = folly::available_concurrency();
  unsigned numRoundEachWorker = 10'000;

  folly::CPUThreadPoolExecutor producer(numThreads);
  folly::CPUThreadPoolExecutor consumer(numThreads);

  folly::relaxed_atomic<unsigned> counter{0};

  auto producerFunc = [&]() {
    for (unsigned i = 0; i < numRoundEachWorker; ++i) {
      pile.enqueue(state.makeServerRequestForBucket(0, 0));
    }
  };

  auto consumerFunc = [&]() {
    while (counter.load() != numThreads * numRoundEachWorker) {
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

  suspender.rehire();

  for (unsigned i = 0; i < numThreads; ++i) {
    producer.add(producerFunc);
  }

  producer.join();
  consumer.join();
}

BENCHMARK(RoundRobinBehavior) {
  RequestPileTestState state;

  folly::BenchmarkSuspender suspender;
  suspender.dismiss();

  unsigned numBuckets = 100;
  unsigned numRoundsPerWorker = 100;
  auto numThreads = folly::available_concurrency();

  // single bucket, unlimited request pile, with control on
  RoundRobinRequestPile::Options opts(
      {numBuckets}, RequestPileTestUtils::makePileSelectionFunction());
  RoundRobinRequestPile pile(opts);

  folly::CPUThreadPoolExecutor producer(numThreads);
  folly::CPUThreadPoolExecutor consumer(numThreads);

  folly::relaxed_atomic<unsigned> counter{0};

  auto producerFunc = [&]() {
    for (unsigned i = 0; i < numRoundsPerWorker; ++i) {
      for (unsigned j = 0; j < numBuckets; ++j) {
        pile.enqueue(state.makeServerRequestForBucket(0, j));
      }
    }
  };

  auto sum = numThreads * numRoundsPerWorker * numBuckets;

  auto consumerFunc = [&]() {
    while (counter.load() != sum) {
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

  suspender.rehire();

  for (unsigned i = 0; i < numThreads; ++i) {
    producer.add(producerFunc);
  }

  producer.join();
  consumer.join();
}

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
