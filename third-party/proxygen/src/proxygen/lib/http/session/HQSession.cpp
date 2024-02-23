/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQSession.h>

#include <proxygen/lib/http/HTTPPriorityFunctions.h>
#include <proxygen/lib/http/codec/HQControlCodec.h>
#include <proxygen/lib/http/codec/HQStreamCodec.h>
#include <proxygen/lib/http/codec/HQUtils.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/http/codec/QPACKDecoderCodec.h>
#include <proxygen/lib/http/codec/QPACKEncoderCodec.h>
#include <proxygen/lib/http/session/HTTPSession.h>
#include <proxygen/lib/http/session/HTTPSessionController.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

#include <folly/CppAttributes.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestructionBase.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>
#include <quic/QuicConstants.h>
#include <quic/codec/QuicInteger.h>
#include <quic/common/BufUtil.h>
#include <quic/logging/QLoggerConstants.h>
#include <sstream>
#include <wangle/acceptor/ConnectionManager.h>

namespace {
static const uint16_t kMaxReadsPerLoop = 16;
static const std::string kNoProtocolString("");
static const std::string kQUICProtocolName("QUIC");
constexpr uint64_t kMaxQuarterStreamId = (1ull << 60) - 1;

using namespace proxygen::HTTP3;
bool noError(quic::QuicErrorCode error) {
  return (error.type() == quic::QuicErrorCode::Type::LocalErrorCode &&
          (*error.asLocalErrorCode() == quic::LocalErrorCode::NO_ERROR ||
           *error.asLocalErrorCode() == quic::LocalErrorCode::IDLE_TIMEOUT)) ||
         (error.type() == quic::QuicErrorCode::Type::ApplicationErrorCode &&
          (proxygen::HTTP3::ErrorCode(*error.asApplicationErrorCode()) ==
               proxygen::HTTP3::ErrorCode::HTTP_NO_ERROR ||
           uint16_t(*error.asApplicationErrorCode()) ==
               uint16_t(quic::GenericApplicationErrorCode::NO_ERROR))) ||
         (error.type() == quic::QuicErrorCode::Type::TransportErrorCode &&
          *error.asTransportErrorCode() == quic::TransportErrorCode::NO_ERROR);
}

bool isVlogLevel(quic::TransportErrorCode code) {
  return code == quic::TransportErrorCode::INVALID_MIGRATION;
}

bool isVlogLevel(quic::QuicErrorCode error) {
  return error.type() == quic::QuicErrorCode::Type::TransportErrorCode &&
         isVlogLevel(*error.asTransportErrorCode());
}

// handleSessionError is mostly setup to process application error codes
// that we want to send.  If we receive an application error code, convert to
// HTTP_CLOSED_CRITICAL_STREAM
quic::QuicErrorCode quicControlStreamError(quic::QuicErrorCode error) {
  switch (error.type()) {
    case quic::QuicErrorCode::Type::ApplicationErrorCode:
      return quic::QuicErrorCode(
          proxygen::HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM);
    case quic::QuicErrorCode::Type::LocalErrorCode:
    case quic::QuicErrorCode::Type::TransportErrorCode:
      return error;
  }
  folly::assume_unreachable();
}

quic::Priority toQuicPriority(const proxygen::HTTPPriority& pri) {
  return quic::Priority(pri.urgency, pri.incremental, pri.orderId);
}

bool writeWTStreamPrefaceToSock(
    quic::QuicSocket& sock,
    quic::StreamId wtStreamId,
    quic::StreamId wtSessionId,
    proxygen::hq::WebTransportStreamType streamType) {
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  auto res =
      proxygen::hq::writeWTStreamPreface(writeBuf, streamType, wtSessionId);
  if (!res) {
    LOG(ERROR) << "Failed to write WT stream preface";
    return false;
  }
  auto writeRes = sock.writeChain(wtStreamId, writeBuf.move(), false);
  if (writeRes.hasError()) {
    LOG(ERROR) << "Failed to write stream preface to socket";
    return false;
  }
  return true;
}
} // namespace

using namespace proxygen::hq;

namespace proxygen {

const std::string kH3FBCurrentDraft("h3-fb-05");
const std::string kH3AliasV1("h3-alias-01");
const std::string kH3AliasV2("h3-alias-02");
const std::string kH3("h3");
const std::string kHQ("hq-interop");

// TODO: remove these constants, the library no longer negotiates them
const std::string kH3CurrentDraft("h3-29");
const std::string kHQCurrentDraft("hq-29");

const http2::PriorityUpdate hqDefaultPriority{kSessionStreamId, false, 15};

HQSession::~HQSession() {
  VLOG(3) << *this << " closing";
  runDestroyCallbacks();
}

void HQSession::setSessionStats(HTTPSessionStats* stats) {
  HTTPSessionBase::setSessionStats(stats);
  invokeOnAllStreams([&stats](HQStreamTransportBase* stream) {
    stream->byteEventTracker_.setTTLBAStats(stats);
  });
}

void HQSession::onNewBidirectionalStream(quic::StreamId id) noexcept {
  VLOG(4) << __func__ << " sess=" << *this << ": new streamID=" << id;
  // The transport should never call onNewBidirectionalStream before
  // onTransportReady

  // Reject all bidirectional, server-initiated streams, unless WT is supported
  if (id == kMaxClientBidiStreamId ||
      (direction_ == TransportDirection::UPSTREAM && !supportsWebTransport())) {
    abortStream(HTTPException::Direction::INGRESS_AND_EGRESS,
                id,
                HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
    return;
  }
  auto hqStream = findNonDetachedStream(id);
  DCHECK(!hqStream);
  if (supportsWebTransport()) {
    bidirectionalReadDispatcher_.takeTemporaryOwnership(id);
    sock_->setPeekCallback(id, &bidirectionalReadDispatcher_);
  } else {
    dispatchRequestStreamImpl(id);
  }
}

void HQSession::dispatchRequestStream(quic::StreamId id) {
  if (!sock_->good()) {
    LOG(ERROR) << "Bad socket sess=" << *this;
    return;
  }
  sock_->setPeekCallback(id, nullptr);
  dispatchRequestStreamImpl(id);
}

void HQSession::dispatchRequestStreamImpl(quic::StreamId id) {
  if (maybeRejectRequestAfterGoaway(id)) {
    return;
  }
  // id < kMaxClientBidiStreamId, so id + 4 will not wrap
  minUnseenIncomingStreamId_ = std::max(minUnseenIncomingStreamId_, id + 4);
  auto hqStream = createStreamTransport(id);
  DCHECK(hqStream);
  sock_->setReadCallback(id, this);
  if (ingressLimitExceeded()) {
    sock_->pauseRead(id);
  }
  if (id == 0 && version_ == HQVersion::HQ) {
    // generate grease frame
    auto writeGreaseFrameResult = hq::writeGreaseFrame(hqStream->writeBuf_);
    if (writeGreaseFrameResult.hasError()) {
      VLOG(2) << __func__ << " failed to create grease frame: " << *this
              << ". Error = " << writeGreaseFrameResult.error();
    }
  }
}

void HQSession::onBidirectionalStreamsAvailable(
    uint64_t numStreamsAvailable) noexcept {
  if (direction_ == TransportDirection::UPSTREAM) {
    VLOG(4) << "Got new max number of concurrent streams we can initiate: "
            << numStreamsAvailable << " sess=" << *this;
    if (infoCallback_ && supportsMoreTransactions()) {
      infoCallback_->onSettingsOutgoingStreamsNotFull(*this);
    }
  }
}

void HQSession::onNewUnidirectionalStream(quic::StreamId id) noexcept {
  // This is where a new unidirectional ingress stream is available
  // Try to check whether this is a push
  // if yes, register this as a push
  VLOG(4) << __func__ << " sess=" << *this << ": new streamID=" << id;
  // The transport should never call onNewUnidirectionalStream
  // before onTransportReady
  // The new stream should not exist yet.
  auto existingStream = findStream(id);
  DCHECK(!existingStream) << "duplicate " << __func__ << " for streamID=" << id;
  // This has to be a new control or push stream, but we haven't read the
  // preface yet
  // Assign the stream to the dispatcher
  unidirectionalReadDispatcher_.takeTemporaryOwnership(id);
  sock_->setPeekCallback(id, &unidirectionalReadDispatcher_);
}

void HQSession::onStopSending(quic::StreamId id,
                              quic::ApplicationErrorCode error) noexcept {
  auto errorCode = static_cast<HTTP3::ErrorCode>(error);
  VLOG(3) << __func__ << " sess=" << *this << ": new streamID=" << id
          << " error=" << toString(errorCode);
  auto stream = findStream(id);
  if (stream) {
    handleWriteError(stream, error);
  } else if (supportsWebTransport() &&
             // TODO: is it valid to STOP_SENDING WebTransport streams with
             // error codes outside this range (eg: REJECTED)?
             WebTransport::isEncodedApplicationErrorCode(error)) {
    // might be a WT stream, but there's no mapping HERE to find the session ID
    // just tell all sessions to stop sending
    auto appErrorCode = WebTransport::toApplicationErrorCode(errorCode);
    if (!appErrorCode) {
      return;
    }

    for (auto& streamIt : streams_) {
      if (!streamIt.second.detached_ &&
          streamIt.second.txn_.isWebTransportConnectStream()) {
        if (streamIt.second.txn_.onWebTransportStopSending(id, *appErrorCode)) {
          break;
        }
      }
    }
  }
}

void HQSession::onKnob(uint64_t knobSpace,
                       uint64_t knobId,
                       quic::Buf knobBlob) {
  VLOG(3) << __func__ << " sess=" << *this << " knob frame received: "
          << " KnobSpace: " << std::hex << knobSpace << " KnobId: " << knobId
          << " KnobBlob: "
          << std::string(reinterpret_cast<const char*>(knobBlob->data()),
                         knobBlob->length());
}

bool HQSession::maybeRejectRequestAfterGoaway(quic::StreamId id) {
  // Cancel any stream that is out of the range allowed by GOAWAY
  if (drainState_ != DrainState::NONE) {
    // You can't check upstream here, because upstream GOAWAY sends PUSH IDs.
    // It could be checked in HQUpstreamSesssion::onNewPushStream
    if (direction_ == TransportDirection::DOWNSTREAM &&
        sock_->isBidirectionalStream(id) && id >= getGoawayStreamId()) {
      abortStream(HTTPException::Direction::INGRESS_AND_EGRESS,
                  id,
                  HTTP3::ErrorCode::HTTP_REQUEST_REJECTED);
      return true;
    }
  }

  return false;
}

bool HQSession::onTransportReadyCommon() noexcept {
  localAddr_ = sock_->getLocalAddress();
  peerAddr_ = sock_->getPeerAddress();
  initQuicProtocolInfo(*quicInfo_, *sock_);
  // NOTE: this can drop the connection if the next protocol is not supported
  if (!getAndCheckApplicationProtocol()) {
    return false;
  }
  transportInfo_.acceptTime = getCurrentTime();
  getCurrentTransportInfoWithoutUpdate(&transportInfo_);
  transportInfo_.setupTime = millisecondsSince(transportStart_);
  transportInfo_.connectLatency = millisecondsSince(transportStart_).count();
  transportInfo_.protocolInfo = quicInfo_;
  if (!createEgressControlStreams()) {
    return false;
  }
  // Apply the default settings
  SettingsList defaultSettings = {};
  if (datagramEnabled_) {
    // If local supports Datagram, assume the peer does too, until receiving
    // peer settings
    defaultSettings.push_back({SettingsId::_HQ_DATAGRAM, 1});
    sock_->setDatagramCallback(this);
  }
  sock_->setPingCallback(this);
  // TODO: 0-RTT settings
  applySettings(defaultSettings);
  // notifyPendingShutdown may be invoked before onTransportReady,
  // so we need to address that here by kicking the GOAWAY logic if needed
  if (drainState_ == DrainState::PENDING) {
    sendGoaway();
  }

  informSessionControllerTransportReady();
  return true;
}

bool HQSession::createEgressControlStreams() {
  if (!createEgressControlStream(UnidirectionalStreamType::CONTROL) ||
      !createEgressControlStream(UnidirectionalStreamType::QPACK_ENCODER) ||
      !createEgressControlStream(UnidirectionalStreamType::QPACK_DECODER)) {
    return false;
  }

  sendSettings();
  scheduleWrite();
  return true;
}

bool HQSession::createEgressControlStream(UnidirectionalStreamType streamType) {
  auto id = sock_->createUnidirectionalStream();
  if (id.hasError()) {
    LOG(ERROR) << "Failed to create " << streamType
               << " unidirectional stream. error='" << id.error() << "'";
    onConnectionError(
        quic::QuicError(quic::LocalErrorCode::CONNECT_FAILED,
                        "Failed to create unidirectional stream"));
    return false;
  }

  auto matchPair = controlStreams_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(streamType),
      std::forward_as_tuple(*this, id.value(), streamType));
  CHECK(matchPair.second) << "Emplacement failed";
  sock_->setControlStream(id.value());
  matchPair.first->second.generateStreamPreface();
  return true;
}

HQSession::HQControlStream* FOLLY_NULLABLE
HQSession::createIngressControlStream(quic::StreamId id,
                                      UnidirectionalStreamType streamType) {

  auto ctrlStream = findControlStream(streamType);
  // this is an error in the use of the API, egress control streams must get
  // created at the very beginning
  if (!ctrlStream) {
    LOG(FATAL) << "Cannot create ingress control stream without an egress "
                  "stream streamID="
               << id << " sess=" << *this;
    return nullptr;
  }

  if (ctrlStream->ingressCodec_) {
    LOG(ERROR) << "Too many " << streamType << " streams for sess=" << *this;
    dropConnectionAsync(
        quic::QuicError(HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR,
                        "HTTP wrong stream count"),
        kErrorConnection);
    return nullptr;
  }

  ctrlStream->setIngressStreamId(id);
  ctrlStream->setIngressCodec(createControlCodec(streamType, *ctrlStream));
  return ctrlStream;
}

std::unique_ptr<hq::HQUnidirectionalCodec> HQSession::createControlCodec(
    hq::UnidirectionalStreamType type, HQControlStream& controlStream) {
  switch (type) {
    case hq::UnidirectionalStreamType::CONTROL: {
      auto codec = std::make_unique<hq::HQControlCodec>(
          controlStream.getIngressStreamId(),
          direction_,
          hq::StreamDirection::INGRESS,
          ingressSettings_,
          type);
      codec->setCallback(&controlStream);
      return codec;
    }
    // This is quite weird for now. The stream types are defined  based on
    // the component that initiates them, so the ingress stream from the
    // QPACK Encoder is linked to the local QPACKDecoder, and viceversa
    case hq::UnidirectionalStreamType::QPACK_ENCODER:
      return std::make_unique<hq::QPACKEncoderCodec>(qpackCodec_,
                                                     controlStream);
    case hq::UnidirectionalStreamType::QPACK_DECODER:
      return std::make_unique<hq::QPACKDecoderCodec>(qpackCodec_,
                                                     controlStream);
    default:
      LOG(FATAL) << "Failed to create ingress codec";
      return nullptr;
  }
}

bool HQSession::getAndCheckApplicationProtocol() {
  CHECK(sock_);
  auto alpn = sock_->getAppProtocol();
  if (alpn) {
    if (alpn == kH3FBCurrentDraft || alpn == kH3AliasV1 || alpn == kH3AliasV2 ||
        alpn == kH3) {
      version_ = HQVersion::HQ;
    }
  }
  if (!alpn || !version_) {
    // next protocol not specified or version not supported, close connection
    // with error
    LOG(ERROR) << "next protocol not supported: "
               << (alpn ? *alpn : "no protocol") << " sess=" << *this;

    onConnectionError(quic::QuicError(quic::LocalErrorCode::CONNECT_FAILED,
                                      "ALPN not supported"));
    return false;
  }
  alpn_ = *alpn;
  versionUtilsReady_.set();
  return true;
}

void HQSession::onReplaySafe() noexcept {
  // We might have got onTransportReady with 0-rtt in which case we only get the
  // server connection id after replay safe.
  quicInfo_->serverConnectionId = sock_->getServerConnectionId();
  if (infoCallback_) {
    infoCallback_->onFullHandshakeCompletion(*this);
  }

  for (auto callback : waitingForReplaySafety_) {
    callback->onReplaySafe();
  }
  waitingForReplaySafety_.clear();
}

void HQSession::onConnectionEnd(quic::QuicError error) noexcept {
  if (noError(error.code)) {
    onConnectionEnd();
  } else {
    onConnectionError(std::move(error));
  }
}

void HQSession::onConnectionEnd() noexcept {
  VLOG(4) << __func__ << " sess=" << *this;
  // The transport will not call onConnectionEnd after we call close(),
  // so there is no need for us here to handle re-entrancy
  // checkForShutdown->close->onConnectionEnd.
  drainState_ = DrainState::DONE;
  qpackCodec_.encoderStreamEnd();
  qpackCodec_.decoderStreamEnd();
  closeWhenIdle();
}

void HQSession::onConnectionSetupError(quic::QuicError code) noexcept {
  onConnectionError(std::move(code));
}

void HQSession::onConnectionError(quic::QuicError code) noexcept {
  // the connector will drop the connection in case of connect error
  HQSession::DestructorGuard dg(this);
  VLOG(4) << __func__ << " sess=" << *this
          << ": connection error=" << code.message;

  // Map application errors here to kErrorConnectionReset: eg, the peer tore
  // down the connection
  auto proxygenErr = toProxygenError(code.code, /*fromPeer=*/true);
  if (proxygenErr == kErrorNone && !streams_.empty()) {
    // Peer closed with NO_ERROR but there are open streams
    proxygenErr = kErrorEOF;
  }
  if (infoCallback_) {
    infoCallback_->onIngressError(*this, proxygenErr);
  }

  if (code.code.type() == quic::QuicErrorCode::Type::ApplicationErrorCode &&
      isQPACKError(
          static_cast<HTTP3::ErrorCode>(*code.code.asApplicationErrorCode()))) {
    LOG(ERROR) << "Peer QPACK error err="
               << static_cast<uint32_t>(*code.code.asApplicationErrorCode())
               << " msg=" << code.message << " " << *this;
  } else if (!noError(code.code)) {
    std::stringstream msgStream;
    msgStream << "Peer closed with error err=" << code.code
              << " msg=" << code.message << " " << *this;
    if (isVlogLevel(code.code)) {
      VLOG(3) << msgStream.str();
    } else {
      LOG(ERROR) << msgStream.str();
    }
  }

  // force close all streams.
  // close with error won't invoke any connection callback, reentrancy safe
  dropConnectionSync(std::move(code), proxygenErr);
}

bool HQSession::getCurrentTransportInfo(wangle::TransportInfo* tinfo) {
  getCurrentTransportInfoWithoutUpdate(tinfo);
  tinfo->setupTime = transportInfo_.setupTime;
  tinfo->secure = transportInfo_.secure;
  tinfo->appProtocol = transportInfo_.appProtocol;
  tinfo->connectLatency = transportInfo_.connectLatency;
  // Copy props from the transport info.
  transportInfo_.rtt = tinfo->rtt;
  transportInfo_.rtt_var = tinfo->rtt_var;
  if (sock_) {
    updateQuicProtocolInfo(*quicInfo_, *sock_);
    tinfo->protocolInfo = quicInfo_;
    auto flowControl = sock_->getConnectionFlowControl();
    if (!flowControl.hasError() && flowControl->sendWindowAvailable) {
      tinfo->recvwnd = flowControl->receiveWindowAvailable;
    }
  }
  return true;
}

bool HQSession::getCurrentTransportInfoWithoutUpdate(
    wangle::TransportInfo* tinfo) const {
  tinfo->validTcpinfo = true;
  tinfo->appProtocol = std::make_shared<std::string>(alpn_);
  tinfo->securityType = kQUICProtocolName;
  tinfo->protocolInfo = quicInfo_;
  if (sock_) {
    auto quicInfo = sock_->getTransportInfo();
    tinfo->rtt = quicInfo.srtt;
    tinfo->rtt_var = static_cast<int64_t>(quicInfo.rttvar.count());
    tinfo->caAlgo = std::string(
        congestionControlTypeToString(quicInfo.congestionControlType));
    // Cwnd is logged in terms of MSS.
    tinfo->cwnd =
        static_cast<int64_t>(quicInfo.congestionWindow / quicInfo.mss);
    tinfo->mss = quicInfo.mss;
    tinfo->cwndBytes = static_cast<int64_t>(quicInfo.congestionWindow);
    tinfo->rtx = static_cast<int64_t>(quicInfo.packetsRetransmitted);
    tinfo->rtx_tm = static_cast<int64_t>(quicInfo.timeoutBasedLoss);
    tinfo->rto = static_cast<int64_t>(quicInfo.pto.count());
    tinfo->totalBytes = static_cast<int64_t>(quicInfo.bytesSent);
    // For calculation of packet loss rate
    tinfo->totalPackets = static_cast<int64_t>(quicInfo.totalPacketsSent);
    tinfo->totalPacketsLost =
        static_cast<int64_t>(quicInfo.totalPacketsMarkedLost);
  }
  // TODO: fill up other properties.
  return true;
}

bool HQSession::getCurrentStreamTransportInfo(QuicStreamProtocolInfo* qspinfo,
                                              quic::StreamId streamId) {
  if (sock_) {
    auto streamTransportInfo = sock_->getStreamTransportInfo(streamId);
    if (streamTransportInfo) {
      qspinfo->streamTransportInfo = streamTransportInfo.value();
      return true;
    }
  }
  return false;
}

size_t HQSession::sendPriority(HTTPCodec::StreamID id, HTTPPriority priority) {
  if (streams_.find(id) == streams_.end() && !findPushStream(id)) {
    return 0;
  }
  sock_->setStreamPriority(id, toQuicPriority(priority));
  // PRIORITY_UPDATE frames are sent by clients on the control stream.
  // Servers do not send PRIORITY_UPDATE
  if (direction_ == TransportDirection::DOWNSTREAM) {
    return 0;
  }
  auto controlStream = findControlStream(UnidirectionalStreamType::CONTROL);
  if (!controlStream) {
    return 0;
  }
  auto g = folly::makeGuard(controlStream->setActiveCodec(__func__));
  auto ret = controlStream->codecFilterChain->generatePriority(
      controlStream->writeBuf_, id, priority);
  scheduleWrite();
  return ret;
}

size_t HQSession::sendPushPriority(hq::PushId pushId, HTTPPriority priority) {
  auto iter = pushIdToStreamId_.find(pushId);
  if (iter == pushIdToStreamId_.end()) {
    return 0;
  }
  auto streamId = iter->second;
  if (!findPushStream(streamId)) {
    LOG(ERROR) << "Cannot find push streamId=" << streamId
               << " with pushId=" << pushId << " presented in id map";
    return 0;
  }
  sock_->setStreamPriority(streamId, toQuicPriority(priority));
  auto controlStream = findControlStream(UnidirectionalStreamType::CONTROL);
  if (!controlStream) {
    return 0;
  }
  auto g = folly::makeGuard(controlStream->setActiveCodec(__func__));
  auto ret = controlStream->codecFilterChain->generatePushPriority(
      controlStream->writeBuf_, pushId, priority);
  scheduleWrite();
  return ret;
}

size_t HQSession::HQStreamTransportBase::changePriority(
    HTTPTransaction* txn, HTTPPriority priority) noexcept {
  CHECK_EQ(txn, &txn_);
  // For a client there is no point in changing priority if the response has
  // been fully received
  if (session_.direction_ == TransportDirection::UPSTREAM &&
      txn->isIngressEOMSeen()) {
    return 0;
  }
  if (txn->isPushed()) {
    auto pushId = txn->getID();
    return session_.sendPushPriority(pushId, priority);
  }
  return session_.sendPriority(txn->getID(), priority);
}

size_t HQSession::HQStreamTransportBase::sendPriority(
    HTTPTransaction*, const http2::PriorityUpdate&) noexcept {
  return 0;
}

size_t HQSession::HQStreamTransportBase::writeBufferSize() const {
  return writeBuf_.chainLength() + bufMeta_.length;
}

bool HQSession::HQStreamTransportBase::hasWriteBuffer() const {
  return writeBuf_.chainLength() != 0 || bufMeta_.length != 0;
}

bool HQSession::HQStreamTransportBase::hasPendingBody() const {
  return hasWriteBuffer() ||
         (queueHandle_.isTransactionEnqueued() && txn_.hasPendingBody());
}

bool HQSession::HQStreamTransportBase::hasPendingEOM() const {
  return pendingEOM_ ||
         (queueHandle_.isTransactionEnqueued() && txn_.isEgressEOMQueued());
}

bool HQSession::HQStreamTransportBase::hasPendingEgress() const {
  return hasWriteBuffer() || pendingEOM_ ||
         queueHandle_.isTransactionEnqueued();
}

bool HQSession::HQStreamTransportBase::wantsOnWriteReady(size_t canSend) const {
  // The txn wants onWriteReady if it's enqueued AND
  //   a) There is available flow control and it has body OR
  //   b) All body is egressed and it has only pending EOM
  return queueHandle_.isTransactionEnqueued() &&
         ((canSend > writeBufferSize() && txn_.hasPendingBody()) ||
          (!txn_.hasPendingBody() && txn_.isEgressEOMQueued()));
}

void HQSession::drainImpl() {
  if (drainState_ != DrainState::NONE) {
    // no op
    VLOG(5) << "Already draining sess=" << *this;
    return;
  }
  drainState_ = DrainState::PENDING;
  sendGoaway();
  setCloseReason(ConnectionCloseReason::SHUTDOWN);
}

void HQSession::sendGoaway() {
  if (direction_ == TransportDirection::UPSTREAM ||
      drainState_ == DrainState::DONE ||
      !versionUtilsReady_.allConditionsMet()) {
    return;
  }
  // send GOAWAY frame on the control stream
  DCHECK(drainState_ == DrainState::PENDING ||
         drainState_ == DrainState::FIRST_GOAWAY);

  auto connCtrlStream = findControlStream(UnidirectionalStreamType::CONTROL);
  auto g = folly::makeGuard(connCtrlStream->setActiveCodec(__func__));
  DCHECK(connCtrlStream);
  auto goawayStreamId = getGoawayStreamId();
  auto generated = connCtrlStream->codecFilterChain->generateGoaway(
      connCtrlStream->writeBuf_, goawayStreamId, ErrorCode::NO_ERROR);
  auto writeOffset =
      sock_->getStreamWriteOffset(connCtrlStream->getEgressStreamId());
  auto writeBufferedBytes =
      sock_->getStreamWriteBufferedBytes(connCtrlStream->getEgressStreamId());
  if (generated == 0 || writeOffset.hasError() ||
      writeBufferedBytes.hasError()) {
    // shortcut to shutdown
    LOG(ERROR) << " error generating GOAWAY sess=" << *this;
    drainState_ = DrainState::DONE;
    return;
  }
  VLOG(3) << "generated GOAWAY maxStreamID=" << goawayStreamId
          << " sess=" << *this;

  auto totalStreamLength = *writeOffset + *writeBufferedBytes +
                           connCtrlStream->writeBuf_.chainLength();
  CHECK_GT(totalStreamLength, 0);
  auto res =
      sock_->registerDeliveryCallback(connCtrlStream->getEgressStreamId(),
                                      totalStreamLength - 1,
                                      connCtrlStream);
  if (res.hasError()) {
    // shortcut to shutdown
    LOG(ERROR) << " error generating GOAWAY sess=" << *this;
    drainState_ = DrainState::DONE;
    return;
  }
  scheduleWrite();
  if (drainState_ == DrainState::PENDING) {
    drainState_ = DrainState::FIRST_GOAWAY;
  } else {
    DCHECK_EQ(drainState_, DrainState::FIRST_GOAWAY);
    drainState_ = DrainState::SECOND_GOAWAY;
  }
}

quic::StreamId HQSession::getGoawayStreamId() {
  DCHECK_NE(direction_, TransportDirection::UPSTREAM);
  if (drainState_ == DrainState::NONE || drainState_ == DrainState::PENDING) {
    return HTTPCodec::MaxStreamID;
  }
  // If/when we send client GOAWAYs, change this to return
  // minUnseenIncomingPushId_ in that case.
  return minUnseenIncomingStreamId_;
}

size_t HQSession::sendSettings() {
  for (auto& setting : egressSettings_.getAllSettings()) {
    auto id = httpToHqSettingsId(setting.id);
    if (id) {
      switch (*id) {
        case hq::SettingId::HEADER_TABLE_SIZE:
          qpackCodec_.setDecoderHeaderTableMaxSize(setting.value);
          break;
        case hq::SettingId::QPACK_BLOCKED_STREAMS:
          qpackCodec_.setMaxBlocking(setting.value);
          break;
        case hq::SettingId::MAX_HEADER_LIST_SIZE:
          // TODO: qpackCodec_.setMaxUncompressed(setting.value)
          break;
        case hq::SettingId::ENABLE_CONNECT_PROTOCOL:
        case hq::SettingId::H3_DATAGRAM:
        case hq::SettingId::H3_DATAGRAM_DRAFT_8:
        case hq::SettingId::H3_DATAGRAM_RFC:
        case hq::SettingId::WEBTRANSPORT_MAX_SESSIONS:
          break;
        case hq::SettingId::ENABLE_WEBTRANSPORT:
          if (setting.value) {
            VLOG(4) << "enable_webtransport sess=" << *this;
            supportsWebTransport_.set(
                folly::to_underlying(SettingEnabled::SELF));
          }
          break;
      }
    }
  }

  auto connCtrlStream = findControlStream(UnidirectionalStreamType::CONTROL);
  auto g = folly::makeGuard(connCtrlStream->setActiveCodec(__func__));
  DCHECK(connCtrlStream);
  auto generated = connCtrlStream->codecFilterChain->generateSettings(
      connCtrlStream->writeBuf_);
  scheduleWrite();
  return generated;
}

void HQSession::notifyPendingShutdown() {
  VLOG(4) << __func__ << " sess=" << *this;
  drainImpl();
}

void HQSession::closeWhenIdle() {
  VLOG(4) << __func__ << " sess=" << *this;
  drainImpl();
  cleanupPendingStreams();
  checkForShutdown();
}

void HQSession::dropConnection(const std::string& errorMsg) {
  auto msg = errorMsg.empty() ? "Stopping" : errorMsg;
  dropConnectionSync(quic::QuicError(HTTP3::ErrorCode::HTTP_NO_ERROR, msg),
                     kErrorDropped);
}

void HQSession::dropConnectionAsync(quic::QuicError errorCode,
                                    ProxygenError proxygenError) {
  if (!dropInNextLoop_.has_value()) {
    dropInNextLoop_ = std::make_pair(errorCode, proxygenError);
    scheduleLoopCallback(true);
  } else {
    VLOG(4) << "Session already scheduled to be dropped: sess=" << *this;
  }
}

void HQSession::dropConnectionSync(quic::QuicError errorCode,
                                   ProxygenError proxygenError) {
  VLOG(4) << __func__ << " sess=" << *this;
  HQSession::DestructorGuard dg(this);
  // dropping_ is used to guard against dropConnection->onError->dropConnection
  // re-entrancy. instead drainState_ = DONE means the connection can only be
  // deleted naturally in checkForShutdown.
  // We can get here with drainState_ == DONE, if somthing is holding a
  // DestructorGuardon the session when it gets dropped.
  if (dropping_) {
    VLOG(5) << "Already dropping sess=" << *this;
    return;
  }
  dropping_ = true;
  onConnectionSetupErrorHandler(errorCode);
  if (getNumStreams() > 0) {
    // should deliver errors to all open streams, they will all detach-
    sock_->close(std::move(errorCode));
    sock_.reset();
    setCloseReason(ConnectionCloseReason::SHUTDOWN);
    // If the txn had no registered cbs, there could be streams left
    // But we are not supposed to unregister the read callback, so this really
    // shouldn't happen
    invokeOnAllStreams([proxygenError](HQStreamTransportBase* stream) {
      stream->errorOnTransaction(proxygenError, "Dropped connection");
    });
  } else {
    // Can only be here if this wasn't fully drained. Cases like
    //  notify + drop  (PENDING)
    //  notify + CLOSE_SENT (in last request) + reset (no response) + drop
    //  CLOSE_RECEIVED (in last response) + drop
    // In any of these cases, it's ok to just close the socket.
    // Note that the socket could already be deleted in case multiple calls
    // happen, under a destructod guard.
    if (sock_) {
      // this should be closeNow()
      sock_->close(std::move(errorCode));
      sock_.reset();
    }
  }
  drainState_ = DrainState::DONE;
  cancelLoopCallback();
  checkForShutdown();
  if (VLOG_IS_ON(5)) {
    unidirectionalReadDispatcher_.invokeOnPendingStreamIDs(
        [&](quic::StreamId pendingStreamId) {
          VLOG(5) << __func__ << " pendingStreamStillOpen: " << pendingStreamId;
        });
  }
  CHECK_EQ(getNumStreams(), 0);
}

void HQSession::checkForShutdown() {
  // For HQ upstream connections with a control stream, if the client wants to
  // go away, it can just stop creating new connections and set drainining
  // state to DONE, so that it will just shut down the socket when all the
  // request streams are done. In the process it will still be able to receive
  // and process GOAWAYs from the server
  if (direction_ == TransportDirection::UPSTREAM &&
      drainState_ == DrainState::PENDING) {
    if (VLOG_IS_ON(5)) {
      unidirectionalReadDispatcher_.invokeOnPendingStreamIDs(
          [&](quic::StreamId pendingStreamId) {
            VLOG(5) << __func__
                    << " pendingStreamStillOpen: " << pendingStreamId;
          });
    }
    drainState_ = DrainState::DONE;
  }

  // This is somewhat inefficient, checking every stream for possible detach
  // when we know explicitly earlier which ones are ready.  This is here to
  // minimize issues with iterator invalidation.
  invokeOnAllStreams(
      [](HQStreamTransportBase* stream) { stream->checkForDetach(); });
  if (drainState_ == DrainState::DONE && (getNumStreams() == 0) &&
      !isLoopCallbackScheduled()) {
    if (sock_) {
      auto err = HTTP3::ErrorCode::HTTP_NO_ERROR;
      sock_->close(quic::QuicError(quic::QuicErrorCode(err), toString(err)));
      sock_.reset();
    }

    destroy();
  }
}

void HQSession::errorOnTransactionId(quic::StreamId id, HTTPException ex) {
  auto stream = findStream(id);
  if (stream) {
    stream->errorOnTransaction(std::move(ex));
  }
}

void HQSession::HQStreamTransportBase::errorOnTransaction(
    ProxygenError err, const std::string& errorMsg) {
  std::string extraErrorMsg;
  if (!errorMsg.empty()) {
    extraErrorMsg = folly::to<std::string>(". ", errorMsg);
  }
  auto streamIdStr =
      hasStreamId() ? folly::to<std::string>(getStreamId()) : "n/a";
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                   folly::to<std::string>(getErrorString(err),
                                          " on transaction id: ",
                                          streamIdStr,
                                          extraErrorMsg));
  ex.setProxygenError(err);
  errorOnTransaction(std::move(ex));
}

void HQSession::HQStreamTransportBase::errorOnTransaction(HTTPException ex) {
  auto isIngressException = ex.isIngressException();
  auto isEgressException = ex.isEgressException();
  if (!detached_) {
    txn_.onError(std::move(ex));
  }
  if (isIngressException) {
    abortIngress();
  }
  if (isEgressException) {
    abortEgress(true);
  }
}

HQSession::HQStreamTransportBase* FOLLY_NULLABLE
HQSession::findNonDetachedStream(quic::StreamId streamId) {
  return findStreamImpl(streamId,
                        true /* includeEgress */,
                        true /* includeIngress */,
                        false /* includeDetached */);
}

HQSession::HQStreamTransportBase* FOLLY_NULLABLE
HQSession::findStream(quic::StreamId streamId) {
  return findStreamImpl(streamId,
                        true /* includeEgress */,
                        true /* includeIngress */,
                        true /* includeDetached */);
}

HQSession::HQStreamTransportBase* FOLLY_NULLABLE
HQSession::findIngressStream(quic::StreamId streamId, bool includeDetached) {
  return findStreamImpl(streamId,
                        false /* includeEgress */,
                        true /* includeIngress */,
                        includeDetached);
}

HQSession::HQStreamTransportBase* FOLLY_NULLABLE
HQSession::findEgressStream(quic::StreamId streamId, bool includeDetached) {
  return findStreamImpl(streamId,
                        true /* includeEgress */,
                        false /* includeIngress */,
                        includeDetached);
}

HQSession::HQStreamTransportBase* FOLLY_NULLABLE
HQSession::findStreamImpl(quic::StreamId streamId,
                          bool includeEgress,
                          bool includeIngress,
                          bool includeDetached) {
  HQStreamTransportBase* pstream{nullptr};
  auto it = streams_.find(streamId);
  if (it != streams_.end()) {
    pstream = &it->second;
  }
  if (!pstream && (includeIngress || includeEgress)) {
    pstream = findPushStream(streamId);
  }
  if (!pstream) {
    return nullptr;
  }
  CHECK(pstream);
  DCHECK(pstream->isUsing(streamId));
  if (!includeDetached) {
    if (pstream->detached_) {
      return nullptr;
    }
  }
  return pstream;
}

HQSession::HQControlStream* FOLLY_NULLABLE
HQSession::findControlStream(UnidirectionalStreamType streamType) {
  auto it = controlStreams_.find(streamType);
  if (it == controlStreams_.end()) {
    return nullptr;
  } else {
    return &it->second;
  }
}

HQSession::HQControlStream* FOLLY_NULLABLE
HQSession::findControlStream(quic::StreamId streamId) {
  auto it = std::find_if(
      controlStreams_.begin(),
      controlStreams_.end(),
      [&](const std::pair<const UnidirectionalStreamType, HQControlStream>&
              entry) { return entry.second.isUsing(streamId); });
  if (it == controlStreams_.end()) {
    return nullptr;
  } else {
    return &it->second;
  }
}

bool HQSession::eraseStream(quic::StreamId streamId) {
  // Try different possible locations and remove the
  // stream
  bool erased = false;
  if (streams_.erase(streamId)) {
    erased = true;
  }

  // TODO: only do this when stream is server-uni
  erased |= erasePushStream(streamId);

  return erased;
}

void HQSession::runLoopCallback() noexcept {
  // We schedule this callback to run at the end of an event
  // loop iteration if either of two conditions has happened:
  //   * The session has generated some egress data (see scheduleWrite())
  //   * Reads have become unpaused (see resumeReads())

  inLoopCallback_ = true;
  HQSession::DestructorGuard dg(this);
  auto scopeg = folly::makeGuard([this] {
    // This ScopeGuard needs to be under the above DestructorGuard
    updatePendingWrites();
    checkForShutdown();
    inLoopCallback_ = false;
  });

  if (dropInNextLoop_.has_value()) {
    dropConnectionSync(dropInNextLoop_->first, dropInNextLoop_->second);
    return;
  }

  readsPerLoop_ = 0;

  // First process the read data
  //   - and maybe resume reads on the stream
  processReadData();

  readDataProcessed();

  // Then handle the writes
  // Write all the control streams first
  maxToSend_ -= writeControlStreams(maxToSend_);
  // Then write the request streams
  if (!txnEgressQueue_.empty() && maxToSend_ > 0) {
    // TODO: we could send FIN only?
    maxToSend_ = writeRequestStreams(maxToSend_);
  }
  // Zero out maxToSend_ here.  We won't egress anything else until the next
  // onWriteReady call
  maxToSend_ = 0;

  if (!txnEgressQueue_.empty()) {
    scheduleWrite();
  }

  // Maybe schedule the next loop callback
  VLOG(4) << "sess=" << *this << " maybe schedule the next loop callback. "
          << " pending writes: " << !txnEgressQueue_.empty()
          << " pending processing reads: " << pendingProcessReadSet_.size();
  if (!pendingProcessReadSet_.empty()) {
    scheduleLoopCallback(false);
  }
  // checkForShutdown is now in ScopeGuard
}

void HQSession::readDataProcessed() {
  auto ici = qpackCodec_.encodeInsertCountInc();
  if (ici) {
    auto QPACKDecoderStream =
        findControlStream(UnidirectionalStreamType::QPACK_DECODER);
    DCHECK(QPACKDecoderStream);
    QPACKDecoderStream->writeBuf_.append(std::move(ici));
    // don't need to explicitly schedule write because this is called in the
    // loop before control streams are written
  }
}

void HQSession::scheduleWrite() {
  // always call for the whole connection and iterate trough all the streams
  // in onWriteReady
  if (scheduledWrite_) {
    return;
  }

  scheduledWrite_ = true;
  sock_->notifyPendingWriteOnConnection(this);
}

void HQSession::scheduleLoopCallback(bool thisIteration) {
  if (sock_ && sock_->getEventBase()) {
    if (!isLoopCallbackScheduled()) {
      getEventBase()->runInLoop(this, thisIteration);
    }
  }
}

void HQSession::resumeReads(quic::StreamId streamId) {
  VLOG(4) << __func__ << " sess=" << *this
          << ": resuming reads id=" << streamId;
  sock_->resumeRead(streamId);
  scheduleLoopCallback(true);
  // TODO: ideally we should cancel the managed timeout when all the streams are
  // paused and then restart it when the timeouts are unpaused
}

void HQSession::resumeReads() {
  VLOG(4) << __func__ << " sess=" << *this << ": resuming reads";
  invokeOnIngressStreams([this](HQStreamTransportBase* hqStream) {
    if (sock_) {
      sock_->resumeRead(hqStream->getIngressStreamId());
    }
  });
}

void HQSession::pauseReads(quic::StreamId streamId) {
  VLOG(4) << __func__ << " sess=" << *this << ": pausing reads id=" << streamId;
  sock_->pauseRead(streamId);
}

void HQSession::pauseReads() {
  VLOG(4) << __func__ << " sess=" << *this << ": pausing reads";
  invokeOnIngressStreams([this](HQStreamTransportBase* hqStream) {
    sock_->pauseRead(hqStream->getIngressStreamId());
  });
}

void HQSession::readAvailable(quic::StreamId id) noexcept {
  // this is the bidirectional callback
  VLOG(4) << __func__ << " sess=" << *this
          << ": readAvailable on streamID=" << id;
  if (readsPerLoop_ >= kMaxReadsPerLoop) {
    VLOG(2) << __func__ << ": skipping read for streamID=" << id
            << " maximum reads per loop reached"
            << " sess=" << *this;
    return;
  }
  readsPerLoop_++;
  readRequestStream(id);

  scheduleLoopCallback(true);
}

void HQSession::readError(quic::StreamId id, quic::QuicError error) noexcept {
  VLOG(4) << __func__ << " sess=" << *this << ": readError streamID=" << id
          << " error: " << error;

  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                   folly::to<std::string>("Got error=", quic::toString(error)));

  switch (error.code.type()) {
    case quic::QuicErrorCode::Type::ApplicationErrorCode: {
      auto errorCode =
          static_cast<HTTP3::ErrorCode>(*error.code.asApplicationErrorCode());
      VLOG(3) << "readError: QUIC Application Error: " << toString(errorCode)
              << " streamID=" << id << " sess=" << *this;
      ex.setHttp3ErrorCode(errorCode);
      auto stream = findNonDetachedStream(id);
      if (stream) {
        stream->onResetStream(errorCode, std::move(ex));
      } else {
        // When a stream is erased, it's callback is cancelled, so it really
        // should be here
        VLOG(3)
            << "readError: received application error=" << toString(errorCode)
            << " for detached streamID=" << id << " sess=" << *this;
      }
      break;
    }
    case quic::QuicErrorCode::Type::LocalErrorCode: {
      quic::LocalErrorCode& errorCode = *error.code.asLocalErrorCode();
      VLOG(3) << "readError: QUIC Local Error: " << errorCode
              << " streamID=" << id << " sess=" << *this;
      if (errorCode == quic::LocalErrorCode::CONNECT_FAILED) {
        ex.setProxygenError(kErrorConnect);
      } else if (errorCode == quic::LocalErrorCode::IDLE_TIMEOUT) {
        ex.setProxygenError(kErrorEOF);
      } else {
        ex.setProxygenError(kErrorShutdown);
      }
      errorOnTransactionId(id, std::move(ex));
      break;
    }
    case quic::QuicErrorCode::Type::TransportErrorCode: {
      quic::TransportErrorCode& errorCode = *error.code.asTransportErrorCode();
      VLOG(3) << "readError: QUIC Transport Error: " << errorCode
              << " streamID=" << id << " sess=" << *this;
      ex.setProxygenError(kErrorConnectionReset);
      // TODO: set Quic error when fbcode/quic/QuicConstants.h is OSS
      ex.setErrno(uint32_t(errorCode));
      errorOnTransactionId(id, std::move(ex));
      break;
    }
  }
}

void HQSession::timeoutExpired() noexcept {
  VLOG(3) << "ManagedConnection timeoutExpired " << *this;
  if (getNumStreams() > 0) {
    VLOG(3) << "ignoring session timeout " << *this;
    resetTimeout();
    return;
  }
  VLOG(3) << "Timeout with nothing pending " << *this;
  setCloseReason(ConnectionCloseReason::TIMEOUT);
  closeWhenIdle();
}

folly::Optional<UnidirectionalStreamType> HQSession::parseUniStreamPreface(
    uint64_t preface) {
  hq::UnidirectionalTypeF parse = [](hq::UnidirectionalStreamType type)
      -> folly::Optional<UnidirectionalStreamType> { return type; };
  auto res = hq::withType(preface, parse);
  if (res && *res == hq::UnidirectionalStreamType::WEBTRANSPORT &&
      !supportsWebTransport()) {
    LOG(ERROR) << "WT stream when it is unsupported sess=" << *this;
    return folly::none;
  }
  return res;
}

void HQSession::readControlStream(HQControlStream* ctrlStream) {
  DCHECK(ctrlStream);
  auto readRes = sock_->read(ctrlStream->getIngressStreamId(), 0);
  if (readRes.hasError()) {
    LOG(ERROR) << "Got synchronous read error=" << readRes.error();
    readError(ctrlStream->getIngressStreamId(),
              quic::QuicError(readRes.error(), "sync read error"));
    return;
  }
  resetTimeout();
  quic::Buf data = std::move(readRes.value().first);
  auto readSize = data ? data->computeChainDataLength() : 0;
  VLOG(4) << "Read " << readSize << " bytes from control stream";
  ctrlStream->readBuf_.append(std::move(data));
  ctrlStream->readEOF_ = readRes.value().second;

  if (infoCallback_) {
    infoCallback_->onRead(
        *this,
        readSize,
        static_cast<HTTPCodec::StreamID>(ctrlStream->getIngressStreamId()));
  }
  // GOAWAY may trigger session destroy, need a guard for that
  DestructorGuard dg(this);
  ctrlStream->processReadData();
}

// Dispatcher method implementation
void HQSession::dispatchControlStream(quic::StreamId id,
                                      hq::UnidirectionalStreamType type,
                                      size_t toConsume) {
  VLOG(4) << __func__ << " streamID=" << id << " type=" << type
          << " toConsume=" << toConsume;

  auto consumeRes = sock_->consume(id, toConsume);
  CHECK(!consumeRes.hasError()) << "Unexpected error consuming bytes";

  // Notify the read callback
  if (infoCallback_) {
    infoCallback_->onRead(
        *this, toConsume, static_cast<HTTPCodec::StreamID>(id));
  }

  auto ctrlStream = createIngressControlStream(id, type);
  if (!ctrlStream) {
    rejectStream(id);
    return;
  }
  sock_->setControlStream(id);

  // After reading the preface we can switch to the regular readCallback
  sock_->setPeekCallback(id, nullptr);
  sock_->setReadCallback(id, &controlStreamReadCallback_);

  // The transport will send notifications via the read callback
  // for the *future* events, but not for this one.
  // In case there is additional data on the control stream,
  // it can be not seen until the next read notification.
  // To mitigate that, we propagate the onReadAvailable to the control stream.
  controlStreamReadAvailable(id);
}

void HQSession::rejectStream(quic::StreamId id) {
  if (!sock_) {
    return;
  }
  // Do not read data for unknown unidirectional stream types.  Send
  // STOP_SENDING and rely on the peer sending a RESET to clear the stream in
  // the transport, or the transport to detect if the peer has sent everything.
  VLOG(4) << "rejectStream id=" << id;
  sock_->stopSending(id, HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
  if (sock_->isBidirectionalStream(id)) {
    sock_->resetStream(id, HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR);
  }
  sock_->setReadCallback(id, nullptr, folly::none);
  sock_->setPeekCallback(id, nullptr);
}

size_t HQSession::cleanupPendingStreams() {
  std::vector<quic::StreamId> streamsToCleanup;

  // Cleanup pending streams in the dispatchers
  unidirectionalReadDispatcher_.cleanup();
  bidirectionalReadDispatcher_.cleanup();

  // These streams have been dispatched as push streams but are missing their
  // push promise
  cleanupUnboundPushStreams(streamsToCleanup);

  // Clean up the streams by detaching all callbacks
  for (auto pendingStreamId : streamsToCleanup) {
    clearStreamCallbacks(pendingStreamId);
  }

  return streamsToCleanup.size();
}

void HQSession::clearStreamCallbacks(quic::StreamId id) {
  if (sock_) {
    sock_->setReadCallback(id, nullptr, folly::none);
    sock_->setPeekCallback(id, nullptr);
    sock_->setDSRPacketizationRequestSender(id, nullptr);

  } else {
    VLOG(4) << "Attempt to clear callbacks on closed socket";
  }
}

void HQSession::controlStreamReadAvailable(quic::StreamId id) {
  VLOG(4) << __func__ << " sess=" << *this << ": streamID=" << id;
  auto ctrlStream = findControlStream(id);
  if (!ctrlStream) {
    LOG(ERROR) << "Got readAvailable on unknown stream id=" << id
               << " sess=" << *this;
    return;
  }
  readControlStream(ctrlStream);
}

void HQSession::controlStreamReadError(quic::StreamId id,
                                       const quic::QuicError& error) {
  VLOG(4) << __func__ << " sess=" << *this << ": readError streamID=" << id
          << " error: " << error;

  auto ctrlStream = findControlStream(id);

  if (!ctrlStream) {
    const quic::LocalErrorCode* err = error.code.asLocalErrorCode();
    bool shouldLog = !err || (*err != quic::LocalErrorCode::NO_ERROR);
    LOG_IF(ERROR, shouldLog)
        << __func__ << " received read error=" << error
        << " for unknown control streamID=" << id << " sess=" << *this;
    return;
  }

  handleSessionError(ctrlStream,
                     StreamDirection::INGRESS,
                     quicControlStreamError(error.code),
                     toProxygenError(error.code));
}

void HQSession::readRequestStream(quic::StreamId id) noexcept {
  auto hqStream = findIngressStream(id, false /* includeDetached */);
  if (!hqStream) {
    // can we even get readAvailable after a stream is marked for detach ?
    DCHECK(findStream(id));
    return;
  }
  // Read as much as you possibly can!
  auto readRes = sock_->read(id, 0);

  if (readRes.hasError()) {
    LOG(ERROR) << "Got synchronous read error=" << readRes.error();
    readError(id, quic::QuicError(readRes.error(), "sync read error"));
    return;
  }

  resetTimeout();
  quic::Buf data = std::move(readRes.value().first);
  auto readSize = data ? data->computeChainDataLength() : 0;
  hqStream->readEOF_ = readRes.value().second;
  VLOG(3) << "Got streamID=" << hqStream->getStreamId() << " len=" << readSize
          << " eof=" << uint32_t(hqStream->readEOF_) << " sess=" << *this;
  if (hqStream->readEOF_) {
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - hqStream->createdTime);
    if (sock_ && sock_->getState() && sock_->getState()->qLogger) {
      sock_->getState()->qLogger->addStreamStateUpdate(
          id, quic::kOnEOM, timeDiff);
    }
  } else if (readSize == 0) {
    VLOG(3) << "Got a blank read, ignoring sess=" << *this;
    return;
  }
  // Just buffer the data and postpone processing in the loop callback
  hqStream->readBuf_.append(std::move(data));

  if (infoCallback_) {
    infoCallback_->onRead(*this, readSize, hqStream->getStreamId());
  }

  pendingProcessReadSet_.insert(id);
}

void HQSession::processReadData() {
  for (auto it = pendingProcessReadSet_.begin();
       it != pendingProcessReadSet_.end();) {
    auto g = folly::makeGuard([&]() {
      // the codec may not have processed all the data, but we won't ask again
      // until we get more
      // TODO: set a timeout?
      it = pendingProcessReadSet_.erase(it);
    });

    HQStreamTransportBase* ingressStream =
        findIngressStream(*it, true /* includeDetached */);

    if (!ingressStream) {
      // ingress on a transaction may cause other transactions to get deleted
      continue;
    }

    // Check whether the stream has been detached
    if (ingressStream->detached_) {
      VLOG(4) << __func__ << " killing pending read data for detached txn="
              << ingressStream->txn_;
      ingressStream->readBuf_.move();
      ingressStream->readEOF_ = false;
      continue;
    }

    // Feed it to the codec
    auto blocked = ingressStream->processReadData();
    if (!blocked) {
      if (ingressStream->readEOF_) {
        ingressStream->onIngressEOF();
      }
      continue;
    }
  }
}

void HQSession::headersComplete(HTTPMessage* /*msg*/) {
  auto QPACKDecoderStream =
      findControlStream(UnidirectionalStreamType::QPACK_DECODER);

  if (QPACKDecoderStream && !QPACKDecoderStream->writeBuf_.empty()) {
    scheduleWrite();
  }
}

void HQSession::onSettings(const SettingsList& settings) {
  applySettings(settings);
  if (infoCallback_) {
    infoCallback_->onSettings(*this, settings);
  }
  receivedSettings_ = true;
}

void HQSession::applySettings(const SettingsList& settings) {
  DestructorGuard g(this);
  VLOG(3) << "Got SETTINGS sess=" << *this;

  uint32_t tableSize = kDefaultIngressHeaderTableSize;
  uint32_t blocked = kDefaultIngressQpackBlockedStream;
  bool datagram = false;
  bool hasWT = false;
  FOLLY_MAYBE_UNUSED uint32_t numPlaceholders = kDefaultIngressNumPlaceHolders;
  for (auto& setting : settings) {
    auto id = httpToHqSettingsId(setting.id);
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
          // TODO
          break;
        case hq::SettingId::H3_DATAGRAM:
        case hq::SettingId::H3_DATAGRAM_DRAFT_8:
        case hq::SettingId::H3_DATAGRAM_RFC:
          datagram = static_cast<bool>(setting.value);
          break;
        case hq::SettingId::ENABLE_WEBTRANSPORT:
          hasWT = setting.value;
          LOG(INFO) << "Peer sent ENABLE_WEBTRANSPORT: " << uint32_t(hasWT);
          supportsWebTransport_.set(folly::to_underlying(SettingEnabled::PEER));
          break;
        case hq::SettingId::WEBTRANSPORT_MAX_SESSIONS:
          break;
      }
    }
  }
  qpackCodec_.setEncoderHeaderTableSize(tableSize);
  qpackCodec_.setMaxVulnerable(blocked);

  // If H3 datagram is enabled but datagram was not negotiated at the
  // transport, close the connection
  if (datagram && sock_->getDatagramSizeLimit() == 0) {
    dropConnectionAsync(
        quic::QuicError(HTTP3::ErrorCode::HTTP_SETTINGS_ERROR,
                        "H3_DATAGRAM without transport support"),
        kErrorConnection);
  }
  // H3 Datagram flows are bi-directional, enable only of local and peer
  // support it
  datagramEnabled_ &= datagram;

  VLOG(3) << "Applied SETTINGS sess=" << *this << " size=" << tableSize
          << " blocked=" << blocked;
}

void HQSession::onGoaway(uint64_t minUnseenId,
                         ErrorCode code,
                         std::unique_ptr<folly::IOBuf> /* debugData */) {
  // NOTE: This function needs to be idempotent. i.e. be a no-op if invoked
  // twice with the same lastGoodStreamID
  if (direction_ == TransportDirection::DOWNSTREAM) {
    VLOG(3) << "Ignoring downstream GOAWAY minUnseenId=" << minUnseenId
            << " sess=" << *this;
    return;
  }
  VLOG(3) << "Got GOAWAY minUnseenId=" << minUnseenId << " sess=" << *this;
  if (minUnseenId > peerMinUnseenId_) {
    LOG(ERROR) << "Goaway id increased=" << minUnseenId << " sess=" << *this;
    dropConnectionAsync(
        quic::QuicError(HTTP3::ErrorCode::HTTP_ID_ERROR, "GOAWAY id increased"),
        kErrorMalformedInput);
    return;
  }
  peerMinUnseenId_ = minUnseenId;
  setCloseReason(ConnectionCloseReason::GOAWAY);
  // drains existing streams and prevents new streams to be created
  drainImpl();

  invokeOnNonDetachedStreams([this, code](HQStreamTransportBase* stream) {
    // Invoke onGoaway on all transactions
    stream->txn_.onGoaway(code);
    // Abort transactions which have been initiated locally but not created
    // successfully at the remote end
    if (stream->getStreamId() >= peerMinUnseenId_) {
      stream->errorOnTransaction(kErrorStreamUnacknowledged, "");
    }
  });

  if (drainState_ == DrainState::NONE || drainState_ == DrainState::PENDING) {
    drainState_ = DrainState::FIRST_GOAWAY;
  } else if (drainState_ == DrainState::FIRST_GOAWAY) {
    drainState_ = DrainState::DONE;
  }
  checkForShutdown();
}

void HQSession::onPriority(quic::StreamId streamId, const HTTPPriority& pri) {
  CHECK_EQ(direction_, TransportDirection::DOWNSTREAM);
  if (drainState_ != DrainState::NONE) {
    return;
  }
  CHECK(sock_);
  // This also covers push streams:
  auto stream = findStream(streamId);
  if (!stream || (!stream->txn_.isPushed() && !stream->hasHeaders_)) {
    // We are supposed to drop the connection with HTTP_ID_ERROR if the streamId
    // points to a control stream. But why should I spend CPU looking it up?
    priorityUpdatesBuffer_.insert(streamId, pri);
    return;
  }
  sock_->setStreamPriority(streamId, toQuicPriority(pri));
}

void HQSession::onPushPriority(hq::PushId pushId, const HTTPPriority& pri) {
  CHECK_EQ(direction_, TransportDirection::DOWNSTREAM);
  if (drainState_ != DrainState::NONE) {
    return;
  }
  CHECK(sock_);

  if (maxAllowedPushId_.hasValue() && *maxAllowedPushId_ < pushId) {
    VLOG(4) << "Priority update stream id=" << pushId
            << " greater than max allowed push id=" << *maxAllowedPushId_;
    dropConnectionAsync(quic::QuicError(HTTP3::ErrorCode::HTTP_ID_ERROR,
                                        "PushId is beyond max allowed push id"),
                        kErrorMalformedInput);
    return;
  }
  auto iter = pushIdToStreamId_.find(pushId);
  if (iter == pushIdToStreamId_.end()) {
    VLOG(4) << "Priority update of unknown push id=" << pushId;
    return;
  }
  auto streamId = iter->second;
  auto stream = findPushStream(streamId);
  if (!stream) {
    return;
  }
  sock_->setStreamPriority(streamId, toQuicPriority(pri));
}

void HQSession::notifyEgressBodyBuffered(int64_t bytes) {
  if (HTTPSessionBase::notifyEgressBodyBuffered(bytes, true) &&
      !inLoopCallback_ && !isLoopCallbackScheduled() && sock_) {
    getEventBase()->runInLoop(this);
  }
}

void HQSession::onFlowControlUpdate(quic::StreamId id) noexcept {
  VLOG(4) << __func__ << " sess=" << *this << ": streamID=" << id;

  auto flowControl = sock_->getStreamFlowControl(id);
  if (flowControl.hasError()) {
    LOG(ERROR) << "Got error=" << flowControl.error() << " streamID=" << id;
    return;
  }

  auto ctrlStream = findControlStream(id);
  if (ctrlStream && flowControl->sendWindowAvailable > 0) {
    if (sock_ && sock_->getState() && sock_->getState()->qLogger) {
      sock_->getState()->qLogger->addStreamStateUpdate(
          id,
          quic::getFlowControlWindowAvailable(flowControl->sendWindowAvailable),
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - ctrlStream->createdTime));
    }
    scheduleWrite();
    return;
  }

  auto stream = findEgressStream(id);

  if (!stream) {
    LOG(ERROR) << "Got flow control update for unknown streamID=" << id
               << " sess=" << this;
    return;
  }

  auto& txn = stream->txn_;
  // Check if this stream has flow control, or has only EOM pending
  if (flowControl->sendWindowAvailable > 0 ||
      (!stream->hasPendingBody() && stream->hasPendingEOM())) {
    // TODO: are we intentionally piggyback the time value for flow control
    // window here?
    if (sock_ && sock_->getState() && sock_->getState()->qLogger) {
      sock_->getState()->qLogger->addStreamStateUpdate(
          id,
          quic::getFlowControlWindowAvailable(flowControl->sendWindowAvailable),
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - stream->createdTime));
    }
    if (stream->hasPendingEgress()) {
      txnEgressQueue_.signalPendingEgress(stream->queueHandle_.getHandle());
    }
    if (!stream->detached_ && txn.isEgressPaused()) {
      // txn might be paused
      txn.resumeEgress();
    }
    scheduleWrite();
  }
}

void HQSession::onConnectionWriteReady(uint64_t maxToSend) noexcept {
  VLOG(4) << __func__ << " sess=" << *this << ": maxToSend=" << maxToSend;
  scheduledWrite_ = false;
  maxToSend_ = maxToSend;

  scheduleLoopCallback(true);
}

void HQSession::onConnectionWriteError(quic::QuicError error) noexcept {
  scheduledWrite_ = false;
  VLOG(4) << __func__ << " sess=" << *this << ": writeError error=" << error;
  // Leave this as a no-op.  We will most likely get onConnectionError soon
}

uint64_t HQSession::writeControlStreams(uint64_t maxEgress) {
  uint64_t maxEgressOrig = maxEgress;
  // NOTE: process the control streams in the order they are stored
  // this could potentially lead to stream starvation
  for (auto& it : controlStreams_) {
    if (it.second.writeBuf_.empty()) {
      continue;
    }
    auto sent = controlStreamWriteImpl(&it.second, maxEgress);
    DCHECK_LE(sent, maxEgress);
    maxEgress -= sent;
    if (maxEgress == 0) {
      break;
    }
  }
  return maxEgressOrig - maxEgress;
}

uint64_t HQSession::controlStreamWriteImpl(HQControlStream* ctrlStream,
                                           uint64_t maxEgress) {
  auto egressStreamId = ctrlStream->getEgressStreamId();
  auto flowControl = sock_->getStreamFlowControl(egressStreamId);
  if (flowControl.hasError()) {
    LOG(ERROR)
        << "Got error=" << flowControl.error() << " streamID=" << egressStreamId
        << " bufLen=" << static_cast<int>(ctrlStream->writeBuf_.chainLength())
        << " readEOF=" << ctrlStream->readEOF_;
    handleSessionError(ctrlStream,
                       StreamDirection::EGRESS,
                       quicControlStreamError(flowControl.error()),
                       toProxygenError(flowControl.error()));
    return 0;
  }

  auto streamSendWindow = flowControl->sendWindowAvailable;
  size_t canSend = std::min(streamSendWindow, maxEgress);
  auto sendLen = std::min(canSend, ctrlStream->writeBuf_.chainLength());
  auto tryWriteBuf = ctrlStream->writeBuf_.splitAtMost(canSend);

  VLOG(4) << __func__ << " before write sess=" << *this
          << ": streamID=" << egressStreamId << " maxEgress=" << maxEgress
          << " sendWindow=" << streamSendWindow << " sendLen=" << sendLen;

  auto writeRes = sock_->writeChain(
      egressStreamId, std::move(tryWriteBuf), false /*eof*/, nullptr);

  if (writeRes.hasError()) {
    LOG(ERROR) << " Got error=" << writeRes.error()
               << " streamID=" << egressStreamId;
    // Going to call this a write error no matter what the underlying reason was
    handleSessionError(ctrlStream,
                       StreamDirection::EGRESS,
                       quicControlStreamError(writeRes.error()),
                       kErrorWrite);
    return 0;
  }

  VLOG(4)
      << __func__ << " after write sess=" << *this
      << ": streamID=" << ctrlStream->getEgressStreamId() << " sent=" << sendLen
      << " buflen=" << static_cast<int>(ctrlStream->writeBuf_.chainLength());
  if (infoCallback_) {
    infoCallback_->onWrite(*this, sendLen);
  }

  return sendLen;
}

void HQSession::handleSessionError(HQStreamBase* stream,
                                   StreamDirection streamDir,
                                   quic::QuicErrorCode err,
                                   ProxygenError proxygenError) {
  // This is most likely a control stream
  std::string appErrorMsg;
  HTTP3::ErrorCode appError = HTTP3::ErrorCode::HTTP_NO_ERROR;
  auto ctrlStream = dynamic_cast<HQControlStream*>(stream);
  if (ctrlStream) {
    auto id = (streamDir == StreamDirection::EGRESS
                   ? ctrlStream->getEgressStreamId()
                   : ctrlStream->getIngressStreamId());
    // We will miss spurious control stream RST or write errors in the logs
    VLOG(3) << "Got error on control stream error=" << err << " streamID=" << id
            << " Dropping connection. sess=" << *this;
    appErrorMsg = "HTTP error on control stream";
  } else {
    auto requestStream = dynamic_cast<HQStreamTransport*>(stream);
    CHECK(requestStream);
    auto id = requestStream->getEgressStreamId();
    LOG(ERROR) << "Got error on request stream error=" << err
               << " streamID=" << id << " Dropping connection. sess=" << *this;
    appErrorMsg = "HTTP error on request stream";
    // for request streams this function must be called with an ApplicationError
    DCHECK(err.asApplicationErrorCode());
  }
  // errors on a control stream means we must drop the entire connection,
  // but there are some errors that we expect during shutdown
  bool shouldDrop = false;
  switch (err.type()) {
    case quic::QuicErrorCode::Type::ApplicationErrorCode:
      // An ApplicationErrorCode is expected when
      //  1. The peer resets a control stream
      //  2. A control codec detects a connection error on a control stream
      //  3. A stream codec detects a connection level error (eg: compression)
      // We always want to drop the connection in these cases.
      appError = static_cast<HTTP3::ErrorCode>(*err.asApplicationErrorCode());
      shouldDrop = true;
      break;
    case quic::QuicErrorCode::Type::LocalErrorCode:
      // a LocalErrorCode::NO_ERROR is expected whenever the socket gets
      // closed without error
      shouldDrop =
          (*err.asLocalErrorCode() != quic::LocalErrorCode::NO_ERROR &&
           *err.asLocalErrorCode() != quic::LocalErrorCode::SHUTTING_DOWN);
      break;
    case quic::QuicErrorCode::Type::TransportErrorCode:
      shouldDrop = true;
      break;
  }
  if (!shouldDrop) {
    return;
  }
  if (ctrlStream && appError == HTTP3::ErrorCode::HTTP_NO_ERROR) {
    // If we got a local or transport error reading or writing on a
    // control stream, send CLOSED_CRITICAL_STREAM.
    appError = HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM;
  }
  // we cannot just simply drop the connection here, since in case of a
  // close received from the remote, we may have other readError callbacks on
  // other streams after this one. So run in the next loop callback, in this
  // same loop
  dropConnectionAsync(quic::QuicError(appError, appErrorMsg), proxygenError);
}

uint64_t HQSession::writeRequestStreams(uint64_t maxEgress) noexcept {
  // requestStreamWriteImpl may call txn->onWriteReady
  txnEgressQueue_.nextEgress(nextEgressResults_);
  for (auto it = nextEgressResults_.begin(); it != nextEgressResults_.end();
       ++it) {
    auto& ratio = it->second;
    auto hqStream =
        static_cast<HQStreamTransportBase*>(&it->first->getTransport());
    // TODO: scale maxToSend by ratio?
    auto sent = requestStreamWriteImpl(hqStream, maxEgress, ratio);
    DCHECK_LE(sent, maxEgress);
    maxEgress -= sent;

    if (maxEgress == 0 && std::next(it) != nextEgressResults_.end()) {
      VLOG(3) << __func__ << " sess=" << *this
              << " got more to send than the transport could take";
      break;
    }
  }
  nextEgressResults_.clear();
  return maxEgress;
}

void HQSession::handleWriteError(HQStreamTransportBase* hqStream,
                                 quic::QuicErrorCode err) {
  // We call this INGRESS_AND_EGRESS so it fully terminates the
  // HTTPTransaction state machine.
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                   folly::to<std::string>("Got error=", quic::toString(err)));
  switch (err.type()) {
    case quic::QuicErrorCode::Type::ApplicationErrorCode: {
      // If we have an application error code, it must have
      // come from the peer (most likely STOP_SENDING). This
      // is logically a stream abort, not a write error
      auto h3ErrorCode =
          static_cast<HTTP3::ErrorCode>(*err.asApplicationErrorCode());
      ex.setHttp3ErrorCode(h3ErrorCode);
      ex.setCodecStatusCode(hqToHttpErrorCode(h3ErrorCode));
      ex.setProxygenError(h3ErrorCode == HTTP3::ErrorCode::HTTP_REQUEST_REJECTED
                              ? kErrorStreamUnacknowledged
                              : kErrorStreamAbort);
      break;
    }
    case quic::QuicErrorCode::Type::LocalErrorCode: {
      ex.setErrno(uint32_t(*err.asLocalErrorCode()));
      ex.setProxygenError(kErrorWrite);
      break;
    }
    case quic::QuicErrorCode::Type::TransportErrorCode: {
      CHECK(false) << "Unexpected errorCode=" << *err.asTransportErrorCode();
      break;
    }
  }
  // Do I need a dguard here?
  abortStream(ex.getDirection(),
              hqStream->getStreamId(),
              HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED);
  hqStream->errorOnTransaction(std::move(ex));
}

template <typename WriteFunc, typename DataType>
size_t HQSession::handleWrite(WriteFunc writeFunc,
                              HQStreamTransportBase* hqStream,
                              DataType dataType,
                              size_t dataChainLen,
                              bool sendEof) {
  quic::QuicSocket::DeliveryCallback* deliveryCallback =
      sendEof ? this : nullptr;
  auto writeRes = writeFunc(hqStream->getEgressStreamId(),
                            std::forward<DataType>(dataType),
                            sendEof,
                            deliveryCallback);
  if (writeRes.hasError()) {
    LOG(ERROR) << " Got error=" << writeRes.error()
               << " streamID=" << hqStream->getEgressStreamId();
    handleWriteError(hqStream, writeRes.error());
    return 0;
  }

  auto sent = dataChainLen;
  if (sendEof) {
    // This will hold the transaction open until onDeliveryAck or onCanceled
    DCHECK(deliveryCallback);
    hqStream->txn_.incrementPendingByteEvents();
    // NOTE: This may not be necessary long term, once we properly implement
    // detach or when we enforce flow control for headers and EOM
    hqStream->pendingEOM_ = false;
  }
  hqStream->bytesWritten_ += sent;
  // hqStream's byteEventTracker cannot be changed, so no need to pass
  // shared_ptr or use in while loop
  hqStream->byteEventTracker_.processByteEvents(
      nullptr, hqStream->streamEgressCommittedByteOffset());
  return sent;
}

uint64_t HQSession::requestStreamWriteImpl(HQStreamTransportBase* hqStream,
                                           uint64_t maxEgress,
                                           double ratio) {
  CHECK(hqStream->queueHandle_.isStreamTransportEnqueued());
  HTTPTransaction::DestructorGuard dg(&hqStream->txn_);

  auto streamId = hqStream->getStreamId();
  auto flowControl = sock_->getStreamFlowControl(streamId);
  if (flowControl.hasError()) {
    LOG(ERROR)
        << "Got error=" << flowControl.error() << " streamID=" << streamId
        << " detached=" << hqStream->detached_
        << " readBufLen=" << static_cast<int>(hqStream->readBuf_.chainLength())
        << " writeBufLen=" << static_cast<int>(hqStream->writeBufferSize())
        << " readEOF=" << hqStream->readEOF_
        << " ingressError_=" << static_cast<int>(hqStream->ingressError_)
        << " eomGate_=" << hqStream->eomGate_;
    handleWriteError(hqStream, flowControl.error());
    return 0;
  }

  auto streamSendWindow = flowControl->sendWindowAvailable;

  size_t canSend = std::min(streamSendWindow, maxEgress);

  // we may have already buffered more than the amount the transport can take,
  // or the txn may not have any more body bytes/EOM to add. In case, there is
  // no need to call txn->onWriteReady.
  if (hqStream->wantsOnWriteReady(canSend)) {
    // Populate the write buffer by telling the transaction how much
    // room is available for body data
    size_t maxBodySend = canSend - hqStream->writeBufferSize();
    VLOG(4) << __func__ << " asking txn for more bytes sess=" << *this
            << ": streamID=" << streamId << " canSend=" << canSend
            << " remain=" << hqStream->writeBufferSize()
            << " pendingEOM=" << hqStream->pendingEOM_
            << " maxBodySend=" << maxBodySend << " ratio=" << ratio;
    hqStream->txn_.onWriteReady(maxBodySend, ratio);
    // onWriteReady may not be able to detach any byte from the deferred egress
    // body bytes, in case it's getting rate limited.
    // In that case the txn will get removed from the egress queue from
    // onWriteReady
    if (!hqStream->hasWriteBuffer() && !hqStream->pendingEOM_) {
      return 0;
    }
  }

  auto bufWritter = [&](quic::StreamId streamId,
                        std::unique_ptr<folly::IOBuf> data,
                        bool sendEof,
                        quic::QuicSocket::DeliveryCallback* deliveryCallback) {
    return sock_->writeChain(
        streamId, std::move(data), sendEof, deliveryCallback);
  };
  auto bufMetaWritter =
      [&](quic::StreamId streamId,
          quic::BufferMeta bufMeta,
          bool sendEof,
          quic::QuicSocket::DeliveryCallback* deliveryCallback) {
        return sock_->writeBufMeta(
            streamId, bufMeta, sendEof, deliveryCallback);
      };

  size_t sent = 0;
  auto bufSendLen = std::min(canSend, hqStream->writeBuf_.chainLength());
  auto tryWriteBuf = hqStream->writeBuf_.splitAtMost(canSend);
  bool sendEof = (hqStream->pendingEOM_ && !hqStream->hasPendingBody());
  if (bufSendLen > 0 || sendEof) {
    VLOG(4) << __func__ << " before write sess=" << *this
            << ": streamID=" << streamId << " maxEgress=" << maxEgress
            << " sendWindow=" << streamSendWindow
            << " tryToSend=" << tryWriteBuf->computeChainDataLength()
            << " sendEof=" << sendEof;
    sent = handleWrite(std::move(bufWritter),
                       hqStream,
                       std::move(tryWriteBuf),
                       bufSendLen,
                       sendEof);
  }
  auto bufMetaWriteLen =
      std::min(canSend - bufSendLen, hqStream->bufMeta_.length);
  auto splitBufMeta = hqStream->bufMeta_.split(bufMetaWriteLen);
  // Refresh sendEof after previous write and the bufMEta split.
  sendEof = (hqStream->pendingEOM_ && !hqStream->hasPendingBody());
  if (sendEof || splitBufMeta.length > 0) {
    quic::BufferMeta quicBufMeta(splitBufMeta.length);
    sent += handleWrite(std::move(bufMetaWritter),
                        hqStream,
                        quicBufMeta,
                        quicBufMeta.length,
                        sendEof);
  }

  VLOG(4) << __func__ << " after write sess=" << *this
          << ": streamID=" << streamId << " sent=" << sent
          << " buflen=" << hqStream->writeBufferSize()
          << " hasPendingBody=" << hqStream->txn_.hasPendingBody()
          << " EOM=" << hqStream->pendingEOM_;
  if (infoCallback_) {
    infoCallback_->onWrite(*this, sent);
  }
  CHECK_GE(maxEgress, sent);

  bool flowControlBlocked = (sent == streamSendWindow && !sendEof);
  if (flowControlBlocked) {
    // TODO: this one doesn't create trouble, but it's certainly not logging the
    // extra params anyway.
    if (sock_ && sock_->getState() && sock_->getState()->qLogger) {
      sock_->getState()->qLogger->addStreamStateUpdate(
          streamId,
          quic::kStreamBlocked,
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - hqStream->createdTime));
    }
  }
  // sendAbort can clear the egress queue, so this stream may no longer be
  // enqueued
  if (hqStream->queueHandle_.isStreamTransportEnqueued() &&
      (!hqStream->hasPendingEgress() || flowControlBlocked)) {
    VLOG(4) << "clearPendingEgress for " << hqStream->txn_;
    txnEgressQueue_.clearPendingEgress(hqStream->queueHandle_.getHandle());
  }
  if (flowControlBlocked && !hqStream->txn_.isEgressComplete()) {
    VLOG(4) << __func__ << " txn flow control blocked, txn=" << hqStream->txn_;
    hqStream->txn_.pauseEgress();
  }
  return sent;
}

void HQSession::onDeliveryAck(quic::StreamId id,
                              uint64_t offset,
                              std::chrono::microseconds rtt) {
  VLOG(4) << __func__ << " sess=" << *this << ": streamID=" << id
          << " offset=" << offset;
  auto pEgressStream = findEgressStream(id, true /* includeDetached */);
  DCHECK(pEgressStream);
  if (pEgressStream) {
    pEgressStream->txn_.onEgressLastByteAck(
        std::chrono::duration_cast<std::chrono::milliseconds>(rtt));
    pEgressStream->txn_.decrementPendingByteEvents();
  } else {
    LOG(ERROR) << " not expecting to receive delivery ack for erased stream";
  }
}

void HQSession::onCanceled(quic::StreamId id, uint64_t /*offset*/) {
  VLOG(3) << __func__ << " sess=" << *this << ": streamID=" << id;
  auto pEgressStream = findEgressStream(id);
  if (pEgressStream) {
    pEgressStream->txn_.decrementPendingByteEvents();
  } else {
    LOG(DFATAL) << __func__ << " sess=" << *this << ": streamID=" << id
                << " onCanceled but txn missing, aborted without reset?";
  }
}

void HQSession::HQControlStream::onDeliveryAck(
    quic::StreamId id, uint64_t /*offset*/, std::chrono::microseconds /*rtt*/) {
  // We set the delivery callback for the control stream to keep track of the
  // GOAWAY being delivered to the remote endpoint. When that happens we can
  // send a second GOAWAY. sendGoaway is a no-op after the second time
  VLOG(3) << "GOAWAY received by remote endpoint on streamID=" << id
          << " sess=" << session_;
  session_.onGoawayAck();
}

void HQSession::HQControlStream::onCanceled(quic::StreamId id,
                                            uint64_t /*offset*/) {
  // This shouldn't really happen, but in case it does let's accelerate draining
  VLOG(3) << "GOAWAY delivery callback canceled on streamID=" << id
          << " sess=" << session_;
  session_.drainState_ = DrainState::DONE;
  // if we are shutting down, do so in the loop callback
  session_.scheduleLoopCallback(false);
}

void HQSession::onGoawayAck() {
  if (drainState_ == DrainState::FIRST_GOAWAY) {
    sendGoaway();
  } else if (drainState_ == DrainState::SECOND_GOAWAY) {
    drainState_ = DrainState::DONE;
  }
  // if we are shutting down, do so in the loop callback
  scheduleLoopCallback(false);
}

HQSession::HQStreamTransport* FOLLY_NULLABLE
HQSession::createStreamTransport(quic::StreamId streamId) {
  VLOG(3) << __func__ << " sess=" << *this;

  // Checking for egress and ingress streams as well
  auto streamAlreadyExists = findStream(streamId);
  if (!sock_->good() || streamAlreadyExists) {
    VLOG(3) << __func__ << " Refusing to add a transaction on a closing "
            << " session / existing transaction"
            << " sock good: " << sock_->good()
            << "; streams count: " << streams_.count(streamId) << "; streamId "
            << streamId;
    return nullptr;
  }

  // If this is the first transport, invoke the connection
  // activation callbacks.
  // NOTE: Should this be called when an an ingress push stream
  // is created ?
  if (getNumStreams() == 0) {
    if (infoCallback_) {
      infoCallback_->onActivateConnection(*this);
    }
    if (getConnectionManager()) {
      getConnectionManager()->onActivated(*this);
    }
  }

  // The transport should never call createStreamTransport before
  // onTransportReady
  std::unique_ptr<HTTPCodec> codec = createCodec(streamId);
  auto matchPair = streams_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(streamId),
      std::forward_as_tuple(
          *this,
          direction_,
          streamId,
          getNumTxnServed(),
          std::move(codec),
          WheelTimerInstance(transactionsTimeout_, getEventBase()),
          nullptr, /*   HTTPSessionStats* sessionStats_ */
          hqDefaultPriority,
          folly::none /* assocStreamId */));
  incrementSeqNo();

  CHECK(matchPair.second) << "Emplacement failed, despite earlier "
                             "existence check.";

  // tracks max historical streams
  HTTPSessionBase::onNewOutgoingStream(getNumOutgoingStreams());
  if (infoCallback_) {
    infoCallback_->onTransactionAttached(*this);
  }

  return &matchPair.first->second;
}

std::unique_ptr<HTTPCodec> HQSession::createCodec(quic::StreamId streamId) {
  auto QPACKEncoderStream =
      findControlStream(UnidirectionalStreamType::QPACK_ENCODER);
  DCHECK(QPACKEncoderStream);
  auto QPACKDecoderStream =
      findControlStream(UnidirectionalStreamType::QPACK_DECODER);
  DCHECK(QPACKDecoderStream);
  auto codec = std::make_unique<hq::HQStreamCodec>(
      streamId,
      direction_,
      qpackCodec_,
      QPACKEncoderStream->writeBuf_,
      QPACKDecoderStream->writeBuf_,
      [this, id = QPACKEncoderStream->getEgressStreamId()] {
        if (!sock_) {
          return uint64_t(0);
        }
        auto res = sock_->getStreamFlowControl(id);
        if (res.hasError()) {
          return uint64_t(0);
        }
        return res->sendWindowAvailable;
      },
      ingressSettings_);
  codec->setStrictValidation(strictValidation_);
  return codec;
}

HQSession::HQStreamTransportBase::HQStreamTransportBase(
    HQSession& session,
    TransportDirection direction,
    quic::StreamId streamId,
    uint32_t seqNo,
    const WheelTimerInstance& wheelTimer,
    HTTPSessionStats* stats,
    http2::PriorityUpdate priority,
    folly::Optional<HTTPCodec::StreamID> parentTxnId,
    folly::Optional<hq::UnidirectionalStreamType> type)
    : HQStreamBase(session, session.codec_, type),
      HTTP2PriorityQueueBase(kSessionStreamId),
      txn_(direction,
           static_cast<HTTPCodec::StreamID>(streamId),
           seqNo,
           *this,
           *this,
           wheelTimer.getWheelTimer(),
           wheelTimer.getDefaultTimeout(),
           stats,
           false, // useFlowControl
           0,     // receiveInitialWindowSize
           0,     // sendInitialWindowSize,
           priority,
           parentTxnId,
           HTTPCodec::NoExAttributes, // exAttributes
           session_.setIngressTimeoutAfterEom_),
      byteEventTracker_(nullptr, session.getQuicSocket(), streamId) {
  VLOG(4) << __func__ << " txn=" << txn_;
  byteEventTracker_.setTTLBAStats(session_.sessionStats_);
  quicStreamProtocolInfo_ = std::make_shared<QuicStreamProtocolInfo>();
}

void HQSession::HQStreamTransportBase::initCodec(
    std::unique_ptr<HTTPCodec> codec, const std::string& where) {
  VLOG(3) << where << " " << __func__ << " txn=" << txn_;
  CHECK(session_.sock_)
      << "Socket is null drainState=" << (int)session_.drainState_
      << " streams=" << session_.getNumStreams();
  realCodec_ = std::move(codec);
  if (session_.version_ == HQVersion::HQ) {
    auto c = dynamic_cast<hq::HQStreamCodec*>(realCodec_.get());
    CHECK(c) << "HQ should use HQStream codec";
    c->setActivationHook([this] { return setActiveCodec("self"); });
  }
  auto g = folly::makeGuard(setActiveCodec(__func__));
  if (session_.direction_ == TransportDirection::UPSTREAM || txn_.isPushed()) {
    codecStreamId_ = codecFilterChain->createStream();
  }
  hasCodec_ = true;
}

void HQSession::HQStreamTransportBase::initIngress(const std::string& where) {
  VLOG(3) << where << " " << __func__ << " txn=" << txn_;
  CHECK(session_.sock_)
      << "Socket is null drainState=" << (int)session_.drainState_
      << " streams=" << session_.getNumStreams();

  if (session_.receiveStreamWindowSize_.has_value()) {
    session_.sock_->setStreamFlowControlWindow(
        getIngressStreamId(), session_.receiveStreamWindowSize_.value());
  }

  auto g = folly::makeGuard(setActiveCodec(where));

  codecFilterChain->setCallback(this);
  eomGate_.then([this] { txn_.onIngressEOM(); });
  hasIngress_ = true;
}

HTTPTransaction* FOLLY_NULLABLE
HQSession::newTransaction(HTTPTransaction::Handler* handler) {
  VLOG(4) << __func__ << " sess=" << *this;
  setStreamLimitExceeded(false);

  if (drainState_ == DrainState::CLOSE_SENT ||
      drainState_ == DrainState::FIRST_GOAWAY ||
      drainState_ == DrainState::DONE) {
    VLOG(4) << __func__ << " newTransaction after drain: " << *this;
    return nullptr;
  }
  if (!sock_->good()) {
    VLOG(4) << __func__ << " newTransaction after sock went bad: " << this;
    return nullptr;
  }

  // TODO stream limit handling
  auto quicStreamId = sock_->createBidirectionalStream();
  if (!quicStreamId) {
    VLOG(2) << __func__ << " failed to create new stream: " << this;
    setStreamLimitExceeded(true);
    return nullptr;
  }

  auto hqStream = createStreamTransport(quicStreamId.value());

  if (quicStreamId.value() == 0 && version_ == HQVersion::HQ) {
    // generate grease frame
    auto writeGreaseFrameResult = hq::writeGreaseFrame(hqStream->writeBuf_);
    if (writeGreaseFrameResult.hasError()) {
      VLOG(2) << __func__ << " failed to create grease frame: " << *this
              << ". Error = " << writeGreaseFrameResult.error();
      return nullptr;
    }
  }

  if (hqStream) {
    // DestructorGuard dg(this);
    hqStream->txn_.setHandler(CHECK_NOTNULL(handler));
    sock_->setReadCallback(quicStreamId.value(), this);
    if (ingressLimitExceeded()) {
      sock_->pauseRead(quicStreamId.value());
    }
    return &hqStream->txn_;
  } else {
    VLOG(3) << __func__ << "Failed to create new transaction on "
            << quicStreamId.value();
    abortStream(HTTPException::Direction::INGRESS_AND_EGRESS,
                quicStreamId.value(),
                HTTP3::ErrorCode::HTTP_INTERNAL_ERROR);
    return nullptr;
  }
}

void HQSession::startNow() {
  VLOG(4) << __func__ << " sess=" << *this;
  CHECK(!started_);
  CHECK(sock_);
  started_ = true;
  transportInfo_.secure = true;
  transportInfo_.validTcpinfo = true;
  transportStart_ = getCurrentTime();
  // TODO: invoke socket.start() here
  resetTimeout();
}

void HQSession::HQStreamTransportBase::checkForDetach() {
  if (detached_ && readBuf_.empty() && !hasWriteBuffer() && !pendingEOM_ &&
      !queueHandle_.isStreamTransportEnqueued()) {
    session_.detachStreamTransport(this);
  }
}

bool HQSession::HQStreamTransportBase::getCurrentTransportInfo(
    wangle::TransportInfo* tinfo) {
  VLOG(4) << __func__ << " txn=" << txn_;
  CHECK(quicStreamProtocolInfo_.get());
  bool success = session_.getCurrentTransportInfo(tinfo);

  // Save connection-level protocol fields in the HQStreamTransport-level
  // protocol info.
  if (success) {
    auto connectionTransportInfo =
        dynamic_cast<QuicProtocolInfo*>(tinfo->protocolInfo.get());
    if (connectionTransportInfo) {
      // NOTE: slicing assignment; stream-level fields of
      // quicStreamProtocolInfo_ are not changed while the connection
      // level fields are overwritten.
      *quicStreamProtocolInfo_ = *connectionTransportInfo;
    }
  }

  // Update the HQStreamTransport-level protocol info with the
  // stream info from the the QUIC transport
  if (hasIngressStreamId() || hasEgressStreamId()) {
    session_.getCurrentStreamTransportInfo(quicStreamProtocolInfo_.get(),
                                           getStreamId());
  }

  // Set the transport info query result to the HQStreamTransport protocol
  // info
  tinfo->protocolInfo = quicStreamProtocolInfo_;

  return success;
}

HTTPTransaction::Transport::Type
HQSession::HQStreamTransportBase::getSessionType() const noexcept {
  return session_.getType();
}

void HQSession::detachStreamTransport(HQStreamTransportBase* hqStream) {
  // Special case - streams that dont have either ingress stream id
  // or egress stream id dont need to be actually detached
  // prior to being erased
  if (hqStream->hasIngressStreamId() || hqStream->hasEgressStreamId()) {
    auto streamId = hqStream->getStreamId();
    VLOG(4) << __func__ << " streamID=" << streamId;
    CHECK(findStream(streamId));
    if (sock_ && hqStream->hasIngressStreamId()) {
      clearStreamCallbacks(streamId);
    }
    eraseStream(streamId);
  } else {
    VLOG(4) << __func__ << " streamID=NA";
    eraseUnboundStream(hqStream);
  }

  if (getNumStreams() == 0) {
    if (infoCallback_) {
      infoCallback_->onDeactivateConnection(*this);
    }
    if (getConnectionManager()) {
      getConnectionManager()->onDeactivated(*this);
    }
    resetTimeout();
  }

  if (infoCallback_) {
    infoCallback_->onTransactionDetached(*this);
  }
}

void HQSession::HQControlStream::processReadData() {
  bool isControl = (*type_ == hq::UnidirectionalStreamType::CONTROL);
  std::unique_ptr<HTTPCodec> savedCodec;
  HQUnidirectionalCodec* ingressCodecPtr = ingressCodec_.get();
  if (isControl) {
    // We need ingressCodec_ to be realCodec_, to correctly wire up the filter
    // chain callbacks
    savedCodec = std::move(realCodec_);
    realCodec_.reset(static_cast<HQControlCodec*>(ingressCodec_.release()));
    CHECK(!ingressCodec_);
  }
  auto g1 = folly::makeGuard([&] {
    if (!isControl) {
      return;
    }
    CHECK(!ingressCodec_);
    ingressCodec_.reset(static_cast<HQControlCodec*>(realCodec_.release()));
    realCodec_ = std::move(savedCodec);
  });
  auto g = folly::makeGuard(setActiveCodec(__func__));
  if (isControl) {
    // Now ingressCodec_ has been pushed onto the codec stack.  Restore the
    // egress codec, in case an ingress callback triggers egress
    CHECK(!realCodec_);
    realCodec_ = std::move(savedCodec);
  }
  auto g2 = folly::makeGuard([&] {
    if (!isControl) {
      return;
    }
    savedCodec = std::move(realCodec_);
  });

  CHECK(ingressCodecPtr->isIngress());
  auto initialLength = readBuf_.chainLength();
  if (initialLength > 0) {
    auto ret = ingressCodecPtr->onUnidirectionalIngress(readBuf_.move());
    VLOG(4) << "streamID=" << getIngressStreamId() << " parsed bytes="
            << static_cast<int>(initialLength - readBuf_.chainLength())
            << " from readBuf remain=" << readBuf_.chainLength()
            << " eof=" << readEOF_;
    readBuf_.append(std::move(ret));
  }
  if (readEOF_ && readBuf_.chainLength() == 0) {
    ingressCodecPtr->onUnidirectionalIngressEOF();
  }
}

bool HQSession::HQStreamTransportBase::processReadData() {
  auto g = folly::makeGuard(setActiveCodec(__func__));
  if (eomGate_.get(EOMType::CODEC) && readBuf_.chainLength() > 0) {
    // why are we calling processReadData with no data?
    VLOG(3) << " Received data after HTTP EOM for txn=" << txn_
            << ", len=" << static_cast<int>(readBuf_.chainLength());
    HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                     "Unexpected data after request");
    errorOnTransaction(std::move(ex));
    return false;
  }
  while (!ingressError_ && readBuf_.chainLength() > 0) {
    // Skip any 0 length buffers before invoking the codec. Since readBuf_ is
    // not empty, we are guaranteed to find a non-empty buffer.
    while (readBuf_.front()->length() == 0) {
      readBuf_.pop_front();
    }
    size_t bytesParsed = codecFilterChain->onIngress(*readBuf_.front());
    VLOG(4) << "streamID=" << getStreamId()
            << " parsed bytes=" << static_cast<int>(bytesParsed)
            << " from readBuf remain=" << readBuf_.chainLength()
            << " eof=" << readEOF_;
    if (bytesParsed == 0) {
      break;
    }
    readBuf_.trimStart(bytesParsed);
  }
  if (ingressError_) {
    abortIngress();
  }
  return (readBuf_.chainLength() > 0);
}

// This method can be invoked via several paths:
//  - last header in the response has arrived
//  - triggered by QPACK
//  - push promise has arrived
//  - 1xx information header (e.g. 100 continue)
// The method is safe to use in all the above scenarios
// see specific comments in the method body
void HQSession::HQStreamTransportBase::onHeadersComplete(
    HTTPCodec::StreamID streamID, std::unique_ptr<HTTPMessage> msg) {
  VLOG(4) << __func__ << " txn=" << txn_;
  msg->dumpMessage(3);
  // TODO: the codec will set this
  msg->setAdvancedProtocolString(session_.alpn_);
  msg->setSecure(true);
  CHECK(codecStreamId_);
  CHECK_EQ(streamID, *codecStreamId_);

  if (msg->isRequest() && session_.userAgent_.empty()) {
    session_.userAgent_ = session_.codec_->getUserAgent();
  }

  hasHeaders_ = true;

  //  setupOnHeadersComplete is only implemented
  //  in the HQDownstreamSession, which does not
  //  receive push promises. Will only be called once
  session_.setupOnHeadersComplete(&txn_, msg.get());

  // Inform observers when request headers (i.e. ingress, from downstream
  // client) are processed.
  if (session_.direction_ == TransportDirection::DOWNSTREAM) {
    if (auto msgPtr = msg.get()) {
      const auto event =
          HTTPSessionObserverInterface::RequestStartedEvent::Builder()
              .setTimestamp(HTTPSessionObserverInterface::Clock::now())
              .setHeaders(msgPtr->getHeaders())
              .build();
      session_.sessionObserverContainer_.invokeInterfaceMethod<
          HTTPSessionObserverInterface::Events::requestStarted>(
          [&event](auto observer, auto observed) {
            observer->requestStarted(observed, event);
          });
    }
  }

  if (!txn_.getHandler()) {
    txn_.sendAbort();
    return;
  }

  // TODO: cleanup this comment
  // for h1q-fb-v1 start draining on receipt of a Connection:: close header
  // if we are getting a response, transportReady has been called!
  session_.headersComplete(msg.get());

  // onHeadersComplete can be triggered by data from a different stream ID
  // - specifically, the QPACK encoder stream.  If that's true, then there may
  // be unparsed data in HQStreamTransport.  Add this stream's id to the
  // read set and schedule a loop callback to restart it.
  if (session_.pendingProcessReadSet_.find(getStreamId()) ==
          session_.pendingProcessReadSet_.end() &&
      !readBuf_.empty()) {
    session_.pendingProcessReadSet_.insert(getStreamId());
    session_.scheduleLoopCallback();
  }

  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  auto sock = session_.sock_;
  auto streamId = getStreamId();
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kOnHeaders, timeDiff);
  }

  // In case a priority update was received on the control stream before
  // getting here that overrides the initial priority received in the headers
  if (sock) {
    auto itr = session_.priorityUpdatesBuffer_.find(streamId);
    if (itr != session_.priorityUpdatesBuffer_.end()) {
      sock->setStreamPriority(streamId, toQuicPriority(itr->second));
    } else {
      const auto httpPriority = httpPriorityFromHTTPMessage(*msg);
      if (httpPriority) {
        sock->setStreamPriority(streamId, toQuicPriority(httpPriority.value()));
      }
    }
  }

  // Tell the HTTPTransaction to start processing the message now
  // that the full ingress headers have arrived.
  // Depending on the push promise latch, the message is delivered to
  // the current transaction (no push promise) or to a freshly created
  // pushed transaction. The latter is done via "onPushPromiseHeadersComplete"
  // callback
  if (ingressPushId_) {
    onPushPromiseHeadersComplete(*ingressPushId_, streamID, std::move(msg));
    ingressPushId_ = folly::none;
  } else {
    txn_.onIngressHeadersComplete(std::move(msg));
  }
  if (auto httpSessionActivityTracker =
          session_.getHTTPSessionActivityTracker()) {
    httpSessionActivityTracker->reportActivity();
  }

  // The stream can now receive datagrams: check for any pending datagram and
  // deliver it to the handler
  if (session_.datagramEnabled_ && !session_.datagramsBuffer_.empty()) {
    auto itr = session_.datagramsBuffer_.find(streamId);
    if (itr != session_.datagramsBuffer_.end()) {
      auto& vec = itr->second;
      for (auto& datagram : vec) {
        txn_.onDatagram(std::move(datagram));
      }
      session_.datagramsBuffer_.erase(itr);
    }
  }
}

void HQSession::HQStreamTransportBase::transactionTimeout(
    HTTPTransaction* txn) noexcept {
  auto g = folly::makeGuard(setActiveCodec(__func__));
  VLOG(4) << __func__ << " txn=" << txn_;
  DCHECK(txn == &txn_);

  if (txn->isPushed()) {
    if (!hasIngressStreamId()) {
      // This transaction has not been assigned a stream id yet.
      // Do not attempt to close the stream but do invoke
      // the timeout on the txn
      VLOG(3) << "Transaction timeout on pushedTxn pushId=" << txn->getID();
      txn_.onIngressTimeout();
      return;
    }
  }
  // Verify that the transaction has egress or ingress stream
  DCHECK(hasIngressStreamId() || hasEgressStreamId())
      << "Timeout on transaction without stream id txnID=" << txn->getID()
      << " isPushed=" << txn->isPushed();
  // A transaction has timed out.  If the transaction does not have
  // a Handler yet, because we haven't yet received the full request
  // headers, we give it a DirectResponseHandler that generates an
  // error page.
  VLOG(3) << "Transaction timeout for streamID=" << getStreamId();

  if (!codecStreamId_) {
    // transactionTimeout before onMessageBegin
    codecStreamId_ = codecFilterChain->createStream();
  }

  if (!txn_.getHandler() &&
      txn_.getEgressState() == HTTPTransactionEgressSM::State::Start) {
    VLOG(4) << " Timed out receiving headers. " << this;
    if (session_.infoCallback_) {
      session_.infoCallback_->onIngressError(session_, kErrorTimeout);
    }

    VLOG(4) << " creating direct error handler. " << this;
    auto handler = session_.getTransactionTimeoutHandler(&txn_);
    txn_.setHandler(handler);
  }

  // There may be unparsed ingress.  Discard it.
  abortIngress();

  // Tell the transaction about the timeout.  The transaction will
  // communicate the timeout to the handler, and the handler will
  // decide how to proceed.
  if (hasIngressStreamId()) {
    session_.abortStream(HTTPException::Direction::INGRESS,
                         getIngressStreamId(),
                         HTTP3::ErrorCode::HTTP_INTERNAL_ERROR);
  }

  txn_.onIngressTimeout();
}

void HQSession::abortStream(HTTPException::Direction dir,
                            quic::StreamId id,
                            HTTP3::ErrorCode err) {
  VLOG(4) << __func__ << "sess=" << *this << " id=" << id << " err=" << err;
  CHECK(sock_);
  if (direction_ == TransportDirection::UPSTREAM &&
      err == HTTP3::ErrorCode::HTTP_REQUEST_REJECTED) {
    // Clients MUST NOT use the H3_REQUEST_REJECTED error code, except when a
    // server has requested closure of the request stream with this error code
    //  -- Safest just to never use it.
    err = HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED;
  }
  if (dir != HTTPException::Direction::EGRESS &&
      (sock_->isBidirectionalStream(id) || isPeerUniStream(id))) {
    // Any INGRESS abort generates a QPACK cancel
    abortStream(id);
    // This will do the stopSending for us.
    sock_->setReadCallback(id, nullptr, err);
    sock_->setPeekCallback(id, nullptr);
  }
  if (dir != HTTPException::Direction::INGRESS &&
      (sock_->isBidirectionalStream(id) || isSelfUniStream(id))) {
    sock_->resetStream(id, err);
  }
}

void HQSession::abortStream(quic::StreamId id) {
  if (sock_ && sock_->getState() && sock_->getState()->qLogger) {
    sock_->getState()->qLogger->addStreamStateUpdate(
        id, quic::kAbort, folly::none);
  }
  auto cancel = qpackCodec_.encodeCancelStream(id);
  auto QPACKDecoderStream =
      findControlStream(hq::UnidirectionalStreamType::QPACK_DECODER);
  DCHECK(QPACKDecoderStream);
  QPACKDecoderStream->writeBuf_.append(std::move(cancel));
  scheduleWrite();
}

void HQSession::HQStreamTransportBase::updatePriority(
    const HTTPMessage& headers) noexcept {
  const auto& sock = session_.sock_;
  auto streamId = getStreamId();
  auto httpPriority = httpPriorityFromHTTPMessage(headers);
  if (sock && httpPriority) {
    sock->setStreamPriority(streamId, toQuicPriority(httpPriority.value()));
  }
}

std::pair<uint64_t, uint64_t>
HQSession::HQStreamTransportBase::generateHeadersCommon(
    quic::StreamId streamId,
    const HTTPMessage& headers,
    bool includeEOM,
    HTTPHeaderSize* size) noexcept {
  const uint64_t oldOffset = streamWriteByteOffset();
  CHECK(codecStreamId_)
      << "Trying to send headers on an half open stream isRequest="
      << headers.isRequest()
      << "; assocTxnId=" << txn_.getAssocTxnId().value_or(-1)
      << "; txn=" << txn_.getID();
  codecFilterChain->generateHeader(writeBuf_,
                                   *codecStreamId_,
                                   headers,
                                   includeEOM,
                                   size,
                                   session_.getExtraHeaders(headers, streamId));

  const uint64_t newOffset = streamWriteByteOffset();
  if (size) {
    VLOG(4) << "sending headers, size=" << size->compressed
            << ", compressedBlock=" << size->compressedBlock
            << ", uncompressedSize=" << size->uncompressed << " txn=" << txn_;
  }

  // only do it for downstream now to bypass handling upstream reuse cases
  if (/* session_.direction_ == TransportDirection::DOWNSTREAM && */
      headers.isResponse() && newOffset > oldOffset &&
      // catch 100-ish response?
      !txn_.testAndSetFirstHeaderByteSent()) {
    byteEventTracker_.addFirstHeaderByteEvent(newOffset, &txn_);
  }

  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);

  auto sock = session_.sock_;
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kHeaders, timeDiff);
  }

  if ((newOffset > 0) &&
      (headers.isRequest() ||
       (headers.isResponse() && headers.getStatusCode() >= 200))) {
    // Track last egress header and notify the handler when the receiver acks
    // the headers.
    // We need to track last byte sent offset, so substract one here.
    armEgressHeadersAckCb(newOffset - 1);
  }

  return std::make_pair(oldOffset, newOffset);
}

bool HQSession::HQStreamTransportBase::sendHeadersWithDelegate(
    HTTPTransaction* txn,
    const HTTPMessage& headers,
    HTTPHeaderSize* size,
    size_t* dataFrameHeaderSize,
    uint64_t contentLength,
    std::unique_ptr<DSRRequestSender> dsrSender) noexcept {
  VLOG(4) << __func__ << " txn=" << *txn;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  CHECK_EQ(txn, &txn_);
  CHECK(!headers.isRequest())
      << "Delegate sending can only happen with response";
  CHECK(!txn->getAssocTxnId()) << "Delegate sending isn't supported with push";
  if (!contentLength) {
    return false;
  }

  updatePriority(headers);
  auto g = folly::makeGuard(setActiveCodec(__func__));
  auto streamId = getStreamId();
  auto sock = session_.sock_;
  if (!sock) {
    LOG(ERROR) << __func__
               << ": HQSession received delegate request without a QuicSocket";
    return false;
  }
  auto dsrRequestSenderRawPtr = CHECK_NOTNULL(dsrSender.get());
  auto quicDSRSenderRawPtr = dynamic_cast<quic::DSRPacketizationRequestSender*>(
      dsrRequestSenderRawPtr);
  if (!quicDSRSenderRawPtr) {
    LOG(ERROR) << __func__ << ": The passed in DSRSender is of wrong type";
    return false;
  }
  dsrSender.release();
  auto setSenderRet = sock->setDSRPacketizationRequestSender(
      streamId,
      std::unique_ptr<quic::DSRPacketizationRequestSender>(
          quicDSRSenderRawPtr));
  if (setSenderRet.hasError()) {
    LOG(ERROR) << __func__ << ": failed to set DSR sender, error="
               << toString(setSenderRet.error());
    return false;
  }
  generateHeadersCommon(streamId, headers, false /* includeEOM */, size);
  // Write a DATA frame header with CL value
  auto writeFrameHeaderResult =
      hq::writeFrameHeader(writeBuf_, FrameType::DATA, contentLength);
  if (writeFrameHeaderResult.hasError()) {
    return false;
  }
  *dataFrameHeaderSize = *writeFrameHeaderResult;
  notifyPendingEgress();
  dsrRequestSenderRawPtr->onHeaderBytesGenerated(streamWriteByteOffset());
  return true;
}

void HQSession::HQStreamTransportBase::sendHeaders(HTTPTransaction* txn,
                                                   const HTTPMessage& headers,
                                                   HTTPHeaderSize* size,
                                                   bool includeEOM) noexcept {
  VLOG(4) << __func__ << " txn=" << txn_;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  DCHECK(txn == &txn_);

  updatePriority(headers);
  // If this is a push promise, send it on the parent stream.
  // The accounting will happen in the nested context
  if (headers.isRequest() && txn->getAssocTxnId()) {
    sendPushPromise(txn, folly::none, headers, size, includeEOM);
    return;
  }
  auto g = folly::makeGuard(setActiveCodec(__func__));
  auto streamId = getStreamId();
  auto headerGenOffsets =
      generateHeadersCommon(streamId, headers, includeEOM, size);
  auto oldOffset = headerGenOffsets.first;
  auto newOffset = headerGenOffsets.second;

  if (includeEOM) {
    CHECK_GE(newOffset, oldOffset);
    session_.handleLastByteEvents(&byteEventTracker_,
                                  &txn_,
                                  newOffset - oldOffset,
                                  streamWriteByteOffset(),
                                  true);
  }

  pendingEOM_ = includeEOM;
  // Headers can be empty for a 0.9 response
  if (writeBuf_.chainLength() > 0 || pendingEOM_) {
    notifyPendingEgress();
  }

  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  auto sock = session_.sock_;
  if (includeEOM) {
    if (sock && sock->getState() && sock->getState()->qLogger) {
      sock->getState()->qLogger->addStreamStateUpdate(
          streamId, quic::kEOM, timeDiff);
    }
  }

  // If this is a client sending request headers to upstream
  // invoke requestStarted event for attached observers.
  if (session_.direction_ == TransportDirection::UPSTREAM) {
    const auto event =
        HTTPSessionObserverInterface::RequestStartedEvent::Builder()
            .setTimestamp(HTTPSessionObserverInterface::Clock::now())
            .setHeaders(headers.getHeaders())
            .build();
    session_.sessionObserverContainer_.invokeInterfaceMethod<
        HTTPSessionObserverInterface::Events::requestStarted>(
        [&event](auto observer, auto observed) {
          observer->requestStarted(observed, event);
        });
  }
}

size_t HQSession::HQStreamTransportBase::sendEOM(
    HTTPTransaction* txn, const HTTPHeaders* trailers) noexcept {
  VLOG(4) << __func__ << " txn=" << txn_;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  DCHECK(txn == &txn_);
  auto g = folly::makeGuard(setActiveCodec(__func__));

  size_t encodedSize = 0;

  CHECK(codecStreamId_);
  if (trailers) {
    encodedSize = codecFilterChain->generateTrailers(
        writeBuf_, *codecStreamId_, *trailers);
  }

  encodedSize += codecFilterChain->generateEOM(writeBuf_, *codecStreamId_);

  // This will suppress the call to onEgressBodyLastByte in
  // handleLastByteEvents, since we're going to add a last byte event anyways.
  // This safely keeps the txn open until we egress the FIN to the transport.
  // At that point, the deliveryCallback should also be registered.
  // Note: even if the byteEventTracker_ is already at streamWriteByteOffset(),
  // it is still invoked with the same offset after egressing the FIN.
  bool pretendPiggybacked = (encodedSize == 0);
  session_.handleLastByteEvents(&byteEventTracker_,
                                &txn_,
                                encodedSize,
                                streamWriteByteOffset(),
                                pretendPiggybacked);
  if (pretendPiggybacked) {
    byteEventTracker_.addLastByteEvent(txn, streamWriteByteOffset());
  }
  // For H1 without chunked transfer-encoding, generateEOM is a no-op
  // We need to make sure writeChain(eom=true) gets called
  pendingEOM_ = true;
  notifyPendingEgress();
  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  auto sock = session_.sock_;
  auto streamId = getStreamId();
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kEOM, timeDiff);
  }
  return encodedSize;
}

size_t HQSession::HQStreamTransportBase::sendAbort(
    HTTPTransaction* txn, ErrorCode errorCode) noexcept {
  return sendAbortImpl(toHTTP3ErrorCode(errorCode),
                       folly::to<std::string>("Application aborts, errorCode=",
                                              getErrorCodeString(errorCode),
                                              " txnID=",
                                              txn->getID(),
                                              " isPushed=",
                                              txn->isPushed()));
}

size_t HQSession::HQStreamTransportBase::sendAbortImpl(HTTP3::ErrorCode code,
                                                       std::string errorMsg) {
  VLOG(4) << __func__ << " txn=" << txn_ << " msg=" << errorMsg;

  // If the HQ stream is bound to a transport stream, abort it.
  if (hasStreamId()) {
    session_.abortStream(getStreamDirection(), getStreamId(), code);
  }
  // Like abortIngress, but not safe to clear readBuf_, because we may be
  // parsing it.  If we are, then abortIngress will be called at the end of
  // processReadData.  If not, then the STOP_SENDING we emit will trigger a peer
  // RST_STREAM (eventually), which will clear the readBuf_.
  ingressError_ = true;
  codecFilterChain->setParserPaused(true);

  if (hasEgressStreamId()) {
    abortEgress(true);
  }
  // NOTE: What about the streams that only `hasIngressStreamId()` ?
  // At the time being, the only case of ingress-only transport stream
  // is an ingress push stream. The essential procedure for aborting
  // the ingress push streams is the same as above - abort the stream via
  // sending the "stop sending" frame on the control stream.
  //
  // Additional logic that is specific to the ingress push stream, such as
  // sending `CANCEL_PUSH` message, does not belong to `HQSession` level, but
  // to `HQUpstreamSesssion::HQIngressPushStream::sendAbort`, which
  // invokes this method.

  // We generated 0 application bytes so return 0?
  return 0;
}

void HQSession::HQStreamTransportBase::abortIngress() {
  VLOG(4) << "Aborting ingress for " << txn_;
  ingressError_ = true;
  readBuf_.move();
  codecFilterChain->setParserPaused(true);
}

void HQSession::HQStreamTransportBase::abortEgress(bool checkForDetach) {
  VLOG(4) << "Aborting egress for " << txn_;
  byteEventTracker_.drainByteEvents();
  writeBuf_.move();
  bufMeta_.length = 0;
  pendingEOM_ = false;
  if (queueHandle_.isStreamTransportEnqueued()) {
    VLOG(4) << "clearPendingEgress for " << txn_;
    session_.txnEgressQueue_.clearPendingEgress(queueHandle_.getHandle());
  }
  if (checkForDetach) {
    HTTPTransaction::DestructorGuard dg(&txn_);
  }
}

void HQSession::HQControlStream::onError(HTTPCodec::StreamID streamID,
                                         const HTTPException& error,
                                         bool /* newTxn */) {
  // All the errors on the control stream are to be considered session errors
  // anyway, so just use the ingress stream id
  if (streamID == kSessionStreamId) {
    streamID = getIngressStreamId();
  }
  if (session_.infoCallback_) {
    session_.infoCallback_->onIngressError(
        session_,
        isQPACKError(error.getHttp3ErrorCode()) ? kErrorBadDecompress
                                                : kErrorMessage);
  }
  LOG(ERROR) << "Got error on control stream error="
             << toString(error.getHttp3ErrorCode()) << " streamID=" << streamID
             << " sess=" << session_;
  session_.handleSessionError(
      CHECK_NOTNULL(session_.findControlStream(streamID)),
      StreamDirection::INGRESS,
      error.getHttp3ErrorCode(),
      kErrorConnection);
}

void HQSession::HQStreamTransportBase::onError(HTTPCodec::StreamID streamID,
                                               const HTTPException& error,
                                               bool /* newTxn */) {
  VLOG(4) << __func__ << " (from Codec) txn=" << txn_ << " err=" << error;
  // Codec must either call onMessageComplete or onError, but not both
  // I think.  The exception might be if stream has more than one HTTP
  // message on it.
  CHECK(!eomGate_.get(EOMType::CODEC));
  ingressError_ = true;

  if (streamID == kSessionStreamId) {
    if (session_.infoCallback_) {
      session_.infoCallback_->onIngressError(
          session_,
          isQPACKError(error.getHttp3ErrorCode()) ? kErrorBadDecompress
                                                  : kErrorMessage);
    }
    LOG(ERROR) << "Got session error error="
               << toString(error.getHttp3ErrorCode()) << " msg=" << error
               << " streamID=" << getIngressStreamId() << " sess=" << session_;
    session_.handleSessionError(this,
                                StreamDirection::INGRESS,
                                error.getHttp3ErrorCode(),
                                kErrorConnection);
    return;
  }

  if (!codecStreamId_ && error.hasHttpStatusCode() && streamID != 0) {
    // onError before onMessageBegin
    codecStreamId_ = streamID;
  }

  if (!txn_.getHandler() &&
      txn_.getEgressState() == HTTPTransactionEgressSM::State::Start) {
    if (error.getDirection() != HTTPException::Direction::INGRESS) {
      // Direct error handler only process INGRESS
      LOG(DFATAL) << "Codec gave egress error with no handler sess="
                  << session_;
    }
    session_.abortStream(HTTPException::Direction::INGRESS,
                         getIngressStreamId(),
                         error.getHttp3ErrorCode());
    session_.handleErrorDirectly(&txn_, error);
    return;
  }

  txn_.onError(error);
  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  auto sock = session_.sock_;
  auto streamId = getStreamId();
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kOnError, timeDiff);
  }
}

void HQSession::HQStreamTransportBase::onResetStream(HTTP3::ErrorCode errorCode,
                                                     HTTPException ex) {
  // kErrorStreamAbort prevents HTTPTransaction from calling sendAbort in reply.
  // We use this code and manually call sendAbort here for appropriate cases
  HTTP3::ErrorCode replyError = HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED;
  if (session_.direction_ == TransportDirection::DOWNSTREAM &&
      !txn_.isIngressStarted()) {
    // Downstream ingress closed with no ingress yet, we can send REJECTED
    // It's actually ok if we've received headers but not made any
    // calls to the handler, but there's no API for that.
    replyError = HTTP3::ErrorCode::HTTP_REQUEST_REJECTED;
  }

  if (errorCode == HTTP3::ErrorCode::HTTP_REQUEST_REJECTED) {
    VLOG_IF(2, session_.direction_ == TransportDirection::DOWNSTREAM)
        << "RST_STREAM/REJECTED should not be sent by clients txn=" << txn_;
    // kErrorStreamUnacknowledged signals that this is safe to retry
    ex.setProxygenError(kErrorStreamUnacknowledged);
  } else {
    ex.setProxygenError(kErrorStreamAbort);
  }
  if (errorCode == HTTP3::ErrorCode::GIVEUP_ZERO_RTT) {
    // This error code comes from application who wants to error out all
    // transactions over hqsession because QUIC lost race with TCP. Passing this
    // error back to transactions through onError so that they can be retried.
    ex.setProxygenError(kErrorEarlyDataFailed);
  }
  ex.setHttp3ErrorCode(errorCode);
  auto msg = ex.what();
  errorOnTransaction(std::move(ex));
  sendAbortImpl(replyError, msg);
}

void HQSession::HQStreamTransportBase::notifyPendingEgress() noexcept {
  VLOG(4) << __func__ << " txn=" << txn_;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  signalPendingEgressImpl();
  session_.scheduleWrite();
}

size_t HQSession::HQStreamTransportBase::sendBody(
    HTTPTransaction* txn,
    const HTTPTransaction::BufferMeta& body,
    bool eom) noexcept {
  VLOG(4) << __func__ << " len=" << body.length << " eof=" << eom
          << " txn=" << *txn;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  CHECK_EQ(txn, &txn_);

  auto g = folly::makeGuard(setActiveCodec(__func__));
  CHECK(codecStreamId_);

  codecFilterChain->generateBodyDSR(
      *codecStreamId_, body.length, HTTPCodec::NoPadding, eom);

  uint64_t offset = streamWriteByteOffset();
  bufMeta_.length += body.length;
  bodyBytesEgressed_ += body.length;

  if (auto httpSessionActivityTracker =
          session_.getHTTPSessionActivityTracker()) {
    httpSessionActivityTracker->addTrackedEgressByteEvent(
        offset, body.length, &byteEventTracker_, txn);
  }

  if (body.length && !txn->testAndSetFirstByteSent()) {
    byteEventTracker_.addFirstBodyByteEvent(offset, txn);
  }

  auto sock = session_.sock_;
  auto streamId = getStreamId();
  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kBody, timeDiff);
  }

  if (eom) {
    coalesceEOM(body.length);
  }
  notifyPendingEgress();
  return body.length;
}

void HQSession::HQStreamTransportBase::coalesceEOM(size_t encodedBodyBytes) {
  session_.handleLastByteEvents(&byteEventTracker_,
                                &txn_,
                                encodedBodyBytes,
                                streamWriteByteOffset(),
                                true);
  VLOG(3) << "sending EOM in body for streamID=" << getStreamId()
          << " txn=" << txn_;
  pendingEOM_ = true;
  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  auto sock = session_.sock_;
  auto streamId = getStreamId();
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kEOM, timeDiff);
  }
}

size_t HQSession::HQStreamTransportBase::sendBody(
    HTTPTransaction* txn,
    std::unique_ptr<folly::IOBuf> body,
    bool includeEOM,
    bool /* trackLastByteFlushed */) noexcept {
  auto bodyLength = body->computeChainDataLength();
  VLOG(4) << __func__ << " len=" << bodyLength << " eof=" << includeEOM
          << " txn=" << txn_;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  DCHECK(txn == &txn_);
  CHECK_EQ(0, bufMeta_.length);
  uint64_t offset = streamWriteByteOffset();

  auto g = folly::makeGuard(setActiveCodec(__func__));
  CHECK(codecStreamId_);
  size_t encodedSize = codecFilterChain->generateBody(writeBuf_,
                                                      *codecStreamId_,
                                                      std::move(body),
                                                      HTTPCodec::NoPadding,
                                                      includeEOM);
  bodyBytesEgressed_ += bodyLength;
  if (auto httpSessionActivityTracker =
          session_.getHTTPSessionActivityTracker()) {
    httpSessionActivityTracker->addTrackedEgressByteEvent(
        offset, encodedSize, &byteEventTracker_, txn);
  }
  if (encodedSize > 0 && !txn->testAndSetFirstByteSent()) {
    byteEventTracker_.addFirstBodyByteEvent(offset, txn);
  }
  auto sock = session_.sock_;
  auto streamId = getStreamId();
  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kBody, timeDiff);
  }
  if (includeEOM) {
    coalesceEOM(encodedSize);
  }
  notifyPendingEgress();
  return encodedSize;
}

size_t HQSession::HQStreamTransportBase::sendChunkHeader(
    HTTPTransaction* txn, size_t length) noexcept {
  VLOG(4) << __func__ << " txn=" << txn_;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  DCHECK(txn == &txn_);
  auto g = folly::makeGuard(setActiveCodec(__func__));
  CHECK(codecStreamId_);
  size_t encodedSize =
      codecFilterChain->generateChunkHeader(writeBuf_, *codecStreamId_, length);
  notifyPendingEgress();
  return encodedSize;
}

size_t HQSession::HQStreamTransportBase::sendChunkTerminator(
    HTTPTransaction* txn) noexcept {
  VLOG(4) << __func__ << " txn=" << txn_;
  CHECK(hasEgressStreamId()) << __func__ << " invoked on stream without egress";
  DCHECK(txn == &txn_);
  auto g = folly::makeGuard(setActiveCodec(__func__));
  CHECK(codecStreamId_);
  size_t encodedSize =
      codecFilterChain->generateChunkTerminator(writeBuf_, *codecStreamId_);
  notifyPendingEgress();
  return encodedSize;
}

void HQSession::HQStreamTransportBase::onMessageBegin(
    HTTPCodec::StreamID streamID, HTTPMessage* /* msg */) {
  VLOG(4) << __func__ << " txn=" << txn_ << " streamID=" << streamID
          << " ingressPushId=" << ingressPushId_.value_or(-1);

  if (ingressPushId_) {
    constexpr auto error =
        "Received onMessageBegin in the middle of push promise";
    LOG(ERROR) << error << " streamID=" << streamID << " session=" << session_;
    // TODO: Audit this error code
    session_.dropConnectionAsync(
        quic::QuicError(HTTP3::ErrorCode::HTTP_FRAME_ERROR, error),
        kErrorDropped);
    return;
  }

  if (session_.infoCallback_) {
    session_.infoCallback_->onRequestBegin(session_);
  }

  // NOTE: for H2 this is where we create a new stream and transaction.
  // for HQ there is nothing to do here, except caching the codec streamID
  codecStreamId_ = streamID;

  // Reset the pending pushID, since the subsequent invocation of
  // `onHeadersComplete` won't be associated with a push
  ingressPushId_ = folly::none;
}

void HQSession::HQStreamTransportBase::trackEgressBodyOffset(
    uint64_t bodyOffset, proxygen::ByteEvent::EventFlags eventFlags) {
  auto g = folly::makeGuard(setActiveCodec(__func__));
  // This calculation is only accurate for a body offset in the most recently
  // generated DATA frame.  Any earlier offsets will skew large by factoring in
  // more recent frame headers or non-DATA frames.  Any later offsets will skew
  // small because the number of non-body bytes is not known.
  uint64_t streamOffset =
      (streamWriteByteOffset() - bodyBytesEgressed_) + bodyOffset;
  // We need to track last byte sent offset, so substract one here.
  auto offset = streamOffset - 1;
  armEgressBodyCallbacks(bodyOffset, offset, eventFlags);
  VLOG(4) << __func__ << ": armed body byte event cb for offset=" << offset
          << "; body offset=" << bodyOffset
          << "; flags=" << uint32_t(eventFlags) << "; txn=" << txn_;
}

void HQSession::HQStreamTransportBase::armStreamByteEventCb(
    uint64_t streamOffset, quic::QuicSocket::ByteEvent::Type type) {
  auto res = session_.sock_->registerByteEventCallback(
      type, getEgressStreamId(), streamOffset, this);
  if (res.hasError()) {
    auto errStr = folly::to<std::string>(
        "failed to register byte event callback: ", toString(res.error()));
    LOG(ERROR) << errStr;
    HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS, errStr);
    ex.setProxygenError(kErrorNetwork);
    errorOnTransaction(std::move(ex));
    return;
  }
  numActiveDeliveryCallbacks_++;

  // Increment pending byte events so the transaction won't detach until we get
  // and ack/cancel from transport here.
  txn_.incrementPendingByteEvents();

  VLOG(4) << __func__ << ": registered type=" << uint32_t(type)
          << " callback for offset=" << streamOffset << "; sess=" << session_
          << "; txn=" << txn_;
}

void HQSession::HQStreamTransportBase::armEgressHeadersAckCb(
    uint64_t streamOffset) {
  VLOG(4) << __func__ << ": registering headers delivery callback for offset="
          << streamOffset << "; sess=" << session_ << "; txn=" << txn_;
  armStreamByteEventCb(streamOffset, quic::QuicSocket::ByteEvent::Type::ACK);
  egressHeadersAckOffset_ = streamOffset;
}

void HQSession::HQStreamTransportBase::armEgressBodyCallbacks(
    uint64_t bodyOffset,
    uint64_t streamOffset,
    proxygen::ByteEvent::EventFlags eventFlags) {
  VLOG(4) << __func__ << ": registering body byte event callback for offset="
          << streamOffset << "; flags=" << uint32_t(eventFlags)
          << "; sess=" << session_ << "; txn=" << txn_;
  if (eventFlags & proxygen::ByteEvent::EventFlags::TX) {
    armStreamByteEventCb(streamOffset, quic::QuicSocket::ByteEvent::Type::TX);
    auto res = egressBodyByteEventOffsets_.try_emplace(
        streamOffset, BodyByteOffset(bodyOffset, 1));
    if (!res.second) {
      res.first->second.callbacks++;
    }
  }
  if (eventFlags & proxygen::ByteEvent::EventFlags::ACK) {
    armStreamByteEventCb(streamOffset, quic::QuicSocket::ByteEvent::Type::ACK);
    auto res = egressBodyByteEventOffsets_.try_emplace(
        streamOffset, BodyByteOffset(bodyOffset, 1));
    if (!res.second) {
      res.first->second.callbacks++;
    }
  }
}

void HQSession::HQStreamTransportBase::handleHeadersAcked(
    uint64_t streamOffset) {
  CHECK(egressHeadersAckOffset_);
  if (*egressHeadersAckOffset_ != streamOffset) {
    LOG(ERROR)
        << ": bad offset for egress headers ack: e=" << *egressHeadersAckOffset_
        << ", r=" << streamOffset << "; sess=" << session_ << "; txn=" << txn_;
    return;
  }

  VLOG(4) << __func__ << ": got delivery ack for egress headers, stream offset="
          << streamOffset << "; sess=" << session_ << "; txn=" << txn_;

  resetEgressHeadersAckOffset();
  txn_.onLastEgressHeaderByteAcked();
}

void HQSession::HQStreamTransportBase::handleBodyEvent(
    uint64_t streamOffset, quic::QuicSocket::ByteEvent::Type type) {
  auto g = folly::makeGuard(setActiveCodec(__func__));

  auto bodyOffset = resetEgressBodyEventOffset(streamOffset);
  if (!bodyOffset) {
    LOG(DFATAL) << __func__ << ": received an unexpected byte event at offset "
                << streamOffset << "; sess=" << session_ << "; txn=" << txn_;
    return;
  }
  VLOG(4) << __func__ << ": got byte event type=" << uint32_t(type)
          << " for egress body, bodyOffset=" << *bodyOffset
          << "; sess=" << session_ << "; txn=" << txn_;

  if (type == quic::QuicSocket::ByteEvent::Type::ACK) {
    txn_.onEgressBodyBytesAcked(*bodyOffset);
  } else if (type == quic::QuicSocket::ByteEvent::Type::TX) {
    txn_.onEgressBodyBytesTx(*bodyOffset);
  }
}

void HQSession::HQStreamTransportBase::handleBodyEventCancelled(
    uint64_t streamOffset, quic::QuicSocket::ByteEvent::Type) {
  auto g = folly::makeGuard(setActiveCodec(__func__));

  auto bodyOffset = resetEgressBodyEventOffset(streamOffset);
  if (!bodyOffset) {
    LOG(DFATAL) << __func__
                << ": received an unexpected onCanceled event at offset "
                << streamOffset;
    return;
  }
  // Use the same callback whether the body did not TX or did not ACK.  Caller
  // may received this more than once if they asked to track both.
  txn_.onEgressBodyDeliveryCanceled(*bodyOffset);
}

void HQSession::HQStreamTransportBase::onByteEvent(
    quic::QuicSocket::ByteEvent byteEvent) {
  VLOG(4) << __func__ << ": got byte event type=" << uint32_t(byteEvent.type)
          << " for offset=" << byteEvent.offset << "; sess=" << session_
          << "; txn=" << txn_;

  DCHECK_GT(numActiveDeliveryCallbacks_, 0);
  numActiveDeliveryCallbacks_--;
  txn_.decrementPendingByteEvents();

  // For a given type (ACK|TX), onByteEvent calls will be called from QuicSocket
  // with monotonically increasing offsets.
  if (egressHeadersAckOffset_) {
    if (byteEvent.type == quic::QuicSocket::ByteEvent::Type::ACK) {
      handleHeadersAcked(byteEvent.offset);
      return;
    }
    // else we don't track header byte tx (yet), but it could be a body TX
  }

  handleBodyEvent(byteEvent.offset, byteEvent.type);
}

void HQSession::HQStreamTransportBase::onByteEventCanceled(
    quic::QuicSocket::ByteEventCancellation cancellation) {
  VLOG(3) << __func__ << ": data cancelled on stream=" << cancellation.id
          << ", type=" << uint32_t(cancellation.type)
          << ", offset=" << cancellation.offset << "; sess=" << session_
          << "; txn=" << txn_;
  DCHECK_GT(numActiveDeliveryCallbacks_, 0);
  numActiveDeliveryCallbacks_--;
  txn_.decrementPendingByteEvents();

  // Are byte events of a given type always cancelled in offset order?

  if (egressHeadersAckOffset_) {
    if (cancellation.type == quic::QuicSocket::ByteEvent::Type::ACK) {
      resetEgressHeadersAckOffset();
      return;
    }
    // else we don't track header byte tx (yet), but it could be a body TX
  }

  handleBodyEventCancelled(cancellation.offset, cancellation.type);
}

// Methods specific to StreamTransport subclasses
void HQSession::HQStreamTransportBase::onPushMessageBegin(
    HTTPCodec::StreamID pushID,
    HTTPCodec::StreamID assocStreamID,
    HTTPMessage* /* msg */) {
  VLOG(4) << __func__ << " txn=" << txn_ << " streamID=" << getIngressStreamId()
          << " assocStreamID=" << assocStreamID
          << " ingressPushId=" << ingressPushId_.value_or(-1);

  if (ingressPushId_) {
    constexpr auto error =
        "Received onPushMessageBegin in the middle of push promise";
    LOG(ERROR) << error;
    // TODO: Audit this error code
    session_.dropConnectionAsync(
        quic::QuicError(HTTP3::ErrorCode::HTTP_FRAME_ERROR, error),
        kErrorDropped);
    return;
  }

  if (session_.infoCallback_) {
    session_.infoCallback_->onRequestBegin(session_);
  }

  // Notify the testing callbacks
  if (session_.serverPushLifecycleCb_) {
    session_.serverPushLifecycleCb_->onPushPromiseBegin(
        assocStreamID, static_cast<hq::PushId>(pushID));
  }

  ingressPushId_ = static_cast<hq::PushId>(pushID);
}

HQSession::HQStreamTransportBase* HQSession::findWTSessionOrAbort(
    quic::StreamId sessionID, quic::StreamId streamID) {
  CHECK(supportsWebTransport());
  auto wtSession = findNonDetachedStream(sessionID);
  if (!wtSession || !wtSession->txn_.isWebTransportConnectStream()) {
    LOG(ERROR) << "Missing or invalid webtransport connect stream id="
               << sessionID << " for peer initiated stream id=" << streamID;
    // need to error stopSending/reset this stream
    abortStream(HTTPException::Direction::INGRESS_AND_EGRESS,
                streamID,
                HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR);
    return nullptr;
  }
  return wtSession;
}

// Peer initiated Uni WT streams
void HQSession::dispatchUniWTStream(quic::StreamId streamID,
                                    quic::StreamId sessionID,
                                    size_t toConsume) {
  sock_->setPeekCallback(streamID, nullptr);
  auto consumeRes = sock_->consume(streamID, toConsume);
  CHECK(!consumeRes.hasError()) << "Unexpected error consuming bytes";
  VLOG(6) << __func__ << " sess=" << *this << " id=" << streamID
          << " wt-sess-id=" << sessionID;

  // Notify the read callback
  if (infoCallback_) {
    infoCallback_->onRead(
        *this, toConsume, static_cast<HTTPCodec::StreamID>(streamID));
  }

  auto parent = findWTSessionOrAbort(sessionID, streamID);
  if (!parent) {
    return;
  }
  auto streamTransport = static_cast<HQStreamTransport*>(parent);
  sock_->setReadCallback(streamID, streamTransport->getWTReadCallback());
  parent->txn_.onWebTransportUniStream(streamID);
}

// Peer initiated Bidi WT streams
void HQSession::dispatchBidiWTStream(HTTPCodec::StreamID streamID,
                                     HTTPCodec::StreamID sessionID,
                                     size_t toConsume) {
  sock_->setPeekCallback(streamID, nullptr);
  auto consumeRes = sock_->consume(streamID, toConsume);
  CHECK(!consumeRes.hasError()) << "Unexpected error consuming bytes";
  VLOG(6) << __func__ << " sess=" << *this << " id=" << streamID
          << " wt-sess-id=" << sessionID;

  auto parent = findWTSessionOrAbort(sessionID, streamID);
  if (!parent) {
    return;
  }

  auto streamTransport = static_cast<HQStreamTransport*>(parent);
  sock_->setReadCallback(streamID, streamTransport->getWTReadCallback());
  parent->txn_.onWebTransportBidiStream(streamID);
}

// Methods specific to StreamTransport subclasses
//
//

// Request-stream implementation of the "sendPushPromise"
// HQEgressPushStream::sendPushPromise calls this
void HQSession::HQStreamTransport::sendPushPromise(
    HTTPTransaction* txn,
    folly::Optional<hq::PushId> pushId,
    const HTTPMessage& headers,
    HTTPHeaderSize* size,
    bool includeEOM) {
  CHECK(txn);

  CHECK(pushId.has_value()) << " Request stream impl expects pushID to be set";
  const uint64_t oldOffset = streamWriteByteOffset();
  auto g = folly::makeGuard(setActiveCodec(__func__));

  codecFilterChain->generatePushPromise(
      writeBuf_, *codecStreamId_, headers, pushId.value(), includeEOM, size);

  const uint64_t newOffset = streamWriteByteOffset();
  if (size) {
    VLOG(4) << "sending push promise, size=" << size->compressed
            << ", uncompressedSize=" << size->uncompressed << " txn=" << txn_;
  }

  if (includeEOM) {
    CHECK_GE(newOffset, oldOffset);
    session_.handleLastByteEvents(&byteEventTracker_,
                                  &txn_,
                                  newOffset - oldOffset,
                                  streamWriteByteOffset(),
                                  true);
  }

  pendingEOM_ = includeEOM;
  notifyPendingEgress();

  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - createdTime);
  auto sock = session_.sock_;
  auto streamId = getStreamId();
  if (sock && sock->getState() && sock->getState()->qLogger) {
    sock->getState()->qLogger->addStreamStateUpdate(
        streamId, quic::kPushPromise, timeDiff);
  }
  if (includeEOM) {
    if (sock && sock->getState() && sock->getState()->qLogger) {
      sock->getState()->qLogger->addStreamStateUpdate(
          streamId, quic::kEOM, timeDiff);
    }
  }
}

HTTPTransaction* FOLLY_NULLABLE
HQSession::HQStreamTransport::newPushedTransaction(
    HTTPCodec::StreamID parentRequestStreamId,
    HTTPTransaction::PushHandler* handler,
    ProxygenError* error) noexcept {

  CHECK_EQ(parentRequestStreamId, txn_.getID());

  return session_.newPushedTransaction(
      parentRequestStreamId, // stream id of the egress push stream
      handler,
      error);
}

void HQSession::HQStreamTransport::onPushPromiseHeadersComplete(
    hq::PushId pushID,
    HTTPCodec::StreamID assocStreamID,
    std::unique_ptr<HTTPMessage> msg) {
  VLOG(4) << "processing new Push Promise msg=" << msg.get()
          << " streamID=" << assocStreamID << " maybePushID=" << pushID
          << ", txn= " << txn_;

  // Notify the testing callbacks
  if (session_.serverPushLifecycleCb_) {
    session_.serverPushLifecycleCb_->onPushPromise(
        assocStreamID, pushID, msg.get());
  }

  // Create ingress push stream (will also create the transaction)
  // If a corresponding nascent push stream is ready, it will be
  // bound to the newly created stream.
  // virtual function call into UpstreamSession.  This will crash if it happens
  // downstream.
  auto pushStream = session_.createIngressPushStream(assocStreamID, pushID);
  CHECK(pushStream);

  // Notify the *parent* transaction that the *pushed* transaction has been
  // successfully created.
  txn_.onPushedTransaction(&pushStream->txn_);

  // Notify the *pushed* transaction on the push promise headers
  // This has to be called AFTER "onPushedTransaction" upcall
  pushStream->txn_.onIngressHeadersComplete(std::move(msg));
}

void HQSession::onDatagramsAvailable() noexcept {
  auto result = sock_->readDatagramBufs();
  if (result.hasError()) {
    LOG(ERROR) << "Got error while reading datagrams: error="
               << toString(result.error());
    dropConnectionAsync(quic::QuicError(HTTP3::ErrorCode::HTTP_INTERNAL_ERROR,
                                        "H3_DATAGRAM: internal error "),
                        kErrorConnection);
    return;
  }
  VLOG(4) << "Received " << result.value().size()
          << " datagrams. sess=" << *this;
  for (auto& datagram : result.value()) {
    folly::io::Cursor cursor(datagram.get());
    auto quarterStreamId = quic::decodeQuicInteger(cursor);
    if (!quarterStreamId || quarterStreamId->first > kMaxQuarterStreamId) {
      dropConnectionAsync(
          quic::QuicError(HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR,
                          "H3_DATAGRAM: error decoding stream-id"),
          kErrorConnection);
      break;
    }
    // TODO: draft 8 and rfc don't include context ID
    auto ctxId = quic::decodeQuicInteger(cursor);
    if (!ctxId) {
      dropConnectionAsync(
          quic::QuicError(HTTP3::ErrorCode::HTTP_GENERAL_PROTOCOL_ERROR,
                          "H3_DATAGRAM: error decoding context-id"),
          kErrorConnection);
    }

    quic::BufQueue datagramQ;
    datagramQ.append(std::move(datagram));
    datagramQ.trimStart(quarterStreamId->second + ctxId->second);

    auto streamId = quarterStreamId->first * 4;
    auto stream = findNonDetachedStream(streamId);

    if (!stream || !stream->hasHeaders_) {
      VLOG(4) << "Stream cannot receive datagrams yet. streamId=" << streamId
              << " ctx=" << ctxId->first << " len=" << datagramQ.chainLength()
              << " sess=" << *this;
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

    VLOG(4) << "Received datagram for streamId=" << streamId
            << " ctx=" << ctxId->first << " len=" << datagramQ.chainLength()
            << " sess=" << *this;
    stream->txn_.onDatagram(datagramQ.move());
  }
}

uint16_t HQSession::HQStreamTransport::getDatagramSizeLimit() const noexcept {
  if (!session_.datagramEnabled_) {
    return 0;
  }
  auto transportMaxDatagramSize = session_.sock_->getDatagramSizeLimit();
  if (transportMaxDatagramSize < kMaxDatagramHeaderSize) {
    return 0;
  }
  return session_.sock_->getDatagramSizeLimit() - kMaxDatagramHeaderSize;
}

bool HQSession::HQStreamTransport::sendDatagram(
    std::unique_ptr<folly::IOBuf> datagram) {
  if (!streamId_.hasValue() || !session_.datagramEnabled_) {
    return false;
  }
  // Prepend the H3 Datagram header to the datagram payload
  // HTTP/3 Datagram {
  //   Quarter Stream ID (i),
  //   [Context ID (i)],
  //   HTTP/3 Datagram Payload (..),
  // }
  quic::Buf headerBuf =
      quic::Buf(folly::IOBuf::create(session_.sock_->getDatagramSizeLimit()));
  quic::BufAppender appender(headerBuf.get(), kMaxDatagramHeaderSize);
  auto streamIdRes = quic::encodeQuicInteger(
      streamId_.value() / 4, [&](auto val) { appender.writeBE(val); });
  if (streamIdRes.hasError()) {
    return false;
  }
  // Always use context-id = 0 for now
  auto ctxIdRes =
      quic::encodeQuicInteger(0, [&](auto val) { appender.writeBE(val); });
  if (ctxIdRes.hasError()) {
    return false;
  }
  VLOG(4) << "Sending datagram for streamId=" << streamId_.value()
          << " len=" << datagram->computeChainDataLength()
          << " sess=" << session_;
  quic::BufQueue queue(std::move(headerBuf));
  queue.append(std::move(datagram));
  auto writeRes = session_.sock_->writeDatagram(queue.move());
  if (writeRes.hasError()) {
    LOG(ERROR) << "Failed to send datagram for streamId=" << streamId_.value();
    return false;
  }
  return true;
}

folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
HQSession::HQStreamTransport::newWebTransportBidiStream() {
  auto id = session_.sock_->createBidirectionalStream();
  if (!id) {
    LOG(ERROR) << "Failed to create new bidirectional stream";
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  if (!writeWTStreamPrefaceToSock(*session_.sock_,
                                  *id,
                                  getEgressStreamId(),
                                  hq::WebTransportStreamType::BIDI)) {
    LOG(ERROR) << "Failed to write bidirectional stream preface";
    // TODO: resetStream/stopSending?
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  session_.sock_->setReadCallback(*id, getWTReadCallback());
  return *id;
}

folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
HQSession::HQStreamTransport::newWebTransportUniStream() {
  auto id = session_.sock_->createUnidirectionalStream();
  if (!id) {
    LOG(ERROR) << "Failed to create unidirectional stream. error='"
               << id.error() << "'";
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  if (!writeWTStreamPrefaceToSock(*session_.sock_,
                                  *id,
                                  getEgressStreamId(),
                                  hq::WebTransportStreamType::UNI)) {
    LOG(ERROR) << "Failed to write unidirectional stream preface";
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  return *id;
}

folly::Expected<HTTPTransaction::Transport::FCState, WebTransport::ErrorCode>
HQSession::HQStreamTransport::sendWebTransportStreamData(
    HTTPCodec::StreamID id, std::unique_ptr<folly::IOBuf> data, bool eof) {
  auto res = session_.sock_->writeChain(id, std::move(data), eof);
  if (res.hasError()) {
    LOG(ERROR) << "Failed to write WT stream data";
    return folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR);
  }
  auto flowControl = session_.sock_->getStreamFlowControl(id);
  if (!flowControl) {
    LOG(ERROR) << "Failed to get flow control";
    return folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR);
  }
  if (!eof && flowControl->sendWindowAvailable == 0) {
    session_.sock_->notifyPendingWriteOnStream(id, getWTWriteCallback());
    VLOG(4) << "Closing fc window";
    return HTTPTransaction::Transport::FCState::BLOCKED;
  } else {
    return HTTPTransaction::Transport::FCState::UNBLOCKED;
  }
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
HQSession::HQStreamTransport::resetWebTransportEgress(HTTPCodec::StreamID id,
                                                      uint32_t errorCode) {
  if (session_.sock_) {
    auto res = session_.sock_->resetStream(
        id,
        quic::ApplicationErrorCode(WebTransport::toHTTPErrorCode(errorCode)));
    if (res.hasError()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
HQSession::HQStreamTransport::pauseWebTransportIngress(HTTPCodec::StreamID id) {
  auto res = session_.sock_->pauseRead(id);
  if (res.hasError()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
HQSession::HQStreamTransport::resumeWebTransportIngress(
    HTTPCodec::StreamID id) {
  auto res = session_.sock_->resumeRead(id);
  if (res.hasError()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
HQSession::HQStreamTransport::stopReadingWebTransportIngress(
    HTTPCodec::StreamID id, uint32_t errorCode) {
  if (session_.sock_) {
    auto res = session_.sock_->setReadCallback(
        id,
        nullptr,
        quic::ApplicationErrorCode(WebTransport::toHTTPErrorCode(errorCode)));
    if (res.hasError()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
  }
  return folly::unit;
}

void HQSession::HQStreamTransport::WTReadCallback::readAvailable(
    quic::StreamId id) noexcept {
  auto readRes = session_.sock_->read(id, 65535);
  if (readRes.hasError()) {
    LOG(ERROR) << "Got synchronous read error=" << readRes.error();
    readError(id, quic::QuicError(readRes.error(), "sync read error"));
    return;
  }
  quic::Buf data = std::move(readRes.value().first);
  bool eof = readRes.value().second;
  if (eof) {
    session_.sock_->setReadCallback(id, nullptr);
  }
  txn_.onWebTransportStreamIngress(id, std::move(data), eof);
}

void HQSession::HQStreamTransport::WTReadCallback::readError(
    quic::StreamId id, quic::QuicError error) noexcept {
  auto quicAppErrorCode = error.code.asApplicationErrorCode();
  if (quicAppErrorCode) {
    auto appErrorCode = WebTransport::toApplicationErrorCode(*quicAppErrorCode);
    if (appErrorCode) {
      txn_.onWebTransportStreamError(id, *appErrorCode);
      return;
    }
  }
  // any other error
  txn_.onWebTransportStreamError(id, WebTransport::kInternalError);

  session_.sock_->setReadCallback(id, nullptr);
}

std::ostream& operator<<(std::ostream& os, const HQSession& session) {
  session.describe(os);
  return os;
}

std::ostream& operator<<(std::ostream& os, HQSession::DrainState drainState) {
  switch (drainState) {
    case HQSession::DrainState::NONE:
      os << "none";
      break;
    case HQSession::DrainState::PENDING:
      os << "pending";
      break;
    case HQSession::DrainState::CLOSE_SENT:
      os << "close_sent";
      break;
    case HQSession::DrainState::CLOSE_RECEIVED:
      os << "close_recvd";
      break;
    case HQSession::DrainState::FIRST_GOAWAY:
      os << "first_goaway";
      break;
    case HQSession::DrainState::SECOND_GOAWAY:
      os << "second_goaway";
      break;
    case HQSession::DrainState::DONE:
      os << "done";
      break;
    default:
      folly::assume_unreachable();
  }
  return os;
}

} // namespace proxygen
