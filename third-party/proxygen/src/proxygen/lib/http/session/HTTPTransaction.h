/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <climits>
#include <folly/Optional.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestructionBase.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/lang/Assume.h>
#include <iosfwd>
#include <proxygen/lib/http/HTTPConstants.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/ProxygenErrorEnum.h>
#include <proxygen/lib/http/Window.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/session/ByteEvents.h>
#include <proxygen/lib/http/session/HTTP2PriorityQueue.h>
#include <proxygen/lib/http/session/HTTPEvent.h>
#include <proxygen/lib/http/session/HTTPTransactionEgressSM.h>
#include <proxygen/lib/http/session/HTTPTransactionIngressSM.h>
#include <proxygen/lib/http/sink/FlowControlInfo.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <proxygen/lib/utils/Time.h>
#include <proxygen/lib/utils/TraceEvent.h>
#include <proxygen/lib/utils/TraceEventObserver.h>
#include <proxygen/lib/utils/WheelTimerInstance.h>
#include <set>
#include <wangle/acceptor/TransportInfo.h>

namespace proxygen {

/**
 * Experimental
 *
 * A sender interface to send out DSR delegated packetization requests.
 */
class DSRRequestSender {
 public:
  virtual ~DSRRequestSender() = default;

  // This is called back when the underlying session has generated the header
  // bytes for the transaction. At this point it is the responsibility of the
  // DSRRequestSender to call addBufferMeta and sendEOM so that the buffer meta
  // data can start flowing through the transport. The parameter is the offset
  // at which the DSR data begins.
  virtual void onHeaderBytesGenerated(size_t /*dsrDataStartingOffset*/) {
  }
};

/**
 * An HTTPTransaction represents a single request/response pair
 * for some HTTP-like protocol.  It works with a Transport that
 * performs the network processing and wire-protocol formatting
 * and a Handler that implements some sort of application logic.
 *
 * The typical sequence of events for a simple application is:
 *
 *   * The application accepts a connection and creates a Transport.
 *   * The Transport reads from the connection, parses whatever
 *     protocol the client is speaking, and creates a Transaction
 *     to represent the first request.
 *   * Once the Transport has received the full request headers,
 *     it creates a Handler, plugs the handler into the Transaction,
 *     and calls the Transaction's onIngressHeadersComplete() method.
 *   * The Transaction calls the Handler's onHeadersComplete() method
 *     and the Handler begins processing the request.
 *   * If there is a request body, the Transport streams it through
 *     the Transaction to the Handler.
 *   * When the Handler is ready to produce a response, it streams
 *     the response through the Transaction to the Transport.
 *   * When the Transaction has seen the end of both the request
 *     and the response, it detaches itself from the Handler and
 *     Transport and deletes itself.
 *   * The Handler deletes itself at some point after the Transaction
 *     has detached from it.
 *   * The Transport may, depending on the protocol, process other
 *     requests after -- or even in parallel with -- that first
 *     request.  Each request gets its own Transaction and Handler.
 *
 * For some applications, like proxying, a Handler implementation
 * may obtain one or more upstream connections, each represented
 * by another Transport, and create outgoing requests on the upstream
 * connection(s), with each request represented as a new Transaction.
 *
 * With a multiplexing protocol like SPDY on both sides of a proxy,
 * the cardinality relationship can be:
 *
 *                 +-----------+     +-----------+     +-------+
 *   (Client-side) | Transport |1---*|Transaction|1---1|Handler|
 *                 +-----------+     +-----------+     +-------+
 *                                                         1
 *                                                         |
 *                                                         |
 *                                                         1
 *                                   +---------+     +-----------+
 *                (Server-side)      |Transport|1---*|Transaction|
 *                                   +---------+     +-----------+
 *
 * A key design goal of HTTPTransaction is to serve as a protocol-
 * independent abstraction that insulates Handlers from the semantics
 * different of HTTP-like protocols.
 */

/** Info about Transaction running on this session */
class TransactionInfo {
 public:
  TransactionInfo() {
  }

  TransactionInfo(std::chrono::milliseconds ttfb,
                  std::chrono::milliseconds ttlb,
                  uint64_t eHeader,
                  uint64_t inHeader,
                  uint64_t eBody,
                  uint64_t inBody,
                  bool completed)
      : timeToFirstByte(ttfb),
        timeToLastByte(ttlb),
        egressHeaderBytes(eHeader),
        ingressHeaderBytes(inHeader),
        egressBodyBytes(eBody),
        ingressBodyBytes(inBody),
        isCompleted(completed) {
  }

  /** Time to first byte */
  std::chrono::milliseconds timeToFirstByte{0};
  /** Time to last byte */
  std::chrono::milliseconds timeToLastByte{0};

  /** Number of bytes send in headers */
  uint64_t egressHeaderBytes{0};
  /** Number of bytes receive headers */
  uint64_t ingressHeaderBytes{0};
  /** Number of bytes send in body */
  uint64_t egressBodyBytes{0};
  /** Number of bytes receive in body */
  uint64_t ingressBodyBytes{0};

  /** Is the transaction was completed without error */
  bool isCompleted{false};
};

class HTTPSessionStats;
class HTTPTransaction;
class HTTPTransactionHandler : public TraceEventObserver {
 public:
  /**
   * Called once per transaction. This notifies the handler of which
   * transaction it should talk to and will receive callbacks from.
   */
  virtual void setTransaction(HTTPTransaction* txn) noexcept = 0;

  /**
   * Called once after a transaction successfully completes. It
   * will be called even if a read or write error happened earlier.
   * This is a terminal callback, which means that the HTTPTransaction
   * object that gives this call will be invalid after this function
   * completes.
   */
  virtual void detachTransaction() noexcept = 0;

  /**
   * Called at most once per transaction. This is usually the first
   * ingress callback. It is possible to get a read error before this
   * however. If you had previously called pauseIngress(), this callback
   * will be delayed until you call resumeIngress().
   */
  virtual void onHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept = 0;

  /**
   * Can be called multiple times per transaction. If you had previously
   * called pauseIngress(), this callback will be delayed until you call
   * resumeIngress().
   */
  virtual void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept = 0;

  /**
   * Same as onBody() but with additional offset parameter.
   */
  virtual void onBodyWithOffset(uint64_t /* bodyOffset */,
                                std::unique_ptr<folly::IOBuf> chain) {
    onBody(std::move(chain));
  }

  /**
   * Can be called multiple times per transaction. If you had previously
   * called pauseIngress(), this callback will be delayed until you call
   * resumeIngress(). This signifies the beginning of a chunk of length
   * 'length'. You will receive onBody() after this. Also, the length will
   * be greater than zero.
   */
  virtual void onChunkHeader(size_t /* length */) noexcept {
  }

  /**
   * Can be called multiple times per transaction. If you had previously
   * called pauseIngress(), this callback will be delayed until you call
   * resumeIngress(). This signifies the end of a chunk.
   */
  virtual void onChunkComplete() noexcept {
  }

  /**
   * Can be called any number of times per transaction. If you had
   * previously called pauseIngress(), this callback will be delayed until
   * you call resumeIngress(). Trailers can be received once right before
   * the EOM of a chunked HTTP/1.1 reponse or multiple times per
   * transaction from SPDY and HTTP/2.0 HEADERS frames.
   */
  virtual void onTrailers(std::unique_ptr<HTTPHeaders> trailers) noexcept = 0;

  /**
   * Can be called once per transaction. If you had previously called
   * pauseIngress(), this callback will be delayed until you call
   * resumeIngress(). After this callback is received, there will be no
   * more normal ingress callbacks received (onEgress*() and onError()
   * may still be invoked). The Handler should consider
   * ingress complete after receiving this message. This Transaction is
   * still valid, and work may still occur on it until detachTransaction
   * is called.
   */
  virtual void onEOM() noexcept = 0;

  /**
   * Can be called once per transaction. If you had previously called
   * pauseIngress(), this callback will be delayed until you call
   * resumeIngress(). After this callback is invoked, further data
   * will be forwarded using the onBody() callback. Once the data transfer
   * is completed (EOF recevied in case of CONNECT), onEOM() callback will
   * be invoked.
   */
  virtual void onUpgrade(UpgradeProtocol protocol) noexcept = 0;

  /**
   * Can be called at any time before detachTransaction(). This callback
   * implies that an error has occurred. To determine if ingress or egress
   * is affected, check the direciont on the HTTPException. If the
   * direction is INGRESS, it MAY still be possible to send egress.
   */
  virtual void onError(const HTTPException& error) noexcept = 0;

  /**
   * Can be called at any time before detachTransaction(). This callback is
   * invoked in cases that violate an internal invariant that is fatal to the
   * transaction but can be recoverable for the session or library.  One such
   * example is mis-use of the egress APIs (sendBody() before sendHeaders()).
   */
  virtual void onInvariantViolation(const HTTPException& error) noexcept {
    LOG(FATAL) << error.what();
  }
  /**
   * If the remote side's receive buffer fills up, this callback will be
   * invoked so you can attempt to stop sending to the remote side.
   */
  virtual void onEgressPaused() noexcept = 0;

  /**
   * This callback lets you know that the remote side has resumed reading
   * and you can now continue to send data.
   */
  virtual void onEgressResumed() noexcept = 0;

  /**
   * Ask the handler to construct a handler for a pushed transaction associated
   * with its transaction.
   *
   * TODO: Reconsider default implementation here. If the handler
   * does not implement, better set max initiated to 0 in a settings frame?
   */
  virtual void onPushedTransaction(HTTPTransaction* /* txn */) noexcept {
  }

  /**
   * Ask the handler to construct a handler for a ExTransaction associated
   * with its transaction.
   */
  virtual void onExTransaction(HTTPTransaction* /* txn */) noexcept {
  }

  /**
   * Inform the handler that a GOAWAY has been received on the
   * transport. This callback will only be invoked if the transport is
   * SPDY or HTTP/2. It may be invoked multiple times, as HTTP/2 allows this.
   *
   * @param code The error code received in the GOAWAY frame
   */
  virtual void onGoaway(ErrorCode /* code */) noexcept {
  }

  /**
   * Can be called multiple times per transaction after onHeadersComplete and
   * before detachTransaction()
   *
   * It does not obey pauseIngress/resumeIngress it is up to the handler
   * to decide whether to buffer/drop datagrams
   */
  virtual void onDatagram(std::unique_ptr<folly::IOBuf> /*datagram*/) noexcept {
  }

  /**
   * Invoked when the peer initiates a new bidirectional WebTransport stream.
   * This can only be invoked when the transaction has successfully negotiated
   * WebTransport (CONNECT(webtransport) + 2xx).
   *
   * Once it is called the handler is responsible for disposing of the stream
   * until the transaction detaches, at which point it will be automatically
   * reset.
   */
  virtual void onWebTransportBidiStream(
      HTTPCodec::StreamID /*id*/,
      WebTransport::BidiStreamHandle /*stream*/) noexcept {
  }

  /**
   * Invoked when the peer initiates a new unidirectional WebTransport stream.
   * This can only be invoked when the transaction has successfully negotiated
   * WebTransport (CONNECT(webtransport) + 2xx).
   *
   * Once it is called the handler is responsible for disposing of the stream
   * until the transaction detaches, at which point it will be automatically
   * abandoned.
   */
  virtual void onWebTransportUniStream(
      HTTPCodec::StreamID /*id*/,
      WebTransport::StreamReadHandle* /*stream*/) noexcept {
  }

  /**
   * Invoked when the peer closes the WebTransport session.
   * This can only be invoked when the transaction has successfully negotiated
   * WebTransport (CONNECT(webtransport) + 2xx).
   *
   * Before this is called, all outstanding stream reads and writes will return
   * an error, and all stream handles will be cancelled and invalidated.
   *
   * This will be called prior to detachTransaction.  An error code will be
   * passed if the peer sent a DRAIN_WEBTRANSPORT_SESSION capsule.
   */
  virtual void onWebTransportSessionClose(
      folly::Optional<uint32_t> /*error*/) noexcept {
  }

  virtual ~HTTPTransactionHandler() override {
  }
};

class HTTPPushTransactionHandler : public HTTPTransactionHandler {
 public:
  ~HTTPPushTransactionHandler() override {
  }

  void onHeadersComplete(std::unique_ptr<HTTPMessage>) noexcept final {
    LOG(FATAL) << "push txn received headers";
  }

  void onBody(std::unique_ptr<folly::IOBuf>) noexcept final {
    LOG(FATAL) << "push txn received body";
  }

  void onBodyWithOffset(uint64_t,
                        std::unique_ptr<folly::IOBuf>) noexcept final {
    LOG(FATAL) << "push txn received body with offset";
  }

  void onChunkHeader(size_t /* length */) noexcept final {
    LOG(FATAL) << "push txn received chunk header";
  }

  void onChunkComplete() noexcept final {
    LOG(FATAL) << "push txn received chunk complete";
  }

  void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept final {
    LOG(FATAL) << "push txn received trailers";
  }

  void onEOM() noexcept final {
    LOG(FATAL) << "push txn received EOM";
  }

  void onUpgrade(UpgradeProtocol) noexcept final {
    LOG(FATAL) << "push txn received upgrade";
  }

  void onPushedTransaction(HTTPTransaction*) noexcept final {
    LOG(FATAL) << "push txn received push txn";
  }
};

/**
 * Callback interface to be notified of events on the byte stream.
 */
class HTTPTransactionTransportCallback {
 public:
  virtual void firstHeaderByteFlushed() noexcept = 0;

  virtual void firstByteFlushed() noexcept = 0;

  virtual void lastByteFlushed() noexcept = 0;

  virtual void trackedByteFlushed() noexcept {
  }

  virtual void lastByteAcked(std::chrono::milliseconds latency) noexcept = 0;

  virtual void trackedByteEventTX(const ByteEvent& /* event */) noexcept {
  }

  virtual void trackedByteEventAck(const ByteEvent& /* event */) noexcept {
  }

  virtual void egressBufferEmpty() noexcept {
  }

  virtual void headerBytesGenerated(HTTPHeaderSize& size) noexcept = 0;

  virtual void headerBytesReceived(const HTTPHeaderSize& size) noexcept = 0;

  // May include extra bytes for EOF/trailers
  virtual void bodyBytesGenerated(size_t nbytes) noexcept = 0;

  virtual void bodyBytesReceived(size_t size) noexcept = 0;

  virtual void lastEgressHeaderByteAcked() noexcept {
  }

  virtual void bodyBytesTx(uint64_t /* bodyOffset */) noexcept {
  }

  virtual void bodyBytesDelivered(uint64_t /* bodyOffset */) noexcept {
  }

  virtual void bodyBytesDeliveryCancelled(uint64_t /* bodyOffset */) noexcept {
  }

  virtual void transportAppRateLimited() noexcept {
  }

  virtual void datagramBytesGenerated(size_t /* nbytes */) noexcept {
  }
  virtual void datagramBytesReceived(size_t /* size */) noexcept {
  }

  virtual ~HTTPTransactionTransportCallback() {
  }
};

class HTTPSessionBase;
class HTTPTransaction
    : public folly::HHWheelTimer::Callback
    , public folly::DelayedDestructionBase {
 public:
  using Handler = HTTPTransactionHandler;
  using PushHandler = HTTPPushTransactionHandler;

  using FlowControlInfo = proxygen::FlowControlInfo;
  /**
   * Experimental.
   *
   * BufferMeta represents a buffer. The real data will be sourced from another
   * place.
   */
  struct BufferMeta {
    size_t length{0};

    BufferMeta() = default;
    explicit BufferMeta(size_t lengthIn) : length(lengthIn) {
    }

    BufferMeta split(size_t splitLen) {
      CHECK_GE(length, splitLen);
      length -= splitLen;
      return BufferMeta(splitLen);
    }
  };

  /**
   * Opaque token that identifies the underlying connection.
   * Transaction handlers can use this token to group different
   * Transport instances by the distinct underlying connections.
   * Its uniqueness is not enforced by the Transport.
   */
  using ConnectionToken = std::string;

  class Transport {
   public:
    enum class Type : uint8_t { TCP, QUIC };

    virtual ~Transport() {
    }

    virtual void pauseIngress(HTTPTransaction* txn) noexcept = 0;

    virtual void resumeIngress(HTTPTransaction* txn) noexcept = 0;

    virtual void transactionTimeout(HTTPTransaction* txn) noexcept = 0;

    virtual void sendHeaders(HTTPTransaction* txn,
                             const HTTPMessage& headers,
                             HTTPHeaderSize* size,
                             bool eom) noexcept = 0;

    /**
     * Experimental API
     *
     * Send headers and a DSRRequestSender to Transport.
     * The Transport will generate header for DATA frame.
     *
     * dataFrameHeaderSize: An output parameter to get the size of DATA frame
     * header.
     */
    virtual bool sendHeadersWithDelegate(
        HTTPTransaction*,
        const HTTPMessage&,
        HTTPHeaderSize*,
        size_t* /* dataFrameHeaderSize */,
        uint64_t /* contentLength */,
        std::unique_ptr<DSRRequestSender>) noexcept {
      return false;
    }

    // Experimental
    virtual size_t sendBody(HTTPTransaction*,
                            const BufferMeta&,
                            bool /* eom */) noexcept = 0;

    virtual size_t sendBody(HTTPTransaction* txn,
                            std::unique_ptr<folly::IOBuf> iobuf,
                            bool eom,
                            bool trackLastByteFlushed) noexcept = 0;

    virtual size_t sendChunkHeader(HTTPTransaction* txn,
                                   size_t length) noexcept = 0;

    virtual size_t sendChunkTerminator(HTTPTransaction* txn) noexcept = 0;

    virtual size_t sendEOM(HTTPTransaction* txn,
                           const HTTPHeaders* trailers) noexcept = 0;

    virtual size_t sendAbort(HTTPTransaction* txn,
                             ErrorCode statusCode) noexcept = 0;

    virtual size_t sendPriority(HTTPTransaction* txn,
                                const http2::PriorityUpdate& pri) noexcept = 0;
    /*
     * Updates the Local priority for the transaction.
     * For an upstream transaction it also sends the priority update to the peer
     */
    virtual size_t changePriority(HTTPTransaction* txn,
                                  HTTPPriority pri) noexcept = 0;

    virtual size_t sendWindowUpdate(HTTPTransaction* txn,
                                    uint32_t bytes) noexcept = 0;

    virtual void notifyPendingEgress() noexcept = 0;

    virtual void detach(HTTPTransaction* txn) noexcept = 0;

    virtual void notifyIngressBodyProcessed(uint32_t bytes) noexcept = 0;

    virtual void notifyEgressBodyBuffered(int64_t bytes) noexcept = 0;

    virtual const folly::SocketAddress& getLocalAddress() const noexcept = 0;

    virtual const folly::SocketAddress& getPeerAddress() const noexcept = 0;

    [[nodiscard]] virtual std::chrono::seconds getLatestIdleTime() const = 0;

    virtual void describe(std::ostream&) const = 0;

    virtual const wangle::TransportInfo& getSetupTransportInfo()
        const noexcept = 0;

    virtual bool getCurrentTransportInfo(wangle::TransportInfo* tinfo) = 0;

    virtual void getFlowControlInfo(FlowControlInfo* info) = 0;

    virtual HTTPTransaction::Transport::Type getSessionType()
        const noexcept = 0;

    virtual const HTTPCodec& getCodec() const noexcept = 0;

    /*
     * Drain the underlying session. This will affect other transactions
     * running on the same session and is discouraged unless you are confident
     * that the session is broken.
     */
    virtual void drain() = 0;

    virtual bool isDraining() const = 0;

    virtual HTTPTransaction* newPushedTransaction(
        HTTPCodec::StreamID assocStreamId,
        HTTPTransaction::PushHandler* handler,
        ProxygenError* error = nullptr) noexcept = 0;

    virtual HTTPTransaction* newExTransaction(HTTPTransaction::Handler* handler,
                                              HTTPCodec::StreamID controlStream,
                                              bool unidirectional) noexcept = 0;

    virtual std::string getSecurityProtocol() const = 0;

    virtual void addWaitingForReplaySafety(
        folly::AsyncTransport::ReplaySafetyCallback* callback) noexcept = 0;

    virtual void removeWaitingForReplaySafety(
        folly::AsyncTransport::ReplaySafetyCallback* callback) noexcept = 0;

    virtual bool needToBlockForReplaySafety() const = 0;

    virtual const folly::AsyncTransport* getUnderlyingTransport()
        const noexcept = 0;

    /**
     * Returns true if the underlying transport has completed full handshake.
     */
    virtual bool isReplaySafe() const = 0;

    virtual void setHTTP2PrioritiesEnabled(bool enabled) = 0;
    virtual bool getHTTP2PrioritiesEnabled() const = 0;

    virtual HTTPSessionBase* getHTTPSessionBase() = 0;

    virtual folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
        uint8_t level) = 0;

    virtual folly::Optional<HTTPPriority> getHTTPPriority() {
      return folly::none;
    }

    virtual uint16_t getDatagramSizeLimit() const noexcept {
      return 0;
    }

    virtual bool sendDatagram(std::unique_ptr<folly::IOBuf> /*datagram*/) {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    [[nodiscard]] virtual bool supportsWebTransport() const {
      return false;
    }

    virtual folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
    newWebTransportBidiStream() {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    virtual folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
    newWebTransportUniStream() {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    enum class FCState { BLOCKED, UNBLOCKED };
    virtual folly::Expected<FCState, WebTransport::ErrorCode>
    sendWebTransportStreamData(HTTPCodec::StreamID /*id*/,
                               std::unique_ptr<folly::IOBuf> /*data*/,
                               bool /*eof*/) {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
    resetWebTransportEgress(HTTPCodec::StreamID /*id*/,
                            uint32_t /*errorCode*/) {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
    pauseWebTransportIngress(HTTPCodec::StreamID /*id*/) {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
    resumeWebTransportIngress(HTTPCodec::StreamID /*id*/) {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    virtual folly::Expected<folly::Unit, WebTransport::ErrorCode>
    stopReadingWebTransportIngress(HTTPCodec::StreamID /*id*/,
                                   uint32_t /*errorCode*/) {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    /**
     * Ask transport to track and ack body delivery.
     */
    virtual void trackEgressBodyOffset(uint64_t /* bodyOffset */,
                                       ByteEvent::EventFlags /*flags*/) {
      LOG(FATAL) << __func__ << " not supported";
      folly::assume_unreachable();
    }

    virtual folly::Optional<HTTPTransaction::ConnectionToken>
    getConnectionToken() const noexcept = 0;
  };

  using TransportCallback = HTTPTransactionTransportCallback;

  /**
   * readBufLimit and sendWindow are only used if useFlowControl is
   * true. Furthermore, if flow control is enabled, no guarantees can be
   * made on the borders of the L7 chunking/data frames of the outbound
   * messages.
   *
   * priority is only used by SPDY. The -1 default makes sure that all
   * plain HTTP transactions land up in the same queue as the control data.
   */
  HTTPTransaction(
      TransportDirection direction,
      HTTPCodec::StreamID id,
      uint32_t seqNo,
      Transport& transport,
      HTTP2PriorityQueueBase& egressQueue,
      folly::HHWheelTimer* timer = nullptr,
      const folly::Optional<std::chrono::milliseconds>& defaultIdleTimeout =
          folly::Optional<std::chrono::milliseconds>(),
      HTTPSessionStats* stats = nullptr,
      bool useFlowControl = false,
      uint32_t receiveInitialWindowSize = 0,
      uint32_t sendInitialWindowSize = 0,
      http2::PriorityUpdate = http2::DefaultPriority,
      folly::Optional<HTTPCodec::StreamID> assocStreamId = HTTPCodec::NoStream,
      folly::Optional<HTTPCodec::ExAttributes> exAttributes =
          HTTPCodec::NoExAttributes,
      bool setIngressTimeoutAfterEom = false);

  ~HTTPTransaction() override;

  void reset(bool useFlowControl,
             uint32_t receiveInitialWindowSize,
             uint32_t receiveStreamWindowSize,
             uint32_t sendInitialWindowSize);

  HTTPCodec::StreamID getID() const {
    return id_;
  }

  uint32_t getSequenceNumber() const {
    return seqNo_;
  }

  const Transport& getTransport() const {
    return transport_;
  }

  Transport& getTransport() {
    return transport_;
  }

  virtual void setHandler(Handler* handler) {
    if (handler_ != handler) {
      handlerEgressPaused_ = false;
    }
    handler_ = handler;
    if (handler_) {
      DestructorGuard dg(this);
      handler_->setTransaction(this);
      updateHandlerPauseState();
    }
  }

  const Handler* getHandler() const {
    return handler_;
  }

  Handler* getHandler() {
    return handler_;
  }

  http2::PriorityUpdate getPriority() const {
    return priority_;
  }

  std::tuple<uint64_t, uint64_t, double> getPrioritySummary() const {
    return std::make_tuple(
        insertDepth_,
        currentDepth_,
        egressCalls_ > 0 ? cumulativeRatio_ / static_cast<double>(egressCalls_)
                         : 0);
  }

  folly::Optional<HTTPPriority> getHTTPPriority() const {
    return transport_.getHTTPPriority();
  }

  bool getPriorityFallback() const {
    return priorityFallback_;
  }

  HTTPTransactionEgressSM::State getEgressState() const {
    return egressState_;
  }

  HTTPTransactionIngressSM::State getIngressState() const {
    return ingressState_;
  }

  bool isUpstream() const {
    return direction_ == TransportDirection::UPSTREAM;
  }

  bool isDownstream() const {
    return direction_ == TransportDirection::DOWNSTREAM;
  }

  void getLocalAddress(folly::SocketAddress& addr) const {
    addr = transport_.getLocalAddress();
  }

  void getPeerAddress(folly::SocketAddress& addr) const {
    addr = transport_.getPeerAddress();
  }

  const folly::SocketAddress& getLocalAddress() const noexcept {
    return transport_.getLocalAddress();
  }

  const folly::SocketAddress& getPeerAddress() const noexcept {
    return transport_.getPeerAddress();
  }

  const wangle::TransportInfo& getSetupTransportInfo() const noexcept {
    return transport_.getSetupTransportInfo();
  }

  void getCurrentTransportInfo(wangle::TransportInfo* tinfo) const {
    transport_.getCurrentTransportInfo(tinfo);
  }

  void getCurrentFlowControlInfo(FlowControlInfo* info) const {
    transport_.getFlowControlInfo(info);
    info->streamSendWindow_ = sendWindow_.getCapacity();
    info->streamSendOutstanding_ = sendWindow_.getOutstanding();
    info->streamRecvWindow_ = recvWindow_.getCapacity();
    info->streamRecvOutstanding_ = recvWindow_.getOutstanding();
  }

  HTTPSessionStats* getSessionStats() const {
    return stats_;
  }

  /**
   * Check whether more response is expected. One or more 1xx status
   * responses can be received prior to the regular response.
   * Note: 101 is handled by the codec using a separate onUpgrade callback
   */
  virtual bool extraResponseExpected() const {
    return (lastResponseStatus_ >= 100 && lastResponseStatus_ < 200) &&
           lastResponseStatus_ != 101;
  }

  /**
   * Change the size of the receive window and propagate the change to the
   * remote end using a window update.
   *
   * TODO: when HTTPSession sends a SETTINGS frame indicating a
   * different initial window, it should call this function on all its
   * transactions.
   */
  virtual void setReceiveWindow(uint32_t capacity);

  /**
   * Get the receive window of the transaction
   */
  virtual const Window& getReceiveWindow() const {
    return recvWindow_;
  }

  uint32_t getMaxDeferredSize() {
    return maxDeferredIngress_;
  }

  /**
   * Invoked by the session when the ingress headers are complete
   */
  void onIngressHeadersComplete(std::unique_ptr<HTTPMessage> msg);

  /**
   * Invoked by the session when some or all of the ingress entity-body has
   * been parsed.
   */
  void onIngressBody(std::unique_ptr<folly::IOBuf> chain, uint16_t padding);

  /**
   * Invoked by the session when a chunk header has been parsed.
   */
  void onIngressChunkHeader(size_t length);

  /**
   * Invoked by the session when the CRLF terminating a chunk has been parsed.
   */
  void onIngressChunkComplete();

  /**
   * Invoked by the session when the ingress trailers have been parsed.
   */
  void onIngressTrailers(std::unique_ptr<HTTPHeaders> trailers);

  /**
   * Invoked by the session when the session and transaction need to be
   * upgraded to a different protocol
   */
  void onIngressUpgrade(UpgradeProtocol protocol);

  /**
   * Invoked by the session when the ingress message is complete.
   */
  void onIngressEOM();

  /**
   * Invoked by the session when there is an error (e.g., invalid syntax,
   * TCP RST) in either the ingress or egress stream. Note that this
   * message is processed immediately even if this transaction normally
   * would queue ingress.
   *
   * @param error Details for the error. This exception also has
   * information about whether the error applies to the ingress, egress,
   * or both directions of the transaction
   */
  void onError(const HTTPException& error);

  /**
   * Invoked by the session when a GOAWAY frame is received.
   * TODO: we may consider exposing the additional debug data here in the
   * future.
   *
   * @param code The error code received in the GOAWAY frame
   */
  void onGoaway(ErrorCode code);

  /**
   * Invoked by the session when there is a timeout on the ingress stream.
   * Note that each transaction has its own timer but the session
   * is the effective target of the timer.
   */
  void onIngressTimeout();

  /**
   * Invoked by the session when the remote endpoint of this transaction
   * signals that it has consumed 'amount' bytes. This is only for
   * versions of HTTP that support per transaction flow control.
   */
  void onIngressWindowUpdate(uint32_t amount);

  /**
   * Invoked by the session when the remote endpoint signals that we
   * should change our send window. This is only for
   * versions of HTTP that support per transaction flow control.
   */
  void onIngressSetSendWindow(uint32_t newWindowSize);

  /**
   * Notify this transaction that it is ok to egress.  Returns true if there
   * is additional pending egress
   */
  bool onWriteReady(uint32_t maxEgress, double ratio);

  /**
   * Invoked by the session when there is a timeout on the egress stream.
   */
  void onEgressTimeout();

  /**
   * Invoked by the session when the first header byte is flushed.
   */
  void onEgressHeaderFirstByte();

  /**
   * Invoked by the session when the first byte is flushed.
   */
  void onEgressBodyFirstByte();

  /**
   * Invoked by the session when the last byte is flushed.
   */
  void onEgressBodyLastByte();

  /**
   * Invoked by the session when the tracked byte is flushed.
   */
  void onEgressTrackedByte();

  /**
   * Invoked when the ACK_LATENCY event is delivered
   *
   * @param latency the time between the moment when the last byte was sent
   *        and the moment when we received the ACK from the client
   */
  void onEgressLastByteAck(std::chrono::milliseconds latency);

  /**
   * Invoked by the session when last egress headers have been acked by the
   * peer.
   */
  void onLastEgressHeaderByteAcked();

  /**
   * Invoked by the session when egress body has been transmitted to the
   * peer. Called for each sendBody() call if body bytes tracking is enabled.
   */
  void onEgressBodyBytesTx(uint64_t bodyOffset);

  /**
   * Invoked by the session when egress body has been acked by the
   * peer. Called for each sendBody() call if body bytes tracking is enabled.
   */
  void onEgressBodyBytesAcked(uint64_t bodyOffset);

  /**
   * Invoked by the session when egress body tx or delivery has been cancelled
   * by the peer.
   */
  void onEgressBodyDeliveryCanceled(uint64_t bodyOffset);

  /**
   * Invoked by the session when a tracked ByteEvent is transmitted by NIC.
   */
  void onEgressTrackedByteEventTX(const ByteEvent& event);

  /**
   * Invoked by the session when a tracked ByteEvent is ACKed by remote peer.
   *
   * LAST_BYTE events are processed by legacy functions.
   */
  void onEgressTrackedByteEventAck(const ByteEvent& event);

  /**
   * Invoked if the egress transport becomes app rate limited.
   *
   * TODO(bschlinker): Add support for QUIC.
   */
  void onEgressTransportAppRateLimited();

  /**
   * Can be called multiple times per transaction after onHeadersComplete and
   * before detachTransaction()
   *
   * It does not obey pauseIngress/resumeIngress it is up to the handler
   * to decide whether to buffer/drop datagrams
   */
  void onDatagram(std::unique_ptr<folly::IOBuf> datagram) noexcept;

  // Calls from Transport
  void onWebTransportBidiStream(HTTPCodec::StreamID id);

  void onWebTransportUniStream(HTTPCodec::StreamID id);

  void onWebTransportStreamIngress(HTTPCodec::StreamID id,
                                   std::unique_ptr<folly::IOBuf> data,
                                   bool eof);

  void onWebTransportStreamError(HTTPCodec::StreamID id, uint32_t errorCode);

  bool onWebTransportStopSending(HTTPCodec::StreamID id, uint32_t errorCode);

  void onWebTransportEgressReady(HTTPCodec::StreamID id);

  /**
   * Invoked by the handlers that are interested in tracking
   * performance stats.
   */
  virtual void setTransportCallback(TransportCallback* cb) {
    transportCallback_ = cb;
  }

  /**
   * @return true if ingress has started on this transaction.
   */
  bool isIngressStarted() const {
    return ingressState_ != HTTPTransactionIngressSM::State::Start;
  }

  /**
   * @return true iff the ingress EOM has been queued in HTTPTransaction
   * but the handler has not yet been notified of this event.
   */
  bool isIngressEOMQueued() const {
    return ingressState_ == HTTPTransactionIngressSM::State::EOMQueued;
  }

  /**
   * @return true iff the handler has been notified of the ingress EOM.
   */
  bool isIngressComplete() const {
    return ingressState_ == HTTPTransactionIngressSM::State::ReceivingDone;
  }

  /**
   * @return true iff onIngressEOM() has been called.
   */
  bool isIngressEOMSeen() const {
    return isIngressEOMQueued() || isIngressComplete();
  }

  /**
   * @return true if egress has started on this transaction.
   */
  bool isEgressStarted() const {
    return egressState_ != HTTPTransactionEgressSM::State::Start;
  }

  /**
   * @return true iff sendEOM() has been called, but the eom has not been
   * flushed to the socket yet.
   */
  bool isEgressEOMQueued() const {
    return egressState_ == HTTPTransactionEgressSM::State::EOMQueued;
  }

  /**
   * @return true iff the egress EOM has been flushed to the socket.
   */
  bool isEgressComplete() const {
    return egressState_ == HTTPTransactionEgressSM::State::SendingDone;
  }

  /**
   * @return true iff the remote side initiated this transaction.
   */
  bool isRemoteInitiated() const {
    return (direction_ == TransportDirection::DOWNSTREAM && id_ % 2 == 1) ||
           (direction_ == TransportDirection::UPSTREAM && id_ % 2 == 0);
  }

  /**
   * @return true iff sendEOM() has been called.
   */
  bool isEgressEOMSeen() const {
    return isEgressEOMQueued() || isEgressComplete();
  }

  /**
   * @return true if we can send headers on this transaction
   *
   * Here's the logic:
   *  1) In start state (TODO: egress sm needs final/non-final)
   *  2) This is downstream AND state machine says sendHeaders is OK AND
   *   2a) this downstream has not sent a response
   *   2b) this downstream has only sent 1xx responses
   */
  virtual bool canSendHeaders() const {
    return (egressState_ == HTTPTransactionEgressSM::State::Start) ||
           (isDownstream() &&
            HTTPTransactionEgressSM::canTransit(
                egressState_, HTTPTransactionEgressSM::Event::sendHeaders) &&
            (lastResponseStatus_ == 0 || extraResponseExpected()));
  }

  /**
   * Send the egress message headers to the Transport. This method does
   * not actually write the message out on the wire immediately. All
   * writes happen at the end of the event loop at the earliest.
   * Note: This method should be called once per message unless the first
   * headers sent indicate a 1xx status.
   *
   * sendHeaders will not set EOM flag in header frame, whereas
   * sendHeadersWithEOM will. sendHeadersWithOptionalEOM backs both of them.
   *
   * @param headers  Message headers
   */
  virtual void sendHeaders(const HTTPMessage& headers);
  virtual void sendHeadersWithEOM(const HTTPMessage& headers);
  virtual void sendHeadersWithOptionalEOM(const HTTPMessage& headers, bool eom);

  /**
   * Experimental API
   *
   * Send the egress message header and a DSRRequestSender to the Transport.
   * Handler does NOT have to explicitly sendBody and sendEOM after this.
   */
  virtual bool sendHeadersWithDelegate(const HTTPMessage& headers,
                                       std::unique_ptr<DSRRequestSender>);

  /**
   * Send part or all of the egress message body to the Transport. If flow
   * control is enabled, the chunk boundaries may not be respected.
   * This method does not actually write the message out on the wire
   * immediately. All writes happen at the end of the event loop at the
   * earliest.
   * Note: This method may be called zero or more times per message.
   *
   * @param body Message body data; the Transport will take care of
   *             applying any necessary protocol framing, such as
   *             chunk headers.
   */
  virtual void sendBody(std::unique_ptr<folly::IOBuf> body);

  /**
   * Returns the cumulative size of body passed to sendBody so far
   */
  size_t bodyBytesSent() const {
    return actualResponseLength_.value_or(0);
  }

  /**
   * Write any protocol framing required for the subsequent call(s)
   * to sendBody(). This method does not actually write the message out on
   * the wire immediately. All writes happen at the end of the event loop
   * at the earliest.
   * @param length  Length in bytes of the body data to follow.
   */
  virtual void sendChunkHeader(size_t length) {
    if (!validateEgressStateTransition(
            HTTPTransactionEgressSM::Event::sendChunkHeader)) {
      return;
    }
    CHECK_EQ(deferredBufferMeta_.length, 0)
        << "Chunked-encoding doesn't support BufferMeta write";
    // TODO: move this logic down to session/codec
    if (!transport_.getCodec().supportsParallelRequests()) {
      chunkHeaders_.emplace_back(length);
    }
  }

  /**
   * Write any protocol syntax needed to terminate the data. This method
   * does not actually write the message out on the wire immediately. All
   * writes happen at the end of the event loop at the earliest.
   * Frame begun by the last call to sendChunkHeader().
   */
  virtual void sendChunkTerminator() {
    validateEgressStateTransition(
        HTTPTransactionEgressSM::Event::sendChunkTerminator);
    CHECK_EQ(deferredBufferMeta_.length, 0)
        << "Chunked-encoding doesn't support BufferMeta write";
  }

  /**
   * Send message trailers to the Transport. This method does
   * not actually write the message out on the wire immediately. All
   * writes happen at the end of the event loop at the earliest.
   * Note: This method may be called at most once per message.
   *
   * @param trailers  Message trailers.
   */
  virtual void sendTrailers(const HTTPHeaders& trailers) {
    if (!validateEgressStateTransition(
            HTTPTransactionEgressSM::Event::sendTrailers)) {
      return;
    }
    trailers_.reset(new HTTPHeaders(trailers));
  }

  /**
   * Finalize the egress message; depending on the protocol used
   * by the Transport, this may involve sending an explicit "end
   * of message" indicator. This method does not actually write the
   * message out on the wire immediately. All writes happen at the end
   * of the event loop at the earliest.
   *
   * If the ingress message also is complete, the transaction may
   * detach itself from the Handler and Transport and delete itself
   * as part of this method.
   *
   * Note: Either this method or sendAbort() should be called once
   *       per message.
   */
  virtual void sendEOM();

  /**
   * Terminate the transaction. Depending on the underlying protocol, this
   * may cause the connection to close or write egress bytes. This method
   * does not actually write the message out on the wire immediately. All
   * writes happen at the end of the event loop at the earliest.
   *
   * This function may also cause additional callbacks such as
   * detachTransaction() to the handler either immediately or after it returns.
   */
  virtual void sendAbort();

  /**
   * Pause ingress processing.  Upon pause, the HTTPTransaction
   * will call its Transport's pauseIngress() method.  The Transport
   * should make a best effort to stop invoking the HTTPTransaction's
   * onIngress* callbacks.  If the Transport does invoke any of those
   * methods while the transaction is paused, however, the transaction
   * will queue the ingress events and data and delay delivery to the
   * Handler until the transaction is unpaused.
   */
  virtual void pauseIngress();

  /**
   * Resume ingress processing. Only useful after a call to pauseIngress().
   */
  virtual void resumeIngress();

  /**
   * @return true iff ingress processing is paused for the handler
   */
  bool isIngressPaused() const {
    return ingressPaused_;
  }

  /**
   * Pause egress generation. HTTPTransaction may call its Handler's
   * onEgressPaused() method if there is a state change as a result of
   * this call.
   *
   * On receiving onEgressPaused(), the Handler should make a best effort
   * to stop invoking the HTTPTransaction's egress generating methods.  If
   * the Handler does invoke any of those methods while the transaction is
   * paused, however, the transaction will forward them anyway, unless it
   * is a body event. If flow control is enabled, body events will be
   * buffered for later transmission when egress is unpaused.
   */
  void pauseEgress();

  /**
   * Resume egress generation. The Handler's onEgressResumed() will not be
   * invoked if the HTTP/2 send window is full or there is too much
   * buffered egress data on this transaction already. In that case,
   * once the send window is not full or the buffer usage decreases, the
   * handler will finally get onEgressResumed().
   */
  void resumeEgress();

  /**
   * Specify a rate limit for egressing bytes.
   * The transaction will buffer extra bytes if doing so would cause it to go
   * over the specified rate limit.  Setting to a value of 0 will cause no
   * rate-limiting to occur.
   */
  void setEgressRateLimit(uint64_t bitsPerSecond);

  /**
   * @return true iff egress processing is paused for the handler
   */
  bool isEgressPaused() const {
    return handlerEgressPaused_;
  }

  /**
   * @return true iff egress processing is paused due to flow control
   * to the handler
   */
  bool isFlowControlPaused() const {
    return flowControlPaused_;
  }

  /**
   * @return true iff this transaction can be used to push resources to
   * the remote side.
   */
  bool supportsPushTransactions() const {
    return direction_ == TransportDirection::DOWNSTREAM &&
           transport_.getCodec().supportsPushTransactions();
  }

  /**
   * Create a new pushed transaction associated with this transaction,
   * and assign the given handler and priority.
   *
   * @return the new transaction for the push, or nullptr if a new push
   * transaction is impossible right now.
   */
  virtual HTTPTransaction* newPushedTransaction(
      HTTPPushTransactionHandler* handler, ProxygenError* error = nullptr) {
    if (isDelegated_) {
      LOG(ERROR)
          << "Creating Pushed transaction on a delegated HTTPTransaction "
          << "is not supported.";
      return nullptr;
    }

    if (isEgressEOMSeen()) {
      SET_PROXYGEN_ERROR_IF(error,
                            ProxygenError::kErrorEgressEOMSeenOnParentStream);
      return nullptr;
    }
    auto txn = transport_.newPushedTransaction(id_, handler, error);
    if (txn) {
      pushedTransactions_.insert(txn->getID());
    }
    return txn;
  }

  /**
   * Create a new extended transaction associated with this transaction,
   * and assign the given handler and priority.
   *
   * @return the new transaction for pubsub, or nullptr if a new push
   * transaction is impossible right now.
   */
  virtual HTTPTransaction* newExTransaction(HTTPTransactionHandler* handler,
                                            bool unidirectional = false) {
    if (isDelegated_) {
      LOG(ERROR) << "Creating ExTransaction on a delegated HTTPTransaction is "
                 << "not supported.";
      return nullptr;
    }
    auto txn = transport_.newExTransaction(handler, id_, unidirectional);
    if (txn) {
      exTransactions_.insert(txn->getID());
    }
    return txn;
  }

  /**
   * Invoked by the session (upstream only) when a new pushed transaction
   * arrives.  The txn's handler will be notified and is responsible for
   * installing a handler.  If no handler is installed in the callback,
   * the pushed transaction will be aborted.
   */
  bool onPushedTransaction(HTTPTransaction* txn);

  /**
   * Invoked by the session when a new ExTransaction arrives.  The txn's handler
   * will be notified and is responsible for installing a handler.  If no
   * handler is installed in the callback, the transaction will be aborted.
   */
  bool onExTransaction(HTTPTransaction* txn);

  /**
   * True if this transaction is a server push transaction
   */
  bool isPushed() const {
    return assocStreamId_.has_value();
  }

  bool isExTransaction() const {
    return exAttributes_.has_value();
  }

  bool isUnidirectional() const {
    return isExTransaction() && exAttributes_->unidirectional;
  }

  /**
   * @return true iff we should notify the error occured on EX_TXN
   * This logic only applies to EX_TXN with QoS 0
   */
  bool shouldNotifyExTxnError(HTTPException::Direction errorDirection) const {
    if (isUnidirectional()) {
      if (isRemoteInitiated()) {
        // We care about EGRESS errors in this case,
        // because we marked EGRESS state to be completed
        // If EGRESS error is happening, we need to know
        // Same for INGRESS direction, when EX_TXN is not remoteInitiated()
        return errorDirection == HTTPException::Direction::EGRESS;
      } else {
        return errorDirection == HTTPException::Direction::INGRESS;
      }
    }
    return false;
  }

  /**
   * Overrides the default idle timeout value.
   */
  void setIdleTimeout(std::chrono::milliseconds idleTimeout);

  bool hasIdleTimeout() const {
    return idleTimeout_.has_value();
  }

  /**
   * Returns the associated transaction ID for pushed transactions, 0 otherwise
   */
  folly::Optional<HTTPCodec::StreamID> getAssocTxnId() const {
    return assocStreamId_;
  }

  /**
   * Returns the control channel transaction ID for this transaction,
   * folly::none otherwise
   */
  folly::Optional<HTTPCodec::StreamID> getControlStream() const {
    return exAttributes_ ? exAttributes_->controlStream : HTTPCodec::NoStream;
  }

  /*
   * Returns attributes of EX stream (folly::none if not an EX transaction)
   */
  folly::Optional<HTTPCodec::ExAttributes> getExAttributes() const {
    return exAttributes_;
  }

  /**
   * Get a set of server-pushed transactions associated with this transaction.
   */
  const std::set<HTTPCodec::StreamID>& getPushedTransactions() const {
    return pushedTransactions_;
  }

  /**
   * Get a set of exTransactions associated with this transaction.
   */
  std::set<HTTPCodec::StreamID> getExTransactions() const {
    return exTransactions_;
  }

  /**
   * Remove the pushed txn ID from the set of pushed txns
   * associated with this txn.
   */
  void removePushedTransaction(HTTPCodec::StreamID pushStreamId) {
    pushedTransactions_.erase(pushStreamId);
  }

  /**
   * Remove the exTxn ID from the control stream txn.
   */
  void removeExTransaction(HTTPCodec::StreamID exStreamId) {
    exTransactions_.erase(exStreamId);
  }

  /**
   * Schedule or refresh the idle timeout for this transaction
   */
  void refreshTimeout() {
    // TODO(T121147568): Remove the zero-check after the experiment is complete.
    if (timer_ && hasIdleTimeout() &&
        idleTimeout_.value() != std::chrono::milliseconds::zero()) {
      timer_->scheduleTimeout(this, idleTimeout_.value());
    }
  }

  /**
   * Tests if the first byte has already been sent, and if it
   * hasn't yet then it marks it as sent.
   */
  bool testAndSetFirstByteSent() {
    bool ret = firstByteSent_;
    firstByteSent_ = true;
    return ret;
  }

  bool testAndClearIsCountedTowardsStreamLimit() {
    bool ret = isCountedTowardsStreamLimit_;
    isCountedTowardsStreamLimit_ = false;
    return ret;
  }

  void setIsCountedTowardsStreamLimit() {
    isCountedTowardsStreamLimit_ = true;
  }

  /**
   * Tests if the very first byte of Header has already been set.
   * If it hasn't yet, it marks it as sent.
   */
  bool testAndSetFirstHeaderByteSent() {
    bool ret = firstHeaderByteSent_;
    firstHeaderByteSent_ = true;
    return ret;
  }

  /**
   * HTTPTransaction will not detach until it has 0 pending byte events.  If
   * you call incrementPendingByteEvents, you must make a corresponding call
   * to decrementPendingByteEvents or the transaction will never be destroyed.
   */
  void incrementPendingByteEvents() {
    CHECK_LT(pendingByteEvents_,
             std::numeric_limits<decltype(pendingByteEvents_)>::max());
    pendingByteEvents_++;
  }

  void decrementPendingByteEvents() {
    DestructorGuard dg(this);
    CHECK_GT(pendingByteEvents_, 0);
    pendingByteEvents_--;
  }

  uint64_t getNumPendingByteEvents() const {
    return pendingByteEvents_;
  }

  /**
   * Timeout callback for this transaction.  The timer is active
   * until the ingress message is complete or terminated by error.
   */
  void timeoutExpired() noexcept override {
    transport_.transactionTimeout(this);
  }

  /**
   * Write a description of the transaction to a stream
   */
  void describe(std::ostream& os) const;

  /**
   * Change the priority of this transaction, may generate a PRIORITY frame.
   * The first variant is SPDY priority. The second is HTTP/2 priority. The
   * third one is a new proposal in a draft for both HTTP/2 and HTTP/3.
   */
  void updateAndSendPriority(int8_t newPriority);
  void updateAndSendPriority(const http2::PriorityUpdate& pri);
  virtual void updateAndSendPriority(HTTPPriority pri);

  /**
   * Notify of priority change, will not generate a PRIORITY frame
   */
  void onPriorityUpdate(const http2::PriorityUpdate& priority);

  /**
   * Add a callback waiting for this transaction to have a transport with
   * replay protection.
   */
  virtual void addWaitingForReplaySafety(
      folly::AsyncTransport::ReplaySafetyCallback* callback) {
    transport_.addWaitingForReplaySafety(callback);
  }

  /**
   * Remove a callback waiting for replay protection (if it was canceled).
   */
  virtual void removeWaitingForReplaySafety(
      folly::AsyncTransport::ReplaySafetyCallback* callback) {
    transport_.removeWaitingForReplaySafety(callback);
  }

  virtual bool needToBlockForReplaySafety() const {
    return transport_.needToBlockForReplaySafety();
  }

  int32_t getRecvToAck() const;

  void checkIfEgressRateLimitedByUpstream();

  const CompressionInfo& getCompressionInfo() const;

  bool hasPendingBody() const {
    return getOutstandingEgressBodyBytes() > 0;
  }

  size_t getOutstandingEgressBodyBytes() const {
    return deferredEgressBody_.chainLength() + deferredBufferMeta_.length;
  }

  void setLastByteFlushedTrackingEnabled(bool enabled) {
    enableLastByteFlushedTracking_ = enabled;
  }

  // Use this API to track TX or Ack for a particular offset of the HTTP body,
  // if the underlying transport is capable of tracking.
  // It will generate a callback to HTTPTransactionTransportCallback either
  // trackedByteEventTx or trackedByteEventAck.  The the event does not happen,
  // there is no cancellation callback.
  //
  // You can call this API before you have egressed the given bodyOffset.
  // Last byte ack is already tracked implicity and delivered via lastByteAcked.
  bool trackEgressBodyOffset(
      uint64_t bodyOffset,
      ByteEvent::EventFlags flags = ByteEvent::EventFlags::ACK);

  uint16_t getDatagramSizeLimit() const noexcept;
  virtual bool sendDatagram(std::unique_ptr<folly::IOBuf> datagram);

  folly::Optional<ConnectionToken> getConnectionToken() const noexcept;

  bool isWebTransportConnectStream() {
    return transport_.supportsWebTransport() && wtConnectStream_;
  }

  WebTransport* getWebTransport() {
    return isWebTransportConnectStream() ? &webTransport_ : nullptr;
  }

  static void setEgressBufferLimit(uint64_t limit) {
    egressBufferLimit_ = limit;
  }

  virtual bool addBufferMeta() noexcept;

 private:
  HTTPTransaction(const HTTPTransaction&) = delete;
  HTTPTransaction& operator=(const HTTPTransaction&) = delete;

  void dequeue() {
    CHECK(isEnqueued());
    egressQueue_.clearPendingEgress(queueHandle_);
  }

  bool delegatedTransactionChecks(const HTTPMessage& headers) noexcept;
  bool delegatedTransactionChecks() noexcept;

  void abortAndDeliverError(ErrorCode codecErorr, const std::string& msg);

  void onDelayedDestroy(bool delayed) override;

  /**
   * Invokes the handler's onEgressPaused/Resumed if the handler's pause
   * state needs updating
   */
  void updateHandlerPauseState();

  /**
   * Update the CompressionInfo (tableInfo_) struct
   */
  void updateEgressCompressionInfo(const CompressionInfo&);

  void updateIngressCompressionInfo(const CompressionInfo&);

  bool mustQueueIngress() const;

  /**
   * Check if deferredIngress_ points to some queue before pushing HTTPEvent
   * to it.
   */
  void checkCreateDeferredIngress();

  /**
   * Implementation of sending an abort for this transaction.
   */
  void sendAbort(ErrorCode statusCode);

  // Internal implementations of the ingress-related callbacks
  // that work whether the ingress events are immediate or deferred.
  void processIngressHeadersComplete(std::unique_ptr<HTTPMessage> msg);
  void processIngressBody(std::unique_ptr<folly::IOBuf> chain, size_t len);
  void processIngressChunkHeader(size_t length);
  void processIngressChunkComplete();
  void processIngressTrailers(std::unique_ptr<HTTPHeaders> trailers);
  void processIngressUpgrade(UpgradeProtocol protocol);
  void processIngressEOM();
  void processIngressError(const HTTPException& ex);

  void sendBodyFlowControlled(std::unique_ptr<folly::IOBuf> body = nullptr);
  size_t sendBodyNow(std::unique_ptr<folly::IOBuf> body,
                     size_t bodyLen,
                     bool eom);
  size_t sendEOMNow();
  void onDeltaSendWindowSize(int32_t windowDelta);

  void notifyTransportPendingEgress();

  size_t sendDeferredBody(uint32_t maxEgress);

  size_t sendDeferredBufferMeta(uint32_t maxEgress);

  bool maybeDelayForRateLimit();

  bool isEnqueued() const {
    return queueHandle_ ? queueHandle_->isEnqueued() : false;
  }

  // Whther the txn has a pending EOM that can be send out (i.e., no more body
  // bytes need to go before it.)
  bool hasPendingEOM() const {
    return isEgressEOMQueued() && getOutstandingEgressBodyBytes() == 0;
  }

  bool isExpectingIngress() const;

  bool isExpectingWindowUpdate() const;

  void updateReadTimeout();

  /**
   * Causes isIngressComplete() to return true, removes any queued
   * ingress, and cancels the read timeout.
   */
  void markIngressComplete();

  /**
   * Causes isEgressComplete() to return true, removes any queued egress,
   * and cancels the write timeout.
   */
  void markEgressComplete();

  /**
   * Validates the ingress state transition. Returns false and sends an
   * abort with INTERNAL_ERROR if the transition fails. Otherwise it
   * returns true.
   */
  bool validateIngressStateTransition(HTTPTransactionIngressSM::Event);

  /**
   * Validates the egress state transition.
   *
   * If the transition fails, it will invoke onInvariantViolation, and the
   * default implementation is to CHECK/crash.  If you have a custom
   * onInvariantViolation handler, this function can return false.
   */
  bool validateEgressStateTransition(HTTPTransactionEgressSM::Event);

  void invariantViolation(HTTPException ex);

  /**
   * Flushes any pending window updates.  This can happen from setReceiveWindow
   * or sendHeaders depending on transaction state.
   */
  void flushWindowUpdate();

  bool updateContentLengthRemaining(size_t len);

  void rateLimitTimeoutExpired();

  class RateLimitCallback : public folly::HHWheelTimer::Callback {
   public:
    explicit RateLimitCallback(HTTPTransaction& txn) : txn_(txn) {
    }

    void timeoutExpired() noexcept override {
      txn_.rateLimitTimeoutExpired();
    }
    void callbackCanceled() noexcept override {
      // no op
    }

   private:
    HTTPTransaction& txn_;
  };

  RateLimitCallback rateLimitCallback_{*this};

  /**
   * Queue to hold any events that we receive from the Transaction
   * while the ingress is supposed to be paused.
   */
  std::unique_ptr<std::queue<HTTPEvent>> deferredIngress_;

  /**
   * Queue to hold any body bytes to be sent out
   * while egress to the remote is supposed to be paused.
   */
  folly::IOBufQueue deferredEgressBody_{folly::IOBufQueue::cacheChainLength()};

  /**
   * BufferMeta queued at this HTTPTransaction to be sent to Transport.
   */
  BufferMeta deferredBufferMeta_;

  const TransportDirection direction_;
  HTTPTransactionEgressSM::State egressState_{
      HTTPTransactionEgressSM::getNewInstance()};
  HTTPTransactionIngressSM::State ingressState_{
      HTTPTransactionIngressSM::getNewInstance()};
  /**
   * bytes we need to acknowledge to the remote end using a window update
   */
  int32_t recvToAck_{0};

  HTTPCodec::StreamID id_;
  uint32_t seqNo_;
  uint32_t maxDeferredIngress_{0};
  Handler* handler_{nullptr};
  Transport& transport_;

  HTTPSessionStats* stats_{nullptr};

  CompressionInfo tableInfo_;

  /**
   * The recv window and associated data. This keeps track of how many
   * bytes we are allowed to buffer.
   */
  Window recvWindow_;

  /**
   * The send window and associated data. This keeps track of how many
   * bytes we are allowed to send and have outstanding.
   */
  Window sendWindow_;

  TransportCallback* transportCallback_{nullptr};

  /**
   * Trailers to send, if any.
   */
  std::unique_ptr<HTTPHeaders> trailers_;

  struct Chunk {
    explicit Chunk(size_t inLength) : length(inLength), headerSent(false) {
    }
    size_t length;
    bool headerSent;
  };
  std::list<Chunk> chunkHeaders_;

  /**
   * Reference to our priority queue
   */
  HTTP2PriorityQueueBase& egressQueue_;

  /**
   * Handle to our position in the priority queue.
   */
  HTTP2PriorityQueueBase::Handle queueHandle_{nullptr};

  /**
   * ID of request transaction (for pushed txns only)
   */
  folly::Optional<HTTPCodec::StreamID> assocStreamId_;

  /**
   * Attributes of http2 Ex_HEADERS
   */
  folly::Optional<HTTPCodec::ExAttributes> exAttributes_;

  /**
   * Set of all push transactions IDs associated with this transaction.
   */
  std::set<HTTPCodec::StreamID> pushedTransactions_;

  /**
   * Set of all exTransaction IDs associated with this transaction.
   */
  std::set<HTTPCodec::StreamID> exTransactions_;

  /**
   * Priority of this transaction
   */
  http2::PriorityUpdate priority_;

  /**
   * Information about this transaction's priority.
   *
   * insertDepth_ is the depth of this node in the tree when the txn was created
   * currentDepth_ is the depth of this node in the tree after the last
   *               onPriorityUpdate. It may not reflect its real position in
   *               realtime, since after the last onPriorityUpdate, it may get
   *               reparented as parent transactions complete.
   * cumulativeRatio_ / egressCalls_ is the average relative weight of this
   *                                 txn during egress
   */
  uint64_t insertDepth_{0};
  uint64_t currentDepth_{0};
  double cumulativeRatio_{0};
  uint64_t egressCalls_{0};

  uint64_t pendingByteEvents_{0};
  folly::Optional<uint64_t> expectedIngressContentLength_;
  folly::Optional<uint64_t> expectedIngressContentLengthRemaining_;
  folly::Optional<uint64_t> expectedResponseLength_;
  folly::Optional<uint64_t> actualResponseLength_{0};
  uint64_t bodyBytesEgressed_{0};
  std::map<uint64_t, ByteEvent::EventFlags> egressBodyOffsetsToTrack_;

  bool ingressPaused_ : 1;
  bool egressPaused_ : 1;
  bool flowControlPaused_ : 1;
  bool handlerEgressPaused_ : 1;
  bool egressRateLimited_ : 1;
  bool useFlowControl_ : 1;
  bool aborted_ : 1;
  bool deleting_ : 1;
  bool firstByteSent_ : 1;
  bool firstHeaderByteSent_ : 1;
  bool inResume_ : 1;
  bool isCountedTowardsStreamLimit_ : 1;
  bool ingressErrorSeen_ : 1;
  bool priorityFallback_ : 1;
  bool headRequest_ : 1;
  bool enableLastByteFlushedTracking_ : 1;
  bool wtConnectStream_ : 1;

  // Prevents the application from calling skipBodyTo() before egress
  // headers have been delivered.
  bool egressHeadersDelivered_ : 1;

  // Whether the HTTPTransaction has sent out a 1xx response HTTPMessage.
  bool has1xxResponse_ : 1;

  // Whether this HTTPTransaction delegates body sending to another entity.
  bool isDelegated_ : 1;

  /**
   * If this transaction represents a request (ie, it is backed by an
   * HTTPUpstreamSession) , this field indicates the last response status
   * received from the server. If this transaction represents a response,
   * this field indicates the last status we've sent. For instances, this
   * could take on multiple 1xx values, and then take on 200.
   */
  uint16_t lastResponseStatus_{0};

  // Maximum size of egress buffer before invoking onEgressPaused
  static uint64_t egressBufferLimit_;

  uint64_t egressLimitBytesPerMs_{0};
  proxygen::TimePoint startRateLimit_;
  uint64_t numLimitedBytesEgressed_{0};

  folly::Optional<std::chrono::milliseconds> idleTimeout_;

  folly::HHWheelTimer* timer_;

  // Keeps track for body offset processed so far.
  uint64_t ingressBodyOffset_{0};

  bool setIngressTimeoutAfterEom_{false};

  folly::Expected<Transport::FCState, WebTransport::ErrorCode>
  sendWebTransportStreamData(HTTPCodec::StreamID id,
                             std::unique_ptr<folly::IOBuf> data,
                             bool eof);

  folly::Expected<folly::Unit, WebTransport::ErrorCode> resetWebTransportEgress(
      HTTPCodec::StreamID id, uint32_t errorCode);

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  stopReadingWebTransportIngress(HTTPCodec::StreamID id, uint32_t errorCode);

  folly::Expected<WebTransport::BidiStreamHandle, WebTransport::ErrorCode>
  newWebTransportBidiStream();

  folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
  newWebTransportUniStream();

  class TxnWebTransport : public WebTransport {
   public:
    explicit TxnWebTransport(HTTPTransaction& txn) : txn_(txn) {
    }

    ~TxnWebTransport() override = default;

    folly::Expected<StreamWriteHandle*, WebTransport::ErrorCode>
    createUniStream() override {
      return txn_.newWebTransportUniStream();
    }

    folly::Expected<BidiStreamHandle, WebTransport::ErrorCode>
    createBidiStream() override {
      return txn_.newWebTransportBidiStream();
    }
    folly::SemiFuture<folly::Unit> awaitUniStreamCredit() override {
      // TODO
      return folly::makeFuture(folly::unit);
    }
    folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() override {
      // TODO
      return folly::makeFuture(folly::unit);
    }
    folly::Expected<folly::SemiFuture<StreamData>, WebTransport::ErrorCode>
    readStreamData(uint64_t id) override {
      auto it = txn_.wtIngressStreams_.find(id);
      if (it == txn_.wtIngressStreams_.end()) {
        return folly::makeUnexpected(
            WebTransport::ErrorCode::INVALID_STREAM_ID);
      }
      return it->second.readStreamData();
    }
    folly::Expected<folly::Unit, WebTransport::ErrorCode> stopSending(
        uint64_t id, uint32_t error) override {
      auto it = txn_.wtIngressStreams_.find(id);
      if (it == txn_.wtIngressStreams_.end()) {
        return folly::makeUnexpected(
            WebTransport::ErrorCode::INVALID_STREAM_ID);
      }
      return it->second.stopSending(error);
    }
    folly::Expected<folly::SemiFuture<folly::Unit>, ErrorCode> writeStreamData(
        uint64_t id, std::unique_ptr<folly::IOBuf> data, bool fin) override {
      auto it = txn_.wtEgressStreams_.find(id);
      if (it == txn_.wtEgressStreams_.end()) {
        return folly::makeUnexpected(
            WebTransport::ErrorCode::INVALID_STREAM_ID);
      }
      return it->second.writeStreamData(std::move(data), fin);
    }
    folly::Expected<folly::Unit, WebTransport::ErrorCode> resetStream(
        uint64_t id, uint32_t error) override {
      auto it = txn_.wtEgressStreams_.find(id);
      if (it == txn_.wtEgressStreams_.end()) {
        return folly::makeUnexpected(
            WebTransport::ErrorCode::INVALID_STREAM_ID);
      }
      return it->second.resetStream(error);
    }
    folly::Expected<folly::Unit, WebTransport::ErrorCode> sendDatagram(
        std::unique_ptr<folly::IOBuf> datagram) override {
      if (!txn_.sendDatagram(std::move(datagram))) {
        return folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR);
      }
      return folly::unit;
    }

    folly::Expected<folly::Unit, WebTransport::ErrorCode> closeSession(
        folly::Optional<uint32_t> /*error*/) override {
      // TODO: serialize error in a CLOSE_WEBTRANSPORT_SESSION capsule
      if (!txn_.isEgressEOMSeen()) {
        txn_.sendEOM();
      }
      return folly::unit;
    }

   private:
    HTTPTransaction& txn_;
  };

  class TxnStreamWriteHandle : public WebTransport::StreamWriteHandle {
   public:
    TxnStreamWriteHandle(HTTPTransaction& txn, HTTPCodec::StreamID id)
        : txn_(txn), id_(id) {
    }

    ~TxnStreamWriteHandle() override {
      cancellationSource_.requestCancellation();
    }

    folly::CancellationToken getCancelToken() override {
      return cancellationSource_.getToken();
    }

    uint64_t getID() override {
      return id_;
    }

    folly::Expected<folly::SemiFuture<folly::Unit>, WebTransport::ErrorCode>
    writeStreamData(std::unique_ptr<folly::IOBuf> data, bool fin) override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode> resetStream(
        uint32_t errorCode) override {
      return txn_.resetWebTransportEgress(id_, errorCode);
    }

    void onStopSending(uint32_t errorCode);

    void onEgressReady();

   private:
    HTTPTransaction& txn_;
    HTTPCodec::StreamID id_;
    folly::Optional<folly::Promise<folly::Unit>> writePromise_;
    folly::CancellationSource cancellationSource_;
  };

  class TxnStreamReadHandle : public WebTransport::StreamReadHandle {
   public:
    TxnStreamReadHandle(HTTPTransaction& txn, HTTPCodec::StreamID id)
        : txn_(txn), id_(id) {
    }

    ~TxnStreamReadHandle() override = default;

    uint64_t getID() override {
      return id_;
    }

    folly::CancellationToken getCancelToken() override {
      return cancellationSource_.getToken();
    }

    folly::SemiFuture<WebTransport::StreamData> readStreamData() override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode> stopSending(
        uint32_t error) override {
      return txn_.stopReadingWebTransportIngress(id_, error);
    }

    Transport::FCState dataAvailable(std::unique_ptr<folly::IOBuf> data,
                                     bool eof);
    void error(uint32_t error);
    [[nodiscard]] bool open() const {
      return !eof_ && !error_;
    }

   private:
    HTTPTransaction& txn_;
    HTTPCodec::StreamID id_;
    folly::Optional<folly::Promise<WebTransport::StreamData>> readPromise_;
    folly::IOBufQueue buf_{folly::IOBufQueue::cacheChainLength()};
    bool eof_{false};
    folly::Optional<uint32_t> error_;
    folly::CancellationSource cancellationSource_;
  };

  std::map<HTTPCodec::StreamID, TxnStreamWriteHandle> wtEgressStreams_;
  std::map<HTTPCodec::StreamID, TxnStreamReadHandle> wtIngressStreams_;
  TxnWebTransport webTransport_{*this};
};

/**
 * Write a description of an HTTPTransaction to an ostream
 */
std::ostream& operator<<(std::ostream& os, const HTTPTransaction& txn);

} // namespace proxygen
