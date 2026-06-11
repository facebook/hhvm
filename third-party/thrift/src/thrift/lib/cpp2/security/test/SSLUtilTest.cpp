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

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/FizzServerContext.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/security/SSLUtil.h>

using namespace apache::thrift;

class ToFDSocketZeroCopyTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto serverSock = folly::netops::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(serverSock, folly::NetworkSocket());

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    ASSERT_EQ(
        0,
        folly::netops::bind(serverSock, (struct sockaddr*)&addr, sizeof(addr)));
    ASSERT_EQ(0, folly::netops::listen(serverSock, 1));

    socklen_t len = sizeof(addr);
    ASSERT_EQ(
        0,
        folly::netops::getsockname(serverSock, (struct sockaddr*)&addr, &len));

    clientFd_ = folly::netops::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(clientFd_, folly::NetworkSocket());
    ASSERT_EQ(
        0,
        folly::netops::connect(
            clientFd_, (struct sockaddr*)&addr, sizeof(addr)));

    serverFd_ = folly::netops::accept(serverSock, nullptr, nullptr);
    ASSERT_NE(serverFd_, folly::NetworkSocket());
    folly::netops::close(serverSock);
  }

  void TearDown() override {
    if (serverFd_ != folly::NetworkSocket()) {
      folly::netops::close(serverFd_);
    }
    if (clientFd_ != folly::NetworkSocket()) {
      folly::netops::close(clientFd_);
    }
  }

  void testClient(bool enableZeroCopy) {
    auto sock = folly::AsyncSocket::newSocket(&evb_, clientFd_);
    clientFd_ = folly::NetworkSocket();
    if (enableZeroCopy) {
      sock->setZeroCopy(true);
      if (!sock->getZeroCopy()) {
        // SO_ZEROCOPY not supported on this platform; nothing to verify.
        return;
      }
    }
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    auto fizzClient = fizz::client::AsyncFizzClient::UniquePtr(
        new fizz::client::AsyncFizzClient(std::move(sock), std::move(ctx)));
    auto result = toFDSocket(fizzClient.get(), kSecurityProtocolStopTLS);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getZeroCopy(), enableZeroCopy);
  }

  void testServer(bool enableZeroCopy) {
    auto sock = folly::AsyncSocket::newSocket(&evb_, serverFd_);
    serverFd_ = folly::NetworkSocket();
    if (enableZeroCopy) {
      sock->setZeroCopy(true);
      if (!sock->getZeroCopy()) {
        // SO_ZEROCOPY not supported on this platform; nothing to verify.
        return;
      }
    }
    auto serverCtx = std::make_shared<fizz::server::FizzServerContext>();
    auto fizzServer = fizz::server::AsyncFizzServer::UniquePtr(
        new fizz::server::AsyncFizzServer(std::move(sock), serverCtx));
    auto result = toFDSocket(fizzServer.get(), kSecurityProtocolStopTLS);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->getZeroCopy(), enableZeroCopy);
  }

  folly::NetworkSocket serverFd_;
  folly::NetworkSocket clientFd_;
  folly::EventBase evb_;
};

TEST_F(ToFDSocketZeroCopyTest, PreservesZeroCopyOnClient) {
  testClient(true);
}

TEST_F(ToFDSocketZeroCopyTest, NoZeroCopyOnClient) {
  testClient(false);
}

TEST_F(ToFDSocketZeroCopyTest, PreservesZeroCopyOnServer) {
  testServer(true);
}

TEST_F(ToFDSocketZeroCopyTest, NoZeroCopyOnServer) {
  testServer(false);
}

TEST_F(ToFDSocketZeroCopyTest, NullInputReturnsNull) {
  fizz::client::AsyncFizzClient* nullClient = nullptr;
  auto result = toFDSocket(nullClient, kSecurityProtocolStopTLS);
  EXPECT_EQ(result, nullptr);
}
