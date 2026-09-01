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

#include <thrift/lib/cpp2/fast_thrift/connection/handler/ConnectionLimitHandler.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/observer/Observer.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/ConnectionStats.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>

namespace apache::thrift::fast_thrift::connection::handler {

namespace {

using channel_pipeline::erase_and_box;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;

class FakeContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    forwarded.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception = std::move(e);
  }

  std::vector<TypeErasedBox> forwarded;
  folly::exception_wrapper exception;
};

// The transport is never touched by this handler — only the peer address is —
// so a message carrying just the address is enough to drive it.
TypeErasedBox makeConnection(const std::string& peer) {
  return erase_and_box(
      ConnectionMessage{
          .transport = nullptr,
          .clientAddr = folly::SocketAddress(peer, 12345),
          .peerSecurity = nullptr});
}

struct Fixture {
  explicit Fixture(uint32_t limit)
      : handler(
            folly::observer::makeStaticObserver<uint32_t>(limit),
            &liveConnections,
            &stats) {}

  std::atomic<size_t> liveConnections{0};
  ConnectionStatsShard stats;
  ConnectionLimitHandler handler;
  FakeContext ctx;
};

} // namespace

TEST(ConnectionLimitHandlerTest, ForwardsWhileBelowTheLimit) {
  Fixture f(/*limit=*/2);
  f.liveConnections = 1;

  EXPECT_EQ(
      f.handler.onRead(f.ctx, makeConnection("10.0.0.1")), Result::Success);

  EXPECT_EQ(f.ctx.forwarded.size(), 1);
  EXPECT_EQ(f.stats.connectionsRejected.value(), 0);
}

TEST(ConnectionLimitHandlerTest, RefusesOnceTheLimitIsReached) {
  Fixture f(/*limit=*/2);
  f.liveConnections = 2;

  EXPECT_EQ(f.handler.onRead(f.ctx, makeConnection("10.0.0.1")), Result::Error);

  EXPECT_TRUE(f.ctx.forwarded.empty());
  EXPECT_EQ(f.stats.connectionsRejected.value(), 1);
}

// A server at its limit must still answer the local health probe, or the agent
// that owns it concludes the process is dead and restarts it.
TEST(ConnectionLimitHandlerTest, AdmitsLoopbackAtTheLimit) {
  Fixture f(/*limit=*/2);
  f.liveConnections = 2;

  EXPECT_EQ(
      f.handler.onRead(f.ctx, makeConnection("127.0.0.1")), Result::Success);

  EXPECT_EQ(f.ctx.forwarded.size(), 1);
  EXPECT_EQ(f.stats.connectionsRejected.value(), 0);
}

TEST(ConnectionLimitHandlerTest, ZeroLimitDisablesTheCap) {
  Fixture f(/*limit=*/0);
  f.liveConnections = 1000;

  EXPECT_EQ(
      f.handler.onRead(f.ctx, makeConnection("10.0.0.1")), Result::Success);

  EXPECT_EQ(f.ctx.forwarded.size(), 1);
  EXPECT_EQ(f.stats.connectionsRejected.value(), 0);
}

} // namespace apache::thrift::fast_thrift::connection::handler
