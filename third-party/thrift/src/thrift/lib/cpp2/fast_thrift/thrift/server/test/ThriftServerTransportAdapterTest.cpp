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
 * Unit tests for ThriftServerTransportAdapter.
 *
 * Tests the adapter in isolation: concept satisfaction, inbound conversion
 * (RocketRequestMessage → ThriftServerRequestMessage), and outbound conversion
 * (ThriftServerResponseMessage → RocketResponseMessage).
 */

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

namespace apache::thrift::fast_thrift::thrift::server::test {

using channel_pipeline::erase_and_box;
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
    channel_pipeline::HeadEndpointHandler<ThriftServerTransportAdapter>,
    "ThriftServerTransportAdapter must satisfy HeadEndpointHandler");

// =============================================================================
// Helpers
// =============================================================================

namespace {

TypeErasedBox makeRocketRequestBox(uint32_t streamId = 1) {
  auto data = folly::IOBuf::copyBuffer("request-data");
  auto frameBuf = frame::write::serialize(
      frame::write::RequestResponseHeader{
          .streamId = streamId, .follows = false},
      nullptr,
      std::move(data));

  rocket::server::RocketRequestMessage request{
      .frame = frame::read::parseFrame(std::move(frameBuf)),
      .error = {},
      .streamId = streamId,
  };
  return erase_and_box(std::move(request));
}

TypeErasedBox makeThriftResponseBox(uint32_t streamId = 1) {
  ThriftServerResponseMessage msg{
      .payload =
          ThriftServerResponsePayload{
              .data = folly::IOBuf::copyBuffer("response-data"),
              .metadata = folly::IOBuf::copyBuffer("response-meta"),
              .complete = true,
          },
      .streamId = streamId,
      .errorCode = 0,
  };
  return erase_and_box(std::move(msg));
}

struct AdapterWithRocketPipeline {
  folly::EventBase evb;
  MockHeadHandler rocketHead;
  TestAllocator allocator;
  rocket::server::RocketServerAppAdapter::Ptr appAdapter{
      new rocket::server::RocketServerAppAdapter()};
  std::unique_ptr<ThriftServerTransportAdapter> adapter;

  AdapterWithRocketPipeline() {
    adapter = std::make_unique<ThriftServerTransportAdapter>(*appAdapter);

    rocketHead.setOnWriteCallback(
        [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });

    auto rocketPipeline = PipelineBuilder<
                              MockHeadHandler,
                              rocket::server::RocketServerAppAdapter,
                              TestAllocator>()
                              .setEventBase(&evb)
                              .setHead(&rocketHead)
                              .setTail(appAdapter.get())
                              .setAllocator(&allocator)
                              .build();

    appAdapter->setPipeline(rocketPipeline.get());
    rocketPipeline_ = std::move(rocketPipeline);
  }

 private:
  PipelineImpl::Ptr rocketPipeline_;
};

} // namespace

// =============================================================================
// Unit tests
// =============================================================================

TEST(ThriftServerTransportAdapterTest, OnWriteConvertsAndWritesToRocket) {
  AdapterWithRocketPipeline fixture;

  auto result = fixture.adapter->onWrite(makeThriftResponseBox());
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(fixture.rocketHead.writeCount(), 1);
}

TEST(ThriftServerTransportAdapterTest, OnWriteConvertsResponseFields) {
  AdapterWithRocketPipeline fixture;
  TypeErasedBox capturedMsg;

  fixture.rocketHead.setOnWriteCallback([&](TypeErasedBox&& msg) {
    capturedMsg = std::move(msg);
    return Result::Success;
  });

  auto result = fixture.adapter->onWrite(makeThriftResponseBox(42));
  EXPECT_EQ(result, Result::Success);

  auto& rocketMsg = capturedMsg.get<rocket::server::RocketResponseMessage>();
  EXPECT_EQ(rocketMsg.streamId, 42u);
  EXPECT_TRUE(rocketMsg.complete);
  EXPECT_EQ(rocketMsg.errorCode, 0u);
}

TEST(ThriftServerTransportAdapterTest, InboundRequestConvertedToThrift) {
  AdapterWithRocketPipeline fixture;

  MockTailHandler thriftTail;
  TestAllocator thriftAllocator;

  auto thriftPipeline = PipelineBuilder<
                            ThriftServerTransportAdapter,
                            MockTailHandler,
                            TestAllocator>()
                            .setEventBase(&fixture.evb)
                            .setHead(fixture.adapter.get())
                            .setTail(&thriftTail)
                            .setAllocator(&thriftAllocator)
                            .build();

  fixture.adapter->setPipeline(thriftPipeline.get());

  auto requestBox = makeRocketRequestBox(7);
  auto result = fixture.appAdapter->onRead(std::move(requestBox));
  EXPECT_EQ(result, Result::Success);

  EXPECT_EQ(thriftTail.readCount(), 1);
}

TEST(ThriftServerTransportAdapterTest, OnTransportErrorPropagatesException) {
  AdapterWithRocketPipeline fixture;

  MockTailHandler thriftTail;
  TestAllocator thriftAllocator;

  auto thriftPipeline = PipelineBuilder<
                            ThriftServerTransportAdapter,
                            MockTailHandler,
                            TestAllocator>()
                            .setEventBase(&fixture.evb)
                            .setHead(fixture.adapter.get())
                            .setTail(&thriftTail)
                            .setAllocator(&thriftAllocator)
                            .build();

  fixture.adapter->setPipeline(thriftPipeline.get());

  fixture.adapter->onTransportError(
      folly::make_exception_wrapper<std::runtime_error>("connection lost"));

  EXPECT_EQ(thriftTail.exceptionCount(), 1);
}

TEST(ThriftServerTransportAdapterTest, OnExceptionIsNoOp) {
  rocket::server::RocketServerAppAdapter::Ptr appAdapter(
      new rocket::server::RocketServerAppAdapter());
  ThriftServerTransportAdapter adapter(*appAdapter);

  folly::EventBase evb;
  MockTailHandler thriftTail;
  TestAllocator thriftAllocator;

  auto thriftPipeline = PipelineBuilder<
                            ThriftServerTransportAdapter,
                            MockTailHandler,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&adapter)
                            .setTail(&thriftTail)
                            .setAllocator(&thriftAllocator)
                            .build();

  adapter.setPipeline(thriftPipeline.get());

  adapter.onException(
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  EXPECT_EQ(thriftTail.exceptionCount(), 0);
}

} // namespace apache::thrift::fast_thrift::thrift::server::test
