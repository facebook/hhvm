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

#include <folly/io/async/EventBaseLocal.h>
#include <proxygen/httpserver/samples/hq/HQParams.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <quic/server/QuicHandshakeSocketHolder.h>
#include <quic/server/QuicServer.h>

namespace proxygen {
class HQSession;
}

namespace quic::samples {

using HTTPTransactionHandlerProvider =
    std::function<proxygen::HTTPTransactionHandler*(proxygen::HTTPMessage*)>;

class HQServer {
 public:
  HQServer(
      HQServerParams params,
      HTTPTransactionHandlerProvider httpTransactionHandlerProvider,
      std::function<void(proxygen::HQSession*)> onTransportReadyFn = nullptr);

  HQServer(HQServerParams params,
           std::unique_ptr<quic::QuicServerTransportFactory> factory);

  // Starts the QUIC transport in background thread
  void start();

  // Returns the listening address of the server
  // NOTE: can block until the server has started
  const folly::SocketAddress getAddress() const;

  std::vector<folly::EventBase*> getWorkerEvbs() const noexcept {
    return server_->getWorkerEvbs();
  }

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

class HQServerTransportFactory
    : public quic::QuicServerTransportFactory
    , private quic::QuicHandshakeSocketHolder::Callback {
 public:
  explicit HQServerTransportFactory(
      const HQServerParams& params,
      HTTPTransactionHandlerProvider httpTransactionHandlerProvider,
      std::function<void(proxygen::HQSession*)> onTransportReadyFn_);
  ~HQServerTransportFactory() override = default;

  // Creates new quic server transport
  quic::QuicServerTransport::Ptr make(
      folly::EventBase* evb,
      std::unique_ptr<quic::FollyAsyncUDPSocketAlias> socket,
      const folly::SocketAddress& /* peerAddr */,
      quic::QuicVersion quicVersion,
      std::shared_ptr<const fizz::server::FizzServerContext> ctx) noexcept
      override;

  using AlpnHandlerFn = std::function<void(std::shared_ptr<quic::QuicSocket>,
                                           wangle::ConnectionManager*)>;
  void addAlpnHandler(const std::vector<std::string>& alpns,
                      const AlpnHandlerFn& handler) {
    for (auto& alpn : alpns) {
      alpnHandlers_[alpn] = handler;
    }
  }

 private:
  void onQuicTransportReady(
      std::shared_ptr<quic::QuicSocket> quicSocket) override;
  void onConnectionSetupError(std::shared_ptr<quic::QuicSocket> quicSocket,
                              quic::QuicError code) override;
  wangle::ConnectionManager* getConnectionManager(folly::EventBase* evb);
  void handleHQAlpn(std::shared_ptr<quic::QuicSocket> quicSocket,
                    wangle::ConnectionManager* connMgr);

  // Configuration params
  const HQServerParams& params_;
  // Provider of HTTPTransactionHandler
  HTTPTransactionHandlerProvider httpTransactionHandlerProvider_;
  std::function<void(proxygen::HQSession*)> onTransportReadyFn_;
  folly::EventBaseLocal<wangle::ConnectionManager::UniquePtr> connMgr_;
  std::map<std::string, AlpnHandlerFn> alpnHandlers_;
};

} // namespace quic::samples
