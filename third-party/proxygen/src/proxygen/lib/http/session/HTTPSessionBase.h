/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncUDPSocket.h>
#include <folly/io/async/SSLContext.h>
#include <proxygen/lib/http/codec/ControlMessageRateLimitFilter.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/http/codec/RateLimitFilter.h>
#include <proxygen/lib/http/observer/HTTPSessionObserverContainer.h>
#include <proxygen/lib/http/observer/HTTPSessionObserverInterface.h>
#include <proxygen/lib/http/session/HTTPSessionActivityTracker.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/utils/Time.h>
#include <wangle/acceptor/ManagedConnection.h>
#include <wangle/acceptor/TransportInfo.h>

namespace proxygen {
class HTTPSessionController;
class HTTPSessionStats;
class HTTPTransaction;
class ByteEventTracker;

constexpr uint32_t kDefaultMaxConcurrentOutgoingStreams = 100;

class HTTPPriorityMapFactoryProvider {
 public:
  virtual ~HTTPPriorityMapFactoryProvider() = default;
  virtual HTTPCodec::StreamID sendPriority(http2::PriorityUpdate pri) = 0;
};

class HTTPSessionBase : public wangle::ManagedConnection {
 public:
  /**
   * Optional callback interface that the HTTPSessionBase
   * notifies of connection lifecycle events.
   */
  class InfoCallback {
   public:
    virtual ~InfoCallback() {
    }

    // Note: you must not start any asynchronous work from onCreate()
    virtual void onCreate(const HTTPSessionBase&) {
    }
    virtual void onTransportReady(const HTTPSessionBase&) {
    }
    virtual void onConnectionError(const HTTPSessionBase&) {
    }
    virtual void onFullHandshakeCompletion(const HTTPSessionBase&) {
    }
    virtual void onIngressError(const HTTPSessionBase&, ProxygenError) {
    }
    virtual void onIngressEOF() {
    }
    virtual void onRead(const HTTPSessionBase&, size_t /*bytesRead*/) {
    }
    /**
     * New version of the API.  Includes the stream these bytes belong to,
     * or HTTPCodec::NoStream if unknown.  bytesRead can be 0 if the stream
     * ended.
     *
     * If onRead is currently implemented with the old signature (without stream
     * ID), safest path is to keep it and change it to call onRead with the new
     * signature that includes the stream ID with folly::none as the Stream ID.
     */
    virtual void onRead(const HTTPSessionBase& sess,
                        size_t bytesRead,
                        folly::Optional<HTTPCodec::StreamID> /*stream id*/) {
      onRead(sess, bytesRead);
    }
    virtual void onWrite(const HTTPSessionBase&, size_t /*bytesWritten*/) {
    }
    virtual void onRequestBegin(const HTTPSessionBase&) {
    }
    virtual void onRequestEnd(const HTTPSessionBase&,
                              uint32_t /*maxIngressQueueSize*/) {
    }
    virtual void onActivateConnection(const HTTPSessionBase&) {
    }
    virtual void onDeactivateConnection(const HTTPSessionBase&) {
    }
    // Note: you must not start any asynchronous work from onDestroy()
    virtual void onDestroy(const HTTPSessionBase&) {
    }
    virtual void onIngressMessage(const HTTPSessionBase&, const HTTPMessage&) {
    }
    virtual void onIngressLimitExceeded(const HTTPSessionBase&) {
    }
    virtual void onIngressPaused(const HTTPSessionBase&) {
    }
    virtual void onTransactionAttached(const HTTPSessionBase&) {
    }
    virtual void onTransactionDetached(const HTTPSessionBase&) {
    }
    virtual void onPingReplySent(int64_t /*latency*/) {
    }
    virtual void onPingReplyReceived() {
    }
    virtual void onSettingsOutgoingStreamsFull(const HTTPSessionBase&) {
    }
    virtual void onSettingsOutgoingStreamsNotFull(const HTTPSessionBase&) {
    }
    virtual void onFlowControlWindowClosed(const HTTPSessionBase&) {
    }
    virtual void onEgressBuffered(const HTTPSessionBase&) {
    }
    virtual void onEgressBufferCleared(const HTTPSessionBase&) {
    }
    virtual void onSettings(const HTTPSessionBase&, const SettingsList&) {
    }
    virtual void onSettingsAck(const HTTPSessionBase&) {
    }
  };

  HTTPSessionBase(const folly::SocketAddress& localAddr,
                  const folly::SocketAddress& peerAddr,
                  HTTPSessionController* controller,
                  const wangle::TransportInfo& tinfo,
                  InfoCallback* infoCallback,
                  std::unique_ptr<HTTPCodec> codec,
                  const WheelTimerInstance& wheelTimer,
                  HTTPCodec::StreamID rootNodeId);

  virtual ~HTTPSessionBase() override;

  virtual void setHTTPSessionActivityTracker(
      std::unique_ptr<HTTPSessionActivityTracker> httpSessionActivityTracker) {
    httpSessionActivityTracker_ = std::move(httpSessionActivityTracker);
  }

  [[nodiscard]] HTTPSessionActivityTracker* getHTTPSessionActivityTracker()
      const {
    return httpSessionActivityTracker_.get();
  }
  /**
   * Set the read buffer limit to be used for all new HTTPSessionBase objects.
   */
  static void setDefaultReadBufferLimit(uint32_t limit) {
    kDefaultReadBufLimit = limit;
    VLOG(3) << "read buffer limit: " << int(limit / 1000) << "KB";
  }

  static void setMaxReadBufferSize(uint32_t bytes) {
    maxReadBufferSize_ = bytes;
  }

  /**
   * Set the maximum egress body size for any outbound body bytes per loop,
   * when there are > 1 transactions.
   */
  static void setFlowControlledBodySizeLimit(uint32_t limit) {
    egressBodySizeLimit_ = limit;
  }

  /**
   * Set the default number of egress bytes this session will buffer before
   * pausing all transactions' egress.
   */
  static void setDefaultWriteBufferLimit(uint32_t max) {
    kDefaultWriteBufLimit = max;
  }

  void setInfoCallback(InfoCallback* callback) {
    infoCallback_ = callback;
  }

  void setRateLimitParams(RateLimitFilter::Type type,
                          uint32_t maxEventsPerInterval,
                          std::chrono::milliseconds intervalDuration);

  InfoCallback* getInfoCallback() const {
    return infoCallback_;
  }

  virtual void setHeaderIndexingStrategy(
      const HeaderIndexingStrategy* strat) = 0;

  virtual void setSessionStats(HTTPSessionStats* stats);

  virtual HTTPTransaction::Transport::Type getType() const noexcept = 0;

  virtual folly::AsyncTransport* getTransport() = 0;

  virtual const folly::AsyncTransport* getTransport() const = 0;

  virtual folly::EventBase* getEventBase() const = 0;

  /**
   * Called by handleErrorDirectly (when handling parse errors) if the
   * transaction has no handler.
   */
  HTTPTransaction::Handler* getParseErrorHandler(HTTPTransaction* txn,
                                                 const HTTPException& error);

  virtual bool hasActiveTransactions() const = 0;

  /**
   * Returns true iff a new outgoing transaction can be made on this session
   */
  virtual bool supportsMoreTransactions() const {
    return (getNumOutgoingStreams() < getMaxConcurrentOutgoingStreams());
  }

  virtual uint32_t getNumStreams() const = 0;

  virtual uint32_t getNumOutgoingStreams() const = 0;

  // SimpleSessionPool
  uint32_t getHistoricalMaxOutgoingStreams() const {
    return historicalMaxOutgoingStreams_;
  }

  virtual uint32_t getNumIncomingStreams() const = 0;

  virtual uint32_t getMaxConcurrentOutgoingStreamsRemote() const = 0;

  uint32_t getMaxConcurrentOutgoingStreams() const {
    return std::min(maxConcurrentOutgoingStreamsConfig_,
                    getMaxConcurrentOutgoingStreamsRemote());
  }

  HTTPSessionController* getController() const {
    return controller_;
  }

  void setController(HTTPSessionController* controller) {
    controller_ = controller;

    // Controller controlled settings
    initCodecHeaderIndexingStrategy();
  }

  ConnectionCloseReason getConnectionCloseReason() const {
    return closeReason_;
  }

  template <typename Filter, typename... Args>
  void addCodecFilter(Args&&... args) {
    codec_.add<Filter>(std::forward<Args>(args)...);
  }

  void addRateLimitFilter(RateLimitFilter::Type type) {
    CHECK_LT(folly::to_underlying(type),
             folly::to_underlying(RateLimitFilter::Type::MAX))
        << "Received a rate limit type that exceeded the specified maximum";
    if (!rateLimitFilters_[folly::to_underlying(type)]) {
      auto filter = RateLimitFilter::createRateLimitFilter(
          type, &getEventBase()->timer(), sessionStats_);
      CHECK(filter) << "Unable to construct a rate limit filter of type "
                    << RateLimitFilter::toStr(type);
      rateLimitFilters_[folly::to_underlying(type)] = filter.get();
      codec_.addFilters(std::move(filter));
    }
  }

  virtual CodecProtocol getCodecProtocol() const {
    return codec_->getProtocol();
  }

  /**
   * Set flow control properties on the session.
   *
   * @param initialReceiveWindow      size of initial receive window
   *                                  for all ingress streams; set via
   *                                  the initial SETTINGS frame
   * @param receiveStreamWindowSize   per-stream receive window for NEW streams;
   *                                  sent via a WINDOW_UPDATE frame
   * @param receiveSessionWindowSize  per-session receive window; sent
   *                                  via a WINDOW_UPDATE frame
   */
  virtual void setFlowControl(size_t initialReceiveWindow,
                              size_t receiveStreamWindowSize,
                              size_t receiveSessionWindowSize) = 0;

  /**
   * Set outgoing settings for this session
   */
  virtual void setEgressSettings(const SettingsList& inSettings) = 0;

  /**
   * Global flag for turning HTTP2 priorities off
   **/
  void setHTTP2PrioritiesEnabled(bool enabled) /*override*/ {
    h2PrioritiesEnabled_ = enabled;
  }

  virtual bool getHTTP2PrioritiesEnabled() const {
    return h2PrioritiesEnabled_;
  }

  /**
   * Set the maximum number of outgoing transactions this session can open
   * at once. Note: you can only call function before startNow() is called
   * since the remote side can change this value.
   */
  void setMaxConcurrentOutgoingStreams(uint32_t num) {
    // TODO: CHECK(started_);
    maxConcurrentOutgoingStreamsConfig_ = num;
  }

  /**
   * Set the maximum number of transactions the remote can open at once.
   */
  virtual void setMaxConcurrentIncomingStreams(uint32_t num) = 0;

  /**
   * Get/Set the number of egress bytes this session will buffer before
   * pausing all transactions' egress.
   */
  uint32_t getWriteBufferLimit() const {
    return writeBufLimit_;
  }

  void setWriteBufferLimit(uint32_t limit) {
    writeBufLimit_ = limit;
    VLOG(4) << "write buffer limit: " << int(limit / 1000) << "KB";
  }

  void setReadBufferLimit(uint32_t limit) {
    readBufLimit_ = limit;
  }

  /**
   * Start reading from the transport and send any introductory messages
   * to the remote side. This function must be called once per session to
   * begin reads.
   */
  virtual void startNow() = 0;

  /**
   * Send a settings frame
   */
  virtual size_t sendSettings() = 0;

  /**
   * Causes a ping to be sent on the session. If the underlying protocol
   * doesn't support pings, this will return 0. Otherwise, it will return
   * the number of bytes written on the transport to send the ping.
   */
  virtual size_t sendPing() = 0;

  virtual size_t sendPing(uint64_t data) = 0;

  /**
   * Sends a priority message on this session.  If the underlying protocol
   * doesn't support priority, this is a no-op.  A new stream identifier will
   * be selected and returned.
   */
  virtual HTTPCodec::StreamID sendPriority(http2::PriorityUpdate pri)
      /*override*/
      = 0;

  /**
   * As above, but updates an existing priority node.  Do not use for
   * real nodes, prefer HTTPTransaction::changePriority.
   */
  virtual size_t sendPriority(HTTPCodec::StreamID id,
                              http2::PriorityUpdate pri) = 0;

  /**
   * Send a CERTIFICATE_REQUEST frame. If the underlying protocol doesn't
   * support secondary authentication, this is a no-op and 0 is returned.
   */
  virtual size_t sendCertificateRequest(
      std::unique_ptr<folly::IOBuf> /* certificateRequestContext */,
      std::vector<fizz::Extension> /* extensions */) {
    return 0;
  }

  uint64_t getNumTxnServed() const {
    return transactionSeqNo_;
  }

  [[nodiscard]] bool getStreamLimitExceeded() const {
    return streamLimitExceeded_;
  }

  [[nodiscard]] std::chrono::seconds getLatestIdleTime() const /*override*/ {
    DCHECK_GT(transactionSeqNo_, 0u)
        << "No idle time for the first transaction";
    DCHECK(latestActive_ > TimePoint::min());
    return latestIdleDuration_;
  }

  // public HTTPTransaction::Transport overrides
  virtual const folly::SocketAddress& getLocalAddress() const noexcept {
    return localAddr_;
  }
  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return peerAddr_;
  }
  const wangle::TransportInfo& getSetupTransportInfo() const noexcept
  /*override*/ {
    return transportInfo_;
  }
  virtual bool getCurrentTransportInfo(
      wangle::TransportInfo* tinfo) /*override*/
      = 0;

  virtual bool getCurrentTransportInfoWithoutUpdate(
      wangle::TransportInfo* tinfo) const = 0;

  virtual void setHeaderCodecStats(HeaderCodec::Stats* stats) = 0;

  virtual void enableDoubleGoawayDrain() = 0;

  wangle::TransportInfo& getSetupTransportInfo() noexcept {
    return transportInfo_;
  }

  /**
   * If the connection is closed by remote end
   */
  virtual bool connCloseByRemote() = 0;

  // Upstream API

  // The interfaces defined below update the virtual stream based priority
  // scheme from the current system which allows only strict priorities to a
  // flexible system allowing an arbitrary tree of virtual streams, subject only
  // to the limitations in the HTTP/2 specification. An arbitrary prioritization
  // scheme can be implemented by constructing virtual streams corresponding to
  // the desired prioritization and attaching new streams as dependencies of the
  // appropriate virtual stream.
  //
  // The user must define a map from an opaque integer priority level to an
  // HTTP/2 priority corresponding to the virtual stream. This map is
  // implemented by the user in a class that extends
  // HTTPUpstreamSession::PriorityMapFactory. A shared pointer to this class is
  // passed into the constructor of HTTPUpstreamSession. This method will send
  // the virtual streams and return a unique pointer to a class that extends
  // HTTPUpstreamSession::PriorityAdapter. This class implements the map between
  // the user defined priority level and the HTTP/2 priority level.
  //
  // When the session is started, the createVirtualStreams method of
  // PriorityMapFactory is called by HTTPUpstreamSession::startNow. The returned
  // pointer to the PriorityAdapter map is cached in HTTPUpstreamSession. The
  // HTTP/2 priority that should be used for a new stream dependent on a virtual
  // stream corresponding to a given priority level is then retrieved by calling
  // the HTTPUpstreamSession::getHTTPPriority(uint8_t level) method.
  //
  // The prior strict priority system has been left in place for now, but if
  // both maxLevels and PriorityMapFactory are passed into the
  // HTTPUpstreamSession constructor, the maxLevels parameter will be ignored.

  // Implments a map from generic priority level to HTTP/2 priority.
  class PriorityAdapter {
   public:
    virtual folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
        uint8_t level) = 0;
    virtual ~PriorityAdapter() = default;
  };

  class PriorityMapFactory {
   public:
    // Creates the map implemented by PriorityAdapter, sends the corresponding
    // virtual stream on the given session, and retuns the map.
    virtual std::unique_ptr<PriorityAdapter> createVirtualStreams(
        HTTPPriorityMapFactoryProvider* session) const = 0;
    virtual ~PriorityMapFactory() = default;
  };

  using FilterIteratorFn = std::function<void(HTTPCodecFilter*)>;

  virtual bool isDetachable(bool checkSocket) const = 0;

  virtual void attachThreadLocals(folly::EventBase* eventBase,
                                  folly::SSLContextPtr sslContext,
                                  const WheelTimerInstance& wheelTimer,
                                  HTTPSessionStats* stats,
                                  FilterIteratorFn fn,
                                  HeaderCodec::Stats* headerCodecStats,
                                  HTTPSessionController* controller) = 0;

  virtual void detachThreadLocals(bool detachSSLContext = false) = 0;

  /**
   * Creates a new transaction on this upstream session. Invoking this function
   * also has the side-affect of starting reads after this event loop completes.
   *
   * @param handler The request handler to attach to this transaction. It must
   *                not be null.
   */
  virtual HTTPTransaction* newTransaction(
      HTTPTransaction::Handler* handler) = 0;

  virtual bool isReplaySafe() const = 0;

  /**
   * Returns true if the underlying transport can be used again in a new
   * request.
   */
  virtual bool isReusable() const = 0;

  /**
   * Returns true if the session is shutting down
   */
  virtual bool isClosing() const = 0;

  /**
   * Drains the current transactions and prevents new transactions from being
   * created on this session. When the number of transactions reaches zero, this
   * session will shutdown the transport and delete itself.
   */
  virtual void drain() = 0;

  virtual folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
      uint8_t level) = 0;

  /**
   * Enable to use Ex Headers in HTTPSession
   */
  void enableExHeadersSettings() noexcept;

  bool isExHeadersEnabled() noexcept {
    return exHeadersEnabled_;
  }

  void setConnectionToken(
      const HTTPTransaction::ConnectionToken& token) noexcept {
    connectionToken_ = token;
  }

  // Use the protocol's ping feature to test liveness of the peer.  Send a ping
  // every interval seconds.  If the ping is not returned by timeout, drop the
  // connection.
  // If extendIntervalOnIngress is true, then any ingress data will reset the
  // timer until the next PING.
  // If immediate is true, send a ping immediately.  Otherwise, wait one
  // interval.
  virtual void enablePingProbes(std::chrono::seconds interval,
                                std::chrono::seconds timeout,
                                bool extendIntervalOnIngress,
                                bool immediate = false) = 0;

  void setIngressTimeoutAfterEom(bool setIngressTimeoutAfterEom) noexcept {
    setIngressTimeoutAfterEom_ = setIngressTimeoutAfterEom;
  }

  /**
   * Adds an observer.
   *
   * If the observer is already added, this is a no-op.
   *
   * @param observer     Observer to add.
   */
  bool addObserver(HTTPSessionObserverContainer::Observer* observer) {
    if (auto list = getHTTPSessionObserverContainer()) {
      list->addObserver(observer);
      return true;
    }
    return false;
  }

  /**
   * Adds an observer.
   *
   * If the observer is already added, this is a no-op.
   *
   * @param observer     <shared_ptr> Observer to add
   * @return             Whether the observer was added (fails if no list).
   */
  bool addObserver(
      std::shared_ptr<HTTPSessionObserverContainer::Observer> observer) {
    if (auto list = getHTTPSessionObserverContainer()) {
      list->addObserver(std::move(observer));
      return true;
    }
    return false;
  }

  /**
   * Removes an observer.
   *
   * @param observer     Observer to remove.
   * @return             Whether the observer was found and removed.
   */
  bool removeObserver(HTTPSessionObserverContainer::Observer* observer) {
    if (auto list = getHTTPSessionObserverContainer()) {
      list->removeObserver(observer);
      return true;
    } else {
      return false;
    }
  }

  /**
   * Removes an observer.
   *
   * @param observer     Observer to remove.
   * @return             Whether the observer was found and removed.
   */
  bool removeObserver(
      std::shared_ptr<HTTPSessionObserverContainer::Observer> observer) {
    if (auto list = getHTTPSessionObserverContainer()) {
      return list->removeObserver(std::move(observer));
    }
    return false;
  }

 protected:
  bool notifyEgressBodyBuffered(int64_t bytes, bool update);

  void updateWriteBufSize(int64_t delta);

  void updatePendingWrites();

  bool hasPendingEgress() {
    return pendingWriteSize_ + pendingWriteSizeDelta_ > 0;
  }

  uint64_t getPendingWriteSize() const {
    return pendingWriteSize_;
  }

  /**
   * Install a direct response handler for the transaction based on the
   * error.
   */
  void handleErrorDirectly(HTTPTransaction* txn, const HTTPException& error);

  bool onBodyImpl(std::unique_ptr<folly::IOBuf> chain,
                  size_t length,
                  uint16_t padding,
                  HTTPTransaction* txn);

  bool notifyBodyProcessed(uint32_t bytes);

  void setLatestActive() {
    latestActive_ = getCurrentTime();
  }

  bool ingressLimitExceeded() const {
    return pendingReadSize_ > readBufLimit_;
  }
  void onCreateTransaction() {
    if (transactionSeqNo_ >= 1) {
      // idle duration only exists since the 2nd transaction in the session
      latestIdleDuration_ = secondsSince(latestActive_);
    }
  }

  void incrementSeqNo() {
    ++transactionSeqNo_;
  }

  void setStreamLimitExceeded(bool streamLimitExceeded) {
    streamLimitExceeded_ = streamLimitExceeded;
  }

  void onNewOutgoingStream(uint32_t outgoingStreams) {
    if (outgoingStreams > historicalMaxOutgoingStreams_) {
      historicalMaxOutgoingStreams_ = outgoingStreams;
    }
  }

  void setCloseReason(ConnectionCloseReason reason) {
    if (closeReason_ == ConnectionCloseReason::kMAX_REASON) {
      closeReason_ = reason;
    }
  }

  static void handleLastByteEvents(ByteEventTracker* byteEventTracker,
                                   HTTPTransaction* txn,
                                   size_t encodedSize,
                                   size_t byteOffset,
                                   bool piggybacked);

  void runDestroyCallbacks();

  /*
   * Invoked by children upon updating the actual codec wrapped by the filter
   * chain.
   */
  void onCodecChanged();

  /**
   * Initializes the underlying codec's header indexing strategy, if applicable,
   * by retrieving the requisite strategy from the bound controller.
   * This methods exists as some sessions, notably HTTPUpstreamSessions, have
   # their parent controller set after instantiation
   */
  void initCodecHeaderIndexingStrategy();

  /**
   * Attaches session to HTTPSessionController.
   */
  void attachToSessionController();

  /**
   * Informs HTTPSessionController that transport is ready.
   */
  void informSessionControllerTransportReady();

  /**
   * Returns the HTTPSessionObserverContainer or nullptr if not available.
   *
   * HTTPSession implementations that support observers should override this
   * function and return the session observer container that they hold in
   * memory.
   *
   * We have a default implementation to ensure that there is no risk of a
   * pure-virtual function being called during constructon or destruction of
   * the session. If this was to occur the derived class which implements this
   * function may be unavailable leading to undefined behavior. While this is
   * true for any pure-virtual function, the potential for this issue is
   * greater for observers.
   */
  [[nodiscard]] virtual HTTPSessionObserverContainer*
  getHTTPSessionObserverContainer() const {
    return nullptr;
  }

  HTTPSessionStats* sessionStats_{nullptr};

  InfoCallback* infoCallback_{nullptr}; // maybe can move to protected

  wangle::TransportInfo transportInfo_;

  HTTPCodecFilterChain codec_;

  HTTP2PriorityQueue txnEgressQueue_;

  /**
   * Maximum number of ingress body bytes that can be buffered across all
   * transactions for this single session/connection.
   * While changing this value from multiple threads is supported,
   * it is not correct since a thread can use a value changed by another thread
   * We should change the code to avoid this
   */
  static std::atomic<uint32_t> kDefaultReadBufLimit;

  /**
   * The maximum size of the read buffer from the socket.
   */
  static uint32_t maxReadBufferSize_;

  /**
   * Maximum number of bytes that can be buffered across all transactions before
   * this session will start applying backpressure to its transactions.
   */
  static uint32_t kDefaultWriteBufLimit;
  /**
   * Maximum number of bytes to egress per loop when there are > 1
   * transactions.  Otherwise defaults to kDefaultWriteBufLimit.
   */
  static uint32_t egressBodySizeLimit_;

  /** Address of this end of the connection */
  folly::SocketAddress localAddr_;

  /** Address of the remote end of the connection */
  folly::SocketAddress peerAddr_;

  /**
   * Optional connection token associated with this session.
   */
  folly::Optional<HTTPTransaction::ConnectionToken> connectionToken_;

  /**
   * Indicates whether ingress timeout has to be scheduled after EOM is sent.
   */
  bool setIngressTimeoutAfterEom_{false};

  std::unique_ptr<HTTPSessionActivityTracker> httpSessionActivityTracker_;

  std::array<RateLimitFilter*, folly::to_underlying(RateLimitFilter::Type::MAX)>
      rateLimitFilters_{};

 private:
  // Underlying controller_ is marked as private so that callers must
  // utilize getController/setController protected methods.  This ensures we
  // have a single path to update controller_
  HTTPSessionController* controller_{nullptr};

  // private ManagedConnection methods
  std::chrono::milliseconds getIdleTime() const override {
    if (timePointInitialized(latestActive_)) {
      return millisecondsSince(latestActive_);
    } else {
      return std::chrono::milliseconds(0);
    }
  }

  /**
   * The latest time when this session became idle status
   */
  TimePoint latestActive_{};

  /**
   * The idle duration between latest two consecutive active status
   */
  std::chrono::seconds latestIdleDuration_{};

  /** Transaction sequence number */
  uint32_t transactionSeqNo_{0};

  /**
   * The root cause reason this connection was closed.
   */
  ConnectionCloseReason closeReason_{ConnectionCloseReason::kMAX_REASON};

  /**
   * The maximum number concurrent transactions in the history of this session
   */
  uint32_t historicalMaxOutgoingStreams_{0};

  /**
   * The maximum number of concurrent transactions that this session may
   * create, as configured locally.
   */
  uint32_t maxConcurrentOutgoingStreamsConfig_{
      kDefaultMaxConcurrentOutgoingStreams};

  /**
   * Maximum number of cumulative bytes that can be buffered by the
   * transactions in this session before applying backpressure.
   *
   * Note readBufLimit_ is settable via setFlowControl
   */
  uint32_t readBufLimit_{kDefaultReadBufLimit};
  uint32_t writeBufLimit_{kDefaultWriteBufLimit};

  /**
   * Bytes of egress data sent to the socket but not yet written
   * to the network.
   */
  uint64_t pendingWriteSize_{0};

  /**
   * The net change this event loop in the amount of buffered bytes
   * for all this session's txns and socket write buffer.
   */
  int64_t pendingWriteSizeDelta_{0};

  /**
   * Bytes of ingress data read from the socket, but not yet sent to a
   * transaction.
   */
  uint32_t pendingReadSize_{0};

  bool h2PrioritiesEnabled_ : 1;

  /**
   * Indicates whether Ex Headers is supported in HTTPSession
   */
  bool exHeadersEnabled_ : 1;

  /**
   * Indicates whether quic stream ID limit is exceeded.
   */
  bool streamLimitExceeded_{false};
};

} // namespace proxygen
