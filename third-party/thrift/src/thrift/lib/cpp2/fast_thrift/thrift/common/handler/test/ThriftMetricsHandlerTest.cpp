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
#include <thrift/lib/cpp2/fast_thrift/thrift/common/handler/ThriftMetricsHandler.h>

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

class ThriftMetricsHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    stats_ = std::make_shared<MockStats>();
    handler_ =
        std::make_unique<ThriftMetricsHandler<Direction::Server, MockStats>>(
            stats_.get());
  }

  std::shared_ptr<MockStats> stats_;
  std::unique_ptr<ThriftMetricsHandler<Direction::Server, MockStats>> handler_;
  MockMetricsContext ctx_;
};

// =============================================================================
// onRead
// =============================================================================

namespace {
// A minimal request/response pair — what this handler sees in a real server
// pipeline. Boxing a bare value no longer stands in, because counting now
// depends on whether the message is an RPC.
channel_pipeline::TypeErasedBox makeRequestBox() {
  thrift::ThriftServerRequestMessage req;
  req.streamId = 1;
  req.payload.emplace<thrift::ThriftRequestResponsePayload>();
  return channel_pipeline::erase_and_box(std::move(req));
}

channel_pipeline::TypeErasedBox makeResponseBox() {
  thrift::ThriftServerResponseMessage resp;
  resp.payload.emplace<thrift::ThriftInitialResponsePayload>();
  return channel_pipeline::erase_and_box(std::move(resp));
}
} // namespace

TEST_F(ThriftMetricsHandlerTest, OnReadIncrementsThriftInbound) {
  auto result = handler_->onRead(ctx_, makeRequestBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftInbound.value(), 1);
}

TEST_F(ThriftMetricsHandlerTest, OnReadIncrementsThriftActive) {
  auto result = handler_->onRead(ctx_, makeRequestBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 1);
}

TEST_F(ThriftMetricsHandlerTest, OnReadForwardsMessageUnmodified) {
  auto result = handler_->onRead(ctx_, makeRequestBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.readMessages()[0]
                  .get<thrift::ThriftServerRequestMessage>()
                  .payload.is<thrift::ThriftRequestResponsePayload>());
}

TEST_F(ThriftMetricsHandlerTest, OnReadReturnsContextResult) {
  ctx_.setReadResult(channel_pipeline::Result::Backpressure);
  auto result = handler_->onRead(ctx_, makeRequestBox());
  EXPECT_EQ(result, channel_pipeline::Result::Backpressure);
}

// =============================================================================
// onWrite
// =============================================================================

TEST_F(ThriftMetricsHandlerTest, OnWriteIncrementsThriftOutbound) {
  auto result = handler_->onWrite(ctx_, makeResponseBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftOutbound.value(), 1);
}

TEST_F(ThriftMetricsHandlerTest, OnWriteDecrementsThriftActive) {
  stats_->thriftActive.incrementValue(1);
  auto result = handler_->onWrite(ctx_, makeResponseBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 0);
}

TEST_F(ThriftMetricsHandlerTest, OnWriteForwardsMessageUnmodified) {
  auto result = handler_->onWrite(ctx_, makeResponseBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages()[0]
                  .get<thrift::ThriftServerResponseMessage>()
                  .payload.is<thrift::ThriftInitialResponsePayload>());
}

// =============================================================================
// onException
// =============================================================================

TEST_F(ThriftMetricsHandlerTest, OnExceptionIncrementsThriftErrors) {
  handler_->onException(
      ctx_, folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_EQ(stats_->thriftErrors.value(), 1);
}

TEST_F(ThriftMetricsHandlerTest, OnExceptionDoesNotAffectThriftActive) {
  stats_->thriftActive.incrementValue(1);
  handler_->onException(
      ctx_, folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_EQ(stats_->thriftActive.value(), 1);
}

TEST_F(ThriftMetricsHandlerTest, OnExceptionForwardsException) {
  handler_->onException(
      ctx_, folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_TRUE(ctx_.hasException());
}

// =============================================================================
// Accumulation and lifecycle
// =============================================================================

TEST_F(ThriftMetricsHandlerTest, MultipleRequestsCountsAccumulate) {
  for (int i = 0; i < 100; ++i) {
    auto result = handler_->onRead(ctx_, makeRequestBox());
    EXPECT_EQ(result, channel_pipeline::Result::Success);
  }
  EXPECT_EQ(stats_->thriftInbound.value(), 100);
}

TEST_F(ThriftMetricsHandlerTest, RequestResponseCycleActiveReturnsToZero) {
  auto r1 = handler_->onRead(ctx_, makeRequestBox());
  EXPECT_EQ(r1, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 1);
  auto r2 = handler_->onWrite(ctx_, makeResponseBox());
  EXPECT_EQ(r2, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 0);
}

// =============================================================================
// Direction::Client — active increments on write, decrements on read
// =============================================================================

class ThriftMetricsHandlerClientTest : public ::testing::Test {
 protected:
  void SetUp() override {
    stats_ = std::make_shared<MockStats>();
    handler_ =
        std::make_unique<ThriftMetricsHandler<Direction::Client, MockStats>>(
            stats_.get());
  }

  std::shared_ptr<MockStats> stats_;
  std::unique_ptr<ThriftMetricsHandler<Direction::Client, MockStats>> handler_;
  MockMetricsContext ctx_;
};

TEST_F(ThriftMetricsHandlerClientTest, OnWriteIncrementsThriftActive) {
  auto result = handler_->onWrite(ctx_, makeResponseBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 1);
}

TEST_F(ThriftMetricsHandlerClientTest, OnReadDecrementsThriftActive) {
  stats_->thriftActive.incrementValue(1);
  auto result = handler_->onRead(ctx_, makeRequestBox());
  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 0);
}

TEST_F(
    ThriftMetricsHandlerClientTest, RequestResponseCycleActiveReturnsToZero) {
  auto r1 = handler_->onWrite(ctx_, channel_pipeline::TypeErasedBox(1));
  EXPECT_EQ(r1, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 1);
  auto r2 = handler_->onRead(ctx_, channel_pipeline::TypeErasedBox(2));
  EXPECT_EQ(r2, channel_pipeline::Result::Success);
  EXPECT_EQ(stats_->thriftActive.value(), 0);
}
