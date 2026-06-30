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
#include <vector>

#include <folly/io/async/EventBaseLocal.h>
#include <proxygen/httpserver/samples/hq/FizzContext.h>
#include <proxygen/httpserver/samples/hq/HQParams.h>
#include <proxygen/lib/http/codec/H3EarlyDataHandler.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <quic/server/QuicHandshakeSocketHolder.h>
#include <quic/server/QuicServer.h>
#include <quic/state/EarlyDataAppParamsHandler.h>

namespace proxygen {
class HQSession;
}

namespace quic::samples {

using HTTPTransactionHandlerProvider =
    std::function<proxygen::HTTPTransactionHandler*(proxygen::HTTPMessage*)>;

class HQServer {
 public:
  HQServer(HQServerParams params,
           HTTPTransactionHandlerProvider httpTransactionHandlerProvider,
           std::function<void(proxygen::HQSession*)> onTransportReadyFn,
           std::shared_ptr<const fizz::server::FizzServerContext> fizzCtx);

  HQServer(HQServerParams params,
           std::unique_ptr<quic::QuicServerTransportFactory> factory,
           const std::string& certificateFilePath,
           const std::string& keyFilePath,
           fizz::server::ClientAuthMode clientAuth,
           const std::vector<std::string>& supportedAlpns);

  HQServer(HQServerParams params,
           std::unique_ptr<quic::QuicServerTransportFactory> factory,
           std::shared_ptr<const fizz::server::FizzServerContext> fizzCtx);

  // Starts the QUIC transport in background thread
  void start(const folly::SocketAddress& localAddress,
             std::vector<folly::EventBase*> evbs = {});

  // Returns the listening address of the server
  // NOTE: can block until the server has started
  const folly::SocketAddress getAddress() const;

  std::vector<folly::EventBase*> getWorkerEvbs() const noexcept {
    return server_->getWorkerEvbs();
  }

  // Stops both the QUIC transport AND the HTTP server handling loop
  void stop();

  void setStatsFactory(
      std::unique_ptr<quic::QuicTransportStatsCallbackFactory>&& statsFactory) {
    CHECK(server_);
    server_->setTransportStatsCallbackFactory(std::move(statsFactory));
  }

  // Forward a transport-settings override fn to the underlying QuicServer; it
  // runs per-connection. Call before start(). Lets callers gate settings (e.g.
  // behind a JustKnob) without HQServer depending on the gating system.
  void setTransportSettingsOverrideFn(
      quic::QuicServer::TransportSettingsOverrideFn fn) {
    CHECK(server_);
    server_->setTransportSettingsOverrideFn(std::move(fn));
  }

  // Takeover runtime wrapper methods - forward to underlying QuicServer
  // Takeover part 1: Methods called on the old instance.
  void allowBeingTakenOver(const folly::SocketAddress& addr);
  [[nodiscard]] int getTakeoverHandlerSocketFD() const;
  [[nodiscard]] std::vector<int> getAllListeningSocketFDs() const;

  // Takeover part 2: Methods called during the initialization of the new
  // process.
  void setListeningFDs(const std::vector<int>& fds);
  void setProcessId(quic::ProcessId pid);
  void setHostId(uint32_t hostId);
  void setConnectionIdVersion(quic::ConnectionIdVersion version);
  void waitUntilInitialized();

  // Takeover part 3: Methods called during the packet forwarding setup.
  [[nodiscard]] quic::ProcessId getProcessId() const;
  [[nodiscard]] TakeoverProtocolVersion getTakeoverProtocolVersion() const;
  void startPacketForwarding(const folly::SocketAddress& addr);

  // Takeover part 4: Methods called on the old instance to wind down.
  void rejectNewConnections(std::function<bool()> rejectFn);
  void pauseRead();

  void setFizzContext(
      std::shared_ptr<const fizz::server::FizzServerContext> ctx);

  void setFizzContext(
      folly::EventBase* evb,
      std::shared_ptr<const fizz::server::FizzServerContext> ctx);

 private:
  HQServerParams params_;
  std::shared_ptr<quic::QuicServer> server_;
};

class ScopedHQServer {
 public:
  static std::unique_ptr<ScopedHQServer> start(
      const HQServerParams& params,
      const folly::SocketAddress& localAddress,
      HTTPTransactionHandlerProvider handlerProvider,
      const std::string& certificateFilePath,
      const std::string& keyFilePath,
      fizz::server::ClientAuthMode clientAuth,
      const std::vector<std::string>& supportedAlpns) {
    return std::make_unique<ScopedHQServer>(params,
                                            localAddress,
                                            std::move(handlerProvider),
                                            certificateFilePath,
                                            keyFilePath,
                                            clientAuth,
                                            supportedAlpns);
  }

  ScopedHQServer(HQServerParams params,
                 const folly::SocketAddress& localAddress,
                 HTTPTransactionHandlerProvider handlerProvider,
                 const std::string& certificateFilePath,
                 const std::string& keyFilePath,
                 fizz::server::ClientAuthMode clientAuth,
                 const std::vector<std::string>& supportedAlpns);

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
    , public quic::EarlyDataAppParamsHandler
    , private quic::QuicHandshakeSocketHolder::Callback {
 public:
  using QLoggerFactory =
      std::function<std::shared_ptr<quic::QLogger>(quic::VantagePoint)>;

  explicit HQServerTransportFactory(
      const HQServerParams& params,
      HTTPTransactionHandlerProvider httpTransactionHandlerProvider,
      std::function<void(proxygen::HQSession*)> onTransportReadyFn_);
  ~HQServerTransportFactory() override = default;

  void setQLoggerFactory(QLoggerFactory fn) {
    qloggerFactory_ = std::move(fn);
  }

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
  void addAlpnHandler(
      const std::vector<std::string>& alpns,
      const AlpnHandlerFn& handler,
      quic::EarlyDataAppParamsHandler* earlyDataHandler = nullptr) {
    for (auto& alpn : alpns) {
      alpnHandlers_[alpn] = {handler, earlyDataHandler};
    }
  }

  // EarlyDataAppParamsHandler — dispatches to per-ALPN handlers
  bool validate(const quic::Optional<std::string>& alpn,
                const quic::BufPtr& appParams) override;
  quic::BufPtr get() override;

  void setDefaultEarlyDataHandler(quic::EarlyDataAppParamsHandler* handler) {
    defaultEarlyDataHandler_ = handler;
  }

 private:
  bool onQuicWriteCipherAvailable(
      std::shared_ptr<quic::QuicSocket> quicSocket) override;
  void onQuicTransportReady(
      std::shared_ptr<quic::QuicSocket> quicSocket) override;
  void onConnectionSetupError(std::shared_ptr<quic::QuicSocket> quicSocket,
                              quic::QuicError code) override;
  wangle::ConnectionManager* getConnectionManager(folly::EventBase* evb);
  void handleHQAlpn(std::shared_ptr<quic::QuicSocket> quicSocket,
                    wangle::ConnectionManager* connMgr,
                    bool calledFromWriteCipherPath = false);

  // Configuration params
  const HQServerParams& params_;
  // Provider of HTTPTransactionHandler
  HTTPTransactionHandlerProvider httpTransactionHandlerProvider_;
  std::function<void(proxygen::HQSession*)> onTransportReadyFn_;
  folly::EventBaseLocal<wangle::ConnectionManager::UniquePtr> connMgr_;

  struct AlpnEntry {
    AlpnHandlerFn handler;
    quic::EarlyDataAppParamsHandler* earlyDataHandler{nullptr};
  };
  std::map<std::string, AlpnEntry> alpnHandlers_;
  proxygen::H3EarlyDataHandler h3EarlyDataHandler_;
  quic::EarlyDataAppParamsHandler* defaultEarlyDataHandler_{nullptr};
  QLoggerFactory qloggerFactory_;
};

} // namespace quic::samples
