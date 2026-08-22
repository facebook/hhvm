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

#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/DecoratedAsyncTransportWrapper.h>
#include <folly/memory/IOBufArenaFactory.h>
#include <folly/memory/IoUringArena.h>

#include <thrift/lib/cpp2/async/AsyncClient.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;

namespace {

using iua = folly::IoUringArena;

// A malloc-backed arena that records its allocations so the integration test
// can verify buffers were sourced from our IOBufFactory. Always available, so
// the test runs on hosts without io_uring support instead of skipping (a
// GTEST_SKIP is scored as a test failure by the fleet test runner).
// SIZED_FREE-safe because the buffers come from malloc.
class TrackingArena {
 public:
  static bool initialized() { return true; }

  static void* allocate(size_t size) {
    void* p = std::malloc(size);
    if (p) {
      auto addr = reinterpret_cast<uintptr_t>(p);
      std::lock_guard<std::mutex> g(state().mutex);
      state().ranges.emplace_back(addr, size);
    }
    return p;
  }

  static bool addressInArena(const uint8_t* address) {
    auto a = reinterpret_cast<uintptr_t>(address);
    std::lock_guard<std::mutex> g(state().mutex);
    for (const auto& [start, size] : state().ranges) {
      if (a >= start && a < start + size) {
        return true;
      }
    }
    return false;
  }

 private:
  struct State {
    std::mutex mutex;
    std::vector<std::pair<uintptr_t, size_t>> ranges;
  };
  static State& state() {
    static State s;
    return s;
  }
};

class ArenaCheckingTransport
    : public folly::DecoratedAsyncTransportWrapper<folly::AsyncTransport> {
 public:
  using Base = folly::DecoratedAsyncTransportWrapper<folly::AsyncTransport>;

  ArenaCheckingTransport(
      folly::AsyncTransport::UniquePtr socket,
      std::function<bool(const uint8_t*)> inArena)
      : Base(std::move(socket)), inArena_(std::move(inArena)) {}

  size_t writesChecked{0};
  size_t bufsChecked{0};
  bool allInArena{true};

  void reset() {
    writesChecked = 0;
    bufsChecked = 0;
    allInArena = true;
  }

  void writeChain(
      WriteCallback* cb,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags) override {
    writesChecked++;
    const auto* curr = buf.get();
    do {
      bufsChecked++;
      if (!inArena_(curr->data())) {
        allInArena = false;
      }
      curr = curr->next();
    } while (curr != buf.get());

    Base::writeChain(cb, std::move(buf), flags);
  }

 private:
  std::function<bool(const uint8_t*)> inArena_;
};

} // namespace

TEST(IOBufFactoryIntegrationTest, FrameInArena) {
  constexpr size_t kArenaSize = 4 * 1024 * 1024;

  // Prefer the real io_uring arena; fall back to a malloc-backed tracking arena
  // on hosts without io_uring support. Both exercise the same contract: frames
  // written by RocketClientChannel are allocated through our IOBufFactory.
  folly::IOBufFactory factoryFn;
  std::function<bool(const uint8_t*)> inArena;
  if (iua::ioUringArenaSupported() && iua::init(kArenaSize)) {
    factoryFn = folly::memory::makeIOBufArenaFactory<iua>();
    inArena = [](const uint8_t* p) {
      return iua::addressInArena(const_cast<uint8_t*>(p));
    };
  } else {
    factoryFn = folly::memory::makeIOBufArenaFactory<TrackingArena>();
    inArena = [](const uint8_t* p) { return TrackingArena::addressInArena(p); };
  }
  auto factory = std::make_shared<folly::IOBufFactory>(std::move(factoryFn));

  auto server = ScopedServerInterfaceThread(std::make_shared<TestHandler>());

  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb, server.getAddress());
  auto transport = ArenaCheckingTransport::UniquePtr(
      new ArenaCheckingTransport(std::move(socket), std::move(inArena)));
  auto* checker = static_cast<ArenaCheckingTransport*>(transport.get());

  auto channel = RocketClientChannel::newChannel(std::move(transport));
  auto options = GeneratedAsyncClient::Options().setIOBufFactory(factory);
  auto client = std::make_unique<apache::thrift::Client<test::TestService>>(
      std::move(channel), std::move(options));

  std::string response;
  client->sync_echoRequest(response, "hello");
  EXPECT_GT(checker->writesChecked, 0);
  EXPECT_GT(checker->bufsChecked, 0);
  EXPECT_TRUE(checker->allInArena);

  checker->reset();
  std::string largePayload(1024 * 1024, 'x');
  client->sync_echoRequest(response, largePayload);
  EXPECT_GT(checker->writesChecked, 0);
  EXPECT_GT(checker->bufsChecked, 0);
  EXPECT_TRUE(checker->allInArena);
}
