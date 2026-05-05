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
#include <thrift/lib/cpp2/fast_thrift/common/test/MockMetricsContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/handler/RocketMetricsHandler.h>

using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::test;

// Mock stats type for testing
struct MockStats {
  struct MockCounter {
    void incrementValue(int64_t delta) noexcept { value_ += delta; }
    int64_t value() const noexcept { return value_; }
    int64_t value_{0};
  };

  MockCounter rocketInbound;
  MockCounter rocketOutbound;
  MockCounter rocketErrors;
  MockCounter rocketActive;
  MockCounter thriftInbound;
  MockCounter thriftOutbound;
  MockCounter thriftErrors;
  MockCounter thriftActive;
};

class RocketMetricsHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    stats_ = std::make_shared<MockStats>();
    handler_ =
        std::make_unique<RocketMetricsHandler<Direction::Server, MockStats>>(
            stats_);
  }

  std::shared_ptr<MockStats> stats_;
  std::unique_ptr<RocketMetricsHandler<Direction::Server, MockStats>> handler_;
  MockMetricsContext ctx_;
};

// =============================================================================
// onRead
// =============================================================================

TEST_F(RocketMetricsHandlerTest, OnReadIncrementsRocketInbound) {
  auto result = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketInbound.value(), 1);
}

TEST_F(RocketMetricsHandlerTest, OnReadIncrementsRocketActive) {
  auto result = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 1);
}

TEST_F(RocketMetricsHandlerTest, OnReadForwardsMessageUnmodified) {
  auto result = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(ctx_.readMessages()[0].get<int>(), 42);
}

TEST_F(RocketMetricsHandlerTest, OnReadReturnsContextResult) {
  ctx_.setReadResult(channel_pipeline::Result::Backpressure);
  auto result = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Backpressure);
}

// =============================================================================
// onWrite
// =============================================================================

TEST_F(RocketMetricsHandlerTest, OnWriteIncrementsRocketOutbound) {
  auto result = handler_->onWrite(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketOutbound.value(), 1);
}

TEST_F(RocketMetricsHandlerTest, OnWriteDecrementsRocketActive) {
  stats_->rocketActive.incrementValue(1);
  auto result = handler_->onWrite(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 0);
}

TEST_F(RocketMetricsHandlerTest, OnWriteForwardsMessageUnmodified) {
  auto result = handler_->onWrite(ctx_, channel_pipeline::TypeErasedBox(99));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_EQ(ctx_.writeMessages()[0].get<int>(), 99);
}

// =============================================================================
// onException
// =============================================================================

TEST_F(RocketMetricsHandlerTest, OnExceptionIncrementsRocketErrors) {
  handler_->onException(
      ctx_, folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_EQ(stats_->rocketErrors.value(), 1);
}

TEST_F(RocketMetricsHandlerTest, OnExceptionDoesNotAffectRocketActive) {
  stats_->rocketActive.incrementValue(1);
  handler_->onException(
      ctx_, folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_EQ(stats_->rocketActive.value(), 1);
}

TEST_F(RocketMetricsHandlerTest, OnExceptionForwardsException) {
  handler_->onException(
      ctx_, folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_TRUE(ctx_.hasException());
}

// =============================================================================
// Accumulation and lifecycle
// =============================================================================

TEST_F(RocketMetricsHandlerTest, MultipleRequestsCountsAccumulate) {
  for (int i = 0; i < 100; ++i) {
    auto result = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(i));
    EXPECT_EQ(result, channel_pipeline::Result::Success);
  }
  EXPECT_EQ(stats_->rocketInbound.value(), 100);
}

TEST_F(RocketMetricsHandlerTest, RequestResponseCycleActiveReturnsToZero) {
  auto r1 = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(1));
  EXPECT_EQ(r1, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 1);
  auto r2 = handler_->onWrite(ctx_, channel_pipeline::TypeErasedBox(2));
  EXPECT_EQ(r2, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 0);
}

// =============================================================================
// Direction::Client — active increments on write, decrements on read
// =============================================================================

class RocketMetricsHandlerClientTest : public ::testing::Test {
 protected:
  void SetUp() override {
    stats_ = std::make_shared<MockStats>();
    handler_ =
        std::make_unique<RocketMetricsHandler<Direction::Client, MockStats>>(
            stats_);
  }

  std::shared_ptr<MockStats> stats_;
  std::unique_ptr<RocketMetricsHandler<Direction::Client, MockStats>> handler_;
  MockMetricsContext ctx_;
};

TEST_F(RocketMetricsHandlerClientTest, OnWriteIncrementsRocketActive) {
  auto result = handler_->onWrite(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 1);
}

TEST_F(RocketMetricsHandlerClientTest, OnReadDecrementsRocketActive) {
  stats_->rocketActive.incrementValue(1);
  auto result = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(42));
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 0);
}

TEST_F(
    RocketMetricsHandlerClientTest, RequestResponseCycleActiveReturnsToZero) {
  auto r1 = handler_->onWrite(ctx_, channel_pipeline::TypeErasedBox(1));
  EXPECT_EQ(r1, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 1);
  auto r2 = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(2));
  EXPECT_EQ(r2, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->rocketActive.value(), 0);
}
