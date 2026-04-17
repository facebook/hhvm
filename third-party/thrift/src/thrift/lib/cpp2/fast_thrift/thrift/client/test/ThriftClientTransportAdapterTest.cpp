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
 * Unit tests for ThriftClientTransportAdapter.
 *
 * Tests the adapter in isolation: concept satisfaction, outbound conversion
 * (ThriftRequestMessage → RocketRequestMessage), and inbound conversion
 * (RocketResponseMessage → ThriftResponseMessage).
 */

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>

namespace apache::thrift::fast_thrift::thrift::client::test {

using channel_pipeline::erase_and_box;
using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;
using channel_pipeline::test::MockHeadHandler;
using channel_pipeline::test::TestAllocator;

// =============================================================================
// Concept static assertions
// =============================================================================

static_assert(
    channel_pipeline::EndpointHandler<ThriftClientTransportAdapter>,
    "ThriftClientTransportAdapter must satisfy EndpointHandler");

// =============================================================================
// Helpers
// =============================================================================

namespace {

TypeErasedBox makeThriftRequestBox(
    apache::thrift::RpcKind rpcKind =
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE) {
  ThriftRequestMessage msg{
      .payload =
          ThriftRequestPayload{
              .metadata = folly::IOBuf::copyBuffer("meta"),
              .data = folly::IOBuf::copyBuffer("data"),
              .rpcKind = rpcKind,
              .complete = true,
          },
      .requestHandle = 42,
  };
  return erase_and_box(std::move(msg));
}

TypeErasedBox makeRocketResponseBox(
    uint32_t requestHandle = 42,
    frame::FrameType requestFrameType = frame::FrameType::REQUEST_RESPONSE) {
  auto data = folly::IOBuf::copyBuffer("response-data");
  auto frameBuf = frame::write::serialize(
      frame::write::PayloadHeader{
          .streamId = 1, .follows = false, .complete = true, .next = true},
      nullptr,
      std::move(data));

  rocket::RocketResponseMessage response{
      .frame = frame::read::parseFrame(std::move(frameBuf)),
      .requestHandle = requestHandle,
      .requestFrameType = requestFrameType,
  };
  return erase_and_box(std::move(response));
}

struct AdapterWithRocketPipeline {
  folly::EventBase evb;
  MockHeadHandler rocketTail;
  TestAllocator allocator;
  std::unique_ptr<ThriftClientTransportAdapter> adapter;

  AdapterWithRocketPipeline() {
    auto connection =
        std::make_unique<rocket::client::RocketClientConnection>();
    auto* appAdapter = connection->appAdapter.get();

    auto rocketPipeline = PipelineBuilder<
                              rocket::client::RocketClientAppAdapter,
                              MockHeadHandler,
                              TestAllocator>()
                              .setEventBase(&evb)
                              .setHead(appAdapter)
                              .setTail(&rocketTail)
                              .setAllocator(&allocator)
                              .build();

    appAdapter->setPipeline(rocketPipeline.get());
    connection->pipeline = std::move(rocketPipeline);

    adapter =
        std::make_unique<ThriftClientTransportAdapter>(std::move(connection));
  }
};

} // namespace

// =============================================================================
// Unit tests
// =============================================================================

TEST(ThriftClientTransportAdapterTest, OnMessageConvertsAndWritesToRocket) {
  AdapterWithRocketPipeline fixture;

  auto result = fixture.adapter->onMessage(makeThriftRequestBox());
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(fixture.rocketTail.messageCount(), 1);
}

TEST(ThriftClientTransportAdapterTest, OnMessageConvertsRpcKindToFrameType) {
  AdapterWithRocketPipeline fixture;
  TypeErasedBox capturedMsg;

  fixture.rocketTail.setOnMessageCallback([&](TypeErasedBox&& msg) {
    capturedMsg = std::move(msg);
    return Result::Success;
  });

  auto result = fixture.adapter->onMessage(makeThriftRequestBox(
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE));
  EXPECT_EQ(result, Result::Success);

  auto& rocketMsg = capturedMsg.get<rocket::RocketRequestMessage>();
  EXPECT_EQ(rocketMsg.frameType, frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(rocketMsg.requestHandle, 42u);
}

TEST(ThriftClientTransportAdapterTest, InboundResponseConvertedToThrift) {
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  auto* appAdapter = connection->appAdapter.get();

  folly::EventBase evb;
  MockHeadHandler rocketTail;
  TestAllocator rocketAllocator;

  auto rocketPipeline = PipelineBuilder<
                            rocket::client::RocketClientAppAdapter,
                            MockHeadHandler,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(appAdapter)
                            .setTail(&rocketTail)
                            .setAllocator(&rocketAllocator)
                            .build();

  appAdapter->setPipeline(rocketPipeline.get());
  connection->pipeline = std::move(rocketPipeline);

  ThriftClientTransportAdapter adapter(std::move(connection));

  MockHeadHandler thriftHead;
  TestAllocator thriftAllocator;

  auto thriftPipeline = PipelineBuilder<
                            MockHeadHandler,
                            ThriftClientTransportAdapter,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&thriftHead)
                            .setTail(&adapter)
                            .setAllocator(&thriftAllocator)
                            .build();

  adapter.setPipeline(thriftPipeline.get());

  auto responseBox =
      makeRocketResponseBox(42, frame::FrameType::REQUEST_RESPONSE);
  auto result = appAdapter->onMessage(std::move(responseBox));
  EXPECT_EQ(result, Result::Success);

  EXPECT_EQ(thriftHead.messageCount(), 1);
}

TEST(ThriftClientTransportAdapterTest, OnTransportErrorPropagatesException) {
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  auto* appAdapter = connection->appAdapter.get();

  folly::EventBase evb;
  MockHeadHandler rocketTail;
  TestAllocator rocketAllocator;

  auto rocketPipeline = PipelineBuilder<
                            rocket::client::RocketClientAppAdapter,
                            MockHeadHandler,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(appAdapter)
                            .setTail(&rocketTail)
                            .setAllocator(&rocketAllocator)
                            .build();

  appAdapter->setPipeline(rocketPipeline.get());
  connection->pipeline = std::move(rocketPipeline);

  ThriftClientTransportAdapter adapter(std::move(connection));

  MockHeadHandler thriftHead;
  TestAllocator thriftAllocator;

  auto thriftPipeline = PipelineBuilder<
                            MockHeadHandler,
                            ThriftClientTransportAdapter,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&thriftHead)
                            .setTail(&adapter)
                            .setAllocator(&thriftAllocator)
                            .build();

  adapter.setPipeline(thriftPipeline.get());

  adapter.onTransportError(
      folly::make_exception_wrapper<std::runtime_error>("connection lost"));

  EXPECT_EQ(thriftHead.exceptionCount(), 1);
}

TEST(ThriftClientTransportAdapterTest, OnExceptionIsNoOp) {
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  ThriftClientTransportAdapter adapter(std::move(connection));

  folly::EventBase evb;
  MockHeadHandler thriftHead;
  TestAllocator thriftAllocator;

  auto thriftPipeline = PipelineBuilder<
                            MockHeadHandler,
                            ThriftClientTransportAdapter,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&thriftHead)
                            .setTail(&adapter)
                            .setAllocator(&thriftAllocator)
                            .build();

  adapter.setPipeline(thriftPipeline.get());

  adapter.onException(
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  EXPECT_EQ(thriftHead.exceptionCount(), 0);
}

} // namespace apache::thrift::fast_thrift::thrift::client::test
