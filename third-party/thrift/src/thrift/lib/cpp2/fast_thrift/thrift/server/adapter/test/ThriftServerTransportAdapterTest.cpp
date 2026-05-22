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
 * Unit tests for ThriftServerTransportAdapter lifecycle propagation.
 *
 * The bridge sits between the rocket pipeline (where it subscribes to the
 * rocket app adapter's lifecycle callbacks) and the thrift pipeline (where
 * its own pipeline-hook fan-out lives). These tests validate that
 * lifecycle events flow correctly in BOTH directions and that the
 * connected_ guard absorbs the reentry that arises when one direction
 * triggers the other.
 */

#include <gtest/gtest.h>

#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/EndpointAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>

namespace apache::thrift::fast_thrift::thrift::server::test {

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

namespace {

// Fixture that wires two pipelines together via the bridge, matching the
// production server topology:
//
//   Rocket pipeline (head → tail):
//       MockHeadHandler → ... → RocketServerAppAdapter
//
//   Thrift pipeline (head → tail):
//       ThriftServerTransportAdapter (bridge) → ... → MockTailHandler
//
// The bridge subscribes to the rocket adapter's lifecycle on construction.
// Action callbacks count invocations into rocketAction.
struct RocketAction {
  int disconnect{0};
  int destroy{0};
};

struct BridgeFixture {
  folly::EventBase evb;
  MockHeadHandler rocketHead;
  TestAllocator rocketAllocator;
  TestAllocator thriftAllocator;
  rocket::server::RocketServerAppAdapter::Ptr rocketAdapter{
      new rocket::server::RocketServerAppAdapter()};
  // thriftTail must outlive the thrift pipeline; declare before the bridge
  // so it stays alive across teardown.
  MockTailHandler thriftTail;
  RocketAction rocketAction;
  std::unique_ptr<ThriftServerTransportAdapter> bridge;
  PipelineImpl::Ptr rocketPipeline;
  PipelineImpl::Ptr thriftPipeline;

  BridgeFixture() {
    rocketHead.setOnWriteCallback(
        [](TypeErasedBox&&) { return Result::Success; });

    rocketPipeline = PipelineBuilder<
                         MockHeadHandler,
                         rocket::server::RocketServerAppAdapter,
                         TestAllocator>()
                         .setEventBase(&evb)
                         .setHead(&rocketHead)
                         .setTail(rocketAdapter.get())
                         .setAllocator(&rocketAllocator)
                         .build();
    rocketAdapter->setPipeline(rocketPipeline.get());

    bridge = std::make_unique<ThriftServerTransportAdapter>(
        *rocketAdapter,
        /*onRocketDisconnect=*/[this]() noexcept { rocketAction.disconnect++; },
        /*onRocketDestroy=*/[this]() noexcept { rocketAction.destroy++; });

    thriftPipeline = PipelineBuilder<
                         ThriftServerTransportAdapter,
                         MockTailHandler,
                         TestAllocator>()
                         .setEventBase(&evb)
                         .setHead(bridge.get())
                         .setTail(&thriftTail)
                         .setAllocator(&thriftAllocator)
                         .build();
    bridge->setPipeline(thriftPipeline.get());
  }

  ~BridgeFixture() {
    // Mirror the production teardown order: thrift first, then rocket.
    // Tests that already closed the pipelines short-circuit via the
    // null/closed checks inside pipeline->close().
    if (thriftPipeline) {
      thriftPipeline->deactivate();
      thriftPipeline->close();
    }
    if (rocketPipeline) {
      rocketPipeline->deactivate();
      rocketPipeline->close();
    }
    bridge->resetPipeline();
    rocketAdapter->resetPipeline();
  }
};

} // namespace

TEST(
    ThriftServerTransportAdapterTest,
    RocketActivePropagatesToThriftPipelineActivate) {
  BridgeFixture fixture;

  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 0);

  // Activate the rocket pipeline (TransportHandler::onConnect normally
  // does this when the server accepts a connection). The rocket adapter
  // fires onConnect_ on its lifecycle subscription, which the bridge
  // translates into thrift pipeline activate.
  fixture.rocketPipeline->activate();

  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 1);

  // Re-activating rocket must NOT re-fire the thrift active path
  // because of the bridge's connected_ guard.
  fixture.rocketPipeline->activate();
  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 1);
}

TEST(
    ThriftServerTransportAdapterTest,
    RocketInactivePropagatesToThriftPipelineDeactivate) {
  BridgeFixture fixture;
  fixture.rocketPipeline->activate();
  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 1);
  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 0);

  // Rocket side goes inactive (socket closed). The bridge translates
  // this into thrift pipeline deactivate.
  fixture.rocketPipeline->deactivate();

  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 1);
}

TEST(
    ThriftServerTransportAdapterTest,
    ThriftDeactivateInvokesRocketDisconnectCallback) {
  BridgeFixture fixture;
  fixture.rocketPipeline->activate();
  EXPECT_EQ(fixture.rocketAction.disconnect, 0);

  // Thrift side initiates: deactivate the thrift pipeline. The bridge's
  // onPipelineInactive must invoke the owner-supplied onRocketDisconnect
  // callback so the rocket side can tear down too.
  fixture.thriftPipeline->deactivate();

  EXPECT_EQ(fixture.rocketAction.disconnect, 1);
}

TEST(
    ThriftServerTransportAdapterTest,
    ThriftHandlerRemovedInvokesRocketDestroyCallback) {
  BridgeFixture fixture;
  fixture.rocketPipeline->activate();
  // deactivate so handlerRemoved's DCHECK on !connected_ is satisfied.
  fixture.thriftPipeline->deactivate();
  EXPECT_EQ(fixture.rocketAction.destroy, 0);

  // Close the thrift pipeline → handlerRemoved fan-out → bridge's
  // handlerRemoved invokes the destroy callback.
  fixture.thriftPipeline->close();
  fixture.thriftPipeline.reset();

  EXPECT_EQ(fixture.rocketAction.destroy, 1);
}

TEST(
    ThriftServerTransportAdapterTest,
    RocketCloseSubscriptionNullsOutActionCallbacks) {
  BridgeFixture fixture;
  fixture.rocketPipeline->activate();

  // Closing the rocket pipeline runs handlerRemoved on the rocket
  // adapter, which fires the bridge's onClose subscription. The bridge
  // nulls out its action callbacks so any later invocation (e.g. from
  // the bridge's own destruction path) is a no-op rather than a UAF
  // through dangling rocket-side pointers.
  fixture.rocketPipeline->close();
  fixture.rocketPipeline.reset();

  // Now deactivate the thrift pipeline. The bridge would normally fire
  // onRocketDisconnect_, but it's been nulled out — counter stays at 0.
  fixture.thriftPipeline->deactivate();
  EXPECT_EQ(fixture.rocketAction.disconnect, 0);
}

TEST(ThriftServerTransportAdapterTest, ReactivateAfterDeactivateCyclesCleanly) {
  BridgeFixture fixture;

  // First activate.
  fixture.rocketPipeline->activate();
  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 1);
  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 0);

  // Deactivate; thrift side observes.
  fixture.rocketPipeline->deactivate();
  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 1);

  // Re-activate — the bridge's connected_ flag was cleared by the
  // deactivate path, so the second activate must propagate again.
  fixture.rocketPipeline->activate();
  EXPECT_EQ(fixture.thriftTail.pipelineActiveCount(), 2);

  // And deactivate again.
  fixture.rocketPipeline->deactivate();
  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 2);
}

TEST(
    ThriftServerTransportAdapterTest,
    DtorWithoutHandlerRemovedInvokesDestroyCallbackDefensively) {
  // Build a bridge without ever wiring it into a thrift pipeline, then
  // let it go out of scope. The destroy callback must still fire (so
  // the rocket side gets torn down even if the bridge was destroyed
  // without the normal handlerRemoved teardown path running first).
  rocket::server::RocketServerAppAdapter::Ptr rocketAdapter(
      new rocket::server::RocketServerAppAdapter());
  int destroyCount = 0;
  {
    ThriftServerTransportAdapter bridge(
        *rocketAdapter,
        /*onRocketDisconnect=*/[]() noexcept {},
        /*onRocketDestroy=*/[&]() noexcept { destroyCount++; });
    // No setPipeline, no handlerRemoved.
  }
  EXPECT_EQ(destroyCount, 1);
}

TEST(
    ThriftServerTransportAdapterTest,
    DtorAfterHandlerRemovedDoesNotDoubleInvokeDestroy) {
  // Normal teardown path: handlerRemoved fires, consuming the destroy
  // callback by moving it out. The dtor must NOT re-invoke it.
  // External counter so it outlives the bridge.
  int destroyCount = 0;
  {
    rocket::server::RocketServerAppAdapter::Ptr rocketAdapter(
        new rocket::server::RocketServerAppAdapter());
    auto bridge = std::make_unique<ThriftServerTransportAdapter>(
        *rocketAdapter,
        /*onRocketDisconnect=*/[]() noexcept {},
        /*onRocketDestroy=*/[&]() noexcept { destroyCount++; });

    // Simulate the normal teardown path: handlerRemoved fires once.
    bridge->handlerRemoved();
    EXPECT_EQ(destroyCount, 1);

    // Now the bridge goes out of scope — the dtor's defensive
    // onRocketDestroy_ check must observe the moved-out callback and
    // skip the second invocation.
  }
  EXPECT_EQ(destroyCount, 1);
}

TEST(
    ThriftServerTransportAdapterTest,
    ReentryFromRocketToThriftToRocketDoesNotLoop) {
  BridgeFixture fixture;
  fixture.rocketPipeline->activate();
  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 0);
  EXPECT_EQ(fixture.rocketAction.disconnect, 0);

  // Rocket deactivates → bridge propagates to thrift pipeline (fires
  // onPipelineInactive on thriftTail). The thrift pipeline deactivate
  // also fires onPipelineInactive on the bridge itself, which would
  // normally invoke onRocketDisconnect_ — but the connected_ guard was
  // already cleared in the rocket→thrift propagation path, so the
  // callback does NOT fire a second time. No infinite loop.
  fixture.rocketPipeline->deactivate();

  EXPECT_EQ(fixture.thriftTail.pipelineInactiveCount(), 1);
  EXPECT_EQ(fixture.rocketAction.disconnect, 0);
}

} // namespace apache::thrift::fast_thrift::thrift::server::test
