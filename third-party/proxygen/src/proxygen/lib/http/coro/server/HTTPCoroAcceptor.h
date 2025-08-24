/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/ProxygenErrorEnum.h"
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>
#include <proxygen/lib/services/AcceptorConfiguration.h>
#include <wangle/acceptor/Acceptor.h>

namespace proxygen {
class HTTPCodecFactory;
class HTTPCodec;
class HTTPSessionStats;
} // namespace proxygen

namespace quic {
class QuicSocket;
}

namespace proxygen::coro {

class HTTPHandler;
class LifecycleObserver;
class HTTPCoroSession;

class HTTPCoroDownstreamSessionFactory {
 public:
  struct Config {
    Config() {
    }
    LifecycleObserver* sessionLifecycleCb{nullptr};
    HTTPSessionStats* sessionStats{nullptr};
    HeaderCodec::Stats* headerCodecStats{nullptr};
  };

  explicit HTTPCoroDownstreamSessionFactory(
      std::shared_ptr<const AcceptorConfiguration> accConfig,
      std::shared_ptr<HTTPCodecFactory> codecFactory = nullptr,
      Config config = Config());

  void setConfig(const Config& config) {
    config_ = config;
  }

  HTTPCoroSession* FOLLY_NULLABLE
  makeUniplexSession(folly::AsyncTransport::UniquePtr sock,
                     const folly::SocketAddress* address,
                     const std::string& nextProtocol,
                     wangle::SecureTransportType secureTransportType,
                     const wangle::TransportInfo& tinfo,
                     std::shared_ptr<HTTPHandler> handler);

  HTTPCoroSession* makeQuicSession(std::shared_ptr<quic::QuicSocket> quicSocket,
                                   wangle::TransportInfo tinfo,
                                   std::shared_ptr<HTTPHandler> handler,
                                   bool strictValidation);

 private:
  void applySettingsToCodec(HTTPCodec& codec);
  void applySettingsToSession(HTTPCoroSession& session);

  std::shared_ptr<const AcceptorConfiguration> accConfig_;
  std::shared_ptr<HTTPCodecFactory> codecFactory_;
  Config config_;
};

class HTTPCoroAcceptor : public wangle::Acceptor {

 public:
  /**
   * Type of function to run inside onNewConnection() of acceptors.
   * If the function throws or returns false, the socket will be closed
   * immediately. Useful for validating client cert before processing the
   * request.
   */
  using NewConnectionFilter =
      folly::Function<bool(const folly::SocketAddress* /* address */,
                           const folly::AsyncTransportCertificate* /*peerCert*/,
                           const std::string& /* nextProtocolName */,
                           SecureTransportType /* secureTransportType */,
                           const wangle::TransportInfo& /* tinfo */) const>;

  explicit HTTPCoroAcceptor(
      std::shared_ptr<const AcceptorConfiguration> accConfig,
      std::shared_ptr<HTTPHandler> handler,
      NewConnectionFilter* newConnectionFilter = nullptr,
      std::shared_ptr<HTTPCodecFactory> codecFactory = nullptr);
  ~HTTPCoroAcceptor() override {
  }

  void init(folly::AsyncServerSocket* serverSocket,
            folly::EventBase* eventBase,
            wangle::SSLStats* stats = nullptr,
            std::shared_ptr<const fizz::server::FizzServerContext> fizzContext =
                nullptr) override {
    if (eventBase) {
      auto keepAlive = keepAlive_.wlock();
      *keepAlive = eventBase;
    }
    Acceptor::init(serverSocket, eventBase, stats, fizzContext);
  }

  // TODO: custom error pages
  // Priorities enabled?

  void onNewConnection(std::shared_ptr<quic::QuicSocket> quicSocket,
                       wangle::TransportInfo tinfo);

  void setOnConnectionDrainedFn(std::function<void()> fn) {
    onConnectionsDrainedFn_ = std::move(fn);
  }

  void stopAcceptingQuic() {
    acceptStopped();
  }

  // Returns an EventBase KeepAlive for this Acceptor's EventBase if the
  // Acceptor has not yet drained.  After drain, it returns an empty KeepAlive.
  // The caller (from any thread) can use this to determine if this Acceptor
  // needs to be drained or stopped.
  folly::Executor::KeepAlive<folly::EventBase> getEventBaseKeepalive() {
    return *keepAlive_.rlock();
  }

 protected:
  void onSessionReady(folly::EventBase* eventBase, HTTPCoroSession* session);
  // HTTPSessionStats* downstreamSessionStats_{nullptr};

  // Acceptor methods
  void onNewConnection(folly::AsyncTransport::UniquePtr sock,
                       const folly::SocketAddress* address,
                       const std::string& nextProtocol,
                       wangle::SecureTransportType secureTransportType,
                       const wangle::TransportInfo& tinfo) override;

  virtual void onSessionCreationError(ProxygenError /*error*/) {
  }

  void onConnectionsDrained() override {
    {
      auto keepAlive = keepAlive_.wlock();
      keepAlive->reset();
    }
    if (onConnectionsDrainedFn_) {
      onConnectionsDrainedFn_();
    }
  }

 private:
  HTTPCoroAcceptor(const HTTPCoroAcceptor&) = delete;
  HTTPCoroAcceptor& operator=(const HTTPCoroAcceptor&) = delete;

  HTTPCoroDownstreamSessionFactory factory_;
  std::shared_ptr<HTTPHandler> handler_;
  NewConnectionFilter* newConnectionFilter_{nullptr};
  std::function<void()> onConnectionsDrainedFn_;
  folly::Synchronized<folly::Executor::KeepAlive<folly::EventBase>> keepAlive_;
};

} // namespace proxygen::coro
