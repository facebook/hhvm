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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServerRegistry.h>

#include <atomic>
#include <chrono>
#include <optional>
#include <string_view>
#include <thread>

#include <gtest/gtest.h>

#include <folly/SocketAddress.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>

namespace apache::thrift::fast_thrift::thrift {
namespace {

using instrumentation::forEachServer;
using instrumentation::getServerCount;
using instrumentation::kFastThriftServerTrackerKey;

// Build a FastThriftServer with minimal config — registration is done by the
// embedded ServerTracker member, which fires from the constructor before
// start() / serve() are ever called. Tests intentionally never call start();
// the registry contract is decoupled from lifecycle.
FastThriftServerConfig minimalConfig() {
  // Tests never call start(), so the address is unused. Avoid
  // setFromLocalIpPort here — it does DNS-style resolution and fails on
  // hosts that lack the requested address family.
  FastThriftServerConfig cfg;
  cfg.numIOThreads = 1;
  return cfg;
}

TEST(FastThriftServerRegistryTest, EmptyByDefault) {
  EXPECT_EQ(getServerCount(kFastThriftServerTrackerKey), 0u);
}

TEST(
    FastThriftServerRegistryTest, ConstructionRegistersDestructionDeregisters) {
  EXPECT_EQ(getServerCount(kFastThriftServerTrackerKey), 0u);
  {
    FastThriftServer server(minimalConfig());
    EXPECT_EQ(getServerCount(kFastThriftServerTrackerKey), 1u);
  }
  EXPECT_EQ(getServerCount(kFastThriftServerTrackerKey), 0u);
}

TEST(FastThriftServerRegistryTest, MultipleServersAreAllVisible) {
  FastThriftServer a(minimalConfig());
  FastThriftServer b(minimalConfig());
  FastThriftServer c(minimalConfig());

  EXPECT_EQ(getServerCount(kFastThriftServerTrackerKey), 3u);

  std::vector<FastThriftServer*> seen;
  forEachServer(kFastThriftServerTrackerKey, [&](FastThriftServer& s) {
    seen.push_back(&s);
  });
  ASSERT_EQ(seen.size(), 3u);
  // We don't assume iteration order, just identity coverage.
  EXPECT_TRUE(
      std::find(seen.begin(), seen.end(), &a) != seen.end() &&
      std::find(seen.begin(), seen.end(), &b) != seen.end() &&
      std::find(seen.begin(), seen.end(), &c) != seen.end());
}

TEST(FastThriftServerRegistryTest, UnknownKeyYieldsNoIteration) {
  FastThriftServer server(minimalConfig());
  size_t calls = 0;
  forEachServer("not_a_real_key", [&](FastThriftServer&) { ++calls; });
  EXPECT_EQ(calls, 0u);
  EXPECT_EQ(getServerCount("not_a_real_key"), 0u);
}

// Destruction must block until any in-flight forEachServer callback has
// returned — otherwise a callback could observe a half-destroyed server.
// Drive that contract with two threads: the iterator parks inside the
// callback while a second thread races to destroy the server.
TEST(FastThriftServerRegistryTest, DestructorWaitsForIteratorCallback) {
  std::optional<FastThriftServer> server;
  server.emplace(minimalConfig());

  folly::Baton<> insideCallback;
  folly::Baton<> releaseCallback;
  std::atomic<bool> destructorReturned{false};

  std::thread iterator([&] {
    forEachServer(kFastThriftServerTrackerKey, [&](FastThriftServer&) {
      insideCallback.post();
      releaseCallback.wait();
      // While we're parked here, the destructor must not be allowed to
      // finish. Verify by snapshotting the flag at callback exit time.
      EXPECT_FALSE(destructorReturned.load());
    });
  });

  insideCallback.wait();

  std::thread destroyer([&] {
    server.reset();
    destructorReturned.store(true);
  });

  // Give the destroyer a chance to run its registry-removal but be blocked
  // on cb_.join(). Without a real synchronization primitive on the join,
  // this is a soft check; the assertion inside the callback above is the
  // strict one.
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  EXPECT_FALSE(destructorReturned.load());

  releaseCallback.post();
  iterator.join();
  destroyer.join();

  EXPECT_TRUE(destructorReturned.load());
  EXPECT_EQ(getServerCount(kFastThriftServerTrackerKey), 0u);
}

} // namespace
} // namespace apache::thrift::fast_thrift::thrift
