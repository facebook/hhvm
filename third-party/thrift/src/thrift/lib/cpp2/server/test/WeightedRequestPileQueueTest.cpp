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

#include <folly/ScopeGuard.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/server/WeightedRequestPileQueue.h>

using namespace apache::thrift::server;

TEST(WeightedRequestPileQueueTest, RightOrdering) {
  // For one dimensional control block
  WeightedRequestPileQueue<int, false> queue1;

  for (int i = 0; i < 10; ++i) {
    queue1.enqueue(std::move(i));
  }

  EXPECT_EQ(queue1.size(), 10);

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(*queue1.tryDequeue(), i);
  }

  auto last = queue1.tryDequeue();
  EXPECT_EQ(last, std::nullopt);

  WeightedRequestPileQueue<int, true> queue2;

  for (int i = 0; i < 10; ++i) {
    queue2.enqueue(std::move(i));
  }

  EXPECT_EQ(queue2.size(), 10);

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(*queue2.tryDequeue(), i);
  }

  last = queue2.tryDequeue();
  EXPECT_EQ(last, std::nullopt);

  // For two dimensional control block
  WeightedRequestPileQueue<int, false, TwoDimensionalControlBlock> queue3;
  for (int i = 0; i < 10; ++i) {
    queue3.enqueue(std::move(i));
  }

  EXPECT_EQ(queue3.size(), 10);

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(*queue3.tryDequeue(), i);
  }

  last = queue3.tryDequeue();
  EXPECT_EQ(last, std::nullopt);

  WeightedRequestPileQueue<int, true, TwoDimensionalControlBlock> queue4;
  for (int i = 0; i < 10; ++i) {
    queue4.enqueue(std::move(i));
  }

  EXPECT_EQ(queue4.size(), 10);

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(*queue4.tryDequeue(), i);
  }

  last = queue4.tryDequeue();
  EXPECT_EQ(last, std::nullopt);
}

TEST(WeightedRequestPileQueueTest, ApplyLimits) {
  WeightedRequestPileQueue<int, true> queue;
  queue.setLimit(10);

  queue.enqueue(1, 2);
  queue.enqueue(1, 2);
  queue.enqueue(1, 2);
  queue.enqueue(1, 2);
  queue.enqueue(1, 2);

  // now the weight sum should be 10
  // reaching the limit
  EXPECT_EQ(queue.size(), 5);

  // This should then be rejected
  auto res = queue.enqueue(1, 2);
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Failed);
  res = queue.enqueue(1, 2);
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Failed);
  EXPECT_EQ(queue.size(), 5);

  queue.tryDequeue();
  EXPECT_EQ(queue.size(), 4);

  // This will make the weight sum to be 11
  // so should be rejected as well
  res = queue.enqueue(1, 3);
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Failed);
  EXPECT_EQ(queue.size(), 4);

  res = queue.enqueue(1, 2);
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Success);
  EXPECT_EQ(queue.size(), 5);

  queue.tryDequeue();
  queue.tryDequeue();
  queue.tryDequeue();
  queue.tryDequeue();
  queue.tryDequeue();

  // Let's also try this in a concurrent set up
  queue.setLimit(10000);

  auto task = [&] { queue.enqueue(1, 100); };

  // For two dimensional control block
  WeightedRequestPileQueue<int, true, TwoDimensionalControlBlock> queue2;
  queue2.setLimit({10, 10});

  queue2.enqueue(1, {2, 2});
  queue2.enqueue(1, {2, 2});
  queue2.enqueue(1, {2, 2});
  queue2.enqueue(1, {2, 2});
  queue2.enqueue(1, {2, 2});

  // now the weight sum should be 10
  // reaching the limit
  EXPECT_EQ(queue2.size(), 5);

  // This should then be rejected
  res = queue2.enqueue(1, {2, 2});
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Failed);
  res = queue2.enqueue(1, {2, 2});
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Failed);
  EXPECT_EQ(queue2.size(), 5);

  queue2.tryDequeue();
  EXPECT_EQ(queue2.size(), 4);

  // This will make the weight sum to be 11
  // so should be rejected as well
  res = queue2.enqueue(1, {3, 3});
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Failed);
  EXPECT_EQ(queue2.size(), 4);

  res = queue2.enqueue(1, {2, 2});
  EXPECT_EQ(res, RequestPileQueueAcceptResult::Success);
  EXPECT_EQ(queue2.size(), 5);

  queue2.tryDequeue();
  queue2.tryDequeue();
  queue2.tryDequeue();
  queue2.tryDequeue();
  queue2.tryDequeue();

  // Let's also try this in a concurrent set up
  queue2.setLimit({10000, 10000});

  auto task2 = [&] { queue2.enqueue(1, {100, 100}); };

  // we should let executor destructor run first to
  // join the threads before we check the counter
  auto g = folly::makeGuard([&] { EXPECT_EQ(queue.size(), 100); });
  auto g2 = folly::makeGuard([&] { EXPECT_EQ(queue2.size(), 100); });

  folly::CPUThreadPoolExecutor ex(32);
  for (int i = 0; i < 1000; ++i) {
    ex.add(task);
  }

  for (int i = 0; i < 1000; ++i) {
    ex.add(task2);
  }
}

TEST(WeightedRequestPileQueueTest, OnlyOneThreadGetsFirst) {
  // For one dimensional control block
  WeightedRequestPileQueue<int, true> queue;

  std::atomic<int> counter{0};

  auto task = [&] {
    auto res = queue.enqueue(1, 100);
    if (res == RequestPileQueueAcceptResult::FirstSuccess) {
      counter++;
    }
  };

  // For two dimensional control block
  WeightedRequestPileQueue<int, true, TwoDimensionalControlBlock> queue2;

  // we should let executor destructor run first to
  // join the threads before we check the counter
  auto g = folly::makeGuard([&] { EXPECT_EQ(counter.load(), 1); });

  folly::CPUThreadPoolExecutor ex(32);
  for (int i = 0; i < 1000; ++i) {
    ex.add(task);
  }

  std::atomic<int> counter2{0};

  auto task2 = [&] {
    auto res = queue2.enqueue(1, {100, 100});
    if (res == RequestPileQueueAcceptResult::FirstSuccess) {
      counter2++;
    }
  };

  // we should let executor destructor run first to
  // join the threads before we check the counter
  auto g2 = folly::makeGuard([&] { EXPECT_EQ(counter2.load(), 1); });

  for (int i = 0; i < 1000; ++i) {
    ex.add(task2);
  }
}

TEST(WeightedRequestPileQueueTest, DequeueWithCapacity) {
  using Queue1 =
      WeightedRequestPileQueue<int, true, OneDimensionalControlBlock>;
  Queue1 queue1;
  queue1.enqueue(1, 10);
  queue1.enqueue(1, 10);
  queue1.enqueue(1, 10);

  EXPECT_EQ(queue1.tryDequeue(), 1);
  EXPECT_TRUE(std::holds_alternative<int>(queue1.tryDequeueWithCapacity(20)));
  auto res1 =
      std::get<Queue1::DequeueRejReason>(queue1.tryDequeueWithCapacity(5));
  EXPECT_EQ(res1, Queue1::DequeueRejReason::OverCapacity);

  using Queue2 =
      WeightedRequestPileQueue<int, true, TwoDimensionalControlBlock>;
  Queue2 queue2;
  queue2.enqueue(1, {10, 10});
  queue2.enqueue(1, {10, 10});
  queue2.enqueue(1, {10, 10});

  EXPECT_EQ(queue2.tryDequeue(), 1);
  EXPECT_TRUE(
      std::holds_alternative<int>(queue2.tryDequeueWithCapacity({20, 20})));
  auto res2 =
      std::get<Queue2::DequeueRejReason>(queue2.tryDequeueWithCapacity({5, 5}));
  EXPECT_EQ(res2, Queue2::DequeueRejReason::OverCapacity);

  // now the queue is empty
  queue2.tryDequeueWithCapacity({11, 11});
  EXPECT_EQ(queue2.size(), 0);

  queue2.enqueue(1, {5, 5});
  res2 =
      std::get<Queue2::DequeueRejReason>(queue2.tryDequeueWithCapacity({6, 4}));
  EXPECT_EQ(res2, Queue2::DequeueRejReason::OverCapacity);
  res2 =
      std::get<Queue2::DequeueRejReason>(queue2.tryDequeueWithCapacity({4, 6}));
  EXPECT_EQ(res2, Queue2::DequeueRejReason::OverCapacity);
}
