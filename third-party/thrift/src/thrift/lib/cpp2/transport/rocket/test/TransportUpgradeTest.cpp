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

#include <gmock/gmock.h>
#include <folly/fibers/FiberManagerMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/TestUtil.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/gen-cpp2/TransportUpgrade.h>

THRIFT_FLAG_DECLARE_int64(raw_client_rocket_upgrade_timeout_ms);

namespace apache::thrift {

class TransportUpgradeService
    : public apache::thrift::ServiceHandler<TransportUpgrade> {
 public:
  MOCK_METHOD(void, noResponse, (std::unique_ptr<std::string>), (override));

  int32_t addTwoNumbers(int32_t num1, int32_t num2) override {
    return num1 + num2;
  }

  int32_t add(int32_t x) override {
    sum += x;
    return sum;
  }

 protected:
  std::atomic<int32_t> sum{0};
};

// Testing transport layers for upgrading from header to rocket
class TransportUpgradeTest : public TestSetup {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<TransportUpgradeService>();
    std::string transport = "header";
    server_ = createServer(
        std::make_shared<
            ThriftServerAsyncProcessorFactory<TransportUpgradeService>>(
            handler_),
        port_,
        0 /*maxRequests*/,
        transport);
  }

  void TearDown() override {
    if (server_) {
      server_->cleanUp();
      server_.reset();
      handler_.reset();
    }
  }

  void testRawClientRocketUpgradeSync(
      HeaderClientChannel::Options options = HeaderClientChannel::Options()) {
    folly::EventBase evb;
    auto socket = folly::AsyncSocket::newSocket(&evb, "::1", port_);
    auto channel =
        HeaderClientChannel::newChannel(std::move(socket), std::move(options));

    auto client =
        std::make_unique<TransportUpgradeAsyncClient>(std::move(channel));
    auto result = client->sync_addTwoNumbers(13, 42);
    EXPECT_EQ(55, result);

    auto* upgradeChannel =
        dynamic_cast<HeaderClientChannel::RocketUpgradeChannel*>(
            client->getChannel());
    ASSERT_NE(nullptr, upgradeChannel);
    ASSERT_NE(nullptr, upgradeChannel->rocketChannel_);
  }

  void testRawClientRocketUpgradeAsync(
      HeaderClientChannel::Options options = HeaderClientChannel::Options()) {
    folly::EventBase evb;
    std::vector<folly::SemiFuture<folly::Unit>> futures;

    auto channel = HeaderClientChannel::newChannel(
        folly::AsyncSocket::newSocket(&evb, "::1", port_), std::move(options));
    auto client =
        std::make_unique<TransportUpgradeAsyncClient>(std::move(channel));
    auto f1 = client->semifuture_addTwoNumbers(1, 2).via(&evb).thenTry(
        [&](auto&& response) {
          EXPECT_TRUE(response.hasValue());
          EXPECT_EQ(3, response.value());
        });
    futures.emplace_back(std::move(f1));
    auto f2 = client->semifuture_add(1).via(&evb).thenTry([&](auto&& response) {
      EXPECT_TRUE(response.hasValue());
      EXPECT_LE(1, response.value());
      EXPECT_GE(8, response.value());
    });
    futures.emplace_back(std::move(f2));
    auto f3 = client->semifuture_add(2).via(&evb).thenTry([&](auto&& response) {
      EXPECT_TRUE(response.hasValue());
      EXPECT_LE(2, response.value());
      EXPECT_GE(8, response.value());
    });
    futures.emplace_back(std::move(f3));
    auto f4 = client->semifuture_addTwoNumbers(42, 15).via(&evb).thenTry(
        [&](auto&& response) {
          EXPECT_TRUE(response.hasValue());
          EXPECT_EQ(57, response.value());
        });
    futures.emplace_back(std::move(f4));
    auto f5 = client->semifuture_add(5).via(&evb).thenTry([&](auto&& response) {
      EXPECT_TRUE(response.hasValue());
      EXPECT_LE(5, response.value());
      EXPECT_GE(8, response.value());
    });
    futures.emplace_back(std::move(f5));
    auto f6 = client->semifuture_noResponse("test").via(&evb);
    futures.emplace_back(std::move(f6));

    folly::collectAllUnsafe(std::move(futures)).getVia(&evb);

    auto* upgradeChannel =
        dynamic_cast<HeaderClientChannel::RocketUpgradeChannel*>(
            client->getChannel());
    ASSERT_NE(nullptr, upgradeChannel);
    ASSERT_NE(nullptr, upgradeChannel->rocketChannel_);
  }

 protected:
  std::unique_ptr<ThriftServer> server_;
  std::shared_ptr<TransportUpgradeService> handler_;
  uint16_t port_;
};

TEST_F(TransportUpgradeTest, RawClientRocketUpgradeSyncEnabled) {
  testRawClientRocketUpgradeSync();
}

TEST_F(TransportUpgradeTest, RawClientRocketUpgradeAsyncEnabled) {
  testRawClientRocketUpgradeAsync();
}

TEST_F(TransportUpgradeTest, RawClientRocketUpgradeSyncEnabled_BinaryProtocol) {
  testRawClientRocketUpgradeSync(HeaderClientChannel::Options().setProtocolId(
      protocol::T_BINARY_PROTOCOL));
}

TEST_F(
    TransportUpgradeTest, RawClientRocketUpgradeAsyncEnabled_BinaryProtocol) {
  testRawClientRocketUpgradeAsync(HeaderClientChannel::Options().setProtocolId(
      protocol::T_BINARY_PROTOCOL));
}

TEST_F(TransportUpgradeTest, RawClientRocketUpgradeOneway) {
  EXPECT_CALL(*handler_.get(), noResponse(_)).Times(1);

  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb, "::1", port_);
  auto channel = HeaderClientChannel::newChannel(std::move(socket));

  auto client =
      std::make_unique<TransportUpgradeAsyncClient>(std::move(channel));
  // oneway call
  client->sync_noResponse("test");

  EXPECT_EQ(84, client->sync_addTwoNumbers(11, 73));

  auto* upgradeChannel =
      dynamic_cast<HeaderClientChannel::RocketUpgradeChannel*>(
          client->getChannel());
  ASSERT_NE(nullptr, upgradeChannel);
  ASSERT_NE(nullptr, upgradeChannel->rocketChannel_);
}

TEST_F(TransportUpgradeTest, RawClientNoUpgrade) {
  folly::EventBase evb;
  auto socket = folly::AsyncSocket::newSocket(&evb, "::1", port_);
  // specifically request no upgrade
  auto channel = HeaderClientChannel::newChannel(
      HeaderClientChannel::WithoutRocketUpgrade{}, std::move(socket));

  auto client =
      std::make_unique<TransportUpgradeAsyncClient>(std::move(channel));
  EXPECT_EQ(84, client->sync_addTwoNumbers(11, 73));

  auto* headerChannel =
      dynamic_cast<HeaderClientChannel*>(client->getChannel());
  ASSERT_NE(nullptr, headerChannel);
}

TEST_F(TransportUpgradeTest, RawClientRocketUpgradeTimeout) {
  folly::EventBase evb;
  auto* slowWritingSocket =
      new SlowWritingSocket(&evb, folly::SocketAddress("::1", port_));
  auto channel = HeaderClientChannel::newChannel(
      folly::AsyncSocket::UniquePtr(slowWritingSocket));
  TransportUpgradeAsyncClient client(std::move(channel));

  // Hold off on writing any requests so the upgrade request times out.
  slowWritingSocket->delayWritingAfterFirstNBytes(1);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  auto sf =
      folly::makeSemiFuture()
          .delayed(std::chrono::milliseconds(150))
          .via(&evb)
          .thenValue([&](auto&&) { slowWritingSocket->flushBufferedWrites(); });
  futures.emplace_back(std::move(sf));

  THRIFT_FLAG_SET_MOCK(raw_client_rocket_upgrade_timeout_ms, 100);

  sf = client.semifuture_addTwoNumbers(41, 19).via(&evb).thenTry(
      [&](auto&& response) {
        EXPECT_TRUE(response.hasException());
        EXPECT_TRUE(
            response.exception()
                .template is_compatible_with<transport::TTransportException>());
        response.exception()
            .template with_exception<transport::TTransportException>(
                [](const auto& tex) {
                  EXPECT_EQ(
                      transport::TTransportException::TTransportExceptionType::
                          TIMED_OUT,
                      tex.getType());
                });

        // upgrade should not have succeeded
        auto* upgradeChannel =
            dynamic_cast<HeaderClientChannel::RocketUpgradeChannel*>(
                client.getChannel());
        ASSERT_NE(nullptr, upgradeChannel);
        ASSERT_EQ(nullptr, upgradeChannel->rocketChannel_);
      });
  futures.emplace_back(std::move(sf));

  folly::collectAllUnsafe(std::move(futures)).getVia(&evb);
}

TEST_F(TransportUpgradeTest, Fibers) {
  folly::EventBase evb;
  auto& fm = folly::fibers::getFiberManager(evb);
  EXPECT_EQ(
      42, fm.addTaskFuture([&] {
              folly::EventBase evb2;
              auto socket = folly::AsyncSocket::newSocket(&evb2, "::1", port_);
              auto channel = RocketClientChannel::newChannel(std::move(socket));
              TransportUpgradeAsyncClient client(std::move(channel));
              return client.sync_addTwoNumbers(41, 1);
            }).getVia(&evb));
  EXPECT_EQ(
      42, fm.addTaskFuture([&] {
              folly::EventBase evb2;
              auto socket = folly::AsyncSocket::newSocket(&evb2, "::1", port_);
              auto channel = HeaderClientChannel::newChannel(std::move(socket));
              TransportUpgradeAsyncClient client(std::move(channel));
              return client.sync_addTwoNumbers(40, 2);
            }).getVia(&evb));
}

} // namespace apache::thrift
