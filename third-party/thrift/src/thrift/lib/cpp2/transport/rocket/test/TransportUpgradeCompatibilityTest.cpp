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

#include <thrift/lib/cpp2/transport/core/testutil/MockCallback.h>
#include <thrift/lib/cpp2/transport/core/testutil/TransportCompatibilityTest.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>

DECLARE_string(transport); // ConnectionManager depends on this flag.

namespace apache::thrift {

using namespace testutil::testservice;
using namespace apache::thrift::transport;

class TransportUpgradeCompatibilityTest : public testing::TestWithParam<bool> {
 public:
  TransportUpgradeCompatibilityTest() {
    FLAGS_transport = "header";

    compatibilityTest_ = std::make_unique<TransportCompatibilityTest>();
    compatibilityTest_->setTransportUpgradeExpected(GetParam());
    compatibilityTest_->startServer();
  }

 protected:
  std::unique_ptr<TransportCompatibilityTest> compatibilityTest_;
};

INSTANTIATE_TEST_CASE_P(
    NoUpgrade, TransportUpgradeCompatibilityTest, testing::Values(false));
INSTANTIATE_TEST_CASE_P(
    Upgrade, TransportUpgradeCompatibilityTest, testing::Values(true));

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_Simple) {
  compatibilityTest_->TestRequestResponse_Simple();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_Sync) {
  compatibilityTest_->TestRequestResponse_Sync();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_Destruction) {
  compatibilityTest_->TestRequestResponse_Destruction();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_MultipleClients) {
  compatibilityTest_->TestRequestResponse_MultipleClients();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_ExpectedException) {
  compatibilityTest_->TestRequestResponse_ExpectedException();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_UnexpectedException) {
  compatibilityTest_->TestRequestResponse_UnexpectedException();
}

// Warning: This test may be flaky due to use of timeouts.
TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_Timeout) {
  compatibilityTest_->TestRequestResponse_Timeout();
}

TEST_P(TransportUpgradeCompatibilityTest, DefaultTimeoutValueTest) {
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

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_Header) {
  compatibilityTest_->TestRequestResponse_Header();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_Header_Load) {
  compatibilityTest_->TestRequestResponse_Header_Load();
}

TEST_P(
    TransportUpgradeCompatibilityTest,
    RequestResponse_Header_ExpectedException) {
  compatibilityTest_->TestRequestResponse_Header_ExpectedException();
}

TEST_P(
    TransportUpgradeCompatibilityTest,
    RequestResponse_Header_UnexpectedException) {
  compatibilityTest_->TestRequestResponse_Header_UnexpectedException();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_IsOverloaded) {
  compatibilityTest_->TestRequestResponse_IsOverloaded();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_Connection_CloseNow) {
  compatibilityTest_->connectToServer([](auto client) {
    // It should not reach to server: no EXPECT_CALL for add_(3)

    // Observe the behavior if the connection is closed already
    auto channel = static_cast<ClientChannel*>(client->getChannel());
    channel->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { channel->closeNow(); });

    try {
      client->future_add(3).get();
      EXPECT_TRUE(false) << "future_add should have thrown";
    } catch (TTransportException& ex) {
      EXPECT_EQ(TTransportException::UNKNOWN, ex.getType());
      EXPECT_PRED_FORMAT2(IsSubstring, "Channel is !good()", ex.what());
    }
  });
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_ServerQueueTimeout) {
  compatibilityTest_->TestRequestResponse_ServerQueueTimeout();
}

TEST_P(TransportUpgradeCompatibilityTest, RequestResponse_ResponseSizeTooBig) {
  compatibilityTest_->TestRequestResponse_ResponseSizeTooBig();
}

// TODO(T90625074)
TEST_P(
    TransportUpgradeCompatibilityTest, DISABLED_RequestResponse_Checksumming) {
  // Checksum not implemented for header transport
  if (!GetParam()) {
    return;
  }
  compatibilityTest_->TestRequestResponse_Checksumming();
}

TEST_P(TransportUpgradeCompatibilityTest, Oneway_Simple) {
  compatibilityTest_->TestOneway_Simple();
}

TEST_P(TransportUpgradeCompatibilityTest, Oneway_WithDelay) {
  compatibilityTest_->TestOneway_WithDelay();
}

TEST_P(TransportUpgradeCompatibilityTest, Oneway_UnexpectedException) {
  compatibilityTest_->TestOneway_UnexpectedException();
}

TEST_P(TransportUpgradeCompatibilityTest, Oneway_Connection_CloseNow) {
  compatibilityTest_->TestOneway_Connection_CloseNow();
}

TEST_P(TransportUpgradeCompatibilityTest, Oneway_ServerQueueTimeout) {
  compatibilityTest_->TestOneway_ServerQueueTimeout();
}

TEST_P(TransportUpgradeCompatibilityTest, Oneway_Checksumming) {
  // Checksum not implemented for header transport
  if (!GetParam()) {
    return;
  }
  compatibilityTest_->TestOneway_Checksumming();
}

TEST_P(TransportUpgradeCompatibilityTest, Oneway_Sampled_Checksumming) {
  // Checksum not implemented for header transport
  if (!GetParam()) {
    return;
  }
  compatibilityTest_->TestOneway_Checksumming(true);
}

TEST_P(TransportUpgradeCompatibilityTest, RequestContextIsPreserved) {
  compatibilityTest_->TestRequestContextIsPreserved();
}

TEST_P(TransportUpgradeCompatibilityTest, BadPayload) {
  compatibilityTest_->TestBadPayload();
}

TEST_P(TransportUpgradeCompatibilityTest, EvbSwitch) {
  compatibilityTest_->TestEvbSwitch();
}

TEST_P(TransportUpgradeCompatibilityTest, EvbSwitch_Failure) {
  compatibilityTest_->TestEvbSwitch_Failure();
}

class CloseCallbackTest : public CloseCallback {
 public:
  void channelClosed() override {
    EXPECT_FALSE(closed_);
    closed_ = true;
  }
  bool isClosed() { return closed_; }

 private:
  bool closed_{false};
};

TEST_P(TransportUpgradeCompatibilityTest, CloseCallback) {
  compatibilityTest_->connectToServer(
      [this](std::unique_ptr<TestServiceAsyncClient> client) {
        EXPECT_CALL(*compatibilityTest_->handler_.get(), sumTwoNumbers_(1, 2))
            .Times(1);

        auto closeCb = std::make_unique<CloseCallbackTest>();
        auto channel = static_cast<ClientChannel*>(client->getChannel());
        auto evb = channel->getEventBase();
        evb->runInEventBaseThreadAndWait(
            [&]() { channel->setCloseCallback(closeCb.get()); });
        // send a request so that transport upgrade kicks in (if enabled)
        EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());

        EXPECT_FALSE(closeCb->isClosed());
        evb->runInEventBaseThreadAndWait([&]() { channel->closeNow(); });
        EXPECT_TRUE(closeCb->isClosed());
      });
}

TEST_P(TransportUpgradeCompatibilityTest, ConnectionStats) {
  compatibilityTest_->TestConnectionStats();
}

TEST_P(TransportUpgradeCompatibilityTest, ObserverSendReceiveRequests) {
  compatibilityTest_->TestObserverSendReceiveRequests();
}

TEST_P(TransportUpgradeCompatibilityTest, ConnectionContext) {
  compatibilityTest_->TestConnectionContext();
}

TEST_P(TransportUpgradeCompatibilityTest, ClientIdentityHook) {
  compatibilityTest_->TestClientIdentityHook();
}

} // namespace apache::thrift
