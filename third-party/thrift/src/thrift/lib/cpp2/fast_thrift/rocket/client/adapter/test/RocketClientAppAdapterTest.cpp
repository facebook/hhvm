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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::rocket::client::test {

namespace transport = apache::thrift::fast_thrift::transport;

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
          .payload = {},
          .requestContext = {},
          .streamType = frame::FrameType::REQUEST_RESPONSE,
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

TEST(RocketClientAppAdapterTest, OnPipelineActiveFiresOnActiveCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int activeCount = 0;

  adapter->setLifecycleHandlers(
      [&]() noexcept { activeCount++; }, []() noexcept {});

  adapter->onPipelineActive();
  EXPECT_EQ(activeCount, 1);
}

TEST(RocketClientAppAdapterTest, OnPipelineInactiveFiresOnInactiveCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int inactiveCount = 0;

  adapter->setLifecycleHandlers(
      []() noexcept {}, [&]() noexcept { inactiveCount++; });

  adapter->onPipelineInactive();
  EXPECT_EQ(inactiveCount, 1);
}

TEST(RocketClientAppAdapterTest, LifecycleCallbacksNoOpWhenUnset) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  // No setLifecycleHandlers call.
  adapter->onPipelineActive();
  adapter->onPipelineInactive();
  adapter->handlerRemoved();
  // Reaching here without crash is the test.
}

TEST(RocketClientAppAdapterTest, LifecycleAndErrorChannelsAreIndependent) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int errorCount = 0;
  int inactiveCount = 0;

  adapter->setResponseHandlers(
      [](TypeErasedBox&&) noexcept -> Result { return Result::Success; },
      [&](folly::exception_wrapper&&) noexcept { errorCount++; });
  adapter->setLifecycleHandlers(
      []() noexcept {}, [&]() noexcept { inactiveCount++; });

  // Lifecycle event must NOT route through the error callback.
  adapter->onPipelineInactive();
  EXPECT_EQ(inactiveCount, 1);
  EXPECT_EQ(errorCount, 0);

  // Error event must NOT route through the lifecycle callback.
  adapter->onException(
      folly::make_exception_wrapper<std::runtime_error>("boom"));
  EXPECT_EQ(errorCount, 1);
  EXPECT_EQ(inactiveCount, 1);
}

TEST(RocketClientAppAdapterTest, OnWriteReadyInvokesCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int readyCount = 0;
  adapter->setOnWriteReady([&]() noexcept { readyCount++; });

  adapter->onWriteReady();
  adapter->onWriteReady();

  EXPECT_EQ(readyCount, 2);
}

TEST(RocketClientAppAdapterTest, OnWriteReadyNoOpWhenCallbackUnset) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  // No setOnWriteReady call — must not crash.
  adapter->onWriteReady();
}

TEST(RocketClientAppAdapterTest, HandlerRemovedClearsOnWriteReadyCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int readyCount = 0;
  adapter->setOnWriteReady([&]() noexcept { readyCount++; });

  adapter->handlerRemoved();
  adapter->onWriteReady();

  EXPECT_EQ(readyCount, 0);
}

TEST(RocketClientAppAdapterTest, OnEventFiresOnCloseCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int closeCount = 0;
  adapter->setOnClose([&]() noexcept { closeCount++; });

  // ConnectionClose carries no payload — the id alone is the signal.
  adapter->onEvent(
      RocketClientEventId::ConnectionClose, channel_pipeline::TypeErasedBox{});

  EXPECT_EQ(closeCount, 1);
}

TEST(RocketClientAppAdapterTest, OnEventNoOpWhenCloseCallbackUnset) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  // No setOnClose — must not crash.
  adapter->onEvent(
      RocketClientEventId::ConnectionClose, channel_pipeline::TypeErasedBox{});
}

TEST(RocketClientAppAdapterTest, HandlerRemovedClearsOnCloseCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int closeCount = 0;
  adapter->setOnClose([&]() noexcept { closeCount++; });

  adapter->handlerRemoved();
  adapter->onEvent(
      RocketClientEventId::ConnectionClose, channel_pipeline::TypeErasedBox{});

  EXPECT_EQ(closeCount, 0);
}

TEST(RocketClientAppAdapterTest, OnEventInvokesOnWriteCompleteCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int count = 0;
  transport::WriteCompletionStatus status{};
  size_t frameCount = 0;
  size_t bytes = 0;
  adapter->setOnWriteComplete([&](const RocketWriteCompleteEvent& e) noexcept {
    count++;
    status = e.status;
    frameCount = e.frameCount;
    bytes = e.bytes;
  });

  adapter->onEvent(
      RocketClientEventId::RocketWriteComplete,
      TypeErasedBox(
          RocketWriteCompleteEvent{
              .status = transport::WriteCompletionStatus::Error,
              .frameCount = 3,
              .bytes = 128,
          }));

  EXPECT_EQ(count, 1);
  EXPECT_EQ(status, transport::WriteCompletionStatus::Error);
  EXPECT_EQ(frameCount, 3u);
  EXPECT_EQ(bytes, 128u);
}

TEST(RocketClientAppAdapterTest, OnEventNoOpWhenCallbackUnset) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  // No setOnWriteComplete call — must not crash.
  adapter->onEvent(
      RocketClientEventId::RocketWriteComplete,
      TypeErasedBox(
          RocketWriteCompleteEvent{
              .status = transport::WriteCompletionStatus::Success,
              .frameCount = 1,
              .bytes = 0,
          }));
}

TEST(RocketClientAppAdapterTest, HandlerRemovedClearsOnWriteCompleteCallback) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  int count = 0;
  adapter->setOnWriteComplete(
      [&](const RocketWriteCompleteEvent&) noexcept { count++; });

  adapter->handlerRemoved();
  adapter->onEvent(
      RocketClientEventId::RocketWriteComplete,
      TypeErasedBox(
          RocketWriteCompleteEvent{
              .status = transport::WriteCompletionStatus::Success,
              .frameCount = 1,
              .bytes = 0,
          }));

  EXPECT_EQ(count, 0);
}

HANDLER_TAG(mock_head_tag);

TEST(RocketClientAppAdapterTest, WriteWithPipelineCallsFireWrite) {
  folly::EventBase evb;
  MockHeadHandler head;
  head.setOnWriteCallback(
      [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });
  TestAllocator allocator;
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());

  auto pipeline =
      PipelineBuilder<MockHeadHandler, RocketClientAppAdapter, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(adapter.get())
          .setAllocator(&allocator)
          .build();

  adapter->setPipeline(pipeline.get());

  RocketRequestMessage msg{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType = frame::FrameType::REQUEST_RESPONSE,
              .metadata = nullptr,
              .data = folly::IOBuf::copyBuffer("test"),
          },
      .requestContext = {},
      .streamType = frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = adapter->write(std::move(msg));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(head.writeCount(), 1);

  pipeline->deactivate();
  pipeline->close();
  adapter->resetPipeline();
}

TEST(RocketClientAppAdapterTest, NotifyReadReadyFiresPipelineOnReadReady) {
  folly::EventBase evb;
  MockHeadHandler head;
  TestAllocator allocator;
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());

  auto pipeline =
      PipelineBuilder<MockHeadHandler, RocketClientAppAdapter, TestAllocator>()
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

TEST(RocketClientAppAdapterTest, NotifyReadReadyNoOpWhenPipelineUnset) {
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());
  // No setPipeline call — must not crash.
  adapter->notifyReadReady();
}

TEST(RocketClientAppAdapterTest, RocketWriteCompleteDeliveredThroughPipeline) {
  folly::EventBase evb;
  MockHeadHandler head;
  TestAllocator allocator;
  RocketClientAppAdapter::Ptr adapter(new RocketClientAppAdapter());

  int count = 0;
  size_t frameCount = 0;
  adapter->setOnWriteComplete([&](const RocketWriteCompleteEvent& e) noexcept {
    count++;
    frameCount = e.frameCount;
  });

  // Built with RocketClientEventId so the adapter's subscription is wired; a
  // NoEvent pipeline would compile the subscription out entirely.
  auto pipeline = PipelineBuilder<
                      MockHeadHandler,
                      RocketClientAppAdapter,
                      TestAllocator,
                      RocketClientEventId>()
                      .setEventBase(&evb)
                      .setHead(&head)
                      .setTail(adapter.get())
                      .setAllocator(&allocator)
                      .build();

  // The enriched event reaches the subscribed callback.
  pipeline->fireEvent(
      RocketClientEventId::RocketWriteComplete,
      TypeErasedBox(
          RocketWriteCompleteEvent{
              .status = transport::WriteCompletionStatus::Success,
              .frameCount = 4,
              .bytes = 256,
          }));
  EXPECT_EQ(count, 1);
  EXPECT_EQ(frameCount, 4u);

  // The raw transport event is not delivered — the adapter subscribes only to
  // RocketWriteComplete.
  pipeline->fireEvent(
      RocketClientEventId::TransportWriteComplete,
      TypeErasedBox(
          TransportWriteCompleteEvent{
              .status = transport::WriteCompletionStatus::Success,
              .bytes = 256,
          }));
  EXPECT_EQ(count, 1);

  pipeline->close();
}

} // namespace apache::thrift::fast_thrift::rocket::client::test
