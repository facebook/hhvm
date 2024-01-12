/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <folly/IntrusiveList.h>
#include <folly/Optional.h>
#include <folly/container/F14Map.h>
#include <folly/container/F14Set.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>
#include <proxygen/lib/http/HTTPConstants.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/codec/FlowControlFilter.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/http/session/ByteEventTracker.h>
#include <proxygen/lib/http/session/HTTPEvent.h>
#include <proxygen/lib/http/session/HTTPSessionActivityTracker.h>
#include <proxygen/lib/http/session/HTTPSessionBase.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/SecondaryAuthManagerBase.h>
#include <proxygen/lib/utils/WheelTimerInstance.h>
#include <queue>
#include <set>
#include <vector>

namespace proxygen {

class HTTPSessionController;
class HTTPSessionStats;

#define PROXYGEN_HTTP_SESSION_USES_BASE 1
constexpr uint32_t kDefaultMaxConcurrentOutgoingStreamsRemote = 100000;
constexpr uint32_t kDefaultMaxConcurrentIncomingStreams = 100;

class HTTPSession
    : public HTTPSessionBase
    , public HTTPTransaction::Transport
    , public ByteEventTracker::Callback
    , protected folly::AsyncTransport::BufferCallback
    , protected HTTPPriorityMapFactoryProvider
    , private FlowControlFilter::Callback
    , private HTTPCodec::Callback
    , private folly::EventBase::LoopCallback
    , private folly::AsyncTransport::ReadCallback
    , private folly::AsyncTransport::ReplaySafetyCallback
    , private folly::AsyncTransport::WriteCallback {
 public:
  using UniquePtr = std::unique_ptr<HTTPSession, Destructor>;

  HTTPTransaction::Transport::Type getType() const noexcept override {
    return HTTPTransaction::Transport::Type::TCP;
  }

  void setHTTPSessionActivityTracker(std::unique_ptr<HTTPSessionActivityTracker>
                                         httpSessionActivityTracker) override {
    if (!getByteEventTracker()) {
      setByteEventTracker(std::make_shared<ByteEventTracker>(this));
    }
    HTTPSessionBase::setHTTPSessionActivityTracker(
        std::move(httpSessionActivityTracker));
  }

  folly::AsyncTransport* getTransport() override {
    return sock_.get();
  }

  folly::EventBase* getEventBase() const override {
    if (sock_) {
      return sock_->getEventBase();
    }
    return nullptr;
  }

  const folly::AsyncTransport* getTransport() const override {
    return sock_.get();
  }

  bool hasActiveTransactions() const override {
    return !transactions_.empty();
  }

  uint32_t getNumStreams() const override {
    return static_cast<uint32_t>(transactions_.size());
  }

  uint32_t getNumOutgoingStreams() const override {
    return outgoingStreams_;
  }

  uint32_t getNumIncomingStreams() const override {
    return incomingStreams_;
  }

  ByteEventTracker* getByteEventTracker() {
    return byteEventTracker_.get();
  }

  HTTPSessionBase* getHTTPSessionBase() override {
    return this;
  }

  void setByteEventTracker(std::shared_ptr<ByteEventTracker> byteEventTracker);

  void setSessionStats(HTTPSessionStats* stats) override;
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
  void setFlowControl(size_t initialReceiveWindow,
                      size_t receiveStreamWindowSize,
                      size_t receiveSessionWindowSize) override;

  /**
   * Set outgoing settings for this session
   */
  void setEgressSettings(const SettingsList& inSettings) override;

  bool getHTTP2PrioritiesEnabled() const override {
    return HTTPSessionBase::getHTTP2PrioritiesEnabled();
  }

  void setHTTP2PrioritiesEnabled(bool enabled) override {
    HTTPSessionBase::setHTTP2PrioritiesEnabled(enabled);
  }

  folly::Optional<HTTPTransaction::ConnectionToken> getConnectionToken()
      const noexcept override {
    return connectionToken_;
  }

  const folly::SocketAddress& getLocalAddress() const noexcept override {
    return HTTPSessionBase::getLocalAddress();
  }

  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return HTTPSessionBase::getPeerAddress();
  }

  const wangle::TransportInfo& getSetupTransportInfo() const noexcept override {
    return HTTPSessionBase::getSetupTransportInfo();
  }

  [[nodiscard]] std::chrono::seconds getLatestIdleTime() const override {
    return HTTPSessionBase::getLatestIdleTime();
  }

  bool getCurrentTransportInfo(wangle::TransportInfo* tinfo) override;

  void getFlowControlInfo(HTTPTransaction::FlowControlInfo* info) override;

  HTTPTransaction::Transport::Type getSessionType() const noexcept override;

  /**
   * Set the maximum number of transactions the remote can open at once.
   */
  void setMaxConcurrentIncomingStreams(uint32_t num) override;

  /**
   * Set the maximum number of bytes allowed to be egressed in the session
   * before cutting it off
   */
  void setEgressBytesLimit(uint64_t bytesLimit);

  /**
   * If set to true, HTTPSession will abort the push streams when receiving
   * a STREAM_RST on the associated stream.
   *
   * This applies to HTTP/2, and it's temporarily available to perform an
   * experiment.
   */
  void setAbortPushesOnRST(bool value) {
    abortPushesOnRST_ = value;
  }

  /**
   * Start reading from the transport and send any introductory messages
   * to the remote side. This function must be called once per session to
   * begin reads.
   */
  void startNow() override;

  /**
   * Send a settings frame
   */
  size_t sendSettings() override;

  /**
   * Causes a ping to be sent on the session. If the underlying protocol
   * doesn't support pings, this will return 0. Otherwise, it will return
   * the number of bytes written on the transport to send the ping.
   */
  size_t sendPing() override;

  /**
   * Sends a priority message on this session.  If the underlying protocol
   * doesn't support priority, this is a no-op.  A new stream identifier will
   * be selected and returned.
   */
  HTTPCodec::StreamID sendPriority(http2::PriorityUpdate pri) override;

  /**
   * As above, but updates an existing priority node.  Do not use for
   * real nodes, prefer HTTPTransaction::changePriority.
   */
  size_t sendPriority(HTTPCodec::StreamID id,
                      http2::PriorityUpdate pri) override;

  /**
   * Send a CERTIFICATE_REQUEST frame. If the underlying protocol doesn't
   * support secondary authentication, this is a no-op and 0 is returned.
   */
  size_t sendCertificateRequest(
      std::unique_ptr<folly::IOBuf> certificateRequestContext,
      std::vector<fizz::Extension> extensions) override;

  // public ManagedConnection methods
  void timeoutExpired() noexcept override {
    readTimeoutExpired();
  }

  void describe(std::ostream& os) const override;
  bool isBusy() const override;
  void notifyPendingShutdown() override;
  void closeWhenIdle() override;
  void dropConnection(const std::string& errorMsg = "") override;
  void dumpConnectionState(uint8_t loglevel) override;

  bool getCurrentTransportInfoWithoutUpdate(
      wangle::TransportInfo* tinfo) const override;

  void setHeaderIndexingStrategy(const HeaderIndexingStrategy* strat) override;

  void setHeaderCodecStats(HeaderCodec::Stats* stats) override {
    codec_->setHeaderCodecStats(stats);
  }

  void enableDoubleGoawayDrain() override {
    codec_->enableDoubleGoawayDrain();
  }

  /**
   * If the connection is closed by remote end
   */
  bool connCloseByRemote() override {
    auto sock = getTransport()->getUnderlyingTransport<folly::AsyncSocket>();
    if (sock) {
      return sock->isClosedByPeer();
    }
    return false;
  }

  /**
   * Attach a SecondaryAuthManager to this session to control secondary
   * certificate authentication in HTTP/2.
   */
  void setSecondAuthManager(
      std::unique_ptr<SecondaryAuthManagerBase> secondAuthManager);

  /**
   * Get the SecondaryAuthManager attached to this session.
   */
  SecondaryAuthManagerBase* getSecondAuthManager() const;

  bool isDetachable(bool checkSocket = true) const override;

  /**
   * Returns true if this session is draining. This can happen if drain()
   * is called explicitly, if a GOAWAY frame is received, or during shutdown.
   */
  bool isDraining() const override {
    return draining_;
  }

  bool readsUnpaused() const {
    return reads_ == SocketState::UNPAUSED;
  }

  bool readsPaused() const {
    return reads_ == SocketState::PAUSED;
  }

  bool readsShutdown() const {
    return reads_ == SocketState::SHUTDOWN;
  }

  bool writesUnpaused() const {
    return writes_ == SocketState::UNPAUSED;
  }

  bool writesPaused() const {
    return writes_ == SocketState::PAUSED;
  }

  bool writesShutdown() const {
    return writes_ == SocketState::SHUTDOWN;
  }

  void enablePingProbes(std::chrono::seconds interval,
                        std::chrono::seconds timeout,
                        bool extendIntervalOnIngress,
                        bool immediate = false) override;

 protected:
  /**
   * HTTPSession is an abstract base class and cannot be instantiated
   * directly. If you want to handle requests and send responses (act as a
   * server), construct a HTTPDownstreamSession. If you want to make
   * requests and handle responses (act as a client), construct a
   * HTTPUpstreamSession.
   *
   * @param wheelTimer  Shared HHWheel Timer instance for scheduling timeouts.
   * @param sock                 An open socket on which any applicable TLS
   *                               handshaking has been completed already.
   * @param localAddr            Address and port of the local end of
   *                               the socket.
   * @param peerAddr             Address and port of the remote end of
   *                               the socket.
   * @param controller           Controller which can create the handler for
   *                               a new transaction.
   * @param codec                A codec with which to parse/generate messages
   *                               in whatever HTTP-like wire format this
   *                               session needs.
   * @param tinfo                Struct containing the transport's TCP/SSL
   *                               level info.
   * @param InfoCallback         Optional callback to be informed of session
   *                               lifecycle events.
   */
  HTTPSession(const WheelTimerInstance& wheelTimer,
              folly::AsyncTransport::UniquePtr sock,
              const folly::SocketAddress& localAddr,
              const folly::SocketAddress& peerAddr,
              HTTPSessionController* controller,
              std::unique_ptr<HTTPCodec> codec,
              const wangle::TransportInfo& tinfo,
              InfoCallback* infoCallback);

  // thrift uses WheelTimer
  HTTPSession(folly::HHWheelTimer* wheelTimer,
              folly::AsyncTransport::UniquePtr sock,
              const folly::SocketAddress& localAddr,
              const folly::SocketAddress& peerAddr,
              HTTPSessionController* controller,
              std::unique_ptr<HTTPCodec> codec,
              const wangle::TransportInfo& tinfo,
              InfoCallback* infoCallback);

  ~HTTPSession() override;

  /**
   * Called by onHeadersComplete(). This function allows downstream and
   * upstream to do any setup (like preparing a handler) when headers are
   * first received from the remote side on a given transaction.
   */
  virtual void setupOnHeadersComplete(HTTPTransaction* txn,
                                      HTTPMessage* msg) = 0;

  /**
   * Called by transactionTimeout if the transaction has no handler.
   */
  virtual HTTPTransaction::Handler* getTransactionTimeoutHandler(
      HTTPTransaction* txn) = 0;

  /**
   * Invoked when headers have been sent.
   */
  virtual void onHeadersSent(const HTTPMessage& /* headers */,
                             bool /* codecWasReusable */) {
  }

  virtual bool allTransactionsStarted() const = 0;

  /**
   * Invoked when the transaction finishes sending a message and
   * appropriately shuts down reads and/or writes with respect to
   * downstream or upstream semantics.
   */
  void onEgressMessageFinished(HTTPTransaction* txn, bool withRST = false);

  /**
   * Gets the next IOBuf to send (either writeBuf_ or new egress from
   * the priority queue), and sets cork appropriately
   */
  std::unique_ptr<folly::IOBuf> getNextToSend(bool* cork,
                                              bool* timestampTx,
                                              bool* timestampAck);

  void decrementTransactionCount(HTTPTransaction* txn,
                                 bool ingressEOM,
                                 bool egressEOM);

  size_t getCodecSendWindowSize() const;

  /**
   * Sends a priority message on this session.  If the underlying protocol
   * doesn't support priority, this is a no-op.  Returns the number of bytes
   * written on the transport
   */
  size_t sendPriorityImpl(HTTPCodec::StreamID streamID,
                          http2::PriorityUpdate pri);

  bool onNativeProtocolUpgradeImpl(HTTPCodec::StreamID txn,
                                   std::unique_ptr<HTTPCodec> codec,
                                   const std::string& protocolString);

  using HTTPTransaction::Transport::getHTTPPriority;
  virtual folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
      uint8_t) override {
    return folly::none;
  }

  void readTimeoutExpired() noexcept;
  void writeTimeoutExpired() noexcept;
  void flowControlTimeoutExpired() noexcept;

  // AsyncTransport::ReadCallback methods
  void getReadBuffer(void** buf, size_t* bufSize) override;
  void readDataAvailable(size_t readSize) noexcept override;
  bool isBufferMovable() noexcept override;
  void readBufferAvailable(std::unique_ptr<folly::IOBuf>) noexcept override;
  void processReadData();
  void readEOF() noexcept override;
  void readErr(const folly::AsyncSocketException&) noexcept override;

  std::string getSecurityProtocol() const override {
    return sock_->getSecurityProtocol();
  }

  // HTTPCodec::Callback methods
  void onMessageBegin(HTTPCodec::StreamID streamID, HTTPMessage* msg) override;
  void onPushMessageBegin(HTTPCodec::StreamID streamID,
                          HTTPCodec::StreamID assocStreamID,
                          HTTPMessage* msg) override;
  void onExMessageBegin(HTTPCodec::StreamID streamID,
                        HTTPCodec::StreamID controlStream,
                        bool unidirectional,
                        HTTPMessage* msg) override;
  void onHeadersComplete(HTTPCodec::StreamID streamID,
                         std::unique_ptr<HTTPMessage> msg) override;
  void onBody(HTTPCodec::StreamID streamID,
              std::unique_ptr<folly::IOBuf> chain,
              uint16_t padding) override;
  void onChunkHeader(HTTPCodec::StreamID stream, size_t length) override;
  void onChunkComplete(HTTPCodec::StreamID stream) override;
  void onTrailersComplete(HTTPCodec::StreamID streamID,
                          std::unique_ptr<HTTPHeaders> trailers) override;
  void onMessageComplete(HTTPCodec::StreamID streamID, bool upgrade) override;
  void onError(HTTPCodec::StreamID streamID,
               const HTTPException& error,
               bool newTxn) override;
  void onAbort(HTTPCodec::StreamID streamID, ErrorCode code) override;
  void onGoaway(uint64_t lastGoodStreamID,
                ErrorCode code,
                std::unique_ptr<folly::IOBuf> debugData = nullptr) override;
  void onPingRequest(uint64_t data) override;
  void onPingReply(uint64_t data) override;
  void onWindowUpdate(HTTPCodec::StreamID stream, uint32_t amount) override;
  void onSettings(const SettingsList& settings) override;
  void onSettingsAck() override;
  void onPriority(HTTPCodec::StreamID stream,
                  const HTTPMessage::HTTP2Priority&) override;
  void onPriority(HTTPCodec::StreamID, const HTTPPriority&) override;
  void onCertificateRequest(uint16_t requestId,
                            std::unique_ptr<folly::IOBuf> authRequest) override;
  void onCertificate(uint16_t certId,
                     std::unique_ptr<folly::IOBuf> authenticator) override;
  uint32_t numOutgoingStreams() const override {
    return outgoingStreams_;
  }
  uint32_t numIncomingStreams() const override {
    return incomingStreams_;
  }

  // HTTPTransaction::Transport methods
  void pauseIngress(HTTPTransaction* txn) noexcept override;
  void resumeIngress(HTTPTransaction* txn) noexcept override;
  void transactionTimeout(HTTPTransaction* txn) noexcept override;
  void sendHeaders(HTTPTransaction* txn,
                   const HTTPMessage& headers,
                   HTTPHeaderSize* size,
                   bool includeEOM) noexcept override;
  size_t sendBody(HTTPTransaction* txn,
                  std::unique_ptr<folly::IOBuf>,
                  bool includeEOM,
                  bool trackLastByteFlushed) noexcept override;
  size_t sendBody(HTTPTransaction*,
                  const HTTPTransaction::BufferMeta&,
                  bool /* eom */) noexcept override {
    return 0;
  }
  size_t sendChunkHeader(HTTPTransaction* txn, size_t length) noexcept override;
  size_t sendChunkTerminator(HTTPTransaction* txn) noexcept override;
  size_t sendEOM(HTTPTransaction* txn,
                 const HTTPHeaders* trailers) noexcept override;
  size_t sendAbort(HTTPTransaction* txn,
                   ErrorCode statusCode) noexcept override;
  size_t sendPriority(HTTPTransaction* txn,
                      const http2::PriorityUpdate& pri) noexcept override;
  size_t changePriority(HTTPTransaction* /*txn*/,
                        HTTPPriority /* pri */) noexcept override;
  void detach(HTTPTransaction* txn) noexcept override;
  size_t sendWindowUpdate(HTTPTransaction* txn,
                          uint32_t bytes) noexcept override;
  void notifyPendingEgress() noexcept override;
  void notifyIngressBodyProcessed(uint32_t bytes) noexcept override;
  void notifyEgressBodyBuffered(int64_t bytes) noexcept override;
  HTTPTransaction* newPushedTransaction(
      HTTPCodec::StreamID assocStreamId,
      HTTPTransaction::PushHandler* handler,
      ProxygenError* error = nullptr) noexcept override;
  HTTPTransaction* newExTransaction(
      HTTPTransaction::Handler* handler,
      HTTPCodec::StreamID controlStream,
      bool unidirectional = false) noexcept override;

  const HTTPCodec& getCodec() const noexcept override {
    return codec_.getChainEnd();
  }

  /**
   * Returns the underlying AsyncTransport.
   * Overrides HTTPTransaction::Transport::getUnderlyingTransport().
   */
  const folly::AsyncTransport* getUnderlyingTransport()
      const noexcept override {
    return sock_.get();
  }

  /**
   * Drains the current transactions and prevents new transactions from being
   * created on this session. If this is an upstream session and the
   * number of transactions reaches zero, this session will shutdown the
   * transport and delete itself. For downstream sessions, an explicit
   * call to dropConnection() or shutdownTransport() is required.
   */
  void drain() override;

  /**
   * Start closing the socket.
   * @param shutdownReads  Whether to close the read side of the
   * socket. All transactions which are not ingress complete will receive
   * an error.
   * @param shutdownWrites Whether to close the write side of the
   * socket. All transactions which are not egress complete will receive
   * an error.
   * @param errorMsg additional error information to pass to each transaction
   */
  void shutdownTransport(bool shutdownReads = true,
                         bool shutdownWrites = true,
                         const std::string& errorMsg = "",
                         ProxygenError error = kErrorEOF);

  /**
   * Immediately close the socket in both directions, discarding any
   * queued writes that haven't yet been transferred to the kernel,
   * and send a RST to the client.
   * All transactions receive onWriteError.
   *
   * @param errorCode  Error code sent with the onWriteError to transactions.
   * @param errorMsg   Error string included in the final error msg.
   */
  void shutdownTransportWithReset(ProxygenError errorCode,
                                  const std::string& errorMsg = "");

  /**
   * Immediately shuts down read events by unhooking the read callback.
   *
   * Since read events must be enabled to receive socket error messages (which
   * include timestamps), disables and drains socket timestamping events in byte
   * tracker to prevent connection shutdown from stalling due to pending
   * timestamp events.
   */
  void shutdownRead();

  // EventBase::LoopCallback methods
  void runLoopCallback() noexcept override;

  /**
   * Schedule a write to occur at the end of this event loop.
   */
  void scheduleWrite();

  /**
   * Update the size of the unwritten egress data and invoke
   * callbacks if the size has crossed the buffering limit.
   */
  void updateWriteCount();

  /**
   * Tells us what would be the offset of the next byte to be
   * enqueued within the whole session.
   */
  inline uint64_t sessionByteOffset() {
    return bytesScheduled_ + writeBuf_.chainLength();
  }

  /**
   * Immediately shut down the session, by deleting the loop callbacks first
   */
  void immediateShutdown();

  /**
   * Check whether the socket is shut down in both directions; if it is,
   * initiate the destruction of this HTTPSession.
   */
  void checkForShutdown();

  /**
   * Get the HTTPTransaction for the given transaction ID, or nullptr if that
   * transaction ID does not exist within this HTTPSession.
   */
  HTTPTransaction* findTransaction(HTTPCodec::StreamID streamID);

  /**
   * Create a new transaction.
   * @return pointer to the transaction on success, or else nullptr if it
   * already exists
   */
  HTTPTransaction* createTransaction(
      HTTPCodec::StreamID streamID,
      const folly::Optional<HTTPCodec::StreamID>& assocStreamID,
      const folly::Optional<HTTPCodec::ExAttributes>& exAttributes,
      const http2::PriorityUpdate& priority = http2::DefaultPriority,
      ProxygenError* error = nullptr);

  /** Invoked by WriteSegment on completion of a write. */
  void writeSuccess() noexcept override;

  /** Invoked by WriteSegment on write failure. */
  void writeErr(size_t bytesWritten,
                const folly::AsyncSocketException& ex) noexcept override;

  /** Check whether to shut down the transport after a write completes. */
  void onWriteCompleted();

  /** Stop reading from the transport until resumeReads() is called */
  void pauseReads();

  /**
   * Send a session layer abort and shutdown the transport for reads and
   * writes.
   */
  void onSessionParseError(const HTTPException& error);

  /**
   * Send a transaction abort and leave the session and transport intact.
   */
  void onNewTransactionParseError(HTTPCodec::StreamID streamID,
                                  const HTTPException& error);

  /**
   * Unpause reading from the transport.
   * @note If any codec callbacks arrived while reads were paused,
   * they will be processed before network reads resume.
   */
  void resumeReads();

  /** Check whether the session has any writes in progress or upcoming */
  bool hasMoreWrites() const;

  /**
   * This function invokes a callback on all transactions. It is safe,
   * but runs in O(n*log n) and if the callback *adds* transactions,
   * they will not get the callback.
   */
  void invokeOnAllTransactions(folly::Function<void(HTTPTransaction*)> fn);

  /**
   * This function invokes a callback on all transactions. It is safe,
   * but runs in O(n*log n) and if the callback *adds* transactions,
   * they will not get the callback.
   */
  void errorOnAllTransactions(ProxygenError err, const std::string& errorMsg);

  void errorOnTransactionIds(const std::vector<HTTPCodec::StreamID>& ids,
                             ProxygenError err,
                             const std::string& extraErrorMsg = "");

  void errorOnTransactionId(HTTPCodec::StreamID id, HTTPException ex);

  /**
   * Returns true iff this session should shutdown at this time. Default
   * behavior is to not shutdown.
   */
  bool shouldShutdown() const;

  void drainImpl();

  void pauseReadsImpl();
  void resumeReadsImpl();

  void rescheduleLoopCallbacks() {
    if (!isLoopCallbackScheduled()) {
      sock_->getEventBase()->runInLoop(this);
    }

    if (shutdownTransportCb_ &&
        !shutdownTransportCb_->isLoopCallbackScheduled()) {
      sock_->getEventBase()->runInLoop(shutdownTransportCb_.get(), true);
    }
  }

  void cancelLoopCallbacks() {
    if (isLoopCallbackScheduled()) {
      cancelLoopCallback();
    }
    if (shutdownTransportCb_) {
      shutdownTransportCb_->cancelLoopCallback();
    }
  }

  // protected members
  class WriteTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit WriteTimeout(HTTPSession* session) : session_(session) {
    }
    ~WriteTimeout() override {
    }

    void timeoutExpired() noexcept override {
      session_->writeTimeoutExpired();
    }

   private:
    HTTPSession* session_;
  };
  WriteTimeout writeTimeout_;

  /** Queue of egress IOBufs */
  folly::IOBufQueue writeBuf_{folly::IOBufQueue::cacheChainLength()};

  /** Chain of ingress IOBufs */
  folly::IOBufQueue readBuf_{folly::IOBufQueue::cacheChainLength()};

  folly::F14NodeMap<HTTPCodec::StreamID, HTTPTransaction> transactions_;
  folly::F14FastSet<HTTPCodec::StreamID> transactionIds_;

  /**
   * Track all current known control streams we have within this session. A
   *stream is considered as a control stream, after some ExStream is associated
   *with it.
   **/
  folly::F14FastSet<HTTPCodec::StreamID> controlStreamIds_;

  /** Count of transactions awaiting input */
  uint32_t liveTransactions_{0};

  folly::AsyncTransport::UniquePtr sock_;

  WheelTimerInstance wheelTimer_;

  /**
   * Number of writes submitted to the transport for which we haven't yet
   * received completion or failure callbacks.
   */
  unsigned numActiveWrites_{0};

  /**
   * Indicates if the session is waiting for existing transactions to close.
   * Once all transactions close, the session will be deleted.
   */
  bool draining_ : 1;

  bool started_ : 1;

  bool writesDraining_ : 1;
  bool resetAfterDrainingWrites_ : 1;
  bool ingressError_ : 1;

 private:
  uint32_t getMaxConcurrentOutgoingStreamsRemote() const override {
    return maxConcurrentOutgoingStreamsRemote_;
  }

  bool isUpstream() const;
  bool isDownstream() const;

  // from folly::AsyncTransport::BufferCallback
  void onEgressBuffered() override;
  void onEgressBufferCleared() override;

  void setupCodec();
  void onSetSendWindow(uint32_t windowSize);
  void onSetMaxInitiatedStreams(uint32_t maxTxns);

  uint32_t getCertAuthSettingVal();

  bool verifyCertAuthSetting(uint32_t value);

  void addLastByteEvent(HTTPTransaction* txn, uint64_t byteNo) noexcept;

  void addAckToLastByteEvent(HTTPTransaction* txn,
                             const ByteEvent& lastByteEvent);

  /**
   * Callback function from the flow control filter if the full window
   * becomes not full.
   */
  void onConnectionSendWindowOpen() override;
  void onConnectionSendWindowClosed() override;

  /**
   * Invoked when the codec processes callbacks for a stream we are no
   * longer tracking.
   */
  void invalidStream(HTTPCodec::StreamID stream,
                     ErrorCode code = ErrorCode::STREAM_CLOSED);

  http2::PriorityUpdate getMessagePriority(const HTTPMessage* msg);

  bool isConnWindowFull() const {
    return connFlowControl_ && connFlowControl_->getAvailableSend() == 0;
  }

  // ByteEventTracker::Callback functions
  void onPingReplyLatency(int64_t latency) noexcept override;
  void onTxnByteEventWrittenToBuf(const ByteEvent& event) noexcept override;
  void onDeleteTxnByteEvent() noexcept override;

  /**
   * Common EOM process shared by sendHeaders, sendBody and sendEOM
   *
   * @param txn             the transaction that's sending request
   * @param encodedSize     size of data frame generated by codec
   * @param piggybacked     whether this eom is a separate sendEOM or
   *                          piggybacked in sendHeaders and sendBody
   */
  void commonEom(HTTPTransaction* txn,
                 size_t encodedSize,
                 bool piggybacked) noexcept;

  /**
   * Add a ReplaySafetyCallback requesting notification when the transport has
   * replay protection.
   *
   * Most transport-layer security protocols (like TLS) provide protection
   * against an eavesdropper capturing data, and later replaying it to the
   * server. However, 0-RTT security protocols allow initial data to be sent
   * without replay protection before the security handshake completes. This
   * function can be used when a HTTP session is in that initial non-replay safe
   * stage, but a request requires a replay safe transport. Will trigger
   * callback synchronously if the transport is already replay safe.
   */
  void addWaitingForReplaySafety(
      ReplaySafetyCallback* callback) noexcept override {
    if (sock_->isReplaySafe()) {
      callback->onReplaySafe();
    } else {
      waitingForReplaySafety_.push_back(callback);
    }
  }

  /**
   * Remove a ReplaySafetyCallback that had been waiting for replay safety
   * (eg if a transaction waiting for replay safety is canceled).
   */
  void removeWaitingForReplaySafety(
      ReplaySafetyCallback* callback) noexcept override {
    waitingForReplaySafety_.remove(callback);
  }

  /**
   * This is a temporary workaround until we have a better way to allocate
   * stream IDs to waiting transactions.
   */
  bool needToBlockForReplaySafety() const override {
    return !waitingForReplaySafety_.empty();
  }

  /**
   * Callback from the transport to this HTTPSession to signal when the
   * transport has become replay safe.
   */
  void onReplaySafe() noexcept override;

  // Returns the number of streams that count against a pipelining limit.
  // Upstreams can't really pipleine (send responses before requests), so
  // count ANY stream against the limit.
  size_t getPipelineStreamCount() const {
    return isDownstream() ? incomingStreams_ : transactions_.size();
  }

  bool maybeResumePausedPipelinedTransaction(size_t oldStreamCount,
                                             uint32_t txnSeqn);

  void incrementOutgoingStreams(HTTPTransaction* txn);
  void incrementIncomingStreams(HTTPTransaction* txn);

  // returns true if the threshold has been exceeded
  bool incrementNumControlMsgsInCurInterval(http2::FrameType frameType);

  // returns true if the rate limiting threshold has been exceeded
  bool incrementDirectErrorHandlingInCurInterval();

  void scheduleResetNumControlMsgs();

  void scheduleResetDirectErrorHandling();

  size_t sendPing(uint64_t data) override;

  // private members

  std::list<ReplaySafetyCallback*> waitingForReplaySafety_;

  folly::Optional<std::pair<uint64_t, HTTPSession::DestructorGuard>>
      pendingWrite_;

  /**
   * Connection level flow control for SPDY >= 3.1 and HTTP/2
   */
  FlowControlFilter* connFlowControl_{nullptr};

  /**
   * The received setting for the maximum number of concurrent
   * transactions that this session may create. We may assume the
   * remote allows unlimited transactions until we get a SETTINGS frame,
   * but to be reasonable, assume the remote doesn't allow more than 100K
   * concurrent transactions on one connection.
   */
  uint32_t maxConcurrentOutgoingStreamsRemote_{
      kDefaultMaxConcurrentOutgoingStreamsRemote};

  /**
   * The maximum number of concurrent transactions that this session's peer
   * may create.
   */
  uint32_t maxConcurrentIncomingStreams_{kDefaultMaxConcurrentIncomingStreams};

  /**
   * The number concurrent transactions initiated by this session
   */
  uint32_t outgoingStreams_{0};

  /**
   * The number of concurrent transactions initiated by this sessions's peer
   */
  uint32_t incomingStreams_{0};

  /**
   * Number of bytes written so far.
   */
  uint64_t bytesWritten_{0};

  /**
   * Number of bytes scheduled so far.
   */
  uint64_t bytesScheduled_{0};

  /**
   * Number of body un-encoded bytes in the write buffer per write iteration.
   */
  uint64_t bodyBytesPerWriteBuf_{0};

  /**
   * Container to hold the results of HTTP2PriorityQueue::nextEgress
   */
  HTTP2PriorityQueue::NextEgressResult nextEgressResults_;

  std::shared_ptr<ByteEventTracker> byteEventTracker_{nullptr};

  std::unique_ptr<HTTPSessionActivityTracker> httpSessionActivityTracker_;

  HTTPTransaction* lastTxn_{nullptr};

  /**
   * Max number of bytes to egress per session
   */
  uint64_t egressBytesLimit_{0};

  // Flow control settings
  size_t initialReceiveWindow_{0};
  size_t receiveStreamWindowSize_{0};
  size_t receiveSessionWindowSize_{0};

  class ShutdownTransportCallback : public folly::EventBase::LoopCallback {
   public:
    explicit ShutdownTransportCallback(HTTPSession* session)
        : session_(session), dg_(std::make_unique<DestructorGuard>(session)) {
    }

    ~ShutdownTransportCallback() override {
    }

    void runLoopCallback() noexcept override {
      VLOG(4) << *session_ << " shutdown from onEgressMessageFinished";
      bool shutdownReads =
          session_->isDownstream() && !session_->ingressUpgraded_;
      auto dg = dg_.release();
      session_->shutdownTransport(shutdownReads, true);
      delete dg;
    }

   private:
    HTTPSession* session_;
    std::unique_ptr<DestructorGuard> dg_;
  };
  std::unique_ptr<ShutdownTransportCallback> shutdownTransportCb_;

  class FlowControlTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit FlowControlTimeout(HTTPSession* session) : session_(session) {
    }
    ~FlowControlTimeout() override {
    }

    void timeoutExpired() noexcept override {
      session_->flowControlTimeoutExpired();
    }

    std::chrono::milliseconds getTimeoutDuration() const {
      return duration_;
    }

    void setTimeoutDuration(std::chrono::milliseconds duration) {
      duration_ = duration;
    }

   private:
    HTTPSession* session_;
    std::chrono::milliseconds duration_{std::chrono::milliseconds(0)};
  };
  FlowControlTimeout flowControlTimeout_;

  class DrainTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit DrainTimeout(HTTPSession* session) : session_(session) {
    }
    ~DrainTimeout() override {
    }

    void timeoutExpired() noexcept override {
      session_->closeWhenIdle();
    }

   private:
    HTTPSession* session_;
  };
  DrainTimeout drainTimeout_;

  class PingProber : public folly::HHWheelTimer::Callback {
   public:
    PingProber(HTTPSession& session,
               std::chrono::seconds interval,
               std::chrono::seconds timeout,
               bool extendIntervalOnIngress,
               bool immediate);

    void startProbes();

    void cancelProbes();

    void refreshTimeout(bool onIngress);

    void timeoutExpired() noexcept override;

    void callbackCanceled() noexcept override {
    }

    void onPingReply(uint64_t data);

   private:
    HTTPSession& session_;
    std::chrono::seconds interval_;
    std::chrono::seconds timeout_;
    folly::Optional<uint64_t> pingVal_;
    bool extendIntervalOnIngress_{true};
  };
  std::unique_ptr<PingProber> pingProber_;

  // secondary authentication manager
  std::unique_ptr<SecondaryAuthManagerBase> secondAuthManager_;

  enum class SocketState : uint8_t {
    UNPAUSED = 0,
    PAUSED = 1,
    SHUTDOWN = 2,
  };

  SocketState reads_;
  SocketState writes_;

  /**
   * Indicates whether an upgrade request has been received from the codec.
   */
  bool ingressUpgraded_ : 1;
  bool resetSocketOnShutdown_ : 1;
  // indicates a fatal error that prevents further ingress data processing
  bool inLoopCallback_ : 1;
  bool pendingPause_ : 1;
  bool writeBufSplit_ : 1;

  bool abortPushesOnRST_{false};

  /**
   * Accessor implementation for HTTPSessionObserver.
   */
  class ObserverAccessor : public HTTPSessionObserverAccessor {
   public:
    explicit ObserverAccessor(HTTPSessionBase* sessionBasePtr)
        : sessionBasePtr_(sessionBasePtr) {
      (void)sessionBasePtr_; // silence unused variable warnings
    }
    ~ObserverAccessor() override = default;

    size_t sendPing(uint64_t data) override {
      if (sessionBasePtr_) {
        return sessionBasePtr_->sendPing(data);
      }
      return 0;
    }

   private:
    HTTPSessionBase* sessionBasePtr_{nullptr};
  };

  ObserverAccessor sessionObserverAccessor_;

  // Container of observers for a HTTP session
  //
  // This member MUST be last in the list of members to ensure it is destroyed
  // first, before any other members are destroyed. This ensures that observers
  // can inspect any session state available through public methods
  // when destruction of the session begins.
  HTTPSessionObserverContainer sessionObserverContainer_;

  HTTPSessionObserverContainer* getHTTPSessionObserverContainer()
      const override {
    return const_cast<HTTPSessionObserverContainer*>(
        &sessionObserverContainer_);
  }
};

} // namespace proxygen
