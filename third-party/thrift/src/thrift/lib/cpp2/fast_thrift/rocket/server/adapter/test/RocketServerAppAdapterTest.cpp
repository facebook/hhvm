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
      rocket::server::RocketRequestMessage{.frame = {}, .streamId = 1});

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

TEST(RocketServerAppAdapterTest, OnPipelineActiveFiresOnConnectCallback) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  int connectCount = 0;

  adapter->setLifecycleHandlers(
      [&]() noexcept { connectCount++; }, []() noexcept {});

  adapter->onPipelineActive();
  EXPECT_EQ(connectCount, 1);
}

TEST(RocketServerAppAdapterTest, OnPipelineInactiveFiresOnDisconnectCallback) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  int disconnectCount = 0;

  adapter->setLifecycleHandlers(
      []() noexcept {}, [&]() noexcept { disconnectCount++; });

  adapter->onPipelineActive();
  adapter->onPipelineInactive();
  EXPECT_EQ(disconnectCount, 1);
}

TEST(
    RocketServerAppAdapterTest,
    HandlerRemovedAfterDeactivateDoesNotRefireDisconnect) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  int disconnectCount = 0;

  adapter->setLifecycleHandlers(
      []() noexcept {}, [&]() noexcept { disconnectCount++; });

  adapter->onPipelineActive();
  adapter->onPipelineInactive();
  EXPECT_EQ(disconnectCount, 1);

  adapter->handlerRemoved();
  // handlerRemoved must not synthesize a second disconnect.
  EXPECT_EQ(disconnectCount, 1);
}

TEST(RocketServerAppAdapterTest, LifecycleCallbacksNoOpWhenUnset) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  // No setLifecycleHandlers call.
  adapter->onPipelineActive();
  adapter->onPipelineInactive();
  adapter->handlerRemoved();
  // Reaching here without crash is the test.
}

TEST(RocketServerAppAdapterTest, LifecycleAndErrorChannelsAreIndependent) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  int errorCount = 0;
  int inactiveCount = 0;

  adapter->setRequestHandlers(
      [](TypeErasedBox&&) noexcept -> Result { return Result::Success; },
      [&](folly::exception_wrapper&&) noexcept { errorCount++; });
  adapter->setLifecycleHandlers(
      []() noexcept {}, [&]() noexcept { inactiveCount++; });

  // Lifecycle event must NOT route through the error callback.
  adapter->onPipelineActive();
  adapter->onPipelineInactive();
  EXPECT_EQ(inactiveCount, 1);
  EXPECT_EQ(errorCount, 0);

  // Error event must NOT route through the lifecycle callback.
  adapter->onException(
      folly::make_exception_wrapper<std::runtime_error>("boom"));
  EXPECT_EQ(errorCount, 1);
  EXPECT_EQ(inactiveCount, 1);
}

TEST(RocketServerAppAdapterTest, OnWriteReadyInvokesCallback) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  int readyCount = 0;
  adapter->setOnWriteReady([&]() noexcept { readyCount++; });

  adapter->onWriteReady();
  adapter->onWriteReady();

  EXPECT_EQ(readyCount, 2);
}

TEST(RocketServerAppAdapterTest, OnWriteReadyNoOpWhenCallbackUnset) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  // No setOnWriteReady call — must not crash.
  adapter->onWriteReady();
}

HANDLER_TAG(mock_head_tag);

TEST(RocketServerAppAdapterTest, WriteWithPipelineCallsFireWrite) {
  folly::EventBase evb;
  MockHeadHandler head;
  head.setOnWriteCallback(
      [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });
  TestAllocator allocator;
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());

  // Server pipeline: head=transport(mock), tail=app(adapter)
  auto pipeline =
      PipelineBuilder<MockHeadHandler, RocketServerAppAdapter, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(adapter.get())
          .setAllocator(&allocator)
          .build();

  adapter->setPipeline(pipeline.get());

  RocketResponseMessage msg{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType =
                  apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
              .streamId = 1,
              .metadata = nullptr,
              .data = folly::IOBuf::copyBuffer("response"),
          },
  };

  auto result = adapter->write(std::move(msg));
  EXPECT_EQ(result, Result::Success);
  // Write goes from tail→head, so mock head should receive it
  EXPECT_EQ(head.writeCount(), 1);

  pipeline->deactivate();
  pipeline->close();
  adapter->resetPipeline();
}

TEST(RocketServerAppAdapterTest, NotifyReadReadyFiresPipelineOnReadReady) {
  folly::EventBase evb;
  MockHeadHandler head;
  TestAllocator allocator;
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());

  auto pipeline =
      PipelineBuilder<MockHeadHandler, RocketServerAppAdapter, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(adapter.get())
          .setAllocator(&allocator)
          .build();
  adapter->setPipeline(pipeline.get());

  adapter->notifyReadReady();

  EXPECT_EQ(head.onReadReadyCount(), 1);

  pipeline->close();
  adapter->resetPipeline();
}

TEST(RocketServerAppAdapterTest, NotifyReadReadyNoOpWhenPipelineUnset) {
  RocketServerAppAdapter::Ptr adapter(new RocketServerAppAdapter());
  // No setPipeline call — must not crash.
  adapter->notifyReadReady();
}

} // namespace apache::thrift::fast_thrift::rocket::server::test
