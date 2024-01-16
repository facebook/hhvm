/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPSession.h>

#include <chrono>
#include <fizz/protocol/AsyncFizzBase.h>
#include <folly/Conv.h>
#include <folly/CppAttributes.h>
#include <folly/Random.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/tracing/ScopedTraceSection.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/HTTPChecks.h>
#include <proxygen/lib/http/session/HTTPSessionController.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>
#include <wangle/acceptor/ConnectionManager.h>
#include <wangle/acceptor/SocketOptions.h>

using fizz::AsyncFizzBase;
using folly::AsyncSocket;
using folly::AsyncSocketException;
using folly::AsyncTransport;
using folly::IOBuf;
using folly::SocketAddress;
using std::pair;
using std::string;
using std::unique_ptr;
using wangle::TransportInfo;

namespace {
static const uint32_t kMinReadSize = 1460;
static const uint32_t kWriteReadyMax = 65536;

// Lower = higher latency, better prioritization
// Higher = lower latency, less prioritization
static const uint32_t kMaxWritesPerLoop = 32;

static constexpr folly::StringPiece kClientLabel =
    "EXPORTER HTTP CERTIFICATE client";
static constexpr folly::StringPiece kServerLabel =
    "EXPORTER HTTP CERTIFICATE server";
} // anonymous namespace

namespace proxygen {

HTTPSession::HTTPSession(folly::HHWheelTimer* wheelTimer,
                         AsyncTransport::UniquePtr sock,
                         const SocketAddress& localAddr,
                         const SocketAddress& peerAddr,
                         HTTPSessionController* controller,
                         unique_ptr<HTTPCodec> codec,
                         const TransportInfo& tinfo,
                         InfoCallback* infoCallback)
    : HTTPSession(WheelTimerInstance(wheelTimer),
                  std::move(sock),
                  localAddr,
                  peerAddr,
                  controller,
                  std::move(codec),
                  tinfo,
                  infoCallback) {
}

HTTPSession::HTTPSession(const WheelTimerInstance& wheelTimer,
                         AsyncTransport::UniquePtr sock,
                         const SocketAddress& localAddr,
                         const SocketAddress& peerAddr,
                         HTTPSessionController* controller,
                         unique_ptr<HTTPCodec> codec,
                         const TransportInfo& tinfo,
                         InfoCallback* infoCallback)
    : HTTPSessionBase(localAddr,
                      peerAddr,
                      controller,
                      tinfo,
                      infoCallback,
                      std::move(codec),
                      wheelTimer,
                      HTTPCodec::StreamID(0)),
      writeTimeout_(this),
      sock_(std::move(sock)),
      wheelTimer_(wheelTimer),
      draining_(false),
      started_(false),
      writesDraining_(false),
      resetAfterDrainingWrites_(false),
      ingressError_(false),
      flowControlTimeout_(this),
      drainTimeout_(this),
      reads_(SocketState::PAUSED),
      writes_(SocketState::UNPAUSED),
      ingressUpgraded_(false),
      resetSocketOnShutdown_(false),
      inLoopCallback_(false),
      pendingPause_(false),
      writeBufSplit_(false),
      sessionObserverAccessor_(this),
      sessionObserverContainer_(&sessionObserverAccessor_) {
  setByteEventTracker(std::make_shared<ByteEventTracker>(this));
  initialReceiveWindow_ = receiveStreamWindowSize_ = receiveSessionWindowSize_ =
      codec_->getDefaultWindowSize();

  codec_.add<HTTPChecks>();

  setupCodec();

  nextEgressResults_.reserve(maxConcurrentIncomingStreams_);

  if (infoCallback_) {
    infoCallback_->onCreate(*this);
  }

  auto controllerPtr = getController();
  if (controllerPtr) {
    flowControlTimeout_.setTimeoutDuration(
        controllerPtr->getSessionFlowControlTimeout());
  }
  attachToSessionController();
  informSessionControllerTransportReady();

  if (!sock_->isReplaySafe()) {
    sock_->setReplaySafetyCallback(this);
  }
  initCodecHeaderIndexingStrategy();
}

uint32_t HTTPSession::getCertAuthSettingVal() {
  uint32_t certAuthSettingVal = 0;
  constexpr uint16_t settingLen = 4;
  std::unique_ptr<folly::IOBuf> ekm;
  folly::StringPiece label;
  if (isUpstream()) {
    label = kClientLabel;
  } else {
    label = kServerLabel;
  }
  auto fizzBase = getTransport()->getUnderlyingTransport<AsyncFizzBase>();
  if (fizzBase) {
    ekm = fizzBase->getExportedKeyingMaterial(label, nullptr, settingLen);
  } else {
    VLOG(4) << "Underlying transport does not support secondary "
               "authentication.";
    return certAuthSettingVal;
  }
  if (ekm && ekm->computeChainDataLength() == settingLen) {
    folly::io::Cursor cursor(ekm.get());
    uint32_t ekmVal = cursor.readBE<uint32_t>();
    certAuthSettingVal = (ekmVal & 0x3fffffff) | 0x80000000;
  }
  return certAuthSettingVal;
}

bool HTTPSession::verifyCertAuthSetting(uint32_t value) {
  uint32_t certAuthSettingVal = 0;
  constexpr uint16_t settingLen = 4;
  std::unique_ptr<folly::IOBuf> ekm;
  folly::StringPiece label;
  if (isUpstream()) {
    label = kServerLabel;
  } else {
    label = kClientLabel;
  }
  auto fizzBase = getTransport()->getUnderlyingTransport<AsyncFizzBase>();
  if (fizzBase) {
    ekm = fizzBase->getExportedKeyingMaterial(label, nullptr, settingLen);
  } else {
    VLOG(4) << "Underlying transport does not support secondary "
               "authentication.";
    return false;
  }
  if (ekm && ekm->computeChainDataLength() == settingLen) {
    folly::io::Cursor cursor(ekm.get());
    uint32_t ekmVal = cursor.readBE<uint32_t>();
    certAuthSettingVal = (ekmVal & 0x3fffffff) | 0x80000000;
  } else {
    return false;
  }
  if (certAuthSettingVal == value) {
    return true;
  } else {
    return false;
  }
}

void HTTPSession::setupCodec() {
  if (!codec_->supportsParallelRequests()) {
    // until we support upstream pipelining
    maxConcurrentIncomingStreams_ = 1;
    maxConcurrentOutgoingStreamsRemote_ = isDownstream() ? 0 : 1;
  }

  // If a secondary authentication manager is configured for this session, set
  // the SETTINGS_HTTP_CERT_AUTH to indicate support for HTTP-layer certificate
  // authentication.
  uint32_t certAuthSettingVal = 0;
  if (secondAuthManager_) {
    certAuthSettingVal = getCertAuthSettingVal();
  }
  HTTPSettings* settings = codec_->getEgressSettings();
  if (settings) {
    settings->setSetting(SettingsId::MAX_CONCURRENT_STREAMS,
                         maxConcurrentIncomingStreams_);
    if (certAuthSettingVal != 0) {
      settings->setSetting(SettingsId::SETTINGS_HTTP_CERT_AUTH,
                           certAuthSettingVal);
    }
  }
  codec_->generateConnectionPreface(writeBuf_);

  if (codec_->supportsSessionFlowControl() && !connFlowControl_) {
    connFlowControl_ = new FlowControlFilter(*this, writeBuf_, codec_.call());
    codec_.addFilters(std::unique_ptr<FlowControlFilter>(connFlowControl_));
    // if we really support switching from spdy <-> h2, we need to update
    // existing flow control filter
  }
  if (codec_->supportsParallelRequests() && sock_ &&
      codec_->getTransportDirection() == TransportDirection::DOWNSTREAM) {
    auto rateLimitFilter = std::make_unique<RateLimitFilter>(
        &getEventBase()->timer(), sessionStats_);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::HEADERS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::DIRECT_ERROR_HANDLING);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::MISC_CONTROL_MSGS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::RSTS);
    rateLimitFilter_ = rateLimitFilter.get();
    codec_.addFilters(std::move(rateLimitFilter));
  }

  codec_.setCallback(this);
}

HTTPSession::~HTTPSession() {
  VLOG(4) << *this << " closing";

  CHECK(transactions_.empty());
  txnEgressQueue_.dropPriorityNodes();
  CHECK(txnEgressQueue_.empty());
  DCHECK(!sock_->getReadCallback());

  if (writeTimeout_.isScheduled()) {
    writeTimeout_.cancelTimeout();
  }

  if (flowControlTimeout_.isScheduled()) {
    flowControlTimeout_.cancelTimeout();
  }

  if (pingProber_ && pingProber_->isScheduled()) {
    pingProber_->cancelProbes();
  }

  runDestroyCallbacks();
}

void HTTPSession::startNow() {
  CHECK(!started_);
  started_ = true;
  codec_->generateSettings(writeBuf_);
  if (connFlowControl_) {
    connFlowControl_->setReceiveWindowSize(writeBuf_,
                                           receiveSessionWindowSize_);
  }
  // For HTTP/2 if we are currently draining it means we got notified to
  // shutdown before we sent a SETTINGS frame, so we defer sending a GOAWAY
  // util we've started and sent SETTINGS.
  if (draining_) {
    codec_->generateGoaway(writeBuf_);
    auto controller = getController();
    if (controller && codec_->isWaitingToDrain()) {
      wheelTimer_.scheduleTimeout(&drainTimeout_,
                                  controller->getGracefulShutdownTimeout());
    }
  }
  scheduleWrite();
  resumeReads();
}

void HTTPSession::setByteEventTracker(
    std::shared_ptr<ByteEventTracker> byteEventTracker) {
  if (byteEventTracker && byteEventTracker_) {
    byteEventTracker->absorb(std::move(*byteEventTracker_));
  }
  byteEventTracker_ = byteEventTracker;
  if (byteEventTracker_) {
    byteEventTracker_->setCallback(this);
    byteEventTracker_->setTTLBAStats(sessionStats_);
  }
}

void HTTPSession::setSessionStats(HTTPSessionStats* stats) {
  HTTPSessionBase::setSessionStats(stats);
  if (byteEventTracker_) {
    byteEventTracker_->setTTLBAStats(stats);
  }

  if (rateLimitFilter_) {
    rateLimitFilter_->setSessionStats(stats);
  }
}

void HTTPSession::setFlowControl(size_t initialReceiveWindow,
                                 size_t receiveStreamWindowSize,
                                 size_t receiveSessionWindowSize) {
  CHECK(!started_);
  initialReceiveWindow_ = initialReceiveWindow;
  receiveStreamWindowSize_ = receiveStreamWindowSize;
  receiveSessionWindowSize_ = receiveSessionWindowSize;
  HTTPSessionBase::setReadBufferLimit(receiveSessionWindowSize);
  HTTPSettings* settings = codec_->getEgressSettings();
  if (settings) {
    settings->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                         initialReceiveWindow_);
  }
}

void HTTPSession::setEgressSettings(const SettingsList& inSettings) {
  VLOG_IF(4, started_) << "Must flush egress settings to peer";
  HTTPSettings* settings = codec_->getEgressSettings();
  if (settings) {
    for (const auto& setting : inSettings) {
      settings->setSetting(setting.id, setting.value);
      if (setting.id == SettingsId::MAX_CONCURRENT_STREAMS) {
        maxConcurrentIncomingStreams_ = setting.value;
      }
    }
  }
}

void HTTPSession::setMaxConcurrentIncomingStreams(uint32_t num) {
  CHECK(!started_);
  if (codec_->supportsParallelRequests()) {
    maxConcurrentIncomingStreams_ = num;
    HTTPSettings* settings = codec_->getEgressSettings();
    if (settings) {
      settings->setSetting(SettingsId::MAX_CONCURRENT_STREAMS,
                           maxConcurrentIncomingStreams_);
    }
  }
}

void HTTPSession::setEgressBytesLimit(uint64_t bytesLimit) {
  CHECK(!started_);
  egressBytesLimit_ = bytesLimit;
}

void HTTPSession::readTimeoutExpired() noexcept {
  VLOG(3) << "session-level timeout on " << *this;

  DestructorGuard g(this);
  setCloseReason(ConnectionCloseReason::TIMEOUT);
  notifyPendingShutdown();
}

void HTTPSession::writeTimeoutExpired() noexcept {
  VLOG(4) << "Write timeout for " << *this;

  CHECK(pendingWrite_.hasValue());
  DestructorGuard g(this);

  setCloseReason(ConnectionCloseReason::TIMEOUT);
  shutdownTransportWithReset(kErrorWriteTimeout);
}

void HTTPSession::flowControlTimeoutExpired() noexcept {
  VLOG(4) << "Flow control timeout for " << *this;

  DestructorGuard g(this);

  setCloseReason(ConnectionCloseReason::TIMEOUT);
  shutdownTransport(true, true);
}

void HTTPSession::describe(std::ostream& os) const {
  os << "proto=" << getCodecProtocolString(codec_->getProtocol());
  if (isDownstream()) {
    os << ", UA=" << codec_->getUserAgent()
       << ", downstream=" << getPeerAddress() << ", " << getLocalAddress()
       << "=local";
  } else {
    os << ", local=" << getLocalAddress() << ", " << getPeerAddress()
       << "=upstream";
  }
}

bool HTTPSession::isBusy() const {
  return !transactions_.empty() || codec_->isBusy();
}

void HTTPSession::notifyPendingEgress() noexcept {
  scheduleWrite();
}

void HTTPSession::notifyPendingShutdown() {
  VLOG(4) << *this << " notified pending shutdown";
  drain();
}

void HTTPSession::closeWhenIdle() {
  // If drain() already called, this is a noop
  drain();
  // Generate the second GOAWAY now. No-op if second GOAWAY already sent.
  if (codec_->generateGoaway(writeBuf_)) {
    scheduleWrite();
  }
  if (!isBusy() && !hasMoreWrites()) {
    // if we're already idle, close now
    dropConnection();
  }
}

void HTTPSession::immediateShutdown() {
  if (isLoopCallbackScheduled()) {
    cancelLoopCallback();
  }
  if (shutdownTransportCb_) {
    shutdownTransportCb_.reset();
  }
  // checkForShutdown only closes the connection if these conditions are true
  DCHECK(writesShutdown());
  DCHECK(transactions_.empty());
  checkForShutdown();
}

void HTTPSession::dropConnection(const std::string& errorMsg) {
  VLOG(4) << "dropping " << *this;
  if (!sock_ || (readsShutdown() && writesShutdown())) {
    VLOG(4) << *this << " already shutdown";
    DCHECK(!shutdownTransportCb_) << "Why is there a shutdownTransportCb_?";
    if (isLoopCallbackScheduled()) {
      immediateShutdown();
    }
    return;
  }

  setCloseReason(ConnectionCloseReason::SHUTDOWN);
  if (transactions_.empty() && !hasMoreWrites()) {
    DestructorGuard dg(this);
    shutdownTransport(true, true);
    // shutdownTransport might have generated a write (goaway)
    // If so, writes will not be shutdown, so fall through to
    // shutdownTransportWithReset.
    if (readsShutdown() && writesShutdown()) {
      immediateShutdown();
      return;
    }
  }
  shutdownTransportWithReset(kErrorDropped, errorMsg);
}

void HTTPSession::dumpConnectionState(uint8_t /*loglevel*/) {
}

bool HTTPSession::isUpstream() const {
  return codec_->getTransportDirection() == TransportDirection::UPSTREAM;
}

bool HTTPSession::isDownstream() const {
  return codec_->getTransportDirection() == TransportDirection::DOWNSTREAM;
}

void HTTPSession::getReadBuffer(void** buf, size_t* bufSize) {
  FOLLY_SCOPED_TRACE_SECTION("HTTPSession - getReadBuffer");
  pair<void*, uint32_t> readSpace =
      readBuf_.preallocate(kMinReadSize, HTTPSessionBase::maxReadBufferSize_);
  *buf = readSpace.first;
  *bufSize = readSpace.second;
}

void HTTPSession::readDataAvailable(size_t readSize) noexcept {
  FOLLY_SCOPED_TRACE_SECTION(
      "HTTPSession - readDataAvailable", "readSize", readSize);
  VLOG(10) << "read completed on " << *this << ", bytes=" << readSize;

  DestructorGuard dg(this);
  if (pingProber_) {
    pingProber_->refreshTimeout(/*onIngress=*/true);
  }
  resetTimeout();

  if (ingressError_) {
    VLOG(3) << "discarding readBuf due to ingressError_ sess=" << *this
            << " bytes=" << readSize;
    return;
  }
  readBuf_.postallocate(readSize);

  if (infoCallback_) {
    infoCallback_->onRead(*this, readSize, HTTPCodec::NoStream);
  }

  processReadData();
}

bool HTTPSession::isBufferMovable() noexcept {
  return true;
}

void HTTPSession::readBufferAvailable(std::unique_ptr<IOBuf> readBuf) noexcept {
  size_t readSize = readBuf->computeChainDataLength();
  FOLLY_SCOPED_TRACE_SECTION(
      "HTTPSession - readBufferAvailable", "readSize", readSize);
  VLOG(5) << "read completed on " << *this << ", bytes=" << readSize;

  if (pingProber_) {
    pingProber_->refreshTimeout(/*onIngress=*/true);
  }

  DestructorGuard dg(this);
  resetTimeout();

  if (ingressError_) {
    VLOG(3) << "discarding readBuf due to ingressError_ sess=" << *this
            << " bytes=" << readSize;
    return;
  }
  readBuf_.append(std::move(readBuf));

  if (infoCallback_) {
    infoCallback_->onRead(*this, readSize, HTTPCodec::NoStream);
  }

  processReadData();
}

void HTTPSession::processReadData() {
  FOLLY_SCOPED_TRACE_SECTION("HTTPSession - processReadData");

  // Pass the ingress data through the codec to parse it. The codec
  // will invoke various methods of the HTTPSession as callbacks.
  while (!ingressError_ && readsUnpaused() && !readBuf_.empty()) {
    // Skip any 0 length buffers before invoking the codec. Since readBuf_ is
    // not empty, we are guaranteed to find a non-empty buffer.
    while (readBuf_.front()->length() == 0) {
      readBuf_.pop_front();
    }

    // We're about to parse, make sure the parser is not paused
    codec_->setParserPaused(false);
    size_t bytesParsed = codec_->onIngress(*readBuf_.front());
    if (bytesParsed == 0) {
      // If the codec didn't make any progress with current input, we
      // better get more.
      break;
    }
    readBuf_.trimStart(bytesParsed);
  }
}

void HTTPSession::readEOF() noexcept {
  DestructorGuard guard(this);
  VLOG(4) << "EOF on " << *this;
  // for SSL only: error without any bytes from the client might happen
  // due to client-side issues with the SSL cert. Note that it can also
  // happen if the client sends a SPDY frame header but no body.
  if (infoCallback_ && transportInfo_.secure && getNumTxnServed() == 0 &&
      readBuf_.empty()) {
    infoCallback_->onIngressError(*this, kErrorClientSilent);
  }

  // Shut down reads, and also shut down writes if there are no
  // transactions.  (If there are active transactions, leave the
  // write side of the socket open so those transactions can
  // finish generating responses.)
  setCloseReason(ConnectionCloseReason::READ_EOF);
  shutdownTransport(true, transactions_.empty());
}

void HTTPSession::readErr(const AsyncSocketException& ex) noexcept {
  DestructorGuard guard(this);
  VLOG(4) << "read error on " << *this << ": " << ex.what();

  auto sslEx = dynamic_cast<const folly::SSLException*>(&ex);
  if (infoCallback_ && sslEx) {
    if (sslEx->getSSLError() == folly::SSLError::CLIENT_RENEGOTIATION) {
      infoCallback_->onIngressError(*this, kErrorClientRenegotiation);
    }
  }

  // We're definitely finished reading. Don't close the write side
  // of the socket if there are outstanding transactions, though.
  // Instead, give the transactions a chance to produce any remaining
  // output.
  if (sslEx && sslEx->getSSLError() == folly::SSLError::SSL_ERROR) {
    transportInfo_.sslError = ex.what();
  }
  setCloseReason(ConnectionCloseReason::IO_READ_ERROR);
  shutdownTransport(true, transactions_.empty(), ex.what());
}

HTTPTransaction* HTTPSession::newPushedTransaction(
    HTTPCodec::StreamID assocStreamId,
    HTTPTransaction::PushHandler* handler,
    ProxygenError* error) noexcept {
  if (!codec_->supportsPushTransactions()) {
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorPushNotSupported);
    return nullptr;
  }
  CHECK(isDownstream());
  CHECK_NOTNULL(handler);
  if (draining_) {
    // This session doesn't support any more push transactions
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorTransportIsDraining);
    return nullptr;
  }

  if (outgoingStreams_ >= maxConcurrentOutgoingStreamsRemote_) {
    // This session doesn't support any more push transactions
    // This could be an actual problem - since a single downstream SPDY session
    // might be connected to N upstream hosts, each of which send M pushes,
    // which exceeds the limit.
    // should we queue?
    SET_PROXYGEN_ERROR_IF(
        error, ProxygenError::kErrorMaxConcurrentOutgoingStreamLimitReached);
    return nullptr;
  }

  HTTPTransaction* txn = createTransaction(codec_->createStream(),
                                           assocStreamId,
                                           HTTPCodec::NoExAttributes,
                                           http2::DefaultPriority,
                                           error);
  if (!txn) {
    return nullptr;
  }
  DestructorGuard dg(this);
  txn->setHandler(handler);
  return txn;
}

HTTPTransaction* FOLLY_NULLABLE
HTTPSession::newExTransaction(HTTPTransaction::Handler* handler,
                              HTTPCodec::StreamID controlStream,
                              bool unidirectional) noexcept {
  CHECK(handler && controlStream > 0);
  auto eSettings = codec_->getEgressSettings();
  if (!eSettings || !eSettings->getSetting(SettingsId::ENABLE_EX_HEADERS, 0)) {
    LOG(ERROR) << getCodecProtocolString(codec_->getProtocol())
               << " does not support ExTransaction";
    return nullptr;
  }
  if (draining_ || (outgoingStreams_ >= maxConcurrentOutgoingStreamsRemote_)) {
    LOG(ERROR) << "cannot support any more transactions in " << *this
               << " isDraining: " << draining_
               << " outgoing streams: " << outgoingStreams_
               << " max concurrent outgoing streams: "
               << maxConcurrentOutgoingStreamsRemote_;
    return nullptr;
  }

  DCHECK(started_);
  HTTPTransaction* txn =
      createTransaction(codec_->createStream(),
                        HTTPCodec::NoStream,
                        HTTPCodec::ExAttributes(controlStream, unidirectional));
  if (!txn) {
    return nullptr;
  }

  // we find a control stream, let's track it
  controlStreamIds_.emplace(controlStream);

  DestructorGuard dg(this);
  txn->setHandler(handler);
  return txn;
}

size_t HTTPSession::getCodecSendWindowSize() const {
  const HTTPSettings* settings = codec_->getIngressSettings();
  if (settings) {
    return settings->getSetting(SettingsId::INITIAL_WINDOW_SIZE,
                                codec_->getDefaultWindowSize());
  }
  return codec_->getDefaultWindowSize();
}

http2::PriorityUpdate HTTPSession::getMessagePriority(const HTTPMessage* msg) {
  http2::PriorityUpdate h2Pri = http2::DefaultPriority;

  // if HTTP2 priorities are enabled, get them from the message
  // and ignore otherwise
  if (getHTTP2PrioritiesEnabled() && msg) {
    auto res = msg->getHTTP2Priority();
    if (res) {
      h2Pri.streamDependency = std::get<0>(*res);
      h2Pri.exclusive = std::get<1>(*res);
      h2Pri.weight = std::get<2>(*res);
    } else {
      // HTTPMessage with setPriority called explicitly
      h2Pri.streamDependency =
          codec_->mapPriorityToDependency(msg->getPriority());
    }
  }
  return h2Pri;
}

void HTTPSession::onMessageBegin(HTTPCodec::StreamID streamID,
                                 HTTPMessage* msg) {
  VLOG(4) << "processing new msg streamID=" << streamID << " " << *this;

  HTTPTransaction* txn = findTransaction(streamID);
  if (txn) {
    if (isDownstream() && txn->isPushed()) {
      // Push streams are unidirectional (half-closed). If the downstream
      // attempts to send ingress, abort with STREAM_CLOSED error.
      HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                       "Downstream attempts to send ingress, abort.");
      ex.setCodecStatusCode(ErrorCode::STREAM_CLOSED);
      txn->onError(ex);
    }
    return; // If this transaction is already registered, no need to add it now
  }

  if (infoCallback_) {
    infoCallback_->onRequestBegin(*this);
  }

  http2::PriorityUpdate messagePriority = getMessagePriority(msg);
  txn = createTransaction(streamID,
                          HTTPCodec::NoStream,
                          HTTPCodec::NoExAttributes,
                          messagePriority);
  if (!txn) {
    return; // This could happen if the socket is bad.
  }

  if (!codec_->supportsParallelRequests() && getPipelineStreamCount() > 1) {
    // The previous transaction hasn't completed yet. Pause reads until
    // it completes; this requires pausing both transactions.

    // HTTP/1.1 pipeline is detected, and which is incompactible with
    // ByteEventTracker. Drain all the ByteEvents
    CHECK(byteEventTracker_);
    byteEventTracker_->drainByteEvents();

    // drainByteEvents() may detach txn(s). Don't pause read if one txn left
    if (getPipelineStreamCount() < 2) {
      DCHECK(readsUnpaused());
      return;
    }

    // There must be at least two transactions (we just checked). The previous
    // txns haven't completed yet. Pause reads until they complete
    DCHECK_GE(transactions_.size(), 2);
    std::map<HTTPCodec::StreamID, HTTPTransaction*> sortedTxns;
    for (auto& x : transactions_) {
      sortedTxns.emplace(x.first, &x.second);
    }
    for (auto it = ++sortedTxns.rbegin(); it != sortedTxns.rend(); ++it) {
      DCHECK(it->second->isIngressEOMSeen());
      it->second->pauseIngress();
    }
    sortedTxns.rbegin()->second->pauseIngress();
    DCHECK_EQ(liveTransactions_, 0);
    DCHECK(readsPaused());
  }
}

void HTTPSession::onPushMessageBegin(HTTPCodec::StreamID streamID,
                                     HTTPCodec::StreamID assocStreamID,
                                     HTTPMessage* msg) {
  VLOG(4) << "processing new push promise streamID=" << streamID
          << " on assocStreamID=" << assocStreamID << " " << *this;
  if (infoCallback_) {
    infoCallback_->onRequestBegin(*this);
  }
  if (assocStreamID == 0) {
    VLOG(2) << "push promise " << streamID << " should be associated with "
            << "an active stream=" << assocStreamID << " " << *this;
    invalidStream(streamID, ErrorCode::PROTOCOL_ERROR);
    return;
  }

  if (isDownstream()) {
    VLOG(2) << "push promise cannot be sent to upstream " << *this;
    invalidStream(streamID, ErrorCode::PROTOCOL_ERROR);
    return;
  }

  HTTPTransaction* assocTxn = findTransaction(assocStreamID);
  if (!assocTxn || assocTxn->isIngressEOMSeen()) {
    VLOG(2) << "cannot find the assocTxn=" << assocTxn
            << ", or assoc stream is already closed by upstream" << *this;
    invalidStream(streamID, ErrorCode::PROTOCOL_ERROR);
    return;
  }

  http2::PriorityUpdate messagePriority = getMessagePriority(msg);
  auto txn = createTransaction(
      streamID, assocStreamID, HTTPCodec::NoExAttributes, messagePriority);
  if (!txn) {
    return; // This could happen if the socket is bad.
  }

  if (!assocTxn->onPushedTransaction(txn)) {
    VLOG(1) << "Failed to add pushed txn " << streamID << " to assoc txn "
            << assocStreamID << " on " << *this;
    HTTPException ex(
        HTTPException::Direction::INGRESS_AND_EGRESS,
        folly::to<std::string>("Failed to add pushed transaction ", streamID));
    ex.setCodecStatusCode(ErrorCode::REFUSED_STREAM);
    onError(streamID, ex, true);
  }
}

void HTTPSession::onExMessageBegin(HTTPCodec::StreamID streamID,
                                   HTTPCodec::StreamID controlStream,
                                   bool unidirectional,
                                   HTTPMessage* msg) {
  VLOG(4) << "processing new ExMessage=" << streamID
          << " on controlStream=" << controlStream << ", " << *this;
  if (infoCallback_) {
    infoCallback_->onRequestBegin(*this);
  }
  if (controlStream == 0) {
    LOG(ERROR) << "ExMessage=" << streamID << " should have an active control "
               << "stream=" << controlStream << ", " << *this;
    invalidStream(streamID, ErrorCode::PROTOCOL_ERROR);
    return;
  }

  HTTPTransaction* controlTxn = findTransaction(controlStream);
  if (!controlTxn) {
    // control stream is broken, or remote sends a bogus stream id
    LOG(ERROR) << "no control stream=" << controlStream << ", " << *this;
    return;
  }

  http2::PriorityUpdate messagePriority = getMessagePriority(msg);
  auto txn =
      createTransaction(streamID,
                        HTTPCodec::NoStream,
                        HTTPCodec::ExAttributes(controlStream, unidirectional),
                        messagePriority);
  if (!txn) {
    return; // This could happen if the socket is bad.
  }
  // control stream may be paused if the upstream is not ready yet
  if (controlTxn->isIngressPaused()) {
    txn->pauseIngress();
  }
}

void HTTPSession::onHeadersComplete(HTTPCodec::StreamID streamID,
                                    unique_ptr<HTTPMessage> msg) {
  // The codec's parser detected the end of an ingress message's
  // headers.
  VLOG(4) << "processing ingress headers complete for " << *this
          << ", streamID=" << streamID;

  if (!codec_->isReusable()) {
    setCloseReason(ConnectionCloseReason::REQ_NOTREUSABLE);
  }

  if (infoCallback_) {
    infoCallback_->onIngressMessage(*this, *msg.get());
  }

  // Inform observers when request headers (i.e. ingress, from downstream
  // client) are processed.
  if (isDownstream()) {
    if (auto msgPtr = msg.get()) {
      const auto event =
          HTTPSessionObserverInterface::RequestStartedEvent::Builder()
              .setTimestamp(HTTPSessionObserverInterface::Clock::now())
              .setHeaders(msgPtr->getHeaders())
              .build();
      sessionObserverContainer_.invokeInterfaceMethod<
          HTTPSessionObserverInterface::Events::requestStarted>(
          [&event](auto observer, auto observed) {
            observer->requestStarted(observed, event);
          });
    }
  }

  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    invalidStream(streamID);
    return;
  }

  HTTPTransaction::DestructorGuard dg(txn);

  const char* sslCipher =
      transportInfo_.sslCipher ? transportInfo_.sslCipher->c_str() : nullptr;
  msg->setSecureInfo(transportInfo_.sslVersion, sslCipher);
  msg->setSecure(transportInfo_.secure);

  auto controlStreamID = txn->getControlStream();
  if (controlStreamID) {
    auto controlTxn = findTransaction(*controlStreamID);
    if (!controlTxn) {
      VLOG(2) << "txn=" << streamID
              << " with a broken controlTxn=" << *controlStreamID << " "
              << *this;
      HTTPException ex(
          HTTPException::Direction::INGRESS_AND_EGRESS,
          folly::to<std::string>("broken controlTxn ", *controlStreamID));
      onError(streamID, ex, true);
      return;
    }

    // Call onExTransaction() only for requests.
    if (txn->isRemoteInitiated() && !controlTxn->onExTransaction(txn)) {
      VLOG(2) << "Failed to add exTxn=" << streamID
              << " to controlTxn=" << *controlStreamID << ", " << *this;
      HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                       folly::to<std::string>("Fail to add exTxn ", streamID));
      ex.setCodecStatusCode(ErrorCode::REFUSED_STREAM);
      onError(streamID, ex, true);
      return;
    }
  } else {
    setupOnHeadersComplete(txn, msg.get());
  }

  // The txn may have already been aborted by the handler.
  // Verify that the txn is not done.
  if (txn->isIngressComplete() && txn->isEgressComplete()) {
    return;
  }

  if (!txn->getHandler()) {
    txn->sendAbort();
    return;
  }

  // Tell the Transaction to start processing the message now
  // that the full ingress headers have arrived.
  txn->onIngressHeadersComplete(std::move(msg));
  if (httpSessionActivityTracker_) {
    httpSessionActivityTracker_->reportActivity();
  }
}

void HTTPSession::onBody(HTTPCodec::StreamID streamID,
                         unique_ptr<IOBuf> chain,
                         uint16_t padding) {
  FOLLY_SCOPED_TRACE_SECTION("HTTPSession - onBody");
  DestructorGuard dg(this);
  // The codec's parser detected part of the ingress message's
  // entity-body.
  uint64_t length = chain->computeChainDataLength();
  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    if (connFlowControl_ &&
        connFlowControl_->ingressBytesProcessed(writeBuf_, length)) {
      scheduleWrite();
    }
    invalidStream(streamID);
    return;
  }

  if (HTTPSessionBase::onBodyImpl(std::move(chain), length, padding, txn)) {
    VLOG(4) << *this << " pausing due to read limit exceeded.";
    pauseReads();
  }
}

void HTTPSession::onChunkHeader(HTTPCodec::StreamID streamID, size_t length) {
  // The codec's parser detected a chunk header (meaning that this
  // connection probably is HTTP/1.1).
  //
  // After calling onChunkHeader(), the codec will call onBody() zero
  // or more times and then call onChunkComplete().
  //
  // The reason for this callback on the chunk header is to support
  // an optimization.  In general, the job of the codec is to present
  // the HTTPSession with an abstract view of a message,
  // with all the details of wire formatting hidden.  However, there's
  // one important case where we want to know about chunking: reverse
  // proxying where both the client and server streams are HTTP/1.1.
  // In that scenario, we preserve the server's chunk boundaries when
  // sending the response to the client, in order to avoid possibly
  // making the egress packetization worse by rechunking.
  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    invalidStream(streamID);
    return;
  }
  txn->onIngressChunkHeader(length);
}

void HTTPSession::onChunkComplete(HTTPCodec::StreamID streamID) {
  // The codec's parser detected the end of the message body chunk
  // associated with the most recent call to onChunkHeader().
  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    invalidStream(streamID);
    return;
  }
  txn->onIngressChunkComplete();
}

void HTTPSession::onTrailersComplete(HTTPCodec::StreamID streamID,
                                     unique_ptr<HTTPHeaders> trailers) {
  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    invalidStream(streamID);
    return;
  }
  txn->onIngressTrailers(std::move(trailers));
}

void HTTPSession::onMessageComplete(HTTPCodec::StreamID streamID,
                                    bool upgrade) {
  DestructorGuard dg(this);
  // The codec's parser detected the end of the ingress message for
  // this transaction.
  VLOG(4) << "processing ingress message complete for " << *this
          << ", streamID=" << streamID;
  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    invalidStream(streamID);
    return;
  }

  if (upgrade) {
    /* Send the upgrade callback to the transaction and the handler.
     * Currently we support upgrades for only HTTP sessions and not SPDY
     * sessions.
     */
    ingressUpgraded_ = true;
    txn->onIngressUpgrade(UpgradeProtocol::TCP);
    return;
  }

  decrementTransactionCount(txn, true, false);
  txn->onIngressEOM();

  // The codec knows, based on the semantics of whatever protocol it
  // supports, whether it's valid for any more ingress messages to arrive
  // after this one.  For example, an HTTP/1.1 request containing
  // "Connection: close" indicates the end of the ingress, whereas a
  // SPDY session generally can handle more messages at any time.
  //
  // If the connection is not reusable, we close the read side of it
  // but not the write side.  There are two reasons why more writes
  // may occur after this point:
  //   * If there are previous writes buffered up in the
  //     queue, we need to attempt to complete them.
  //   * The Handler associated with the transaction may want to
  //     produce more egress data when the ingress message is fully
  //     complete.  (As a common example, an application that handles
  //     form POSTs may not be able to even start generating a response
  //     until it has received the full request body.)
  //
  // There may be additional checks that need to be performed that are
  // specific to requests or responses, so we call the subclass too.
  if (!codec_->isReusable() && !codec_->supportsParallelRequests()) {
    VLOG(4) << *this << " cannot reuse ingress";
    shutdownTransport(true, false);
  }
}

void HTTPSession::onError(HTTPCodec::StreamID streamID,
                          const HTTPException& error,
                          bool newTxn) {
  DestructorGuard dg(this);
  // The codec detected an error in the ingress stream, possibly bad
  // syntax, a truncated message, or bad semantics in the frame.  If reads
  // are paused, queue up the event; otherwise, process it now.
  VLOG(4) << "Error on " << *this << ", streamID=" << streamID << ", " << error;

  if (ingressError_) {
    return;
  }
  if (!codec_->supportsParallelRequests()) {
    // this error should only prevent us from reading/handling more errors
    // on serial streams
    ingressError_ = true;
    setCloseReason(ConnectionCloseReason::SESSION_PARSE_ERROR);
  }
  if ((streamID == 0) && infoCallback_) {
    infoCallback_->onIngressError(*this, kErrorMessage);
  }

  if (!streamID) {
    ingressError_ = true;
    onSessionParseError(error);
    return;
  }

  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    if (error.hasHttpStatusCode() && streamID != 0) {
      // If the error has an HTTP code, then parsing was fine, it just was
      // illegal in a higher level way
      txn = createTransaction(
          streamID, HTTPCodec::NoStream, HTTPCodec::NoExAttributes);
      if (infoCallback_) {
        infoCallback_->onRequestBegin(*this);
      }
      if (txn) {
        handleErrorDirectly(txn, error);
      }
    } else if (newTxn) {
      onNewTransactionParseError(streamID, error);
    } else {
      VLOG(4) << *this << " parse error with invalid transaction";
      invalidStream(streamID);
    }
    return;
  }

  if (!txn->getHandler() &&
      txn->getEgressState() == HTTPTransactionEgressSM::State::Start) {
    handleErrorDirectly(txn, error);
    return;
  }

  txn->onError(error);
  if (!codec_->isReusable() && transactions_.empty()) {
    VLOG(4) << *this << "shutdown from onError";
    setCloseReason(ConnectionCloseReason::SESSION_PARSE_ERROR);
    shutdownTransport(true, true);
  }
}

void HTTPSession::onAbort(HTTPCodec::StreamID streamID, ErrorCode code) {
  VLOG(4) << "stream abort on " << *this << ", streamID=" << streamID
          << ", code=" << getErrorCodeString(code);

  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    VLOG(4) << *this
            << " abort for unrecognized transaction, streamID= " << streamID;
    return;
  }
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                   folly::to<std::string>("Stream aborted, streamID=",
                                          streamID,
                                          ", code=",
                                          getErrorCodeString(code)));
  ex.setProxygenError(kErrorStreamAbort);
  ex.setCodecStatusCode(code);
  DestructorGuard dg(this);

  if (abortPushesOnRST_ && isDownstream() && !txn->getAssocTxnId() &&
      code == ErrorCode::CANCEL) {
    VLOG(4) << "Cancel all push txns because assoc txn has been cancelled.";
    for (auto it = txn->getPushedTransactions().begin();
         it != txn->getPushedTransactions().end();) {
      auto pushTxn = findTransaction(*it);
      ++it;
      DCHECK(pushTxn != nullptr);
      pushTxn->onError(ex);
    }
  }

  auto exTxns = txn->getExTransactions();
  for (auto it = exTxns.begin(); it != exTxns.end(); ++it) {
    auto exTxn = findTransaction(*it);
    if (exTxn) {
      exTxn->onError(ex);
    }
  }
  txn->onError(ex);
}

void HTTPSession::onGoaway(uint64_t lastGoodStreamID,
                           ErrorCode code,
                           std::unique_ptr<folly::IOBuf> debugData) {
  DestructorGuard g(this);
  VLOG(4) << "GOAWAY on " << *this << ", code=" << getErrorCodeString(code);

  setCloseReason(ConnectionCloseReason::GOAWAY);

  // Drain active transactions and prevent new transactions
  drain();

  // We give the less-forceful onGoaway() first so that transactions have
  // a chance to do stat tracking before potentially getting a forceful
  // onError().
  invokeOnAllTransactions(
      [code](HTTPTransaction* txn) { txn->onGoaway(code); });

  // Abort transactions which have been initiated but not created
  // successfully at the remote end. Upstream transactions are created
  // with odd transaction IDs and downstream transactions with even IDs.
  std::vector<HTTPCodec::StreamID> refusedIds;
  std::vector<HTTPCodec::StreamID> errorIds;
  std::vector<HTTPCodec::StreamID> pubSubControlIds;

  for (const auto& id : transactionIds_) {
    if (((bool)(id & 0x01) == isUpstream()) && (id > lastGoodStreamID)) {
      refusedIds.push_back(id);
    } else if (code != ErrorCode::NO_ERROR) {
      // Error goaway -> error all streams
      errorIds.push_back(id);
    } else if (lastGoodStreamID < http2::kMaxStreamID &&
               (controlStreamIds_.find(id) != controlStreamIds_.end())) {
      // Final (non-error) goaway -> error control streams
      pubSubControlIds.push_back(id);
    }
  }
  errorOnTransactionIds(refusedIds, kErrorStreamUnacknowledged);

  if (code != ErrorCode::NO_ERROR) {
    string debugStr;
    if (debugData) {
      debugData->coalesce();
      debugStr = debugData->moveToFbString();
    }
    auto msg = folly::to<std::string>("GOAWAY error with codec error: ",
                                      getErrorCodeString(code),
                                      " with debug info: ",
                                      debugStr);
    errorOnTransactionIds(errorIds, kErrorConnectionReset, msg);
  }

  errorOnTransactionIds(pubSubControlIds, kErrorStreamAbort);
}

void HTTPSession::onPingRequest(uint64_t data) {
  VLOG(4) << *this << " got ping request with data=" << data;

  TimePoint timestamp = getCurrentTime();

  uint64_t bytesScheduledBeforePing = 0;
  size_t pingSize = 0;
  if (writeBufSplit_) {
    // Stick the ping at the end, we don't know that writeBuf_ begins on a
    // frame boundary anymore
    bytesScheduledBeforePing = sessionByteOffset();
    pingSize = codec_->generatePingReply(writeBuf_, data);
  } else {
    // Insert the ping reply to the head of writeBuf_
    folly::IOBufQueue pingBuf(folly::IOBufQueue::cacheChainLength());
    pingSize = codec_->generatePingReply(pingBuf, data);
    pingBuf.append(writeBuf_.move());
    writeBuf_.append(pingBuf.move());
    bytesScheduledBeforePing = bytesScheduled_;
  }

  if (byteEventTracker_) {
    // addPingByteEvent has logic to shift all ByteEvents after
    // 'bytesScheduledBeforePing'.  In the case where we're putting it at the
    // end, there will be no events with an offset as high - so it will be a
    // no-op.
    byteEventTracker_->addPingByteEvent(
        pingSize, timestamp, bytesScheduledBeforePing);
  }
  scheduleWrite();
}

void HTTPSession::onPingReply(uint64_t data) {
  VLOG(4) << *this << " got ping reply with id=" << data;
  if (pingProber_) {
    pingProber_->onPingReply(data);
  }
  if (infoCallback_) {
    infoCallback_->onPingReplyReceived();
  }

  const auto pingReplyEvent =
      HTTPSessionObserverInterface::PingReplyEvent::Builder()
          .setId(data)
          .setTimestamp(HTTPSessionObserverInterface::Clock::now())
          .build();
  sessionObserverContainer_
      .invokeInterfaceMethod<HTTPSessionObserverInterface::Events::pingReply>(
          [&](auto observer, auto observed) {
            observer->pingReply(observed, pingReplyEvent);
          });
}

void HTTPSession::onWindowUpdate(HTTPCodec::StreamID streamID,
                                 uint32_t amount) {
  VLOG(4) << *this << " got window update on streamID=" << streamID << " for "
          << amount << " bytes.";
  HTTPTransaction* txn = findTransaction(streamID);
  if (!txn) {
    // We MUST be using SPDY/3+ if we got WINDOW_UPDATE. The spec says that -
    //
    // A sender should ignore all the WINDOW_UPDATE frames associated with the
    // stream after it send the last frame for the stream.
    //
    // TODO: Only ignore if this is from some past transaction
    return;
  }
  txn->onIngressWindowUpdate(amount);
}

void HTTPSession::onSettings(const SettingsList& settings) {
  DestructorGuard g(this);
  for (auto& setting : settings) {
    if (setting.id == SettingsId::INITIAL_WINDOW_SIZE) {
      onSetSendWindow(setting.value);
    } else if (setting.id == SettingsId::MAX_CONCURRENT_STREAMS) {
      onSetMaxInitiatedStreams(setting.value);
    } else if (setting.id == SettingsId::SETTINGS_HTTP_CERT_AUTH) {
      if (!(verifyCertAuthSetting(setting.value))) {
        return;
      }
    }
  }
  if (codec_->generateSettingsAck(writeBuf_) > 0) {
    scheduleWrite();
  }
  if (infoCallback_) {
    infoCallback_->onSettings(*this, settings);
  }
}

void HTTPSession::onSettingsAck() {
  VLOG(4) << *this << " received settings ack";
  if (infoCallback_) {
    infoCallback_->onSettingsAck(*this);
  }
}

void HTTPSession::onPriority(HTTPCodec::StreamID streamID,
                             const HTTPMessage::HTTP2Priority& pri) {
  if (!getHTTP2PrioritiesEnabled()) {
    return;
  }
  http2::PriorityUpdate h2Pri{
      std::get<0>(pri), std::get<1>(pri), std::get<2>(pri)};
  HTTPTransaction* txn = findTransaction(streamID);
  if (txn) {
    // existing txn, change pri
    txn->onPriorityUpdate(h2Pri);
  } else {
    // virtual node
    txnEgressQueue_.addOrUpdatePriorityNode(streamID, h2Pri);
  }
}

void HTTPSession::onPriority(HTTPCodec::StreamID, const HTTPPriority&) {
}

void HTTPSession::onCertificateRequest(uint16_t requestId,
                                       std::unique_ptr<IOBuf> authRequest) {
  DestructorGuard dg(this);
  VLOG(4) << "CERTIFICATE_REQUEST on" << *this << ", requestId=" << requestId;

  if (!secondAuthManager_) {
    return;
  }

  std::pair<uint16_t, std::unique_ptr<folly::IOBuf>> authenticator;
  auto fizzBase = getTransport()->getUnderlyingTransport<AsyncFizzBase>();
  if (fizzBase) {
    if (isUpstream()) {
      authenticator =
          secondAuthManager_->getAuthenticator(*fizzBase,
                                               TransportDirection::UPSTREAM,
                                               requestId,
                                               std::move(authRequest));
    } else {
      authenticator =
          secondAuthManager_->getAuthenticator(*fizzBase,
                                               TransportDirection::DOWNSTREAM,
                                               requestId,
                                               std::move(authRequest));
    }
  } else {
    VLOG(4) << "Underlying transport does not support secondary "
               "authentication.";
    return;
  }
  if (codec_->generateCertificate(writeBuf_,
                                  authenticator.first,
                                  std::move(authenticator.second)) > 0) {
    scheduleWrite();
  }
}

void HTTPSession::onCertificate(uint16_t certId,
                                std::unique_ptr<IOBuf> authenticator) {
  DestructorGuard dg(this);
  VLOG(4) << "CERTIFICATE on" << *this << ", certId=" << certId;

  if (!secondAuthManager_) {
    return;
  }

  bool isValid = false;
  auto fizzBase = getTransport()->getUnderlyingTransport<AsyncFizzBase>();
  if (fizzBase) {
    if (isUpstream()) {
      isValid = secondAuthManager_->validateAuthenticator(
          *fizzBase,
          TransportDirection::UPSTREAM,
          certId,
          std::move(authenticator));
    } else {
      isValid = secondAuthManager_->validateAuthenticator(
          *fizzBase,
          TransportDirection::DOWNSTREAM,
          certId,
          std::move(authenticator));
    }
  } else {
    VLOG(4) << "Underlying transport does not support secondary "
               "authentication.";
    return;
  }
  if (isValid) {
    VLOG(4) << "Successfully validated the authenticator provided by the peer.";
  } else {
    VLOG(4) << "Failed to validate the authenticator provided by the peer";
  }
}

bool HTTPSession::onNativeProtocolUpgradeImpl(
    HTTPCodec::StreamID streamID,
    std::unique_ptr<HTTPCodec> codec,
    const std::string& protocolString) {
  CHECK_EQ(streamID, 1);
  HTTPTransaction* txn = findTransaction(streamID);
  CHECK(txn);
  // only HTTP1xCodec calls onNativeProtocolUpgrade
  CHECK(!codec_->supportsParallelRequests());

  // Reset to  defaults
  maxConcurrentIncomingStreams_ = kDefaultMaxConcurrentIncomingStreams;
  maxConcurrentOutgoingStreamsRemote_ =
      kDefaultMaxConcurrentOutgoingStreamsRemote;

  // overwrite destination, delay current codec deletion until the end
  // of the event loop
  auto oldCodec = codec_.setDestination(std::move(codec));
  sock_->getEventBase()->runInLoop([oldCodec = std::move(oldCodec)]() {});

  onCodecChanged();

  setupCodec();

  // txn will be streamID=1, have to make a placeholder
  (void)codec_->createStream();

  // This can happen if flow control was not explicitly set, and it got the
  // HTTP1xCodec defaults.  Reset to the new codec default
  if (initialReceiveWindow_ == 0 || receiveStreamWindowSize_ == 0 ||
      receiveSessionWindowSize_ == 0) {
    initialReceiveWindow_ = receiveStreamWindowSize_ =
        receiveSessionWindowSize_ = codec_->getDefaultWindowSize();
  }

  // trigger settings frame that would have gone out in startNow()
  HTTPSettings* settings = codec_->getEgressSettings();
  if (settings) {
    settings->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                         initialReceiveWindow_);
  }
  sendSettings();
  if (connFlowControl_) {
    connFlowControl_->setReceiveWindowSize(writeBuf_,
                                           receiveSessionWindowSize_);
    scheduleWrite();
  }

  // Convert the transaction that contained the Upgrade header
  txn->reset(codec_->supportsStreamFlowControl(),
             initialReceiveWindow_,
             receiveStreamWindowSize_,
             getCodecSendWindowSize());

  if (!transportInfo_.secure &&
      (!transportInfo_.appProtocol || transportInfo_.appProtocol->empty())) {
    transportInfo_.appProtocol = std::make_shared<string>(protocolString);
  }

  return true;
}

void HTTPSession::onSetSendWindow(uint32_t windowSize) {
  VLOG(4) << *this << " got send window size adjustment. new=" << windowSize;
  invokeOnAllTransactions([windowSize](HTTPTransaction* txn) {
    txn->onIngressSetSendWindow(windowSize);
  });
}

void HTTPSession::onSetMaxInitiatedStreams(uint32_t maxTxns) {
  VLOG(4) << *this << " got new maximum number of concurrent txns "
          << "we can initiate: " << maxTxns;
  const bool didSupport = supportsMoreTransactions();
  maxConcurrentOutgoingStreamsRemote_ = maxTxns;
  if (infoCallback_ && didSupport != supportsMoreTransactions()) {
    if (didSupport) {
      infoCallback_->onSettingsOutgoingStreamsFull(*this);
    } else {
      infoCallback_->onSettingsOutgoingStreamsNotFull(*this);
    }
  }
}

size_t HTTPSession::sendSettings() {
  size_t size = codec_->generateSettings(writeBuf_);
  scheduleWrite();
  return size;
}

void HTTPSession::pauseIngress(HTTPTransaction* txn) noexcept {
  VLOG(4) << *this << " pausing streamID=" << txn->getID()
          << ", liveTransactions_ was " << liveTransactions_;
  CHECK_GT(liveTransactions_, 0);
  --liveTransactions_;
  auto exTxns = txn->getExTransactions();
  for (auto it = exTxns.begin(); it != exTxns.end(); ++it) {
    auto exTxn = findTransaction(*it);
    if (exTxn) {
      exTxn->pauseIngress();
    }
  }

  if (liveTransactions_ == 0) {
    pauseReads();
  }
}

void HTTPSession::resumeIngress(HTTPTransaction* txn) noexcept {
  VLOG(4) << *this << " resuming streamID=" << txn->getID()
          << ", liveTransactions_ was " << liveTransactions_;
  ++liveTransactions_;
  auto exTxns = txn->getExTransactions();
  for (auto it = exTxns.begin(); it != exTxns.end(); ++it) {
    auto exTxn = findTransaction(*it);
    if (exTxn) {
      exTxn->resumeIngress();
    }
  }

  // This function can be called from detach(), in which case liveTransactions_
  // may go to 1 briefly, even though we are still anit-pipelining.
  if (liveTransactions_ == 1 &&
      (codec_->supportsParallelRequests() || getPipelineStreamCount() <= 1)) {
    resumeReads();
  }
}

void HTTPSession::transactionTimeout(HTTPTransaction* txn) noexcept {
  // A transaction has timed out.  If the transaction does not have
  // a Handler yet, because we haven't yet received the full request
  // headers, we give it a DirectResponseHandler that generates an
  // error page.
  VLOG(3) << "Transaction timeout for streamID=" << txn->getID();
  if (!codec_->supportsParallelRequests()) {
    // this error should only prevent us from reading/handling more errors
    // on serial streams
    ingressError_ = true;
  }

  if (!txn->getHandler() &&
      txn->getEgressState() == HTTPTransactionEgressSM::State::Start) {
    VLOG(4) << *this << " Timed out receiving headers";
    if (infoCallback_) {
      infoCallback_->onIngressError(*this, kErrorTimeout);
    }
    if (codec_->supportsParallelRequests()) {
      // This can only happen with HTTP/2 where the HEADERS frame is incomplete
      // and we time out waiting for the CONTINUATION.  Abort the request.
      //
      // It would maybe be a little nicer to use the timeout handler for these
      // also.
      txn->sendAbort();
      return;
    }

    VLOG(4) << *this << " creating direct error handler";
    auto handler = getTransactionTimeoutHandler(txn);
    txn->setHandler(handler);
  }

  // Tell the transaction about the timeout.  The transaction will
  // communicate the timeout to the handler, and the handler will
  // decide how to proceed.
  txn->onIngressTimeout();
}

void HTTPSession::sendHeaders(HTTPTransaction* txn,
                              const HTTPMessage& headers,
                              HTTPHeaderSize* size,
                              bool includeEOM) noexcept {
  CHECK(started_);
  unique_ptr<IOBuf> goawayBuf;
  if (draining_ && isUpstream() && codec_->isReusable() &&
      allTransactionsStarted()) {
    // For HTTP/1.1, add Connection: close
    // For SPDY/H2, save the goaway for AFTER the request
    auto writeBuf = writeBuf_.move();
    drainImpl();
    goawayBuf = writeBuf_.move();
    writeBuf_.append(std::move(writeBuf));
  }
  if (isUpstream() || (txn->isPushed() && headers.isRequest())) {
    // upstream picks priority
    if (getHTTP2PrioritiesEnabled()) {
      auto pri = getMessagePriority(&headers);
      if (pri.streamDependency == txn->getID()) {
        LOG(ERROR) << "Attempted to create circular dependency txn=" << *this;
      } else {
        txn->onPriorityUpdate(pri);
      }
    }
  }

  const bool wasReusable = codec_->isReusable();
  const uint64_t oldOffset = sessionByteOffset();
  auto exAttributes = txn->getExAttributes();
  auto assocStream = txn->getAssocTxnId();
  if (exAttributes) {
    codec_->generateExHeader(
        writeBuf_, txn->getID(), headers, *exAttributes, includeEOM, size);
  } else if (headers.isRequest() && assocStream) {
    // Only PUSH_PROMISE (not push response) has an associated stream
    codec_->generatePushPromise(
        writeBuf_, txn->getID(), headers, *assocStream, includeEOM, size);
  } else {
    codec_->generateHeader(writeBuf_, txn->getID(), headers, includeEOM, size);
  }
  const uint64_t newOffset = sessionByteOffset();

  // for push response count towards the MAX_CONCURRENT_STREAMS limit
  if (isDownstream() && headers.isResponse() && txn->isPushed()) {
    incrementOutgoingStreams(txn);
  }

  // For all upstream headers, addFirstHeaderByteEvent should be added
  // For all downstream, only response headers need addFirstHeaderByteEvent
  bool shouldAddFirstHeaderByteEvent =
      isUpstream() || (isDownstream() && headers.isResponse());
  if (shouldAddFirstHeaderByteEvent && newOffset > oldOffset &&
      !txn->testAndSetFirstHeaderByteSent() && byteEventTracker_) {
    byteEventTracker_->addFirstHeaderByteEvent(newOffset, txn);
  }

  if (size) {
    VLOG(4) << *this << " sending headers, size=" << size->compressed
            << ", uncompressedSize=" << size->uncompressed;
  }
  if (goawayBuf) {
    VLOG(4) << *this << " moved GOAWAY to end of writeBuf";
    writeBuf_.append(std::move(goawayBuf));
  }
  if (includeEOM) {
    CHECK_GE(newOffset, oldOffset);
    commonEom(txn, newOffset - oldOffset, true);
  }
  scheduleWrite();
  onHeadersSent(headers, wasReusable);

  // If this is a client sending request headers to upstream
  // invoke requestStarted event for attached observers.
  if (isUpstream()) {
    const auto event =
        HTTPSessionObserverInterface::RequestStartedEvent::Builder()
            .setTimestamp(HTTPSessionObserverInterface::Clock::now())
            .setHeaders(headers.getHeaders())
            .build();
    sessionObserverContainer_.invokeInterfaceMethod<
        HTTPSessionObserverInterface::Events::requestStarted>(
        [&event](auto observer, auto observed) {
          observer->requestStarted(observed, event);
        });
  }
}

void HTTPSession::commonEom(HTTPTransaction* txn,
                            size_t encodedSize,
                            bool piggybacked) noexcept {
  HTTPSessionBase::handleLastByteEvents(byteEventTracker_.get(),
                                        txn,
                                        encodedSize,
                                        sessionByteOffset(),
                                        piggybacked);
  onEgressMessageFinished(txn);
}

size_t HTTPSession::sendBody(HTTPTransaction* txn,
                             std::unique_ptr<folly::IOBuf> body,
                             bool includeEOM,
                             bool trackLastByteFlushed) noexcept {
  uint64_t offset = sessionByteOffset();
  size_t bodyLen = body ? body->computeChainDataLength() : 0;
  size_t encodedSize = codec_->generateBody(writeBuf_,
                                            txn->getID(),
                                            std::move(body),
                                            HTTPCodec::NoPadding,
                                            includeEOM);
  CHECK(inLoopCallback_);
  bodyBytesPerWriteBuf_ += bodyLen;
  if (httpSessionActivityTracker_) {
    httpSessionActivityTracker_->addTrackedEgressByteEvent(
        offset, encodedSize, byteEventTracker_.get(), txn);
  }
  if (encodedSize > 0 && !txn->testAndSetFirstByteSent() && byteEventTracker_) {
    byteEventTracker_->addFirstBodyByteEvent(offset + 1, txn);
  }

  if (trackLastByteFlushed && encodedSize > 0 && byteEventTracker_) {
    byteEventTracker_->addTrackedByteEvent(txn, offset + encodedSize);
  }

  if (includeEOM) {
    VLOG(5) << *this << " sending EOM in body for streamID=" << txn->getID();
    commonEom(txn, encodedSize, true);
  }
  return encodedSize;
}

size_t HTTPSession::sendChunkHeader(HTTPTransaction* txn,
                                    size_t length) noexcept {
  size_t encodedSize =
      codec_->generateChunkHeader(writeBuf_, txn->getID(), length);
  scheduleWrite();
  return encodedSize;
}

size_t HTTPSession::sendChunkTerminator(HTTPTransaction* txn) noexcept {
  size_t encodedSize = codec_->generateChunkTerminator(writeBuf_, txn->getID());
  scheduleWrite();
  return encodedSize;
}

void HTTPSession::onEgressMessageFinished(HTTPTransaction* txn, bool withRST) {
  // If the semantics of the protocol don't permit more messages
  // to be read or sent on this connection, close the socket in one or
  // more directions.
  CHECK(!transactions_.empty());

  if (infoCallback_) {
    infoCallback_->onRequestEnd(*this, txn->getMaxDeferredSize());
  }
  auto oldStreamCount = getPipelineStreamCount();
  decrementTransactionCount(txn, false, true);

  // We should shutdown reads if we are closing with RST or we aren't
  // interested in any further messages (ie if we are a downstream session).
  // Upgraded sessions have independent ingress and egress, and the reads
  // need not be shutdown on egress finish.
  if (withRST) {
    // Let any queued writes complete, but send a RST when done.
    VLOG(4) << *this << " resetting egress after this message";
    resetAfterDrainingWrites_ = true;
    setCloseReason(ConnectionCloseReason::TRANSACTION_ABORT);
    shutdownTransport(true, true);
  } else if (!codec_->isReusable() || readsShutdown()) {
    if (transactions_.size() == 1) {
      // the reason is already set (either not reusable or readshutdown).

      // Defer normal shutdowns until after the end of the loop.  This
      // handles an edge case with direct responses with Connection:
      // close served before ingress EOM.  The remainder of the ingress
      // message may be in the parse loop, so give it a chance to
      // finish out and avoid a kErrorEOF

      // we can get here during shutdown, in that case do not schedule a
      // shutdown callback again
      if (!shutdownTransportCb_) {
        // Just for safety, the following bumps the refcount on this session
        // to keep it live until the loopCb runs
        shutdownTransportCb_.reset(new ShutdownTransportCallback(this));
        sock_->getEventBase()->runInLoop(shutdownTransportCb_.get());
      }
    } else {
      // Parsing of new transactions is not going to work anymore, but we can't
      // shutdown immediately because there are outstanding transactions.
      // This sets the draining_ flag, which will trigger shutdown checks as
      // existing transactions detach.
      drain();
    }
  } else {
    maybeResumePausedPipelinedTransaction(oldStreamCount,
                                          txn->getSequenceNumber());
  }
}

size_t HTTPSession::sendEOM(HTTPTransaction* txn,
                            const HTTPHeaders* trailers) noexcept {

  VLOG(4) << *this << " sending EOM for streamID=" << txn->getID()
          << " trailers=" << (trailers ? "yes" : "no");

  size_t encodedSize = 0;
  if (trailers) {
    encodedSize = codec_->generateTrailers(writeBuf_, txn->getID(), *trailers);
  }

  // Don't send EOM for HTTP2, when trailers sent.
  // sendTrailers already flagged end of stream.
  bool http2Trailers = trailers && isHTTP2CodecProtocol(codec_->getProtocol());
  if (!http2Trailers) {
    encodedSize += codec_->generateEOM(writeBuf_, txn->getID());
  }

  commonEom(txn, encodedSize, false);
  return encodedSize;
}

size_t HTTPSession::sendAbort(HTTPTransaction* txn,
                              ErrorCode statusCode) noexcept {
  // Ask the codec to generate an abort indicator for the transaction.
  // Depending on the protocol, this may be a no-op.
  // Schedule a network write to send out whatever egress we might
  // have queued up.
  VLOG(4) << *this << " sending abort for streamID=" << txn->getID();
  // drain this transaction's writeBuf instead of flushing it
  // then enqueue the abort directly into the Session buffer,
  // hence with max priority.
  size_t rstStreamSize =
      codec_->generateRstStream(writeBuf_, txn->getID(), statusCode);

  if (!codec_->isReusable()) {
    setCloseReason(ConnectionCloseReason::TRANSACTION_ABORT);
  }

  scheduleWrite();

  bool sendTcpRstFallback = !rstStreamSize;
  onEgressMessageFinished(txn, sendTcpRstFallback);
  return rstStreamSize;
}

size_t HTTPSession::sendPriority(HTTPTransaction* txn,
                                 const http2::PriorityUpdate& pri) noexcept {
  return sendPriorityImpl(txn->getID(), pri);
}

size_t HTTPSession::changePriority(HTTPTransaction* /*txn*/,
                                   HTTPPriority /*pri*/) noexcept {
  return 0;
}

void HTTPSession::setSecondAuthManager(
    std::unique_ptr<SecondaryAuthManagerBase> secondAuthManager) {
  secondAuthManager_ = std::move(secondAuthManager);
}

SecondaryAuthManagerBase* HTTPSession::getSecondAuthManager() const {
  return secondAuthManager_.get();
}

/**
 * Send a CERTIFICATE_REQUEST frame. If the underlying protocol doesn't
 * support secondary authentication, this is a no-op and 0 is returned.
 */
size_t HTTPSession::sendCertificateRequest(
    std::unique_ptr<folly::IOBuf> certificateRequestContext,
    std::vector<fizz::Extension> extensions) {
  // Check if both sending and receiving peer have advertised valid
  // SETTINGS_HTTP_CERT_AUTH setting. Otherwise, the frames for secondary
  // authentication should not be sent.
  auto ingressSettings = codec_->getIngressSettings();
  auto egressSettings = codec_->getEgressSettings();
  if (ingressSettings && egressSettings) {
    if (ingressSettings->getSetting(SettingsId::SETTINGS_HTTP_CERT_AUTH, 0) ==
            0 ||
        egressSettings->getSetting(SettingsId::SETTINGS_HTTP_CERT_AUTH, 0) ==
            0) {
      VLOG(4) << "Secondary certificate authentication is not supported.";
      return 0;
    }
  }
  if (!secondAuthManager_) {
    return 0;
  }
  auto authRequest = secondAuthManager_->createAuthRequest(
      std::move(certificateRequestContext), std::move(extensions));
  auto encodedSize = codec_->generateCertificateRequest(
      writeBuf_, authRequest.first, std::move(authRequest.second));
  if (encodedSize > 0) {
    scheduleWrite();
  } else {
    VLOG(4) << "Failed to generate CERTIFICATE_REQUEST frame.";
  }
  return encodedSize;
}

void HTTPSession::decrementTransactionCount(HTTPTransaction* txn,
                                            bool ingressEOM,
                                            bool egressEOM) {
  if ((isUpstream() && !txn->isPushed()) ||
      (isDownstream() && txn->isPushed())) {
    if (ingressEOM && txn->testAndClearIsCountedTowardsStreamLimit()) {
      DCHECK_NE(outgoingStreams_, 0);
      outgoingStreams_--;
    }
  } else {
    if (egressEOM && txn->testAndClearIsCountedTowardsStreamLimit()) {
      DCHECK_NE(incomingStreams_, 0);
      incomingStreams_--;
    }
  }
}

// This is a kludgy function because it requires the caller to remember
// the old value of pipelineStreamCount from before it calls
// decrementTransactionCount.  I'm trying to avoid yet more state in
// HTTPSession.  If decrementTransactionCount actually closed a stream
// and there is still a pipelinable stream, then it was pipelining
bool HTTPSession::maybeResumePausedPipelinedTransaction(size_t oldStreamCount,
                                                        uint32_t txnSeqn) {
  if (!codec_->supportsParallelRequests() && !transactions_.empty()) {
    auto pipelineStreamCount = getPipelineStreamCount();
    if (pipelineStreamCount < oldStreamCount && pipelineStreamCount == 1) {
      // For H1, StreamID = txnSeqn + 1
      auto curStreamId = txnSeqn + 1;
      auto txnIt = transactions_.find(curStreamId + 1);
      CHECK(txnIt != transactions_.end());
      DCHECK(transactionIds_.count(curStreamId + 1));
      auto& nextTxn = txnIt->second;
      DCHECK_EQ(nextTxn.getSequenceNumber(), txnSeqn + 1);
      DCHECK(!nextTxn.isIngressComplete());
      DCHECK(nextTxn.isIngressPaused());
      VLOG(4) << "Resuming paused pipelined txn " << nextTxn;
      nextTxn.resumeIngress();
    }
    return true;
  }
  return false;
}

void HTTPSession::detach(HTTPTransaction* txn) noexcept {
  DestructorGuard guard(this);
  HTTPCodec::StreamID streamID = txn->getID();
  auto txnSeqn = txn->getSequenceNumber();
  auto it = transactions_.find(txn->getID());
  DCHECK(it != transactions_.end());
  DCHECK(transactionIds_.count(txn->getID()));

  if (txn->isIngressPaused()) {
    // Someone detached a transaction that was paused.  Make the resumeIngress
    // call to keep liveTransactions_ in order
    VLOG(4) << *this << " detached paused transaction=" << streamID;
    resumeIngress(txn);
  }

  VLOG(4) << *this << " removing streamID=" << streamID
          << ", liveTransactions was " << liveTransactions_;
  CHECK_GT(liveTransactions_, 0);
  liveTransactions_--;

  if (txn->isPushed()) {
    auto assocTxn = findTransaction(*txn->getAssocTxnId());
    if (assocTxn) {
      assocTxn->removePushedTransaction(streamID);
    }
  }
  if (txn->getControlStream()) {
    auto controlTxn = findTransaction(*txn->getControlStream());
    if (controlTxn) {
      controlTxn->removeExTransaction(streamID);
    }
  }

  // do not track a detached control stream
  controlStreamIds_.erase(txn->getID());

  auto oldStreamCount = getPipelineStreamCount();
  decrementTransactionCount(txn, true, true);
  if (lastTxn_ == txn) {
    lastTxn_ = nullptr;
  }
  DCHECK(transactionIds_.count(it->first));
  transactionIds_.erase(it->first);
  transactions_.erase(it);

  if (transactions_.empty()) {
    HTTPSessionBase::setLatestActive();
    if (pingProber_) {
      pingProber_->cancelProbes();
    }
    if (infoCallback_) {
      infoCallback_->onDeactivateConnection(*this);
    }
    if (getConnectionManager()) {
      getConnectionManager()->onDeactivated(*this);
    }
  }

  if (infoCallback_) {
    infoCallback_->onTransactionDetached(*this);
  }

  if (!readsShutdown()) {
    if (maybeResumePausedPipelinedTransaction(oldStreamCount, txnSeqn)) {
      return;
    } else {
      // this will resume reads if they were paused (eg: 0 HTTP transactions)
      resumeReads();
    }
  }

  if (liveTransactions_ == 0 && transactions_.empty() && !isScheduled()) {
    resetTimeout();
  }

  // It's possible that this is the last transaction in the session,
  // so check whether the conditions for shutdown are satisfied.
  if (transactions_.empty()) {
    if (shouldShutdown()) {
      writesDraining_ = true;
    }
    // Handle the case where we are draining writes but all remaining
    // transactions terminated with no egress.
    if (writesDraining_ && !writesShutdown() && !hasMoreWrites()) {
      shutdownTransport(false, true);
      return;
    }
  }
  checkForShutdown();
}

size_t HTTPSession::sendWindowUpdate(HTTPTransaction* txn,
                                     uint32_t bytes) noexcept {
  size_t sent = codec_->generateWindowUpdate(writeBuf_, txn->getID(), bytes);
  if (sent) {
    scheduleWrite();
  }
  return sent;
}

void HTTPSession::notifyIngressBodyProcessed(uint32_t bytes) noexcept {
  if (HTTPSessionBase::notifyBodyProcessed(bytes)) {
    resumeReads();
  }
  if (connFlowControl_ &&
      connFlowControl_->ingressBytesProcessed(writeBuf_, bytes)) {
    scheduleWrite();
  }
}

void HTTPSession::notifyEgressBodyBuffered(int64_t bytes) noexcept {
  if (HTTPSessionBase::notifyEgressBodyBuffered(bytes, true) &&
      !inLoopCallback_ && !isLoopCallbackScheduled()) {
    sock_->getEventBase()->runInLoop(this);
  }
}

bool HTTPSession::getCurrentTransportInfoWithoutUpdate(
    TransportInfo* tinfo) const {
  auto sock = sock_->getUnderlyingTransport<AsyncSocket>();
  if (sock) {
    tinfo->initWithSocket(sock);
#if defined(__linux__) || defined(__FreeBSD__)
    tinfo->readTcpCongestionControl(sock);
    tinfo->readMaxPacingRate(sock);
#endif // defined(__linux__) || defined(__FreeBSD__)
    tinfo->totalBytes = sock->getRawBytesWritten();
    return true;
  }
  return false;
}

bool HTTPSession::getCurrentTransportInfo(TransportInfo* tinfo) {
  if (getCurrentTransportInfoWithoutUpdate(tinfo)) {
    // some fields are the same with the setup transport info
    tinfo->setupTime = transportInfo_.setupTime;
    tinfo->secure = transportInfo_.secure;
    tinfo->sslSetupTime = transportInfo_.sslSetupTime;
    tinfo->sslVersion = transportInfo_.sslVersion;
    tinfo->sslCipher = transportInfo_.sslCipher;
    tinfo->sslResume = transportInfo_.sslResume;
    tinfo->appProtocol = transportInfo_.appProtocol;
    tinfo->sslError = transportInfo_.sslError;
#if defined(__linux__) || defined(__FreeBSD__)
    tinfo->recvwnd = tinfo->tcpinfo.tcpi_rcv_space
                     << tinfo->tcpinfo.tcpi_rcv_wscale;
    // update connection transport info with the latest RTT
    if (tinfo->tcpinfo.tcpi_rtt > 0) {
      transportInfo_.tcpinfo.tcpi_rtt = tinfo->tcpinfo.tcpi_rtt;
      transportInfo_.rtt = std::chrono::microseconds(tinfo->tcpinfo.tcpi_rtt);
    }
    transportInfo_.rtx = tinfo->rtx;
#elif defined(__APPLE__)
    tinfo->recvwnd = tinfo->tcpinfo.tcpi_rcv_wnd
                     << tinfo->tcpinfo.tcpi_rcv_wscale;
#endif
    return true;
  }
  return false;
}

void HTTPSession::setHeaderIndexingStrategy(
    const HeaderIndexingStrategy* strat) {
  if (isHTTP2CodecProtocol(codec_->getProtocol())) {
    auto* h2Codec = dynamic_cast<HTTP2Codec*>(codec_.getChainEndPtr());
    if (h2Codec) {
      h2Codec->setHeaderIndexingStrategy(strat);
    }
  }
}

void HTTPSession::getFlowControlInfo(HTTPTransaction::FlowControlInfo* info) {
  info->sessionWritesPaused_ = writesPaused();
  info->sessionReadsPaused_ = readsPaused();
  info->flowControlEnabled_ = connFlowControl_ != nullptr;
  if (connFlowControl_) {
    info->sessionRecvWindow_ = connFlowControl_->getRecvWindow().getCapacity();
    info->sessionSendWindow_ = connFlowControl_->getSendWindow().getSize();
    info->sessionRecvOutstanding_ =
        connFlowControl_->getRecvWindow().getOutstanding();
    info->sessionSendOutstanding_ =
        connFlowControl_->getSendWindow().getOutstanding();
  }
}

HTTPTransaction::Transport::Type HTTPSession::getSessionType() const noexcept {
  return getType();
}

unique_ptr<IOBuf> HTTPSession::getNextToSend(bool* cork,
                                             bool* timestampTx,
                                             bool* timestampAck) {
  // limit ourselves to one outstanding write at a time (onWriteSuccess calls
  // scheduleWrite)
  if (numActiveWrites_ > 0 || writesShutdown()) {
    VLOG(4) << "skipping write during this loop, numActiveWrites_="
            << numActiveWrites_ << " writesShutdown()=" << writesShutdown();
    return nullptr;
  }

  // We always tack on at least one body packet to the current write buf
  // This ensures that a short HTTPS response will go out in a single SSL record
  while (!txnEgressQueue_.empty()) {
    uint32_t toSend = kWriteReadyMax;
    if (connFlowControl_) {
      if (connFlowControl_->getAvailableSend() == 0) {
        VLOG(4) << "Session-level send window is full, skipping remaining "
                << "body writes this loop";
        break;
      }
      toSend = std::min(toSend, connFlowControl_->getAvailableSend());
    }
    txnEgressQueue_.nextEgress(nextEgressResults_, false);
    CHECK(!nextEgressResults_.empty()); // Queue was non empty, so this must be
    // The maximum we will send for any transaction in this loop
    uint32_t txnMaxToSend = toSend * nextEgressResults_.front().second;
    if (txnMaxToSend == 0) {
      // toSend is smaller than the number of transactions.  Give all egress
      // to the first transaction
      nextEgressResults_.erase(++nextEgressResults_.begin(),
                               nextEgressResults_.end());
      txnMaxToSend = std::min(toSend, egressBodySizeLimit_);
      nextEgressResults_.front().second = 1;
    }
    if (nextEgressResults_.size() > 1 && txnMaxToSend > egressBodySizeLimit_) {
      // Cap the max to egressBodySizeLimit_, and recompute toSend accordingly
      txnMaxToSend = egressBodySizeLimit_;
      toSend = txnMaxToSend / nextEgressResults_.front().second;
    }
    // split allowed by relative weight, with some minimum
    for (auto txnPair : nextEgressResults_) {
      uint32_t txnAllowed = txnPair.second * toSend;
      if (nextEgressResults_.size() > 1) {
        CHECK_LE(txnAllowed, egressBodySizeLimit_);
      }
      if (connFlowControl_) {
        CHECK_LE(txnAllowed, connFlowControl_->getAvailableSend());
      }
      if (txnAllowed == 0) {
        // The ratio * toSend was so small this txn gets nothing.
        VLOG(4) << *this << " breaking egress loop on 0 txnAllowed";
        break;
      }

      VLOG(4) << *this << " egressing txnID=" << txnPair.first->getID()
              << " allowed=" << txnAllowed;
      txnPair.first->onWriteReady(txnAllowed, txnPair.second);
    }
    nextEgressResults_.clear();
    // it can be empty because of HTTPTransaction rate limiting.  We should
    // change rate limiting to clearPendingEgress while waiting.
    if (!writeBuf_.empty()) {
      break;
    }
  }
  *timestampTx = false;
  *timestampAck = false;
  if (byteEventTracker_) {
    uint64_t needed = byteEventTracker_->preSend(
        cork, timestampTx, timestampAck, bytesWritten_);
    if (needed > 0) {
      VLOG(5) << *this
              << " writeBuf_.chainLength(): " << writeBuf_.chainLength()
              << " txnEgressQueue_.empty(): " << txnEgressQueue_.empty();

      if (needed < writeBuf_.chainLength()) {
        // split the next SOM / EOM chunk
        VLOG(5) << *this << " splitting " << needed << " bytes out of a "
                << writeBuf_.chainLength() << " bytes IOBuf";
        *cork = true;
        if (sessionStats_) {
          sessionStats_->recordPresendIOSplit();
        }
        writeBufSplit_ = true;
        return writeBuf_.split(needed);
      } else {
        CHECK_EQ(needed, writeBuf_.chainLength());
      }
    }
  }

  // cork if there are txns with pending egress and room to send them
  *cork = !txnEgressQueue_.empty() && !isConnWindowFull();
  return writeBuf_.move();
}

void HTTPSession::runLoopCallback() noexcept {
  // We schedule this callback to run at the end of an event
  // loop iteration if either of two conditions has happened:
  //   * The session has generated some egress data (see scheduleWrite())
  //   * Reads have become unpaused (see resumeReads())
  DestructorGuard dg(this);
  inLoopCallback_ = true;
  auto scopeg = folly::makeGuard([this] {
    inLoopCallback_ = false;
    // This ScopeGuard needs to be under the above DestructorGuard
    updatePendingWrites();
    if (!hasMoreWrites() && isDownstream() && !hasPendingEgress()) {
      invokeOnAllTransactions([](HTTPTransaction* txn) {
        txn->checkIfEgressRateLimitedByUpstream();
      });
    }
    checkForShutdown();
  });
  VLOG(5) << *this << " in loop callback";

  if (isDownstream() && !txnEgressQueue_.empty()) {
    const auto event =
        HTTPSessionObserverInterface::PreWriteEvent::Builder()
            .setPendingEgressBytes(getPendingWriteSize())
            .setTimestamp(HTTPSessionObserverInterface::Clock::now())
            .build();
    sessionObserverContainer_
        .invokeInterfaceMethod<HTTPSessionObserverInterface::Events::preWrite>(
            [&event](auto observer, auto observed) {
              observer->preWrite(observed, event);
            });
  }

  for (uint32_t i = 0; i < kMaxWritesPerLoop; ++i) {
    bodyBytesPerWriteBuf_ = 0;
    bool cork = true;
    bool timestampTx = false;
    bool timestampAck = false;
    unique_ptr<IOBuf> writeBuf =
        getNextToSend(&cork, &timestampTx, &timestampAck);

    if (!writeBuf) {
      break;
    }
    uint64_t len = writeBuf->computeChainDataLength();
    VLOG(11) << *this << " bytes of egress to be written: " << len
             << " cork:" << cork << " timestampTx:" << timestampTx
             << " timestampAck:" << timestampAck;
    if (len == 0) {
      checkForShutdown();
      return;
    }

    folly::WriteFlags flags = folly::WriteFlags::NONE;
    flags |= (cork) ? folly::WriteFlags::CORK : folly::WriteFlags::NONE;
    flags |= (timestampTx) ? folly::WriteFlags::TIMESTAMP_TX
                           : folly::WriteFlags::NONE;
    flags |= (timestampAck) ? folly::WriteFlags::EOR : folly::WriteFlags::NONE;
    CHECK(!pendingWrite_.hasValue());
    pendingWrite_.emplace(len, DestructorGuard(this));

    if (!writeTimeout_.isScheduled()) {
      // Any performance concern here?
      wheelTimer_.scheduleTimeout(&writeTimeout_);
    }
    numActiveWrites_++;
    VLOG(4) << *this << " writing " << len
            << ", activeWrites=" << numActiveWrites_ << " cork:" << cork
            << " timestampTx:" << timestampTx
            << " timestampAck:" << timestampAck;
    bytesScheduled_ += len;
    sock_->writeChain(this, std::move(writeBuf), flags);
    if (numActiveWrites_ > 0) {
      updateWriteCount();
      HTTPSessionBase::notifyEgressBodyBuffered(len, false);
      // updateWriteBufSize called in scope guard
      break;
    }
    // writeChain can result in a writeError and trigger the shutdown code path
  }
  if (numActiveWrites_ == 0 && !writesShutdown() && hasMoreWrites() &&
      (!connFlowControl_ || connFlowControl_->getAvailableSend())) {
    scheduleWrite();
  }

  if (readsUnpaused()) {
    processReadData();

    // Install the read callback if necessary
    if (readsUnpaused() && !sock_->getReadCallback()) {
      sock_->setReadCB(this);
    }
  }
  // checkForShutdown is now in ScopeGuard
}

void HTTPSession::scheduleWrite() {
  // Do all the network writes for this connection in one batch at
  // the end of the current event loop iteration.  Writing in a
  // batch helps us packetize the network traffic more efficiently,
  // as well as saving a few system calls.
  if (!isLoopCallbackScheduled() &&
      (writeBuf_.front() || !txnEgressQueue_.empty())) {
    VLOG(5) << *this << " scheduling write callback";
    sock_->getEventBase()->runInLoop(this);
  }
}

void HTTPSession::updateWriteCount() {
  if (numActiveWrites_ > 0 && writesUnpaused()) {
    // Exceeded limit. Pause reading on the incoming stream.
    VLOG(3) << "Pausing egress for " << *this;
    writes_ = SocketState::PAUSED;
  } else if (numActiveWrites_ == 0 && writesPaused()) {
    // Dropped below limit. Resume reading on the incoming stream if needed.
    VLOG(3) << "Resuming egress for " << *this;
    writes_ = SocketState::UNPAUSED;
  }
}

void HTTPSession::shutdownTransport(bool shutdownReads,
                                    bool shutdownWrites,
                                    const std::string& errorMsg,
                                    ProxygenError error) {
  DestructorGuard guard(this);

  // shutdowns not accounted for, shouldn't see any
  setCloseReason(ConnectionCloseReason::UNKNOWN);

  VLOG(4) << "shutdown request for " << *this << ": reads=" << shutdownReads
          << " (currently " << readsShutdown() << "), writes=" << shutdownWrites
          << " (currently " << writesShutdown() << ")";

  bool notifyEgressShutdown = false;
  bool notifyIngressShutdown = false;

  if (!transportInfo_.sslError.empty()) {
    error = kErrorSSL;
  } else if (sock_->error()) {
    VLOG(3) << "shutdown request for " << *this
            << " on bad socket. Shutting down writes too.";
    if (getConnectionCloseReason() == ConnectionCloseReason::IO_WRITE_ERROR) {
      error = kErrorWrite;
    } else {
      error = kErrorConnectionReset;
    }
    shutdownWrites = true;
  } else if (getConnectionCloseReason() == ConnectionCloseReason::TIMEOUT) {
    error = kErrorTimeout;
  }

  if (shutdownReads && !shutdownWrites && flowControlTimeout_.isScheduled()) {
    // reads are dead and writes are blocked on a window update that will never
    // come.  shutdown writes too.
    VLOG(4) << *this
            << " Converting read shutdown to read/write due to"
               " flow control";
    shutdownWrites = true;
  }

  if (shutdownWrites && !writesShutdown()) {
    // Need to shutdown, bypass double GOAWAY
    if (codec_->generateImmediateGoaway(writeBuf_)) {
      scheduleWrite();
    }
    if (!hasMoreWrites() &&
        (transactions_.empty() || codec_->closeOnEgressComplete())) {
      writes_ = SocketState::SHUTDOWN;
      if (byteEventTracker_) {
        byteEventTracker_->drainByteEvents();
      }
      if (resetAfterDrainingWrites_) {
        VLOG(4) << *this << " writes drained, sending RST";
        resetSocketOnShutdown_ = true;
        shutdownReads = true;
      } else {
        VLOG(4) << *this << " writes drained, closing";
        sock_->shutdownWriteNow();
      }
      notifyEgressShutdown = true;
    } else if (!writesDraining_) {
      writesDraining_ = true;
      notifyEgressShutdown = true;
    } // else writes are already draining; don't double notify
  }

  if (shutdownReads && !readsShutdown()) {
    notifyIngressShutdown = true;
    // TODO: send an RST if readBuf_ is non empty?
    shutdownRead();
    if (!transactions_.empty() && error == kErrorConnectionReset) {
      if (infoCallback_) {
        infoCallback_->onIngressError(*this, error);
      }
    } else if (error == kErrorEOF) {
      // Report to the codec that the ingress stream has ended
      codec_->onIngressEOF();
      if (infoCallback_) {
        infoCallback_->onIngressEOF();
      }
    }
    // Once reads are shutdown the parser should stop processing
    codec_->setParserPaused(true);
  }

  if (notifyIngressShutdown || notifyEgressShutdown) {
    auto dir = (notifyIngressShutdown && notifyEgressShutdown)
                   ? HTTPException::Direction::INGRESS_AND_EGRESS
                   : (notifyIngressShutdown ? HTTPException::Direction::INGRESS
                                            : HTTPException::Direction::EGRESS);
    HTTPException ex(dir,
                     folly::to<std::string>("Shutdown transport: ",
                                            getErrorString(error),
                                            errorMsg.empty() ? "" : " ",
                                            errorMsg,
                                            ", ",
                                            getPeerAddress().describe()));
    ex.setProxygenError(error);
    invokeOnAllTransactions([&ex](HTTPTransaction* txn) { txn->onError(ex); });
  }

  if (readsShutdown() && writesShutdown()) {
    // No need to defer shutdown
    shutdownTransportCb_.reset();
  }

  // Close the socket only after the onError() callback on the txns
  // and handler has been detached.
  checkForShutdown();
}

void HTTPSession::shutdownTransportWithReset(ProxygenError errorCode,
                                             const std::string& errorMsg) {
  DestructorGuard guard(this);
  VLOG(4) << "shutdownTransportWithReset";

  if (!readsShutdown()) {
    shutdownRead();
  }

  if (!writesShutdown()) {
    writes_ = SocketState::SHUTDOWN;
    IOBuf::destroy(writeBuf_.move());
    if (pendingWrite_.hasValue()) {
      numActiveWrites_--;
    }
    VLOG(4) << *this << " cancel write timer";
    writeTimeout_.cancelTimeout();
    resetSocketOnShutdown_ = true;
  }

  errorOnAllTransactions(errorCode, errorMsg);
  // drainByteEvents() can call detach(txn), which can in turn call
  // shutdownTransport if we were already draining. To prevent double
  // calling onError() to the transactions, we call drainByteEvents()
  // after we've given the explicit error.
  if (byteEventTracker_) {
    byteEventTracker_->drainByteEvents();
  }

  // HTTPTransaction::onError could theoretically schedule more callbacks,
  // so do this last.
  if (isLoopCallbackScheduled()) {
    cancelLoopCallback();
  }

  // If there was a pending transport shutdown, we don't need it anymore
  shutdownTransportCb_.reset();

  // onError() callbacks or drainByteEvents() could result in txns detaching
  // due to CallbackGuards going out of scope. Close the socket only after
  // the txns are detached.
  checkForShutdown();
}

void HTTPSession::shutdownRead() {
  VLOG(10) << *this << " shutting down reads";
  sock_->setReadCB(nullptr);
  reads_ = SocketState::SHUTDOWN;
  // disable socket timestamp events as we're shutting down reads
  // (once reads are disabled, we cannot receive socket timestamp events)
  if (byteEventTracker_) {
    const auto numEventDrained =
        byteEventTracker_->disableSocketTimestampEvents();
    VLOG(10) << *this << " drained " << numEventDrained
             << " pending socket timestamp events when shutting down reads";
  }
}

void HTTPSession::checkForShutdown() {
  VLOG(10) << *this
           << " checking for shutdown, readShutdown=" << readsShutdown()
           << ", writesShutdown=" << writesShutdown()
           << ", transaction set empty=" << transactions_.empty();

  // Two conditions are required to destroy the HTTPSession:
  //   * All writes have been finished.
  //   * There are no transactions remaining on the session.
  if (writesShutdown() && transactions_.empty() && !isLoopCallbackScheduled()) {
    VLOG(4) << "destroying " << *this;
    shutdownRead();
    auto asyncSocket = sock_->getUnderlyingTransport<folly::AsyncSocket>();
    if (asyncSocket) {
      asyncSocket->setBufferCallback(nullptr);
    }
    if (resetSocketOnShutdown_) {
      sock_->closeWithReset();
    } else {
      sock_->closeNow();
    }
    destroy();
  }
}

void HTTPSession::drain() {
  if (!draining_) {
    VLOG(4) << *this << " draining";
    draining_ = true;
    setCloseReason(ConnectionCloseReason::SHUTDOWN);

    if (allTransactionsStarted()) {
      drainImpl();
    }
    if (transactions_.empty() && isUpstream()) {
      // We don't do this for downstream since we need to wait for
      // inflight requests to arrive
      VLOG(4) << *this << " shutdown from drain";
      shutdownTransport(true, true);
    }
  } else {
    VLOG(4) << *this << " already draining";
  }
}

void HTTPSession::drainImpl() {
  setCloseReason(ConnectionCloseReason::SHUTDOWN);
  // For HTTP/2, if we haven't started yet then we cannot send a GOAWAY frame
  // since we haven't sent the initial SETTINGS frame. Defer sending that
  // GOAWAY until the initial SETTINGS is sent.
  if (started_) {
    if (codec_->generateGoaway(writeBuf_) > 0) {
      scheduleWrite();
    }
    auto controller = getController();
    if (controller && codec_->isWaitingToDrain()) {
      wheelTimer_.scheduleTimeout(&drainTimeout_,
                                  controller->getGracefulShutdownTimeout());
    }
  }
}

bool HTTPSession::shouldShutdown() const {
  return draining_ && allTransactionsStarted() &&
         (!codec_->supportsParallelRequests() || isUpstream() ||
          !codec_->isReusable());
}

size_t HTTPSession::sendPing() {
  return sendPing(folly::Random::rand64());
}

size_t HTTPSession::sendPing(uint64_t data) {
  const size_t bytes = codec_->generatePingRequest(writeBuf_, data);
  if (bytes) {
    scheduleWrite();
  }
  return bytes;
}

void HTTPSession::enablePingProbes(std::chrono::seconds interval,
                                   std::chrono::seconds timeout,
                                   bool extendIntervalOnIngress,
                                   bool immediate) {
  if (isHTTP2CodecProtocol(codec_->getProtocol())) {
    pingProber_ = std::make_unique<PingProber>(
        *this, interval, timeout, extendIntervalOnIngress, immediate);
  }
}

HTTPSession::PingProber::PingProber(HTTPSession& session,
                                    std::chrono::seconds interval,
                                    std::chrono::seconds timeout,
                                    bool extendIntervalOnIngress,
                                    bool immediate)
    : session_(session),
      interval_(interval),
      timeout_(timeout),
      extendIntervalOnIngress_(extendIntervalOnIngress) {
  if (immediate) {
    timeoutExpired();
  } else if (session_.getNumStreams() > 0) {
    startProbes();
  } // else session will start them when a stream is created
}

void HTTPSession::PingProber::startProbes() {
  refreshTimeout(/*onIngress=*/false);
}

void HTTPSession::PingProber::cancelProbes() {
  if (pingVal_) {
    VLOG(4) << "Canceling active probe sess=" << session_;
    pingVal_.reset();
  }
  cancelTimeout();
}

void HTTPSession::PingProber::refreshTimeout(bool onIngress) {
  if (!pingVal_ && (!onIngress || extendIntervalOnIngress_)) {
    VLOG(4) << "Scheduling next ping probe for sess=" << session_;
    session_.getEventBase()->timer().scheduleTimeout(this, interval_);
  }
}

void HTTPSession::PingProber::timeoutExpired() noexcept {
  if (pingVal_) {
    VLOG(3) << "Ping probe timed out, dropping connection sess=" << session_;
    if (auto sessionStats = session_.sessionStats_) {
      sessionStats->recordSessionPeriodicPingProbeTimeout();
    }
    session_.dropConnection("Ping probe timed out");
  } else {
    pingVal_ = folly::Random::rand64();
    VLOG(4) << "Sending ping probe with value=" << *pingVal_
            << " sess=" << session_;
    session_.sendPing(*pingVal_);
    session_.getEventBase()->timer().scheduleTimeout(this, timeout_);
  }
}

void HTTPSession::PingProber::onPingReply(uint64_t pingVal) {
  if (!pingVal_ || *pingVal_ != pingVal) {
    // This can happen if someone calls sendPing() manually
    VLOG(3) << "Received unexpected PING reply=" << pingVal << " expecting="
            << ((pingVal_) ? folly::to<std::string>(*pingVal_)
                           : std::string("none"));
    return;
  }
  VLOG(4) << "Received expected ping, rescheduling";
  pingVal_.reset();
  refreshTimeout(/*onIngress=*/false);
}

HTTPCodec::StreamID HTTPSession::sendPriority(http2::PriorityUpdate pri) {
  if (!codec_->supportsParallelRequests()) {
    // For HTTP/1.1, don't call createStream()
    return 0;
  }
  auto id = codec_->createStream();
  sendPriority(id, pri);
  return id;
}

size_t HTTPSession::sendPriority(HTTPCodec::StreamID id,
                                 http2::PriorityUpdate pri) {
  auto res = sendPriorityImpl(id, pri);
  txnEgressQueue_.addOrUpdatePriorityNode(id, pri);
  return res;
}

size_t HTTPSession::sendPriorityImpl(HTTPCodec::StreamID id,
                                     http2::PriorityUpdate pri) {
  CHECK_NE(id, 0);
  const size_t bytes = codec_->generatePriority(
      writeBuf_,
      id,
      std::make_tuple(pri.streamDependency, pri.exclusive, pri.weight));
  if (bytes) {
    scheduleWrite();
  }
  return bytes;
}

HTTPTransaction* HTTPSession::findTransaction(HTTPCodec::StreamID streamID) {
  if (lastTxn_ && streamID == lastTxn_->getID()) {
    return lastTxn_;
  }
  auto it = transactions_.find(streamID);
  if (it == transactions_.end()) {
    DCHECK(transactionIds_.count(streamID) == 0);
    return nullptr;
  } else {
    DCHECK(transactionIds_.count(streamID));
    lastTxn_ = &it->second;
    return lastTxn_;
  }
}

HTTPTransaction* HTTPSession::createTransaction(
    HTTPCodec::StreamID streamID,
    const folly::Optional<HTTPCodec::StreamID>& assocStreamID,
    const folly::Optional<HTTPCodec::ExAttributes>& exAttributes,
    const http2::PriorityUpdate& priority,
    ProxygenError* error) {
  if (!sock_->good() || writesShutdown()) {
    // Refuse to add a transaction on a closing session
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorBadSocket);
    return nullptr;
  }

  if (transactions_.count(streamID)) {
    // Refuse to add a transaction if a transaction of that ID already exists.
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorDuplicatedStreamId);
    return nullptr;
  }

  if (transactions_.empty()) {
    if (pingProber_) {
      pingProber_->startProbes();
    }
    if (infoCallback_) {
      infoCallback_->onActivateConnection(*this);
    }
    if (getConnectionManager()) {
      getConnectionManager()->onActivated(*this);
    }
    HTTPSessionBase::onCreateTransaction();
  }

  auto matchPair = transactions_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(streamID),
      std::forward_as_tuple(codec_->getTransportDirection(),
                            streamID,
                            getNumTxnServed(),
                            *this,
                            txnEgressQueue_,
                            wheelTimer_.getWheelTimer(),
                            wheelTimer_.getDefaultTimeout(),
                            sessionStats_,
                            codec_->supportsStreamFlowControl(),
                            initialReceiveWindow_,
                            getCodecSendWindowSize(),
                            priority,
                            assocStreamID,
                            exAttributes,
                            setIngressTimeoutAfterEom_));

  CHECK(matchPair.second) << "Emplacement failed, despite earlier "
                             "existence check.";
  transactionIds_.emplace(streamID);

  HTTPTransaction* txn = &matchPair.first->second;

  if (getNumTxnServed() > 0) {
    auto stats = txn->getSessionStats();
    if (stats != nullptr) {
      stats->recordSessionReused();
    }
  }

  VLOG(5) << *this << " adding streamID=" << txn->getID()
          << ", liveTransactions_ was " << liveTransactions_;

  ++liveTransactions_;
  incrementSeqNo();
  txn->setReceiveWindow(receiveStreamWindowSize_);

  if (isUpstream() && !txn->isPushed()) {
    incrementOutgoingStreams(txn);
    // do not count towards MAX_CONCURRENT_STREAMS for PUSH_PROMISE
  } else if (!(isDownstream() && txn->isPushed())) {
    incrementIncomingStreams(txn);
  }
  // Downstream push is counted in HTTPSession::sendHeaders

  if (infoCallback_) {
    infoCallback_->onTransactionAttached(*this);
  }

  return txn;
}

void HTTPSession::incrementOutgoingStreams(HTTPTransaction* txn) {
  DCHECK(txn);
  outgoingStreams_++;
  txn->setIsCountedTowardsStreamLimit();
  HTTPSessionBase::onNewOutgoingStream(outgoingStreams_);
}

void HTTPSession::incrementIncomingStreams(HTTPTransaction* txn) {
  DCHECK(txn);
  incomingStreams_++;
  txn->setIsCountedTowardsStreamLimit();
}

void HTTPSession::writeSuccess() noexcept {
  CHECK(pendingWrite_.hasValue());
  DestructorGuard dg(this);
  auto bytesWritten = pendingWrite_->first;
  bytesWritten_ += bytesWritten;
  transportInfo_.totalBytes += bytesWritten;
  CHECK(writeTimeout_.isScheduled());
  VLOG(10) << "Cancel write timer on last successful write";
  writeTimeout_.cancelTimeout();
  pendingWrite_.reset();

  if (infoCallback_) {
    infoCallback_->onWrite(*this, bytesWritten);
  }

  VLOG(5) << "total bytesWritten_: " << bytesWritten_;

  // processByteEvents will return true if it has been replaced with another
  // tracker in the middle and needs to be re-run.  Should happen at most
  // once.  while with no body is intentional
  while (byteEventTracker_ && byteEventTracker_->processByteEvents(
                                  byteEventTracker_, bytesWritten_)) {
  } // pass

  if ((!codec_->isReusable() || readsShutdown()) && (transactions_.empty())) {
    if (!codec_->isReusable()) {
      // Shouldn't happen unless there is a bug. This can only happen when
      // someone calls shutdownTransport, but did not specify a reason before.
      setCloseReason(ConnectionCloseReason::UNKNOWN);
    }
    VLOG(4) << *this << " shutdown from onWriteSuccess";
    shutdownTransport(true, true);
  }
  numActiveWrites_--;
  if (!inLoopCallback_) {
    updateWriteCount();
    // safe to resume here:
    updateWriteBufSize(-folly::to<int64_t>(bytesWritten));
    // PRIO_FIXME: this is done because of the corking business...
    //             in the future we may want to have a pull model
    //             whereby the socket asks us for a given amount of
    //             data to send...
    if (numActiveWrites_ == 0 && hasMoreWrites()) {
      runLoopCallback();
    } else if (isDownstream() && !hasPendingEgress()) {
      invokeOnAllTransactions([](HTTPTransaction* txn) {
        txn->checkIfEgressRateLimitedByUpstream();
      });
    }
  }
  onWriteCompleted();

  if (egressBytesLimit_ > 0 && bytesWritten_ >= egressBytesLimit_) {
    VLOG(4) << "Egress limit reached, shutting down "
               "session (egressed "
            << bytesWritten_ << ", limit set to " << egressBytesLimit_ << ")";
    shutdownTransport(true, true);
  }
}

void HTTPSession::writeErr(size_t bytesWritten,
                           const AsyncSocketException& ex) noexcept {
  VLOG(4) << *this << " write error: " << ex.what();
  DestructorGuard dg(this);
  DCHECK(pendingWrite_.hasValue());
  pendingWrite_.reset();
  if (infoCallback_) {
    infoCallback_->onWrite(*this, bytesWritten);
  }

  auto sslEx = dynamic_cast<const folly::SSLException*>(&ex);
  // Save the SSL error, if there was one.  It will be recorded later
  if (sslEx && sslEx->getSSLError() == folly::SSLError::SSL_ERROR) {
    transportInfo_.sslError = ex.what();
  }

  setCloseReason(ConnectionCloseReason::IO_WRITE_ERROR);
  shutdownTransportWithReset(kErrorWrite, ex.what());
}

void HTTPSession::onWriteCompleted() {
  if (!writesDraining_) {
    return;
  }

  if (numActiveWrites_) {
    return;
  }

  // Don't shutdown if there might be more writes
  if (pendingWrite_.hasValue()) {
    return;
  }

  // All finished draining writes, so shut down the egress
  shutdownTransport(false, true);
}

void HTTPSession::onSessionParseError(const HTTPException& error) {
  VLOG(4) << *this << " session layer parse error. Terminate the session.";
  if (error.hasCodecStatusCode()) {
    std::unique_ptr<folly::IOBuf> errorMsg =
        folly::IOBuf::copyBuffer(error.what());
    if (codec_->generateImmediateGoaway(
            writeBuf_, error.getCodecStatusCode(), std::move(errorMsg))) {
      scheduleWrite();
    }
  }
  if (error.hasProxygenError() && error.getProxygenError() == kErrorDropped) {
    // Codec is requesting a connection drop
    dropConnection();
  } else {
    setCloseReason(ConnectionCloseReason::SESSION_PARSE_ERROR);
    shutdownTransport(true,
                      true,
                      "",
                      error.hasProxygenError() ? error.getProxygenError()
                                               : kErrorMalformedInput);
  }
}

void HTTPSession::onNewTransactionParseError(HTTPCodec::StreamID streamID,
                                             const HTTPException& error) {
  VLOG(4) << *this << " parse error with new transaction";
  if (error.hasCodecStatusCode()) {
    codec_->generateRstStream(writeBuf_, streamID, error.getCodecStatusCode());
    scheduleWrite();
  }
  if (!codec_->isReusable()) {
    // HTTP 1x codec does not support per stream abort so this will
    // render the codec not reusable
    setCloseReason(ConnectionCloseReason::SESSION_PARSE_ERROR);
  }
}

void HTTPSession::pauseReads() {
  // Make sure the parser is paused.  Note that if reads are shutdown
  // before they are paused, we never make it past the if.
  codec_->setParserPaused(true);
  if (!readsUnpaused() ||
      (codec_->supportsParallelRequests() && !ingressLimitExceeded())) {
    return;
  }
  pauseReadsImpl();
}

void HTTPSession::pauseReadsImpl() {
  VLOG(4) << *this << ": pausing reads";
  if (infoCallback_) {
    infoCallback_->onIngressPaused(*this);
  }
  cancelTimeout();
  sock_->setReadCB(nullptr);
  reads_ = SocketState::PAUSED;
}

void HTTPSession::resumeReads() {
  if (!readsPaused() ||
      (codec_->supportsParallelRequests() && ingressLimitExceeded())) {
    return;
  }
  resumeReadsImpl();
}

void HTTPSession::resumeReadsImpl() {
  VLOG(4) << *this << ": resuming reads";
  resetTimeout();
  reads_ = SocketState::UNPAUSED;
  codec_->setParserPaused(false);
  if (!isLoopCallbackScheduled()) {
    sock_->getEventBase()->runInLoop(this);
  }
}

bool HTTPSession::hasMoreWrites() const {
  VLOG(10) << __PRETTY_FUNCTION__ << " numActiveWrites_: " << numActiveWrites_
           << " pendingWrite_.hasValue(): " << pendingWrite_.hasValue()
           << " txnEgressQueue_.empty(): " << txnEgressQueue_.empty();

  return (numActiveWrites_ != 0) || pendingWrite_.hasValue() ||
         writeBuf_.front() || !txnEgressQueue_.empty();
}

void HTTPSession::errorOnAllTransactions(ProxygenError err,
                                         const std::string& errorMsg) {
  std::vector<HTTPCodec::StreamID> ids;
  ids.reserve(transactionIds_.size());
  std::copy(
      transactionIds_.begin(), transactionIds_.end(), std::back_inserter(ids));
  errorOnTransactionIds(ids, err, errorMsg);
}

void HTTPSession::errorOnTransactionIds(
    const std::vector<HTTPCodec::StreamID>& ids,
    ProxygenError err,
    const std::string& errorMsg) {
  std::string extraErrorMsg;
  if (!errorMsg.empty()) {
    extraErrorMsg = folly::to<std::string>(". ", errorMsg);
  }

  for (auto id : ids) {
    HTTPException ex(
        HTTPException::Direction::INGRESS_AND_EGRESS,
        folly::to<std::string>(
            getErrorString(err), " on transaction id: ", id, extraErrorMsg));
    ex.setProxygenError(err);
    errorOnTransactionId(id, std::move(ex));
  }
}

void HTTPSession::errorOnTransactionId(HTTPCodec::StreamID id,
                                       HTTPException ex) {
  auto txn = findTransaction(id);
  if (txn != nullptr) {
    txn->onError(std::move(ex));
  }
}

void HTTPSession::onConnectionSendWindowOpen() {
  flowControlTimeout_.cancelTimeout();
  // We can write more now. Schedule a write.
  scheduleWrite();
}

void HTTPSession::onConnectionSendWindowClosed() {
  if (!txnEgressQueue_.empty()) {
    VLOG(4) << *this << " session stalled by flow control";
    if (sessionStats_) {
      sessionStats_->recordSessionStalled();
    }
  }
  DCHECK(!flowControlTimeout_.isScheduled());
  if (infoCallback_) {
    infoCallback_->onFlowControlWindowClosed(*this);
  }
  auto timeout = flowControlTimeout_.getTimeoutDuration();
  if (timeout != std::chrono::milliseconds(0)) {
    wheelTimer_.scheduleTimeout(&flowControlTimeout_, timeout);
  } else {
    wheelTimer_.scheduleTimeout(&flowControlTimeout_);
  }
}

void HTTPSession::invalidStream(HTTPCodec::StreamID stream, ErrorCode code) {
  if (!codec_->supportsParallelRequests()) {
    LOG(ERROR) << "Invalid stream on non-parallel codec.";
    return;
  }

  HTTPException err(HTTPException::Direction::INGRESS_AND_EGRESS,
                    folly::to<std::string>("invalid stream=", stream));
  // TODO: Below line will change for HTTP/2 -- just call a const getter
  // function for the status code.
  err.setCodecStatusCode(code);
  onError(stream, err, true);
}

void HTTPSession::onPingReplyLatency(int64_t latency) noexcept {
  if (infoCallback_ && latency >= 0) {
    infoCallback_->onPingReplySent(latency);
  }
}

void HTTPSession::onDeleteTxnByteEvent() noexcept {
  if (readsShutdown()) {
    shutdownTransport(true, transactions_.empty());
  }
}

void HTTPSession::onEgressBuffered() {
  if (infoCallback_) {
    infoCallback_->onEgressBuffered(*this);
  }
}

void HTTPSession::onEgressBufferCleared() {
  if (infoCallback_) {
    infoCallback_->onEgressBufferCleared(*this);
  }
}

void HTTPSession::onReplaySafe() noexcept {
  CHECK(sock_);
  sock_->setReplaySafetyCallback(nullptr);

  if (infoCallback_) {
    infoCallback_->onFullHandshakeCompletion(*this);
  }

  for (auto callback : waitingForReplaySafety_) {
    callback->onReplaySafe();
  }
  waitingForReplaySafety_.clear();
}

void HTTPSession::onTxnByteEventWrittenToBuf(const ByteEvent& event) noexcept {
  if (!sock_->isEorTrackingEnabled() || event.getTransaction() == nullptr ||
      event.byteOffset_ != sock_->getAppBytesWritten()) {
    return;
  }

  // by default, we're going to add TX and ACK events whenever they're available
  const auto& txn = event.getTransaction();
  if (event.timestampTx_) {
    byteEventTracker_->addTxByteEvent(
        sock_->getRawBytesWritten(), event.eventType_, txn);
  }
  if (event.timestampAck_) {
    byteEventTracker_->addAckByteEvent(
        sock_->getRawBytesWritten(), event.eventType_, txn);
  }
}

bool HTTPSession::isDetachable(bool checkSocket) const {
  if (checkSocket && sock_ && !sock_->isDetachable()) {
    return false;
  }
  return transactions_.size() == 0 && getNumIncomingStreams() == 0 &&
         !writesPaused() && !flowControlTimeout_.isScheduled() &&
         !writeTimeout_.isScheduled() && !drainTimeout_.isScheduled();
}

void HTTPSession::invokeOnAllTransactions(
    folly::Function<void(HTTPTransaction*)> fn) {
  DestructorGuard g(this);
  std::vector<HTTPCodec::StreamID> ids;
  ids.reserve(transactionIds_.size());
  std::copy(
      transactionIds_.begin(), transactionIds_.end(), std::back_inserter(ids));
  for (auto idit = ids.begin(); idit != ids.end() && !transactions_.empty();
       ++idit) {
    auto txn = findTransaction(*idit);
    if (txn != nullptr) {
      fn(txn);
    }
  }
}

} // namespace proxygen
