/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPUpstreamSession.h>

#include <proxygen/lib/http/webtransport/HTTPWebTransport.h>
#include <proxygen/lib/http/webtransport/WtUtils.h>

#include <folly/io/async/AsyncSSLSocket.h>
#include <wangle/acceptor/ConnectionManager.h>

namespace proxygen {

HTTPUpstreamSession::HTTPUpstreamSession(
    const WheelTimerInstance& wheelTimer,
    folly::AsyncTransport::UniquePtr&& sock,
    const folly::SocketAddress& localAddr,
    const folly::SocketAddress& peerAddr,
    std::unique_ptr<HTTPCodec> codec,
    const wangle::TransportInfo& tinfo,
    InfoCallback* infoCallback)
    : HTTPSession(wheelTimer,
                  std::move(sock),
                  localAddr,
                  peerAddr,
                  nullptr,
                  std::move(codec),
                  tinfo,
                  infoCallback) {
  if (sock_) {
    auto asyncSocket = sock_->getUnderlyingTransport<folly::AsyncSocket>();
    if (asyncSocket) {
      asyncSocket->setBufferCallback(this);
    }
  }
  CHECK_EQ(codec_->getTransportDirection(), TransportDirection::UPSTREAM);
}

// uses folly::HHWheelTimer instance which is used on client side & thrift
HTTPUpstreamSession::HTTPUpstreamSession(
    folly::HHWheelTimer* wheelTimer,
    folly::AsyncTransport::UniquePtr&& sock,
    const folly::SocketAddress& localAddr,
    const folly::SocketAddress& peerAddr,
    std::unique_ptr<HTTPCodec> codec,
    const wangle::TransportInfo& tinfo,
    InfoCallback* infoCallback)
    : HTTPUpstreamSession(WheelTimerInstance(wheelTimer),
                          std::move(sock),
                          localAddr,
                          peerAddr,
                          std::move(codec),
                          tinfo,
                          infoCallback) {
}

HTTPUpstreamSession::~HTTPUpstreamSession() = default;

bool HTTPUpstreamSession::isReplaySafe() const {
  return sock_ ? sock_->isReplaySafe() : false;
}

bool HTTPUpstreamSession::isReusable() const {
  VLOG(4) << "isReusable: " << *this
          << ", liveTransactions_=" << liveTransactions_
          << ", isClosing()=" << isClosing()
          << ", sock_->connecting()=" << sock_->connecting()
          << ", codec_->isReusable()=" << codec_->isReusable()
          << ", codec_->isBusy()=" << codec_->isBusy()
          << ", numActiveWrites_=" << numActiveWrites_
          << ", writeTimeout_.isScheduled()=" << writeTimeout_.isScheduled()
          << ", ingressError_=" << ingressError_
          << ", hasMoreWrites()=" << hasMoreWrites()
          << ", codec_->supportsParallelRequests()="
          << codec_->supportsParallelRequests();
  return !isClosing() && !sock_->connecting() && codec_->isReusable() &&
         !codec_->isBusy() && !ingressError_ &&
         (codec_->supportsParallelRequests() ||
          (
              // These conditions only apply to serial codec sessions
              !hasMoreWrites() && liveTransactions_ == 0 &&
              !writeTimeout_.isScheduled()));
}

bool HTTPUpstreamSession::isClosing() const {
  VLOG(5) << "isClosing: " << *this << ", sock_->good()=" << sock_->good()
          << ", draining_=" << draining_
          << ", readsShutdown()=" << readsShutdown()
          << ", writesShutdown()=" << writesShutdown()
          << ", writesDraining_=" << writesDraining_
          << ", resetAfterDrainingWrites_=" << resetAfterDrainingWrites_;
  return !sock_->good() || draining_ || readsShutdown() || writesShutdown() ||
         writesDraining_ || resetAfterDrainingWrites_;
}

void HTTPUpstreamSession::startNow() {
  // startNow in base class CHECKs this session has not started.
  HTTPSession::startNow();
}

HTTPTransaction* HTTPUpstreamSession::newTransaction(
    HTTPTransaction::Handler* handler) {
  auto txn = newTransactionWithError(handler);
  return txn.value_or(nullptr);
}

folly::Expected<HTTPTransaction*, HTTPUpstreamSession::NewTransactionError>
HTTPUpstreamSession::newTransactionWithError(
    HTTPTransaction::Handler* handler) {
  if (!supportsMoreTransactions()) {
    // This session doesn't support any more parallel transactions
    return folly::makeUnexpected<NewTransactionError>(
        "Number of HTTP outgoing transactions reaches limit in the session");
  } else if (draining_) {
    return folly::makeUnexpected<NewTransactionError>("Connection is draining");
  }

  if (!started_) {
    startNow();
  }

  ProxygenError error = kErrorNone;
  auto txn = createTransaction(codec_->createStream(),
                               HTTPCodec::NoStream,
                               http2::DefaultPriority,
                               &error);

  if (!txn) {
    switch (error) {
      case ProxygenError::kErrorBadSocket:
        return folly::makeUnexpected<NewTransactionError>(
            "Socket connection is closing");
      case ProxygenError::kErrorDuplicatedStreamId:
        return folly::makeUnexpected<NewTransactionError>(
            "HTTP Stream ID already exists");
      default:
        return folly::makeUnexpected<NewTransactionError>(
            "Unknown error when creating HTTP transaction");
    }
  }

  DestructorGuard dg(this);
  txn->setHandler(CHECK_NOTNULL(handler));
  return txn;
}

HTTPTransaction::Handler* HTTPUpstreamSession::getTransactionTimeoutHandler(
    HTTPTransaction* /*txn*/) {
  // No special handler for upstream requests that time out
  return nullptr;
}

bool HTTPUpstreamSession::allTransactionsStarted() const {
  for (const auto& txn : transactions_) {
    if (!txn.second.isPushed() && !txn.second.isEgressStarted()) {
      return false;
    }
  }
  return true;
}

void HTTPUpstreamSession::attachThreadLocals(
    folly::EventBase* eventBase,
    std::shared_ptr<const folly::SSLContext> sslContext,
    const WheelTimerInstance& wheelTimer,
    HTTPSessionStats* stats,
    FilterIteratorFn fn,
    HeaderCodec::Stats* headerCodecStats,
    HTTPSessionController* controller) {
  txnEgressQueue_.attachThreadLocals(wheelTimer);
  if (rateLimitFilter_) {
    rateLimitFilter_->attachThreadLocals(&eventBase->timer());
  }
  wheelTimer_ = wheelTimer;
  setController(controller);
  setSessionStats(stats);
  if (sock_) {
    sock_->attachEventBase(eventBase);
    maybeAttachSSLContext(sslContext);
  }
  codec_.foreach (fn);
  codec_->setHeaderCodecStats(headerCodecStats);
  resumeReadsImpl();
  rescheduleLoopCallbacks();
}

void HTTPUpstreamSession::maybeAttachSSLContext(
    std::shared_ptr<const folly::SSLContext> sslContext) const {
#ifndef NO_ASYNCSSLSOCKET
  auto sslSocket = sock_->getUnderlyingTransport<folly::AsyncSSLSocket>();
  if (sslSocket && sslContext) {
    sslSocket->attachSSLContext(sslContext);
  }
#endif
}

void HTTPUpstreamSession::detachThreadLocals(bool detachSSLContext) {
  CHECK(transactions_.empty());
  cancelLoopCallbacks();
  pauseReadsImpl();
  if (sock_) {
    if (detachSSLContext) {
      maybeDetachSSLContext();
    }
    sock_->detachEventBase();
  }
  txnEgressQueue_.detachThreadLocals();
  if (rateLimitFilter_) {
    rateLimitFilter_->detachThreadLocals();
  }
  setController(nullptr);
  setSessionStats(nullptr);
  // The codec filters *shouldn't* be accessible while the socket is detached,
  // I hope
  codec_->setHeaderCodecStats(nullptr);
  auto cm = getConnectionManager();
  if (cm) {
    cm->removeConnection(this);
  }
}

namespace {

constexpr std::string_view kWtNotSupported = "WebTransport not supported";
constexpr std::string_view kInvalidWtReq = "Invalid WebTransport request";
constexpr std::string_view kStreamFailed = "Failed to create stream";

folly::exception_wrapper makeHttpEx(const std::string& err) noexcept {
  constexpr auto kExDir = HTTPException::Direction::INGRESS_AND_EGRESS;
  return folly::make_exception_wrapper<HTTPException>(kExDir, err);
}

using WtReqResult = std::unique_ptr<HTTPMessage>;
using WtReqResultPromise = folly::Promise<WtReqResult>;
folly::Promise<WtReqResult> emptyWtReqPromise() noexcept {
  return folly::Promise<WtReqResult>::makeEmpty();
}

class WtClientCallback final
    : public HttpWtClientCallbackIf
    , public HTTPTransactionHandler {
 private:
  folly::Promise<WtReqResult> promise{emptyWtReqPromise()};

 public:
  explicit WtClientCallback(WtReqResultPromise p) noexcept
      : promise(std::move(p)) {
  }
  ~WtClientCallback() noexcept override = default;
  folly::Promise<WtReqResult> resetPromise() noexcept {
    return std::exchange(promise, emptyWtReqPromise());
  }
  /**
   * Either ::onHeaders or ::onErr can be invoked first
   *
   * When ::onHeadersComplete is invoked, resolve the promise with the non-final
   * http headers
   *
   * When ::onError is invoked, resolve the promise with HTTPException
   */
  void onHeaders(std::unique_ptr<HTTPMessage> msg) noexcept override {
    if (msg->isFinal()) {
      auto p = resetPromise();
      CHECK(p.valid());
      p.setValue(std::move(msg));
    }
  }
  void onErr(const HTTPException& ex) noexcept override {
    auto p = resetPromise();
    CHECK(p.valid());
    p.setException(ex);
  }

  // **ignored**
  void onHeadersComplete(std::unique_ptr<HTTPMessage>) noexcept override {
  }
  void onError(const HTTPException&) noexcept override {
  }
  void detachTransaction() noexcept override {
  }
  void setTransaction(HTTPTransaction*) noexcept override {
  }
  void onBody(std::unique_ptr<folly::IOBuf>) noexcept override {
  }
  void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept override {
  }
  void onEOM() noexcept override {
  }
  void onUpgrade(UpgradeProtocol) noexcept override {
  }
  void onEgressPaused() noexcept override {
  }
  void onEgressResumed() noexcept override {
  }
};

} // namespace

folly::SemiFuture<WtReqResult> HTTPUpstreamSession::sendWebTransportRequest(
    const HTTPMessage& req, WebTransportHandler::Ptr wtHandler) noexcept {
  // both self and peer must indicate support for WebTransport
  const bool supportsWt = proxygen::detail::supportsH2Wt(
      {codec_->getIngressSettings(), codec_->getEgressSettings()});
  const bool validWtReq = HTTPWebTransport::isConnectMessage(req);
  if (!(supportsWt && validWtReq)) {
    auto err = !validWtReq ? kInvalidWtReq : kWtNotSupported;
    VLOG(6) << __func__ << " err=" << err << "; sess=" << *this;
    return makeHttpEx(std::string(err));
  }

  auto [p, f] = folly::makePromiseContract<WtReqResult>();
  auto wtClientCb = std::make_unique<WtClientCallback>(std::move(p));

  auto* txn = newTransaction(wtClientCb.get());
  if (!txn) {
    return makeHttpEx(std::string(kStreamFailed));
  }

  // send wt upgrade req
  txn->setHandler(nullptr); // clear handler, will be replaced by sendWtHeaders
  txn->sendWtHeaders(req, std::move(wtHandler), std::move(wtClientCb));
  return std::move(f);
}

void HTTPUpstreamSession::maybeDetachSSLContext() const {
#ifndef NO_ASYNCSSLSOCKET
  auto sslSocket = sock_->getUnderlyingTransport<folly::AsyncSSLSocket>();
  if (sslSocket) {
    sslSocket->detachSSLContext();
  }
#endif
}

} // namespace proxygen
