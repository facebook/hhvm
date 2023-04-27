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

#pragma once

#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncUDPServerSocket.h>
#include <folly/io/async/EventBaseManager.h>
#include <wangle/acceptor/Acceptor.h>

namespace wangle {

class ServerSocketFactory {
 public:
  virtual std::shared_ptr<folly::AsyncSocketBase> newSocket(
      folly::SocketAddress address,
      int backlog,
      bool reuse,
      const ServerSocketConfig& config) = 0;

  virtual void removeAcceptCB(
      std::shared_ptr<folly::AsyncSocketBase> sock,
      Acceptor* callback,
      folly::EventBase* base) = 0;

  virtual void addAcceptCB(
      std::shared_ptr<folly::AsyncSocketBase> sock,
      Acceptor* callback,
      folly::EventBase* base) = 0;

  virtual ~ServerSocketFactory() = default;
};

class AsyncServerSocketFactory : public ServerSocketFactory {
 public:
  std::shared_ptr<folly::AsyncSocketBase> newSocket(
      folly::SocketAddress address,
      int /*backlog*/,
      bool reuse,
      const ServerSocketConfig& config) override {
    auto* evb = folly::EventBaseManager::get()->getEventBase();
    std::shared_ptr<folly::AsyncServerSocket> socket(
        new folly::AsyncServerSocket(evb), ThreadSafeDestructor());
    if (config.useZeroCopy) {
      socket->setZeroCopy(true);
    }
    socket->setMaxNumMessagesInQueue(config.maxNumPendingConnectionsPerWorker);
    socket->setReusePortEnabled(reuse);
    if (config.enableTCPFastOpen) {
      socket->setTFOEnabled(true, config.fastOpenQueueSize);
    }
    socket->bind(address);

    socket->listen(config.acceptBacklog);
    socket->startAccepting();

    return socket;
  }

  void removeAcceptCB(
      std::shared_ptr<folly::AsyncSocketBase> s,
      Acceptor* callback,
      folly::EventBase* base) override {
    auto socket = std::dynamic_pointer_cast<folly::AsyncServerSocket>(s);
    CHECK(socket);
    socket->removeAcceptCallback(callback, base);
  }

  void addAcceptCB(
      std::shared_ptr<folly::AsyncSocketBase> s,
      Acceptor* callback,
      folly::EventBase* base) override {
    auto socket = std::dynamic_pointer_cast<folly::AsyncServerSocket>(s);
    CHECK(socket);
    socket->addAcceptCallback(callback, base);
  }

  class ThreadSafeDestructor {
   public:
    void operator()(folly::AsyncServerSocket* socket) const {
      folly::EventBase* evb = socket->getEventBase();
      if (evb) {
        evb->runImmediatelyOrRunInEventBaseThreadAndWait(
            [socket]() { socket->destroy(); });
      } else {
        socket->destroy();
      }
    }
  };
};

class AsyncUDPServerSocketFactory : public ServerSocketFactory {
 public:
  std::shared_ptr<folly::AsyncSocketBase> newSocket(
      folly::SocketAddress address,
      int /*backlog*/,
      bool reuse,
      const ServerSocketConfig& /*config*/) override {
    folly::EventBase* evb = folly::EventBaseManager::get()->getEventBase();
    std::shared_ptr<folly::AsyncUDPServerSocket> socket(
        new folly::AsyncUDPServerSocket(evb), ThreadSafeDestructor());
    socket->setReusePort(reuse);
    socket->bind(address);
    socket->listen();

    return socket;
  }

  void removeAcceptCB(
      std::shared_ptr<folly::AsyncSocketBase> /*s*/,
      Acceptor* /*callback*/,
      folly::EventBase* /*base*/) override {}

  void addAcceptCB(
      std::shared_ptr<folly::AsyncSocketBase> s,
      Acceptor* callback,
      folly::EventBase* base) override {
    auto socket = std::dynamic_pointer_cast<folly::AsyncUDPServerSocket>(s);
    DCHECK(socket);
    socket->addListener(base, callback);
  }

  class ThreadSafeDestructor {
   public:
    void operator()(folly::AsyncUDPServerSocket* socket) const {
      socket->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
          [socket]() { delete socket; });
    }
  };
};

} // namespace wangle
