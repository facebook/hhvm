/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>
#include <string>

#include <proxygen/httpserver/samples/hq/HQParams.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <quic/server/QuicServer.h>

namespace proxygen {
class HQSession;
}

namespace quic::samples {

using HTTPTransactionHandlerProvider =
    std::function<proxygen::HTTPTransactionHandler*(proxygen::HTTPMessage*)>;

class HQServer {
 public:
  explicit HQServer(
      HQServerParams params,
      HTTPTransactionHandlerProvider httpTransactionHandlerProvider,
      std::function<void(proxygen::HQSession*)> onTransportReadyFn = nullptr);

  // Starts the QUIC transport in background thread
  void start();

  // Returns the listening address of the server
  // NOTE: can block until the server has started
  const folly::SocketAddress getAddress() const;

  // Stops both the QUIC transport AND the HTTP server handling loop
  void stop();

  // Sets/unsets "reject connections" flag on the QUIC server
  void rejectNewConnections(bool reject);

  void setStatsFactory(
      std::unique_ptr<quic::QuicTransportStatsCallbackFactory>&& statsFactory) {
    CHECK(server_);
    server_->setTransportStatsCallbackFactory(std::move(statsFactory));
  }

 private:
  HQServerParams params_;
  std::shared_ptr<quic::QuicServer> server_;
};

class ScopedHQServer {
 public:
  static std::unique_ptr<ScopedHQServer> start(
      const HQServerParams& params,
      HTTPTransactionHandlerProvider handlerProvider) {
    return std::make_unique<ScopedHQServer>(params, std::move(handlerProvider));
  }

  ScopedHQServer(HQServerParams params,
                 HTTPTransactionHandlerProvider handlerProvider)
      : server_(std::move(params), std::move(handlerProvider)) {
    server_.start();
  }

  ~ScopedHQServer() {
    server_.stop();
  }

  [[nodiscard]] const folly::SocketAddress getAddress() const {
    return server_.getAddress();
  }

 private:
  HQServer server_;
};

} // namespace quic::samples
