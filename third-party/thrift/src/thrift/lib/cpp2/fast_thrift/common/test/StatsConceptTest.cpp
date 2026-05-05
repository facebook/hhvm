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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/fast_thrift/common/Stats.h>

using namespace apache::thrift::fast_thrift;

// =============================================================================
// Concept satisfaction tests
// =============================================================================

// Custom stats implementation for testing
struct CustomStats {
  struct CustomCounter {
    void incrementValue(int64_t delta) noexcept { val += delta; }
    int64_t value() const noexcept { return val; }
    int64_t val{0};
  };

  CustomCounter rocketInbound;
  CustomCounter rocketOutbound;
  CustomCounter rocketErrors;
  CustomCounter rocketActive;
  CustomCounter thriftInbound;
  CustomCounter thriftOutbound;
  CustomCounter thriftErrors;
  CustomCounter thriftActive;
};

TEST(StatsConceptTest, CustomStatsSatisfiesConcept) {
  // Verify that a custom stats type with the right interface satisfies the
  // concept
  static_assert(FastThriftStatsConcept<CustomStats>);
  EXPECT_TRUE(true);
}

TEST(StatsConceptTest, CustomStatsCounterOperations) {
  CustomStats stats;

  // Test increment and value operations
  stats.rocketInbound.incrementValue(5);
  EXPECT_EQ(stats.rocketInbound.value(), 5);

  stats.rocketOutbound.incrementValue(3);
  EXPECT_EQ(stats.rocketOutbound.value(), 3);

  stats.rocketErrors.incrementValue(1);
  EXPECT_EQ(stats.rocketErrors.value(), 1);

  stats.rocketActive.incrementValue(10);
  EXPECT_EQ(stats.rocketActive.value(), 10);

  stats.thriftInbound.incrementValue(7);
  EXPECT_EQ(stats.thriftInbound.value(), 7);

  stats.thriftOutbound.incrementValue(2);
  EXPECT_EQ(stats.thriftOutbound.value(), 2);

  stats.thriftErrors.incrementValue(1);
  EXPECT_EQ(stats.thriftErrors.value(), 1);

  stats.thriftActive.incrementValue(4);
  EXPECT_EQ(stats.thriftActive.value(), 4);
}

// Test that Counter concept works with custom counter
TEST(StatsConceptTest, CounterConceptSatisfiedByCustomCounter) {
  static_assert(CounterConcept<CustomStats::CustomCounter>);
  EXPECT_TRUE(true);
}
