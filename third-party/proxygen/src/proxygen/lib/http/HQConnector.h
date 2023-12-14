/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/AsyncFizzClient.h>
#include <folly/io/SocketOptionMap.h>
#include <proxygen/lib/http/session/HQUpstreamSession.h>
#include <quic/api/LoopDetectorCallback.h>
#include <quic/api/QuicSocket.h>
#include <quic/client/QuicClientTransport.h>
#include <quic/common/events/FollyQuicEventBase.h>
#include <quic/fizz/client/handshake/QuicPskCache.h>
#include <quic/logging/QLogger.h>

namespace proxygen {

class HQSession;

/**
 * This class establishes new connections to HTTP servers over a QUIC transport.
 * It can be reused, even to connect to different addresses, but it can only
 * service setting up one connection at a time.
 */
class HQConnector : public HQSession::ConnectCallback {
 public:
  class Callback {
   public:
    virtual ~Callback() {
    }
    virtual void connectSuccess(HQUpstreamSession* session) = 0;
    virtual void connectError(const quic::QuicErrorCode& code) = 0;
  };

  explicit HQConnector(Callback* callback,
                       std::chrono::milliseconds transactionTimeout,
                       bool useConnectionEndWithErrorCallback = false)
      : cb_(CHECK_NOTNULL(callback)),
        transactionTimeout_(transactionTimeout),
        useConnectionEndWithErrorCallback_(useConnectionEndWithErrorCallback) {
  }

  ~HQConnector() override {
    reset();
  }

  void setTransportSettings(quic::TransportSettings transportSettings);
  void setSupportedQuicVersions(
      const std::vector<quic::QuicVersion>& versions) {
    quicVersions_ = versions;
  }
  void setH3Settings(SettingsList settings) {
    h3Settings_ = std::move(settings);
  }

  void setQuicPskCache(std::shared_ptr<quic::QuicPskCache> quicPskCache);

  void reset();

  void connect(
      folly::EventBase* eventBase,
      folly::Optional<folly::SocketAddress> localAddr,
      const folly::SocketAddress& connectAddr,
      std::shared_ptr<const fizz::client::FizzClientContext> fizzContext,
      std::shared_ptr<const fizz::CertificateVerifier> verifier,
      std::chrono::milliseconds connectTimeout = std::chrono::milliseconds(0),
      const folly::SocketOptionMap& socketOptions = folly::emptySocketOptionMap,
      folly::Optional<std::string> sni = folly::none,
      std::shared_ptr<quic::QLogger> qLogger = nullptr,
      std::shared_ptr<quic::LoopDetectorCallback> quicLoopDetectorCallback =
          nullptr,
      std::shared_ptr<quic::QuicTransportStatsCallback>
          quicTransportStatsCallback = nullptr);

  std::chrono::milliseconds timeElapsed();

  bool isBusy() const {
    return (session_ != nullptr);
  }

  // HQSession::ConnectCallback
  void onReplaySafe() noexcept override;
  void connectError(quic::QuicError error) noexcept override;

 private:
  Callback* cb_;
  std::chrono::milliseconds transactionTimeout_;
  TimePoint connectStart_;
  HQUpstreamSession* session_{nullptr};
  quic::TransportSettings transportSettings_;
  std::vector<quic::QuicVersion> quicVersions_;
  folly::Optional<SettingsList> h3Settings_;
  std::shared_ptr<quic::QuicPskCache> quicPskCache_;
  bool useConnectionEndWithErrorCallback_{false};
};

} // namespace proxygen
