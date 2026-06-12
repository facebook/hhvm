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

#include <array>

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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/Event.h>
#include <thrift/lib/cpp2/util/ManagedStringView.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::test {

namespace transport = apache::thrift::fast_thrift::transport;

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
    channel_pipeline::HeadEndpointHandler<ThriftClientTransportAdapter>,
    "ThriftClientTransportAdapter must satisfy HeadEndpointHandler");

// =============================================================================
// Helpers
// =============================================================================

namespace {

TypeErasedBox makeThriftRequestBox() {
  auto metadata = std::make_unique<apache::thrift::RequestRpcMetadata>();
  metadata->protocol() = apache::thrift::ProtocolId::BINARY;
  metadata->kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata->name() = apache::thrift::ManagedStringViewWithConversions(
      apache::thrift::ManagedStringView::from_static(std::string_view{"test"}));
  ThriftRequestMessage msg{
      .payload =
          ThriftRequestResponsePayload{
              .data = folly::IOBuf::copyBuffer("data"),
              .metadata = std::move(metadata),
          },
      .requestContext = rocket::borrow(reinterpret_cast<void*>(0x42)),
  };
  return erase_and_box(std::move(msg));
}

TypeErasedBox makeRocketResponseBox(
    void* requestContext = reinterpret_cast<void*>(0x42),
    frame::FrameType streamType = frame::FrameType::REQUEST_RESPONSE) {
  auto data = folly::IOBuf::copyBuffer("response-data");
  auto frameBuf = frame::write::serialize(
      frame::write::PayloadHeader{
          .streamId = 1, .follows = false, .complete = true, .next = true},
      nullptr,
      std::move(data));

  rocket::RocketResponseMessage response{
      .payload = frame::read::parseFrame(std::move(frameBuf)),
      .requestContext = rocket::borrow(requestContext),
      .streamType = streamType,
  };
  return erase_and_box(std::move(response));
}

TypeErasedBox makeRocketErrorResponseBox(
    folly::exception_wrapper ew,
    void* requestContext = reinterpret_cast<void*>(0x42),
    uint32_t streamId = 1,
    frame::FrameType streamType = frame::FrameType::REQUEST_RESPONSE) {
  rocket::RocketResponseMessage response;
  response.payload = rocket::RocketResponseError{
      .ew = std::move(ew),
      .streamId = streamId,
  };
  response.requestContext = rocket::borrow(requestContext);
  response.streamType = streamType;
  return erase_and_box(std::move(response));
}

struct AdapterWithRocketPipeline {
  folly::EventBase evb;
  MockHeadHandler rocketHead;
  TestAllocator allocator;
  std::unique_ptr<ThriftClientTransportAdapter> adapter;

  AdapterWithRocketPipeline() {
    auto connection =
        std::make_unique<rocket::client::RocketClientConnection>();
    auto* appAdapter = connection->appAdapter.get();

    rocketHead.setOnWriteCallback(
        [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });

    auto rocketPipeline = PipelineBuilder<
                              MockHeadHandler,
                              rocket::client::RocketClientAppAdapter,
                              TestAllocator>()
                              .setEventBase(&evb)
                              .setHead(&rocketHead)
                              .setTail(appAdapter)
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

TEST(ThriftClientTransportAdapterTest, OnWriteConvertsAndWritesToRocket) {
  AdapterWithRocketPipeline fixture;

  auto result = fixture.adapter->onWrite(makeThriftRequestBox());
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(fixture.rocketHead.writeCount(), 1);
}

TEST(ThriftClientTransportAdapterTest, OnWriteConvertsRpcKindToFrameType) {
  AdapterWithRocketPipeline fixture;
  TypeErasedBox capturedMsg;

  fixture.rocketHead.setOnWriteCallback([&](TypeErasedBox&& msg) {
    capturedMsg = std::move(msg);
    return Result::Success;
  });

  auto result = fixture.adapter->onWrite(makeThriftRequestBox());
  EXPECT_EQ(result, Result::Success);

  auto& rocketMsg = capturedMsg.get<rocket::RocketRequestMessage>();
  EXPECT_EQ(rocketMsg.streamType, frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(rocketMsg.requestContext.get(), reinterpret_cast<void*>(0x42));
}

TEST(ThriftClientTransportAdapterTest, InboundResponseConvertedToThrift) {
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  auto* appAdapter = connection->appAdapter.get();

  folly::EventBase evb;
  MockHeadHandler rocketHead;
  rocketHead.setOnWriteCallback(
      [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });
  TestAllocator rocketAllocator;

  // Declared before `adapter` so they outlive the bridge's deferred thrift
  // pipeline destruction (the bridge holds a guard on the thrift pipeline).
  MockTailHandler thriftTail;
  TestAllocator thriftAllocator;

  auto rocketPipeline = PipelineBuilder<
                            MockHeadHandler,
                            rocket::client::RocketClientAppAdapter,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&rocketHead)
                            .setTail(appAdapter)
                            .setAllocator(&rocketAllocator)
                            .build();

  appAdapter->setPipeline(rocketPipeline.get());
  connection->pipeline = std::move(rocketPipeline);

  ThriftClientTransportAdapter adapter(std::move(connection));

  auto thriftPipeline = PipelineBuilder<
                            ThriftClientTransportAdapter,
                            MockTailHandler,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&adapter)
                            .setTail(&thriftTail)
                            .setAllocator(&thriftAllocator)
                            .build();

  adapter.setPipeline(thriftPipeline.get());

  auto responseBox = makeRocketResponseBox(
      reinterpret_cast<void*>(0x42), frame::FrameType::REQUEST_RESPONSE);
  auto result = appAdapter->onRead(std::move(responseBox));
  EXPECT_EQ(result, Result::Success);

  EXPECT_EQ(thriftTail.readCount(), 1);
}

TEST(
    ThriftClientTransportAdapterTest,
    OnTransportResponseWithErrorRoutesAsThriftClientResponseErrorViaFireRead) {
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  auto* appAdapter = connection->appAdapter.get();

  folly::EventBase evb;
  MockHeadHandler rocketHead;
  rocketHead.setOnWriteCallback(
      [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });
  TestAllocator rocketAllocator;

  // Declared before `adapter` so they outlive the bridge's deferred thrift
  // pipeline destruction (the bridge holds a guard on the thrift pipeline).
  MockTailHandler thriftTail;
  TestAllocator thriftAllocator;

  auto rocketPipeline = PipelineBuilder<
                            MockHeadHandler,
                            rocket::client::RocketClientAppAdapter,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&rocketHead)
                            .setTail(appAdapter)
                            .setAllocator(&rocketAllocator)
                            .build();

  appAdapter->setPipeline(rocketPipeline.get());
  connection->pipeline = std::move(rocketPipeline);

  ThriftClientTransportAdapter adapter(std::move(connection));

  auto thriftPipeline = PipelineBuilder<
                            ThriftClientTransportAdapter,
                            MockTailHandler,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&adapter)
                            .setTail(&thriftTail)
                            .setAllocator(&thriftAllocator)
                            .build();

  adapter.setPipeline(thriftPipeline.get());

  // In-process per-request error from below arrives as a RocketResponseMessage
  // with the RocketResponseError variant. The transport adapter translates it
  // to a ThriftResponseMessage with the ThriftClientResponseError variant and
  // routes it via fireRead so the tail can fail just this one callback — the
  // channel stays Open (no fireException).
  auto errorBox = makeRocketErrorResponseBox(
      folly::make_exception_wrapper<std::runtime_error>("serialize boom"));
  auto result = appAdapter->onRead(std::move(errorBox));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(thriftTail.readCount(), 1);
  EXPECT_EQ(thriftTail.exceptionCount(), 0);
}

TEST(ThriftClientTransportAdapterTest, OnTransportErrorPropagatesException) {
  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  auto* appAdapter = connection->appAdapter.get();

  folly::EventBase evb;
  MockHeadHandler rocketHead;
  rocketHead.setOnWriteCallback(
      [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });
  TestAllocator rocketAllocator;

  // Declared before `adapter` so they outlive the bridge's deferred thrift
  // pipeline destruction (the bridge holds a guard on the thrift pipeline).
  MockTailHandler thriftTail;
  TestAllocator thriftAllocator;

  auto rocketPipeline = PipelineBuilder<
                            MockHeadHandler,
                            rocket::client::RocketClientAppAdapter,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&rocketHead)
                            .setTail(appAdapter)
                            .setAllocator(&rocketAllocator)
                            .build();

  appAdapter->setPipeline(rocketPipeline.get());
  connection->pipeline = std::move(rocketPipeline);

  ThriftClientTransportAdapter adapter(std::move(connection));

  auto thriftPipeline = PipelineBuilder<
                            ThriftClientTransportAdapter,
                            MockTailHandler,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&adapter)
                            .setTail(&thriftTail)
                            .setAllocator(&thriftAllocator)
                            .build();

  adapter.setPipeline(thriftPipeline.get());

  adapter.onTransportError(
      folly::make_exception_wrapper<std::runtime_error>("connection lost"));

  EXPECT_EQ(thriftTail.exceptionCount(), 1);
}

// =============================================================================
// Cross-pipeline lifecycle wiring
// =============================================================================

namespace {

// Counts lifecycle callbacks fired on the rocket pipeline so we can
// observe how the bridge propagates between the two pipelines.
struct RocketLifecycleCounter {
  int active{0};
  int inactive{0};
};

struct AdapterWithBothPipelines {
  folly::EventBase evb;
  MockHeadHandler rocketHead;
  TestAllocator rocketAllocator;
  TestAllocator thriftAllocator;
  // thriftTail must be declared BEFORE bridge: the bridge holds a guard
  // on the thrift pipeline, so pipeline destruction is deferred to bridge
  // destruction. The tail handler must outlive that deferred destruction.
  MockTailHandler thriftTail;
  // Non-owning pointers retained for test-side reach-through into the
  // rocket pipeline (otherwise hidden inside the bridge's connection_).
  rocket::client::RocketClientAppAdapter* rocketAppAdapter{nullptr};
  PipelineImpl* rocketPipelineRaw{nullptr};
  std::unique_ptr<ThriftClientTransportAdapter> bridge;
  PipelineImpl::Ptr thriftPipeline;

  AdapterWithBothPipelines() {
    auto connection =
        std::make_unique<rocket::client::RocketClientConnection>();
    rocketAppAdapter = connection->appAdapter.get();

    rocketHead.setOnWriteCallback(
        [](channel_pipeline::TypeErasedBox&&) { return Result::Success; });

    auto rocketPipeline = PipelineBuilder<
                              MockHeadHandler,
                              rocket::client::RocketClientAppAdapter,
                              TestAllocator>()
                              .setEventBase(&evb)
                              .setHead(&rocketHead)
                              .setTail(rocketAppAdapter)
                              .setAllocator(&rocketAllocator)
                              .build();

    rocketAppAdapter->setPipeline(rocketPipeline.get());
    rocketPipelineRaw = rocketPipeline.get();
    connection->pipeline = std::move(rocketPipeline);

    bridge =
        std::make_unique<ThriftClientTransportAdapter>(std::move(connection));

    thriftPipeline = PipelineBuilder<
                         ThriftClientTransportAdapter,
                         MockTailHandler,
                         TestAllocator>()
                         .setEventBase(&evb)
                         .setHead(bridge.get())
                         .setTail(&thriftTail)
                         .setAllocator(&thriftAllocator)
                         .build();

    bridge->setPipeline(thriftPipeline.get());
  }

  ~AdapterWithBothPipelines() {
    // If the test already reset thriftPipeline, the bridge's handlerRemoved
    // will have run connection->destroy(), which already destroyed the
    // rocket pipeline. Touching rocketPipelineRaw would be a UAF.
    if (!thriftPipeline) {
      return;
    }
    rocketPipelineRaw->deactivate();
    thriftPipeline->deactivate();
  }
};

} // namespace

TEST(
    ThriftClientTransportAdapterTest,
    ThriftPipelineDeactivateDisconnectsRocketConnection) {
  AdapterWithBothPipelines fixture;

  // Thrift side initiates: deactivate the thrift pipeline. Bridge head's
  // onPipelineInactive must call connection_->disconnect(), which in turn
  // deactivates the rocket pipeline. deactivate() is a no-op unless the
  // pipeline is Active, so activate first.
  fixture.rocketPipelineRaw->activate();
  fixture.thriftPipeline->activate();
  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 0);

  fixture.thriftPipeline->deactivate();

  // Tail handler of thrift pipeline saw inactive exactly once (proves the
  // deactivate fan-out ran). The rocket-side tail's onPipelineInactive
  // would have tried to re-deactivate the thrift pipeline, but the
  // bridge's inactivated_ flag absorbs the loop.
  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 1);
}

TEST(ThriftClientTransportAdapterTest, HandlerRemovedDestroysRocketConnection) {
  AdapterWithBothPipelines fixture;

  // Closing the thrift pipeline runs handlerRemoved on every handler.
  // The bridge's handlerRemoved must destroy the rocket connection
  // (which closes the rocket pipeline + drops the transport handler).
  // Validate that close completes without crashing.
  fixture.thriftPipeline->close();
  fixture.thriftPipeline.reset();
  // Reaching here without crash is the test — the bridge tore the rocket
  // side down cleanly.
}

TEST(ThriftClientTransportAdapterTest, RocketActivePropagatesToThriftPipeline) {
  AdapterWithBothPipelines fixture;

  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 0);

  // Activate the rocket pipeline (TransportHandler::onConnect normally
  // does this when the socket connects). The bridge's onActive_ lambda
  // must propagate the activation to the thrift pipeline.
  fixture.rocketPipelineRaw->activate();

  // Thrift tail saw onPipelineActive exactly once (proves bridge's
  // propagateActive ran and called thrift pipeline activate).
  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 1);

  // Re-activating rocket should NOT re-fire the thrift active path
  // because of the bridge's activated_ guard.
  fixture.rocketPipelineRaw->activate();
  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 1);
}

TEST(
    ThriftClientTransportAdapterTest, OnWriteReadyFromRocketReachesThriftTail) {
  AdapterWithBothPipelines fixture;

  // Simulate rocket-tail onWriteReady (fired when rocket transport's write
  // buffer drains). The bridge must walk the thrift pipeline and notify
  // the thrift tail.
  fixture.rocketAppAdapter->onWriteReady();

  EXPECT_EQ(fixture.thriftTail.onWriteReadyCount(), 1);
}

TEST(ThriftClientTransportAdapterTest, OnReadReadyFromThriftReachesRocketHead) {
  AdapterWithBothPipelines fixture;

  // Simulate the thrift pipeline firing onReadReady. The bridge head must
  // forward it down into the rocket pipeline and reach the rocket head.
  fixture.thriftPipeline->onReadReady();

  EXPECT_EQ(fixture.rocketHead.onReadReadyCount(), 1);
}

// =============================================================================
// Write-completion relay
// =============================================================================

namespace {

// Thrift-pipeline tail that subscribes to the WriteComplete event and records
// the relayed payload — models the consumer the bridge fires toward.
struct WriteCompleteCapturingTail {
  Result onRead(TypeErasedBox&&) noexcept { return Result::Success; }
  void onException(folly::exception_wrapper&&) noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  static constexpr std::array<ThriftClientEventType, 1> kSubscribedEvents{
      ThriftClientEventType::WriteComplete};

  void onEvent(
      ThriftClientEventType /*ev*/, const TypeErasedBox& box) noexcept {
    events.push_back(box.get<ThriftClientWriteCompleteEvent>());
  }

  std::vector<ThriftClientWriteCompleteEvent> events;
};

} // namespace

TEST(
    ThriftClientTransportAdapterTest,
    RocketWriteCompleteRelaysToThriftWriteCompleteEvent) {
  folly::EventBase evb;
  MockHeadHandler rocketHead;
  rocketHead.setOnWriteCallback(
      [](TypeErasedBox&&) { return Result::Success; });
  TestAllocator rocketAllocator;
  TestAllocator thriftAllocator;
  WriteCompleteCapturingTail thriftTail;

  auto connection = std::make_unique<rocket::client::RocketClientConnection>();
  auto* rocketAppAdapter = connection->appAdapter.get();
  auto rocketPipeline = PipelineBuilder<
                            MockHeadHandler,
                            rocket::client::RocketClientAppAdapter,
                            TestAllocator>()
                            .setEventBase(&evb)
                            .setHead(&rocketHead)
                            .setTail(rocketAppAdapter)
                            .setAllocator(&rocketAllocator)
                            .build();
  rocketAppAdapter->setPipeline(rocketPipeline.get());
  connection->pipeline = std::move(rocketPipeline);

  auto bridge =
      std::make_unique<ThriftClientTransportAdapter>(std::move(connection));

  // Thrift pipeline built with ThriftClientEventType so the tail's
  // WriteComplete subscription is wired.
  auto thriftPipeline = PipelineBuilder<
                            ThriftClientTransportAdapter,
                            WriteCompleteCapturingTail,
                            TestAllocator,
                            ThriftClientEventType>()
                            .setEventBase(&evb)
                            .setHead(bridge.get())
                            .setTail(&thriftTail)
                            .setAllocator(&thriftAllocator)
                            .build();
  bridge->setPipeline(thriftPipeline.get());

  // Simulate the rocket pipeline delivering an enriched write-completion to
  // the rocket app adapter; the bridge's callback relays it up as a thrift
  // WriteComplete event.
  rocketAppAdapter->onEvent(
      rocket::client::RocketClientEventId::RocketWriteComplete,
      TypeErasedBox(
          rocket::client::RocketWriteCompleteEvent{
              .status = transport::WriteCompletionStatus::Success,
              .frameCount = 4,
              .bytes = 256,
          }));

  ASSERT_EQ(thriftTail.events.size(), 1u);
  EXPECT_EQ(
      thriftTail.events[0].status, transport::WriteCompletionStatus::Success);
  EXPECT_EQ(thriftTail.events[0].frameCount, 4u);
  EXPECT_EQ(thriftTail.events[0].bytes, 256u);

  thriftPipeline->deactivate();
  thriftPipeline->close();
  bridge.reset();
}

} // namespace apache::thrift::fast_thrift::thrift::client::test
