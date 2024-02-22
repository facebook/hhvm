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
#include <chrono>
#include <compare>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include <folly/Try.h>
#include <folly/experimental/TestUtil.h>
#include <folly/experimental/observer/Observer.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp/concurrency/Util.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/FieldRef.h>
#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/BaseThriftServer.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/test/gen-cpp2/AsyncProcessor_types.h>
#include <thrift/lib/cpp2/test/gen-cpp2/Service_types.h>
#include <thrift/lib/cpp2/test/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using namespace apache::thrift::async;
using namespace apache::thrift::transport;
using namespace apache::thrift::concurrency;
using namespace std::literals;
using folly::test::find_resource;

DECLARE_bool(rocket_client_enable_keep_alive);
DECLARE_int64(rocket_client_keep_alive_interval_ms);
DECLARE_int64(rocket_client_keep_alive_timeout_ms);

class RocketClientTest : public testing::Test {
 public:
  ClientChannel::Ptr makeChannel(folly::AsyncTransport::UniquePtr socket) {
    return RocketClientChannel::newChannel(std::move(socket));
  }

  std::string getRandomString(size_t len) {
    auto randomChar = []() -> char {
      return 32 + (folly::Random::rand32() % 95);
    };
    std::string str(len, 0);
    std::generate_n(str.begin(), len, randomChar);
    return str;
  }

  class TestInterface : public apache::thrift::ServiceHandler<TestService> {
   public:
    int sync_echoInt(int) override {
      shrinkSocketSendBuffer();
      return 0;
    }

    void sync_echoRequest(
        ::std::string& _return, std::unique_ptr<::std::string>) override {
      hit++;
      _return = "__";
    }

    int getHit() { return hit; }

   private:
    int shrinkSocketSendBuffer() {
      auto const_transport =
          getRequestContext()->getConnectionContext()->getTransport();
      auto transport = const_cast<folly::AsyncTransport*>(const_transport);
      auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
      sock->setSendBufSize(0);
      int bufsize = 0;
      socklen_t bufsizelen = sizeof(bufsize);
      sock->getSockOpt<int>(SOL_SOCKET, SO_SNDBUF, &bufsize, &bufsizelen);
      return bufsize;
    }

    int hit{0};
  };
};

// This test is using a big payload to test the keep alive mechanism. The size
// of payload is 80MB for now to ensure even in test environment, it will take
// a long time to transfer. This test can be flaky if the local transfer speed
// increased drastically.
TEST_F(RocketClientTest, KeepAliveWatcherTest) {
  FLAGS_rocket_client_enable_keep_alive = true;
  // Smaller numbers to make keep_alive sensitive.
  FLAGS_rocket_client_keep_alive_interval_ms = 10;
  FLAGS_rocket_client_keep_alive_timeout_ms = 100;
  // Increase this if test is flaky.
  size_t payloadSize = /*80MB*/ 80000000;

  auto testInterface = std::make_shared<TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);

  auto client = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr, [&](auto socket) {
        // Set send buffer size to minimum to similate slow network.
        socket->setSendBufSize(0);
        return makeChannel(std::move(socket));
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

  // The big payload should not hit server.
  EXPECT_EQ(testInterface->getHit(), 1);

  /*
   * Turn-off Keep Alive and Do the same thing. The large payload should not
   * have problem finishing transfer.
   */
  FLAGS_rocket_client_enable_keep_alive = false;

  auto client1 = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr, [&](auto socket) { return makeChannel(std::move(socket)); });

  client1->sync_echoRequest(response, "");
  client1->sync_echoRequest(response, getRandomString(payloadSize));
  EXPECT_EQ(response, "__");

  EXPECT_EQ(testInterface->getHit(), 3);
}

// This test ensure KeepAlive works as normal when connection was bouncing
// between eventbases.
TEST_F(RocketClientTest, KeepAliveEvbDetachAttachTest) {
  FLAGS_rocket_client_enable_keep_alive = true;
  // Smaller numbers to make keep_alive sensitive.
  FLAGS_rocket_client_keep_alive_interval_ms = 10;
  FLAGS_rocket_client_keep_alive_timeout_ms = 100;
  // Increase this if test is flaky.
  size_t payloadSize = /*80MB*/ 80000000;

  auto testInterface = std::make_shared<TestInterface>();
  ScopedServerInterfaceThread runner(testInterface);

  auto client = runner.newStickyClient<apache::thrift::Client<TestService>>(
      nullptr, [&](auto socket) {
        // Set send buffer size to minimum to similate slow network.
        socket->setSendBufSize(0);
        // KeepAlive will be created and started here.
        auto channel = makeChannel(std::move(socket));
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
