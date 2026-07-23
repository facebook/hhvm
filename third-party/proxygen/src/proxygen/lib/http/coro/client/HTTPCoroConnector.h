/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/FizzClientContext.h>
#include <fizz/protocol/CertificateVerifier.h>
#include <folly/SocketAddress.h>
#include <folly/coro/Task.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/SSLContext.h>
#include <folly/ssl/SSLSession.h>
#include <quic/api/LoopDetectorCallback.h>
#include <quic/fizz/client/handshake/QuicPskCache.h>
#include <quic/logging/QLogger.h>
#include <quic/state/QuicTransportStatsCallback.h>
#include <quic/state/TransportSettings.h>
#include <string>

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/client/ProxygenCertVerifier.h"
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>
#include <proxygen/lib/sampling/Sampling.h>

namespace wangle {
class SSLStats;
}

namespace quic {
class QuicClientTransport;
}

namespace proxygen {
extern const std::string empty_string;
class HTTPSessionStats;
class HeaderIndexingStrategy;
namespace coro {

class HTTPCoroSession;
class LifecycleObserver;

/**
 * Asynchronously establish a new connection to the specified server, and
 * construct an HTTPCoroSession around it, using the negotiated protocol.
 */
class HTTPCoroConnector {
 public:
  class SslSessionManagerIf {
   public:
    virtual ~SslSessionManagerIf() = default;
    using SslSessionPtr = std::shared_ptr<folly::ssl::SSLSession>;
    virtual void onNewSslSession(SslSessionPtr session) noexcept = 0;
    virtual SslSessionPtr getSslSession() noexcept = 0;
  };

  struct TLSParams {
    explicit TLSParams(std::list<std::string> inNextProtos = {"h2", "http/1.1"})
        : nextProtocols(std::move(inNextProtos)) {
    }
    std::vector<std::string> caPaths;
    std::string clientCertPath;
    std::string clientKeyPath;
    // TODO: other fancy TLS stuff
    bool earlyData{false}; // Support for TCP?
    std::list<std::string> nextProtocols;
    std::shared_ptr<fizz::client::PskCache> pskCache;
  };

  static const TLSParams& defaultTLSParams() {
    static const TLSParams tlsParams;
    return tlsParams;
  }

  struct FizzContextAndVerifier {
    std::shared_ptr<const fizz::client::FizzClientContext> fizzContext;
    std::shared_ptr<const fizz::CertificateVerifier> fizzCertVerifier;
  };

  static const TLSParams& defaultQuicTLSParams() {
    static const TLSParams tlsParams({"h3"});
    return tlsParams;
  }

  // Helpers to make a Fizz or SSL context
  // Builds cert verifier using makeFizzCertVerifier function below.
  static FizzContextAndVerifier makeFizzClientContextAndVerifier(
      const TLSParams& params);
  static std::shared_ptr<const fizz::client::FizzClientContext>
  makeFizzClientContext(const TLSParams& params);
  /**
   * Constructs a Fizz certificate verifier.
   *
   * If `params.caPaths` is not explicitly set, then the following environment
   * variables are tried, in order:
   *    1. PROXYGEN_CORO_CA_PATH
   *    2. CURL_CA_BUNDLE
   *    3. hardcoded default
   * If no ca path candidate exists, then this function will throw.
   */
  static std::shared_ptr<const fizz::CertificateVerifier> makeFizzCertVerifier(
      const TLSParams& params);

  static std::shared_ptr<folly::SSLContext> makeSSLContext(
      const TLSParams& params);

  struct BaseConnectionParams {
    folly::SocketOptionMap socketOptions{folly::emptySocketOptionMap};
    folly::SocketAddress bindAddr{folly::AsyncSocket::anyAddress()};

    /**
     * `serverName` indicates the DNS hostname associated with this connection.
     *
     * A non-empty `serverName` sends this as the server_name extension for
     * TLS.
     *
     * If the original URL hostname is an IP address, this should not be set.
     */
    std::string serverName;

    // TLS connections must supply either an sslContext or a fizzContext
    FizzContextAndVerifier fizzContextAndVerifier;

    wangle::SSLStats* tlsStats{nullptr};
  };

  struct ConnectionParams : public BaseConnectionParams {
    folly::Optional<std::string> congestionFlavor;
    folly::Optional<std::string> fizzPskIdentity;

    std::shared_ptr<const folly::SSLContext> sslContext;
    SslSessionManagerIf* sslSessionManager{nullptr};

    // Next protocol for plaintext (TCP) connections
    std::string plaintextProtocol;

    // INSECURE: Disables identity verification on server presented end entity
    // certificates
    bool insecureSkipIdentityValidation{true};

    CertVerifyLogFn certVerifyLogFn;
  };

  static const ConnectionParams& defaultConnectionParams() {
    static const ConnectionParams params;
    return params;
  }

  struct QuicConnectionParams : public BaseConnectionParams {
    quic::TransportSettings transportSettings;

    std::shared_ptr<quic::QuicPskCache> quicPskCache;

    Sampling qlogSampling;
    std::shared_ptr<quic::QLogger> qLogger;
    std::shared_ptr<quic::LoopDetectorCallback> quicLoopDetectorCallback;
    std::shared_ptr<quic::QuicTransportStatsCallback>
        quicTransportStatsCallback;
    std::shared_ptr<quic::CongestionControllerFactory> ccFactory;
    std::function<void(quic::QuicClientTransport&)> onTransportCreated;
  };

  struct SessionParams {
    SettingsList settings;
    std::optional<uint32_t> maxConcurrentOutgoingStreams;
    std::optional<size_t> connFlowControl;
    std::optional<std::chrono::milliseconds> streamReadTimeout;
    std::optional<std::chrono::milliseconds> connReadTimeout;
    std::optional<std::chrono::milliseconds> writeTimeout;
    LifecycleObserver* lifecycleObserver{nullptr};
    HTTPSessionStats* sessionStats{nullptr};
    HeaderCodec::Stats* headerCodecStats{nullptr};
    const HeaderIndexingStrategy* headerIndexingStrategy{nullptr};
  };

  static const SessionParams& defaultSessionParams() {
    static const SessionParams params;
    return params;
  }

  static folly::coro::Task<CoroSessionHandle> connect(
      folly::EventBase* evb,
      folly::SocketAddress serverAddr,
      std::chrono::milliseconds timeout,
      const ConnectionParams& connParams = defaultConnectionParams(),
      const SessionParams& sessionParams = defaultSessionParams());

  static constexpr std::chrono::milliseconds kHappyEyeballsDelay{150};

  static folly::coro::Task<CoroSessionHandle> happyEyeballsConnect(
      folly::EventBase* evb,
      folly::SocketAddress primaryAddr,
      folly::SocketAddress fallbackAddr,
      std::chrono::milliseconds timeout,
      const ConnectionParams& connParams = defaultConnectionParams(),
      const SessionParams& sessionParams = defaultSessionParams(),
      std::chrono::milliseconds happyEyeballsTimeout = kHappyEyeballsDelay);

  // For HTTP connections over HTTP CONNECT
  static folly::coro::Task<CoroSessionHandle> proxyConnect(
      CoroSessionHandle proxySession,
      HTTPCoroSession::RequestReservation reservation,
      std::string authority,
      bool connectUnique,
      std::chrono::milliseconds timeout,
      const ConnectionParams& connParams = defaultConnectionParams(),
      const SessionParams& sessionParams = defaultSessionParams());

  static folly::coro::Task<CoroSessionHandle> connect(
      folly::EventBase* evb,
      folly::SocketAddress serverAddr,
      std::chrono::milliseconds timeout,
      const QuicConnectionParams& connParams,
      const SessionParams& sessionParams = defaultSessionParams());
};

} // namespace coro
} // namespace proxygen
