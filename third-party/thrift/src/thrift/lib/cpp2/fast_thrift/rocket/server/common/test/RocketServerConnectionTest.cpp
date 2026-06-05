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

TEST(RocketServerConnectionTest, DestroyDeactivatesPipelineBeforeClose) {
  // Regression test: destroy() must deactivate the pipeline before closing
  // it. The app adapter's handlerRemoved (fired by pipeline->close()) asserts
  // disconnected_ — it must already have observed onPipelineInactive.
  // Owner-initiated teardown that bypasses the socket lifecycle would
  // otherwise reach close() with the adapter still flagged connected,
  // tripping that contract. Activating without a socket-driven close
  // exercises exactly that path.
  ConnectionFixture f;
  f.conn.pipeline->activate();

  f.conn.destroy();

  EXPECT_EQ(f.conn.appAdapter, nullptr);
  EXPECT_EQ(f.conn.pipeline, nullptr);
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
