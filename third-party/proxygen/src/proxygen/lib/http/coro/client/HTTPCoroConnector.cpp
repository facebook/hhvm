/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/HTTPCoroConnector.h"
#include <folly/logging/xlog.h>

#include "proxygen/lib/http/coro/transport/CoroSSLTransport.h"
#include "proxygen/lib/http/coro/transport/HTTPConnectAsyncTransport.h"
#include "proxygen/lib/http/coro/transport/HTTPConnectStream.h"
#include "proxygen/lib/http/coro/transport/HTTPConnectTransport.h"
#include "proxygen/lib/http/coro/util/Transport.h"
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/client/AsyncFizzClient.h>
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <folly/FileUtil.h>
#include <folly/SocketAddress.h>
#include <folly/coro/Collect.h>
#include <folly/coro/SharedPromise.h>
#include <folly/coro/Sleep.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/SSLOptions.h>
#include <proxygen/lib/http/codec/DefaultHTTPCodecFactory.h>
#include <quic/QuicException.h>
#include <quic/api/QuicSocket.h>
#include <quic/client/QuicClientTransport.h>
#include <quic/common/events/FollyQuicEventBase.h>
#include <quic/common/udpsocket/FollyQuicAsyncUDPSocket.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>
#include <wangle/ssl/SSLStats.h>
#include <wangle/ssl/SSLUtil.h>

using folly::coro::co_error;
using folly::coro::co_nothrow;
using CoroTransport = folly::coro::Transport;
using CoroTransportIf = folly::coro::TransportIf;

namespace {

using namespace fizz;
using namespace fizz::client;
using namespace proxygen;
using namespace proxygen::coro;

// if the sni is an ip addr format or an empty string, we return folly::none
folly::Optional<std::string> getValidSni(std::string_view sni) {
  if (sni.empty() || folly::IPAddress::validate(sni)) {
    return folly::none;
  }
  return std::string(sni);
}

// default conn & stream fc are ~32MB & ~2MB respectively
constexpr size_t kDefaultConnFlowControl = 1u << 25;
constexpr size_t kDefaultStreamFlowControl = 1u << 21;

class ConnectCB
    : public folly::AsyncSocket::ConnectCallback
    , public AsyncFizzClient::HandshakeCallback {
 public:
  explicit ConnectCB(
      folly::EventBase* evb = nullptr,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0))
      : baton(evb, timeout) {
  }

  void connectSuccess() noexcept override {
    baton.signal();
  }
  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    exception = ex;
    baton.signal();
  }

  void fizzHandshakeSuccess(AsyncFizzClient* /*transport*/) noexcept override {
    connectSuccess();
  }

  void fizzHandshakeError(AsyncFizzClient* /*transport*/,
                          folly::exception_wrapper ex) noexcept override {
    if (auto* exc = ex.get_exception<folly::AsyncSocketException>()) {
      exception = std::move(*exc);
    } else {
      exception = folly::AsyncSocketException(
          folly::AsyncSocketException::SSL_ERROR,
          folly::to<std::string>("Fizz handshake error: ", ex.what()));
    }

    baton.signal();
  }

  std::optional<folly::AsyncSocketException> exception;
  TimedBaton baton;
};

folly::coro::Task<std::unique_ptr<CoroTransportIf>> connectTCP(
    folly::EventBase* eventBase,
    folly::SocketAddress connectAddr,
    std::chrono::milliseconds timeoutMs,
    const HTTPCoroConnector::ConnectionParams& connParams) {
  auto asyncSocket = folly::AsyncSocket::newSocket(eventBase);
  ConnectCB cb;
  asyncSocket->connect(&cb,
                       connectAddr,
                       timeoutMs.count(),
                       connParams.socketOptions,
                       connParams.bindAddr);
  co_await cb.baton.wait();
  co_await folly::coro::co_safe_point;
  if (cb.exception) {
    co_yield co_error(*cb.exception);
  }
  if (connParams.congestionFlavor) {
    asyncSocket->setCongestionFlavor(*connParams.congestionFlavor);
  }
  co_return std::make_unique<proxygen::coro::detail::Transport>(
      eventBase, std::move(asyncSocket));
}

void initTransportInfoFromSSLSocket(wangle::TransportInfo& tinfo,
                                    folly::AsyncSSLSocket& sslSocket) {
  tinfo.secure = true;
  tinfo.appProtocol =
      std::make_shared<std::string>(sslSocket.getApplicationProtocol());
  auto cipher = sslSocket.getNegotiatedCipherName();
  tinfo.sslCipher = cipher ? std::make_shared<std::string>(cipher) : nullptr;
  tinfo.sslVersion = sslSocket.getSSLVersion();
  tinfo.sslResume = wangle::SSLUtil::getResumeState(&sslSocket);
  tinfo.securityType = "OpenSSL";
  // There are a bunch more TLS related fields that are only set on accept
}

void initTransportInfoFromCoroSSLTransport(wangle::TransportInfo& tinfo,
                                           CoroSSLTransport& sslTransport) {
  tinfo.secure = true;
  tinfo.appProtocol =
      std::make_shared<std::string>(sslTransport.getApplicationProtocol());
  auto cipher = sslTransport.getNegotiatedCipherName();
  tinfo.sslCipher = cipher ? std::make_shared<std::string>(cipher) : nullptr;
  tinfo.sslVersion = sslTransport.getSSLVersion();
  // Assuming it's tickets for now.  AsyncSSLSocket only sets session ID on
  // the server.
  tinfo.sslResume = sslTransport.getSSLSessionReused()
                        ? wangle::SSLResumeEnum::RESUME_TICKET
                        : wangle::SSLResumeEnum::HANDSHAKE;

  // Fizz also sets "securityType"
  // There are a bunch more TLS related fields that are only set on accept
}

void initTransportInfoFromFizz(wangle::TransportInfo& tinfo,
                               AsyncFizzClient& fizzClient) {
  tinfo.secure = true;
  tinfo.appProtocol =
      std::make_shared<std::string>(fizzClient.getApplicationProtocol());
  auto negotiatedCipher = fizzClient.getState().cipher();
  tinfo.sslCipher =
      negotiatedCipher
          ? std::make_shared<std::string>(fizz::toString(*negotiatedCipher))
          : nullptr;
  auto fizzVersion = fizzClient.getState().version();
  tinfo.sslVersion = fizzVersion ? static_cast<int>(fizzVersion.value()) : 0;
  auto pskType =
      fizzClient.getState().pskType().value_or(fizz::PskType::NotAttempted);
  tinfo.sslResume = (pskType == fizz::PskType::Resumption)
                        ? wangle::SSLResumeEnum::RESUME_TICKET
                        : wangle::SSLResumeEnum::HANDSHAKE;
  tinfo.securityType = fizzClient.getSecurityProtocol();
}

void initTransportInfoFromQuic(wangle::TransportInfo& tinfo,
                               quic::QuicClientTransport& quicClient) {
  tinfo.secure = true;
  tinfo.appProtocol =
      std::make_shared<std::string>(quicClient.getAppProtocol().value_or(""));
  tinfo.securityType = "Fizz";
}

void setupCodec(HTTPCodec& codec,
                const HTTPCoroConnector::SessionParams& sessionParams) {
  auto settings = codec.getEgressSettings();
  if (!settings) {
    return;
  }

  // default stream fc window to ~2MB, will be overridden below with user
  // supplied value if present
  settings->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                       kDefaultStreamFlowControl);
  for (const auto& setting : sessionParams.settings) {
    settings->setSetting(setting.id, setting.value);
  }

  if (sessionParams.headerCodecStats) {
    codec.setHeaderCodecStats(sessionParams.headerCodecStats);
  }
}

void setupSession(HTTPCoroSession* session,
                  const HTTPCoroConnector::SessionParams& sessionParams) {
  // default conn fc window to ~32MB
  session->setConnectionFlowControl(
      sessionParams.connFlowControl.value_or(kDefaultConnFlowControl));

  if (sessionParams.maxConcurrentOutgoingStreams) {
    session->setMaxConcurrentOutgoingStreams(
        *sessionParams.maxConcurrentOutgoingStreams);
  }
  if (sessionParams.streamReadTimeout) {
    session->setStreamReadTimeout(*sessionParams.streamReadTimeout);
  }
  if (sessionParams.connReadTimeout) {
    session->setConnectionReadTimeout(*sessionParams.connReadTimeout);
  }
  if (sessionParams.writeTimeout) {
    session->setWriteTimeout(*sessionParams.writeTimeout);
  }
  if (sessionParams.lifecycleObserver) {
    session->addLifecycleObserver(sessionParams.lifecycleObserver);
  }
  if (sessionParams.sessionStats) {
    session->setSessionStats(sessionParams.sessionStats);
  }
  session->run().start();
}

folly::coro::Task<std::unique_ptr<CoroTransportIf>> connectFizz(
    folly::EventBase* eventBase,
    folly::SocketAddress connectAddr,
    std::unique_ptr<HTTPConnectStream> connectStream,
    std::chrono::milliseconds timeoutMs,
    const HTTPCoroConnector::ConnectionParams& connParams,
    wangle::TransportInfo& tinfo) {
  ConnectCB cb;
  AsyncFizzClient::UniquePtr fizzClient;
  auto pskIdentity = connParams.fizzPskIdentity.value_or(
      connParams.serverName.empty() ? connectAddr.getAddressStr()
                                    : connParams.serverName);

  auto sni = getValidSni(connParams.serverName);
  if (connectStream) {
    folly::AsyncTransportWrapper::UniquePtr asyncTransport{
        new HTTPConnectAsyncTransport(std::move(connectStream))};
    fizzClient.reset(
        new AsyncFizzClient(std::move(asyncTransport),
                            connParams.fizzContextAndVerifier.fizzContext));
    fizzClient->connect(&cb,
                        connParams.fizzContextAndVerifier.fizzCertVerifier,
                        std::move(sni),
                        pskIdentity,
                        folly::none, /* echConfigs */
                        timeoutMs /* timeout */);
  } else {
    fizzClient.reset(new AsyncFizzClient(
        eventBase, connParams.fizzContextAndVerifier.fizzContext));
    fizzClient->connect(connectAddr,
                        &cb,
                        connParams.fizzContextAndVerifier.fizzCertVerifier,
                        std::move(sni),
                        pskIdentity,
                        timeoutMs, /* total timeout */
                        timeoutMs, /* tcpConnectTimeout */
                        connParams.socketOptions,
                        connParams.bindAddr);
  }

  co_await cb.baton.wait();
  co_await folly::coro::co_safe_point;
  if (cb.exception) {
    co_yield co_error(*cb.exception);
  }
  initTransportInfoFromFizz(tinfo, *fizzClient);
  co_return std::make_unique<proxygen::coro::detail::Transport>(
      eventBase, std::move(fizzClient));
}

inline std::shared_ptr<folly::ssl::SSLSession> getSslSession(
    const HTTPCoroConnector::ConnectionParams& connParams) {
  return connParams.sslSessionManager
             ? connParams.sslSessionManager->getSslSession()
             : nullptr;
}

folly::coro::Task<std::unique_ptr<CoroTransportIf>> connectTLS(
    folly::EventBase* eventBase,
    folly::SocketAddress connectAddr,
    std::unique_ptr<HTTPConnectStream> connectStream,
    std::chrono::milliseconds timeoutMs,
    const HTTPCoroConnector::ConnectionParams& connParams,
    wangle::TransportInfo& tinfo) {

  auto sslSession = getSslSession(connParams);
  auto sni = getValidSni(connParams.serverName);
  if (connectStream) {
    auto sslTransport = std::make_unique<CoroSSLTransport>(
        std::make_unique<HTTPConnectTransport>(std::move(connectStream)),
        connParams.sslContext);
    if (sslSession) {
      sslTransport->setSSLSession(std::move(sslSession));
    }

    co_await sslTransport->connect(std::move(sni), timeoutMs);
    co_await folly::coro::co_safe_point;
    initTransportInfoFromCoroSSLTransport(tinfo, *sslTransport);
    if (!sslTransport->getSSLSessionReused() && connParams.sslSessionManager) {
      connParams.sslSessionManager->onNewSslSession(
          sslTransport->getSSLSession());
    }
    co_return sslTransport;
  } else {
    folly::AsyncSSLSocket::UniquePtr sslSock(
        new folly::AsyncSSLSocket(connParams.sslContext, eventBase));
    if (sslSession) {

      sslSock->setSSLSession(std::move(sslSession));
    }
    sslSock->setServerName(std::move(sni).value_or(""));
    sslSock->forceCacheAddrOnFailure(true);
    ConnectCB cb;
    sslSock->connect(&cb,
                     connectAddr,
                     timeoutMs.count(),
                     connParams.socketOptions,
                     connParams.bindAddr);
    co_await cb.baton.wait();
    co_await folly::coro::co_safe_point;
    if (cb.exception) {
      co_yield co_error(*cb.exception);
    }
    initTransportInfoFromSSLSocket(tinfo, *sslSock);
    if (!sslSock->getSSLSessionReused() && connParams.sslSessionManager) {
      connParams.sslSessionManager->onNewSslSession(sslSock->getSSLSession());
    }
    if (connParams.congestionFlavor) {
      sslSock->setCongestionFlavor(*connParams.congestionFlavor);
    }
    co_return std::make_unique<proxygen::coro::detail::Transport>(
        eventBase, std::move(sslSock));
  }
}

class QuicConnectCB
    : public quic::QuicSocket::ConnectionSetupCallback
    , public ConnectCB {
 public:
  QuicConnectCB(folly::EventBase* evb,
                std::chrono::milliseconds timeout,
                std::shared_ptr<quic::QuicClientTransport> quicClient,
                const HTTPCoroConnector::SessionParams& sessionParams,
                folly::CancellationToken cancellationToken)
      : ConnectCB(evb, timeout),
        quicClient_(std::move(quicClient)),
        sessionParams_(sessionParams),
        cancellationToken_(std::move(cancellationToken)) {
  }

  folly::exception_wrapper quicException;
  HTTPCoroSession* session{nullptr};

 private:
  void quicConnectErr(folly::exception_wrapper ex) noexcept {
    quicException = std::move(ex);
    baton.signal();
  }
  void onConnectionSetupError(quic::QuicError error) noexcept override {
    switch (error.code.type()) {
      case quic::QuicErrorCode::Type::ApplicationErrorCode:
        quicConnectErr(quic::QuicApplicationException(
            error.message, *error.code.asApplicationErrorCode()));
        break;
      case quic::QuicErrorCode::Type::LocalErrorCode:
        quicConnectErr(quic::QuicInternalException(
            error.message, *error.code.asLocalErrorCode()));
        break;
      case quic::QuicErrorCode::Type::TransportErrorCode:
        quicConnectErr(quic::QuicTransportException(
            error.message, *error.code.asTransportErrorCode()));
        break;
    }
  }
  void onReplaySafe() noexcept override {
    replaySafe_ = true;
  }
  void onTransportReady() noexcept override {
    if (cancellationToken_.isCancellationRequested()) {
      quicConnectErr(quic::QuicTransportException(
          "Connection has been cancelled",
          quic::TransportErrorCode::INTERNAL_ERROR));
    }
    initTransportInfoFromQuic(tinfo_, *quicClient_);

    auto codec = hq::HQMultiCodec::Factory::getCodec(
        TransportDirection::UPSTREAM,
        /*useStrictValidation=*/false,
        sessionParams_.headerIndexingStrategy);
    setupCodec(*codec, sessionParams_);

    tinfo_.acceptTime = getCurrentTime();
    tinfo_.sslSetupTime = millisecondsSince(startTime_);
    // TODO: pass replaySafe_
    session = HTTPCoroSession::makeUpstreamCoroSession(
        std::move(quicClient_), std::move(codec), std::move(tinfo_));
    setupSession(session, sessionParams_);
    connectSuccess();
  }
  std::shared_ptr<quic::QuicClientTransport> quicClient_;
  const HTTPCoroConnector::SessionParams& sessionParams_;
  std::chrono::steady_clock::time_point startTime_{getCurrentTime()};
  wangle::TransportInfo tinfo_;
  bool replaySafe_{false};
  folly::CancellationToken cancellationToken_;
};

folly::coro::Task<HTTPCoroSession*> connectQuic(
    folly::EventBase* eventBase,
    folly::SocketAddress connectAddr,
    std::chrono::milliseconds timeoutMs,
    const HTTPCoroConnector::QuicConnectionParams connParams,
    const HTTPCoroConnector::SessionParams sessionParams) {
  auto qEvb = std::make_shared<quic::FollyQuicEventBase>(eventBase);
  auto sock = std::make_unique<quic::FollyQuicAsyncUDPSocket>(qEvb);
  auto quicClient = quic::QuicClientTransport::newClient(
      std::move(qEvb),
      std::move(sock),
      quic::FizzClientQuicHandshakeContext::Builder()
          .setFizzClientContext(connParams.fizzContextAndVerifier.fizzContext)
          .setCertificateVerifier(
              connParams.fizzContextAndVerifier.fizzCertVerifier)
          .setPskCache(connParams.quicPskCache)
          .build(),
      /*connectionIdSize=*/0);

  auto hostname = connParams.serverName.empty() ? connectAddr.getAddressStr()
                                                : connParams.serverName;
  quicClient->setHostname(std::move(hostname));
  quicClient->addNewPeerAddress(connectAddr);
  if (connParams.bindAddr != folly::AsyncSocket::anyAddress()) {
    quicClient->setLocalAddress(connParams.bindAddr);
  }
  if (connParams.ccFactory) {
    quicClient->setCongestionControllerFactory(connParams.ccFactory);
  } else {
    quicClient->setCongestionControllerFactory(
        std::make_shared<quic::DefaultCongestionControllerFactory>());
  }
  if (connParams.qlogSampling.isLucky()) {
    quicClient->setQLogger(connParams.qLogger);
  }
  quicClient->setLoopDetectorCallback(connParams.quicLoopDetectorCallback);
  quicClient->setTransportStatsCallback(connParams.quicTransportStatsCallback);
  quicClient->setSocketOptions(connParams.socketOptions);

  // setTransportSettings has to be after the setTransportStatsCallback.
  // Otherwise, the attempts to log stats while setting the transport
  // settings are lost. The congestion controller type logged by the congestion
  // controller factory is one such stat.
  quicClient->setTransportSettings(connParams.transportSettings);

  folly::CancellationToken cancellationToken =
      co_await folly::coro::co_current_cancellation_token;
  QuicConnectCB cb(eventBase,
                   timeoutMs,
                   quicClient,
                   sessionParams,
                   std::move(cancellationToken));
  quicClient->start(&cb, nullptr);
  auto res = co_await cb.baton.wait();
  quicClient->setConnectionSetupCallback(nullptr);
  if (res != TimedBaton::Status::signalled) {
    auto err = HTTP3::ErrorCode::HTTP_NO_ERROR;
    std::string errString = (res == TimedBaton::Status::timedout)
                                ? "Connect timed out"
                                : "Connection cancelled";
    quicClient->close(
        quic::QuicError(quic::QuicErrorCode(err), std::string(errString)));
    co_yield co_error(quic::QuicInternalException(
        errString, quic::LocalErrorCode::CONNECT_FAILED));
  }
  if (cb.quicException) {
    co_yield co_error(std::move(cb.quicException));
  }
  if (connParams.onTransportCreated) {
    connParams.onTransportCreated(*quicClient.get());
  }
  co_return cb.session;
}

folly::coro::Task<HTTPCoroSession*> connectImpl(
    folly::EventBase* evb,
    folly::SocketAddress serverAddr,
    std::unique_ptr<HTTPConnectStream> connectStream,
    std::chrono::milliseconds timeout,
    proxygen::coro::HTTPCoroConnector::ConnectionParams connParams,
    proxygen::coro::HTTPCoroConnector::SessionParams sessionParams) {
  auto startTime = getCurrentTime();
  wangle::TransportInfo tinfo;

  folly::Try<std::unique_ptr<CoroTransportIf>> socket;
  bool isSecure = true;
  if (connParams.fizzContextAndVerifier.fizzContext) {
    socket = co_await co_awaitTry(connectFizz(
        evb, serverAddr, std::move(connectStream), timeout, connParams, tinfo));
  } else if (connParams.sslContext) {
    socket = co_await co_awaitTry(connectTLS(
        evb, serverAddr, std::move(connectStream), timeout, connParams, tinfo));
  } else {
    if (connectStream) {
      socket.emplace(
          std::make_unique<HTTPConnectTransport>(std::move(connectStream)));
    } else {
      socket = co_await co_awaitTry(
          connectTCP(evb, serverAddr, timeout, connParams));
    }
    isSecure = false;
  }
  co_await folly::coro::co_safe_point;
  if (socket.hasException()) {
    XLOG(DBG4) << "Failed to connect to: " << serverAddr.describe()
               << " err=" << socket.exception().what();
    if (isSecure && connParams.tlsStats) {
      bool verifyError = false; // TODO?
      // TOOD: exclude OperationCancelled?
      connParams.tlsStats->recordSSLUpstreamConnectionError(verifyError);
    }

    co_yield co_error(socket.exception());
  }
  if (isSecure && connParams.tlsStats) {
    bool handshake = (tinfo.sslResume == wangle::SSLResumeEnum::HANDSHAKE);
    connParams.tlsStats->recordSSLUpstreamConnection(handshake);
  }

  HTTPCodecFactory::CodecConfig codecConfig;
  codecConfig.h1.forceHTTP1xCodecTo1_1 = true;
  codecConfig.h2.headerIndexingStrategy = sessionParams.headerIndexingStrategy;
  // Tries H1 for unknown/uninit proto
  DefaultHTTPCodecFactory codecFactory(codecConfig);
  const std::string* nextProto =
      isSecure ? tinfo.appProtocol.get() : &connParams.plaintextProtocol;
  auto codec =
      codecFactory.getCodec(*nextProto, TransportDirection::UPSTREAM, isSecure);
  setupCodec(*codec, sessionParams);

  // TransportInfo has
  tinfo.acceptTime = getCurrentTime();
  if (isSecure) {
    tinfo.sslSetupTime = millisecondsSince(startTime);
  }
  auto session = HTTPCoroSession::makeUpstreamCoroSession(
      std::move(*socket), std::move(codec), std::move(tinfo));
  setupSession(session, sessionParams);
  co_return session;
}

} // namespace

namespace proxygen::coro {

folly::coro::Task<HTTPCoroSession*> HTTPCoroConnector::connect(
    folly::EventBase* evb,
    folly::SocketAddress serverAddr,
    std::chrono::milliseconds timeout,
    const ConnectionParams& connParams,
    const SessionParams& sessionParams) {
  return connectImpl(
      evb, std::move(serverAddr), nullptr, timeout, connParams, sessionParams);
}

folly::coro::Task<HTTPCoroSession*> HTTPCoroConnector::happyEyeballsConnect(
    folly::EventBase* evb,
    folly::SocketAddress primaryAddr,
    folly::SocketAddress fallbackAddr,
    std::chrono::milliseconds timeout,
    const ConnectionParams& connParams,
    const SessionParams& sessionParams,
    std::chrono::milliseconds happyEyeballsDelay) {
  folly::coro::SharedPromise<void> failedConnection;
  auto primaryConnect = [](folly::EventBase* evb,
                           folly::SocketAddress primaryAddr,
                           std::chrono::milliseconds timeout,
                           const ConnectionParams& connParams,
                           const SessionParams& sessionParams,
                           folly::coro::SharedPromise<void>& failedConnection)
      -> folly::coro::Task<HTTPCoroSession*> {
    auto sessionTry =
        co_await folly::coro::co_awaitTry(HTTPCoroConnector::connect(
            evb, std::move(primaryAddr), timeout, connParams, sessionParams));
    if (sessionTry.hasException()) {
      XLOG(DBG4) << "Happy eyeballs primary connect failed: "
                 << sessionTry.exception().what();
      failedConnection.setValue();
    }
    co_return sessionTry;
  };

  // Coroutine that handles the delay, then starts to connect
  auto secondaryConnect =
      [](folly::EventBase* evb,
         folly::SocketAddress fallbackAddr,
         std::chrono::milliseconds timeout,
         std::chrono::milliseconds happyEyeballsDelay,
         const ConnectionParams& connParams,
         const SessionParams& sessionParams,
         const folly::coro::SharedPromise<void>& failedConnection)
      -> folly::coro::Task<HTTPCoroSession*> {
    // Wait for happyEyeballsDelay or until the first attempt fails
    co_await folly::coro::collectAny(
        folly::coro::sleepReturnEarlyOnCancel(happyEyeballsDelay),
        failedConnection.getFuture());
    co_await folly::coro::co_safe_point;
    XLOG(DBG4) << "Happy eyeballs fallback attempt to " << fallbackAddr;
    co_return co_await co_nothrow(HTTPCoroConnector::connect(
        evb, fallbackAddr, timeout, connParams, sessionParams));
  };

  auto res = co_await folly::coro::collectAnyWithoutException(
      primaryConnect(evb,
                     std::move(primaryAddr),
                     timeout,
                     connParams,
                     sessionParams,
                     failedConnection),
      secondaryConnect(evb,
                       std::move(fallbackAddr),
                       timeout,
                       happyEyeballsDelay,
                       connParams,
                       sessionParams,
                       failedConnection));
  co_return res.second;
}

folly::coro::Task<HTTPCoroSession*> HTTPCoroConnector::proxyConnect(
    HTTPCoroSession* proxySession,
    HTTPCoroSession::RequestReservation reservation,
    std::string authority,
    bool connectUnique,
    std::chrono::milliseconds timeout,
    const ConnectionParams& connParams,
    const SessionParams& sessionParams) {

  // egress bufer option?
  XLOG(DBG2) << "Sending CONNECT to " << authority;
  std::unique_ptr<HTTPConnectStream> connectStream;
  if (connectUnique) {
    connectStream = co_await co_nothrow(HTTPConnectStream::connectUnique(
        proxySession, std::move(reservation), authority, timeout));
  } else {
    connectStream = co_await co_nothrow(HTTPConnectStream::connect(
        proxySession, std::move(reservation), authority, timeout));
  }
  auto peerAddr = connectStream->peerAddr_;
  co_return co_await co_nothrow(connectImpl(proxySession->getEventBase(),
                                            std::move(peerAddr),
                                            std::move(connectStream),
                                            timeout,
                                            connParams,
                                            sessionParams));
}

folly::coro::Task<HTTPCoroSession*> HTTPCoroConnector::connect(
    folly::EventBase* evb,
    folly::SocketAddress serverAddr,
    std::chrono::milliseconds timeout,
    const QuicConnectionParams& connParams,
    const SessionParams& sessionParams) {
  return connectQuic(evb, serverAddr, timeout, connParams, sessionParams);
}

std::shared_ptr<folly::SSLContext> HTTPCoroConnector::makeSSLContext(
    const TLSParams& params) {
  auto sslContext = std::make_shared<folly::SSLContext>();
  sslContext->setOptions(SSL_OP_NO_COMPRESSION);
  folly::ssl::setCipherSuites<folly::ssl::SSLCommonOptions>(*sslContext);

  if (params.caPaths.size() > 0) {
    sslContext->loadTrustedCertificates(params.caPaths);
    folly::SSLContext::VerifyServerCertificate verify{
        folly::SSLContext::VerifyServerCertificate::IF_PRESENTED};
    sslContext->setVerificationOption(verify);
  }
  if (!params.clientCertPath.empty() && !params.clientKeyPath.empty()) {
    sslContext->loadCertKeyPairFromFiles(params.clientCertPath.c_str(),
                                         params.clientKeyPath.c_str());
  }
  if (!params.nextProtocols.empty()) {
    sslContext->setAdvertisedNextProtocols(params.nextProtocols);
  }
  return sslContext;
}

HTTPCoroConnector::FizzContextAndVerifier
HTTPCoroConnector::makeFizzClientContext(const TLSParams& params) {
  FizzContextAndVerifier fizzCtx;
  auto fizzContext = std::make_shared<fizz::client::FizzClientContext>();

  std::string certData;
  if (!params.clientCertPath.empty()) {
    folly::readFile(params.clientCertPath.c_str(), certData);
  }
  std::string keyData;
  if (!params.clientKeyPath.empty()) {
    folly::readFile(params.clientKeyPath.c_str(), keyData);
  }
  if (!certData.empty() && !keyData.empty()) {
    auto cert = fizz::openssl::CertUtils::makeSelfCert(std::move(certData),
                                                       std::move(keyData));
    auto certMgr = std::make_shared<fizz::client::CertManager>();
    certMgr->addCert(std::move(cert));
    fizzContext->setClientCertManager(std::move(certMgr));
  }
  fizzContext->setSupportedAlpns(
      {params.nextProtocols.begin(), params.nextProtocols.end()});
  fizzContext->setSendEarlyData(params.earlyData);
  fizzContext->setPskCache(params.pskCache ? params.pskCache
                                           : std::make_shared<BasicPskCache>());

  if (params.caPaths.size() > 0) {
    fizzCtx.fizzCertVerifier =
        fizz::DefaultCertificateVerifier::createFromCAFiles(
            fizz::VerificationContext::Client, params.caPaths);
  }
  fizzCtx.fizzContext = std::move(fizzContext);
  return fizzCtx;
}

} // namespace proxygen::coro
