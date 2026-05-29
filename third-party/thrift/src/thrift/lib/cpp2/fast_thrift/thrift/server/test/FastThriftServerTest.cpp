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

#include <thread>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestServiceAsyncClient.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::server::test {

using apache::thrift::fast_thrift::thrift::test::
    BackwardsCompatibilityTestService;

class TestHandler
    : public apache::thrift::ServiceHandler<BackwardsCompatibilityTestService> {
 public:
  void echo(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = *message;
  }

  int64_t add(int64_t a, int64_t b) override { return a + b; }

  void sendResponse(std::string& response, int64_t size) override {
    response = std::string(static_cast<size_t>(size), 'x');
  }

  void ping() override {}
};

class FastThriftServerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();

    thrift::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    server_ =
        std::make_unique<thrift::FastThriftServer>(std::move(config), handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    server_->stop();
    server_.reset();
  }

  std::unique_ptr<apache::thrift::Client<BackwardsCompatibilityTestService>>
  createClient() {
    auto* evb = clientThread_->getEventBase();
    std::unique_ptr<apache::thrift::Client<BackwardsCompatibilityTestService>>
        client;

    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client = std::make_unique<
          apache::thrift::Client<BackwardsCompatibilityTestService>>(
          std::move(channel));
    });

    return client;
  }

  void destroyClientOnEvb(
      std::unique_ptr<
          apache::thrift::Client<BackwardsCompatibilityTestService>>& client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  // Runs a semifuture-returning lambda on the client EventBase and blocks
  // until the result is available. Fails the test on RPC error.
  template <typename Fn>
  auto syncCall(Fn&& fn) {
    using SemiFuture = std::invoke_result_t<Fn>;
    using T = typename SemiFuture::value_type;
    if constexpr (std::is_void_v<T>) {
      folly::Baton<> baton;
      auto* evb = clientThread_->getEventBase();
      evb->runInEventBaseThread([&] {
        fn().via(evb)
            .thenValue([&](folly::Unit) { baton.post(); })
            .thenError([&](const folly::exception_wrapper& ew) {
              ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
              baton.post();
            });
      });
      baton.wait();
    } else {
      folly::Baton<> baton;
      T result{};
      auto* evb = clientThread_->getEventBase();
      evb->runInEventBaseThread([&] {
        fn().via(evb)
            .thenValue([&](T val) {
              result = std::move(val);
              baton.post();
            })
            .thenError([&](const folly::exception_wrapper& ew) {
              ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
              baton.post();
            });
      });
      baton.wait();
      return result;
    }
  }

  std::shared_ptr<TestHandler> handler_;
  std::unique_ptr<thrift::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

TEST_F(FastThriftServerTest, Ping) {
  auto client = createClient();
  syncCall([&] { return client->semifuture_ping(); });
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, Echo) {
  auto client = createClient();
  auto result =
      syncCall([&] { return client->semifuture_echo("hello fast_thrift"); });
  EXPECT_EQ(result, "hello fast_thrift");
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, Add) {
  auto client = createClient();
  auto result = syncCall([&] { return client->semifuture_add(17, 25); });
  EXPECT_EQ(result, 42);
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, LargeResponse) {
  auto client = createClient();
  constexpr int64_t kResponseSize = 10000;
  auto result =
      syncCall([&] { return client->semifuture_sendResponse(kResponseSize); });
  EXPECT_EQ(result, std::string(kResponseSize, 'x'));
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, MultipleRequests) {
  auto client = createClient();

  folly::Baton<> baton1, baton2, baton3;
  std::string echo1, echo2, echo3;

  clientThread_->getEventBase()->runInEventBaseThread([&] {
    client->semifuture_echo("first")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string r) {
          echo1 = std::move(r);
          baton1.post();
        })
        .thenError([&](const folly::exception_wrapper&) { baton1.post(); });

    client->semifuture_echo("second")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string r) {
          echo2 = std::move(r);
          baton2.post();
        })
        .thenError([&](const folly::exception_wrapper&) { baton2.post(); });

    client->semifuture_echo("third")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string r) {
          echo3 = std::move(r);
          baton3.post();
        })
        .thenError([&](const folly::exception_wrapper&) { baton3.post(); });
  });

  baton1.wait();
  baton2.wait();
  baton3.wait();

  EXPECT_EQ(echo1, "first");
  EXPECT_EQ(echo2, "second");
  EXPECT_EQ(echo3, "third");
  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerTest, MultipleClients) {
  auto client1 = createClient();
  auto client2 = createClient();

  folly::Baton<> baton1, baton2;
  std::string result1, result2;

  clientThread_->getEventBase()->runInEventBaseThread([&] {
    client1->semifuture_echo("from client 1")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string r) {
          result1 = std::move(r);
          baton1.post();
        })
        .thenError([&](const folly::exception_wrapper&) { baton1.post(); });

    client2->semifuture_echo("from client 2")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string r) {
          result2 = std::move(r);
          baton2.post();
        })
        .thenError([&](const folly::exception_wrapper&) { baton2.post(); });
  });

  baton1.wait();
  baton2.wait();

  EXPECT_EQ(result1, "from client 1");
  EXPECT_EQ(result2, "from client 2");
  destroyClientOnEvb(client1);
  destroyClientOnEvb(client2);
}

TEST_F(FastThriftServerTest, GetAddress) {
  auto address = server_->getAddress();
  EXPECT_NE(address.getPort(), 0);
}

TEST(FastThriftServerServeTest, ServeBlocksUntilStop) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

  auto handler = std::make_shared<TestHandler>();

  thrift::FastThriftServerConfig config;
  config.address = folly::SocketAddress("::1", 0);
  config.numIOThreads = 1;

  thrift::FastThriftServer server(std::move(config), handler);

  std::atomic<bool> serveDone{false};
  std::thread serveThread([&] {
    server.serve();
    serveDone.store(true);
  });

  // Give serve() time to start and verify it's blocking
  /* sleep override */ std::this_thread::sleep_for(
      std::chrono::milliseconds(100));
  EXPECT_FALSE(serveDone.load());

  server.stop();
  serveThread.join();
  EXPECT_TRUE(serveDone.load());
}

} // namespace apache::thrift::fast_thrift::thrift::server::test
