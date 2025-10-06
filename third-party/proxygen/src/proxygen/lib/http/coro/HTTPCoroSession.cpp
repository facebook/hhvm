/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include <proxygen/lib/http/HTTPPriorityFunctions.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <proxygen/lib/http/codec/HTTPChecks.h>
#include <proxygen/lib/http/codec/HTTPParallelCodec.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>
#include <proxygen/lib/http/webtransport/HTTPWebTransport.h>

#include <folly/logging/xlog.h>
#include <quic/priority/HTTPPriorityQueue.h>
#include <quic/state/QuicStreamUtilities.h>
#include <wangle/acceptor/ConnectionManager.h>

#include <algorithm>

namespace {

using namespace proxygen;
using namespace proxygen::coro;

constexpr size_t kMinReadSize = 1460;
constexpr size_t kReadBufNewAllocSize = 4000;
// 16 is the default reads-per-loop for AsyncSocket
constexpr uint64_t kMaxReadDataPerLoop = kReadBufNewAllocSize * 16;
// 64 data (default fcw) + 9 byte stream headers for 100 (default) streams
constexpr uint32_t kWriteBufLimit = 65535 + 900;
constexpr uint64_t kMaxQuarterStreamId = (1ull << 60) - 1;

const quic::QuicError kHTTPNoError{
    quic::ApplicationErrorCode(HTTP3::ErrorCode::HTTP_NO_ERROR), ""};

const HTTPPriority kDefaultPriority{3, false};

//  ErrorCodes that can be generated from within HTTPStreamSource
HTTPErrorCode sourceCompleteErr2ErrorCode(HTTPErrorCode ec) {
  switch (ec) {
    case HTTPErrorCode::NO_ERROR:
    case HTTPErrorCode::CANCEL:
    case HTTPErrorCode::FLOW_CONTROL_ERROR:
      return ec;
    case HTTPErrorCode::CORO_CANCELLED:
      // Called when a reader cancels a reading coroutine
      return HTTPErrorCode::CANCEL;
    case HTTPErrorCode::READ_TIMEOUT:
      return HTTPErrorCode::CANCEL;
    case HTTPErrorCode::INVALID_STATE_TRANSITION:
      return HTTPErrorCode::MESSAGE_ERROR;
    case HTTPErrorCode::CONTENT_LENGTH_MISMATCH:
      return HTTPErrorCode::CONTENT_LENGTH_MISMATCH;
    default:
      XLOG(FATAL) << "Invalid error from sourceComplete ec=" << uint16_t(ec);
  }
}

#define SESS_STATS(method, ...)                                             \
  if (sessionStats_) {                                                      \
    folly::invoke(&HTTPSessionStats::method, sessionStats_, ##__VA_ARGS__); \
  }                                                                         \
  static_assert(true, "semicolon required")

void setSecureMsg(HTTPMessage& msg,
                  const wangle::TransportInfo& setupTransportInfo) {
  msg.setSecureInfo(setupTransportInfo.sslVersion,
                    setupTransportInfo.sslCipher
                        ? setupTransportInfo.sslCipher->c_str()
                        : nullptr);
  msg.setSecure(setupTransportInfo.secure);
}

} // namespace

using folly::coro::co_error;
using folly::coro::co_nothrow;
using folly::coro::co_withCancellation;

namespace proxygen::coro {

HTTPSource* getErrorResponse(uint16_t statusCode, const std::string& body) {
  auto resp = proxygen::coro::HTTPFixedSource::makeFixedResponse(
      statusCode, folly::IOBuf::copyBuffer(body));
  resp->msg_->setWantsKeepalive(false);
  return resp;
}

struct HTTPCoroSession::StreamState {
 private:
  // Detach Criteria: source and egressCoro start as complete until the stream
  // enters a state where they are relevant.
  bool streamSourceComplete_ : 1;
  bool egressComplete_ : 1;
  bool egressCoroComplete_ : 1;

  bool deferredStopSending_ : 1;
  bool pendingEgressEOM_ : 1; // required for QUIC with no body

  // Egress State
  bool egressStarted_ : 1;

  enum class Upgrade : uint8_t {
    NONE,
    PENDING,
    UPGRADED
  } upgrade_{Upgrade::NONE};
  Window sendWindow_;
  HTTPBodyEventQueue bodyEventQueue_;
  folly::IOBufQueue* writeBuf_{nullptr};
  folly::IOBufQueue hqWriteBuf_{folly::IOBufQueue::cacheChainLength()};
  HTTPPriority priority_;
  uint64_t streamOffset_{0}; // total bytes including codec framing overhead

 public:
  HTTPStreamSource streamSource;
  struct {
    folly::CancellationSource ingress, egress;
  } cs;

  // handlers can only egress this status code on some ingress errors
  uint16_t errorStatusCode{0};

  // Push state - these are left public because the session usually
  // has to manipulate state of two streams at the same time (parent & push)

  // Only set for push streams
  std::optional<HTTPCodec::StreamID> parent;
  std::optional<uint64_t> currentPushID;

  constexpr static size_t kEgressBufferLimit = 65535;

  StreamState(folly::EventBase* evb,
              HTTPCodec::StreamID id,
              HTTPCoroSession& session,
              uint32_t sendFlowControlWindow,
              uint32_t recvFlowControlWindow,
              std::chrono::milliseconds readTimeout,
              std::chrono::milliseconds writeTimeout)
      : sendWindow_(sendFlowControlWindow),
        bodyEventQueue_(evb, id, session, kEgressBufferLimit, writeTimeout),
        streamSource(evb, id, session, recvFlowControlWindow, readTimeout) {
    streamSourceComplete_ = true;
    egressComplete_ = false;
    egressCoroComplete_ = true;
    deferredStopSending_ = false;
    pendingEgressEOM_ = false;
    egressStarted_ = false;
  }

  HTTPCodec::StreamID getID() const {
    return streamSource.getID();
  }

  void setWriteBuf(folly::IOBufQueue* writeBuf) {
    if (writeBuf) {
      writeBuf_ = writeBuf;
    } else {
      writeBuf_ = &hqWriteBuf_;
    }
  }

  folly::IOBufQueue& getWriteBuf() {
    return *writeBuf_;
  }

  void abortIngress(HTTPErrorCode err, std::string_view details = "") {
    streamSource.abort(err, details);
    cs.ingress.requestCancellation();
  }

  void setReadTimeout(std::chrono::milliseconds readTimeout) {
    // A caller may have already set a specific timeout for this source
    if (streamSource.getReadTimeout() == std::chrono::milliseconds(0)) {
      streamSource.setReadTimeout(readTimeout);
    }
  }

  HTTPPriority getPriority() const {
    return priority_;
  }

  void setPriority(HTTPPriority priority) {
    priority_ = priority;
  }

  uint64_t getStreamOffset() const {
    return streamOffset_;
  }

  void addToStreamOffset(uint64_t bytes) {
    streamOffset_ += bytes;
  }

  void initIngressPush(HTTPCodec::StreamID assocStreamID) {
    parent = assocStreamID;
    markEgressComplete();
  }

  void streamSourceActive() {
    // Called when the streamSource is handed outside the class for reading
    streamSourceComplete_ = false;
  }

  void startEgressCoro() {
    // Called when this CoroSession starts a coroutine reading from an
    // external source to produce egress on this stream.
    egressCoroComplete_ = false;
  }

  // Returns true if there is an upgrade in progress or completed
  bool checkForUpgrade(const HTTPMessage& msg, bool isIngress) {
    bool isWebSocketUpgrade = (isIngress && msg.isIngressWebsocketUpgrade()) ||
                              (!isIngress && msg.isEgressWebsocketUpgrade());
    if (msg.isRequest() &&
        (msg.getMethod() == HTTPMethod::CONNECT || isWebSocketUpgrade)) {
      XCHECK(upgrade_ == Upgrade::NONE);
      upgrade_ = Upgrade::PENDING;
    } else if (upgrade_ == Upgrade::PENDING && msg.isResponse() &&
               (msg.getStatusCode() == 200 || isWebSocketUpgrade)) {
      upgrade_ = Upgrade::UPGRADED;
    }
    return upgrade_ != Upgrade::NONE;
  }

  bool isUpgraded() const {
    return upgrade_ == Upgrade::UPGRADED;
  }

  void markIngressAsHeadResponse() {
    streamSource.skipContentLengthValidation();
  }

  void markStreamSourceComplete() {
    XCHECK(!streamSourceComplete_);
    streamSourceComplete_ = true;
  }

  void markEgressComplete() {
    egressComplete_ = true;
  }

  void markEgressCoroComplete() {
    XCHECK(!egressCoroComplete_);
    egressCoroComplete_ = true;
  }

  bool canSendHeaders() const {
    return !egressStarted_ && !egressComplete_;
  }

  bool isEgressComplete() const {
    return egressComplete_;
  }

  bool isDetachable() const {
    // streamSourceComplete -> external source reader finished or stopReading
    // egressComplete -> we have egressed a FIN, RST or received a RST
    // egressCoroComplete -> internal coroutine producing stream egress
    // terminated
    return (streamSourceComplete_ && egressComplete_ && egressCoroComplete_);
  }

  uint64_t observedBodyLength() const {
    return bodyEventQueue_.observedBodyLength();
  }

  void setPendingEgressEOM() {
    pendingEgressEOM_ = true;
  }

  bool pendingEgressEOM() const {
    return pendingEgressEOM_;
  }

  bool isBodyQueueEmpty() const {
    return bodyEventQueue_.empty();
  }

  void setEgressSource(HTTPSource* source) {
    bodyEventQueue_.setSource(source);
  }

  void setErrorStatusCode(uint16_t statusCode) {
    XCHECK(canSendHeaders());
    XCHECK(!egressCoroComplete_);
    errorStatusCode = statusCode;
    cs.ingress.requestCancellation();
    std::exchange(cs.egress, {}).requestCancellation();
  }

  folly::coro::Task<HTTPHeaderEvent> nextHeaderEvent() {
    // TODO: do we want to throttle reading response headers on buffer space
    auto headerEvent = co_await co_nothrow(bodyEventQueue_.readHeaderEvent());
    egressStarted_ = headerEvent.isFinal();
    co_return headerEvent;
  }

  folly::coro::Task<HTTPBodyEventQueue::ReadBodyResult> nextBodyEvent() {
    return bodyEventQueue_.readBodyEvent();
  }

  void stopReading(HTTPErrorCode error) {
    bodyEventQueue_.stopReading(error);
  }

  std::pair<HTTPBodyEvent, bool> nextEgressEvent(uint32_t maxToSend) {
    bool flowControlBlocked = false;
    maxToSend = std::min(maxToSend, sendWindow_.getNonNegativeSize());
    HTTPBodyEvent bodyEvent(bodyEventQueue_.dequeueBodyEvent(maxToSend));
    if (bodyEvent.eventType == HTTPBodyEvent::BODY) {
      auto length = bodyEvent.event.body.chainLength();
      XLOG(DBG4) << "Sending body length=" << length << " id=" << getID()
                 << " eom=" << uint32_t(bodyEvent.eom);
      if (length == 0 && !bodyEvent.eom) {
        flowControlBlocked = true;
      } else {
        XCHECK(sendWindow_.reserve(length));
      }
    }
    XCHECK(!bodyEvent.eom || bodyEventQueue_.empty())
        << "stream returned EOM with pending events";
    if (bodyEvent.eom) {
      markEgressComplete();
    }
    return {std::move(bodyEvent), flowControlBlocked};
  }

  folly::Expected<bool, folly::Unit> onWindowUpdate(uint32_t amount) {
    auto wasBlocked = (sendWindow_.getSize() <= 0);
    if (!sendWindow_.free(amount)) {
      return folly::makeUnexpected(folly::Unit());
    }
    auto isBlocked = (sendWindow_.getSize() <= 0);
    return wasBlocked && !isBlocked && !bodyEventQueue_.empty();
  }

  void setSendWindow(uint32_t capacity) {
    sendWindow_.setCapacity(capacity);
  }

  bool isFlowControlBlocked() const {
    return sendWindow_.getSize() <= 0;
  }

  void resetStream(const HTTPError& err) {
    markEgressComplete();
    if (!bodyEventQueue_.empty()) {
      XLOG(DBG4) << "Discarding pending egress on reset for stream=" << getID();
      bodyEventQueue_.clear(err);
    }
    cs.egress.requestCancellation();
    if (streamSourceComplete_) {
      cs.ingress.requestCancellation();
    }
    deferredStopSending_ = false;
  }

  bool isStateReset() const {
    return isBodyQueueEmpty() && cs.egress.isCancellationRequested();
  }

  void setExpectedEgressContentLength(const std::string& contentLen, bool eom) {
    bodyEventQueue_.setExpectedEgressContentLength(contentLen, eom);
  }

  void skipEgressContentLengthValidation() {
    bodyEventQueue_.skipContentLengthValidation();
  }

  void setDeferredStopSending(bool deferredStopSending) {
    deferredStopSending_ = deferredStopSending;
  }

  bool getDeferredStopSending() const {
    return deferredStopSending_;
  }
};

using quic::HTTPPriorityQueue;

HTTPCoroSession::HTTPCoroSession(folly::EventBase* eventBase,
                                 folly::SocketAddress localAddr,
                                 folly::SocketAddress peerAddr,
                                 std::unique_ptr<HTTPCodec> codec,
                                 wangle::TransportInfo tinfo,
                                 std::shared_ptr<HTTPHandler> handler)
    : eventBase_(eventBase),
      direction_(handler ? TransportDirection::DOWNSTREAM
                         : TransportDirection::UPSTREAM),
      localAddr_(std::move(localAddr)),
      peerAddr_(std::move(peerAddr)),
      codec_(std::move(codec)),
      handler_(std::move(handler)),
      setupTransportInfo_(std::move(tinfo)),
      sendWindow_(codec_->supportsSessionFlowControl()
                      ? codec_->getDefaultWindowSize()
                      : std::numeric_limits<int32_t>::max()),
      recvWindow_(codec_->supportsSessionFlowControl()
                      ? codec_->getDefaultWindowSize()
                      : std::numeric_limits<int32_t>::max()) {
  localAddr_.tryConvertToIPv4();
  peerAddr_.tryConvertToIPv4();
}

HTTPCoroSession::~HTTPCoroSession() {
  XLOG(DBG4) << "Destroying " << *this;
  XCHECK(streams_.empty());
  XCHECK_EQ(numPushStreams_, 0UL);
  SESS_STATS(recordTransactionsServed, nextStreamSequenceNumber_);
  deliverLifecycleEvent(&LifecycleObserver::onDestroy, *this);
}

HTTPQuicCoroSession::HTTPQuicCoroSession(
    std::shared_ptr<quic::QuicSocket> sock,
    std::unique_ptr<hq::HQMultiCodec> codec,
    wangle::TransportInfo tinfo,
    std::shared_ptr<HTTPHandler> handler)
    : HTTPCoroSession(sock->getEventBase()
                          ->getTypedEventBase<quic::FollyQuicEventBase>()
                          ->getBackingEventBase(),
                      sock->getLocalAddress(),
                      sock->getPeerAddress(),
                      std::move(codec),
                      std::move(tinfo),
                      std::move(handler)),
      quicSocket_(std::move(sock)),
      multiCodec_(static_cast<hq::HQMultiCodec*>(codec_.getChainEndPtr())),
      qpackEncoderCodec_(multiCodec_->getQPACKCodec(), *this),
      qpackDecoderCodec_(multiCodec_->getQPACKCodec(), *this),
      uniStreamDispatcher_(*this, direction_) {
  start();
}
HTTPQuicCoroSession::~HTTPQuicCoroSession() = default;

HTTPCoroSession* HTTPCoroSession::makeUpstreamCoroSession(
    std::unique_ptr<folly::coro::TransportIf> coroTransport,
    std::unique_ptr<HTTPCodec> codec,
    wangle::TransportInfo tinfo) {
  XCHECK(codec->getTransportDirection() == TransportDirection::UPSTREAM);
  return new HTTPUniplexTransportSession(
      std::move(coroTransport), std::move(codec), std::move(tinfo));
}

HTTPCoroSession* HTTPCoroSession::makeDownstreamCoroSession(
    std::unique_ptr<folly::coro::TransportIf> coroTransport,
    std::shared_ptr<HTTPHandler> handler,
    std::unique_ptr<HTTPCodec> codec,
    wangle::TransportInfo tinfo) {
  XCHECK(codec->getTransportDirection() == TransportDirection::DOWNSTREAM);
  return new HTTPUniplexTransportSession(std::move(coroTransport),
                                         std::move(codec),
                                         std::move(tinfo),
                                         std::move(handler));
}

HTTPCoroSession* HTTPCoroSession::makeUpstreamCoroSession(
    std::shared_ptr<quic::QuicSocket> sock,
    std::unique_ptr<hq::HQMultiCodec> codec,
    wangle::TransportInfo tinfo) {
  XCHECK(codec->getTransportDirection() == TransportDirection::UPSTREAM);
  return new HTTPQuicCoroSession(
      std::move(sock), std::move(codec), std::move(tinfo));
}

HTTPCoroSession* HTTPCoroSession::makeDownstreamCoroSession(
    std::shared_ptr<quic::QuicSocket> sock,
    std::shared_ptr<HTTPHandler> handler,
    std::unique_ptr<hq::HQMultiCodec> codec,
    wangle::TransportInfo tinfo) {
  XCHECK(codec->getTransportDirection() == TransportDirection::DOWNSTREAM);
  return new HTTPQuicCoroSession(
      std::move(sock), std::move(codec), std::move(tinfo), std::move(handler));
}

void HTTPUniplexTransportSession::start() {
  maybeEnableByteEvents();
  if (!codec_->supportsParallelRequests()) {
    maxConcurrentOutgoingStreamsRemote_ = isDownstream() ? 0 : 1;
  }

  codec_.add<HTTPChecks>();
  if (codec_->supportsParallelRequests() &&
      codec_->getTransportDirection() == TransportDirection::DOWNSTREAM) {
    auto rateLimitFilter =
        std::make_unique<RateLimitFilter>(&eventBase_->timer(), sessionStats_);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::HEADERS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::MISC_CONTROL_MSGS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::RSTS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::DIRECT_ERROR_HANDLING);
    rateLimitFilter->setRateLimitParams(
        RateLimiter::Type::RSTS,
        /*maxEventsPerInterval=*/1'000,
        /*intervalDuration=*/std::chrono::milliseconds(100));
    rateLimitFilter_ = rateLimitFilter.get();
    codec_.addFilters(std::move(rateLimitFilter));
  }
  codec_.setCallback(this);
  sendPreface();
}

void HTTPQuicCoroSession::start() {
  // This bizarre behavior is copied from HQSession - all TransportInfo's share
  // the same shared_ptr to the current protocol info.
  initQuicProtocolInfo(*currentQuicProtocolInfo_, *quicSocket_);
  setupTransportInfo_.protocolInfo = currentQuicProtocolInfo_;
  quicSocket_->setConnectionCallback(this);
  quicSocket_->setPingCallback(this);
  codec_.add<HTTPChecks>();

  if (codec_->getTransportDirection() == TransportDirection::DOWNSTREAM) {
    auto rateLimitFilter =
        std::make_unique<RateLimitFilter>(&eventBase_->timer(), sessionStats_);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::HEADERS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::MISC_CONTROL_MSGS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::RSTS);
    rateLimitFilter->addRateLimiter(RateLimiter::Type::DIRECT_ERROR_HANDLING);
    rateLimitFilter->setRateLimitParams(
        RateLimiter::Type::RSTS,
        /*maxEventsPerInterval=*/1'000,
        /*intervalDuration=*/std::chrono::milliseconds(100));
    codec_.addFilters(std::move(rateLimitFilter));
  }

  if (!createControlStream(
          hq::UnidirectionalStreamType::CONTROL, writeBuf_, controlStreamID_) ||
      !createControlStream(hq::UnidirectionalStreamType::QPACK_ENCODER,
                           multiCodec_->getQPACKEncoderWriteBuf(),
                           qpackEncoderStreamID_) ||
      !createControlStream(hq::UnidirectionalStreamType::QPACK_DECODER,
                           multiCodec_->getQPACKDecoderWriteBuf(),
                           qpackDecoderStreamID_)) {
    // connection is unusable
    XLOG(DBG4) << "Failed to create control stream; sess=" << *this;
    return;
  }
  multiCodec_->setQPACKEncoderMaxDataFn([this] {
    auto res = quicSocket_->getStreamFlowControl(qpackEncoderStreamID_);
    if (res.hasError()) {
      return uint64_t(0);
    }
    return res->sendWindowAvailable;
  });

  if (isDatagramEnabled()) {
    quicSocket_->setDatagramCallback(this);
  }

  sendPreface();
  codec_.setCallback(this);
}

void HTTPUniplexTransportSession::setRateLimitParams(
    RateLimiter::Type type,
    uint32_t maxEventsPerInterval,
    std::chrono::milliseconds intervalDuration) {
  if (rateLimitFilter_) {
    rateLimitFilter_->setRateLimitParams(
        type, maxEventsPerInterval, intervalDuration);
  }
}

void HTTPQuicCoroSession::onPriority(quic::StreamId streamId,
                                     const HTTPPriority& pri) {
  XCHECK(isDownstream());
  if (isDraining() || !quicSocket_->good()) {
    return;
  }

  if (!findStream(streamId)) {
    priorityUpdatesBuffer_.insert(streamId, pri);
    return;
  }

  quicSocket_->setStreamPriority(
      streamId, HTTPPriorityQueue::Priority(pri.urgency, pri.incremental));
}

/**
 * Calling sendPriority(pri) does both of the following:
 *  - Sends the priority_update frame to remote (server)
 *  - Sets the local egress priority to the same level
 */
size_t HTTPQuicCoroSession::sendPriority(quic::StreamId id, HTTPPriority pri) {
  XCHECK(isUpstream());

  if (!quic::isClientBidirectionalStream(id) || !findStream(id)) {
    XLOG(WARNING) << "sendPriority streamID = " << id
                  << " not found; sess=" << *this;
    return 0;
  }

  quicSocket_->setStreamPriority(
      id, HTTPPriorityQueue::Priority(pri.urgency, pri.incremental));

  auto ret = codec_->generatePriority(writeBuf_, id, pri);
  writeEvent_.signal();

  return ret;
}

size_t HTTPQuicCoroSession::sendPushPriority(uint64_t pushId,
                                             HTTPPriority pri) {
  XCHECK(isUpstream());

  // Find push stream
  auto found = pushStreamsAwaitingStreamID_.contains(pushId);

  // TODO: improve efficiency – this is linear with respect to streams_
  if (!found) {
    found = std::ranges::find_if(streams_, [=](auto& entry) {
              return quic::isServerUnidirectionalStream(entry.first) &&
                     entry.second->currentPushID &&
                     *entry.second->currentPushID == pushId;
            }) != streams_.cend();
  }

  if (!found) {
    XLOG(WARNING) << "sendPushPriority pushId = " << pushId
                  << " was not found; sess=" << *this;
    return 0;
  }

  auto ret = codec_->generatePushPriority(writeBuf_, pushId, pri);

  writeEvent_.signal();
  return ret;
}

bool HTTPQuicCoroSession::createControlStream(
    hq::UnidirectionalStreamType streamType,
    folly::IOBufQueue& writeBuf,
    quic::StreamId& id) {
  auto newId = quicSocket_->createUnidirectionalStream();
  if (!newId.hasError()) {
    id = *newId;
    auto res = hq::writeStreamPreface(writeBuf, uint64_t(streamType));
    if (!res.hasError()) {
      quicSocket_->setControlStream(id);
      writeEvent_.signal();
      return true;
    }
  }
  XLOG(ERR) << __func__ << " failed for type=" << uint64_t(streamType)
            << " sess=" << *this;
  connectionError(
      HTTPErrorCode::STREAM_CREATION_ERROR,
      folly::to<std::string>("Failed to create HTTP control stream, type=",
                             streamType));
  return false;
}

folly::coro::TaskWithExecutor<void> HTTPUniplexTransportSession::run() {
  return co_withExecutor(&readExec_, runImpl());
}

folly::coro::Task<void> HTTPUniplexTransportSession::runImpl() {
  XLOG(DBG6) << "starting run sess=" << *this;
  // Start the write loop asynchronously.  The writeLoop coroutine is not
  // chained for cancellation.  If readLoop detects cancellation, it will
  // trigger writeLoop to exit.
  co_withExecutor(&writeExec_, writeLoop()).start();

  co_await co_awaitTry(readLoop());

  // Wait for writes to complete
  co_await writesFinished_;
  XLOG(DBG6) << "waiting for outstanding refs sess=" << *this;
  // expects a post outside of evb (i.e. our readExec_ will trip a check here)
  co_await co_withExecutor(eventBase_, zeroRefs()).startInlineUnsafe();
  XLOG(DBG6) << "terminating run sess=" << *this;
  destroy();
}

void HTTPCoroSession::sendPreface() {
  codec_->generateConnectionPreface(writeBuf_);
  if (isUpstream()) {
    setSetting(SettingsId::ENABLE_PUSH, 1);
  }
  applyEgressSettings();
  codec_->generateSettings(writeBuf_);
  writeEvent_.signal();
}

void HTTPQuicCoroSession::applyEgressSettings() {
  for (auto& setting : multiCodec_->getEgressSettings()->getAllSettings()) {
    auto id = hq::httpToHqSettingsId(setting.id);
    if (id) {
      switch (*id) {
        case hq::SettingId::HEADER_TABLE_SIZE:
          multiCodec_->getQPACKCodec().setDecoderHeaderTableMaxSize(
              setting.value);
          break;
        case hq::SettingId::QPACK_BLOCKED_STREAMS:
          multiCodec_->getQPACKCodec().setMaxBlocking(setting.value);
          break;
        case hq::SettingId::MAX_HEADER_LIST_SIZE:
          break;
        case hq::SettingId::ENABLE_CONNECT_PROTOCOL:
        case hq::SettingId::H3_DATAGRAM:
        case hq::SettingId::H3_DATAGRAM_DRAFT_8:
        case hq::SettingId::H3_DATAGRAM_RFC:
          break;
        case hq::SettingId::ENABLE_WEBTRANSPORT:
        case hq::SettingId::WEBTRANSPORT_MAX_SESSIONS:
        case hq::SettingId::WT_INITIAL_MAX_DATA:
          // TODO
          break;
      }
    }
  }
}

HTTPCoroSession::StreamState& HTTPCoroSession::createNewStream(
    HTTPCodec::StreamID streamID, bool fromSendRequest) {
  auto res = streams_.emplace(
      streamID,
      std::make_unique<StreamState>(
          eventBase_.get(),
          streamID,
          *this,
          getStreamSendFlowControlWindow(),
          getStreamRecvFlowControlWindow(),
          fromSendRequest ? std::chrono::milliseconds(0) : streamReadTimeout_,
          writeTimeout_));
  XCHECK(res.second) << "Duplicate stream";
  XLOG(DBG4) << "Creating id=" << streamID << " sess=" << *this;
  if (sessionStats_) {
    sessionStats_->recordTransactionOpened();
    if (nextStreamSequenceNumber_ > 0) {
      sessionStats_->recordSessionReused();
    }
  }

  nextStreamSequenceNumber_++;
  if (!fromSendRequest) {
    transactionAttached();
  }
  setupStreamWriteBuf(*res.first->second, writeBuf_);
  return *res.first->second;
}

void HTTPUniplexTransportSession::setupStreamWriteBuf(
    StreamState& stream, folly::IOBufQueue& sessionWriteBuf) {
  stream.setWriteBuf(&sessionWriteBuf);
}

void HTTPQuicCoroSession::setupStreamWriteBuf(StreamState& stream,
                                              folly::IOBufQueue& /*unused*/) {
  stream.setWriteBuf(nullptr);
}

HTTPCoroSession::StreamState* HTTPCoroSession::findStream(
    HTTPCodec::StreamID id) {
  auto it = streams_.find(id);
  if (it == streams_.end()) {
    XLOG(DBG4) << "Stream not found sess=" << *this << " id=" << id;
    return nullptr;
  }
  return it->second.get();
}

void HTTPCoroSession::insertWithPriority(const StreamState& stream) {
  auto id = quic::PriorityQueue::Identifier::fromStreamID(stream.getID());
  auto pri = stream.getPriority();
  auto httpPri =
      HTTPPriorityQueue::Priority{pri.urgency, pri.incremental, pri.orderId};
  writableStreams_.insertOrUpdate(id, httpPri);
}

void HTTPCoroSession::onMessageBegin(HTTPCodec::StreamID streamID,
                                     HTTPMessage* /*msg*/) {
  XLOG(DBG6) << __func__ << " streamId=" << streamID << " sess=" << *this;
  auto it = streams_.find(streamID);
  if (it == streams_.end()) {
    if (isUpstream()) {
      // Headers on a stream that has already been reset?
      if (!codec_->supportsParallelRequests()) {
        connectionError(HTTPErrorCode::PROTOCOL_ERROR,
                        "HTTP response without request");
      }
      return;
    }
  } else {
    if (isDownstream()) {
      // The codecs shouldn't do this, but HQ manually invokes onMessageBegin
      XLOG_IF(DFATAL, codec_->getProtocol() != CodecProtocol::HQ)
          << "Duplicate stream sess=" << *this << " id=" << streamID;
    } // upstream, it can happen
    return;
  }

  // http/1.1 edge-case – read returned bytes after cancel; to prevent new
  // streams from being created, we simply pause the parser here
  if (!codec_->isReusable()) {
    codec_->setParserPaused(true);
    return;
  }

  deliverLifecycleEvent(&LifecycleObserver::onRequestBegin, *this);
  auto& stream = createNewStream(streamID);
  if (isDownstream()) {
    if (!codec_->supportsParallelRequests() && streams_.size() > 1) {
      codec_->setParserPaused(true);
    }
    // Start a task to read the response to this request (downstream only)
    // onMessageBegin can only be invoked once per downstream txn
    stream.startEgressCoro();
    co_withExecutor(
        eventBase_,
        co_withCancellation(stream.cs.egress.getToken(),
                            readResponse(stream, handleRequest(stream))))
        .start();
  }
}

void HTTPUniplexTransportSession::onPushMessageBegin(
    HTTPCodec::StreamID streamID,
    HTTPCodec::StreamID assocStreamID,
    HTTPMessage* /*promise*/) {
  XLOG(DBG6) << __func__ << " sess=" << *this;
  deliverLifecycleEvent(&LifecycleObserver::onRequestBegin, *this);
  if (findStream(streamID)) {
    XLOG(DBG4) << "Duplicate push sess=" << *this << " id=" << streamID;
    return;
  }

  auto parent = findStream(assocStreamID);
  if (!parent) {
    // Either the parent has been reset, or never existed.  Either way, this
    // push is going to fail, but it's handled in onHeadersComplete.
    return;
  }

  auto& pushStream = createNewStream(streamID);
  pushStream.initIngressPush(assocStreamID);
  pushStream.currentPushID = streamID;
  numPushStreams_++;
}

void HTTPQuicCoroSession::onPushMessageBegin(HTTPCodec::StreamID pushID,
                                             HTTPCodec::StreamID streamID,
                                             HTTPMessage* /*promise*/) {
  XLOG(DBG6) << __func__ << " sess=" << *this;
  deliverLifecycleEvent(&LifecycleObserver::onRequestBegin, *this);
  auto parent = findStream(streamID);
  // In HQ this comes from parsing on the actual parent stream
  XCHECK(parent);
  XCHECK(!parent->currentPushID);
  parent->currentPushID = pushID;
}

void HTTPCoroSession::onHeadersComplete(HTTPCodec::StreamID streamID,
                                        std::unique_ptr<HTTPMessage> msg
                                        /* bool eom!*/) {
  XLOG(DBG4) << "onHeadersComplete streamId=" << streamID << " sess=" << *this;
  msg->dumpMessage(4);
  deliverLifecycleEvent(
      &LifecycleObserver::onIngressMessage, *this, *msg.get());
  auto stream = findStream(streamID);
  if (!stream) {
    // Has been reset since onMessageBegin/onPushMessageBegin
    return;
  }
  if (checkAndHandlePushPromiseComplete(*stream, msg)) {
    return;
  }

  auto upgrade = stream->checkForUpgrade(*msg, /*isIngress=*/true);
  if (isDownstream() && upgrade && !codec_->supportsParallelRequests()) {
    codec_->setParserPaused(true);
    // TODO: pause reading from transport too?
  }
  if (isDownstream() && msg->isRequest() &&
      msg->getMethod() == HTTPMethod::HEAD) {
    stream->skipEgressContentLengthValidation();
  }

  // Upstream/Downstream receiving headers
  setSecureMsg(*msg, setupTransportInfo_);
  msg->setSeqNo(HTTPCodec::streamIDToSeqNo(codec_->getProtocol(), streamID));
  auto priority = httpPriorityFromHTTPMessage(*msg);
  stream->setPriority(priority.value_or(kDefaultPriority));
  stream->streamSource.headers(std::move(msg));
  streamHeadersComplete(*stream);
}

void HTTPQuicCoroSession::streamHeadersComplete(StreamState& stream) {
  if (isDatagramEnabled() && !datagramsBuffer_.empty()) {
    auto itr = datagramsBuffer_.find(stream.getID());
    if (itr != datagramsBuffer_.end()) {
      auto& vec = itr->second;
      for (auto& datagram : vec) {
        stream.streamSource.datagram(std::move(datagram));
      }
      datagramsBuffer_.erase(itr);
    }
  }
}

void HTTPQuicCoroSession::onHeadersComplete(HTTPCodec::StreamID streamID,
                                            std::unique_ptr<HTTPMessage> msg
                                            /* bool eom!*/) {
  auto priority = httpPriorityFromHTTPMessage(*msg);
  HTTPCoroSession::onHeadersComplete(streamID, std::move(msg));

  if (isDownstream()) {
    auto itr = priorityUpdatesBuffer_.find(streamID);
    if (itr != priorityUpdatesBuffer_.end()) {
      quicSocket_->setStreamPriority(
          streamID,
          HTTPPriorityQueue::Priority(itr->second.urgency,
                                      itr->second.incremental));
      priorityUpdatesBuffer_.erase(itr);
    } else if (priority) {
      onPriority(streamID, *priority);
    }
  }
}

bool HTTPUniplexTransportSession::checkAndHandlePushPromiseComplete(
    StreamState& stream, std::unique_ptr<HTTPMessage>& msg) {
  auto streamID = stream.getID();
  if (isUpstream() && stream.currentPushID) {
    // Upstream push awaiting promise headers
    stream.currentPushID.reset();

    XCHECK(stream.parent);
    auto parent = findStream(*stream.parent);
    if (!parent) {
      XLOG(ERR) << "Received PUSH_PROMISE but parent stream is gone. Parent "
                << "stream=" << *stream.parent << " push stream=" << streamID
                << " sess=" << *this;
      egressResetStream(streamID, &stream, HTTPErrorCode::REFUSED_STREAM);
    } else {
      stream.streamSourceActive();
      parent->streamSource.pushPromise(std::move(msg), &stream.streamSource);
    }
    return true;
  }
  return false;
}

bool HTTPQuicCoroSession::checkAndHandlePushPromiseComplete(
    StreamState& stream, std::unique_ptr<HTTPMessage>& msg) {
  if (!isUpstream() || !stream.currentPushID ||
      !quic::isClientBidirectionalStream(stream.getID())) {
    // Just regular headers
    return false;
  }
  // This stream is awaiting the push promise
  auto pushID = *stream.currentPushID;
  stream.currentPushID.reset();
  StreamState* pushStreamPtr;
  auto it = pushStreamsAwaitingPromises_.find(pushID);
  if (it == pushStreamsAwaitingPromises_.end()) {
    // stream has not yet arrived
    XLOG(DBG4) << "Push promise complete before stream, pushID=" << pushID
               << " parent=" << stream.getID() << " sess=" << *this;
    auto pushStream =
        std::make_unique<StreamState>(eventBase_.get(),
                                      HTTPCodec::MaxStreamID,
                                      *this,
                                      getStreamSendFlowControlWindow(),
                                      getStreamRecvFlowControlWindow(),
                                      streamReadTimeout_,
                                      writeTimeout_);
    pushStreamPtr = pushStream.get();
    pushStreamsAwaitingStreamID_.emplace(pushID, std::move(pushStream));
  } else {
    auto pushStreamID = it->second;
    XLOG(DBG4) << "Push promise matched stream=" << pushStreamID
               << " pushID=" << pushID << " parent=" << stream.getID()
               << " sess=" << *this;
    pushStreamsAwaitingPromises_.erase(it);
    auto res = quicSocket_->resumeRead(pushStreamID);
    if (res.hasError()) {
      XLOG(ERR) << "Failed to resume push stream=" << pushStreamID
                << " sess=" << *this;
      // This push promise just gets silently abandoned
      return true;
    } else {
      auto& pushStream = createNewStream(pushStreamID);
      multiCodec_->addCodec(pushStreamID);
      pushStreamPtr = &pushStream;
      numPushStreams_++;
      co_withExecutor(eventBase_,
                      co_withCancellation(pushStream.cs.ingress.getToken(),
                                          readLoop(pushStreamID)))
          .start();
    }
  }
  pushStreamPtr->initIngressPush(stream.getID());
  pushStreamPtr->streamSourceActive();
  pushStreamPtr->currentPushID = pushID;
  stream.streamSource.pushPromise(std::move(msg), &pushStreamPtr->streamSource);
  return true;
}

void HTTPQuicCoroSession::dispatchPushStream(quic::StreamId id,
                                             uint64_t pushID) {
  multiCodec_->onIngressPushId(pushID);
  auto it = pushStreamsAwaitingStreamID_.find(pushID);
  if (it == pushStreamsAwaitingStreamID_.end()) {
    // promise has not arrived yet, pause the push stream
    XLOG(DBG4) << "Push stream before push promise, pushID=" << pushID
               << " stream=" << id << " sess=" << *this;
    quicSocket_->pauseRead(id);
    // TODO: duplicate possibility?
    pushStreamsAwaitingPromises_.emplace(pushID, id);
  } else {
    auto pushStream = std::move(it->second);
    XCHECK(pushStream->parent);
    XLOG(DBG4) << "Push stream matched promise, stream id=" << id
               << " pushID=" << pushID << " parent=" << *pushStream->parent
               << " sess=" << *this;
    pushStream->streamSource.setStreamID(id);
    multiCodec_->addCodec(id);
    numPushStreams_++;
    co_withExecutor(
        &readExec_,
        co_withCancellation(pushStream->cs.ingress.getToken(), readLoop(id)))
        .start();
    streams_.emplace(id, std::move(pushStream));
    pushStreamsAwaitingStreamID_.erase(it);
  }
}

void HTTPCoroSession::onBody(HTTPCodec::StreamID streamID,
                             std::unique_ptr<folly::IOBuf> chain,
                             uint16_t padding
                             /* bool eom!*/) {
  XLOG(DBG6) << __func__ << " streamId=" << streamID << " sess=" << *this;
  BufQueue body(std::move(chain));
  auto length = body.chainLength();
  if (!recvWindow_.reserve(length, padding)) {
    // flow control error
    connectionError(HTTPErrorCode::FLOW_CONTROL_ERROR,
                    "Peer exceeded connection flow control");
    return;
  }

  SESS_STATS(recordPendingBufferedReadBytes, static_cast<int64_t>(length));

  auto* stream = findStream(streamID);
  HTTPStreamSource::FlowControlState fcRes =
      stream ? stream->streamSource.body(std::move(body), padding)
             : HTTPStreamSource::FlowControlState::ERROR;

  if (fcRes == HTTPStreamSource::FlowControlState::ERROR) {
    // release conn flow control if stream doesn't exist or peer violated stream
    // fc window
    bytesProcessed(streamID, length, 0);
  }

  if (stream && ingressLimitExceeded(*stream)) {
    handleIngressLimitExceeded(stream->getID());
  }
}

void HTTPUniplexTransportSession::handleIngressLimitExceeded(
    HTTPCodec::StreamID) {
  // only applies to h1 since h2 has session fc protocol semantics
  if (!codec_->supportsSessionFlowControl()) {
    flowControlBaton_.reset();
  }
}

void HTTPQuicCoroSession::handleIngressLimitExceeded(HTTPCodec::StreamID id) {
  // pause reading from socket if bytes buffered exceeded threshold
  quicSocket_->pauseRead(id);
}

void HTTPCoroSession::onTrailersComplete(
    HTTPCodec::StreamID streamID, std::unique_ptr<HTTPHeaders> trailers) {
  XLOG(DBG6) << __func__ << " streamId=" << streamID << " sess=" << *this;
  auto stream = findStream(streamID);
  if (!stream) {
    return;
  }
  stream->streamSource.trailers(std::move(trailers));
}

void HTTPCoroSession::onMessageComplete(HTTPCodec::StreamID streamID,
                                        bool upgrade) {
  XLOG(DBG6) << __func__ << " streamId=" << streamID
             << " upgrade=" << int(upgrade) << " sess=" << *this;
  auto stream = findStream(streamID);
  if (!stream) {
    return;
  }

  // TODO: upgrade

  // TODO: what about receiving an EOM (or headers/body for that
  // matter) on an egress push stream?  streamSource.eom will ignore
  // it, but shouldn't we return an error / drop the connection?

  // It will mess up the logic in decrementPushStreamCount, because it will
  // treat this as a decrementing event.  For now, gate
  // decrementPushStreamCount to upstream only
  if (isUpstream()) {
    decrementPushStreamCount(*stream);
  }
  if (!upgrade) {
    stream->streamSource.eom();
  } // else, this is a 200 reply to a CONNECT?  Or WS?

  // handle http/1.1 keep-alive=false
  if (isUpstream() && !codec_->isReusable() &&
      !codec_->supportsParallelRequests()) {
    initiateDrain();
  }
}

void HTTPCoroSession::onError(HTTPCodec::StreamID streamID,
                              const HTTPException& error,
                              bool /*newTxn*/) {
  XLOG(DBG4) << "Parse error, ex=" << error.what() << " id=" << streamID
             << " sess=" << *this;
  if (streamID == getSessionStreamID()) {
    // TODO: HTTPSession reports kErrorMessage here, but that doesn't
    // seem right for H2.
    deliverLifecycleEvent(
        &LifecycleObserver::onIngressError, *this, kErrorMessage);

    if (error.hasCodecStatusCode()) {
      connectionError(ErrorCode2HTTPErrorCode(error.getCodecStatusCode()),
                      error.what());
    } else if (error.hasHttp3ErrorCode()) {
      connectionError(HTTP3ErrorCode2HTTPErrorCode(error.getHttp3ErrorCode()),
                      error.what());
    } else {
      // Session errors, only comes from H2/QCodecs, with an error code
      XLOG(FATAL) << "Session error with no protocol error code sess=" << *this;
    }
    return;
  }

  // queue up an error for the handler if stream exists
  auto ec = HTTPException2HTTPErrorCode(error);
  auto* stream = findStream(streamID);
  if (isDownstream() && stream && stream->canSendHeaders() &&
      error.hasHttpStatusCode()) {
    stream->abortIngress(ec, error.what());
    stream->setErrorStatusCode(error.getHttpStatusCode());
  } else {
    egressResetStream(streamID, stream, ec);
  }

  deliverLifecycleEvent(&LifecycleObserver::onIngressError,
                        *this,
                        error.hasProxygenError() ? error.getProxygenError()
                                                 : kErrorMessage);
}

void HTTPCoroSession::onAbort(HTTPCodec::StreamID streamID, ErrorCode code) {
  onResetStream(streamID,
                (code == ErrorCode::NO_ERROR) ? HTTPErrorCode::CANCEL
                                              : ErrorCode2HTTPErrorCode(code));
}

void HTTPCoroSession::onResetStream(HTTPCodec::StreamID streamID,
                                    HTTPErrorCode code) {
  auto stream = findStream(streamID);
  if (!stream) {
    return;
  }
  XLOG(DBG4) << "Received RST_STREAM code=" << static_cast<uint32_t>(code)
             << " sess=" << *this << " id=" << streamID;
  decrementPushStreamCount(*stream);
  deliverAbort(*stream, code, "received RST_STREAM from peer");
}

void HTTPCoroSession::deliverAbort(StreamState& stream,
                                   HTTPErrorCode error,
                                   std::string_view details) {
  // This abort may never be read if the stream is streamSourceComplete.
  // eg: a complete downstream request or downstream push.
  stream.abortIngress(error, details);
}

void HTTPUniplexTransportSession::deliverAbort(StreamState& stream,
                                               HTTPErrorCode error,
                                               std::string_view details) {
  HTTPCoroSession::deliverAbort(stream, error, details);
  // HTTP/2 only, always reset the egress state here
  // If there's still pending egress (eg readResponse is reading from a source),
  // resetStreamState will cancel the coroutine.
  resetStreamState(stream, HTTPError(error, "Peer reset the stream"));
}

void HTTPQuicCoroSession::deliverAbort(StreamState& stream,
                                       HTTPErrorCode error,
                                       std::string_view details) {
  HTTPCoroSession::deliverAbort(stream, error, details);
  // HTTP/3, egress a reset if egress is incomplete
  if (!stream.isEgressComplete()) {
    HTTPErrorCode egressError = HTTPErrorCode::REQUEST_CANCELLED;
    if (isDownstream() && stream.streamSource.isUnprocessed()) {
      // Server has not touched the request
      egressError = HTTPErrorCode::REQUEST_REJECTED;
    }
    // TODO: the egress error message to onByteEventCanceled will be wrong
    egressResetStream(stream.getID(), &stream, egressError);
  }
}

void HTTPCoroSession::onGoaway(uint64_t lastGoodStreamID,
                               ErrorCode code,
                               std::unique_ptr<folly::IOBuf> /*debugData*/) {
  XLOG(DBG4) << __func__ << " lastGoodStreamID=" << lastGoodStreamID
             << " code=" << int(code) << " sess=" << *this;
  // TODO: Queue an onGoaway event for all active transactions
  //       Do something with debugData?

  XCHECK(codec_->supportsParallelRequests()) << "GOAWAY is for parallel codecs";

  // Notify the interested parties
  deliverLifecycleEvent(
      &LifecycleObserver::onGoaway, *this, lastGoodStreamID, code);

  // Mark all transactions above lastGoodStreamID as REFUSED
  std::vector<StreamState*> refusedStreams;
  for (auto& [_, stream] : streams_) {
    if (streamRefusedByGoaway(*stream, lastGoodStreamID)) {
      refusedStreams.push_back(stream.get());
    }
  }

  for (auto* stream : refusedStreams) {
    // This stream never reached the peer
    constexpr std::string_view kRefusedStreamMsg = "Stream refused by peer";
    stream->abortIngress(HTTPErrorCode::REFUSED_STREAM, kRefusedStreamMsg);
    resetStreamState(*stream,
                     HTTPError(HTTPErrorCode::REFUSED_STREAM,
                               std::string(kRefusedStreamMsg)));
  }

  initiateDrain();
  if (code != ErrorCode::NO_ERROR) {
    // The peer errored the connection, reset all open streams
    resetOpenStreams(ErrorCode2HTTPErrorCode(code),
                     "Peer closed with connection error");
    codec_->generateImmediateGoaway(writeBuf_);
    writeEvent_.signal();
    // ingress callback, no need to interrupt reads
  }
}

bool HTTPUniplexTransportSession::streamRefusedByGoaway(
    StreamState& stream, HTTPCodec::StreamID lastGoodStreamID) {
  return (HTTPParallelCodec::isInitiatedStream(direction_, stream.getID()) &&
          stream.getID() > lastGoodStreamID);
}

bool HTTPQuicCoroSession::streamRefusedByGoaway(
    StreamState& stream, HTTPCodec::StreamID lastGoodStreamID) {
  return (isUpstream() && quicSocket_->isClientStream(stream.getID()) &&
          stream.getID() >= lastGoodStreamID) ||
         (isDownstream() && quicSocket_->isServerStream(stream.getID()) &&
          stream.currentPushID && *stream.currentPushID >= lastGoodStreamID);
}

void HTTPCoroSession::drainStarted() {
  XLOG(DBG4)
      << "Drain started, setting maxConcurrentOutgoingStreamsConfig_ to 0 "
      << "sess=" << *this;
  const bool wasDraining = isDraining();
  maxConcurrentOutgoingStreamsConfig_ = 0;
  if (isDraining() && !wasDraining) {
    deliverLifecycleEvent(&LifecycleObserver::onDrainStarted, *this);
  }
}

void HTTPCoroSession::connectionError(
    HTTPErrorCode httpError,
    std::string msg,
    folly::Optional<HTTPErrorCode> streamError) {
  XLOG(DBG4) << "connectionError msg=" << msg << " sess=" << *this;
  drainStarted();
  handleConnectionError(httpError, msg);
  resetOpenStreams(streamError.value_or(httpError), msg);
  writeEvent_.signal();
  interruptReadLoop();
  XCHECK(!codec_->isWaitingToDrain());
  writableStreams_.clear();
}

void HTTPUniplexTransportSession::handleConnectionError(HTTPErrorCode httpError,
                                                        std::string msg) {
  HTTPError err(httpError, msg);
  byteEventObserver_.cancelEvents(err);
  codec_->generateImmediateGoaway(writeBuf_,
                                  HTTPErrorCode2ErrorCode(httpError, false),
                                  folly::IOBuf::copyBuffer(msg));
  if (!codec_->supportsParallelRequests()) {
    resetAfterDrainingWrites_ = true;
  }
}

void HTTPQuicCoroSession::handleConnectionError(HTTPErrorCode error,
                                                std::string msg) {
  codec_->generateImmediateGoaway(writeBuf_,
                                  ErrorCode::PROTOCOL_ERROR, // ignored
                                  nullptr);                  // ignored
  connectionError_.emplace(
      quic::ApplicationErrorCode(HTTPErrorCode2HTTP3ErrorCode(error, false)),
      std::move(msg));
}

void HTTPCoroSession::initiateDrain() {
  XLOG(DBG4) << __func__ << " sess=" << *this;
  if (isDraining()) {
    return;
  }
  drainStarted();
  // Even if generateGoaway doesn't produce egress, it may change 'isReusable'
  // So signal the writeEvent_.
  codec_->generateGoaway(writeBuf_);
  writeEvent_.signal();
  interruptReadLoop();
  if (codec_->isWaitingToDrain()) {
    scheduleGoawayTimeout();
  }
}

void HTTPCoroSession::closeWhenIdle() {
  XLOG(DBG4) << __func__ << " sess=" << *this;
  goawayTimeoutExpired();
  goawayTimeout_.cancelTimeout();
}

void HTTPCoroSession::goawayTimeoutExpired() {
  // This is the equivalent of HTTPSession::closeWhenIdle
  drainStarted();
  codec_->generateImmediateGoaway(writeBuf_);
  writeEvent_.signal();
  interruptReadLoop();
}

void HTTPCoroSession::dropConnection(const std::string& errorMsg) {
  XLOG(DBG4) << __func__ << " sess=" << *this;

  // TODO: dropping a connection with open streams will almost certainly not
  // complete within the current event loop, which can be problematic.

  // Even if there are NO streams, it will only complete in the current loop if
  // the writeLoop finishes before readLoop.  If readLoop was already scheduled
  // to run, it takes 1 more loop.  This is because TimedBaton::wait does not
  // return true from await_ready() if it's already signalled.
  connectionError(
      HTTPErrorCode::INTERNAL_ERROR, errorMsg, HTTPErrorCode::DROPPED);
}

int HTTPUniplexTransportSession::getAsyncTransportFD() const {
  folly::AsyncSocket* sock = nullptr;
  auto transport = coroTransport_->getTransport();
  if (transport) {
    sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
  }
  if (sock) {
    return sock->getNetworkSocket().toFd();
  }
  return -1;
}

bool HTTPCoroSession::getCurrentTransportInfo(wangle::TransportInfo* tinfo,
                                              bool setupFields) const {
  if (setupFields) {
    // some fields are the same with the setup transport info
    tinfo->setupTime = setupTransportInfo_.setupTime;
    tinfo->secure = setupTransportInfo_.secure;
    tinfo->sslSetupTime = setupTransportInfo_.sslSetupTime;
    tinfo->sslVersion = setupTransportInfo_.sslVersion;
    tinfo->sslCipher = setupTransportInfo_.sslCipher;
    tinfo->sslResume = setupTransportInfo_.sslResume;
    tinfo->appProtocol = setupTransportInfo_.appProtocol;
    tinfo->sslError = setupTransportInfo_.sslError;
    tinfo->sslServerName = setupTransportInfo_.sslServerName;
  }
  return getCurrentTransportInfoImpl(tinfo);
}

bool HTTPUniplexTransportSession::getCurrentTransportInfoImpl(
    wangle::TransportInfo* tinfo) const {
  folly::AsyncSocket* sock = nullptr;
  auto transport = coroTransport_->getTransport();
  if (transport) {
    sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
  }
  if (sock) {
    tinfo->initWithSocket(sock);
#if defined(__linux__) || defined(__FreeBSD__)
    tinfo->readTcpCongestionControl(sock);
    tinfo->readMaxPacingRate(sock);
    tinfo->recvwnd = tinfo->tcpinfo.tcpi_rcv_space
                     << tinfo->tcpinfo.tcpi_rcv_wscale;
#elif defined(__APPLE__)
    tinfo->recvwnd = tinfo->tcpinfo.tcpi_rcv_wnd
                     << tinfo->tcpinfo.tcpi_rcv_wscale;
#endif // defined(__linux__) || defined(__FreeBSD__)
    tinfo->totalBytes = sock->getRawBytesWritten();
    return true;
  }
  return false;
}

bool HTTPQuicCoroSession::getCurrentTransportInfoImpl(
    wangle::TransportInfo* tinfo) const {
  if (quicSocket_->good()) {
    auto connState = quicSocket_->getState();
    auto quicTransportInfo = quicSocket_->getTransportInfo();

    tinfo->rtt = quicTransportInfo.srtt;
    tinfo->rtt_var = quicTransportInfo.rttvar.count();
    tinfo->rtx = quicTransportInfo.packetsRetransmitted;
    tinfo->rtx_tm = quicTransportInfo.totalPacketsMarkedLostByTimeout;
    tinfo->rto = quicTransportInfo.pto.count();
    tinfo->cwndBytes = int64_t(quicTransportInfo.congestionWindow);
    tinfo->mss = int64_t(quicTransportInfo.mss);
    tinfo->cwnd = tinfo->cwndBytes / tinfo->mss;
    tinfo->totalBytes = int64_t(quicTransportInfo.bytesSent);
    updateQuicProtocolInfo(*currentQuicProtocolInfo_, *quicSocket_);
    tinfo->protocolInfo = currentQuicProtocolInfo_;
    tinfo->validTcpinfo = true;

    if (connState) {
      tinfo->recvwnd = int64_t(connState->flowControlState.windowSize);

      if (connState->congestionController) {
        tinfo->caAlgo = quic::congestionControlTypeToString(
            connState->congestionController->type());
        if (connState->congestionController->type() ==
            quic::CongestionControlType::Cubic) {
          quic::CongestionControllerStats ccStats;
          connState->congestionController->getStats(ccStats);
          tinfo->ssthresh = int64_t(ccStats.cubicStats.ssthresh) / tinfo->mss;
        }
      }
    }
    return true;
  }
  return false;
}

void HTTPUniplexTransportSession::onWindowUpdate(HTTPCodec::StreamID streamID,
                                                 uint32_t amount) {
  // for freeing up send window, either connection or stream
  if (streamID == getSessionStreamID()) {
    auto wasBlocked = (sendWindow_.getSize() <= 0);
    if (!sendWindow_.free(amount)) {
      connectionError(HTTPErrorCode::FLOW_CONTROL_ERROR,
                      "Peer overflowed connection window");
      return;
    }
    auto isBlocked = (sendWindow_.getSize() <= 0);
    if (wasBlocked && !isBlocked && !writableStreams_.empty()) {
      // More connection flow control could unblock writing
      writeEvent_.signal();
    }
  } else {
    auto stream = findStream(streamID);
    if (!stream) {
      return;
    }
    auto res = stream->onWindowUpdate(amount);
    if (res.hasError()) {
      // TODO: should this be a stream error?
      connectionError(HTTPErrorCode::FLOW_CONTROL_ERROR,
                      folly::to<std::string>(
                          "Peer overflowed stream window, stream=", streamID));
      return;
    } else if (res.value()) {
      XLOG(DBG4) << "Adding previously blocked stream to writable id="
                 << streamID << " sess=" << *this;
      notifyBodyWrite(*stream);
    }
  }
}

void HTTPUniplexTransportSession::onSettings(const SettingsList& settings) {
  deliverLifecycleEvent(&LifecycleObserver::onSettings, *this, settings);

  for (auto& setting : settings) {
    if (setting.id == SettingsId::INITIAL_WINDOW_SIZE) {
      for (auto& it : streams_) {
        it.second->setSendWindow(setting.value);
      }
    } else if (setting.id == SettingsId::MAX_CONCURRENT_STREAMS) {
      auto maxStreams = setting.value;
      XLOG(DBG4) << "Got new max number of concurrent streams we can initiate: "
                 << maxStreams << " sess=" << *this;
      bool didSupport = supportsMoreTransactions();
      maxConcurrentOutgoingStreamsRemote_ = maxStreams;
      onSetMaxInitiatedStreams(didSupport);
    } else if (setting.id == SettingsId::SETTINGS_HTTP_CERT_AUTH) {
      // TODO
    }
  }

  codec_->generateSettingsAck(writeBuf_);
  writeEvent_.signal();
}

void HTTPQuicCoroSession::onSettings(const SettingsList& settings) {
  deliverLifecycleEvent(&LifecycleObserver::onSettings, *this, settings);
  uint32_t tableSize = hq::kDefaultIngressHeaderTableSize;
  uint32_t blocked = hq::kDefaultIngressQpackBlockedStream;
  for (auto& setting : settings) {
    auto id = hq::httpToHqSettingsId(setting.id);
    if (id) {
      switch (*id) {
        case hq::SettingId::HEADER_TABLE_SIZE:
          tableSize = setting.value;
          break;
        case hq::SettingId::QPACK_BLOCKED_STREAMS:
          blocked = setting.value;
          break;
        case hq::SettingId::MAX_HEADER_LIST_SIZE:
          // this setting is stored in ingressSettings_ and enforced in the
          // StreamCodec
          break;
        case hq::SettingId::ENABLE_CONNECT_PROTOCOL:
        case hq::SettingId::H3_DATAGRAM:
        case hq::SettingId::H3_DATAGRAM_DRAFT_8:
        case hq::SettingId::H3_DATAGRAM_RFC:
          // If H3 datagram is enabled but datagram was not negotiated at the
          // transport, close the connection
          if (static_cast<bool>(setting.value) &&
              quicSocket_->getDatagramSizeLimit() == 0) {
            connectionError(HTTPErrorCode::SETTINGS_ERROR,
                            "H3_DATAGRAM without transport support");
            return;
          }
          break;
        case hq::SettingId::ENABLE_WEBTRANSPORT:
        case hq::SettingId::WEBTRANSPORT_MAX_SESSIONS:
        case hq::SettingId::WT_INITIAL_MAX_DATA:
          // TODO
          break;
      }
    }
  }
  multiCodec_->getQPACKCodec().setEncoderHeaderTableSize(tableSize);
  multiCodec_->getQPACKCodec().setMaxVulnerable(blocked);
  writeEvent_.signal();
  XLOG(DBG3) << "Applied SETTINGS sess=" << *this << " size=" << tableSize
             << " blocked=" << blocked;
}

void HTTPCoroSession::onSetMaxInitiatedStreams(bool didSupport) {
  if (didSupport != supportsMoreTransactions()) {
    if (didSupport) {
      deliverLifecycleEvent(&LifecycleObserver::onSettingsOutgoingStreamsFull,
                            *this);
    } else {
      deliverLifecycleEvent(
          &LifecycleObserver::onSettingsOutgoingStreamsNotFull, *this);
    }
  }
}

void HTTPUniplexTransportSession::onSettingsAck() {
  deliverLifecycleEvent(&LifecycleObserver::onSettingsAck, *this);
}

// HTTPStreamSource::Callback
void HTTPCoroSession::bytesProcessed(HTTPCodec::StreamID id,
                                     size_t delta,
                                     size_t toAckStream) {
  bool pendingWrite = false;
  if (toAckStream) {
    pendingWrite |= sendFlowControlUpdate(id, toAckStream);
  }
  auto toAckSession = recvWindow_.processed(delta);
  if (toAckSession) {
    pendingWrite |= sendFlowControlUpdate(0, toAckSession);
  }
  if (pendingWrite) {
    writeEvent_.signal();
  }
  SESS_STATS(recordPendingBufferedReadBytes, -static_cast<int64_t>(delta));
}

void HTTPUniplexTransportSession::bytesProcessed(HTTPCodec::StreamID id,
                                                 size_t delta,
                                                 size_t toAckStream) {
  HTTPCoroSession::bytesProcessed(id, delta, toAckStream);
  if (!codec_->supportsSessionFlowControl()) {
    auto stream = findStream(id);
    if (stream && shouldResumeIngress(*stream, delta)) {
      XLOG(DBG4) << "resuming stream previously ingress limited; sess="
                 << *this;
      flowControlBaton_.signal();
    }
  }
}

void HTTPQuicCoroSession::bytesProcessed(HTTPCodec::StreamID id,
                                         size_t delta,
                                         size_t toAckStream) {
  HTTPCoroSession::bytesProcessed(id, delta, toAckStream);

  auto stream = findStream(id);
  if (stream && shouldResumeIngress(*stream, delta)) {
    XLOG(DBG4) << "resuming stream previously ingress limited; sess=" << *this;
    // We were previously read limited and now we're not, so we can resume the
    // read loop only if we're not qpack blocked.
    if (multiCodec_->setCurrentStream(id) && !multiCodec_->isParserPaused()) {
      quicSocket_->resumeRead(id);
    }
  }
}

bool HTTPUniplexTransportSession::sendFlowControlUpdate(HTTPCodec::StreamID id,
                                                        size_t delta) {
  return codec_->generateWindowUpdate(writeBuf_, id, delta);
}

void HTTPCoroSession::sourceComplete(HTTPCodec::StreamID id,
                                     folly::Optional<HTTPError> error) {
  XLOG(DBG6) << __func__ << " sess=" << *this << " id=" << id
             << " error=" << (error.has_value() ? error->describe() : "");
  auto stream = CHECK_NOTNULL(findStream(id));
  stream->markStreamSourceComplete();
  if (stream->streamSource.isEOMSeen() && !error) {
    checkForDetach(*stream);
    return;
  }
  XCHECK(error) << "StreamSource must supply an error when source complete "
                << "but not EOM seen";
  if (isDownstream() && stream->canSendHeaders() &&
      (stream->errorStatusCode || error->code == HTTPErrorCode::READ_TIMEOUT)) {
    // If there's an error source, don't reset, otherwise it's a timeout
    if (!stream->errorStatusCode) {
      stream->setErrorStatusCode(408);
    }
  } else if (!stream->streamSource.isEOMSeen() || !stream->isEgressComplete()) {
    // Need to send a RST_STREAM
    auto egressErrorCode = sourceCompleteErr2ErrorCode(error->code);
    if (!stream->isUpgraded() ||
        egressErrorCode == HTTPErrorCode::FLOW_CONTROL_ERROR ||
        egressErrorCode == HTTPErrorCode::PROTOCOL_ERROR) {
      if (isNoError(egressErrorCode) && isDownstream()) {
        // delay STOP_SENDING w/ NO_ERROR until egress EOM (except for
        // http/1.1)
        stream->setDeferredStopSending(codec_->supportsParallelRequests());
      } else {
        // egress bidirectional reset
        egressResetStream(id,
                          stream,
                          isNoError(egressErrorCode) ? HTTPErrorCode::CANCEL
                                                     : egressErrorCode);
      }
    } else {
      // Abandoned CONNECT streams are a no-op
      checkForDetach(*stream);
    }
  } else {
    // The stream is complete, but there's nothing to egress on the wire.
    XLOG(DBG4) << "Clearing stream state for stream=" << id
               << " sess=" << *this;
    resetStreamState(*stream, *error);
  }
  writeEvent_.signal();
}

void HTTPUniplexTransportSession::handleDeferredStopSending(
    HTTPCodec::StreamID id) {
  generateResetStream(id, HTTPErrorCode::NO_ERROR, /*fromSource=*/false);
  writeEvent_.signal();
}

void HTTPQuicCoroSession::handleDeferredStopSending(HTTPCodec::StreamID id) {
  generateStopSending(id, HTTPErrorCode::NO_ERROR, /*fromSource=*/false);
}

void HTTPCoroSession::egressFinished(StreamState& stream) {
  XLOG(DBG4) << "egress finished for stream id=" << stream.getID()
             << " sess=" << *this;
  decrementPushStreamCount(stream,
                           /*eomMarkedEgressComplete=*/true);
  deliverLifecycleEvent(&LifecycleObserver::onRequestEnd, *this, 0);
  if (stream.getDeferredStopSending()) {
    handleDeferredStopSending(stream.getID());
  }
  checkForDetach(stream);
}

folly::coro::Task<HTTPSourceHolder> HTTPCoroSession::handleRequest(
    StreamState& stream) {
  XLOG(DBG6) << "starting handleRequest id=" << stream.getID()
             << " sess=" << *this;
  HTTPSourceHolder responseSource{nullptr};
  // Don't invoke handleRequest if egress is already complete
  if (!stream.isEgressComplete()) {
    stream.streamSourceActive();
    auto res = co_await co_awaitTry(handler_->handleRequest(
        eventBase_.get(), acquireKeepAlive(), &stream.streamSource));
    if (auto* ex = res.tryGetExceptionObject()) {
      XLOG(DBG4) << "Handler generated exception: " << ex->what();
      responseSource = getErrorResponse(500, ex->what());
    } else {
      responseSource = std::move(res.value());
    }
  }
  co_return responseSource;
}

folly::coro::Task<void> HTTPCoroSession::readResponse(
    StreamState& stream,
    folly::coro::Task<HTTPSourceHolder> getRespSourceTask) {
  auto cleanup = [this, &stream] {
    stream.markEgressCoroComplete();
    checkForDetach(stream);
  };
  auto g = folly::makeGuard(cleanup);

  auto responseSource = co_await std::move(getRespSourceTask);
  if (stream.isEgressComplete()) {
    XLOG(DBG4)
        << "readResponse egressComplete responseSource=" << int(responseSource)
        << " id=" << stream.getID() << " sess=" << *this;
    co_return;
  }
  if (!responseSource) {
    XLOG(DBG3) << "Handler did not provide a response source, returning 500"
               << " id=" << stream.getID() << " sess=" << *this;
    responseSource.setSource(getErrorResponse(500));
  }
  stream.setEgressSource(responseSource.release());

  const auto& wcs = stream.cs.egress;
  ResponseState responseState = ResponseState::HEADERS;
  do {
    responseState = processResponseHeaderEvent(
        stream,
        co_await co_awaitTry(
            co_withCancellation(wcs.getToken(), stream.nextHeaderEvent())));
    if (responseState == ResponseState::DONE) {
      co_return;
    }
  } while (responseState == ResponseState::HEADERS);
  XLOG(DBG6) << "Done generating headers sess=" << *this;
  g.dismiss();
  co_withExecutor(eventBase_,
                  co_withCancellation(wcs.getToken(),
                                      transferBody(stream, std::move(cleanup))))
      .startInlineUnsafe();
}

HTTPCoroSession::ResponseState HTTPCoroSession::processResponseHeaderEvent(
    StreamState& stream, folly::Try<HTTPHeaderEvent> headerEvent) {
  if (stream.isEgressComplete()) {
    return ResponseState::DONE;
  }

  auto switchToErrSource = [&](HTTPErrorCode ec) {
    XLOG(DBG4) << "switchToErrSource sc=" << stream.errorStatusCode;
    stream.stopReading(ec);
    stream.setEgressSource(
        getErrorResponse(std::exchange(stream.errorStatusCode, 0)));
    return ResponseState::HEADERS;
  };

  if (headerEvent.hasException()) {
    auto ex = getHTTPError(headerEvent);
    XLOG(DBG4) << "Error getting response headers sess=" << *this
               << " ex=" << ex.msg;
    if (stream.errorStatusCode) {
      return switchToErrSource(ex.code);
    }
    stream.stopReading(ex.code);
    egressResetStream(stream.getID(), &stream, ex.code, true);
    return ResponseState::DONE;
  }

  const auto& headers = *headerEvent->headers;
  if (stream.errorStatusCode &&
      headers.getStatusCode() != stream.errorStatusCode) {
    return switchToErrSource(HTTPErrorCode::INTERNAL_ERROR);
  }

  XLOG(DBG4) << "Got response headers eom=" << uint32_t(headerEvent->eom)
             << " sess=" << *this;
  headers.dumpMessage(4);
  auto upgrade = stream.checkForUpgrade(headers, /*isIngress=*/false);
  if (upgrade & !codec_->supportsParallelRequests()) {
    codec_->setParserPaused(false);
  }

  if (auto pri = headerEvent->headers->getHTTPPriority()) {
    onPriority(stream.getID(), *pri);
  }

  HTTPHeaderSize size;
  codec_->generateHeader(
      stream.getWriteBuf(), stream.getID(), headers, headerEvent->eom, &size);
  stream.addToStreamOffset(size.compressed);
  HTTPByteEvent::FieldSectionInfo fsInfo = {
      HTTPByteEvent::FieldSectionInfo::Type::HEADERS,
      headerEvent->isFinal(),
      size};
  registerByteEvents(stream.getID(),
                     stream.getStreamOffset(),
                     fsInfo,
                     /*bodyOffset=*/0,
                     std::move(headerEvent->byteEventRegistrations),
                     headerEvent->eom);
  if (headerEvent->egressHeadersFn) {
    headerEvent->egressHeadersFn(size);
  }
  notifyHeaderWrite(stream, headerEvent->eom);
  if (headerEvent->eom) {
    return ResponseState::DONE;
  }
  return !headerEvent->isFinal() ? ResponseState::HEADERS : ResponseState::BODY;
}

void HTTPUniplexTransportSession::notifyHeaderWrite(StreamState& stream,
                                                    bool eom) {
  if (eom) {
    stream.markEgressComplete();
    egressFinished(stream);
  }
  writeEvent_.signal();
}

void HTTPQuicCoroSession::notifyHeaderWrite(StreamState& stream, bool eom) {
  notifyBodyWrite(stream);
  if (eom) {
    stream.setPendingEgressEOM();
  }
}

bool HTTPQuicCoroSession::handleWrite(HTTPCodec::StreamID id,
                                      folly::IOBufQueue& writeBuf,
                                      bool eom) {
  if (writeBuf.empty() && !eom) {
    return true;
  }
  deliverLifecycleEvent(
      &LifecycleObserver::onWrite, *this, writeBuf.chainLength());
  XLOG(DBG4) << "Writing id=" << id << " len=" << writeBuf.chainLength()
             << " eom=" << (eom ? "true" : "false") << " sess=" << *this;
  auto res = quicSocket_->writeChain(
      id, writeBuf.move(), eom, eom ? &deliveryCallback_ : nullptr);
  if (res.hasError()) {
    XLOG(DBG3) << "Error writing to stream, err=" << quic::toString(res.error())
               << " id=" << id << " sess=" << *this;
    connectionError(HTTPErrorCode::INTERNAL_ERROR, "Write failed");
    return false;
  }
  return true;
}

folly::coro::Task<void> HTTPCoroSession::transferBody(
    StreamState& stream, std::function<void()> guard) {
  XLOG(DBG6) << __func__;
  auto errorCode = HTTPErrorCode::NO_ERROR;
  SCOPE_EXIT {
    stream.stopReading(errorCode);
    guard();
  };
  bool eom = false;
  do {
    XLOG(DBG4) << "Waiting for a body event id=" << stream.getID()
               << " sess=" << *this;
    auto eomEvent = co_await co_awaitTry(stream.nextBodyEvent());
    if (eomEvent.hasException()) {
      auto err = getHTTPError(eomEvent);
      XLOG(DBG4) << "Error getting body sess=" << *this
                 << " id=" << stream.getID() << " err=" << err.msg;
      if (!stream.isEgressComplete()) {
        egressResetStream(stream.getID(), &stream, err.code, true);
      }
      errorCode = err.code;
      co_return;
    }
    if (!stream.isEgressComplete()) {
      if (eomEvent->resume.has_value()) {
        co_await std::move(eomEvent->resume.value());
      } else {
        notifyBodyWrite(stream);
      }
      eom = eomEvent->eom;
    } else {
      eom = true;
    }
  } while (!eom);
}

void HTTPUniplexTransportSession::registerByteEvents(
    HTTPCodec::StreamID id,
    folly::Optional<uint64_t> streamByteOffset, // stream codec bytes
    folly::Optional<HTTPByteEvent::FieldSectionInfo> fsInfo,
    uint64_t bodyOffset,
    std::vector<HTTPByteEventRegistration>&& regs,
    bool eom) {
  XCHECK(streamByteOffset) << "streamByteOffset required for uniplex";
  auto sessionByteOffset = sessionBytesScheduled_ + writeBuf_.chainLength();
  byteEventObserver_.registerByteEvents(id,
                                        sessionByteOffset,
                                        *streamByteOffset,
                                        fsInfo,
                                        bodyOffset,
                                        std::move(regs),
                                        eom);
}

void HTTPQuicCoroSession::registerByteEvents(
    HTTPCodec::StreamID id,
    folly::Optional<uint64_t> streamByteOffset,
    folly::Optional<HTTPByteEvent::FieldSectionInfo> fsInfo,
    uint64_t bodyOffset,
    std::vector<HTTPByteEventRegistration>&& registrations,
    bool eom) {
  auto localRegistrations = std::move(registrations);
  if (!streamByteOffset) {
    auto maybeStreamByteOffset = quicSocket_->getStreamWriteOffset(id);
    if (maybeStreamByteOffset.has_value()) {
      // TODO: could save this lookup by passing stream*
      auto stream = findStream(id);
      XCHECK(stream);
      streamByteOffset =
          *maybeStreamByteOffset + stream->getWriteBuf().chainLength();
    } else {
      HTTPError err(HTTPErrorCode::INTERNAL_ERROR, "Stream offset unavailable");
      for (auto& reg : localRegistrations) {
        reg.cancel(err);
      }
      return;
    }
  }
  for (auto& reg : localRegistrations) {
    if (reg.events == 0 || !reg.callback) {
      continue;
    }
    HTTPByteEvent httpByteEvent;
    httpByteEvent.streamID = reg.streamID.value_or(id);
    httpByteEvent.fieldSectionInfo = fsInfo;
    httpByteEvent.bodyOffset = bodyOffset;
    httpByteEvent.transportOffset = *streamByteOffset;
    httpByteEvent.streamOffset = *streamByteOffset; // same as transportOffset
    httpByteEvent.eom = eom;
    for (auto eventType : HTTPByteEvent::kByteEventTypes) {
      httpByteEvent.type = eventType;
      if (reg.events & uint8_t(eventType)) {
        if (eventType == HTTPByteEvent::Type::TRANSPORT_WRITE) {
          reg.callback->onByteEvent(httpByteEvent);
        } else {
          auto cb = new QuicByteEventCallback(
              byteEventRefcount_, httpByteEvent, reg.callback);
          auto res = quicSocket_->registerByteEventCallback(
              cb->quicByteEventType(), id, *streamByteOffset, cb);
          if (!res) {
            cb->cancel();
          }
        }
      }
    }
    // clear the callback now so the dtor doesn't cancel
    reg.callback.reset();
  }
}

void HTTPCoroSession::notifyBodyWrite(StreamState& stream) {
  insertWithPriority(stream);
  writeEvent_.signal();
}

void HTTPQuicCoroSession::onDatagramsAvailable() noexcept {
  auto result = quicSocket_->readDatagrams();
  if (result.hasError()) {
    XLOG(ERR) << "Got error while reading datagrams: error="
              << toString(result.error());
    connectionError(HTTPErrorCode::INTERNAL_ERROR,
                    "H3_DATAGRAM: internal error");
    return;
  }
  XLOG(DBG4) << "Received " << result.value().size()
             << " datagrams. sess=" << *this;
  for (auto& datagram : result.value()) {
    folly::io::Cursor cursor(datagram.bufQueue().front());
    auto quarterStreamId = quic::follyutils::decodeQuicInteger(cursor);
    if (!quarterStreamId || quarterStreamId->first > kMaxQuarterStreamId) {
      connectionError(HTTPErrorCode::GENERAL_PROTOCOL_ERROR,
                      "H3_DATAGRAM: error decoding stream-id");
      break;
    }

    quic::BufQueue datagramQ;
    datagramQ.append(datagram.bufQueue().move());
    datagramQ.trimStart(quarterStreamId->second);

    auto streamId = quarterStreamId->first * 4;
    auto stream = findStream(streamId);

    if (!stream || stream->streamSource.isUnprocessed()) {
      XLOG(DBG5) << "Stream cannot receive datagrams. streamId=" << streamId
                 << " len=" << datagramQ.chainLength() << " sess=" << *this;
      // TODO: a possible optimization would be to discard datagrams destined
      // to streams that were already closed
      auto itr = datagramsBuffer_.find(streamId);
      if (itr == datagramsBuffer_.end()) {
        itr = datagramsBuffer_.insert(streamId, {}).first;
      }
      auto& vec = itr->second;
      if (vec.size() < vec.max_size()) {
        vec.emplace_back(datagramQ.move());
      } else {
        // buffer is full: discard the datagram
        datagramQ.move();
      }
      continue;
    }

    XLOG(DBG5) << "Received datagram for streamId=" << streamId
               << " len=" << datagramQ.chainLength() << " sess=" << *this;
    stream->streamSource.datagram(datagramQ.move());
  }
}

uint16_t HTTPQuicCoroSession::getDatagramSizeLimit() const {
  if (!isDatagramEnabled() || !quicSocket_->good()) {
    return 0;
  }
  auto transportMaxDatagramSize = quicSocket_->getDatagramSizeLimit();
  if (transportMaxDatagramSize < kMaxDatagramHeaderSize) {
    return 0;
  }
  return quicSocket_->getDatagramSizeLimit() - kMaxDatagramHeaderSize;
}

bool HTTPQuicCoroSession::sendDatagram(HTTPCodec::StreamID id,
                                       std::unique_ptr<folly::IOBuf> datagram) {
  if (!isDatagramEnabled() || !quic::isClientBidirectionalStream(id) ||
      !quicSocket_->good()) {
    return false;
  }
  // Prepend the H3 Datagram header to the datagram payload
  // HTTP/3 Datagram {
  //   Quarter Stream ID (i),
  //   HTTP/3 Datagram Payload (..),
  // }
  quic::BufPtr headerBuf =
      quic::BufPtr(folly::IOBuf::create(quicSocket_->getDatagramSizeLimit()));
  quic::BufAppender appender(headerBuf.get(), kMaxDatagramHeaderSize);
  auto streamIdRes =
      quic::encodeQuicInteger(id / 4, [&](auto val) { appender.writeBE(val); });
  if (streamIdRes.hasError()) {
    return false;
  }
  XLOG(DBG5) << "Sending datagram for streamId=" << id
             << " len=" << datagram->computeChainDataLength()
             << " sess=" << *this;
  quic::BufQueue queue(std::move(headerBuf));
  queue.append(std::move(datagram));
  auto writeRes = quicSocket_->writeDatagram(queue.move());
  if (writeRes.hasError()) {
    XLOG(DBG5) << "Failed to send datagram for streamId=" << id;
    return false;
  }
  return true;
}

void HTTPCoroSession::transactionAttached() noexcept {
  if (numStreams() == 1) {
    deliverLifecycleEvent(&LifecycleObserver::onActivateConnection, *this);
    if (auto* connManager = getConnectionManager()) {
      connManager->onActivated(*this);
    }
  }
  deliverLifecycleEvent(&LifecycleObserver::onTransactionAttached, *this);
}

void HTTPCoroSession::transactionDetached() noexcept {
  deliverLifecycleEvent(&LifecycleObserver::onTransactionDetached, *this);
  if (numStreams() == 0) {
    deliverLifecycleEvent(&LifecycleObserver::onDeactivateConnection, *this);
    if (auto* connManager = getConnectionManager()) {
      connManager->onDeactivated(*this);
    }
  }
}

folly::coro::Task<HTTPSourceHolder> HTTPCoroSession::sendRequest(
    HTTPSourceHolder requestSource) {
  auto reservation = reserveRequest();
  if (reservation.hasException()) {
    return folly::coro::makeErrorTask<HTTPSourceHolder>(
        std::move(reservation.exception()));
  }
  return sendRequest(std::move(requestSource), std::move(*reservation));
}

folly::Try<HTTPCoroSession::RequestReservation>
HTTPCoroSession::reserveRequest() {
  XLOG(DBG6) << "reserveRequest sess=" << *this;
  if (!supportsMoreTransactions()) {
    XLOG(ERR) << "Refusing to send request, streams=" << numOutgoingStreams()
              << " sess=" << *this;
    return folly::Try<RequestReservation>(
        HTTPError(HTTPErrorCode::REFUSED_STREAM, "Exceeded stream limit"));
  }

  RequestReservation reservation(this);
  transactionAttached();
  return folly::Try<RequestReservation>(std::move(reservation));
}

folly::coro::Task<HTTPSourceHolder> HTTPCoroSession::sendRequest(
    HTTPSourceHolder requestSource, RequestReservation reservation) {
  if (!reservation.fromSession(this)) {
    XLOG(DFATAL) << "Invalid reservation sess=" << *this;
    co_yield co_error(
        HTTPError(HTTPErrorCode::INTERNAL_ERROR, "Invalid reservation"));
  }
  // TODO: do we want to throttle reading request headers on buffer space
  auto headerEvent = co_await co_awaitTry(requestSource.readHeaderEvent());
  const auto& cancelToken = co_await folly::coro::co_current_cancellation_token;
  if (cancelToken.isCancellationRequested()) {
    XLOG(DBG4) << "Egress coro cancelled for new stream, sess=" << *this;
    co_yield co_error(HTTPError(HTTPErrorCode::CORO_CANCELLED, "Cancelled"));
  }
  if (headerEvent.hasException()) {
    XLOG(DBG4) << "Error getting headers for sess=" << *this;
    co_yield co_error(getHTTPError(headerEvent));
  }
  if (isDraining()) {
    XLOG(DBG3) << "Refusing to send new stream on draining sess=" << *this;
    co_yield co_error(
        HTTPError(HTTPErrorCode::REFUSED_STREAM, "Session draining"));
  }
  reservation.consume();
  auto res =
      sendRequestImpl(*headerEvent->headers,
                      std::move(headerEvent->egressHeadersFn),
                      std::move(headerEvent->byteEventRegistrations),
                      headerEvent->eom ? nullptr : std::move(requestSource));
  if (res.hasError()) {
    co_yield co_error(std::move(res.error()));
  }
  co_return std::move(res.value());
}

folly::Expected<HTTPSourceHolder, HTTPError> HTTPCoroSession::sendRequest(
    RequestReservation reservation,
    const HTTPMessage& headers,
    HTTPSourceHolder bodySource) noexcept {
  if (!reservation.fromSession(this)) {
    XLOG(DFATAL) << "Invalid reservation sess=" << *this;
    return folly::makeUnexpected(
        HTTPError{HTTPErrorCode::INTERNAL_ERROR, "Invalid reservation"});
  }
  if (isDraining()) {
    XLOG(DBG3) << "Refusing to send new stream on draining sess=" << *this;
    return folly::makeUnexpected(
        HTTPError(HTTPErrorCode::REFUSED_STREAM, "Session draining"));
  }
  reservation.consume();
  return sendRequestImpl(/*headers=*/headers,
                         /*egressHeadersFn=*/nullptr,
                         /*byteEventRegistrations=*/{},
                         /*bodySource=*/std::move(bodySource));
}

folly::Expected<HTTPSourceHolder, HTTPError> HTTPCoroSession::sendRequestImpl(
    const HTTPMessage& headers,
    folly::Function<void(HTTPHeaderSize) noexcept>&& egressHeadersFn,
    std::vector<HTTPByteEventRegistration>&& byteEventRegistrations,
    HTTPSourceHolder bodySource) noexcept {
  auto* stream = createReqStream();
  if (!stream) {
    XLOG(ERR) << "Failed to create new stream" << *this;
    return folly::makeUnexpected(
        HTTPError{HTTPErrorCode::REFUSED_STREAM, "Create stream failed"});
  }
  if (headers.getMethod() == HTTPMethod::HEAD) {
    stream->markIngressAsHeadResponse();
  }
  auto streamID = stream->getID();
  XLOG(DBG4) << "Got request headers sess=" << *this << " id=" << streamID;
  headers.dumpMessage(4);
  stream->checkForUpgrade(headers, /*isIngress=*/false);
  // TODO: do we want to throttle reading request headers on buffer space
  bool eom = !bodySource.readable();
  HTTPHeaderSize size;
  codec_->generateHeader(stream->getWriteBuf(), streamID, headers, eom, &size);
  stream->addToStreamOffset(size.compressed);
  XLOG(DBG6) << "Done generating headers sess=" << *this << " id=" << streamID;
  HTTPByteEvent::FieldSectionInfo fsInfo = {
      HTTPByteEvent::FieldSectionInfo::Type::HEADERS, true, size};
  registerByteEvents(stream->getID(),
                     stream->getStreamOffset(),
                     fsInfo,
                     /*bodyOffset=*/0,
                     std::move(byteEventRegistrations),
                     eom);
  if (egressHeadersFn) {
    egressHeadersFn(size);
  }
  // TODO: HTTPSession can defer sending the initial goaway until after
  // egressing pending headers.  Is this necessary?
  stream->streamSourceActive();
  stream->setExpectedEgressContentLength(
      headers.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH), eom);
  notifyHeaderWrite(*stream, eom);
  if (eom) {
    deliverLifecycleEvent(&LifecycleObserver::onRequestEnd, *this, 0);
    stream->setReadTimeout(streamReadTimeout_);
  } else {
    stream->startEgressCoro();
    // inline for better perf; if bodyEvent is available, this optimization
    // results in combined headers+body ::writeChain
    co_withExecutor(eventBase_,
                    co_withCancellation(
                        stream->cs.egress.getToken(),
                        transferRequestBody(*stream, std::move(bodySource))))
        .startInlineUnsafe();
  }
  XLOG(DBG6) << "terminating sendRequest sess=" << *this << " id=" << streamID;

  return &stream->streamSource;
}

HTTPCoroSession::StreamState* HTTPUniplexTransportSession::createReqStream() {
  // create request stream with no timeout until EOM is sent
  return &createNewStream(codec_->createStream(), /*fromSendRequest=*/true);
}

HTTPCoroSession::StreamState* HTTPQuicCoroSession::createReqStream() {
  auto quicStreamId = quicSocket_->createBidirectionalStream();
  if (!quicStreamId.has_value()) {
    return nullptr;
  }
  multiCodec_->addCodec(*quicStreamId);
  auto& stream = createNewStream(*quicStreamId, /*fromSendRequest=*/true);
  backgroundScope_.add(
      co_withExecutor(&readExec_,
                      co_withCancellation(stream.cs.ingress.getToken(),
                                          readLoop(*quicStreamId))));
  return &stream;
}

folly::coro::Task<void> HTTPCoroSession::transferRequestBody(
    StreamState& stream, HTTPSourceHolder requestSource) {
  stream.setEgressSource(requestSource.release());
  co_await transferBody(stream, [] {});
  stream.setReadTimeout(streamReadTimeout_);
  stream.markEgressCoroComplete();
  checkForDetach(stream);
}

void HTTPCoroSession::egressResetStream(HTTPCodec::StreamID id,
                                        StreamState* stream,
                                        HTTPErrorCode error,
                                        bool fromSource,
                                        bool bidirectionalReset) {
  generateResetStream(id, error, fromSource, bidirectionalReset);
  writeEvent_.signal();
  if (!stream) {
    stream = findStream(id);
  }
  if (stream) {
    decrementPushStreamCount(*stream);
    std::string_view details("Sent reset");
    if (bidirectionalReset) {
      if (fromSource) {
        error = HTTPErrorCode::CORO_CANCELLED;
      } else if (isNoError(error)) {
        error = HTTPErrorCode::CANCEL;
      }
      stream->abortIngress(error, details);
    }
    // TODO: message will be suboptimal when receiving a STOP_SENDING
    resetStreamState(*stream, HTTPError(error, std::string(details)));
  }
}

void HTTPUniplexTransportSession::generateResetStream(
    HTTPCodec::StreamID id,
    HTTPErrorCode error,
    bool fromSource,
    bool /*bidirectionalReset*/) {
  // h1 & h2 resets are bidirectional (terminates ingress & egress)
  if (!codec_->generateRstStream(
          writeBuf_, id, HTTPErrorCode2ErrorCode(error, fromSource))) {
    XLOG(DBG4) << "resetAfterDrainingWrites sess=" << *this;
    resetAfterDrainingWrites_ = true;
    drainStarted();
  }
}

void HTTPQuicCoroSession::generateResetStream(HTTPCodec::StreamID id,
                                              HTTPErrorCode error,
                                              bool fromSource,
                                              bool bidirectionalReset) {
  HTTP3::ErrorCode h3Error = HTTPErrorCode2HTTP3ErrorCode(error, fromSource);
  // TODO: don't egress RS/SS if that direction is already closed
  // not valid for uni-ingress
  if (quicSocket_->isBidirectionalStream(id) ||
      (quicSocket_->isServerStream(id) == isDownstream())) {
    XLOG(DBG4) << "resetStream err=" << uint32_t(error) << " id=" << id
               << " sess=" << *this;
    quicSocket_->resetStream(id, quic::ApplicationErrorCode(h3Error));
  }

  if (bidirectionalReset) {
    generateStopSending(id, error, fromSource);
  }
  // egressResetStream calls writeEvent_.signal()
}

void HTTPQuicCoroSession::generateStopSending(HTTPCodec::StreamID id,
                                              HTTPErrorCode error,
                                              bool fromSource) {
  // TODO: don't egress RS/SS if that direction is already closed
  // not valid for uni-egress
  if (quicSocket_->isBidirectionalStream(id) ||
      quicSocket_->isServerStream(id) == isUpstream()) {

    HTTP3::ErrorCode h3Error = HTTPErrorCode2HTTP3ErrorCode(error, fromSource);
    XLOG(DBG4) << "stopSending err=" << uint32_t(error) << " id=" << id
               << " sess=" << *this;

    multiCodec_->encodeCancelStream(id);
    quicSocket_->stopSending(id, quic::ApplicationErrorCode(h3Error));
  }
}

void HTTPCoroSession::resetStreamState(StreamState& stream,
                                       const HTTPError& err) {
  // streamSourceComplete_ == true iff consumer has read an error or EOM.
  stream.resetStream(err);
  // unconditional erase could be gratuitous lookup
  removeWritableStream(stream.getID());
  bool signaled = checkForDetach(stream);
  if (!signaled) {
    writeEvent_.signal();
  }
}

bool HTTPCoroSession::checkForDetach(StreamState& stream) {
  if (stream.isDetachable()) {
    XLOG(DBG4) << "detaching stream=" << stream.getID() << " sess=" << *this;
    eraseStream(stream.getID());
    transactionDetached();
    if (!codec_->supportsParallelRequests() && streams_.size() <= 1) {
      handlePipeliningOnDetach();
    }
    writeEvent_.signal();
    return true;
  }
  return false;
}

void HTTPCoroSession::eraseStream(HTTPCodec::StreamID id) {
  streams_.erase(id);
  SESS_STATS(recordTransactionClosed);
}

void HTTPQuicCoroSession::eraseStream(HTTPCodec::StreamID id) {
  HTTPCoroSession::eraseStream(id);
  if (multiCodec_->setCurrentStream(id) && multiCodec_->isParserPaused()) {
    multiCodec_->encodeCancelStream(id);
  }
  multiCodec_->removeCodec(id);
}

void HTTPUniplexTransportSession::handlePipeliningOnDetach() {
  if (streams_.size() == 1 && resetAfterDrainingWrites_) {
    // Super special case: a pipelined request created a second stream,
    // but the active stream triggered a reset.  Just reset the pipelined
    // stream, leave the parser paused, and signal the baton.
    resetStreamState(*streams_.begin()->second,
                     HTTPError(HTTPErrorCode::CANCEL, "Pipeline cancel"));
  } else {
    codec_->setParserPaused(false);
  }
  antiPipelineBaton_.signal();
}

void HTTPCoroSession::setSetting(SettingsId id, uint32_t value) {
  auto settings = codec_->getEgressSettings();
  if (settings) {
    settings->setSetting(id, value);
  }
}

void HTTPUniplexTransportSession::sendPing() {
  codec_->generatePingRequest(writeBuf_, folly::none);
  writeEvent_.signal();
}

void HTTPQuicCoroSession::sendPing() {
  quicSocket_->sendPing(std::chrono::milliseconds(0));
}

void HTTPUniplexTransportSession::setConnectionFlowControl(
    uint32_t connFlowControl) {
  if (codec_->supportsSessionFlowControl()) {
    auto delta = recvWindow_.setCapacity(connFlowControl);
    if (delta) {
      codec_->generateWindowUpdate(writeBuf_, 0, delta);
      writeEvent_.signal();
    }
  }
}

void HTTPQuicCoroSession::setConnectionFlowControl(uint32_t connFlowControl) {
  quicSocket_->setConnectionFlowControlWindow(connFlowControl);
}

bool HTTPUniplexTransportSession::shouldContinueReadLooping() const {
  // Continue reading while there are open streams or the codec is reusable,
  // unless writes have finished or there is a pending reset
  bool continueLoop = !writesFinished_.ready() && !resetAfterDrainingWrites_ &&
                      (codec_->isReusable() || !streams_.empty());
  // clang-format off
  XLOG(DBG4)
    << __func__
    << " continue=" << uint32_t(continueLoop)
    << " writesFinished_=" << uint32_t(writesFinished_.ready())
    << " resetAfterDrainingWrites_=" << uint32_t(resetAfterDrainingWrites_)
    << " isReusable=" << uint32_t(codec_->isReusable())
    << " nStreams=" << streams_.size()
    << " sess=" << *this;
  // clang-format on
  return continueLoop;
}

folly::coro::Task<void> HTTPUniplexTransportSession::readLoop() noexcept {
  XLOG(DBG6) << "starting readLoop sess=" << *this;
  folly::IOBufQueue readBuf(folly::IOBufQueue::cacheChainLength());
  const auto& cancelToken = co_await folly::coro::co_current_cancellation_token;
  folly::CancellationCallback cancellationCallback{
      cancelToken, [this] { dropConnection("Connection dropped (cancel)"); }};
  while (shouldContinueReadLooping()) {
    XLOG(DBG6) << "before read sess=" << *this;
    auto cancellationToken = readCancellationSource_.getToken();
    if (!flowControlBaton_.ready()) {
      flowControlBaton_.reset();
      auto status = co_await flowControlBaton_.timedWait(eventBase_.get(),
                                                         connReadTimeout_);
      if (status == TimedBaton::Status::timedout) {
        XLOG(DBG4) << "Timed out waiting for flow control sess=" << *this;
        connectionError(HTTPErrorCode::READ_TIMEOUT,
                        "ingress backpressure timeout");
        break;
      }
    }
    folly::Try<size_t> rc;
    {
      auto guard = readExec_.acquireGuard();
      rc = co_await co_awaitTry(co_withCancellation(
          cancellationToken,
          coroTransport_->read(
              readBuf, kMinReadSize, kReadBufNewAllocSize, connReadTimeout_)));
    }
    if (cancellationToken.isCancellationRequested()) {
      XLOG(DBG4) << "Read cancelled sess=" << *this;
      XCHECK(!shouldContinueReadLooping());
      if (rc.hasException()) {
        continue;
      }
    }
    if (rc.hasException()) {
      auto ex = rc.exception().get_exception<folly::AsyncSocketException>();
      XCHECK(ex) << "Unexpected exception type "
                 << folly::exceptionStr(rc.exception());
      if (ex->getType() == folly::coro::TransportIf::ErrorCode::TIMED_OUT) {
        XLOG(DBG4) << "Initiating drain due to read timeout sess=" << *this;
        initiateDrain();
        continue;
      }
      deliverLifecycleEvent(
          &LifecycleObserver::onIngressError, *this, kErrorConnectionReset);

      XLOG(DBG4) << "Read Error ex=" << ex->what() << " sess=" << *this;
      connectionError(HTTPErrorCode::TRANSPORT_READ_ERROR, ex->what());
      break;
    }
    if (*rc == 0) { // peer closed the connection
      XLOG(DBG4) << "Read EOF sess=" << *this;
      deliverLifecycleEvent(&LifecycleObserver::onIngressEOF, *this);
      codec_->onIngressEOF();
      for (auto& [_, stream] : streams_) {
        stream->abortIngress(HTTPErrorCode::TRANSPORT_EOF);
      }
      break;
    }
    XLOG(DBG6) << "Read " << *rc << " bytes sess=" << *this;
    deliverLifecycleEvent(
        &LifecycleObserver::onRead, *this, *rc, HTTPCodec::NoStream);
    // We basically have two timeouts here
    resetTimeout();
    size_t bytesParsed = 0;
    do {
      bytesParsed = codec_->onIngress(*readBuf.front());
      readBuf.trimStart(bytesParsed);
      if (bytesParsed == 0 && !readBuf.empty() &&
          !codec_->supportsParallelRequests() && streams_.size() > 1) {
        XLOG(DBG4) << "Waiting for previous transaction(s) to finish before "
                   << "parsing more sess=" << *this;
        antiPipelineBaton_.reset();
        auto status = co_await antiPipelineBaton_.wait();
        XCHECK(status != TimedBaton::Status::timedout);
        if (status != TimedBaton::Status::cancelled) {
          // pretend we parsed something so we go around the loop again
          bytesParsed = 1;
        } // else, bytesParsed == 0, and cancelCallback initiated teardown
      }
    } while (bytesParsed > 0 && !readBuf.empty());
    if (*rc >= kMaxReadDataPerLoop) {
      // Maxed out this loop, give someone else a chance
      co_await folly::coro::co_reschedule_on_current_executor;
    }

    // Never pause reading from H2 streams, memory is bounded by flow control
    // limits, and control messages will be rate limited
    // TODO: pause reading from H1 streams
  }
  readsClosed_ = true;
  writeEvent_.signal();
  XLOG(DBG6) << "readLoop terminating sess=" << *this;
}

void HTTPQuicCoroSession::onNewBidirectionalStream(quic::StreamId id) noexcept {
  XLOG(DBG4) << "New bidi stream=" << id << " sess=" << *this;
  resetIdleTimeout();
  // TODO: downstream only, for now
  if (isUpstream()) {
    XLOG(DBG4) << "Refusing server-init bidi id=" << id << " sess=" << *this;
    egressResetStream(id, nullptr, HTTPErrorCode::STREAM_CREATION_ERROR);
    return;
  }
  if (!multiCodec_->isStreamIngressEgressAllowed(id)) {
    XLOG(DBG4) << "Refusing stream after GOAWAY id=" << id << " sess=" << *this;
    egressResetStream(id, nullptr, HTTPErrorCode::REQUEST_REJECTED);
    return;
  }

  onMessageBegin(id, nullptr);
  auto& stream = *CHECK_NOTNULL(findStream(id));
  multiCodec_->addCodec(id);
  backgroundScope_.add(co_withExecutor(
      &readExec_,
      co_withCancellation(stream.cs.ingress.getToken(), readLoop(id))));
}

void HTTPQuicCoroSession::onNewUnidirectionalStream(
    quic::StreamId id) noexcept {
  XLOG(DBG4) << "New uni stream=" << id << " sess=" << *this;
  resetIdleTimeout();
  uniStreamDispatcher_.takeTemporaryOwnership(id);
  quicSocket_->setPeekCallback(id, &uniStreamDispatcher_);
}

void HTTPQuicCoroSession::onStopSending(
    quic::StreamId id, quic::ApplicationErrorCode /*error*/) noexcept {
  XLOG(DBG4) << "onStopSending stream=" << id << " sess=" << *this;
  // Always reset the stream at the transport.  There are 4 cases:
  // 1. Stream is not yet egress complete from session
  // 2. Stream is egress complete from session, but not transport
  // 3. Stream is egress complete from session and transport - reset is no-op
  // 4. No stream state - Like 2 or 3, but session has removed stream state
  egressResetStream(id,
                    nullptr,
                    HTTPErrorCode::REQUEST_CANCELLED,
                    /*fromSource=*/false,
                    /*bidirectionalReset=*/false);
}

void HTTPQuicCoroSession::onBidirectionalStreamsAvailable(
    uint64_t numStreamsAvailable) noexcept {
  if (isUpstream()) {
    XLOG(DBG4) << "Got new max number of concurrent streams we can initiate: "
               << numStreamsAvailable << " sess=" << *this;
    // Conservatively assume we were at 0
    onSetMaxInitiatedStreams(/*didSupport=*/false);
  }
}

void HTTPQuicCoroSession::onConnectionEnd() noexcept {
  XLOG(DBG4) << "onConnectionEnd sess=" << *this;
  onConnectionError(kHTTPNoError);
}

void HTTPQuicCoroSession::onConnectionError(quic::QuicError error) noexcept {
  bool noError = false;
  HTTPErrorCode httpError = HTTPErrorCode::TRANSPORT_READ_ERROR;
  if (error.code.type() == quic::QuicErrorCode::Type::ApplicationErrorCode) {
    auto code = (HTTP3::ErrorCode)*error.code.asApplicationErrorCode();
    noError = (code == HTTP3::ErrorCode::HTTP_NO_ERROR);
    httpError = noError ? HTTPErrorCode::INTERNAL_ERROR
                        : HTTP3ErrorCode2HTTPErrorCode(code);
  }
  if (noError) {
    // Delivers compression error when the connection ends with queued headers
    multiCodec_->getQPACKCodec().encoderStreamEnd();
    multiCodec_->getQPACKCodec().decoderStreamEnd();
  }
  if (!noError || !streams_.empty()) {
    XLOG(ERR) << "Connection error type=" << int(error.code.type())
              << " err=" << error.message << " httpError=" << (int)httpError
              << " sess=" << *this;
  }
  connectionError(httpError, "QUIC connection error");
  /**
   * Matching both destructors in QuicClientTransport & QuicServerTransport;
   * they invoke the private ::closeImpl(drainConnection = false) member fn,
   * which is most similar to the ::closeNow public api
   */
  quicSocket_->closeNow(
      quic::QuicError{quic::LocalErrorCode::SHUTTING_DOWN, "shutdown"});
  idle_.signal();
}

folly::coro::TaskWithExecutor<void> HTTPQuicCoroSession::run() {
  return co_withExecutor(&readExec_, runImpl());
}

folly::coro::Task<void> HTTPQuicCoroSession::readLoop() noexcept {
  // Idle loop
  while (quicSocket_->good() && !connectionError_) {
    idle_.reset();
    auto guard = readExec_.acquireGuard();
    auto res = co_await idle_.timedWait(eventBase_.get(), connReadTimeout_);
    if (res == TimedBaton::Status::timedout) {
      initiateDrain();
    } else if (res == TimedBaton::Status::cancelled) {
      dropConnection("Connection dropped (cancel)");
    }
  }
  XLOG(DBG6) << "Idle loop complete sess=" << *this;
  multiCodec_->getQPACKCodec().encoderStreamEnd();
  multiCodec_->getQPACKCodec().decoderStreamEnd();
  writeEvent_.signal();
}

folly::coro::Task<void> HTTPQuicCoroSession::runImpl() {
  backgroundScope_.add(co_withExecutor(&writeExec_, writeLoop()));
  co_await readLoop();
  co_await backgroundScope_.joinAsync();
  XLOG(DBG6) << "Background scope joined sess=" << *this;
  co_await co_withCancellation(/*cancelToken=*/{},
                               waitForAllStreams()); // uncancellable
  XLOG(DBG6) << "All streams done sess=" << *this;
  // already uncancellable
  // expects a post outside of evb (i.e. our readExec_ will trip a check here)
  co_await co_withExecutor(eventBase_, zeroRefs()).startInlineUnsafe();
  XLOG(DBG6) << "terminating run sess=" << *this;
  destroy();
}

void HTTPQuicCoroSession::resetIdleTimeout() {
  idle_.signal();
}

void HTTPQuicCoroSession::rejectStream(quic::StreamId id) {
  quicSocket_->stopSending(
      id,
      (quic::ApplicationErrorCode)HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
  quicSocket_->setPeekCallback(id, nullptr);
  // when adding bidi, invoke resetStream too
}

void HTTPQuicCoroSession::dispatchControlStream(
    quic::StreamId id,
    hq::UnidirectionalStreamType streamType,
    size_t toConsume) {
  hq::HQUnidirectionalCodec* controlCodec{nullptr};
  bool isQPACKEncoder = false;
  switch (streamType) {
    case hq::UnidirectionalStreamType::CONTROL:
      controlCodec = multiCodec_;
      break;
    case hq::UnidirectionalStreamType::QPACK_ENCODER:
      controlCodec = &qpackEncoderCodec_;
      isQPACKEncoder = true;
      break;
    case hq::UnidirectionalStreamType::QPACK_DECODER:
      controlCodec = &qpackDecoderCodec_;
      break;
    default:
      XLOG(FATAL) << "Invalid type=" << streamType;
  }
  XLOG(DBG4) << "Dispatching uni stream=" << id << " sess=" << *this;
  quicSocket_->consume(id, toConsume);
  quicSocket_->setPeekCallback(id, nullptr);
  co_withExecutor(&readExec_,
                  readControlStream(id, *controlCodec, isQPACKEncoder))
      .start();
}

void HTTPQuicCoroSession::dispatchPushStream(quic::StreamId id,
                                             hq::PushId pushId,
                                             size_t toConsume) {
  // ingress push streams are not allowed on the server
  XLOG_IF(WARNING, isDownstream())
      << "PUSH stream received on server, id=" << id << " sess=" << *this;
  quicSocket_->consume(id, toConsume);
  quicSocket_->setPeekCallback(id, nullptr);
  dispatchPushStream(id, pushId);
}

folly::coro::Task<void> HTTPQuicCoroSession::readControlStream(
    quic::StreamId id, hq::HQUnidirectionalCodec& codec, bool isQPACKEncoder) {
  class ControlRCB : public QuicReadCallback {
   public:
    ControlRCB(HTTPQuicCoroSession& session,
               hq::HQUnidirectionalCodec& codec,
               bool isQPACKEncoder)
        : QuicReadCallback(session),
          controlCodec_(codec),
          isQPACKEncoder_(isQPACKEncoder) {
    }
    void readAvailable(quic::StreamId id) noexcept override {
      XLOG(DBG4) << "Read available on control stream id=" << id
                 << " sess=" << session_;
      session_.resetIdleTimeout();
      auto buf = session_.quicSocket_->read(id, 0);
      if (buf.hasError()) {
        XLOG(ERR)
            << "Error reading control stream err=" << uint64_t(buf.error())
            << "  id=" << id << " sess=" << session_;
        // TODO: Is the stream state reset in this case?
        baton_.signal();
        return;
      }

      if (buf->first) {
        session_.deliverLifecycleEvent(&LifecycleObserver::onRead,
                                       session_,
                                       buf->first->computeChainDataLength(),
                                       id);
        input_.append(std::move(buf->first));
        if (!input_.empty()) {
          XLOG(DBG4) << "Parsing len=" << input_.chainLength() << " id=" << id
                     << " sess=" << session_;
          input_.append(controlCodec_.onUnidirectionalIngress(input_.move()));
          if (isQPACKEncoder_) {
            // Trigger an iteration of the controlStreamWriteLoop.  This will
            // check for unacknowledged inserts and emit an InsertCountInc
            session_.writeEvent_.signal();
          }
        }
      }
      if (buf->second) {
        XLOG(DBG4) << "Parsing end of stream id=" << id << " sess=" << session_;
        controlCodec_.onUnidirectionalIngressEOF();
        baton_.signal();
      }
    }
    bool noError(quic::QuicErrorCode error) {
      return (error.type() == quic::QuicErrorCode::Type::LocalErrorCode &&
              (*error.asLocalErrorCode() == quic::LocalErrorCode::NO_ERROR ||
               *error.asLocalErrorCode() ==
                   quic::LocalErrorCode::IDLE_TIMEOUT)) ||
             (error.type() == quic::QuicErrorCode::Type::ApplicationErrorCode &&
              (HTTP3::ErrorCode(*error.asApplicationErrorCode()) ==
                   HTTP3::ErrorCode::HTTP_NO_ERROR ||
               uint16_t(*error.asApplicationErrorCode()) ==
                   uint16_t(quic::GenericApplicationErrorCode::NO_ERROR))) ||
             (error.type() == quic::QuicErrorCode::Type::TransportErrorCode &&
              *error.asTransportErrorCode() ==
                  quic::TransportErrorCode::NO_ERROR);
    }
    void readError(quic::StreamId id, quic::QuicError error) noexcept override {
      if (!noError(error.code)) {
        XLOG(ERR) << "Error reading control stream type="
                  << uint64_t(error.code.type()) << " msg=" << error.message
                  << ", id=" << id << " sess=" << session_;
        session_.connectionError(HTTPErrorCode::CLOSED_CRITICAL_STREAM,
                                 "control stream read error");
      }
      baton_.signal();
    }

    hq::HQUnidirectionalCodec& controlCodec_;
    bool isQPACKEncoder_{false};
  };

  XLOG(DBG4) << __func__ << " started id=" << id << " sess=" << *this;
  if (quicSocket_->good()) {
    quicSocket_->setControlStream(id);
    ControlRCB controlRcb(*this, codec, isQPACKEncoder);
    quicSocket_->setReadCallback(id, &controlRcb, std::nullopt);
    auto res = co_await controlRcb.getBaton().wait();
    XCHECK(res != TimedBaton::Status::timedout);
    // Do I need to clear the read callback for this stream?
  }
  XLOG(DBG4) << __func__ << " complete, id=" << id << " sess=" << *this;
}

void HTTPQuicCoroSession::StreamRCB::readAvailable(quic::StreamId id) noexcept {
  if (inProcessRead_) {
    return;
  }
  XLOG(DBG4) << "Read available id=" << id << " sess=" << session_;
  session_.resetIdleTimeout();
  auto buf = session_.quicSocket_->read(id, 0);
  if (buf.hasError()) {
    XLOG(ERR) << "Error reading stream id=" << id
              << quic::toString(buf.error());
    // TODO: Stream state reset?
    baton_.signal();
    return;
  }
  if (buf->first) {
    auto length = buf->first->computeChainDataLength();
    input_.append(std::move(buf->first));
    if (length > 0) {
      session_.deliverLifecycleEvent(
          &LifecycleObserver::onRead, session_, length, id);
    }
  }
  readEOF_ |= buf->second;
  processRead(id);
}

void HTTPQuicCoroSession::StreamRCB::processRead(quic::StreamId id) {
  if (inProcessRead_) {
    return;
  }
  inProcessRead_ = true;
  auto g = folly::makeGuard([this] { inProcessRead_ = false; });
  if (!input_.empty()) {
    XLOG(DBG4) << "Parsing len=" << input_.chainLength() << " id=" << id
               << " sess=" << session_;
    if (!session_.multiCodec_->setCurrentStream(id)) {
      XLOG(DBG3) << "No codec for stream=" << id << " sess=" << session_;
      // Stream has already detached, so a stop-sending must be in flight?
      baton_.signal();
      return;
    }
    auto parsed = session_.codec_->onIngress(*input_.front());
    input_.trimStart(parsed);
  }
  if (input_.empty() && readEOF_) {
    session_.quicSocket_->setReadCallback(id, nullptr, std::nullopt);
    XLOG(DBG4) << "Parsing end of stream id=" << id << " sess=" << session_;
    readEOF_ = false;
    if (!session_.multiCodec_->setCurrentStream(id)) {
      XLOG(DBG3) << "No codec for stream=" << id << " sess=" << session_;
      // Stream has already detached, so a stop-sending must be in flight?
      baton_.signal();
      return;
    }
    session_.codec_->onIngressEOF();
    baton_.signal();
    return;
  }
  // Check at the end of this function, since the parser can be paused but
  // we've received all the data from the transport for this stream
  if (session_.multiCodec_->setCurrentStream(id) &&
      session_.quicSocket_->good()) {
    if (session_.multiCodec_->isParserPaused()) {
      session_.quicSocket_->pauseRead(id);
      isReadPaused_ = true;
    } else if (isReadPaused_) {
      if (!session_.isStreamIngressLimitExceeded(id)) {
        session_.quicSocket_->resumeRead(id);
      }
      isReadPaused_ = false;
    }
  }
}

void HTTPQuicCoroSession::StreamRCB::resumeRead(quic::StreamId id) {
  XLOG(DBG4) << "Process read from resume id=" << id << " sess=" << session_;
  processRead(id);
}

void HTTPQuicCoroSession::StreamRCB::readError(quic::StreamId id,
                                               quic::QuicError error) noexcept {
  XLOG(DBG3) << "Error reading stream id=" << id << " sess=" << session_
             << " type=" << uint32_t(error.code.type())
             << " err=" << quic::toString(error.code);
  HTTPErrorCode httpError = HTTPErrorCode::TRANSPORT_READ_ERROR;
  if (error.code.type() == quic::QuicErrorCode::Type::ApplicationErrorCode) {
    auto code = (HTTP3::ErrorCode)*error.code.asApplicationErrorCode();
    httpError = (code == HTTP3::ErrorCode::HTTP_NO_ERROR)
                    ? HTTPErrorCode::TRANSPORT_EOF
                    : HTTP3ErrorCode2HTTPErrorCode(code);
  } else if (error.code.type() == quic::QuicErrorCode::Type::LocalErrorCode) {
    auto code = *error.code.asLocalErrorCode();
    if (code == quic::LocalErrorCode::IDLE_TIMEOUT) {
      httpError = HTTPErrorCode::TRANSPORT_EOF;
    }
  }
  // TODO: more specific for Local vs. Transport errors?
  session_.onResetStream(id, httpError);
  session_.multiCodec_->getQPACKDecoderWriteBuf().append(
      session_.multiCodec_->getQPACKCodec().encodeCancelStream(id));
  session_.writeEvent_.signal();
  session_.quicSocket_->setReadCallback(id, nullptr, std::nullopt);
  baton_.signal();
}

folly::coro::Task<void> HTTPQuicCoroSession::readLoop(
    quic::StreamId id) noexcept {
  if (quicSocket_->good()) {
    if (!multiCodec_->setCurrentStream(id)) {
      XLOG(DBG2) << "readLoop no-op, stream already cancelled id=" << id
                 << " sess=" << *this;
      return folly::coro::makeTask(folly::Unit());
    }
    auto streamRCB = std::make_shared<StreamRCB>(*this);
    std::weak_ptr<StreamRCB> weakStreamRCB(streamRCB);
    multiCodec_->setResumeHook(id, [weakStreamRCB, id] {
      if (auto streamRCB = weakStreamRCB.lock()) {
        streamRCB->resumeRead(id);
      }
    });
    auto rc = quicSocket_->setReadCallback(id, streamRCB.get(), std::nullopt);
    if (rc.has_value()) {
      return readLoopImpl(std::move(streamRCB), id);
    } else {
      XLOG(ERR) << rc.error();
    }
  }
  return folly::coro::makeTask(folly::Unit());
}

folly::coro::Task<void> HTTPQuicCoroSession::readLoopImpl(
    std::shared_ptr<StreamRCB> streamRCB, quic::StreamId id) noexcept {
  XLOG(DBG4) << __func__ << " started id=" << id << " sess=" << *this;
  auto res = co_await streamRCB->getBaton().wait();
  XCHECK(res != TimedBaton::Status::timedout);
  quicSocket_->setReadCallback(id, nullptr, std::nullopt);
  XLOG(DBG4) << __func__ << " complete id=" << id << " sess=" << *this;
}

void HTTPCoroSession::handleWriteEventTimeout() {
  // This only requires action if the conn or a stream is out of flow control.
  // Otherwise, it just means that no one had anything to write for a given
  // time period.
  if (isConnectionFlowControlBlocked()) {
    for (auto& [id, stream] : streams_) {
      if (!stream->isBodyQueueEmpty()) {
        XLOG(ERR) << "Timed out waiting for conn flow control, sess=" << *this;
        connectionError(HTTPErrorCode::FLOW_CONTROL_ERROR,
                        "Timed out waiting for flow control");
        break;
      }
    }
    // If we get here, then we did time out for conn fcw but there's nothing
    // that needs it.
  } else {
    // Check for streams that are out of flow control
    std::vector<HTTPCodec::StreamID> ids;
    for (auto& [id, stream] : streams_) {
      if (isStreamFlowControlBlocked(*stream)) {
        ids.push_back(id);
      }
    }
    for (auto id : ids) {
      XLOG(DBG4) << "Stream id=" << id
                 << " flow control timeout, resetting, sess=" << *this;
      egressResetStream(id, nullptr, HTTPErrorCode::FLOW_CONTROL_ERROR);
    }
  }
}

void HTTPCoroSession::resetOpenStreams(HTTPErrorCode error,
                                       std::string_view details) {
  // Reset all open streams
  std::vector<StreamState*> streams;
  for (auto& [_, stream] : streams_) {
    streams.push_back(stream.get());
  }
  for (auto* stream : streams) {
    // Queue an ingress abort for any stream that hasn't seen EOM yet
    stream->abortIngress(error, details);
    // marks egress complete, clears queued egress, checks for detach
    resetStreamState(*stream, HTTPError(error, "Connection error"));
  }
}

void HTTPUniplexTransportSession::cleanupAfterWriteError(
    const std::string& msg) {
  // We terminate the actual writing part of the write loop if the socket
  // gives a write error, but we still need to cleanup and wait for the streams
  connectionError(HTTPErrorCode::TRANSPORT_WRITE_ERROR,
                  folly::to<std::string>("write error: ", msg));
  // Clear any unwritten egress
  writeBuf_.move();
}

folly::coro::Task<void> HTTPCoroSession::waitForAllStreams() {
  // Wait for all the readers to read the errors out and detach
  while (!streams_.empty() || pendingSendStreams_ > 0) {
    writeEvent_.reset();
    auto status =
        co_await writeEvent_.timedWait(eventBase_.get(), writeTimeout_);
    if (status == TimedBaton::Status::timedout) {
      XLOG(DBG4) << "Timeout waiting for stream to drain on error, nStreams="
                 << streams_.size() << " sess=" << *this;
    }
  }
}

bool HTTPUniplexTransportSession::shouldContinueWriteLooping() const {
  // We may need to terminate the write loop with open streams - if we need
  // an EOM to terminate the current message, or a TCP RST
  bool closeWithOpenStreams =
      codec_->closeOnEgressComplete() || resetAfterDrainingWrites_;
  // Continue waiting for write events while the socket is good and:
  //   1) There is some data to write OR
  //   2) There is at least one stream and !closeWithOpenStreams OR
  //   3) New streams can be created (reads are open and codec is reusable) OR
  //   4) There are requests waiting to start
  // clang-format off
  bool continueLoop =
      (!writeBuf_.empty() ||
       (!closeWithOpenStreams && !streams_.empty()) ||
       (!readsClosed_ && codec_->isReusable()) ||
       pendingSendStreams_ > 0);
  // clang-format on
  XLOG(DBG6)
      << __func__ << " continueLoop=" << (continueLoop ? 1 : 0)
      << " pendingSendStreams_=" << pendingSendStreams_
      << " closeOnEgressComplete=" << (codec_->closeOnEgressComplete() ? 1 : 0)
      << " resetAfterDrainingWrites_=" << (resetAfterDrainingWrites_ ? 1 : 0)
      << " nStreams=" << streams_.size()
      << " writeBuf_.chainLength()=" << writeBuf_.chainLength()
      << " readsClosed_=" << (readsClosed_ ? 1 : 0)
      << " codec_->isReusable()=" << (codec_->isReusable() ? 1 : 0)
      << " sess=" << *this;

  return continueLoop;
}

bool HTTPQuicCoroSession::shouldContinueWriteLooping() const {
  auto continueLoop =
      hasControlWrite() || codec_->isReusable() || !streams_.empty();
  XLOG(DBG6) << __func__ << " continueLoop=" << (continueLoop ? 1 : 0)
             << " hasControlWrite=" << hasControlWrite()
             << " codec_->isReusable()=" << (codec_->isReusable() ? 1 : 0)
             << " nStreams=" << streams_.size() << " sess=" << *this;
  return continueLoop;
}

folly::coro::Task<void> HTTPUniplexTransportSession::writeLoop() noexcept {
  XLOG(DBG6) << "starting writeLoop sess=" << *this;
  folly::Optional<std::string> writeError;
  while (!writeError && shouldContinueWriteLooping()) {
    if (writeBuf_.empty() &&
        (writableStreams_.empty() || sendWindow_.getSize() == 0)) {
      writeEvent_.reset();
      XLOG(DBG6) << "Waiting for writeEvent sess=" << *this;
      TimedBaton::Status status;
      {
        auto guard = writeExec_.acquireGuard();
        status =
            co_await writeEvent_.timedWait(eventBase_.get(), writeTimeout_);
      }

      XCHECK(status != TimedBaton::Status::cancelled)
          << "writeLoop not cancellable";
      if (status == TimedBaton::Status::timedout) {
        handleWriteEventTimeout();
        // fall through to check for writes below before terminating loop
      }
      XLOG(DBG4) << "Got writeEvent sess=" << *this;
    }

    // Add stream body data, if there's buffer space and flow control
    if (writeBuf_.chainLength() < kWriteBufLimit && sendWindow_.getSize() > 0 &&
        !writableStreams_.empty()) {
      // HEADERS and other control traffic pre-empt the writing of stream
      // bodies
      int32_t bufSpace = kWriteBufLimit - writeBuf_.chainLength();
      auto maxStreamToWrite = std::min(bufSpace, sendWindow_.getSize());
      XCHECK_GT(maxStreamToWrite, 0);
      XLOG(DBG4) << "Egressing stream bodies up to max=" << maxStreamToWrite
                 << " sess=" << *this;
      auto written = addStreamBodyDataToWriteBuf(maxStreamToWrite);
      XLOG(DBG4) << "Egressed len=" << written << " sess=" << *this;
    }

    if (!writeBuf_.empty()) {
      uint64_t writeLength = writeBuf_.chainLength();
      folly::WriteFlags writeFlags = folly::WriteFlags::NONE;
      auto txAckEvent = byteEventObserver_.nextTxAckEvent();
      if (txAckEvent) {
        // If there's a TX or ACK event, we have to split the write on the
        // event offset, and update the writeFlags.
        XLOG(DBG5) << "Split writeBuf_ at " << txAckEvent->sessionByteOffset;
        XCHECK_GT(txAckEvent->sessionByteOffset, sessionBytesScheduled_);
        writeLength = txAckEvent->sessionByteOffset - sessionBytesScheduled_;
        writeFlags = txAckEvent->writeFlags();
      }
      folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
      writeBuf.append(writeBuf_.split(writeLength));
      sessionBytesScheduled_ += writeLength;
      XLOG(DBG4) << "Writing length=" << writeLength << " sess=" << *this;
      byteEventObserver_.transportWrite(sessionBytesScheduled_);
      auto result = co_await co_awaitTry(
          coroTransport_->write(writeBuf, writeTimeout_, writeFlags));
      if (result.hasException()) {
        XLOG(DBG4) << "Write error, err=" << result.exception()
                   << " sess=" << *this;
        if (txAckEvent) {
          txAckEvent->cancel(HTTPError(HTTPErrorCode::TRANSPORT_WRITE_ERROR,
                                       result.exception().what().c_str()));
        }
        writeError.emplace(result.exception().what());
      } else {
        XLOG(DBG4) << "Wrote length=" << writeLength << " sess=" << *this;
        deliverLifecycleEvent(&LifecycleObserver::onWrite, *this, writeLength);
        byteEventObserver_.transportWriteComplete(sessionBytesScheduled_,
                                                  std::move(txAckEvent));
      }
    }
  }

  if (writeError) {
    cleanupAfterWriteError(*writeError);
  }

  XLOG(DBG6)
      << "writeLoop terminating, closing with "
      << ((resetAfterDrainingWrites_ || writeError) ? "error" : "no error")
      << " sess=" << *this;
  if (resetAfterDrainingWrites_ || writeError) {
    coroTransport_->closeWithReset();
  } else {
    coroTransport_->shutdownWrite();
  }
  drainStarted();
  if (!streams_.empty()) {
    co_await waitForAllStreams();
  }
  co_await byteEventObserver_.zeroRefs();
  writesFinished_.post();
  readCancellationSource_.requestCancellation();
}

void HTTPUniplexTransportSession::maybeEnableByteEvents() {
  if (!byteEventObserver_.isRegistered()) {
    auto transport = coroTransport_->getTransport();
    if (transport) {
      auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
      if (sock) {
        sock->addLifecycleObserver(&byteEventObserver_);
      }
    }
  }
}

bool HTTPQuicCoroSession::hasControlWrite() const {
  return !writeBuf_.empty() ||
         !multiCodec_->getQPACKEncoderWriteBuf().empty() ||
         !multiCodec_->getQPACKDecoderWriteBuf().empty();
}

folly::coro::Task<void> HTTPQuicCoroSession::writeLoop() noexcept {
  while (shouldContinueWriteLooping()) {
    if (!hasControlWrite() && writableStreams_.empty()) {
      writeEvent_.reset();
      XLOG(DBG6) << "Waiting for writeEvent sess=" << *this;
      auto guard = writeExec_.acquireGuard();
      auto res =
          co_await writeEvent_.timedWait(eventBase_.get(), writeTimeout_);
      if (res == TimedBaton::Status::cancelled) {
        break;
      }
      if (res == TimedBaton::Status::timedout) {
        XLOG(DBG4) << "writeEvent timeout sess=" << *this;
        handleWriteEventTimeout();
      }
      XLOG_IF(DBG4, res == TimedBaton::Status::signalled)
          << "Got writeEvent sess=" << *this;
    }
    if (!quicSocket_->good()) {
      // session already ended
      co_return;
    }

    // write control stream data first
    writeControlStream(controlStreamID_, writeBuf_);
    writeControlStream(qpackEncoderStreamID_,
                       multiCodec_->getQPACKEncoderWriteBuf());
    multiCodec_->encodeInsertCountIncrement();
    writeControlStream(qpackDecoderStreamID_,
                       multiCodec_->getQPACKDecoderWriteBuf());

    XLOG(DBG4) << "Egressing stream bodies sess=" << *this;
    while (!writableStreams_.empty()) {
      auto id = writableStreams_.getNextScheduledID(std::nullopt);
      auto streamId = id.asStreamID();
      auto stream = findStream(streamId);
      if (!stream) {
        XLOG(ERR) << "Writable stream missing from streams_ id=" << streamId
                  << " sess=" << *this;
        writableStreams_.erase(id);
        continue;
      }

      auto maybeFC = quicSocket_->getMaxWritableOnStream(streamId);
      if (!maybeFC || *maybeFC == 0) {
        // blocked on flow control, attach cb
        quicSocket_->notifyPendingWriteOnStream(streamId, this);
        writableStreams_.erase(id);
        continue;
      }

      bool eom = stream->pendingEgressEOM();
      bool bodyQueueEmpty = stream->isBodyQueueEmpty();
      uint64_t maxSend = *maybeFC;
      uint64_t bytesWritten = 0;
      std::vector<QuicWriteLoopByteEvent> byteEvents;
      while (!eom && !bodyQueueEmpty) {
        // exit loop if we egress more than maxSend
        if (bytesWritten >= maxSend) {
          break;
        }
        HTTPBodyEvent bodyEvent =
            stream->nextEgressEvent(maxSend - bytesWritten).first;
        folly::Optional<HTTPByteEvent::FieldSectionInfo> fieldSectionInfo;
        bodyQueueEmpty = stream->isBodyQueueEmpty();
        switch (bodyEvent.eventType) {
          case HTTPBodyEvent::BODY: {
            if (!bodyEvent.event.body.empty()) {
              SESS_STATS(
                  recordPendingBufferedWriteBytes,
                  -static_cast<int64_t>(bodyEvent.event.body.chainLength()));

              bytesWritten += codec_->generateBody(stream->getWriteBuf(),
                                                   stream->getID(),
                                                   bodyEvent.event.body.move(),
                                                   HTTPCodec::NoPadding,
                                                   bodyEvent.eom);
            }
            break;
          }
          case HTTPBodyEvent::UPGRADE:
            // TODO
            break;
          case HTTPBodyEvent::SUSPEND:
            XCHECK(false) << "Bad event";
            break;
          case HTTPBodyEvent::DATAGRAM: {
            XCHECK(!bodyEvent.eom) << "DATAGRAM can't be EOM";
            if (bodyEvent.event.datagram) {
              sendDatagram(stream->getID(),
                           std::move(bodyEvent.event.datagram));
            }
            break;
          }
          case HTTPBodyEvent::TRAILERS: {
            XLOG(DBG4) << "Sending trailers sess=" << *this
                       << " id=" << stream->getID();
            auto sz = codec_->generateTrailers(stream->getWriteBuf(),
                                               stream->getID(),
                                               *bodyEvent.event.trailers);
            bytesWritten += sz;
            fieldSectionInfo.emplace<HTTPByteEvent::FieldSectionInfo>(
                {HTTPByteEvent::FieldSectionInfo::Type::TRAILERS,
                 true,
                 {uint32_t(sz), uint32_t(sz), 0}});
            XCHECK(bodyEvent.eom) << "Trailers always EOM";
            break;
          }
          case HTTPBodyEvent::PUSH_PROMISE: {
            auto sz = addPushPromiseToWriteBuf(*stream, bodyEvent);
            bytesWritten += sz.compressed;
            fieldSectionInfo.emplace<HTTPByteEvent::FieldSectionInfo>(
                {HTTPByteEvent::FieldSectionInfo::Type::PUSH_PROMISE,
                 true,
                 sz});
            break;
          }
          case HTTPBodyEvent::PADDING: {
            bytesWritten +=
                codec_->generatePadding(stream->getWriteBuf(),
                                        stream->getID(),
                                        bodyEvent.event.paddingSize);
            break;
          }
        }
        eom = bodyEvent.eom;
        byteEvents.emplace_back(std::move(bodyEvent.byteEventRegistrations),
                                std::move(fieldSectionInfo),
                                stream->observedBodyLength(),
                                stream->getWriteBuf().chainLength());
      }
      XLOG(DBG4) << "Egressed len=" << bytesWritten << " id=" << stream->getID()
                 << " sess=" << *this;

      if (bodyQueueEmpty) {
        writableStreams_.erase(id);
      }

      auto streamByteOffset =
          quicSocket_->getStreamWriteOffset(stream->getID());
      if (streamByteOffset) {
        for (auto& byteEvent : byteEvents) {
          registerByteEvents(stream->getID(),
                             *streamByteOffset + byteEvent.eventOffset,
                             byteEvent.fieldSectionInfo,
                             byteEvent.bodyOffset,
                             std::move(byteEvent.byteEventRegistrations),
                             eom && &byteEvent == &byteEvents.back());
        }
      } // else the registrations will be implicitly canceled with no error
      if (handleWrite(stream->getID(), stream->getWriteBuf(), eom) && eom) {
        stream->markEgressComplete();
        egressFinished(*stream);
      } // handleWrite fails -> connectionError, loop terminates
    }
  }
  if (quicSocket_->good()) {
    XLOG(DBG4) << "Closing QuicSocket from writeLoop with "
               << ((connectionError_) ? "error" : "no error")
               << " sess=" << *this;
    if (connectionError_) {
      quicSocket_->close(connectionError_);
    } else {
      // normal close
      registerControlDeliveryCallback(controlStreamID_);
      registerControlDeliveryCallback(qpackEncoderStreamID_);
      registerControlDeliveryCallback(qpackDecoderStreamID_);
      XLOG(DBG4) << "Waiting for all control data to be delivered count="
                 << DeliveryCallback::kDeliveryCallbackTimeout.count()
                 << " sess=" << *this;
      co_await deliveryCallback_.zeroRefs(eventBase_.get());
      XLOG(DBG4) << "Waiting for outstanding byte events";
      co_await byteEventRefcount_.zeroRefs();
      quicSocket_->close(kHTTPNoError);
    }
    XCHECK_EQ(byteEventRefcount_.count(), 0u); // either we waited or error'd
    idle_.signal();
    XLOG(DBG6) << __func__ << " completed";
  }
}

void HTTPQuicCoroSession::writeControlStream(quic::StreamId id,
                                             folly::IOBufQueue& writeBuf) {
  if (id == quic::kInvalidStreamId) {
    // This stream failed to create, clear the write buffer
    writeBuf_.move();
  } else if (!writeBuf.empty()) {
    XLOG(DBG4) << "Writing len=" << writeBuf.chainLength()
               << " on control stream=" << id << " sess=" << *this;
    if (!handleWrite(id, writeBuf, false)) {
      connectionError(HTTPErrorCode::CLOSED_CRITICAL_STREAM,
                      "Write failed on control stream");
    }
  }
}

void HTTPQuicCoroSession::registerControlDeliveryCallback(quic::StreamId id) {
  if (id == quic::kInvalidStreamId) {
    return;
  }
  auto writeOffset = quicSocket_->getStreamWriteOffset(id);
  auto writeBufferedBytes = quicSocket_->getStreamWriteBufferedBytes(id);
  if (writeOffset.hasError() || writeBufferedBytes.hasError()) {
    return;
  }
  auto totalStreamLength = *writeOffset + *writeBufferedBytes;
  if (totalStreamLength > 0) {
    // calls incRef from onByteEventRegistered, if successful
    quicSocket_->registerByteEventCallback(quic::ByteEvent::Type::ACK,
                                           id,
                                           totalStreamLength - 1,
                                           &deliveryCallback_);
  }
}

size_t HTTPUniplexTransportSession::addStreamBodyDataToWriteBuf(uint32_t max) {
  // Precondition, max is at most connection flow control
  size_t bytesWritten = 0;
  size_t fcBytesWritten = 0;

  while (!writableStreams_.empty() && bytesWritten < max) {
    auto id = writableStreams_.getNextScheduledID(std::nullopt);
    auto streamId = id.asStreamID();
    auto stream = findStream(streamId);
    if (!stream) {
      XLOG(ERR) << "Writable stream missing from streams_ id=" << streamId
                << " sess=" << *this;
      writableStreams_.erase(id);
      continue;
    }
    bool flowControlBlocked = false;
    bool bodyQueueEmpty = false;
    bool eom = false;
    do {
      XCHECK_GT(max, fcBytesWritten);
      HTTPBodyEvent bodyEvent;
      std::tie(bodyEvent, flowControlBlocked) =
          stream->nextEgressEvent(max - fcBytesWritten);
      bodyQueueEmpty = stream->isBodyQueueEmpty();
      eom = bodyEvent.eom;
      folly::Optional<HTTPByteEvent::FieldSectionInfo> fieldSectionInfo;
      switch (bodyEvent.eventType) {
        case HTTPBodyEvent::BODY: {
          if (flowControlBlocked) {
            SESS_STATS(recordTransactionStalled);
            break;
          }
          auto length = bodyEvent.event.body.chainLength();
          if (!sendWindow_.reserve(length)) {
            XLOG(DFATAL) << "Underflow connection send flow control";
            // In opt builds, continue and send anyways, the peer will close
          }
          // Simulate window updates for H1 if needed
          if (!codec_->supportsSessionFlowControl()) {
            onWindowUpdate(0, length);
          }
          if (!codec_->supportsStreamFlowControl()) {
            onWindowUpdate(streamId, length);
          }
          if (sendWindow_.getSize() == 0) {
            SESS_STATS(recordSessionStalled);
            deliverLifecycleEvent(&LifecycleObserver::onFlowControlWindowClosed,
                                  *this);
          }
          if (length == 0) {
            XCHECK(bodyEvent.eom);
            auto eomBytes = codec_->generateEOM(writeBuf_, streamId);
            bytesWritten += eomBytes;
            stream->addToStreamOffset(eomBytes);
          } else {
            auto genBytes = codec_->generateBody(writeBuf_,
                                                 streamId,
                                                 bodyEvent.event.body.move(),
                                                 HTTPCodec::NoPadding,
                                                 bodyEvent.eom);
            XCHECK_GT(genBytes, 0ul);
            fcBytesWritten += length;
            bytesWritten += genBytes;
            stream->addToStreamOffset(genBytes);
          }
          SESS_STATS(recordPendingBufferedWriteBytes,
                     -static_cast<int64_t>(length));
          break;
        }
        case HTTPBodyEvent::DATAGRAM:
          // Drop H1/H2 datagram
          // TODO: serialize it as a capsule in the body
          break;
        case HTTPBodyEvent::UPGRADE:
          // TODO
          break;
        case HTTPBodyEvent::SUSPEND:
          XCHECK(false) << "Bad event";
          break;
        case HTTPBodyEvent::TRAILERS: {
          XLOG(DBG4) << "Sending trailers sess=" << *this << " id=" << streamId;
          auto sz = codec_->generateTrailers(
              writeBuf_, streamId, *bodyEvent.event.trailers);
          bytesWritten += sz;
          stream->addToStreamOffset(sz);
          // Compression info not available for trailers
          fieldSectionInfo.emplace<HTTPByteEvent::FieldSectionInfo>(
              {HTTPByteEvent::FieldSectionInfo::Type::TRAILERS,
               true,
               {uint32_t(sz), uint32_t(sz), 0}});
          XCHECK(bodyEvent.eom) << "Trailers always EOM";
          break;
        }
        case HTTPBodyEvent::PADDING: {
          size_t genBytes = codec_->generatePadding(
              writeBuf_, streamId, bodyEvent.event.paddingSize);
          bytesWritten += genBytes;
          stream->addToStreamOffset(genBytes);
          break;
        }
        case HTTPBodyEvent::PUSH_PROMISE: {
          auto sz = addPushPromiseToWriteBuf(*stream, bodyEvent);
          bytesWritten += sz.compressed;
          fieldSectionInfo.emplace<HTTPByteEvent::FieldSectionInfo>(
              {HTTPByteEvent::FieldSectionInfo::Type::PUSH_PROMISE, true, sz});
          stream->addToStreamOffset(sz.compressed);
          break;
        }
      }
      registerByteEvents(streamId,
                         stream->getStreamOffset(),
                         fieldSectionInfo,
                         stream->observedBodyLength(),
                         std::move(bodyEvent.byteEventRegistrations),
                         eom);
    } while (!bodyQueueEmpty && bytesWritten < max && !flowControlBlocked);
    if (bodyQueueEmpty || flowControlBlocked) {
      // The stream ran out of events/flow control, erase from writableStreams_
      XLOG(DBG4)
          << "Ran out of queued events or flow control for stream: blocked="
          << uint32_t(flowControlBlocked) << " eom=" << uint32_t(eom)
          << " id=" << streamId << " sess=" << *this;
      writableStreams_.erase(id);
      if (eom) {
        egressFinished(*stream);
      }
    }
  }
  return bytesWritten;
}

folly::Expected<std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID>, ErrorCode>
HTTPUniplexTransportSession::createEgressPushStream() {
  auto pushStreamID = codec_->createStream();
  std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID> res{pushStreamID,
                                                          pushStreamID};
  return res;
}

folly::Expected<std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID>, ErrorCode>
HTTPQuicCoroSession::createEgressPushStream() {
  auto pushStreamID = quicSocket_->createUnidirectionalStream();
  if (pushStreamID.hasError()) {
    XLOG(ERR) << "Failed to create a uni stream for push sess=" << *this;
    return folly::makeUnexpected(ErrorCode::PROTOCOL_ERROR);
  }
  auto pushID = multiCodec_->nextPushID();
  multiCodec_->addCodec(*pushStreamID);
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  if (hq::writeStreamPreface(writeBuf,
                             uint64_t(hq::UnidirectionalStreamType::PUSH)) &&
      hq::writeStreamPreface(writeBuf, pushID) &&
      handleWrite(*pushStreamID, writeBuf, false)) {
    std::pair<HTTPCodec::StreamID, HTTPCodec::StreamID> res{pushID,
                                                            *pushStreamID};
    return res;
  }
  return folly::makeUnexpected(ErrorCode::PROTOCOL_ERROR);
}

HTTPHeaderSize HTTPCoroSession::addPushPromiseToWriteBuf(
    StreamState& stream, HTTPBodyEvent& bodyEvent) {
  XCHECK(isDownstream());
  XLOG(DBG4) << "Sending push promise sess=" << *this
             << " id=" << stream.getID();
  if (!codec_->supportsPushTransactions()) {
    XLOG(WARNING) << "Ignoring push because peer does not support push";
    return {0, 0, 0};
  }
  if (!supportsMoreTransactions()) {
    // TODO: technically you can send unlimited PUSH_PROMISES, but
    // you need to throttle the streams themselves, but we just
    // throttle the promises.
    XLOG(ERR) << "Exceeded outgoing stream limit sess=" << *this;
    return {0, 0, 0};
  }
  auto res = createEgressPushStream();
  if (res.hasError()) {
    return {0, 0, 0};
  }
  auto pushID = res->first;
  auto pushStreamID = res->second;

  auto& pushStream = createNewStream(pushStreamID);
  pushStream.parent = stream.getID();
  pushStream.currentPushID = pushID;
  pushStream.streamSource.markEgressOnly();
  pushStream.startEgressCoro();
  numPushStreams_++;

  HTTPHeaderSize size;
  codec_->generatePushPromise(stream.getWriteBuf(),
                              pushID,
                              *bodyEvent.event.push.promise,
                              stream.getID(),
                              bodyEvent.eom,
                              &size);

  // A little strange to start a co-routine from the egress path
  XLOG(DBG4) << "Starting egress push readResponse for id="
             << pushStream.getID();
  co_withExecutor(eventBase_,
                  co_withCancellation(
                      pushStream.cs.egress.getToken(),
                      readResponse(pushStream,
                                   folly::coro::makeTask<HTTPSourceHolder>(
                                       bodyEvent.event.push.movePushSource()))))
      .start();
  return size;
}

void HTTPCoroSession::decrementPushStreamCount(const StreamState& stream,
                                               bool eomMarkedEgressComplete) {
  if (!stream.parent) {
    return;
  }
  // following this call, the state needs to transition
  // upstream needs to go to EOM seen
  // downstream needs to go to egress complete
  if ((isUpstream() && !stream.streamSource.isEOMSeen()) ||
      (isDownstream() &&
       (!stream.isEgressComplete() || eomMarkedEgressComplete))) {
    XCHECK_GT(numPushStreams_, 0UL);
    numPushStreams_--;
  }
}

bool HTTPCoroSession::isStreamFlowControlBlocked(StreamState& stream) {
  return !stream.isEgressComplete() && stream.isFlowControlBlocked();
}

bool HTTPQuicCoroSession::isStreamFlowControlBlocked(StreamState& stream) {
  auto streamFC = quicSocket_->getStreamFlowControl(stream.getID());
  return streamFC && streamFC->sendWindowAvailable == 0;
}

bool HTTPCoroSession::ingressLimitExceeded(const StreamState& stream) const {
  return stream.streamSource.bodyBytesBuffered() >= readBufferLimit_;
}

bool HTTPCoroSession::shouldResumeIngress(const StreamState& stream,
                                          uint64_t delta) const {
  bool wasIngressLimited =
      (stream.streamSource.bodyBytesBuffered() + delta) >= readBufferLimit_;
  return wasIngressLimited && !ingressLimitExceeded(stream);
}

bool HTTPCoroSession::isDetachable() const {
  constexpr auto kDetachable = detail::DetachableExecutor::Detachable;
  return isUpstream() && !isDraining() && numStreams() == 0 &&
         readExec_.getState() == kDetachable &&
         writeExec_.getState() == kDetachable;
}

void HTTPCoroSession::detachEvb() {
  XLOG(DBG4) << __func__;
  XCHECK(isDetachable());
  XCHECK(eventBase_ && eventBase_->isInEventBaseThread());
  eventBase_.reset();
  readExec_.detachEvb();
  writeExec_.detachEvb();
  writeEvent_.detach();
}

void HTTPCoroSession::attachEvb(folly::EventBase* evb) {
  XLOG(DBG4) << __func__;
  XCHECK(evb->inRunningEventBaseThread());
  eventBase_ = evb;
  readExec_.attachEvb(evb);
  writeExec_.attachEvb(evb);
  writeEvent_.signal();
}

bool HTTPUniplexTransportSession::isDetachable() const {
  return HTTPCoroSession::isDetachable() && shouldContinueReadLooping() &&
         shouldContinueWriteLooping();
}

void HTTPUniplexTransportSession::detachEvb() {
  HTTPCoroSession::detachEvb();
  coroTransport_->detachEventBase();
}

void HTTPUniplexTransportSession::attachEvb(folly::EventBase* evb) {
  HTTPCoroSession::attachEvb(evb);
  coroTransport_->attachEventBase(evb);
}

bool HTTPQuicCoroSession::isDetachable() const {
  return HTTPCoroSession::isDetachable() && quicSocket_->good() &&
         uniStreamDispatcher_.numberOfStreams() == 0;
}

void HTTPQuicCoroSession::detachEvb() {
  HTTPCoroSession::detachEvb();
  quicSocket_->detachEventBase();
  idle_.detach();
}

void HTTPQuicCoroSession::attachEvb(folly::EventBase* evb) {
  HTTPCoroSession::attachEvb(evb);
  quicSocket_->attachEventBase(std::make_shared<quic::FollyQuicEventBase>(evb));
  idle_.signal();
}

void HTTPCoroSession::describe(std::ostream& os) const {
  if (isDownstream()) {
    os << "downstream=" << peerAddr_ << ", " << localAddr_ << "=local"
       << ", proto=" << getCodecProtocolString(codec_->getProtocol());
  } else {
    os << ", local=" << localAddr_ << ", " << peerAddr_ << "=upstream"
       << ", proto=" << getCodecProtocolString(codec_->getProtocol());
  }
}

std::ostream& operator<<(std::ostream& os, const HTTPCoroSession& session) {
  session.describe(os);
  return os;
}

// WebTransport related functions below
namespace {

constexpr std::string_view kWtNotSupported = "WebTransport not supported";
constexpr std::string_view kInvalidWtReq = "Invalid WebTransport request";

using WtReqResult = HTTPCoroSession::WtReqResult;

/**
 * http/2 wt draft:
 * > In order to indicate support for WebTransport, both the client and the
 * > server MUST send a SETTINGS_WEBTRANSPORT_MAX_SESSIONS value greater than
 * > "0" in their SETTINGS frame
 *
 * > An endpoint needs to send both SETTINGS_ENABLE_CONNECT_PROTOCOL and
 * > SETTINGS_WEBTRANSPORT_MAX_SESSIONS for WebTransport to be enabled.
 */
bool supportsWt(std::initializer_list<const HTTPSettings*> settings) {
  constexpr auto kEnableConnectProto = SettingsId::ENABLE_CONNECT_PROTOCOL;
  constexpr auto kEnableWtMaxSess = SettingsId::WEBTRANSPORT_MAX_SESSIONS;
  return std::ranges::all_of(settings, [](auto* settings) {
    return settings &&
           settings->getSetting(kEnableConnectProto, /*defaultVal=*/0) &&
           settings->getSetting(kEnableWtMaxSess, /*defaultVal=*/0);
  });
}

folly::coro::Task<WtReqResult> makeInternalEx(std::string_view err) {
  return folly::coro::makeErrorTask<WtReqResult>(
      HTTPError{HTTPErrorCode::INTERNAL_ERROR, std::string(err)});
}

}; // namespace

/**
 * Common logic that can be used by derived classes to validate both that
 * WebTransport is supported and request is valid. Although this function is a
 * Task (for derived classes to override as those will have asynchrony), it is
 * sychronously resolved and should only be checked for errors via co_awaitTry()
 */
folly::coro::Task<WtReqResult> HTTPCoroSession::sendWtReq(
    RequestReservation reservation, const HTTPMessage& msg) noexcept {
  // XLOG_IF(FATAL, !folly::kIsDebug) << "wt wip"; // crash in non-debug modes
  if (!reservation.fromSession(this)) {
    return makeInternalEx("Invalid reservation");
  }

  const bool wtEnabled =
      supportsWt({codec_->getIngressSettings(), codec_->getEgressSettings()});
  const bool validWtReq = HTTPWebTransport::isConnectMessage(msg);
  if (!(wtEnabled && validWtReq)) {
    auto err = !validWtReq ? kInvalidWtReq : kWtNotSupported;
    XLOG(DBG6) << __func__ << " err=" << err << "; sess=" << *this;
    return makeInternalEx(err);
  }

  // valid wt req
  return folly::coro::makeTask<WtReqResult>({});
}

folly::coro::Task<WtReqResult> HTTPUniplexTransportSession::sendWtReq(
    RequestReservation reservation, const HTTPMessage& msg) noexcept {
  auto valid = co_await co_awaitTry(
      HTTPCoroSession::sendWtReq(std::move(reservation), msg));
  if (valid.hasException()) {
    co_return valid;
  }
  // valid wt req
  auto res = sendRequestImpl(/*headers=*/msg,
                             /*egressHeadersFn=*/nullptr,
                             /*byteEventRegistrations=*/{},
                             /*bodySource=*/nullptr);

  XCHECK(res.hasValue()); // http/2 should always succeed here
  while (res->readable()) {
    auto ev = co_await co_nothrow(res->readHeaderEvent());
    if (ev.isFinal()) {
      co_return {std::move(ev.headers), nullptr};
    }
  }
  folly::assume_unreachable();
}

} // namespace proxygen::coro
