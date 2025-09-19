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
#include <folly/portability/GFlags.h>
#include <folly/test/TestUtils.h>

#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/transport/core/testutil/MockCallback.h>
#include <thrift/lib/cpp2/transport/core/testutil/TransportCompatibilityTest.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/ThriftRocketServerHandler.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>

DECLARE_int32(num_client_connections);
DECLARE_string(transport); // ConnectionManager depends on this flag.

namespace apache::thrift {

using namespace apache::thrift::transport;

class RocketCompatibilityTest : public testing::Test {
 public:
  RocketCompatibilityTest() {
    FLAGS_transport = "rocket";

    compatibilityTest_ = std::make_unique<TransportCompatibilityTest>();
    // already rocket, no upgrade expected
    compatibilityTest_->setTransportUpgradeExpected(false);
    compatibilityTest_->startServer();
  }

 protected:
  std::unique_ptr<TransportCompatibilityTest> compatibilityTest_;
};

class RocketManuallyStartServerTest : public testing::Test {
 public:
  RocketManuallyStartServerTest() {
    FLAGS_transport = "rocket";

    compatibilityTest_ = std::make_unique<TransportCompatibilityTest>();
  }

 protected:
  std::unique_ptr<TransportCompatibilityTest> compatibilityTest_;
};

TEST_F(RocketCompatibilityTest, RequestResponse_Simple) {
  compatibilityTest_->TestRequestResponse_Simple();
}

TEST_F(RocketCompatibilityTest, RequestResponse_Sync) {
  compatibilityTest_->TestRequestResponse_Sync();
}

TEST_F(RocketCompatibilityTest, RequestResponse_Destruction) {
  compatibilityTest_->TestRequestResponse_Destruction();
}

TEST_F(RocketCompatibilityTest, RequestResponse_MultipleClients) {
  compatibilityTest_->TestRequestResponse_MultipleClients();
}

TEST_F(RocketCompatibilityTest, RequestResponse_ExpectedException) {
  compatibilityTest_->TestRequestResponse_ExpectedException();
}

TEST_F(RocketCompatibilityTest, RequestResponse_UnexpectedException) {
  compatibilityTest_->TestRequestResponse_UnexpectedException();
}

// Warning: This test may be flaky due to use of timeouts.
TEST_F(RocketCompatibilityTest, RequestResponse_Timeout) {
  compatibilityTest_->TestRequestResponse_Timeout();
}

TEST_F(RocketCompatibilityTest, DefaultTimeoutValueTest) {
  compatibilityTest_->connectToServer([](auto client) {
    // Opts with no timeout value
    RpcOptions opts;

    // Ok to sleep for 100msec
    auto cb = std::make_unique<MockCallback>(false, false);
    client->sleep(opts, std::move(cb), 100);

    /* Sleep to give time for all callbacks to be completed */
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto* channel = dynamic_cast<ClientChannel*>(client->getChannel());
    EXPECT_TRUE(channel);
    channel->getEventBase()->runInEventBaseThreadAndWait([&]() {
      channel->setTimeout(1); // 1ms
    });

    // Now it should timeout
    cb = std::make_unique<MockCallback>(false, true);
    client->sleep(opts, std::move(cb), 100);

    /* Sleep to give time for all callbacks to be completed */
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  });
}

TEST_F(RocketCompatibilityTest, RequestResponse_Header) {
  compatibilityTest_->TestRequestResponse_Header();
}

TEST_F(RocketCompatibilityTest, RequestResponse_Header_Load) {
  compatibilityTest_->TestRequestResponse_Header_Load();
}

TEST_F(RocketCompatibilityTest, RequestResponse_Header_ExpectedException) {
  compatibilityTest_->TestRequestResponse_Header_ExpectedException();
}

TEST_F(RocketCompatibilityTest, RequestResponse_Header_UnexpectedException) {
  compatibilityTest_->TestRequestResponse_Header_UnexpectedException();
}

TEST_F(RocketCompatibilityTest, RequestResponse_Saturation) {
  compatibilityTest_->connectToServer([this](auto client) {
    EXPECT_CALL(*compatibilityTest_->handler_.get(), add_(3)).Times(2);
    // note that no EXPECT_CALL for add_(5)
    auto* channel = dynamic_cast<RocketClientChannel*>(client->getChannel());
    ASSERT_NE(nullptr, channel);

    channel->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { channel->setMaxPendingRequests(0u); });
    EXPECT_THROW(client->future_add(5).get(), TTransportException);

    channel->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { channel->setMaxPendingRequests(1u); });

    EXPECT_EQ(3, client->future_add(3).get());
    EXPECT_EQ(6, client->future_add(3).get());
  });
}

TEST_F(RocketCompatibilityTest, RequestResponse_IsOverloaded) {
  compatibilityTest_->TestRequestResponse_IsOverloaded();
}

TEST_F(RocketCompatibilityTest, RequestResponse_Connection_CloseNow) {
  compatibilityTest_->TestRequestResponse_Connection_CloseNow();
}

TEST_F(RocketCompatibilityTest, RequestResponse_ServerQueueTimeout) {
  compatibilityTest_->TestRequestResponse_ServerQueueTimeout();
}

TEST_F(RocketCompatibilityTest, RequestResponse_ResponseSizeTooBig) {
  compatibilityTest_->TestRequestResponse_ResponseSizeTooBig();
}

// TODO(T90625074)
TEST_F(RocketCompatibilityTest, DISABLED_RequestResponse_Checksumming) {
  compatibilityTest_->TestRequestResponse_Checksumming();
}

// Test that without compression, bytes received is greater than total
// payload bytes
TEST_F(RocketCompatibilityTest, RequestResponse_NoCompression) {
  compatibilityTest_->connectToServer([this](auto client) {
    EXPECT_CALL(*compatibilityTest_->handler_.get(), echo_(testing::_));
    static const int kSize = 32 << 10;
    std::string asString(kSize, 'a');
    std::unique_ptr<folly::IOBuf> payload = folly::IOBuf::copyBuffer(asString);
    auto result = client->future_echo(
        RpcOptions().setChecksum(
            apache::thrift::RpcOptions::Checksum::SERVER_ONLY_CRC32),
        *payload);
    EXPECT_EQ(std::move(result).get(), asString);

    auto* channel = dynamic_cast<RocketClientChannel*>(client->getChannel());
    ASSERT_NE(nullptr, channel);
    channel->getEventBase()->runInEventBaseThreadAndWait([&] {
      auto sock = channel->getTransport()
                      ->getUnderlyingTransport<TAsyncSocketIntercepted>();
      ASSERT_NE(nullptr, sock);
      int32_t numRead = sock->getTotalBytesRead();
      // check that compression actually kicked in
      EXPECT_GT(numRead, asString.size());
    });
  });
}

TEST_F(RocketCompatibilityTest, Oneway_Simple) {
  compatibilityTest_->TestOneway_Simple();
}

TEST_F(RocketCompatibilityTest, Oneway_WithDelay) {
  compatibilityTest_->TestOneway_WithDelay();
}

TEST_F(RocketCompatibilityTest, Oneway_Saturation) {
  compatibilityTest_->connectToServer([this](auto client) {
    EXPECT_CALL(*compatibilityTest_->handler_.get(), addAfterDelay_(100, 5));
    EXPECT_CALL(*compatibilityTest_->handler_.get(), addAfterDelay_(50, 5));

    if (auto* channel =
            dynamic_cast<RocketClientChannel*>(client->getChannel())) {
      channel->getEventBase()->runInEventBaseThreadAndWait(
          [&]() { channel->setMaxPendingRequests(0u); });
      EXPECT_THROW(
          client->future_addAfterDelay(0, 5).get(), TTransportException);

      // the first call is not completed as the connection was saturated
      channel->getEventBase()->runInEventBaseThreadAndWait(
          [&]() { channel->setMaxPendingRequests(1u); });
    } else {
      FAIL() << "Test run with unexpected channel type";
    }

    // Client should be able to issue both of these functions as
    // SINGLE_REQUEST_NO_RESPONSE doesn't need to wait for server response
    client->future_addAfterDelay(100, 5).get();
    client->future_addAfterDelay(50, 5).get(); // TODO: H2 fails in this call.

    // This ensure the server to stays alive until the oneway methods have
    // reached it
    client->semifuture_sleep(0).get();
  });
}

TEST_F(RocketCompatibilityTest, Oneway_UnexpectedException) {
  compatibilityTest_->TestOneway_UnexpectedException();
}

TEST_F(RocketCompatibilityTest, Oneway_Connection_CloseNow) {
  compatibilityTest_->TestOneway_Connection_CloseNow();
}

TEST_F(RocketCompatibilityTest, Oneway_ServerQueueTimeout) {
  compatibilityTest_->TestOneway_ServerQueueTimeout();
}

TEST_F(RocketCompatibilityTest, Oneway_Checksumming) {
  compatibilityTest_->TestOneway_Checksumming();
}

TEST_F(RocketCompatibilityTest, Oneway_Sampled_Checksumming) {
  compatibilityTest_->TestOneway_Checksumming(true);
}

TEST_F(RocketCompatibilityTest, RequestContextIsPreserved) {
  compatibilityTest_->TestRequestContextIsPreserved();
}

TEST_F(RocketCompatibilityTest, BadPayload) {
  compatibilityTest_->TestBadPayload();
}

TEST_F(RocketCompatibilityTest, EvbSwitch) {
  compatibilityTest_->TestEvbSwitch();
}

TEST_F(RocketCompatibilityTest, EvbSwitch_Failure) {
  compatibilityTest_->TestEvbSwitch_Failure();
}

TEST_F(RocketCompatibilityTest, CloseCallback) {
  compatibilityTest_->TestCloseCallback();
}

TEST_F(RocketCompatibilityTest, ConnectionStats) {
  compatibilityTest_->TestConnectionStats();
}

TEST_F(RocketCompatibilityTest, ObserverSendReceiveRequests) {
  compatibilityTest_->TestObserverSendReceiveRequests();
}

TEST_F(RocketCompatibilityTest, ConnectionContext) {
  compatibilityTest_->TestConnectionContext();
}

TEST_F(RocketCompatibilityTest, ClientIdentityHook) {
  compatibilityTest_->TestClientIdentityHook();
}

TEST_F(RocketManuallyStartServerTest, TestCustomAsyncProcessor) {
  compatibilityTest_->TestCustomAsyncProcessor();
}

TEST_F(RocketManuallyStartServerTest, TestOnWriteQuiescence) {
  compatibilityTest_->TestOnWriteQuiescence();
}

} // namespace apache::thrift
