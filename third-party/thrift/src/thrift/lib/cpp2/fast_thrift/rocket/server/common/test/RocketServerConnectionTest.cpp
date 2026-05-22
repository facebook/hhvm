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
 * Unit tests for the RocketServerConnection lifecycle split
 * (setLifecycleHandlers, disconnect, destroy, legacy close).
 *
 * Builds a minimal rocket pipeline (TransportHandler ↔ RocketServerAppAdapter)
 * over an in-memory transport so the connection's internal teardown ordering
 * can be exercised without sockets.
 */

#include <gtest/gtest.h>

#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/test/TestAsyncTransport.h>

namespace apache::thrift::fast_thrift::rocket::server::test {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::SimpleBufferAllocator;
using transport::test::TestAsyncTransport;

namespace {

// Wraps a connection with the minimum machinery needed to exercise
// disconnect/destroy: a TestAsyncTransport-backed TransportHandler and a
// trivial rocket pipeline. The pipeline has no rocket handlers between
// the transport and the app adapter — we only care about the connection's
// teardown bookkeeping, not the rocket framing layer.
struct ConnectionFixture {
  folly::EventBase evb;
  RocketServerConnection conn;

  ConnectionFixture() {
    auto socket =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb));
    auto transportHandler =
        transport::TransportHandler::create(std::move(socket));

    auto pipeline = PipelineBuilder<
                        transport::TransportHandler,
                        RocketServerAppAdapter,
                        SimpleBufferAllocator>()
                        .setEventBase(&evb)
                        .setHead(transportHandler.get())
                        .setTail(conn.appAdapter.get())
                        .setAllocator(&conn.allocator)
                        .build();

    transportHandler->setPipeline(pipeline.get());
    conn.appAdapter->setPipeline(pipeline.get());

    conn.transportHandler = std::move(transportHandler);
    conn.pipeline = std::move(pipeline);
  }
};

} // namespace

TEST(RocketServerConnectionTest, SetLifecycleHandlersForwardsToAppAdapter) {
  ConnectionFixture f;
  int connectCount = 0;
  int disconnectCount = 0;

  f.conn.setLifecycleHandlers(
      [&]() noexcept { connectCount++; },
      [&]() noexcept { disconnectCount++; });

  // setLifecycleHandlers forwards via appAdapter->setLifecycleHandlers,
  // so firing pipeline lifecycle on the rocket adapter must reach the
  // connection-level callbacks.
  f.conn.appAdapter->onPipelineActive();
  EXPECT_EQ(connectCount, 1);
  EXPECT_EQ(disconnectCount, 0);

  f.conn.appAdapter->onPipelineInactive();
  EXPECT_EQ(connectCount, 1);
  EXPECT_EQ(disconnectCount, 1);
}

TEST(RocketServerConnectionTest, DisconnectIsIdempotent) {
  ConnectionFixture f;
  f.conn.pipeline->activate();

  // First disconnect tears the socket down.
  f.conn.disconnect();
  EXPECT_TRUE(f.conn.disconnected_);
  EXPECT_NE(f.conn.transportHandler, nullptr);

  // Second disconnect must be a no-op (no crash).
  f.conn.disconnect();
  EXPECT_TRUE(f.conn.disconnected_);

  // Cleanup so the fixture's implicit destruction doesn't trip
  // ordering checks.
  f.conn.destroy();
}

TEST(RocketServerConnectionTest, DisconnectClearsCloseCallbackToBreakReentry) {
  ConnectionFixture f;
  int closeCallbackInvocations = 0;
  f.conn.transportHandler->setCloseCallback(
      [&]() { closeCallbackInvocations++; });

  f.conn.pipeline->activate();
  f.conn.disconnect();

  // The connection's disconnect() must null out the close callback BEFORE
  // calling transportHandler->close() — otherwise close would fire the
  // callback, which on the production path calls back into
  // RocketServerConnection::close → disconnect → close again, looping.
  EXPECT_EQ(closeCallbackInvocations, 0);

  f.conn.destroy();
}

TEST(RocketServerConnectionTest, DestroyAfterDisconnectReleasesEverything) {
  ConnectionFixture f;
  f.conn.pipeline->activate();
  f.conn.disconnect();

  f.conn.destroy();

  // All owning unique_ptrs are released after destroy().
  EXPECT_EQ(f.conn.transportHandler, nullptr);
  EXPECT_EQ(f.conn.pipeline, nullptr);
  EXPECT_EQ(f.conn.appAdapter, nullptr);
}

TEST(RocketServerConnectionTest, DestroyImpliesDisconnect) {
  ConnectionFixture f;
  f.conn.pipeline->activate();

  // Call destroy() without an explicit prior disconnect().
  f.conn.destroy();

  EXPECT_TRUE(f.conn.disconnected_);
  EXPECT_EQ(f.conn.transportHandler, nullptr);
  EXPECT_EQ(f.conn.pipeline, nullptr);
  EXPECT_EQ(f.conn.appAdapter, nullptr);
}

TEST(RocketServerConnectionTest, DestroyIsIdempotent) {
  ConnectionFixture f;
  f.conn.pipeline->activate();
  f.conn.destroy();

  // Second destroy() on the now-empty connection must not crash.
  f.conn.destroy();
  EXPECT_EQ(f.conn.transportHandler, nullptr);
}

TEST(
    RocketServerConnectionTest,
    DestroyResetsAppAdapterPipelineBeforeClosingPipeline) {
  // Regression test: a prior version of destroy() called pipeline->close()
  // before appAdapter->resetPipeline(). The pipeline's handlerRemoved
  // fan-out then reached the adapter with its pipeline_ pointer still
  // live. The onClose callback below probes the adapter's hasPipeline()
  // state at the moment of fan-out — it must already be false.
  ConnectionFixture f;
  bool adapterDetachedAtClose = false;
  // Raw pointer because the adapter unique_ptr is reset during destroy()
  // before this scope ends; a captured Ptr would dangle.
  auto* adapterRaw = f.conn.appAdapter.get();
  f.conn.appAdapter->setLifecycleHandlers(
      []() noexcept {},
      []() noexcept {},
      [&]() noexcept {
        adapterDetachedAtClose = (adapterRaw->getPipeline() == nullptr);
      });

  f.conn.pipeline->activate();
  f.conn.destroy();

  EXPECT_TRUE(adapterDetachedAtClose);
}

TEST(RocketServerConnectionTest, LegacyCloseComposesDisconnectAndDestroy) {
  ConnectionFixture f;
  f.conn.pipeline->activate();

  f.conn.close(folly::exception_wrapper{});

  EXPECT_TRUE(f.conn.disconnected_);
  EXPECT_EQ(f.conn.transportHandler, nullptr);
  EXPECT_EQ(f.conn.pipeline, nullptr);
  EXPECT_EQ(f.conn.appAdapter, nullptr);
}

} // namespace apache::thrift::fast_thrift::rocket::server::test
