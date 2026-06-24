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

#include <stdint.h>
#include <sys/socket.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <compare>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include <folly/Portability.h>
#include <folly/Try.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>
#include <folly/testing/TestUtil.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/gen-cpp2/AsyncProcessor_types.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Service_types.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;
using namespace std::literals;
using folly::test::find_resource;

static constexpr int kSanitizerTimeoutMultiplier =
    folly::kIsSanitizeThread ? 10 : 1;

class RocketClientTest : public testing::Test {
 public:
  ClientChannel::Ptr makeChannelWithMetadata(
      folly::AsyncTransport::UniquePtr socket) {
    return RocketClientChannel::newChannelWithMetadata(
        std::move(socket), metadata_);
  }

  static std::string getRandomString(size_t len) {
    auto randomChar = []() -> char {
      return 32 + (folly::Random::rand32() % 95);
    };
    std::string str(len, 0);
    std::generate_n(str.begin(), len, randomChar);
    return str;
  }

  RequestSetupMetadata metadata_;

  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    int sync_echoInt(int) override {
      shrinkSocketSendBuffer();
      return 0;
    }

    void sync_echoRequest(
        ::std::string& _return, std::unique_ptr<::std::string> input) override {
      hit_++;
      if (*input == "big") {
        _return = getRandomString(1024 * 1024 * 80);
      } else {
        _return = "__";
      }
    }

    int getHit() { return hit_; }

   private:
    int shrinkSocketSendBuffer() {
      auto const_transport =
          getRequestContext()->getConnectionContext()->getTransport();
      auto transport = const_cast<folly::AsyncTransport*>(const_transport);
      auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
      CHECK_EQ(sock->setSendBufSize(1), 0);
      int bufsize = 0;
      socklen_t bufsizelen = sizeof(bufsize);
      sock->getSockOpt<int>(SOL_SOCKET, SO_SNDBUF, &bufsize, &bufsizelen);
      return bufsize;
    }

    std::atomic<int> hit_{0};
  };
};

class FirstResponseErrorCallback : public StreamClientCallback {
 public:
  bool onFirstResponse(
      FirstResponsePayload&&,
      folly::EventBase*,
      StreamServerCallback*) override {
    ADD_FAILURE() << "unexpected first response";
    return false;
  }

  void onFirstResponseError(folly::exception_wrapper) override { ++errors_; }

  bool onStreamNext(StreamPayload&&) override {
    ADD_FAILURE() << "unexpected stream payload";
    return false;
  }

  void onStreamError(folly::exception_wrapper) override {
    ADD_FAILURE() << "unexpected stream error";
  }

  void onStreamComplete() override {
    ADD_FAILURE() << "unexpected stream completion";
  }

  void resetServerCallback(StreamServerCallback&) override {
    ADD_FAILURE() << "unexpected server callback reset";
  }

  int errors() const { return errors_; }

 private:
  int errors_{0};
};

class TestRocketClient : public apache::thrift::rocket::RocketClient {
 public:
  using Ptr =
      std::unique_ptr<TestRocketClient, folly::DelayedDestruction::Destructor>;

  static Ptr create(
      folly::EventBase& evb,
      folly::AsyncTransport::UniquePtr socket,
      RequestSetupMetadata&& setupMetadata) {
    return Ptr(
        new TestRocketClient(evb, std::move(socket), std::move(setupMetadata)));
  }

  uint32_t guardCount() const { return getDestructorGuardCount(); }

 private:
  TestRocketClient(
      folly::EventBase& evb,
      folly::AsyncTransport::UniquePtr socket,
      RequestSetupMetadata&& setupMetadata)
      : RocketClient(evb, std::move(socket), std::move(setupMetadata)) {}
};

// These tests rely on ASAN to catch RocketClient destruction use-after-free, so
// they are no-ops under non-ASAN builds. Early-return (rather than GTEST_SKIP)
// so test infra does not flag them as skipped/bad.
#ifndef FOLLY_SANITIZE_ADDRESS
#define RETURN_IF_NON_ASAN_ROCKET_CLIENT_TEST() \
  do {                                          \
    return;                                     \
  } while (false)
#else
#define RETURN_IF_NON_ASAN_ROCKET_CLIENT_TEST() \
  do {                                          \
  } while (false)
#endif

TEST_F(
    RocketClientTest,
    DestroyPendingFirstResponseTimeoutCancelDoesNotReenterClientDestruction) {
  RETURN_IF_NON_ASAN_ROCKET_CLIENT_TEST();

  apache::thrift::rocket::RocketClient::FlushList flushList;
  auto testInterface = std::make_shared<RocketClientTest::TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);
  FirstResponseErrorCallback callback;
  folly::EventBase evb;
  std::thread evbThread([&] { evb.loopForever(); });

  evb.runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(&evb, runner.getAddress());
    auto client = TestRocketClient::create(
        evb, std::move(socket), RequestSetupMetadata{});
    CHECK_NOTNULL(client.get())->setFlushList(&flushList);
    auto& clientRef = *CHECK_NOTNULL(client.get());
    clientRef.sendRequestStream(
        apache::thrift::rocket::Payload::makeFromData(folly::ByteRange()),
        std::chrono::seconds(60),
        std::chrono::milliseconds::zero(),
        1,
        &callback);

    ASSERT_TRUE(clientRef.streamExists(apache::thrift::rocket::StreamId{1}));
    ASSERT_GT(clientRef.guardCount(), 0);

    auto& rawClient = *CHECK_NOTNULL(client.release());
    rawClient.destroy();
    rawClient.cancelStream(apache::thrift::rocket::StreamId{1});
  });

  evb.runInEventBaseThreadAndWait([&] { evb.terminateLoopSoon(); });
  evbThread.join();

  EXPECT_EQ(callback.errors(), 0);
}

TEST_F(
    RocketClientTest,
    DestroyPendingFirstResponseTimeoutExpiryDoesNotReenterClientDestruction) {
  RETURN_IF_NON_ASAN_ROCKET_CLIENT_TEST();

  apache::thrift::rocket::RocketClient::FlushList flushList;
  auto testInterface = std::make_shared<RocketClientTest::TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);
  FirstResponseErrorCallback callback;
  folly::EventBase evb;
  std::thread evbThread([&] { evb.loopForever(); });

  evb.runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(&evb, runner.getAddress());
    auto client = TestRocketClient::create(
        evb, std::move(socket), RequestSetupMetadata{});
    CHECK_NOTNULL(client.get())->setFlushList(&flushList);
    auto& clientRef = *CHECK_NOTNULL(client.get());
    clientRef.sendRequestStream(
        apache::thrift::rocket::Payload::makeFromData(folly::ByteRange()),
        std::chrono::milliseconds(1),
        std::chrono::milliseconds::zero(),
        1,
        &callback);

    ASSERT_TRUE(clientRef.streamExists(apache::thrift::rocket::StreamId{1}));
    ASSERT_GT(clientRef.guardCount(), 0);

    auto& rawClient = *CHECK_NOTNULL(client.release());
    rawClient.destroy();
  });

  evb.runInEventBaseThreadAndWait([&] {
    CHECK(evb.tryRunAfterDelay(
        [&] { evb.terminateLoopSoon(); },
        static_cast<uint32_t>(std::chrono::milliseconds(100).count())));
  });
  evbThread.join();

  EXPECT_EQ(callback.errors(), 1);
}

TEST_F(
    RocketClientTest,
    ChannelDestroyPendingFirstResponseTimeoutExpiryDoesNotReenterClientDestruction) {
  RETURN_IF_NON_ASAN_ROCKET_CLIENT_TEST();

  apache::thrift::rocket::RocketClient::FlushList flushList;
  auto testInterface = std::make_shared<RocketClientTest::TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);
  FirstResponseErrorCallback callback;
  folly::EventBase evb;
  std::thread evbThread([&] { evb.loopForever(); });

  evb.runInEventBaseThreadAndWait([&] {
    auto socket = folly::AsyncSocket::newSocket(&evb, runner.getAddress());
    auto channel = RocketClientChannel::newChannelWithMetadata(
        std::move(socket), RequestSetupMetadata{});
    CHECK_NOTNULL(channel.get())->setFlushList(&flushList);

    RpcOptions rpcOptions;
    rpcOptions.setTimeout(std::chrono::milliseconds(1));
    channel->sendRequestStream(
        rpcOptions,
        "dummy",
        SerializedRequest(folly::IOBuf::copyBuffer("")),
        std::make_shared<transport::THeader>(),
        &callback,
        nullptr);

    auto& rawChannel = *CHECK_NOTNULL(channel.release());
    rawChannel.destroy();
  });

  evb.runInEventBaseThreadAndWait([&] {
    CHECK(evb.tryRunAfterDelay(
        [&] { evb.terminateLoopSoon(); },
        static_cast<uint32_t>(std::chrono::milliseconds(100).count())));
  });
  evbThread.join();

  EXPECT_EQ(callback.errors(), 1);
}

TEST_F(
    RocketClientTest,
    ChannelDestroyPendingFirstResponseTimeoutEventBaseTeardownDoesNotReenterClientDestruction) {
  RETURN_IF_NON_ASAN_ROCKET_CLIENT_TEST();

  apache::thrift::rocket::RocketClient::FlushList flushList;
  auto testInterface = std::make_shared<RocketClientTest::TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);
  FirstResponseErrorCallback callback;

  {
    folly::EventBase evb;
    auto socket = folly::AsyncSocket::newSocket(&evb, runner.getAddress());
    auto channel = RocketClientChannel::newChannelWithMetadata(
        std::move(socket), RequestSetupMetadata{});
    CHECK_NOTNULL(channel.get())->setFlushList(&flushList);

    RpcOptions rpcOptions;
    rpcOptions.setTimeout(std::chrono::seconds(60));
    channel->sendRequestStream(
        rpcOptions,
        "dummy",
        SerializedRequest(folly::IOBuf::copyBuffer("")),
        std::make_shared<transport::THeader>(),
        &callback,
        nullptr);

    auto& rawChannel = *CHECK_NOTNULL(channel.release());
    rawChannel.destroy();
  }

  EXPECT_EQ(callback.errors(), 1);
}

// This test is using a big payload to test the keep alive mechanism. The size
// of payload is 8MB for now to ensure even in test environment, it will take
// a long time to transfer. This test can be flaky if the local transfer speed
// increased drastically.
TEST_F(RocketClientTest, KeepAliveWatcherLargeRequestTest) {
  // Increase this if test is flaky.
  size_t payloadSize = /*40MB*/ 40 * 1024 * 1024;
  metadata_.keepAliveTimeoutMs() = 25 * kSanitizerTimeoutMultiplier;

  auto testInterface = std::make_shared<TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);

  auto client = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr, [&](auto socket) {
        // Set send buffer size to minimum to simulate slow network.
        CHECK_EQ(socket->setSendBufSize(1), 0);
        return makeChannelWithMetadata(std::move(socket));
      });

  // Call to shrink server socket buffer
  client->semifuture_echoInt(0).get();
  std::string response;
  // Test normal client reads (timeout should not fire).
  client->sync_echoRequest(response, "");
  // Sending a big payload, with a timeout of 100ms, should close connection and
  // return EOF.
  // Note: if this test was flaky, increase payloadSize should help.
  EXPECT_THROW(
      client->sync_echoRequest(response, getRandomString(payloadSize)),
      TTransportException);
  // Connection was closed, so next call should fail with NOT_OPEN.
  try {
    client->sync_echoRequest(response, "");
    FAIL() << "Expected exception.";
  } catch (const TTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::NOT_OPEN);
  }
  // Only the first call should hit server.
  EXPECT_EQ(testInterface->getHit(), 1);
  /*
   * Turn-off Keep Alive and Do the same thing. The large payload should not
   * have problem finishing transfer.
   */
  metadata_.keepAliveTimeoutMs() = 0;
  auto client1 = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr,
      [&](auto socket) { return makeChannelWithMetadata(std::move(socket)); });

  client1->sync_echoRequest(response, "");
  client1->sync_echoRequest(response, getRandomString(payloadSize));
  EXPECT_EQ(response, "__");

  EXPECT_EQ(testInterface->getHit(), 3);
}

// Send a small request, expecting a big response. Such that the server will
// process the request, however KeepAlive will be blocked during the response,
// and close the connection.
TEST_F(RocketClientTest, KeepAliveWatcherLargeResponseTest) {
  metadata_.keepAliveTimeoutMs() = 100 * kSanitizerTimeoutMultiplier;
  auto testInterface = std::make_shared<TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);

  auto client = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr, [&](auto socket) {
        // Set send buffer size to minimum to similate slow network.
        CHECK_EQ(socket->setSendBufSize(1), 0);
        return makeChannelWithMetadata(std::move(socket));
      });

  // Call to shrink server socket buffer
  client->semifuture_echoInt(0).get();
  std::string response;
  // Test normal client reads (timeout should not fire).
  client->sync_echoRequest(response, "");

  // The big response will throw EOF.
  try {
    client->sync_echoRequest(response, "big");
    FAIL() << "Expected exception.";
  } catch (const TTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::END_OF_FILE);
    EXPECT_EQ(
        std::string(ex.what()),
        "Connection was closed due to KeepAliveTimeout.");
  }
  // Connection was closed, so next call should fail with NOT_OPEN.
  try {
    client->sync_echoRequest(response, "");
    FAIL() << "Expected exception.";
  } catch (const TTransportException& ex) {
    EXPECT_EQ(ex.getType(), TTransportException::NOT_OPEN);
  }

  // First two calls hit the server.
  EXPECT_EQ(testInterface->getHit(), 2);

  /*
   * Turn-off Keep Alive and Do the same thing. The large payload should not
   * have problem finishing transfer.
   */
  metadata_.keepAliveTimeoutMs() = 0;
  auto client1 = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr,
      [&](auto socket) { return makeChannelWithMetadata(std::move(socket)); });

  // Test normal client1 reads (timeout should not fire).
  client1->sync_echoRequest(response, "");

  // Both payload should hit server. The big response should have no problem.
  try {
    client1->sync_echoRequest(response, "big");
    EXPECT_EQ(response.size(), 1024 * 1024 * 80);
  } catch (const TTransportException& ex) {
    // It's still possible to hit regular timeout in stress test for large
    // response. This catch is only to deflaky.
    EXPECT_EQ(ex.getType(), TTransportException::TIMED_OUT);
  }

  EXPECT_EQ(testInterface->getHit(), 4);
}

// This test ensure KeepAlive works as normal when connection was bouncing
// between eventbases.
TEST_F(RocketClientTest, KeepAliveEvbDetachAttachTest) {
  // Increase this if test is flaky.
  size_t payloadSize = /*40MB*/ 40 * 1024 * 1024;
  metadata_.keepAliveTimeoutMs() = 25 * kSanitizerTimeoutMultiplier;

  auto testInterface = std::make_shared<TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);

  auto client = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr, [&](auto socket) {
        // Set send buffer size to minimum to simulate slow network.
        CHECK_EQ(socket->setSendBufSize(1), 0);
        // KeepAlive will be created and started here.
        auto channel = makeChannelWithMetadata(std::move(socket));
        auto evb = channel->getEventBase();
        // KeepAlive will be stopped here.
        channel->detachEventBase();
        // KeepAlive will be restarted here.
        channel->attachEventBase(evb);
        return std::move(channel);
      });

  std::string response;
  // Test normal client reads (timeout should not fire).
  client->sync_echoRequest(response, "");
  // Sending a big payload, with a timeout of 100ms, should close connection and
  // return EOF.
  // Note: if this test was flaky, increase payloadSize should help.
  EXPECT_THROW(
      client->sync_echoRequest(response, getRandomString(payloadSize)),
      TTransportException);

  // The big payload should not hit server.
  EXPECT_EQ(testInterface->getHit(), 1);
}
