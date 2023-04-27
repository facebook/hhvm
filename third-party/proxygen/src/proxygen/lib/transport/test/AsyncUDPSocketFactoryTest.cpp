/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include "proxygen/lib/transport/AsyncUDPSocketFactory.h"

namespace {
folly::SocketAddress getV6TestAddress() {
  return folly::SocketAddress("::1", 0);
}

folly::SocketAddress getV4TestAddress() {
  return folly::SocketAddress("127.0.0.1", 0);
}
} // namespace

namespace proxygen {

class AsyncUDPSocketFactoryTest : public testing::Test {
 public:
  void TearDown() override {
    evb_.loop();
  }

 protected:
  folly::EventBase evb_;
};

TEST_F(AsyncUDPSocketFactoryTest, CreateSuccessV6) {
  AsyncUDPSocketFactory factory(&evb_, getV6TestAddress());

  auto maybeSocket = factory.createSocket(getV6TestAddress());
  EXPECT_FALSE(maybeSocket.hasError());
}

TEST_F(AsyncUDPSocketFactoryTest, CreateSuccessV4) {
  AsyncUDPSocketFactory factory(&evb_, getV6TestAddress(), getV4TestAddress());

  auto maybeSocket = factory.createSocket(getV4TestAddress());
  EXPECT_FALSE(maybeSocket.hasError());
}

TEST_F(AsyncUDPSocketFactoryTest, EmptyV4BindAddresses) {
  AsyncUDPSocketFactory factory(&evb_, getV6TestAddress());
  auto maybeSocket = factory.createSocket(getV4TestAddress());
  EXPECT_TRUE(maybeSocket.hasError());
}

TEST_F(AsyncUDPSocketFactoryTest, BindFailure) {
  AsyncUDPSocketFactory factory(
      &evb_, getV4TestAddress(), folly::SocketAddress("0.13.13.13", 0));
  auto maybeSocket = factory.createSocket(getV4TestAddress());
  EXPECT_TRUE(maybeSocket.hasError());
}

} // namespace proxygen
