/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/container/EvictingCacheMap.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/DelayedDestructionBase.h>
#include <folly/io/async/EventBase.h>
#include <folly/lang/Assume.h>
#include <proxygen/lib/http/codec/HQControlCodec.h>
#include <proxygen/lib/http/codec/HQUnidirectionalCodec.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Framer.h>
#include <proxygen/lib/http/codec/HTTPChecks.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <proxygen/lib/http/session/HQByteEventTracker.h>
#include <proxygen/lib/http/session/HQStreamBase.h>
#include <proxygen/lib/http/session/HQStreamDispatcher.h>
#include <proxygen/lib/http/session/HTTPSessionBase.h>
#include <proxygen/lib/http/session/HTTPSessionController.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/QuicProtocolInfo.h>
#include <proxygen/lib/http/session/ServerPushLifecycle.h>
#include <proxygen/lib/utils/ConditionalGate.h>
#include <quic/api/QuicSocket.h>
#include <quic/common/BufUtil.h>

namespace proxygen {

class HTTPSessionController;
class HQSession;

namespace hq {
class HQStreamCodec;
}

std::ostream& operator<<(std::ostream& os, const HQSession& session);

enum class HQVersion : uint8_t {
  H1Q_FB_V1, // HTTP1.1 on each stream, no control stream. For Interop only
  HQ,        // The real McCoy
};

extern const std::string kH3;
extern const std::string kH3AliasV1;
extern const std::string kHQ;
extern const std::string kH3FBCurrentDraft;
// TODO: Remove these constants, the session no longer negotiates these
extern const std::string kH3CurrentDraft;
extern const std::string kHQCurrentDraft;

// Default Priority Node
extern const proxygen::http2::PriorityUpdate hqDefaultPriority;

using HQVersionType = std::underlying_type<HQVersion>::type;

constexpr uint8_t kMaxDatagramHeaderSize = 16;
// Maximum number of datagrams to buffer per stream
constexpr uint8_t kDefaultMaxBufferedDatagrams = 5;
// Maximum number of streams with datagrams buffered
constexpr uint8_t kMaxStreamsWithBufferedDatagrams = 10;
// Maximum number of priority updates received when stream is not available
constexpr uint8_t kMaxBufferedPriorityUpdates = 10;

class HQSession
    : public quic::QuicSocket::ConnectionSetupCallback
    , public quic::QuicSocket::ConnectionCallback
    , public quic::QuicSocket::ReadCallback
    , public quic::QuicSocket::WriteCallback
    , public quic::QuicSocket::DeliveryCallback
    , public quic::QuicSocket::DatagramCallback
    , public quic::QuicSocket::PingCallback
    , public HTTPSessionBase
    , public folly::EventBase::LoopCallback
    , public HQUniStreamDispatcher::Callback
    , public HQBidiStreamDispatcher::Callback {
  // Forward declarations
 public:
  class HQStreamTransportBase;

 protected:
  class HQStreamTransport;

 private:
  class HQControlStream;

  static constexpr uint8_t kMaxCodecStackDepth = 3;

 public:
  folly::Optional<hq::PushId> getMaxAllowedPushId() {
    return maxAllowedPushId_;
  }

  void setServerPushLifecycleCallback(ServerPushLifecycleCallback* cb) {
    serverPushLifecycleCb_ = cb;
  }

  class ConnectCallback {
   public:
    virtual ~ConnectCallback() {
    }

    /**
     * This function is not terminal of the callback, downstream should expect
     * onReplaySafe to be invoked after connectSuccess.
     * onReplaySafe is invoked right after connectSuccess if zero rtt is not
     * attempted.
     * In zero rtt case, onReplaySafe might never be invoked if e.g. server
     * does not respond.
     */
    virtual void connectSuccess() {
      // Default empty implementation is provided in case downstream does not
      // attempt zero rtt data.
    }

    /**
     * Terminal callback.
     */
    virtual void onReplaySafe() = 0;

    /**
     * Terminal callback.
     */
    virtual void connectError(quic::QuicError code) = 0;

    /**
     * Callback for the first time transport has processed a packet from peer.
     */
    virtual void onFirstPeerPacketProcessed() {
    }
  };

  ~HQSession() override;

  HTTPTransaction::Transport::Type getType() const noexcept override {
    return HTTPTransaction::Transport::Type::QUIC;
  }

  void setSocket(std::shared_ptr<quic::QuicSocket> sock) noexcept {
    sock_ = sock;
    if (infoCallback_) {
      infoCallback_->onCreate(*this);
    }

    if (quicInfo_) {
      quicInfo_->transportSettings = sock_->getTransportSettings();
    }
  }

  void setForceUpstream1_1(bool force) {
    forceUpstream1_1_ = force;
  }
  void setStrictValidation(bool strictValidation) {
    strictValidation_ = strictValidation;
  }

  void setSessionStats(HTTPSessionStats* stats) override;

  void onNewBidirectionalStream(quic::StreamId id) noexcept override;

  void onNewUnidirectionalStream(quic::StreamId id) noexcept override;

  void onBidirectionalStreamsAvailable(
      uint64_t /*numStreamsAvailable*/) noexcept override;

  void onStopSending(quic::StreamId id,
                     quic::ApplicationErrorCode error) noexcept override;

  void onConnectionEnd() noexcept override;

  void onConnectionEnd(quic::QuicError error) noexcept override;

  void onConnectionSetupError(quic::QuicError code) noexcept override;

  void onConnectionError(quic::QuicError code) noexcept override;

  void onKnob(uint64_t knobSpace, uint64_t knobId, quic::Buf knobBlob) override;

  // returns false in case of failure
  bool onTransportReadyCommon() noexcept;

  void onReplaySafe() noexcept override;

  void onFlowControlUpdate(quic::StreamId id) noexcept override;

  // quic::QuicSocket::ReadCallback
  void readAvailable(quic::StreamId id) noexcept override;

  void readError(quic::StreamId id, quic::QuicError error) noexcept override;

  // quic::QuicSocket::WriteCallback
  void onConnectionWriteReady(uint64_t maxToSend) noexcept override;

  void onConnectionWriteError(quic::QuicError error) noexcept override;

  // quic::QuicSocket::DatagramCallback
  void onDatagramsAvailable() noexcept override;

  // quic::QuicSocket::PingCallback
  void pingAcknowledged() noexcept override {
    resetTimeout();
  }
  void pingTimeout() noexcept override {
  }
  void onPing() noexcept override {
    resetTimeout();
  }

  // Only for UpstreamSession
  HTTPTransaction* newTransaction(HTTPTransaction::Handler* handler) override;

  void startNow() override;

  void describe(std::ostream& os) const override {
    using quic::operator<<;
    os << "proto=" << alpn_;
    auto clientCid = (sock_ && sock_->getClientConnectionId())
                         ? *sock_->getClientConnectionId()
                         : quic::ConnectionId({0, 0, 0, 0});
    auto serverCid = (sock_ && sock_->getServerConnectionId())
                         ? *sock_->getServerConnectionId()
                         : quic::ConnectionId({0, 0, 0, 0});
    if (direction_ == TransportDirection::DOWNSTREAM) {
      os << ", UA=" << userAgent_ << ", client CID=" << clientCid
         << ", server CID=" << serverCid << ", downstream=" << getPeerAddress()
         << ", " << getLocalAddress() << "=local";
    } else {
      os << ", client CID=" << clientCid << ", server CID=" << serverCid
         << ", local=" << getLocalAddress() << ", " << getPeerAddress()
         << "=upstream";
    }
    os << ", drain=" << drainState_;
  }

  void onGoaway(uint64_t lastGoodStreamID,
                ErrorCode code,
                std::unique_ptr<folly::IOBuf> debugData = nullptr);

  void onSettings(const SettingsList& settings);

  void onPriority(quic::StreamId streamId, const HTTPPriority& pri);
  void onPushPriority(hq::PushId pushId, const HTTPPriority& pri);

  folly::AsyncTransport* getTransport() override {
    return nullptr;
  }

  folly::EventBase* getEventBase() const override {
    if (sock_) {
      return sock_->getEventBase();
    }
    return nullptr;
  }

  const folly::AsyncTransport* getTransport() const override {
    return nullptr;
  }

  bool hasActiveTransactions() const override {
    return getNumStreams() > 0;
  }

  uint32_t getNumStreams() const override {
    return getNumOutgoingStreams() + getNumIncomingStreams();
  }

  CodecProtocol getCodecProtocol() const override {
    return CodecProtocol::HTTP_3;
  }

  const TimePoint& getTransportStart() const {
    return transportStart_;
  }

  /**
   * Set flow control properties on an already started session.
   * QUIC requires both stream and connection flow control window sizes to be
   * specified in the initial transport handshake. Specifying
   * SETTINGS_INITIAL_WINDOW_SIZE in the SETTINGS frame is an error.
   *
   * @param initialReceiveWindow      (unused)
   * @param receiveStreamWindowSize   per-stream receive window for NEW streams;
   * @param receiveSessionWindowSize  per-session receive window;
   */
  void setFlowControl(size_t /* initialReceiveWindow */,
                      size_t receiveStreamWindowSize,
                      size_t receiveSessionWindowSize) override {
    if (sock_) {
      sock_->setConnectionFlowControlWindow(receiveSessionWindowSize);
    }
    receiveStreamWindowSize_ = (uint32_t)receiveStreamWindowSize;
    HTTPSessionBase::setReadBufferLimit((uint32_t)receiveSessionWindowSize);
  }

  /**
   * Set outgoing settings for this session
   */
  void setEgressSettings(const SettingsList& settings) override {
    for (const auto& setting : settings) {
      egressSettings_.setSetting(setting.id, setting.value);
    }
    const auto maxHeaderListSize =
        egressSettings_.getSetting(SettingsId::MAX_HEADER_LIST_SIZE);
    if (maxHeaderListSize) {
      versionUtilsReady_.then([this, size = maxHeaderListSize->value] {
        qpackCodec_.setMaxUncompressed(size);
      });
    }
    auto datagramEnabled = egressSettings_.getSetting(SettingsId::_HQ_DATAGRAM);
    auto datagramDraft8Enabled =
        egressSettings_.getSetting(SettingsId::_HQ_DATAGRAM_DRAFT_8);
    auto datagramRFCEnabled =
        egressSettings_.getSetting(SettingsId::_HQ_DATAGRAM_RFC);
    // if enabling H3 datagrams check that the transport supports datagrams
    if ((datagramEnabled && datagramEnabled->value) ||
        (datagramDraft8Enabled && datagramDraft8Enabled->value) ||
        (datagramRFCEnabled && datagramRFCEnabled->value)) {
      datagramEnabled_ = true;
    }
  }

  [[nodiscard]] bool supportsWebTransport() const {
    return supportsWebTransport_.all();
  }

  void setMaxConcurrentIncomingStreams(uint32_t /*num*/) override {
    // need transport API
  }

  /**
   * Send a settings frame
   */
  size_t sendSettings() override;

  /**
   * Causes a ping to be sent on the session. If the underlying protocol
   * doesn't support pings, this will return 0. Otherwise, it will return
   * the number of bytes written on the transport to send the ping.
   */
  size_t sendPing() override {
    sock_->sendPing(std::chrono::milliseconds(0));
    return 0;
  }

  size_t sendPing(uint64_t data) override {
    sock_->sendPing(std::chrono::milliseconds(data));
    return 0;
  }

  /**
   * Sends a knob frame on the session.
   */
  folly::Expected<folly::Unit, quic::LocalErrorCode> sendKnob(
      uint64_t knobSpace, uint64_t knobId, quic::Buf knobBlob) {
    return sock_->setKnob(knobSpace, knobId, std::move(knobBlob));
  }

  /**
   * Sends a priority message on this session.  If the underlying protocol
   * doesn't support priority, this is a no-op.  A new stream identifier will
   * be selected and returned.
   */
  HTTPCodec::StreamID sendPriority(http2::PriorityUpdate /*pri*/) override {
    return 0;
  }

  /**
   * As above, but updates an existing priority node.  Do not use for
   * real nodes, prefer HTTPTransaction::changePriority.
   */
  size_t sendPriority(HTTPCodec::StreamID /*id*/,
                      http2::PriorityUpdate /*pri*/) override {
    return 0;
  }

  size_t sendPriority(HTTPCodec::StreamID id, HTTPPriority pri);
  size_t sendPushPriority(hq::PushId pushId, HTTPPriority pri);

  /**
   * Get session-level transport info.
   * NOTE: The protocolInfo will be set to connection-level pointer.
   */
  bool getCurrentTransportInfo(wangle::TransportInfo* /*tinfo*/) override;

  /**
   *  Get session level AND stream level transport info.
   *  NOTE: the protocolInfo will be set to stream-level pointer.
   */
  bool getCurrentStreamTransportInfo(QuicStreamProtocolInfo* /*qspinfo*/,
                                     quic::StreamId /*streamId*/);

  bool connCloseByRemote() override {
    return false;
  }

  // From ManagedConnection
  void timeoutExpired() noexcept override;

  bool isBusy() const override {
    return getNumStreams() > 0;
  }
  void notifyPendingShutdown() override;
  void closeWhenIdle() override;
  void dropConnection(const std::string& errorMsg = "") override;
  void dumpConnectionState(uint8_t /*loglevel*/) override {
  }

  /*
   * dropConnectionSync drops the connection immediately.
   * This means that when invoked internally may need a destructor guard and
   * the socket will be invalid after it is invoked.
   *
   * errorCode is passed to transport CLOSE_CONNNECTION frame
   *
   * proxygenError is delivered to open transactions
   */
  void dropConnectionSync(quic::QuicError errorCode,
                          ProxygenError proxygenError);

  // Invokes dropConnectionSync at the beginning of the next loopCallback
  void dropConnectionAsync(quic::QuicError errorCode,
                           ProxygenError proxygenError);

  bool getCurrentTransportInfoWithoutUpdate(
      wangle::TransportInfo* /*tinfo*/) const override;

  void setHeaderCodecStats(HeaderCodec::Stats* stats) override {
    versionUtilsReady_.then([this, stats] { qpackCodec_.setStats(stats); });
  }

  void setHeaderIndexingStrategy(
      const HeaderIndexingStrategy* indexingStrat) override {
    versionUtilsReady_.then([this, indexingStrat] {
      qpackCodec_.setHeaderIndexingStrategy(indexingStrat);
    });
  }

  void enableDoubleGoawayDrain() override {
  }

  // Upstream interface
  bool isReusable() const override {
    VLOG(4) << __func__ << " sess=" << *this;
    return !isClosing();
  }

  bool isClosing() const override {
    VLOG(4) << __func__ << " sess=" << *this;
    return (drainState_ != DrainState::NONE || dropping_);
  }

  void drain() override {
    notifyPendingShutdown();
  }

  folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
      uint8_t /*level*/) override {
    return folly::none;
  }

  virtual quic::QuicSocket* getQuicSocket() const {
    return sock_.get();
  }

  // Override HTTPSessionBase address getter functions
  const folly::SocketAddress& getLocalAddress() const noexcept override {
    return sock_ && sock_->good() ? sock_->getLocalAddress() : localAddr_;
  }

  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return sock_ && sock_->good() ? sock_->getPeerAddress() : peerAddr_;
  }

  void enablePingProbes(std::chrono::seconds /*interval*/,
                        std::chrono::seconds /*timeout*/,
                        bool /*extendIntervalOnIngress*/,
                        bool /*immediate*/) override {
    // TODO
  }

 protected:
  // Finds any transport-like stream that has not been detached
  // by quic stream id
  HQStreamTransportBase* findNonDetachedStream(quic::StreamId streamId);

  //  Find any transport-like stream by quic stream id
  HQStreamTransportBase* findStream(quic::StreamId streamId);

  // Find any transport-like stream suitable for ingress (request/push-ingress)
  HQStreamTransportBase* findIngressStream(quic::StreamId streamId,
                                           bool includeDetached = false);
  // Find any transport-like stream suitable for egress (request/push-egress)
  HQStreamTransportBase* findEgressStream(quic::StreamId streamId,
                                          bool includeDetached = false);

  /**
   * The following functions invoke a callback on all or on all non-detached
   * request streams. It does an extra lookup per stream but it is safe. Note
   * that if the callback *adds* streams, they will not get the callback.
   */
  void invokeOnAllStreams(std::function<void(HQStreamTransportBase*)> fn) {
    invokeOnStreamsImpl(
        std::move(fn),
        [this](quic::StreamId id) { return this->findStream(id); },
        true);
  }

  void invokeOnEgressStreams(std::function<void(HQStreamTransportBase*)> fn,
                             bool includeDetached = false) {
    invokeOnStreamsImpl(std::move(fn),
                        [this, includeDetached](quic::StreamId id) {
                          return this->findEgressStream(id, includeDetached);
                        });
  }

  void invokeOnIngressStreams(std::function<void(HQStreamTransportBase*)> fn,
                              bool includeDetached = false) {
    invokeOnStreamsImpl(
        std::move(fn),
        [this, includeDetached](quic::StreamId id) {
          return this->findIngressStream(id, includeDetached);
        },
        true);
  }

  void invokeOnNonDetachedStreams(
      std::function<void(HQStreamTransportBase*)> fn) {
    invokeOnStreamsImpl(std::move(fn), [this](quic::StreamId id) {
      return this->findNonDetachedStream(id);
    });
  }

  virtual HQStreamTransportBase* findPushStream(quic::StreamId) = 0;

  virtual void findPushStreams(
      std::unordered_set<HQStreamTransportBase*>& streams) = 0;

  // Apply the function on the streams found by the two locators.
  // Note that same stream can be returned by a find-by-stream-id
  // and find-by-push-id locators.
  // This is mitigated by collecting the streams in an unordered set
  // prior to application of the funtion
  // Note that the function is allowed to delete a stream by invoking
  // erase stream, but the locators are not allowed to do so.
  // Note that neither the locators nor the function are allowed
  // to call "invokeOnStreamsImpl"
  void invokeOnStreamsImpl(
      std::function<void(HQStreamTransportBase*)> fn,
      std::function<HQStreamTransportBase*(quic::StreamId)> findByStreamIdFn,
      bool includePush = false) {
    DestructorGuard g(this);
    std::unordered_set<HQStreamTransportBase*> streams;
    streams.reserve(getNumStreams());

    for (const auto& txn : streams_) {
      HQStreamTransportBase* pstream = findByStreamIdFn(txn.first);
      if (pstream) {
        streams.insert(pstream);
      }
    }

    if (includePush) {
      findPushStreams(streams);
    }

    for (HQStreamTransportBase* pstream : streams) {
      CHECK(pstream);
      fn(pstream);
    }
  }

  // Erase the stream. Returns true if the stream
  // has been erased
  bool eraseStream(quic::StreamId);

  virtual bool erasePushStream(quic::StreamId streamId) = 0;

  void resumeReadsForPushStream(quic::StreamId streamId) {
    pendingProcessReadSet_.insert(streamId);
    resumeReads(streamId);
  }

  // Find a control stream by type
  HQControlStream* findControlStream(hq::UnidirectionalStreamType streamType);

  // Find a control stream by stream id (either ingress or egress)
  HQControlStream* findControlStream(quic::StreamId streamId);

  virtual HQStreamTransportBase* createIngressPushStream(quic::StreamId,
                                                         hq::PushId) {
    return nullptr;
  }

  virtual void eraseUnboundStream(HQStreamTransportBase*) {
  }

  virtual HTTPTransaction* newPushedTransaction(
      HTTPCodec::StreamID,           /* parentRequestStreamId */
      HTTPTransaction::PushHandler*, /* handler */
      ProxygenError*) {
    return nullptr;
  }

  // Callback methods that are invoked by the stream dispatchers
  void dispatchControlStream(quic::StreamId /* id */,
                             hq::UnidirectionalStreamType /* type */,
                             size_t /* toConsume */) override;

  void dispatchRequestStream(quic::StreamId /* streamId */) override;
  void dispatchRequestStreamImpl(quic::StreamId /* streamId */);

  std::chrono::milliseconds getDispatchTimeout() const override {
    return transactionsTimeout_;
  }

  void rejectStream(quic::StreamId /* id */) override;

  folly::Optional<hq::UnidirectionalStreamType> parseUniStreamPreface(
      uint64_t preface) override;

  folly::Optional<hq::BidirectionalStreamType> parseBidiStreamPreface(
      uint64_t preface) override {
    if (preface ==
        folly::to_underlying(hq::BidirectionalStreamType::WEBTRANSPORT)) {
      if (supportsWebTransport()) {
        return hq::BidirectionalStreamType::WEBTRANSPORT;
      } else {
        LOG(ERROR) << "WT stream when it is unsupported sess=" << *this;
        return folly::none;
      }
    }
    if (direction_ == TransportDirection::DOWNSTREAM) {
      return hq::BidirectionalStreamType::REQUEST;
    }
    return folly::none;
  }

  HQStreamTransportBase* FOLLY_NULLABLE
  findWTSessionOrAbort(quic::StreamId sessionID, quic::StreamId streamId);

  /**
   * HQSession is an HTTPSessionBase that uses QUIC as the underlying transport
   *
   * HQSession is an abstract base class and cannot be instantiated
   * directly. If you want to handle requests and send responses (act as a
   * server), construct a HQDownstreamSession. If you want to make
   * requests and handle responses (act as a client), construct a
   * HQUpstreamSession.
   */
  HQSession(const std::chrono::milliseconds transactionsTimeout,
            HTTPSessionController* controller,
            proxygen::TransportDirection direction,
            const wangle::TransportInfo& tinfo,
            InfoCallback* sessionInfoCb)
      : HTTPSessionBase(folly::SocketAddress(),
                        folly::SocketAddress(),
                        controller,
                        tinfo,
                        sessionInfoCb,
                        std::make_unique<HTTP1xCodec>(direction),
                        WheelTimerInstance(),
                        hq::kSessionStreamId),
        direction_(direction),
        transactionsTimeout_(transactionsTimeout),
        started_(false),
        dropping_(false),
        inLoopCallback_(false),
        unidirectionalReadDispatcher_(*this, direction),
        bidirectionalReadDispatcher_(*this, direction),
        controlStreamReadCallback_(*this),
        sessionObserverAccessor_(this),
        sessionObserverContainer_(&sessionObserverAccessor_) {
    codec_.add<HTTPChecks>();
    // dummy, ingress, egress
    codecStack_.reserve(kMaxCodecStackDepth);
    codecStack_.emplace_back(nullptr, nullptr, nullptr);

    attachToSessionController();
    nextEgressResults_.reserve(maxConcurrentIncomingStreams_);
    quicInfo_ = std::make_shared<QuicProtocolInfo>();
    initCodecHeaderIndexingStrategy();
  }

  // EventBase::LoopCallback methods
  void runLoopCallback() noexcept override;

  /**
   * Called by transactionTimeout if the transaction has no handler.
   */
  virtual HTTPTransaction::Handler* getTransactionTimeoutHandler(
      HTTPTransaction* txn) = 0;

  /**
   * Called by onHeadersComplete(). This function allows downstream and
   * upstream to do any setup (like preparing a handler) when headers are
   * first received from the remote side on a given transaction.
   */
  virtual void setupOnHeadersComplete(HTTPTransaction* txn,
                                      HTTPMessage* msg) = 0;

  /**
   * Executed on connection setup failure.
   */
  virtual void onConnectionSetupErrorHandler(
      quic::QuicError error) noexcept = 0;

  void applySettings(const SettingsList& settings);

  virtual void connectSuccess() noexcept {
  }

  bool isPeerUniStream(quic::StreamId id) {
    return sock_->isUnidirectionalStream(id) &&
           ((direction_ == TransportDirection::DOWNSTREAM &&
             sock_->isClientStream(id)) ||
            (direction_ == TransportDirection::UPSTREAM &&
             sock_->isServerStream(id)));
  }

  bool isSelfUniStream(quic::StreamId id) {
    return sock_->isUnidirectionalStream(id) &&
           ((direction_ == TransportDirection::DOWNSTREAM &&
             sock_->isServerStream(id)) ||
            (direction_ == TransportDirection::UPSTREAM &&
             sock_->isClientStream(id)));
  }

  void abortStream(HTTPException::Direction dir,
                   quic::StreamId id,
                   HTTP3::ErrorCode err);

  // Get extra HTTP headers we want to add to the HTTPMessage in sendHeaders.
  virtual folly::Optional<HTTPHeaders> getExtraHeaders(const HTTPMessage&,
                                                       quic::StreamId) {
    return folly::none;
  }

  proxygen::TransportDirection direction_;
  std::chrono::milliseconds transactionsTimeout_;
  TimePoint transportStart_;

  std::shared_ptr<quic::QuicSocket> sock_;

  // Callback pointer used for correctness testing. Not used
  // for session logic.
  ServerPushLifecycleCallback* serverPushLifecycleCb_{nullptr};

 private:
  std::unique_ptr<HTTPCodec> createStreamCodec(quic::StreamId streamId);

  // Creates a request stream. All streams that are not control streams
  // or Push streams are request streams.
  HQStreamTransport* createStreamTransport(quic::StreamId streamId);

  bool createEgressControlStreams();

  // Creates outgoing control stream.
  bool createEgressControlStream(hq::UnidirectionalStreamType streamType);

  // Creates incoming control stream
  HQControlStream* createIngressControlStream(
      quic::StreamId id, hq::UnidirectionalStreamType streamType);

  virtual void cleanupUnboundPushStreams(std::vector<quic::StreamId>&) {
  }

  // gets the ALPN from the transport and returns whether the protocol is
  // supported. Drops the connection if not supported
  bool getAndCheckApplicationProtocol();

  // Use ALPN to set the correct version utils strategy.
  void setVersionUtils();

  // Used during 2-phased GOAWAY messages, and EOF sending.
  void onDeliveryAck(quic::StreamId id,
                     uint64_t offset,
                     std::chrono::microseconds rtt) override;

  void onCanceled(quic::StreamId id, uint64_t offset) override;

  // helper functions for reads
  void readRequestStream(quic::StreamId id) noexcept;
  void readControlStream(HQControlStream* controlStream);

  // Runs the codecs on all request streams that have received data
  // during the last event loop
  void processReadData();

  // Pausing reads prevents the read callback to be invoked on the stream
  void resumeReads(quic::StreamId id);

  // Resume all ingress transactions
  void resumeReads();

  // Resuming the reads allows the read callback to be involved
  void pauseReads(quic::StreamId id);

  // Pause all ingress transactions
  void pauseReads();

  void notifyEgressBodyBuffered(int64_t bytes);

  // The max allowed push id value. Value folly::none indicates that
  // a. For downstream session: MAX_PUSH_ID has not been received
  // b. For upstream session: MAX_PUSH_ID has been explicitly set to none
  // In both cases, maxAllowedPushId_ == folly::none means that no push id
  // is allowed. Default to kEightByteLimit assuming this session will
  // be using push.
  folly::Optional<hq::PushId> maxAllowedPushId_{folly::none};

  // Schedule the loop callback.
  // To keep this consistent with EventBase::runInLoop run in the next loop
  // by default
  void scheduleLoopCallback(bool thisIteration = false);

  // helper functions for writes
  uint64_t writeRequestStreams(uint64_t maxEgress) noexcept;
  void scheduleWrite();
  void handleWriteError(HQStreamTransportBase* hqStream,
                        quic::QuicErrorCode err);

  /**
   * Handles the write to the socket and errors for a request stream.
   * Returns the number of bytes written from data.
   */
  template <typename WriteFunc, typename DataType>
  size_t handleWrite(WriteFunc writeFunc,
                     HQStreamTransportBase* hqStream,
                     DataType dataType,
                     size_t dataChainLen,
                     bool sendEof);

  /**
   * Helper function to perform writes on a single request stream
   * The first argument defines whether the implementation should
   * call onWriteReady on the transaction to get data allocated
   * in the write buffer.
   * Returns the number of bytes written to the transport
   */
  uint64_t requestStreamWriteImpl(HQStreamTransportBase* hqStream,
                                  uint64_t maxEgress,
                                  double ratio);

  uint64_t writeControlStreams(uint64_t maxEgress);
  uint64_t controlStreamWriteImpl(HQControlStream* ctrlStream,
                                  uint64_t maxEgress);
  void handleSessionError(HQStreamBase* stream,
                          hq::StreamDirection streamDir,
                          quic::QuicErrorCode err,
                          ProxygenError proxygenError);

  void detachStreamTransport(HQStreamTransportBase* hqStream);

  void drainImpl();

  void checkForShutdown();
  void onGoawayAck();
  quic::StreamId getGoawayStreamId();

  void errorOnTransactionId(quic::StreamId id, HTTPException ex);

  /**
   * Shared implementation of "findXXXstream" methods
   */
  HQStreamTransportBase* findStreamImpl(quic::StreamId streamId,
                                        bool includeEgress = true,
                                        bool includeIngress = true,
                                        bool includeDetached = true);

  /**
   * Shared implementation of "numberOfXXX" methods
   */
  uint32_t countStreamsImpl(bool includeEgress = true,
                            bool includeIngress = true) const;

  std::list<folly::AsyncTransport::ReplaySafetyCallback*>
      waitingForReplaySafety_;

  /**
   * With HTTP/1.1 codecs, graceful shutdown happens when the session has sent
   * and received a Connection: close header, and all streams have completed.
   *
   * The application can signal intent to drain by calling notifyPendingShutdown
   * (or its alias, drain).  The peer can signal intent to drain by including
   * a Connection: close header.
   *
   * closeWhenIdle will bypass the requirement to send/receive Connection:
   * close, and the socket will terminate as soon as the stream count reaches 0.
   *
   * dropConnection will forcibly close all streams and guarantee that the
   * HQSession has been deleted before exiting.
   *
   * The intent is that an application will first notifyPendingShutdown() all
   * open sessions.  Then after some period of time, it will call closeWhenIdle.
   * As a last resort, it will call dropConnection.
   *
   * Note we allow the peer to create streams after draining because of out
   * of order delivery.
   *
   * drainState_ tracks the progress towards shutdown.
   *
   *  NONE - no shutdown requested
   *  PENDING - shutdown requested but no Connection: close seen
   *  CLOSE_SENT - sent Connection: close but not received
   *  CLOSE_RECEIVED - received Connection: close but not sent
   *  DONE - sent and received Connection: close.
   *
   *  NONE ---> PENDING ---> CLOSE_SENT --+--> DONE
   *    |          |                      |
   *    +----------+-------> CLOSE_RECV --+
   *
   * For sessions with a control stream shutdown is driven by GOAWAYs.
   * Only the server can send GOAWAYs so the behavior is asymmetric between
   * upstream and downstream
   *
   *  NONE - no shutdown requested
   *  PENDING - shutdown requested but no GOAWAY sent/received yet
   *  FIRST_GOAWAY - first GOAWAY received/sent
   *  SECOND_GOAWAY - downstream only - second GOAWAY sent
   *  DONE - two GOAWAYs sent/received. can close when request streams are done
   *
   */
  enum DrainState : uint8_t {
    NONE = 0,
    PENDING = 1,
    CLOSE_SENT = 2,
    CLOSE_RECEIVED = 3,
    FIRST_GOAWAY = 4,
    SECOND_GOAWAY = 5,
    DONE = 6
  };

  DrainState drainState_{DrainState::NONE};
  bool started_ : 1;
  bool dropping_ : 1;
  bool inLoopCallback_ : 1;
  folly::Optional<std::pair<quic::QuicError, ProxygenError>> dropInNextLoop_;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250) // inherits 'proxygen::detail::..' via dominance
#endif

  // A control stream is created as egress first, then the ingress counterpart
  // is linked as soon as we read the stream preface on the associated stream
  class HQControlStream
      : public detail::composite::CSBidir
      , public HQStreamBase
      , public hq::HQUnidirectionalCodec::Callback
      , public quic::QuicSocket::DeliveryCallback {
   public:
    HQControlStream() = delete;
    HQControlStream(HQSession& session,
                    quic::StreamId egressStreamId,
                    hq::UnidirectionalStreamType type)
        : detail::composite::CSBidir(egressStreamId, folly::none),
          HQStreamBase(session, session.codec_, type) {
      createEgressCodec();
    }

    void createEgressCodec() {
      CHECK(type_.has_value());
      switch (*type_) {
        case hq::UnidirectionalStreamType::CONTROL:
          realCodec_ =
              std::make_unique<hq::HQControlCodec>(getEgressStreamId(),
                                                   session_.direction_,
                                                   hq::StreamDirection::EGRESS,
                                                   session_.egressSettings_,
                                                   *type_);
          break;
        case hq::UnidirectionalStreamType::QPACK_ENCODER:
        case hq::UnidirectionalStreamType::QPACK_DECODER:
          // These are statically allocated in the session
          break;
        default:
          LOG(FATAL)
              << "Failed to create egress codec."
              << " unrecognized stream type=" << static_cast<uint64_t>(*type_);
      }
    }

    void setIngressCodec(std::unique_ptr<hq::HQUnidirectionalCodec> codec) {
      ingressCodec_ = std::move(codec);
    }

    void processReadData();

    // QuicSocket::DeliveryCallback
    void onDeliveryAck(quic::StreamId id,
                       uint64_t offset,
                       std::chrono::microseconds rtt) override;
    void onCanceled(quic::StreamId id, uint64_t offset) override;

    // HTTPCodec::Callback
    void onMessageBegin(HTTPCodec::StreamID /*stream*/,
                        HTTPMessage* /*msg*/) override {
      LOG(FATAL) << __func__ << " called on a Control Stream.";
    }

    void onHeadersComplete(HTTPCodec::StreamID /*stream*/,
                           std::unique_ptr<HTTPMessage> /*msg*/) override {
      LOG(FATAL) << __func__ << " called on a Control Stream.";
    }

    void onBody(HTTPCodec::StreamID /*stream*/,
                std::unique_ptr<folly::IOBuf> /*chain*/,
                uint16_t /*padding*/) override {
      LOG(FATAL) << __func__ << " called on a Control Stream.";
    }

    void onTrailersComplete(
        HTTPCodec::StreamID /*stream*/,
        std::unique_ptr<HTTPHeaders> /*trailers*/) override {
      LOG(FATAL) << __func__ << " called on a Control Stream.";
    }

    void onMessageComplete(HTTPCodec::StreamID /*stream*/,
                           bool /*upgrade*/) override {
      LOG(FATAL) << __func__ << " called on a Control Stream.";
    }

    void onError(HTTPCodec::StreamID /*stream*/,
                 const HTTPException& /*error*/,
                 bool /* newTxn */ = false) override;

    void onGoaway(uint64_t lastGoodStreamID,
                  ErrorCode code,
                  std::unique_ptr<folly::IOBuf> debugData = nullptr) override {
      session_.onGoaway(lastGoodStreamID, code, std::move(debugData));
    }

    void onSettings(const SettingsList& settings) override {
      session_.onSettings(settings);
    }

    void onPriority(HTTPCodec::StreamID id, const HTTPPriority& pri) override {
      session_.onPriority(id, pri);
    }

    void onPushPriority(HTTPCodec::StreamID id,
                        const HTTPPriority& pri) override {
      session_.onPushPriority(id, pri);
    }

    std::unique_ptr<hq::HQUnidirectionalCodec> ingressCodec_;
    bool readEOF_{false};
  }; // HQControlStream

  // Callback for the control stream - follows the read api
  struct ControlStreamReadCallback : public quic::QuicSocket::ReadCallback {
    explicit ControlStreamReadCallback(HQSession& session) : session_(session) {
    }
    ~ControlStreamReadCallback() override = default;
    void readAvailable(quic::StreamId id) noexcept override {
      session_.controlStreamReadAvailable(id);
    }
    void readError(quic::StreamId id, quic::QuicError error) noexcept override {
      session_.controlStreamReadError(id, error);
    }

   protected:
    HQSession& session_;
  };

 public:
  class HQStreamTransportBase
      : public HQStreamBase
      , public HTTPTransaction::Transport
      , public HTTP2PriorityQueueBase
      , public quic::QuicSocket::ByteEventCallback {
   protected:
    HQStreamTransportBase(
        HQSession& session,
        TransportDirection direction,
        quic::StreamId streamId,
        uint32_t seqNo,
        const WheelTimerInstance& wheelTimer,
        HTTPSessionStats* stats = nullptr,
        http2::PriorityUpdate priority = hqDefaultPriority,
        folly::Optional<HTTPCodec::StreamID> parentTxnId = HTTPCodec::NoStream,
        folly::Optional<hq::UnidirectionalStreamType> type = folly::none);

    void initCodec(std::unique_ptr<HTTPCodec> /* codec */,
                   const std::string& /* where */);

    void initIngress(const std::string& /* where */);

    HTTPSessionBase* getHTTPSessionBase() override {
      return &(getSession());
    }

   public:
    HQStreamTransportBase() = delete;

    bool hasCodec() const {
      return hasCodec_;
    }

    bool hasIngress() const {
      return hasIngress_;
    }

    // process data in the read buffer, returns true if the codec is blocked
    bool processReadData();

    // Process data from QUIC onDataAvailable callback.
    void processPeekData(
        const folly::Range<quic::QuicSocket::PeekIterator>& peekData);

    // QuicSocket::DeliveryCallback
    void onByteEvent(quic::QuicSocket::ByteEvent byteEvent) override;
    void onByteEventCanceled(
        quic::QuicSocket::ByteEventCancellation cancellation) override;

    // HTTPCodec::Callback methods
    void onMessageBegin(HTTPCodec::StreamID streamID,
                        HTTPMessage* /* msg */) override;

    void onPushMessageBegin(HTTPCodec::StreamID /* pushID */,
                            HTTPCodec::StreamID /* parentTxnId */,
                            HTTPMessage* /* msg */) override;

    void onExMessageBegin(HTTPCodec::StreamID /* streamID */,
                          HTTPCodec::StreamID /* controlStream */,
                          bool /* unidirectional */,
                          HTTPMessage* /* msg */) override {
      LOG(ERROR) << "exMessage: txn=" << txn_ << " TODO";
    }

    virtual void onPushPromiseHeadersComplete(
        hq::PushId /* pushID */,
        HTTPCodec::StreamID /* assoc streamID */,
        std::unique_ptr<HTTPMessage> /* msg */) {
      LOG(ERROR) << "push promise: txn=" << txn_ << " TODO";
    }

    void onHeadersComplete(HTTPCodec::StreamID streamID,
                           std::unique_ptr<HTTPMessage> msg) override;

    void onBody(HTTPCodec::StreamID /* streamID */,
                std::unique_ptr<folly::IOBuf> chain,
                uint16_t padding) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      CHECK(chain);
      auto len = chain->computeChainDataLength();
      if (session_.onBodyImpl(std::move(chain), len, padding, &txn_)) {
        session_.pauseReads();
      };
    }

    void onChunkHeader(HTTPCodec::StreamID /* stream */,
                       size_t length) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      txn_.onIngressChunkHeader(length);
    }

    void onChunkComplete(HTTPCodec::StreamID /* stream */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      txn_.onIngressChunkComplete();
    }

    void onTrailersComplete(HTTPCodec::StreamID /* streamID */,
                            std::unique_ptr<HTTPHeaders> trailers) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      txn_.onIngressTrailers(std::move(trailers));
    }

    void onMessageComplete(HTTPCodec::StreamID /* streamID */,
                           bool /* upgrade */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      // for 1xx responses (excluding 101) onMessageComplete may be called
      // more than once
      if (txn_.isUpstream() && txn_.extraResponseExpected()) {
        return;
      }
      if (session_.infoCallback_) {
        session_.infoCallback_->onRequestEnd(session_,
                                             txn_.getMaxDeferredSize());
      }
      // Pause the parser, which will prevent more than one message from being
      // processed
      auto g = folly::makeGuard(setActiveCodec(__func__));
      codecFilterChain->setParserPaused(true);
      eomGate_.set(EOMType::CODEC);
    }

    void onIngressEOF() {
      // Can only call this once
      CHECK(!eomGate_.get(EOMType::TRANSPORT));
      if (ingressError_) {
        // This codec has already errored, no need to give it more input
        return;
      }
      auto g = folly::makeGuard(setActiveCodec(__func__));
      codecFilterChain->onIngressEOF();
      eomGate_.set(EOMType::TRANSPORT);
    }

    void onError(HTTPCodec::StreamID streamID,
                 const HTTPException& error,
                 bool newTxn) override;

    // Invoked when we get a RST_STREAM from the transport
    void onResetStream(HTTP3::ErrorCode error, HTTPException ex);

    void onAbort(HTTPCodec::StreamID /* streamID */,
                 ErrorCode /* code */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      // Can't really get here since no HQ codecs can produce aborts.
      // The entry point is onResetStream via readError()
      LOG(DFATAL) << "Unexpected abort";
    }

    void onFrameHeader(HTTPCodec::StreamID /* stream_id */,
                       uint8_t /* flags */,
                       uint64_t /* length */,
                       uint64_t /* type */,
                       uint16_t /* version */ = 0) override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void onGoaway(
        uint64_t /* lastGoodStreamID */,
        ErrorCode /* code */,
        std::unique_ptr<folly::IOBuf> /* debugData */ = nullptr) override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void onPingRequest(uint64_t /* data */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void onPingReply(uint64_t /* data */) override {
      // This method should not get called
      LOG(FATAL) << __func__ << " txn=" << txn_;
    }

    void onWindowUpdate(HTTPCodec::StreamID /* stream */,
                        uint32_t /* amount */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void onSettings(const SettingsList& /*settings*/) override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void onSettingsAck() override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void onPriority(HTTPCodec::StreamID /* stream */,
                    const HTTPMessage::HTTP2Priority& /* priority */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    bool onNativeProtocolUpgrade(HTTPCodec::StreamID /* stream */,
                                 CodecProtocol /* protocol */,
                                 const std::string& /* protocolString */,
                                 HTTPMessage& /* msg */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return false;
    }

    uint32_t numOutgoingStreams() const override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return 0;
    }

    uint32_t numIncomingStreams() const override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return 0;
    }

    // HTTPTransaction::Transport methods

    // For parity with H2, pause/resumeIngress now a no-op.  All transactions
    // will pause when total buffered egress exceeds the configured limit, which
    // should be equal to the recv flow control window
    void pauseIngress(HTTPTransaction* /* txn */) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void resumeIngress(HTTPTransaction* /* txn */) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    void transactionTimeout(HTTPTransaction* /* txn */) noexcept override;

    void sendHeaders(HTTPTransaction* txn,
                     const HTTPMessage& headers,
                     HTTPHeaderSize* size,
                     bool includeEOM) noexcept override;

    bool sendHeadersWithDelegate(
        HTTPTransaction* txn,
        const HTTPMessage& headers,
        HTTPHeaderSize* size,
        size_t* dataFrameHeaderSize,
        uint64_t contentLength,
        std::unique_ptr<DSRRequestSender> dsrSender) noexcept override;

    size_t sendBody(HTTPTransaction* txn,
                    std::unique_ptr<folly::IOBuf> body,
                    bool includeEOM,
                    bool trackLastByteFlushed) noexcept override;

    size_t sendBody(HTTPTransaction* txn,
                    const HTTPTransaction::BufferMeta& body,
                    bool eom) noexcept override;

    size_t sendChunkHeader(HTTPTransaction* txn,
                           size_t length) noexcept override;

    size_t sendChunkTerminator(HTTPTransaction* txn) noexcept override;

    size_t sendEOM(HTTPTransaction* txn,
                   const HTTPHeaders* trailers) noexcept override;

    size_t sendAbort(HTTPTransaction* txn,
                     ErrorCode statusCode) noexcept override;

    size_t sendAbortImpl(HTTP3::ErrorCode errorCode, std::string errorMsg);

    size_t sendPriority(
        HTTPTransaction* /* txn */,
        const http2::PriorityUpdate& /* pri */) noexcept override;
    size_t changePriority(HTTPTransaction* txn,
                          HTTPPriority pri) noexcept override;

    size_t sendWindowUpdate(HTTPTransaction* /* txn */,
                            uint32_t /* bytes */) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      CHECK(hasEgressStreamId())
          << __func__ << " invoked on stream without egress";
      return 0;
    }

    // Send a push promise. Has different implementations in
    // request streams / push streams
    virtual void sendPushPromise(HTTPTransaction* /* txn */,
                                 folly::Optional<hq::PushId> /* pushId */,
                                 const HTTPMessage& /* headers */,
                                 HTTPHeaderSize* /* outSize */,
                                 bool /* includeEOM */) {
      VLOG(4) << __func__ << " txn=" << txn_;
      CHECK(hasEgressStreamId())
          << __func__ << " invoked on stream without egress";
    }

    void notifyPendingEgress() noexcept override;

    void detach(HTTPTransaction* /* txn */) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      detached_ = true;
      session_.scheduleLoopCallback();
    }
    void checkForDetach();

    void notifyIngressBodyProcessed(uint32_t bytes) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      if (session_.notifyBodyProcessed(bytes)) {
        session_.resumeReads();
      }
    }

    void notifyEgressBodyBuffered(int64_t bytes) noexcept override {
      session_.notifyEgressBodyBuffered(bytes);
    }

    const folly::SocketAddress& getLocalAddress() const noexcept override {
      return session_.getLocalAddress();
    }

    const folly::SocketAddress& getPeerAddress() const noexcept override {
      return session_.getPeerAddress();
    }

    void describe(std::ostream& os) const override {
      session_.describe(os);
    }

    const wangle::TransportInfo& getSetupTransportInfo()
        const noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return session_.transportInfo_;
    }

    [[nodiscard]] std::chrono::seconds getLatestIdleTime() const override {
      return session_.getLatestIdleTime();
    }

    bool getCurrentTransportInfo(wangle::TransportInfo* tinfo) override;

    void getFlowControlInfo(
        HTTPTransaction::FlowControlInfo* /*info*/) override {
      // Not implemented
    }

    HTTPTransaction::Transport::Type getSessionType() const noexcept override;

    virtual const HTTPCodec& getCodec() const noexcept override {
      return HQStreamBase::getCodec();
    }

    void drain() override {
      VLOG(4) << __func__ << " txn=" << txn_;
    }

    bool isDraining() const override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return false;
    }

    HTTPTransaction* newPushedTransaction(
        HTTPCodec::StreamID /* parentTxnId */,
        HTTPTransaction::PushHandler* /* handler */,
        ProxygenError* /* error */ = nullptr) noexcept override {
      LOG(FATAL) << __func__ << " Only available via request stream";
      folly::assume_unreachable();
    }

    HTTPTransaction* newExTransaction(
        HTTPTransactionHandler* /* handler */,
        HTTPCodec::StreamID /* controlStream */,
        bool /* unidirectional */) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return nullptr;
    }

    std::string getSecurityProtocol() const override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return "quic/tls1.3";
    }

    void addWaitingForReplaySafety(folly::AsyncTransport::ReplaySafetyCallback*
                                       callback) noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      if (session_.sock_->replaySafe()) {
        callback->onReplaySafe();
      } else {
        session_.waitingForReplaySafety_.push_back(callback);
      }
    }

    void removeWaitingForReplaySafety(
        folly::AsyncTransport::ReplaySafetyCallback* callback) noexcept
        override {
      VLOG(4) << __func__ << " txn=" << txn_;
      session_.waitingForReplaySafety_.remove(callback);
    }

    bool needToBlockForReplaySafety() const override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return false;
    }

    const folly::AsyncTransport* getUnderlyingTransport()
        const noexcept override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return nullptr;
    }

    bool isReplaySafe() const override {
      return session_.isReplaySafe();
    }

    void setHTTP2PrioritiesEnabled(bool /* enabled */) override {
    }
    bool getHTTP2PrioritiesEnabled() const override {
      return false;
    }

    folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
        uint8_t /* pri */) override {
      VLOG(4) << __func__ << " txn=" << txn_;
      return HTTPMessage::HTTP2Priority(hqDefaultPriority.streamDependency,
                                        hqDefaultPriority.exclusive,
                                        hqDefaultPriority.weight);
    }

    folly::Optional<HTTPPriority> getHTTPPriority() override {
      if (session_.sock_ && hasStreamId()) {
        auto sp = session_.sock_->getStreamPriority(getStreamId());
        if (sp) {
          return HTTPPriority(sp.value().level, sp.value().incremental);
        }
      }
      return folly::none;
    }

    folly::Optional<HTTPTransaction::ConnectionToken> getConnectionToken()
        const noexcept override {
      return session_.connectionToken_;
    }

    void trackEgressBodyOffset(uint64_t bodyOffset,
                               proxygen::ByteEvent::EventFlags flags) override;

    /**
     * Returns whether or no we have any body bytes buffered in the stream, or
     * the txn has any body bytes buffered.
     */
    size_t writeBufferSize() const;
    bool hasWriteBuffer() const;
    bool hasPendingBody() const;
    bool hasPendingEOM() const;
    bool hasPendingEgress() const;

    /**
     * Adapter class for managing different enqueued state between
     * HTTPTransaction and HQStreamTransport.  The decouples whether the
     * transaction thinks it is enqueued for egress (which impacts txn lifetime)
     * and whether the HQStreamTransport is enqueued (which impacts the
     * actual egress algorithm).  Note all 4 states are possible.
     */
    class HQPriHandle : public HTTP2PriorityQueueBase::BaseNode {
     public:
      void init(HTTP2PriorityQueueBase::Handle handle) {
        egressQueueHandle_ = handle;
        enqueued_ = handle->isEnqueued();
      }

      HTTP2PriorityQueueBase::Handle FOLLY_NULLABLE getHandle() const {
        return egressQueueHandle_;
      }

      void clearHandle() {
        egressQueueHandle_ = nullptr;
      }

      // HQStreamTransport is enqueued
      bool isStreamTransportEnqueued() const {
        return egressQueueHandle_ ? egressQueueHandle_->isEnqueued() : false;
      }

      bool isTransactionEnqueued() const {
        return isEnqueued();
      }

      void setEnqueued(bool enqueued) {
        enqueued_ = enqueued;
      }

      bool isEnqueued() const override {
        return enqueued_;
      }

      uint64_t calculateDepth(bool includeVirtual = true) const override {
        return egressQueueHandle_->calculateDepth(includeVirtual);
      }

     private:
      HTTP2PriorityQueueBase::Handle egressQueueHandle_{nullptr};
      bool enqueued_;
    };

    HTTP2PriorityQueueBase::Handle addTransaction(HTTPCodec::StreamID id,
                                                  http2::PriorityUpdate pri,
                                                  HTTPTransaction* txn,
                                                  bool permanent,
                                                  uint64_t* depth) override {
      queueHandle_.init(session_.txnEgressQueue_.addTransaction(
          id, pri, txn, permanent, depth));
      return &queueHandle_;
    }

    // update the priority of an existing node
    HTTP2PriorityQueueBase::Handle updatePriority(
        HTTP2PriorityQueueBase::Handle handle,
        http2::PriorityUpdate pri,
        uint64_t* depth) override {
      CHECK_EQ(handle, &queueHandle_);
      CHECK(queueHandle_.getHandle());
      return session_.txnEgressQueue_.updatePriority(
          queueHandle_.getHandle(), pri, depth);
    }

    // Remove the transaction from the priority tree
    void removeTransaction(HTTP2PriorityQueueBase::Handle handle) override {
      CHECK_EQ(handle, &queueHandle_);
      CHECK(queueHandle_.getHandle());
      session_.txnEgressQueue_.removeTransaction(queueHandle_.getHandle());
      queueHandle_.clearHandle();
    }

    // Notify the queue when a transaction has egress
    void signalPendingEgress(HTTP2PriorityQueueBase::Handle h) override {
      CHECK_EQ(h, &queueHandle_);
      queueHandle_.setEnqueued(true);
      signalPendingEgressImpl();
    }

    void signalPendingEgressImpl() {
      auto flowControl =
          session_.sock_->getStreamFlowControl(getEgressStreamId());
      if (!flowControl.hasError() && flowControl->sendWindowAvailable > 0) {
        session_.txnEgressQueue_.signalPendingEgress(queueHandle_.getHandle());
      } else {
        VLOG(4) << "Delay pending egress signal on blocked txn=" << txn_;
      }
    }

    // Notify the queue when a transaction no longer has egress
    void clearPendingEgress(HTTP2PriorityQueueBase::Handle h) override {
      CHECK_EQ(h, &queueHandle_);
      CHECK(queueHandle_.isTransactionEnqueued());
      queueHandle_.setEnqueued(false);
      if (pendingEOM_ || hasWriteBuffer()) {
        // no-op
        // Only HQSession can clearPendingEgress for these cases
        return;
      }
      // The transaction has pending body data, but it decided to remove itself
      // from the egress queue since it's rate-limited
      if (queueHandle_.isStreamTransportEnqueued()) {
        session_.txnEgressQueue_.clearPendingEgress(queueHandle_.getHandle());
      }
    }

    void addPriorityNode(HTTPCodec::StreamID id,
                         HTTPCodec::StreamID parent) override {
      session_.txnEgressQueue_.addPriorityNode(id, parent);
    }

    /**
     * How many egress bytes we committed to transport, both written and
     * skipped.
     */
    uint64_t streamEgressCommittedByteOffset() const {
      return bytesWritten_;
    }

    /**
     * streamEgressCommittedByteOffset() plus any pending bytes in the egress
     * queue.
     */
    uint64_t streamWriteByteOffset() const {
      return streamEgressCommittedByteOffset() + writeBuf_.chainLength() +
             bufMeta_.length;
    }

    void abortIngress();

    void abortEgress(bool checkForDetach);

    void errorOnTransaction(ProxygenError err, const std::string& errorMsg);
    void errorOnTransaction(HTTPException ex);

    bool wantsOnWriteReady(size_t canSend) const;

    HQPriHandle queueHandle_;
    HTTPTransaction txn_;
    // need to send EOM
    bool pendingEOM_{false};
    // have read EOF
    bool readEOF_{false};
    bool hasCodec_{false};
    bool hasIngress_{false};
    bool detached_{false};
    bool ingressError_{false};
    bool hasHeaders_{false};
    enum class EOMType { CODEC, TRANSPORT };
    ConditionalGate<EOMType, 2> eomGate_;

    folly::Optional<HTTPCodec::StreamID> codecStreamId_;

    HQByteEventTracker byteEventTracker_;

    // Stream + session protocol info
    std::shared_ptr<QuicStreamProtocolInfo> quicStreamProtocolInfo_;

    // BufferMeta represents a buffer that isn't owned by this stream but a
    // remote entity.
    HTTPTransaction::BufferMeta bufMeta_;

    void armStreamByteEventCb(uint64_t streamOffset,
                              quic::QuicSocket::ByteEvent::Type type);
    void armEgressHeadersAckCb(uint64_t streamOffset);
    void armEgressBodyCallbacks(uint64_t bodyOffset,
                                uint64_t streamOffset,
                                proxygen::ByteEvent::EventFlags eventFlags);
    void resetEgressHeadersAckOffset() {
      egressHeadersAckOffset_ = folly::none;
    }
    folly::Optional<uint64_t> resetEgressBodyEventOffset(
        uint64_t streamOffset) {
      auto it = egressBodyByteEventOffsets_.find(streamOffset);
      if (it != egressBodyByteEventOffsets_.end()) {
        CHECK_GT(it->second.callbacks, 0);
        it->second.callbacks--;
        auto bodyOffset = it->second.bodyOffset;
        if (it->second.callbacks == 0) {
          egressBodyByteEventOffsets_.erase(it);
        }
        return bodyOffset;
      }
      return folly::none;
    }

    uint64_t numActiveDeliveryCallbacks() const {
      return numActiveDeliveryCallbacks_;
    }

   private:
    void updatePriority(const HTTPMessage& headers) noexcept;

    // Return the old and new offset of the stream
    std::pair<uint64_t, uint64_t> generateHeadersCommon(
        quic::StreamId streamId,
        const HTTPMessage& headers,
        bool includeEOM,
        HTTPHeaderSize* size) noexcept;
    void coalesceEOM(size_t encodedBodySize);
    void handleHeadersAcked(uint64_t streamOffset);
    void handleBodyEvent(uint64_t streamOffset,
                         quic::QuicSocket::ByteEvent::Type type);
    void handleBodyEventCancelled(uint64_t streamOffset,
                                  quic::QuicSocket::ByteEvent::Type type);
    uint64_t bodyBytesEgressed_{0};
    folly::Optional<uint64_t> egressHeadersAckOffset_;
    struct BodyByteOffset {
      uint64_t bodyOffset;
      uint64_t callbacks;
      BodyByteOffset(uint64_t bo, uint64_t c) : bodyOffset(bo), callbacks(c) {
      }
    };
    // We allow random insert/removal in this map, but removal should be
    // sequential
    folly::F14FastMap<uint64_t, BodyByteOffset> egressBodyByteEventOffsets_;
    // Track number of armed QUIC delivery callbacks.
    uint64_t numActiveDeliveryCallbacks_{0};

    // Used to store last seen ingress push ID between
    // the invocations of onPushPromiseBegin / onHeadersComplete.
    // It is being reset by
    //  - "onNewMessage" (in which case the push promise is being abandoned),
    //  - "onPushMessageBegin" (which may be abandonned / duplicate message id)
    //  - "onHeadersComplete" (not pending anymore)
    folly::Optional<hq::PushId> ingressPushId_;
  }; // HQStreamTransportBase

  void dispatchUniWTStream(quic::StreamId /* streamId */,
                           quic::StreamId /* sessionId */,
                           size_t /* to consume */) override;

  void dispatchBidiWTStream(quic::StreamId /* streamId */,
                            quic::StreamId /* sessionId */,
                            size_t /* to consume */) override;

 protected:
  class HQStreamTransport
      : public detail::singlestream::SSBidir
      , public HQStreamTransportBase {
   public:
    HQStreamTransport(
        HQSession& session,
        TransportDirection direction,
        quic::StreamId streamId,
        uint32_t seqNo,
        std::unique_ptr<HTTPCodec> codec,
        const WheelTimerInstance& wheelTimer,
        HTTPSessionStats* stats = nullptr,
        http2::PriorityUpdate priority = hqDefaultPriority,
        folly::Optional<HTTPCodec::StreamID> parentTxnId = HTTPCodec::NoStream)
        : detail::singlestream::SSBidir(streamId),
          HQStreamTransportBase(session,
                                direction,
                                streamId,
                                seqNo,
                                wheelTimer,
                                stats,
                                priority,
                                parentTxnId) {
      // Request streams are eagerly initialized
      initCodec(std::move(codec), __func__);
      initIngress(__func__);
    }

    HTTPTransaction* newPushedTransaction(
        HTTPCodec::StreamID /* parentTxnId */,
        HTTPTransaction::PushHandler* /* handler */,
        ProxygenError* error = nullptr) noexcept override;

    void sendPushPromise(HTTPTransaction* /* txn */,
                         folly::Optional<hq::PushId> /* pushId */,
                         const HTTPMessage& /* headers */,
                         HTTPHeaderSize* /* outSize */,
                         bool /* includeEOM */) override;

    void onPushPromiseHeadersComplete(
        hq::PushId /* pushID */,
        HTTPCodec::StreamID /* assoc streamID */,
        std::unique_ptr<HTTPMessage> /* promise */) override;

    uint16_t getDatagramSizeLimit() const noexcept override;
    bool sendDatagram(std::unique_ptr<folly::IOBuf> datagram) override;

    [[nodiscard]] bool supportsWebTransport() const override {
      return session_.supportsWebTransport();
    }
    folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
    newWebTransportBidiStream() override;
    folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
    newWebTransportUniStream() override;

    folly::Expected<HTTPTransaction::Transport::FCState,
                    WebTransport::ErrorCode>
    sendWebTransportStreamData(HTTPCodec::StreamID /*id*/,
                               std::unique_ptr<folly::IOBuf> /*data*/,
                               bool /*eof*/) override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode>
        resetWebTransportEgress(HTTPCodec::StreamID /*id*/,
                                uint32_t /*errorCode*/) override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode>
        pauseWebTransportIngress(HTTPCodec::StreamID /*id*/) override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode>
        resumeWebTransportIngress(HTTPCodec::StreamID /*id*/) override;

    folly::Expected<folly::Unit, WebTransport::ErrorCode>
        stopReadingWebTransportIngress(HTTPCodec::StreamID /*id*/,
                                       uint32_t /*errorCode*/) override;

    class WTWriteCallback : public quic::QuicSocket::WriteCallback {
     public:
      explicit WTWriteCallback(HTTPTransaction& txn) : txn_(txn) {
      }

      void onStreamWriteReady(quic::StreamId id, uint64_t) noexcept override {
        VLOG(4) << "onStreamWriteReady id=" << id;
        txn_.onWebTransportEgressReady(id);
      }

     private:
      HTTPTransaction& txn_;
    };

    class WTReadCallback : public quic::QuicSocket::ReadCallback {
     public:
      explicit WTReadCallback(HTTPTransaction& txn, HQSession& session)
          : txn_(txn), session_(session) {
      }

      void readAvailable(quic::StreamId id) noexcept override;

      void readError(quic::StreamId id,
                     quic::QuicError error) noexcept override;

     private:
      HTTPTransaction& txn_;
      HQSession& session_;
    };

    std::unique_ptr<WTWriteCallback> wtWriteCallback_;
    std::unique_ptr<WTReadCallback> wtReadCallback_;

    WTWriteCallback* getWTWriteCallback() {
      if (!wtWriteCallback_) {
        wtWriteCallback_ = std::make_unique<WTWriteCallback>(txn_);
      }
      return wtWriteCallback_.get();
    }
    WTReadCallback* getWTReadCallback() {
      if (!wtReadCallback_) {
        wtReadCallback_ = std::make_unique<WTReadCallback>(txn_, session_);
      }
      return wtReadCallback_.get();
    }
  }; // HQStreamTransport

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  std::unique_ptr<HTTPCodec> createCodec(quic::StreamId id);
  bool maybeRejectRequestAfterGoaway(quic::StreamId id);

 private:
  void sendGoaway();

  std::unique_ptr<hq::HQUnidirectionalCodec> createControlCodec(
      hq::UnidirectionalStreamType type, HQControlStream& controlStream);

  void headersComplete(HTTPMessage* /*msg*/);

  void readDataProcessed();

  void abortStream(quic::StreamId /*id*/);

  uint32_t getMaxConcurrentOutgoingStreamsRemote() const override {
    // need transport API
    return 100;
  }

  using HTTPCodecPtr = std::unique_ptr<HTTPCodec>;
  struct CodecStackEntry {
    HTTPCodecPtr* codecPtr;
    HTTPCodecPtr codec;
    HTTPCodec::Callback* callback;
    CodecStackEntry(HTTPCodecPtr* p, HTTPCodecPtr c, HTTPCodec::Callback* cb)
        : codecPtr(p), codec(std::move(c)), callback(cb) {
    }
  };
  std::vector<CodecStackEntry> codecStack_;

  /**
   * Container to hold the results of HTTP2PriorityQueue::nextEgress
   */
  HTTP2PriorityQueue::NextEgressResult nextEgressResults_;

  // Cleanup all pending streams. Invoked in session timeout
  size_t cleanupPendingStreams();

  // Remove all callbacks from a stream during cleanup
  void clearStreamCallbacks(quic::StreamId /* id */);

  void controlStreamReadAvailable(quic::StreamId id);
  void controlStreamReadError(quic::StreamId id, const quic::QuicError& error);

  using ControlStreamsKey = std::pair<quic::StreamId, hq::StreamDirection>;
  std::unordered_map<hq::UnidirectionalStreamType, HQControlStream>
      controlStreams_;
  HQUniStreamDispatcher unidirectionalReadDispatcher_;
  HQBidiStreamDispatcher bidirectionalReadDispatcher_;
  ControlStreamReadCallback controlStreamReadCallback_;
  QPACKCodec qpackCodec_;

  // Min Stream ID we haven't seen so far
  quic::StreamId minUnseenIncomingStreamId_{0};
  // Whether SETTINGS have been received
  bool receivedSettings_{false};
  enum class SettingEnabled : uint8_t { SELF = 0, PEER = 1 };
  std::bitset<2> supportsWebTransport_;

  /**
   * The maximum number of concurrent transactions that this session's peer
   * may create.
   */
  uint32_t maxConcurrentIncomingStreams_{100};
  folly::Optional<uint32_t> receiveStreamWindowSize_;

  uint64_t maxToSend_{0};
  bool scheduledWrite_{false};

  bool forceUpstream1_1_{true};
  // Default to false for now to match existing behavior
  bool strictValidation_{false};
  bool datagramEnabled_{false};

  /** Reads in the current loop iteration */
  uint16_t readsPerLoop_{0};
  std::unordered_set<quic::StreamId> pendingProcessReadSet_;
  std::shared_ptr<QuicProtocolInfo> quicInfo_;
  folly::Optional<HQVersion> version_;
  std::string alpn_;

 protected:
  HTTPSettings egressSettings_{
      {SettingsId::HEADER_TABLE_SIZE, hq::kDefaultEgressHeaderTableSize},
      {SettingsId::MAX_HEADER_LIST_SIZE, hq::kDefaultEgressMaxHeaderListSize},
      {SettingsId::_HQ_QPACK_BLOCKED_STREAMS,
       hq::kDefaultEgressQpackBlockedStream},
  };
  HTTPSettings ingressSettings_;
  // Maximum Stream/Push ID that we are allowed to open, from GOAWAY
  quic::StreamId peerMinUnseenId_{hq::kMaxClientBidiStreamId};
  uint64_t minUnseenIncomingPushId_{0};

  ReadyGate versionUtilsReady_;

  // NOTE: introduce better decoupling between the streams
  // and the containing session, then remove the friendship.
  friend class HQStreamBase;

  // To let the operator<< access DrainState which is private
  friend std::ostream& operator<<(std::ostream&, DrainState);

  // Bidirectional transport streams
  std::unordered_map<quic::StreamId, HQStreamTransport> streams_;

  // Buffer for datagrams waiting for a stream to be assigned to
  folly::EvictingCacheMap<
      quic::StreamId,
      folly::small_vector<
          std::unique_ptr<folly::IOBuf>,
          kDefaultMaxBufferedDatagrams,
          folly::small_vector_policy::policy_in_situ_only<true>>>
      datagramsBuffer_{kMaxStreamsWithBufferedDatagrams};

  // Buffer for priority updates without an active stream
  folly::EvictingCacheMap<quic::StreamId, HTTPPriority> priorityUpdatesBuffer_{
      kMaxBufferedPriorityUpdates};

  // Lookup maps for matching PushIds to StreamIds
  folly::F14FastMap<hq::PushId, quic::StreamId> pushIdToStreamId_;
  // Lookup maps for matching ingress push streams to push ids
  folly::F14FastMap<quic::StreamId, hq::PushId> streamIdToPushId_;
  std::string userAgent_;

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
}; // HQSession

std::ostream& operator<<(std::ostream& os, HQSession::DrainState drainState);

} // namespace proxygen
