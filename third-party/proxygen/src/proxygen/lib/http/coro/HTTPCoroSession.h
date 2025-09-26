/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPBodyEventQueue.h"
#include "proxygen/lib/http/coro/HTTPByteEventHelpers.h"
#include "proxygen/lib/http/coro/HTTPSource.h"
#include "proxygen/lib/http/coro/HTTPSourceHolder.h"
#include "proxygen/lib/http/coro/HTTPStreamSource.h"
#include "proxygen/lib/http/coro/util/AwaitableKeepAlive.h"
#include "proxygen/lib/http/coro/util/CancellableBaton.h"
#include "proxygen/lib/http/coro/util/DetachableExecutor.h"
#include "proxygen/lib/http/coro/util/Refcount.h"
#include "proxygen/lib/http/coro/util/WindowContainer.h"
#include <folly/container/EvictingCacheMap.h>
#include <folly/container/F14Map.h>
#include <folly/coro/AsyncScope.h>
#include <folly/coro/Task.h>
#include <folly/io/coro/Transport.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/codec/HQMultiCodec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/http/codec/QPACKDecoderCodec.h>
#include <proxygen/lib/http/codec/QPACKEncoderCodec.h>
#include <proxygen/lib/http/codec/RateLimitFilter.h>
#include <proxygen/lib/http/session/HQStreamDispatcher.h>
#include <proxygen/lib/http/session/QuicProtocolInfo.h>
#include <proxygen/lib/utils/UtilInl.h>
#include <quic/api/QuicSocket.h>
#include <quic/common/Optional.h>
#include <quic/common/events/FollyQuicEventBase.h>
#include <quic/priority/HTTPPriorityQueue.h>
#include <wangle/acceptor/ManagedConnection.h>
#include <wangle/acceptor/TransportInfo.h>

namespace proxygen {
class HTTPSessionStats;
class WebTransport;
} // namespace proxygen

namespace proxygen::coro {

class HTTPCoroSession;
class LifecycleObserver;

class HTTPSessionContext
    : public detail::EnableAwaitableKeepAlive<HTTPSessionContext> {
 public:
  virtual ~HTTPSessionContext() override = default;
  virtual folly::EventBase* getEventBase() const = 0;
  virtual void initiateDrain() = 0;
  virtual bool isDownstream() const = 0;
  virtual bool isUpstream() const = 0;
  virtual uint64_t getSessionID() const = 0;
  virtual CodecProtocol getCodecProtocol() const = 0;
  virtual const folly::SocketAddress& getLocalAddress() const = 0;
  virtual const folly::SocketAddress& getPeerAddress() const = 0;
  FOLLY_DEPRECATED("Unsafe")
  virtual const folly::AsyncTransport* getAsyncTransport() const {
    return nullptr;
  }
  virtual int getAsyncTransportFD() const {
    return -1;
  }
  virtual quic::QuicSocket* getQUICTransport() const {
    return nullptr;
  }
  virtual const wangle::TransportInfo& getSetupTransportInfo() const = 0;
  virtual bool getCurrentTransportInfo(
      wangle::TransportInfo* tinfo, bool includeSetupFields = false) const = 0;
  virtual size_t getSequenceNumberFromStreamId(
      HTTPCodec::StreamID streamId) const = 0;
  virtual uint16_t getDatagramSizeLimit() const = 0;
  virtual const folly::AsyncTransportCertificate* getPeerCertificate()
      const = 0;
  virtual void addLifecycleObserver(LifecycleObserver* cb) = 0;
  virtual void removeLifecycleObserver(LifecycleObserver* cb) = 0;
};

using HTTPSessionContextPtr = detail::KeepAlivePtr<HTTPSessionContext>;

/**
 * Abstract handler for HTTP requests.
 */
class HTTPHandler {
 public:
  virtual ~HTTPHandler() {
  }
  virtual folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) = 0;
};

class LifecycleObserver {
 public:
  virtual ~LifecycleObserver() = default;
  // Note: you must not start any asynchronous work from onCreate()
  virtual void onAttached(HTTPCoroSession&) {
  }
  /**
   * Includes the stream these bytes belong to, or folly::none if unknown.
   * bytesRead can be 0 if the stream ended.
   */
  virtual void onRead(const HTTPCoroSession& /*sess*/,
                      size_t /*bytesRead*/,
                      folly::Optional<HTTPCodec::StreamID> /*stream id*/) {
  }
  virtual void onWrite(const HTTPCoroSession&, size_t /*bytesWritten*/) {
  }
  virtual void onActivateConnection(const HTTPCoroSession&) {
  }
  virtual void onTransactionAttached(const HTTPCoroSession&) {
  }
  virtual void onTransactionDetached(const HTTPCoroSession&) {
  }
  virtual void onDeactivateConnection(const HTTPCoroSession&) {
  }
  virtual void onDrainStarted(const HTTPCoroSession&) {
  }
  virtual void onIngressError(const HTTPCoroSession&, ProxygenError) {
  }
  virtual void onIngressEOF(const HTTPCoroSession&) {
  }
  virtual void onRequestBegin(const HTTPCoroSession&) {
  }
  virtual void onRequestEnd(const HTTPCoroSession&,
                            uint32_t /*maxIngressQueueSize*/) {
  }
  virtual void onIngressMessage(const HTTPCoroSession&, const HTTPMessage&) {
  }
  // Note: you must not start any asynchronous work from onDestroy()
  virtual void onDestroy(const HTTPCoroSession&) {
  }
  virtual void onPingReplySent(int64_t /*latency*/) {
  }
  virtual void onPingReplyReceived() {
  }
  virtual void onSettingsOutgoingStreamsFull(const HTTPCoroSession&) {
  }
  virtual void onSettingsOutgoingStreamsNotFull(const HTTPCoroSession&) {
  }
  virtual void onFlowControlWindowClosed(const HTTPCoroSession&) {
  }
  virtual void onSettings(const HTTPCoroSession&, const SettingsList&) {
  }
  virtual void onSettingsAck(const HTTPCoroSession&) {
  }
  // GOAWAY notification
  // Currently use ErrorCode instead of HTTPErrorCode to match the error code in
  // HTTPCoroSession::onGoaway
  virtual void onGoaway(const HTTPCoroSession&,
                        const uint64_t /*lastGoodStreamID*/,
                        const ErrorCode) {
  }
};

/**
 * Class for managing an HTTP/1.x or HTTP/2 connection.
 *
 * At its core, it runs two coroutines, a read loop and a write loop.
 *
 * The read loop reads bytes from the socket and parses them with a codec,
 * populating HTTPStreamSource's with headers, body and the like.
 *
 * The writer awaits the write buffer to have something in it and writes it to
 * the socket.
 *
 * For DOWNSTREAM (eg server), the session starts a new coroutine when the
 * request headers arrive.  This coroutine uses the handler factory to
 * instantiate a handler.  The coroutine then invokes handler to the request
 * and return source for the response.  The coroutine reads the response
 * from the source and serializes it into the write buffer.
 *
 * For UPSTREAM (eg: client), the caller provides a source for the request to
 * the sendRequest method, which reads and serializes it to the write buffer.
 * If the request contains a body, a new coroutine is started to read and
 * serialize it into the write buffer.  Meanwhile, sendRequest returns a source
 * from which the caller can read the response.
 *
 * All HTTPSources are passes as a raw pointer, and self-manage their own
 * lifetime.
 *
 * Currently supports
 *  * trailers
 *  * flow-control
 *  * Non-default SETTINGS
 *  * Stream Limits
 *  * GOAWAY
 *  * Error handling
 *  * Custom timeouts
 *
 * Does not (yet) support
 *  * Priority
 */
class HTTPCoroSession
    : public HTTPCodec::Callback
    , public HTTPSessionContext
    , public wangle::ManagedConnection
    , private HTTPStreamSource::Callback
    , private HTTPBodyEventQueue::Callback {
 protected:
  struct StreamState;

 public:
  static HTTPCoroSession* makeUpstreamCoroSession(
      std::unique_ptr<folly::coro::TransportIf> coroTransport,
      std::unique_ptr<HTTPCodec> codec,
      wangle::TransportInfo tinfo);

  static HTTPCoroSession* makeDownstreamCoroSession(
      std::unique_ptr<folly::coro::TransportIf> coroTransport,
      std::shared_ptr<HTTPHandler> handler,
      std::unique_ptr<HTTPCodec> codec,
      wangle::TransportInfo tinfo);

  static HTTPCoroSession* makeUpstreamCoroSession(
      std::shared_ptr<quic::QuicSocket> sock,
      std::unique_ptr<hq::HQMultiCodec> codec,
      wangle::TransportInfo tinfo);

  static HTTPCoroSession* makeDownstreamCoroSession(
      std::shared_ptr<quic::QuicSocket> sock,
      std::shared_ptr<HTTPHandler> handler,
      std::unique_ptr<hq::HQMultiCodec> codec,
      wangle::TransportInfo tinfo);

  virtual ~HTTPCoroSession() override;

  uint64_t getSessionID() const override {
    return sessionID_;
  }

  uint32_t getNextStreamSeqNum() const {
    return nextStreamSequenceNumber_;
  }

  CodecProtocol getCodecProtocol() const override {
    return codec_->getProtocol();
  }

  size_t getSequenceNumberFromStreamId(
      HTTPCodec::StreamID streamId) const override {
    return HTTPCodec::streamIDToSeqNo(getCodecProtocol(), streamId);
  }

  folly::EventBase* getEventBase() const override {
    return eventBase_.get();
  }

  virtual folly::coro::TaskWithExecutor<void> run() = 0;

  void setSessionStats(HTTPSessionStats* stats) {
    sessionStats_ = stats;
  }

  // Upstream API
  // For most simple cases, call sendRequest without a reservation.  If there
  // no available streams, it can fail with REFUSED_STREAM.
  //
  // When sharing a session (eg: session pooling), call reserveRequest first.
  // This can also fail with REFUSED_STREAM, but does not consume a
  // requestSource, so the caller can try to reserve on another session.
  folly::coro::Task<HTTPSourceHolder> sendRequest(
      HTTPSourceHolder requestSource);

  struct RequestReservation {
    RequestReservation() = default;
    RequestReservation(RequestReservation&& goner) noexcept
        : session_(goner.session_) {
      goner.session_ = nullptr;
    }
    RequestReservation& operator=(RequestReservation&& goner) {
      session_ = goner.session_;
      goner.session_ = nullptr;
      return *this;
    }
    ~RequestReservation() {
      cancel();
    }
    void cancel() {
      if (session_) {
        auto session = session_;
        consume();
        session->transactionDetached();
      }
    }
    bool fromSession(HTTPCoroSession* session) const {
      return session_ == session;
    }

   private:
    explicit RequestReservation(HTTPCoroSession* session) : session_(session) {
      session_->pendingSendStreams_++;
    }
    void consume() {
      if (session_) {
        session_->pendingSendStreams_--;
        session_->writeEvent_.signal();
        session_ = nullptr;
      }
    }
    friend class HTTPCoroSession;
    HTTPCoroSession* session_{nullptr};
  };

  folly::Try<RequestReservation> reserveRequest();

  virtual folly::coro::Task<HTTPSourceHolder> sendRequest(
      HTTPSourceHolder requestSource, RequestReservation reservation);

  /**
   * ::sendRequest variant used if HTTPMessage is synchronously available:
   * !bodySource.readable() => headerEvent w/ eom
   * bodySource.readable() => only ::readBodyEvent will be invoked
   */
  virtual folly::Expected<HTTPSourceHolder, HTTPError> sendRequest(
      RequestReservation reservation,
      const HTTPMessage& headers,
      HTTPSourceHolder bodySource) noexcept;

  /**
   * verifies a WebTransport request is valid; yields an
   * HTTPError(INTERNAL_ERROR) if invalid
   *
   * returns an asynchronous result containing the server's response and a
   * WebTransport handle
   */
  struct WtReqResult {
    std::unique_ptr<HTTPMessage> resp;
    std::shared_ptr<WebTransport> wt;
  };
  virtual folly::coro::Task<WtReqResult> sendWtReq(
      RequestReservation reservation, const HTTPMessage& msg) noexcept;

  void describe(std::ostream& os) const override;

  void addLifecycleObserver(LifecycleObserver* cb) override {
    // add to the beginning of the list, so that a currently iterating
    // ::deliverLifecycleEvent does not call into this observer
    lifecycleObservers_.push_front(CHECK_NOTNULL(cb));
    cb->onAttached(*this);
  }

  void removeLifecycleObserver(LifecycleObserver* cb) override {
    auto it =
        std::find(lifecycleObservers_.begin(), lifecycleObservers_.end(), cb);
    if (it != lifecycleObservers_.end()) {
      lifecycleObservers_.erase(it);
    }
  }

  template <typename Filter, typename... Args>
  void addCodecFilter(Args&&... args) {
    codec_.add<Filter>(std::forward<Args>(args)...);
  }

  void setSetting(SettingsId setting, uint32_t value);

  virtual void sendPing() = 0;

  virtual void setConnectionFlowControl(uint32_t connFlowControl) = 0;

  void setMaxConcurrentOutgoingStreams(uint32_t maxConcurrentOutgoingStreams) {
    if (maxConcurrentOutgoingStreams == 0) {
      XLOG(ERR) << "Cannot set maxConcurrentOutgoingStreams_ to 0";
      return;
    }
    if (codec_->supportsParallelRequests()) {
      maxConcurrentOutgoingStreamsConfig_ = maxConcurrentOutgoingStreams;
    }
  }

  uint32_t numOutgoingStreams() const override {
    auto nStreams = streams_.size();
    XCHECK_GE(nStreams, numPushStreams_);
    XCHECK_LT(nStreams, std::numeric_limits<uint32_t>::max());
    return isUpstream()
               ? uint32_t(nStreams) - numPushStreams_ + pendingSendStreams_
               : numPushStreams_;
  }
  uint32_t numIncomingStreams() const override {
    auto nStreams = streams_.size();
    XCHECK_GE(nStreams, numPushStreams_);
    XCHECK_LT(nStreams, std::numeric_limits<uint32_t>::max());
    return isDownstream() ? uint32_t(nStreams) - numPushStreams_
                          : numPushStreams_;
  }

  uint64_t numStreams() const {
    return numIncomingStreams() + numOutgoingStreams();
  }

  bool supportsMoreTransactions() const {
    return numTransactionsAvailable() > 0;
  }
  virtual uint32_t numTransactionsAvailable() const = 0;

  // If not already draining, sends a GOAWAY, and may trigger other actions.
  void initiateDrain() override;
  void closeWhenIdle() override;
  void dropConnection(const std::string& errorMsg = "") override;

  // How long to wait in read() before initiating a graceful close.
  virtual void setConnectionReadTimeout(std::chrono::milliseconds timeout) {
    connReadTimeout_ = timeout;
  }

  // How long streams will wait for input before a timeout error (408/504)
  void setStreamReadTimeout(std::chrono::milliseconds timeout) {
    streamReadTimeout_ = timeout;
  }

  // How long to wait in coro::Transport::write before erroring the connection.
  // Also how long to wait for a "write event" when blocked on connection
  // or stream flow control.
  // TODO: Because not all write events indicate progress against blocked flow
  // control, this timer may not function as designed?
  void setWriteTimeout(std::chrono::milliseconds timeout) {
    writeTimeout_ = timeout;
  }

  /**
   * Default maximum number of cumulative bytes that can be buffered by the
   * transactions in this session before applying backpressure. Can be
   * overridden with setReadBufferLimit().
   */
  constexpr static size_t kIngressBufferLimit = 65535;
  void setReadBufferLimit(uint64_t readBufferLimit) {
    if (readBufferLimit != 0) {
      readBufferLimit_ = readBufferLimit;
    }
  }

  // whether we've exceeded the ingress buffer threshold.
  bool ingressLimitExceeded(const StreamState& stream) const;
  // whether we were previously in excess of ingress limit but no longer are.
  bool shouldResumeIngress(const StreamState& stream, uint64_t delta) const;

  // Set the amount of time to wait for a TX or ACK ByteEvent to fire once
  // scheduled (bytes accepted by the transport).
  // Currently only supported for TCP.
  virtual void setByteEventTimeout(std::chrono::milliseconds) {
  }

  const folly::SocketAddress& getLocalAddress() const override {
    return localAddr_;
  }
  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return peerAddr_;
  }

  const wangle::TransportInfo& getSetupTransportInfo() const override {
    return setupTransportInfo_;
  }

  bool getCurrentTransportInfo(wangle::TransportInfo* tinfo,
                               bool includeSetupFields = false) const override;

  std::chrono::steady_clock::time_point getStartTime() const {
    return setupTransportInfo_.acceptTime;
  }

  bool isDownstream() const override {
    return direction_ == TransportDirection::DOWNSTREAM;
  }

  bool isUpstream() const override {
    return direction_ == TransportDirection::UPSTREAM;
  }

  virtual void detachEvb();
  virtual void attachEvb(folly::EventBase*);

  virtual bool isDetachable() const;

  // default impl is a no-op, implemented for HTTPUniplexTransportSession
  virtual void setRateLimitParams(RateLimiter::Type,
                                  uint32_t,
                                  std::chrono::milliseconds) {
  }

 private:
  class GoawayTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit GoawayTimeout(HTTPCoroSession& session) : session_(session) {
    }
    void timeoutExpired() noexcept override {
      session_.goawayTimeoutExpired();
    }

    // No-op
    void callbackCanceled() noexcept override {
    }

   private:
    HTTPCoroSession& session_;
  };

 protected:
  using StreamMap =
      folly::F14FastMap<HTTPCodec::StreamID, std::unique_ptr<StreamState>>;

  HTTPCoroSession(folly::EventBase* eventBase,
                  folly::SocketAddress localAddr,
                  folly::SocketAddress peerAddr,
                  std::unique_ptr<HTTPCodec> codec,
                  wangle::TransportInfo tinfo,
                  std::shared_ptr<HTTPHandler> handler = nullptr);

  uint64_t sessionID_{folly::Random::rand64()};
  folly::Executor::KeepAlive<folly::EventBase> eventBase_;
  TransportDirection direction_;
  folly::SocketAddress localAddr_;
  folly::SocketAddress peerAddr_;
  HTTPCodecFilterChain codec_;
  std::shared_ptr<HTTPHandler> handler_;
  HTTPSessionStats* sessionStats_{nullptr};
  wangle::TransportInfo setupTransportInfo_;
  StreamMap streams_;
  // Connection flow control
  Window sendWindow_{http2::kInitialWindow};
  WindowContainer recvWindow_;
  uint64_t sessionBytesScheduled_{0};
  folly::IOBufQueue writeBuf_{folly::IOBufQueue::cacheChainLength()};
  detail::DetachableCancellableBaton writeEvent_;
  GoawayTimeout goawayTimeout_{*this};
  std::chrono::milliseconds connReadTimeout_{std::chrono::seconds(5)};
  std::chrono::milliseconds streamReadTimeout_{std::chrono::seconds(5)};
  std::chrono::milliseconds writeTimeout_{std::chrono::seconds(5)};
  uint64_t readBufferLimit_{kIngressBufferLimit};
  std::list<LifecycleObserver*> lifecycleObservers_{};
  uint32_t maxConcurrentOutgoingStreamsConfig_{100};
  uint32_t numPushStreams_{0};
  uint32_t pendingSendStreams_{0};
  uint32_t nextStreamSequenceNumber_{0};
  quic::HTTPPriorityQueue writableStreams_;
  detail::DetachableExecutor readExec_{eventBase_.get()};
  detail::DetachableExecutor writeExec_{eventBase_.get()};

  void sendPreface();

  StreamState& createNewStream(HTTPCodec::StreamID id,
                               bool fromSendRequest = false);

  StreamState* findStream(HTTPCodec::StreamID id);

  void insertWithPriority(const StreamState&);

  folly::coro::Task<HTTPSourceHolder> handleRequest(StreamState& stream);

  folly::coro::Task<void> readResponse(
      StreamState& stream,
      folly::coro::Task<HTTPSourceHolder> responseSourceTask);

  enum class ResponseState { HEADERS, BODY, DONE };
  ResponseState processResponseHeaderEvent(
      StreamState& stream, folly::Try<HTTPHeaderEvent> headerEvent);

  folly::coro::Task<void> transferRequestBody(StreamState& stream,
                                              HTTPSourceHolder requestSource);

  folly::coro::Task<void> transferBody(StreamState& stream,
                                       std::function<void()>);

  virtual void registerByteEvents(
      HTTPCodec::StreamID id,
      folly::Optional<uint64_t> streamByteOffset,
      folly::Optional<HTTPByteEvent::FieldSectionInfo> fsInfo,
      uint64_t bodyOffset,
      std::vector<HTTPByteEventRegistration>&& registrations,
      bool eom) = 0;

  void onResetStream(HTTPCodec::StreamID id, HTTPErrorCode error);
  virtual void deliverAbort(StreamState& stream,
                            HTTPErrorCode error,
                            std::string_view details);

  void egressFinished(StreamState& stream);
  void egressResetStream(HTTPCodec::StreamID id,
                         StreamState* stream,
                         HTTPErrorCode error,
                         bool fromSource = false,
                         bool bidirectionalReset = true);
  virtual void handleDeferredStopSending(HTTPCodec::StreamID id) = 0;

  // error is delivered to any queued byte events that get cancelled
  void resetStreamState(StreamState& stream, const HTTPError& err);

  void decrementPushStreamCount(const StreamState& stream,
                                bool eomMarkedEgressComplete = false);

  bool checkForDetach(StreamState& stream);

  uint32_t getStreamSendFlowControlWindow() {
    return getStreamFlowControlWindow(codec_->getIngressSettings());
  }
  uint32_t getStreamRecvFlowControlWindow() {
    return getStreamFlowControlWindow(codec_->getEgressSettings());
  }
  uint32_t getStreamFlowControlWindow(const HTTPSettings* settings) {
    if (codec_->supportsStreamFlowControl()) {
      XCHECK(settings) << "H2 has settings and stream flow control";
      auto setting = settings->getSetting(SettingsId::INITIAL_WINDOW_SIZE);
      return setting ? uint32_t(setting->value) : http2::kInitialWindow;
    }
    return std::numeric_limits<int32_t>::max();
  }
  void bytesProcessed(HTTPCodec::StreamID id,
                      size_t delta,
                      size_t toAck) override;
  void sourceComplete(HTTPCodec::StreamID id,
                      folly::Optional<HTTPError> error) override;
  void onEgressBytesBuffered(int64_t bytes) noexcept override {
    if (sessionStats_) {
      sessionStats_->recordPendingBufferedWriteBytes(bytes);
    }
  }

  // HTTPCodec callbacks
  void onMessageBegin(HTTPCodec::StreamID streamID,
                      HTTPMessage* /*msg*/) override;
  // onPushMessageBegin
  void onExMessageBegin(HTTPCodec::StreamID /*streamID*/,
                        HTTPCodec::StreamID /*controlStream*/,
                        bool /*unidirectional*/,
                        HTTPMessage* /*msg*/) override {
  }
  void onHeadersComplete(HTTPCodec::StreamID streamID,
                         std::unique_ptr<HTTPMessage> msg
                         /* bool eom!*/) override;

  void onBody(HTTPCodec::StreamID streamID,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t padding
              /* bool eom!*/) override;

  virtual void handleIngressLimitExceeded(HTTPCodec::StreamID) = 0;

  void onChunkHeader(HTTPCodec::StreamID /*stream*/,
                     size_t /*length*/) override {
  }
  void onChunkComplete(HTTPCodec::StreamID /*stream*/) override {
  }
  void onTrailersComplete(HTTPCodec::StreamID streamID,
                          std::unique_ptr<HTTPHeaders> trailers) override;
  void onMessageComplete(HTTPCodec::StreamID streamID,
                         bool /*upgrade*/) override;
  void onError(HTTPCodec::StreamID /*streamID*/,
               const HTTPException& /*error*/,
               bool /*newTxn*/) override;
  void onAbort(HTTPCodec::StreamID streamID, ErrorCode code) override;

  void onGoaway(uint64_t /*lastGoodStreamID*/,
                ErrorCode /*code*/,
                std::unique_ptr<folly::IOBuf> /*debugData = nullptr*/) override;

  // Default/Implemented in subclass:
  //  onPingRequest/onPingReply
  //  onWindowUpdate
  //  onSettings
  //  onSettingsAck

  void onPriority(HTTPCodec::StreamID /*stream*/,
                  const HTTPPriority& /*pri*/) override {
  }

  void onCertificateRequest(
      uint16_t /*requestId*/,
      std::unique_ptr<folly::IOBuf> /*authRequest*/) override {
  }
  void onCertificate(uint16_t /*certId*/,
                     std::unique_ptr<folly::IOBuf> /*authenticator*/) override {
  }

  void timeoutExpired() noexcept override {
    /* ignore, we have our own timeouts? */
  }
  bool isBusy() const override {
    return !streams_.empty() || codec_->isBusy();
  }
  std::chrono::milliseconds getIdleTime() const override {
    // TODO: implement
    return std::chrono::milliseconds(0);
  }
  void notifyPendingShutdown() override {
    /*
     * Upstream HTTPCoroSessions should not be drained by a
     * wangle::ConnectionManager. Instead, drain them manually via
     * `::initiateDrain()` or held by a containing HTTPCoroSessionPool and drain
     * that.
     */
    if (isDownstream()) {
      initiateDrain();
    }
  }
  void dumpConnectionState(uint8_t /*loglevel*/) override {
  }

  template <typename T, typename... Args>
  void deliverLifecycleEvent(T callbackFn, Args&&... args) {
    auto it = lifecycleObservers_.begin();
    while (it != lifecycleObservers_.end()) {
      auto* observer = *it++;
      (*observer.*callbackFn)(std::forward<Args>(args)...);
    }
  }

  void onSetMaxInitiatedStreams(bool didSupport);

  void scheduleGoawayTimeout() {
    // connReadTimeout_ is not the right value here, HTTPSession gets this
    // value from its "controller"
    eventBase_->timer().scheduleTimeout(&goawayTimeout_, connReadTimeout_);
  }

  void goawayTimeoutExpired();

  folly::coro::Task<void> waitForAllStreams();

  void drainStarted();

  bool isDraining() const {
    return maxConcurrentOutgoingStreamsConfig_ == 0;
  }

  void connectionError(
      HTTPErrorCode httpError,
      std::string msg,
      folly::Optional<HTTPErrorCode> streamError = folly::none);

  void resetOpenStreams(HTTPErrorCode error, std::string_view details);

  bool handleDownstreamHTTPParseError(HTTPCodec::StreamID streamID,
                                      const HTTPException& error);

  HTTPHeaderSize addPushPromiseToWriteBuf(StreamState& stream,
                                          HTTPBodyEvent& bodyEvent);

  // TODO: should be in HTTPCodec
  virtual HTTPCodec::StreamID getSessionStreamID() const = 0;
  virtual void applyEgressSettings() {
  }
  virtual void handlePipeliningOnDetach() {
  }
  virtual bool getCurrentTransportInfoImpl(
      wangle::TransportInfo* /*tinfo*/) const = 0;
  virtual void handleConnectionError(HTTPErrorCode error, std::string msg) = 0;
  virtual void removeWritableStream(HTTPCodec::StreamID id) {
    auto identifier = quic::PriorityQueue::Identifier::fromStreamID(id);
    writableStreams_.erase(identifier);
  }
  virtual void setupStreamWriteBuf(StreamState& stream,
                                   folly::IOBufQueue& sessWriteBuf) = 0;

  virtual void notifyHeaderWrite(StreamState& stream, bool eom) = 0;
  void notifyBodyWrite(StreamState& stream);
  virtual StreamState* createReqStream() = 0;
  virtual bool streamRefusedByGoaway(StreamState& stream,
                                     HTTPCodec::StreamID lastGoodStreamID) = 0;
  virtual void generateResetStream(HTTPCodec::StreamID id,
                                   HTTPErrorCode error,
                                   bool fromSource,
                                   bool bidirectionalReset) = 0;
  virtual void eraseStream(HTTPCodec::StreamID id);
  virtual void streamHeadersComplete(StreamState& /*stream*/) {
  }
  virtual bool checkAndHandlePushPromiseComplete(
      StreamState& stream, std::unique_ptr<HTTPMessage>& msg) = 0;
  virtual folly::Expected<std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID>,
                          ErrorCode>
  createEgressPushStream() = 0;
  virtual bool sendFlowControlUpdate(HTTPCodec::StreamID /*id*/,
                                     size_t /*delta*/) {
    return false;
  }
  virtual bool isConnectionFlowControlBlocked() {
    return sendWindow_.getSize() == 0;
  }
  virtual bool isStreamFlowControlBlocked(StreamState& stream);
  virtual void interruptReadLoop() {
  }
  void handleWriteEventTimeout();

  void transactionAttached() noexcept;
  void transactionDetached() noexcept;

  // NOTE: this will never invoke ::readHeaderEvent on the bodySource
  folly::Expected<HTTPSourceHolder, HTTPError> sendRequestImpl(
      const HTTPMessage& headers,
      folly::Function<void(HTTPHeaderSize) noexcept>&& egressHeadersFn,
      std::vector<HTTPByteEventRegistration>&& byteEventRegistrations,
      HTTPSourceHolder bodySource) noexcept;
};

class HTTPUniplexTransportSession : public HTTPCoroSession {
 public:
  HTTPUniplexTransportSession(
      std::unique_ptr<folly::coro::TransportIf> coroTransport,
      std::unique_ptr<HTTPCodec> codec,
      wangle::TransportInfo tinfo,
      std::shared_ptr<HTTPHandler> handler = nullptr)
      : HTTPCoroSession(coroTransport->getEventBase(),
                        coroTransport->getLocalAddress(),
                        coroTransport->getPeerAddress(),
                        std::move(codec),
                        std::move(tinfo),
                        std::move(handler)),
        coroTransport_(std::move(coroTransport)) {
    flowControlBaton_.signal();
    start();
  }

  ~HTTPUniplexTransportSession() override {
    coroTransport_->close();
  }

  folly::coro::TaskWithExecutor<void> run() override;

  void sendPing() override;

  void setConnectionFlowControl(uint32_t connFlowControl) override;

  folly::coro::Task<WtReqResult> sendWtReq(
      RequestReservation reservation, const HTTPMessage& msg) noexcept final;

 private:
  folly::coro::Task<void> runImpl();
  void handleIngressLimitExceeded(HTTPCodec::StreamID streamID) override;

  void bytesProcessed(HTTPCodec::StreamID id,
                      size_t delta,
                      size_t toAck) override;

  void detachEvb() override;
  void attachEvb(folly::EventBase* evb) override;
  bool isDetachable() const override;

  std::unique_ptr<folly::coro::TransportIf> coroTransport_;
  folly::coro::Baton writesFinished_;
  detail::CancellableBaton antiPipelineBaton_;
  detail::CancellableBaton flowControlBaton_;
  uint32_t maxConcurrentOutgoingStreamsRemote_{100};
  bool readsClosed_{false};
  bool resetAfterDrainingWrites_{false};
  std::list<PendingByteEvent> transportWriteEvents_;
  std::list<PendingByteEvent> kernelWriteEvents_;
  RateLimitFilter* rateLimitFilter_{nullptr};
  folly::CancellationSource readCancellationSource_;
  AsyncSocketByteEventObserver byteEventObserver_;

 public:
  void start();

  void setRateLimitParams(RateLimiter::Type type,
                          uint32_t maxEventsPerInterval,
                          std::chrono::milliseconds intervalDuration) override;

  folly::coro::Task<void> readLoop() noexcept;
  folly::coro::Task<void> writeLoop() noexcept;

  size_t addStreamBodyDataToWriteBuf(uint32_t max);

  // HTTPCoroSession overrides
  bool sendFlowControlUpdate(HTTPCodec::StreamID /*id*/,
                             size_t /*delta*/) override;

  void handleConnectionError(HTTPErrorCode error, std::string msg) override;
  void setupStreamWriteBuf(StreamState& stream,
                           folly::IOBufQueue& sessWriteBuf) override;
  bool getCurrentTransportInfoImpl(
      wangle::TransportInfo* /*tinfo*/) const override;

  FOLLY_DEPRECATED("Unsafe")
  const folly::AsyncTransport* getAsyncTransport() const override {
    return coroTransport_->getTransport();
  }
  int getAsyncTransportFD() const override;
  uint16_t getDatagramSizeLimit() const override {
    return 0;
  }
  const folly::AsyncTransportCertificate* getPeerCertificate() const override {
    return coroTransport_ ? coroTransport_->getPeerCertificate() : nullptr;
  }

  void notifyHeaderWrite(StreamState& stream, bool eom) override;
  StreamState* createReqStream() override;
  bool streamRefusedByGoaway(StreamState& stream,
                             HTTPCodec::StreamID lastGoodStreamID) override;
  void generateResetStream(HTTPCodec::StreamID id,
                           HTTPErrorCode error,
                           bool fromSource,
                           bool bidirectionalReset = true) override;
  void handleDeferredStopSending(HTTPCodec::StreamID id) override;
  bool checkAndHandlePushPromiseComplete(
      StreamState& stream, std::unique_ptr<HTTPMessage>& msg) override;
  folly::Expected<std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID>,
                  ErrorCode>
  createEgressPushStream() override;

  // H1/2 specific HTTPCodec::Callback overrides
  void onPushMessageBegin(HTTPCodec::StreamID streamID,
                          HTTPCodec::StreamID assocStreamID,
                          HTTPMessage* promise) override;
  void onPingRequest(uint64_t data) override {
    // TODO: try inserting at the beginning of writeBuf_ ?
    // TODO: byte events for timing ping latency?
    // TODO: tracking e2e RTT measurements
    deliverLifecycleEvent(&LifecycleObserver::onPingReplySent, 0);
    codec_->generatePingReply(writeBuf_, data);
    writeEvent_.signal();
  }
  void onPingReply(uint64_t /*data*/) override {
    deliverLifecycleEvent(&LifecycleObserver::onPingReplyReceived);
  }
  void onWindowUpdate(HTTPCodec::StreamID streamID, uint32_t amount) override;
  void onSettings(const SettingsList& settings) override;
  void onSettingsAck() override;
  uint32_t getMaxConcurrentOutgoingStreams() const {
    return std::min(maxConcurrentOutgoingStreamsConfig_,
                    maxConcurrentOutgoingStreamsRemote_);
  }
  uint32_t numTransactionsAvailable() const override {
    auto nOutgoing = numOutgoingStreams();
    auto maxConcurrent = getMaxConcurrentOutgoingStreams();
    return (nOutgoing < maxConcurrent) ? maxConcurrent - nOutgoing : 0;
  }

  void cleanupAfterWriteError(const std::string& msg);

  bool shouldContinueReadLooping() const;
  bool shouldContinueWriteLooping() const;
  HTTPCodec::StreamID getSessionStreamID() const override {
    return 0;
  }
  void deliverAbort(StreamState& stream,
                    HTTPErrorCode error,
                    std::string_view details) override;
  void setByteEventTimeout(std::chrono::milliseconds timeout) override {
    byteEventObserver_.setByteEventTimeout(timeout);
  }
  void registerByteEvents(
      HTTPCodec::StreamID id,
      folly::Optional<uint64_t> streamByteOffset,
      folly::Optional<HTTPByteEvent::FieldSectionInfo> fsInfo,
      uint64_t bodyOffset,
      std::vector<HTTPByteEventRegistration>&& registrations,
      bool eom) override;

  void handlePipeliningOnDetach() override;
  void interruptReadLoop() override {
    if (!shouldContinueReadLooping()) {
      readCancellationSource_.requestCancellation();
    }
  }
  void maybeEnableByteEvents();
};

constexpr uint8_t kMaxDatagramHeaderSize = 16;
// Maximum number of datagrams to buffer per stream
constexpr uint8_t kDefaultMaxBufferedDatagrams = 5;
// Maximum number of streams with datagrams buffered
constexpr uint8_t kMaxStreamsWithBufferedDatagrams = 10;

class HTTPQuicCoroSession
    : public HTTPCoroSession
    , public quic::QuicSocket::ConnectionCallback
    , public hq::HQUnidirectionalCodec::Callback
    , public quic::QuicSocket::PingCallback
    , public quic::QuicSocket::WriteCallback
    , public quic::QuicSocket::DatagramCallback
    , public HQUniStreamDispatcher::Callback {
 public:
  HTTPQuicCoroSession(std::shared_ptr<quic::QuicSocket> sock,
                      std::unique_ptr<hq::HQMultiCodec> codec,
                      wangle::TransportInfo tinfo,
                      std::shared_ptr<HTTPHandler> handler = nullptr);

  ~HTTPQuicCoroSession() override;

  folly::coro::TaskWithExecutor<void> run() override;

  void sendPing() override;

  void setConnectionFlowControl(uint32_t connFlowControl) override;

  size_t sendPriority(quic::StreamId id, HTTPPriority pri);
  size_t sendPushPriority(uint64_t pushId, HTTPPriority pri);

  using HTTPCoroSession::onError;
  folly::EventBase* getEventBase() const override {
    return HTTPCoroSession::getEventBase();
  }

 private:
  folly::coro::Task<void> runImpl();
  std::chrono::milliseconds getDispatchTimeout() const override {
    return streamReadTimeout_;
  }

  void rejectStream(quic::StreamId id) override;

  folly::Optional<hq::UnidirectionalStreamType> parseUniStreamPreface(
      uint64_t preface) override {
    hq::UnidirectionalTypeF parse = [](hq::UnidirectionalStreamType type)
        -> folly::Optional<hq::UnidirectionalStreamType> { return type; };
    return hq::withType(preface, parse);
  }

  void dispatchControlStream(quic::StreamId id,
                             hq::UnidirectionalStreamType streamType,
                             size_t toConsume) override;

  void dispatchPushStream(quic::StreamId id,
                          hq::PushId pushId,
                          size_t toConsume) override;

  void dispatchUniWTStream(quic::StreamId streamId,
                           quic::StreamId /*sessionId*/,
                           size_t /*toConsume*/) override {
    // TODO: implement WebTransport for proxygen::coro
    quicSocket_->stopSending(
        streamId,
        quic::ApplicationErrorCode(HTTPErrorCode::STREAM_CREATION_ERROR));
    quicSocket_->setPeekCallback(streamId, nullptr);
  }

  std::shared_ptr<quic::QuicSocket> quicSocket_;
  hq::HQMultiCodec* multiCodec_{nullptr};
  hq::QPACKEncoderCodec qpackEncoderCodec_;
  hq::QPACKDecoderCodec qpackDecoderCodec_;
  quic::StreamId controlStreamID_{quic::kInvalidStreamId};
  quic::StreamId qpackEncoderStreamID_{quic::kInvalidStreamId};
  quic::StreamId qpackDecoderStreamID_{quic::kInvalidStreamId};
  HQUniStreamDispatcher uniStreamDispatcher_;

  void onError(HTTPCodec::StreamID streamID,
               const HTTPException& error,
               bool newTxn) override {
    HTTPCoroSession::onError(streamID, error, newTxn);
  }

  // QuicSocket::ConnectionCallback overrides
  void onNewBidirectionalStream(quic::StreamId id) noexcept override;
  void onNewUnidirectionalStream(quic::StreamId id) noexcept override;
  void onStopSending(quic::StreamId id,
                     quic::ApplicationErrorCode error) noexcept override;
  void onConnectionEnd() noexcept override;
  void onConnectionError(quic::QuicError error) noexcept override;
  void onBidirectionalStreamsAvailable(
      uint64_t numStreamsAvailable) noexcept override;

  // quic::QuicSocket::PingCallback
  void pingAcknowledged() noexcept override {
  }
  void pingTimeout() noexcept override {
  }
  void onPing() noexcept override {
    resetIdleTimeout();
  }

  // QuicSocket::DatagramCallback overrides
  void onDatagramsAvailable() noexcept override;

  // HTTPCoroSession overrides
  void handleConnectionError(HTTPErrorCode error, std::string msg) override;

  void setupStreamWriteBuf(StreamState& stream,
                           folly::IOBufQueue& sessWriteBuf) override;
  void applyEgressSettings() override;
  bool getCurrentTransportInfoImpl(
      wangle::TransportInfo* /*tinfo*/) const override;
  quic::QuicSocket* getQUICTransport() const override {
    return quicSocket_.get();
  }
  uint16_t getDatagramSizeLimit() const override;
  const folly::AsyncTransportCertificate* getPeerCertificate() const override {
    return quicSocket_->getPeerCertificate().get();
  }

  void notifyHeaderWrite(StreamState& stream, bool eom) override;
  StreamState* createReqStream() override;
  bool streamRefusedByGoaway(StreamState& stream,
                             HTTPCodec::StreamID lastGoodStreamID) override;
  void generateResetStream(HTTPCodec::StreamID id,
                           HTTPErrorCode error,
                           bool fromSource,
                           bool bidirectionalReset = true) override;
  void handleDeferredStopSending(HTTPCodec::StreamID id) override;
  void eraseStream(HTTPCodec::StreamID id) override;
  uint32_t numTransactionsAvailable() const override {
    if (!quicSocket_->good()) {
      return 0;
    }
    uint32_t nStreams = numOutgoingStreams(); // includes pendingSendStreams_
    if (maxConcurrentOutgoingStreamsConfig_ <= nStreams) {
      return 0; // at config limit
    }
    uint32_t configAvailable = maxConcurrentOutgoingStreamsConfig_ - nStreams;
    uint64_t transportAvailable =
        isUpstream() ? quicSocket_->getNumOpenableBidirectionalStreams()
                     : quicSocket_->getNumOpenableUnidirectionalStreams();
    XCHECK(pendingSendStreams_ == 0 || isUpstream());
    if (transportAvailable <= pendingSendStreams_) {
      return 0; // at transport limit
    }
    transportAvailable -= pendingSendStreams_;
    return std::min(configAvailable,
                    clamped_downcast<uint32_t>(transportAvailable));
  }
  HTTPCodec::StreamID getSessionStreamID() const override {
    return hq::kSessionStreamId;
  }
  void deliverAbort(StreamState& stream,
                    HTTPErrorCode error,
                    std::string_view details) override;
  void streamHeadersComplete(StreamState& /*stream*/) override;
  void setConnectionReadTimeout(std::chrono::milliseconds timeout) override {
    HTTPCoroSession::setConnectionReadTimeout(timeout);
    idle_.signal(); // this will set to the new timeout
  }
  void registerByteEvents(
      HTTPCodec::StreamID id,
      folly::Optional<uint64_t> streamByteOffset,
      folly::Optional<HTTPByteEvent::FieldSectionInfo> fsInfo,
      uint64_t bodyOffset,
      std::vector<HTTPByteEventRegistration>&& registrations,
      bool eom) override;
  bool checkAndHandlePushPromiseComplete(
      StreamState& stream, std::unique_ptr<HTTPMessage>& msg) override;
  folly::Expected<std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID>,
                  ErrorCode>
  createEgressPushStream() override;

  void onPushMessageBegin(HTTPCodec::StreamID streamID,
                          HTTPCodec::StreamID assocStreamID,
                          HTTPMessage* promise) override;

  void onSettings(const SettingsList& settings) override;

  bool isConnectionFlowControlBlocked() override {
    auto connFC = quicSocket_->getConnectionFlowControl();
    return connFC && connFC->sendWindowAvailable == 0;
  }

  bool isStreamFlowControlBlocked(StreamState& stream) override;

  /**
   * bytesProcessed and onBody callbacks mirror each other in an opposite way.
   * When bytesProcessed and onBody callbacks are invoked, we need to decrement
   * or increment the stream's pendingReadSize.
   *
   * bytesProcessed may resume reading from the stream if the buffer size
   * transitioned from being greater than or equal to kDefaultReadSize, but is
   * now strictly less than kDefaultReadSize.
   *
   * onBody may pause reading from the stream if the buffer size now exceeds the
   * kDefaultReadSize threshold.
   */
  void bytesProcessed(HTTPCodec::StreamID id,
                      size_t delta,
                      size_t toAck) override;

  void handleIngressLimitExceeded(HTTPCodec::StreamID streamID) override;

  void detachEvb() override;
  void attachEvb(folly::EventBase* evb) override;
  bool isDetachable() const override;

  // HTTPQuicCoroSession only
  void start();
  folly::coro::Task<void> dispatchUnidirectionalStream(quic::StreamId id);
  folly::coro::Task<void> readControlStream(quic::StreamId id,
                                            hq::HQUnidirectionalCodec& codec,
                                            bool isQPACKDecoder);
  folly::coro::Task<void> readLoop(quic::StreamId id) noexcept;
  bool shouldContinueWriteLooping() const;
  class StreamRCB;
  folly::coro::Task<void> readLoopImpl(std::shared_ptr<StreamRCB> readCallback,
                                       quic::StreamId id) noexcept;
  folly::coro::Task<void> readLoop() noexcept;
  folly::coro::Task<void> writeLoop() noexcept;
  bool hasControlWrite() const;
  void writeControlStream(quic::StreamId id, folly::IOBufQueue& writeBuf);
  bool createControlStream(hq::UnidirectionalStreamType streamType,
                           folly::IOBufQueue& writeBuf,
                           quic::StreamId& id);
  void registerControlDeliveryCallback(quic::StreamId id);
  bool handleWrite(HTTPCodec::StreamID id,
                   folly::IOBufQueue& writeBuf,
                   bool eom);
  void dispatchPushStream(quic::StreamId id, uint64_t pushID);
  bool isDatagramEnabled() const {
    SettingsId setting = *hq::hqToHttpSettingsId(hq::SettingId::H3_DATAGRAM);
    return multiCodec_->getEgressSettings()->getSetting(setting) &&
           (!multiCodec_->receivedSettings() ||
            codec_->getIngressSettings()->getSetting(setting));
  }
  bool sendDatagram(HTTPCodec::StreamID id,
                    std::unique_ptr<folly::IOBuf> datagram);

  void resetIdleTimeout();

  void onPriority(quic::StreamId, const HTTPPriority&) override;
  void onPushPriority(quic::StreamId, const HTTPPriority&) override {
  }

  void onHeadersComplete(HTTPCodec::StreamID streamID,
                         std::unique_ptr<HTTPMessage> msg
                         /* bool eom!*/) override;

  bool isStreamIngressLimitExceeded(HTTPCodec::StreamID streamID) {
    auto stream = findStream(streamID);
    return stream && ingressLimitExceeded(*stream);
  }

  void onStreamWriteReady(quic::StreamId id,
                          uint64_t /*maxSend*/) noexcept override {
    if (auto* stream = findStream(id)) {
      notifyBodyWrite(*stream);
    }
  }
  void generateStopSending(HTTPCodec::StreamID id,
                           HTTPErrorCode error,
                           bool fromSource);

  class QuicReadCallback : public quic::QuicSocket::ReadCallback {
   public:
    detail::CancellableBaton& getBaton() {
      return baton_;
    }

   protected:
    explicit QuicReadCallback(HTTPQuicCoroSession& session)
        : session_(session) {
    }

    HTTPQuicCoroSession& session_;
    detail::CancellableBaton baton_;
    folly::IOBufQueue input_{folly::IOBufQueue::cacheChainLength()};
    bool readEOF_{false};
  };

  class StreamRCB : public QuicReadCallback {
   public:
    explicit StreamRCB(HTTPQuicCoroSession& session)
        : QuicReadCallback(session) {
    }
    void readAvailable(quic::StreamId id) noexcept override;
    void processRead(quic::StreamId id);
    void resumeRead(quic::StreamId id);
    void readError(quic::StreamId id, quic::QuicError error) noexcept override;

   private:
    bool inProcessRead_{false};
    bool isReadPaused_{false};
  };

  class DeliveryCallback
      : public quic::ByteEventCallback
      , protected Refcount {
   public:
    static constexpr uint8_t kDeliveryCallbackRefcount = 0;
    static constexpr std::chrono::milliseconds kDeliveryCallbackTimeout =
        std::chrono::seconds(5);

    DeliveryCallback() : Refcount(kDeliveryCallbackRefcount) {
    }

    void onByteEventRegistered(quic::ByteEvent) override {
      incRef();
    }

    void onByteEvent(quic::ByteEvent byteEvent) override {
      XLOG(DBG4) << "onDeliveryAck for id=" << byteEvent.id;
      decRef();
    }

    void onByteEventCanceled(quic::ByteEvent byteEvent) override {
      XLOG(DBG4) << "onCanceled for id=" << byteEvent.id;
      decRef();
    }

    folly::coro::Task<TimedBaton::Status> zeroRefs(folly::EventBase* evb) {
      return baton_.timedWait(evb, kDeliveryCallbackTimeout);
    }
  };

  // Maximum number of priority updates received when stream is not available
  constexpr static uint8_t kMaxBufferedPriorityUpdates = 10;

  folly::coro::AsyncScope backgroundScope_;
  detail::DetachableCancellableBaton idle_;
  quic::Optional<quic::QuicError> connectionError_;
  DeliveryCallback deliveryCallback_;
  Refcount byteEventRefcount_{0};
  // PushID -> StreamState
  folly::F14FastMap<uint64_t, std::unique_ptr<StreamState>>
      pushStreamsAwaitingStreamID_;
  // PushID -> StreamID
  folly::F14FastMap<uint64_t, quic::StreamId> pushStreamsAwaitingPromises_;
  // Buffer for priority updates without an active stream
  folly::EvictingCacheMap<quic::StreamId, HTTPPriority> priorityUpdatesBuffer_{
      kMaxBufferedPriorityUpdates};
  // Buffer for datagrams waiting for a stream to be assigned to
  folly::EvictingCacheMap<
      quic::StreamId,
      folly::small_vector<
          std::unique_ptr<folly::IOBuf>,
          kDefaultMaxBufferedDatagrams,
          folly::small_vector_policy::policy_in_situ_only<true>>>
      datagramsBuffer_{kMaxStreamsWithBufferedDatagrams};
  std::shared_ptr<QuicProtocolInfo> currentQuicProtocolInfo_{
      std::make_shared<QuicProtocolInfo>()};
};

std::ostream& operator<<(std::ostream& os, const HTTPCoroSession& session);

HTTPSource* getErrorResponse(uint16_t statusCode, const std::string& body = "");
} // namespace proxygen::coro
