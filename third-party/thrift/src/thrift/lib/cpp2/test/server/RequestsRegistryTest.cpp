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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/server/RequestsRegistry.h>

using namespace apache::thrift;

namespace {
static uint32_t getCurrentServerTickCallCount = 0;
static uint64_t currentTick = 0;

} // namespace

namespace apache::thrift::detail {

THRIFT_PLUGGABLE_FUNC_SET(uint64_t, getCurrentServerTick) {
  ++getCurrentServerTickCallCount;
  return currentTick;
}

} // namespace apache::thrift::detail

// RecentRequestCounter tests
class RecentRequestCounterTest : public testing::Test {
 protected:
  void SetUp() override {
    // Reset mock getCurrentServerTick pluggable function
    getCurrentServerTickCallCount = 0;
    currentTick = 0;
  }

  RecentRequestCounter create() { return {}; }
};

TEST_F(RecentRequestCounterTest, testGetCurrentBucket) {
  auto counter = create();
  counter.increment();
  currentTick += 512;
  auto counts = counter.get();
  EXPECT_EQ(getCurrentServerTickCallCount, 2);
  EXPECT_EQ(counts[0].arrivalCount, 0);
}

TEST_F(RecentRequestCounterTest, testIncrement) {
  auto counter = create();
  counter.increment();
  auto counts = counter.get();
  EXPECT_EQ(counts[0].arrivalCount, 1);

  counter.increment();
  ++currentTick;
  counter.increment();
  counts = counter.get();
  // arrived requests
  EXPECT_EQ(counts[0].arrivalCount, 1);
  EXPECT_EQ(counts[1].arrivalCount, 2);

  // active requests
  EXPECT_EQ(counts[0].activeCount, 3);
  EXPECT_EQ(counts[1].activeCount, 2);
}

TEST_F(RecentRequestCounterTest, testGetReturnsMostRecentBucketFirst) {
  auto counter = create();
  counter.increment();
  counter.incrementOverloadCount();
  currentTick = 256;
  counter.increment();
  counter.increment();
  counter.incrementOverloadCount();
  counter.incrementOverloadCount();
  auto counts = counter.get();
  // arrived requests
  EXPECT_EQ(counts[0].arrivalCount, 2);
  EXPECT_EQ(counts[256].arrivalCount, 1);

  // active requests
  EXPECT_EQ(counts[0].activeCount, 3);
  EXPECT_EQ(counts[256].activeCount, 1);

  // overload error requests
  EXPECT_EQ(counts[0].overloadCount, 2);
  EXPECT_EQ(counts[256].overloadCount, 1);
}

TEST_F(RecentRequestCounterTest, testTooManyDecrement) {
  auto counter = create();
  counter.decrement();
  counter.decrement();
  auto counts = counter.get();
  // arrived requests
  EXPECT_EQ(counts[0].arrivalCount, 0);
  // active requests
  EXPECT_EQ(counts[0].activeCount, 0);
}

TEST_F(RecentRequestCounterTest, testIncrementDecrement) {
  auto counter = create();
  counter.increment();
  counter.decrement();
  counter.increment();
  counter.increment();
  currentTick += 2;
  counter.decrement();
  counter.decrement();
  auto counts = counter.get();
  // arrived requests
  EXPECT_EQ(counts[0].arrivalCount, 0);
  EXPECT_EQ(counts[1].arrivalCount, 0);
  EXPECT_EQ(counts[2].arrivalCount, 3);
  // active requests
  EXPECT_EQ(counts[0].activeCount, 0);
  EXPECT_EQ(counts[1].activeCount, 2);
  EXPECT_EQ(counts[2].activeCount, 2);
}

TEST_F(RecentRequestCounterTest, testIncrementOverloadCount) {
  auto counter = create();
  {
    counter.incrementOverloadCount();
    auto counts = counter.get();
    EXPECT_EQ(counts[0].overloadCount, 1);
  }
  {
    counter.incrementOverloadCount();
    ++currentTick;
    counter.incrementOverloadCount();
    auto counts = counter.get();
    EXPECT_EQ(counts[0].overloadCount, 1);
    EXPECT_EQ(counts[1].overloadCount, 2);
  }
}
