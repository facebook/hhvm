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

class ArenaCheckingTransport
    : public folly::DecoratedAsyncTransportWrapper<folly::AsyncTransport> {
 public:
  using Base = folly::DecoratedAsyncTransportWrapper<folly::AsyncTransport>;
  using Base::Base;

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
      auto inArena = iua::addressInArena(const_cast<uint8_t*>(curr->data()));
      if (!inArena) {
        allInArena = false;
      }
      curr = curr->next();
    } while (curr != buf.get());

    Base::writeChain(cb, std::move(buf), flags);
  }
};

} // namespace

TEST(IOBufFactoryIntegrationTest, FrameInArena) {
  constexpr size_t kArenaSize = 4 * 1024 * 1024;
  if (!iua::ioUringArenaSupported()) {
    GTEST_SKIP() << "IoUringArena not supported";
  }
  if (!iua::init(kArenaSize)) {
    GTEST_SKIP() << "IoUringArena initialization not supported";
  }

  auto server = ScopedServerInterfaceThread(std::make_shared<TestHandler>());
  auto factory = std::make_shared<folly::IOBufFactory>(
      folly::memory::makeIOBufArenaFactory<iua>());

  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb, server.getAddress());
  auto transport = ArenaCheckingTransport::UniquePtr(
      new ArenaCheckingTransport(std::move(socket)));
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
