/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <wangle/acceptor/Acceptor.h>

#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>

using namespace folly;
using namespace wangle;

class TestConnection : public wangle::ManagedConnection {
 public:
  void timeoutExpired() noexcept override {
  }
  void describe(std::ostream& /*os*/) const override {
  }
  bool isBusy() const override {
    return false;
  }
  void notifyPendingShutdown() override {
  }
  void closeWhenIdle() override {
  }
  void dropConnection(const std::string& /* errorMsg */ = "") override {
    delete this;
  }
  void dumpConnectionState(uint8_t /*loglevel*/) override {
  }

  [[nodiscard]] const folly::SocketAddress& getPeerAddress()
      const noexcept override {
    return dummyAddress;
  }

  folly::SocketAddress dummyAddress;
};

class TestAcceptor : public Acceptor {
 public:
  explicit TestAcceptor(const ServerSocketConfig& accConfig)
      : Acceptor(accConfig) {
  }

  void onNewConnection(folly::AsyncTransport::UniquePtr /*sock*/,
                       const folly::SocketAddress* /*address*/,
                       const std::string& /*nextProtocolName*/,
                       SecureTransportType /*secureTransportType*/,
                       const TransportInfo& /*tinfo*/) override {
    addConnection(new TestConnection);

    getEventBase()->terminateLoopSoon();
  }
};

TEST(AcceptorTest, Basic) {

  EventBase base;
  auto socket = AsyncServerSocket::newSocket(&base);
  ServerSocketConfig config;

  TestAcceptor acceptor(config);
  socket->addAcceptCallback(&acceptor, &base);

  acceptor.init(socket.get(), &base);
  socket->bind(0);
  socket->listen(100);

  SocketAddress addy;
  socket->getAddress(&addy);

  socket->startAccepting();

  auto client_socket = AsyncSocket::newSocket(&base, addy);

  base.loopForever();

  CHECK_EQ(acceptor.getNumConnections(), 1);

  CHECK(acceptor.getState() == Acceptor::State::kRunning);
  acceptor.forceStop();
  socket->stopAccepting();
  base.loop();
}
