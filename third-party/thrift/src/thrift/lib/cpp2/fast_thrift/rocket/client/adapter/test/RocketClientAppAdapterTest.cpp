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
 * Unit tests for RocketClientAppAdapter.
 *
 * Tests the adapter in isolation: concept satisfaction, delegation to
 * callbacks, and outbound write behavior.
 */

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientAppAdapter.h>

namespace apache::thrift::fast_thrift::rocket::client::test {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;
using channel_pipeline::test::MockHeadHandler;
using channel_pipeline::test::MockTailHandler;
using channel_pipeline::test::TestAllocator;

// =============================================================================
// Concept static assertions
// =============================================================================

static_assert(
    RocketClientAppOutboundHandler<RocketClientAppAdapter>,
    "RocketClientAppAdapter must satisfy RocketClientAppOutboundHandler");

static_assert(
    channel_pipeline::TailEndpointHandler<RocketClientAppAdapter>,
    "RocketClientAppAdapter must satisfy TailEndpointHandler");

static_assert(
    RocketClientAppInboundHandler<RocketClientAppAdapter>,
    "RocketClientAppAdapter must satisfy RocketClientAppInboundHandler");

// =============================================================================
// Unit tests
// =============================================================================

TEST(RocketClientAppAdapterTest, WriteWithoutPipelineReturnsError) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());

  RocketRequestMessage msg{
      .frame =
          RocketFramePayload{
              .metadata = nullptr,
              .data = folly::IOBuf::copyBuffer("test"),
          },
      .frameType = frame::FrameType::REQUEST_RESPONSE,
  };

  EXPECT_EQ(adapter->write(std::move(msg)), Result::Error);
}

TEST(RocketClientAppAdapterTest, OnReadDelegatesToCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int responseCount = 0;

  adapter->setResponseHandlers(
      [&](TypeErasedBox&& /*msg*/) noexcept -> Result {
        responseCount++;
        return Result::Success;
      },
      [](folly::exception_wrapper&&) noexcept {});

  auto box = channel_pipeline::erase_and_box(
      RocketResponseMessage{
          .frame = {},
          .requestHandle = 42,
          .requestFrameType = frame::FrameType::REQUEST_RESPONSE,
      });

  auto result = adapter->onRead(std::move(box));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(responseCount, 1);
}

TEST(RocketClientAppAdapterTest, OnExceptionDelegatesToCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int errorCount = 0;

  adapter->setResponseHandlers(
      [](TypeErasedBox&&) noexcept -> Result { return Result::Success; },
      [&](folly::exception_wrapper&& /*e*/) noexcept { errorCount++; });

  adapter->onException(
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  EXPECT_EQ(errorCount, 1);
}

TEST(RocketClientAppAdapterTest, OnReadWithoutCallbackReturnsError) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  // No callback set

  auto box = channel_pipeline::erase_and_box(RocketResponseMessage{});
  auto result = adapter->onRead(std::move(box));
  EXPECT_EQ(result, Result::Error);
}

HANDLER_TAG(mock_tail_tag);

TEST(RocketClientAppAdapterTest, WriteWithPipelineCallsFireWrite) {
  folly::EventBase evb;
  MockTailHandler head;
  TestAllocator allocator;
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());

  auto pipeline =
      PipelineBuilder<MockTailHandler, RocketClientAppAdapter, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(adapter.get())
          .setAllocator(&allocator)
          .build();

  adapter->setPipeline(pipeline.get());

  RocketRequestMessage msg{
      .frame =
          RocketFramePayload{
              .metadata = nullptr,
              .data = folly::IOBuf::copyBuffer("test"),
          },
      .frameType = frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = adapter->write(std::move(msg));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(head.messageCount(), 1);
}

} // namespace apache::thrift::fast_thrift::rocket::client::test
