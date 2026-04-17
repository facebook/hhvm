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
 * Unit tests for RocketServerAppAdapter.
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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerAppAdapter.h>

namespace apache::thrift::fast_thrift::rocket::server::test {

using channel_pipeline::HeadToTailOp;
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
    RocketServerAppOutboundHandler<RocketServerAppAdapter>,
    "RocketServerAppAdapter must satisfy RocketServerAppOutboundHandler");

static_assert(
    channel_pipeline::TailEndpointHandler<RocketServerAppAdapter>,
    "RocketServerAppAdapter must satisfy TailEndpointHandler");

static_assert(
    RocketServerAppInboundHandler<RocketServerAppAdapter>,
    "RocketServerAppAdapter must satisfy RocketServerAppInboundHandler");

// =============================================================================
// Unit tests
// =============================================================================

TEST(RocketServerAppAdapterTest, WriteWithoutPipelineReturnsError) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());

  RocketResponseMessage msg{
      .payload = folly::IOBuf::copyBuffer("test"),
      .metadata = nullptr,
      .streamId = 1,
  };

  EXPECT_EQ(adapter->write(std::move(msg)), Result::Error);
}

TEST(RocketServerAppAdapterTest, OnReadDelegatesToCallback) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  int requestCount = 0;

  adapter->setRequestHandlers(
      [&](TypeErasedBox&& /*msg*/) noexcept -> Result {
        requestCount++;
        return Result::Success;
      },
      [](folly::exception_wrapper&&) noexcept {});

  auto box = channel_pipeline::erase_and_box(
      rocket::server::RocketRequestMessage{
          .frame = {}, .error = {}, .streamId = 1});

  auto result = adapter->onRead(std::move(box));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(requestCount, 1);
}

TEST(RocketServerAppAdapterTest, OnExceptionDelegatesToCallback) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  int errorCount = 0;

  adapter->setRequestHandlers(
      [](TypeErasedBox&&) noexcept -> Result { return Result::Success; },
      [&](folly::exception_wrapper&& /*e*/) noexcept { errorCount++; });

  adapter->onException(
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  EXPECT_EQ(errorCount, 1);
}

TEST(RocketServerAppAdapterTest, OnReadWithoutCallbackReturnsError) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  // No callback set

  auto box =
      channel_pipeline::erase_and_box(rocket::server::RocketRequestMessage{});
  auto result = adapter->onRead(std::move(box));
  EXPECT_EQ(result, Result::Error);
}

HANDLER_TAG(mock_head_tag);

TEST(RocketServerAppAdapterTest, WriteWithPipelineCallsFireWrite) {
  folly::EventBase evb;
  MockTailHandler head;
  TestAllocator allocator;
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());

  // Server pipeline: head=transport(mock), tail=app(adapter)
  auto pipeline =
      PipelineBuilder<MockTailHandler, RocketServerAppAdapter, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(adapter.get())
          .setAllocator(&allocator)
          .build();

  adapter->setPipeline(pipeline.get());

  RocketResponseMessage msg{
      .payload = folly::IOBuf::copyBuffer("response"),
      .metadata = nullptr,
      .streamId = 1,
  };

  auto result = adapter->write(std::move(msg));
  EXPECT_EQ(result, Result::Success);
  // Write goes from tail→head, so mock head should receive it
  EXPECT_EQ(head.messageCount(), 1);
}

} // namespace apache::thrift::fast_thrift::rocket::server::test
