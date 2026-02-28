/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SocketConnector.h"

#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/DelayedDestruction.h>

#include <mcrouter/lib/AuxiliaryIOThreadPool.h>

namespace facebook {
namespace memcache {

namespace {
class ConnectHelper : public folly::AsyncSocket::ConnectCallback,
                      public folly::DelayedDestruction {
 public:
  explicit ConnectHelper(folly::AsyncSocket::UniquePtr socket)
      : socket_(std::move(socket)) {}

  folly::Future<folly::AsyncSocket::UniquePtr> connect(
      folly::SocketAddress address,
      int timeout,
      folly::SocketOptionMap options) {
    folly::DelayedDestruction::DestructorGuard dg(this);
    socket_->connect(this, address, timeout, options);
    return promise_.getFuture();
  }

  void connectSuccess() noexcept override {
    promise_.setValue(std::move(socket_));
    destroy();
  }

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    promise_.setException(ex);
    destroy();
  }

  folly::AsyncSocket* getSocket() {
    return socket_.get();
  }

 private:
  // the async socket to connect
  folly::AsyncSocket::UniquePtr socket_;
  // promise we will fullfill
  folly::Promise<folly::AsyncSocket::UniquePtr> promise_;
};

} // namespace

folly::Future<folly::AsyncSocket::UniquePtr> connectSSLSocketWithAuxIO(
    folly::AsyncSSLSocket::UniquePtr socket,
    folly::SocketAddress address,
    int timeout,
    folly::SocketOptionMap options) {
  // helper will clean itself up after connect callbacks
  auto helper = new ConnectHelper(std::move(socket));
  auto auxPool = mcrouter::AuxiliaryIOThreadPoolSingleton::try_get_fast();
  if (auxPool) {
    auto* socketEvb = helper->getSocket()->getEventBase();
    auto* poolEvb = auxPool->getThreadPool().getEventBase();
    helper->getSocket()->detachEventBase();
    return via(poolEvb)
        .thenValue([opts = std::move(options),
                    addr = std::move(address),
                    helper,
                    poolEvb,
                    timeout](auto&&) {
          helper->getSocket()->attachEventBase(poolEvb);
          return helper->connect(addr, timeout, opts);
        })
        .thenValue([](folly::AsyncSocket::UniquePtr sock) {
          sock->detachEventBase();
          return sock;
        })
        .via(socketEvb)
        .thenValue([socketEvb](folly::AsyncSocket::UniquePtr sock) {
          sock->attachEventBase(socketEvb);
          return sock;
        });
  } else {
    // pool isn't available.  We can still connect inline
    return helper->connect(std::move(address), timeout, std::move(options));
  }
}

} // namespace memcache
} // namespace facebook
