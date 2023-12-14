/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQUpstreamSession.h>
#include <quic/common/events/FollyQuicEventBase.h>
#include <wangle/acceptor/ConnectionManager.h>

namespace proxygen {

HQUpstreamSession::~HQUpstreamSession() {
  CHECK_EQ(getNumStreams(), 0);
}

void HQUpstreamSession::startNow() {
  HQSession::startNow();
  if (connectCb_ && connectTimeoutMs_.count() > 0) {
    // Start a timer in case the connection takes too long.
    getEventBase()->timer().scheduleTimeout(&connectTimeout_,
                                            connectTimeoutMs_);
  }
}

void HQUpstreamSession::connectTimeoutExpired() noexcept {
  VLOG(4) << __func__ << " sess=" << *this << ": connection failed";
  if (connectCb_) {
    onConnectionError(quic::QuicError(quic::LocalErrorCode::CONNECT_FAILED,
                                      "connect timeout"));
  }
}

void HQUpstreamSession::onTransportReady() noexcept {
  HQUpstreamSession::DestructorGuard dg(this);
  if (!HQSession::onTransportReadyCommon()) {
    // Something went wrong in onTransportReady, e.g. the ALPN is not supported
    return;
  }
  connectSuccess();
}

void HQUpstreamSession::onFirstPeerPacketProcessed() noexcept {
  HQUpstreamSession::DestructorGuard dg(this);
  if (connectCb_) {
    connectCb_->onFirstPeerPacketProcessed();
  }
}

void HQUpstreamSession::connectSuccess() noexcept {
  HQUpstreamSession::DestructorGuard dg(this);
  if (connectCb_) {
    connectCb_->connectSuccess();
  }
  if (connCbState_ == ConnCallbackState::REPLAY_SAFE) {
    handleReplaySafe();
    connCbState_ = ConnCallbackState::DONE;
  } else {
    connCbState_ = ConnCallbackState::CONNECT_SUCCESS;
  }
}

void HQUpstreamSession::onReplaySafe() noexcept {
  HQUpstreamSession::DestructorGuard dg(this);
  if (connCbState_ == ConnCallbackState::CONNECT_SUCCESS) {
    handleReplaySafe();
    connCbState_ = ConnCallbackState::DONE;
  } else {
    connCbState_ = ConnCallbackState::REPLAY_SAFE;
  }
}

void HQUpstreamSession::handleReplaySafe() noexcept {
  HQSession::onReplaySafe();
  // In the case that zero rtt, onTransportReady is almost called
  // immediately without proof of network reachability, and onReplaySafe is
  // expected to be called in 1 rtt time (if success).
  if (connectCb_) {
    auto cb = connectCb_;
    connectCb_ = nullptr;
    connectTimeout_.cancelTimeout();
    cb->onReplaySafe();
  }
}

void HQUpstreamSession::onConnectionEnd() noexcept {
  VLOG(4) << __func__ << " sess=" << *this;

  HQSession::DestructorGuard dg(this);
  if (connectCb_) {
    onConnectionSetupErrorHandler(quic::QuicError(
        quic::LocalErrorCode::CONNECT_FAILED, "session destroyed"));
  }
  HQSession::onConnectionEnd();
}

void HQUpstreamSession::onConnectionSetupErrorHandler(
    quic::QuicError code) noexcept {
  // For an upstream connection, any error before onTransportReady gets
  // notified as a connect error.
  if (connectCb_) {
    HQSession::DestructorGuard dg(this);
    auto cb = connectCb_;
    connectCb_ = nullptr;
    cb->connectError(std::move(code));
    connectTimeout_.cancelTimeout();
  }
}

bool HQUpstreamSession::isDetachable(bool checkSocket) const {
  VLOG(4) << __func__ << " sess=" << *this;
  // TODO: deal with control streams in h2q
  if (checkSocket && sock_ && !sock_->isDetachable()) {
    return false;
  }
  return getNumOutgoingStreams() == 0 && getNumIncomingStreams() == 0;
}

void HQUpstreamSession::attachThreadLocals(
    folly::EventBase* eventBase,
    std::shared_ptr<const folly::SSLContext>,
    const WheelTimerInstance& timeout,
    HTTPSessionStats* stats,
    FilterIteratorFn fn,
    HeaderCodec::Stats* headerCodecStats,
    HTTPSessionController* controller) {
  // TODO: deal with control streams in h2q
  VLOG(4) << __func__ << " sess=" << *this;
  txnEgressQueue_.attachThreadLocals(timeout);
  setController(controller);
  setSessionStats(stats);
  auto qEvbWrapper = std::make_shared<quic::FollyQuicEventBase>(eventBase);
  if (sock_) {
    sock_->attachEventBase(std::move(qEvbWrapper));
  }
  codec_.foreach (fn);
  setHeaderCodecStats(headerCodecStats);
  getEventBase()->runInLoop(this);
  // The caller MUST re-add the connection to a new connection manager.
}

void HQUpstreamSession::detachThreadLocals(bool) {
  VLOG(4) << __func__ << " sess=" << *this;
  // TODO: deal with control streams in h2q
  CHECK_EQ(getNumOutgoingStreams(), 0);
  cancelLoopCallback();

  // TODO: Pause reads and invoke infocallback
  // pauseReadsImpl();
  if (sock_) {
    sock_->detachEventBase();
  }

  txnEgressQueue_.detachThreadLocals();
  setController(nullptr);
  setSessionStats(nullptr);
  // The codec filters *shouldn't* be accessible while the socket is detached,
  // I hope
  setHeaderCodecStats(nullptr);
  auto cm = getConnectionManager();
  if (cm) {
    cm->removeConnection(this);
  }
}

bool HQUpstreamSession::tryBindIngressStreamToTxn(
    quic::StreamId streamId,
    hq::PushId pushId,
    HQIngressPushStream* pushStream) {
  // lookup pending nascent stream id
  CHECK(pushStream);

  VLOG(4) << __func__ << " attempting to bind streamID=" << streamId
          << " to pushID=" << pushId;
  pushStream->bindTo(streamId);

#if DEBUG
  // Check postconditions - the ingress push stream
  // should own both the push id and the stream id.
  // No nascent stream should own the stream id
  auto streamById = findIngressPushStream(streamId);
  auto streamByPushId = findIngressPushStreamByPushId(pushId);

  DCHECK_EQ(streamId, pushStream->getIngressStreamId());
  DCHECK(streamById) << "Ingress stream must be bound to the streamID="
                     << streamId;
  DCHECK(streamByPushId) << "Ingress stream must be found by the pushID="
                         << pushId;
  DCHECK_EQ(streamById, streamByPushId) << "Must be same stream";
#endif

  VLOG(4) << __func__ << " successfully bound streamID=" << streamId
          << " to pushID=" << pushId;
  return true;
}

// Called when we receive a push promise
HQUpstreamSession::HQStreamTransportBase*
HQUpstreamSession::createIngressPushStream(HTTPCodec::StreamID parentId,
                                           hq::PushId pushId) {

  // Check that a stream with this ID has not been created yet
  DCHECK(!findIngressPushStreamByPushId(pushId))
      << "Ingress stream with this push ID already exists pushID=" << pushId;

  auto matchPair = ingressPushStreams_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(pushId),
      std::forward_as_tuple(
          *this,
          pushId,
          parentId,
          getNumTxnServed(),
          WheelTimerInstance(transactionsTimeout_, getEventBase())));

  CHECK(matchPair.second) << "Emplacement failed, despite earlier "
                             "existence check.";

  auto newIngressPushStream = &matchPair.first->second;

  // If there is a nascent stream ready to be bound to the newly
  // created ingress stream, do it now.
  bool bound = false;
  auto res = pushIdToStreamId_.find(pushId);
  if (res == pushIdToStreamId_.end()) {
    VLOG(4)
        << __func__ << " pushID=" << pushId
        << " not found in the lookup table, size=" << pushIdToStreamId_.size();
  } else {
    bound =
        tryBindIngressStreamToTxn(res->second, pushId, newIngressPushStream);
  }

  VLOG(4) << "Successfully created new ingress push stream"
          << " pushID=" << pushId << " parentStreamID=" << parentId
          << " bound=" << bound << " streamID="
          << (bound ? newIngressPushStream->getIngressStreamId()
                    : static_cast<unsigned long>(-1));

  return newIngressPushStream;
}

HQSession::HQStreamTransportBase* HQUpstreamSession::findPushStream(
    quic::StreamId streamId) {
  return findIngressPushStream(streamId);
}

HQUpstreamSession::HQIngressPushStream* FOLLY_NULLABLE
HQUpstreamSession::findIngressPushStream(quic::StreamId streamId) {
  auto res = streamIdToPushId_.find(streamId);
  if (res == streamIdToPushId_.end()) {
    return nullptr;
  } else {
    return findIngressPushStreamByPushId(res->second);
  }
}

HQUpstreamSession::HQIngressPushStream* FOLLY_NULLABLE
HQUpstreamSession::findIngressPushStreamByPushId(hq::PushId pushId) {
  VLOG(4) << __func__ << " looking up ingress push stream by pushID=" << pushId;
  auto it = ingressPushStreams_.find(pushId);
  if (it == ingressPushStreams_.end()) {
    return nullptr;
  } else {
    return &it->second;
  }
}

bool HQUpstreamSession::erasePushStream(quic::StreamId streamId) {
  auto res = streamIdToPushId_.find(streamId);
  if (res != streamIdToPushId_.end()) {
    auto pushId = res->second;
    // Ingress push stream may be using the push id
    // erase it as well if present
    ingressPushStreams_.erase(pushId);

    // Unconditionally erase the lookup entry tables
    streamIdToPushId_.erase(res);
    pushIdToStreamId_.erase(pushId);
    return true;
  }
  return false;
}

uint32_t HQUpstreamSession::numberOfIngressPushStreams() const {
  return ingressPushStreams_.size();
}

void HQUpstreamSession::dispatchPushStream(quic::StreamId pushStreamId,
                                           hq::PushId pushId,
                                           size_t toConsume) {
  VLOG(4) << __func__ << " streamID=" << pushStreamId << " pushId=" << pushId;

  // TODO: if/when we support client goaway, reject stream if
  // pushId >= minUnseenIncomingPushId_ after the GOAWAY is sent
  minUnseenIncomingPushId_ = std::max(minUnseenIncomingPushId_, pushId);
  DCHECK_GT(toConsume, 0);

  bool eom = false;
  if (serverPushLifecycleCb_) {
    serverPushLifecycleCb_->onNascentPushStreamBegin(pushStreamId, eom);
  }

  auto consumeRes = sock_->consume(pushStreamId, toConsume);
  CHECK(!consumeRes.hasError())
      << "Unexpected error " << consumeRes.error() << " while consuming "
      << toConsume << " bytes from stream=" << pushStreamId
      << " pushId=" << pushId;

  // Replace the peek callback with a read callback and pause the read callback
  sock_->setReadCallback(pushStreamId, this);
  sock_->setPeekCallback(pushStreamId, nullptr);
  sock_->pauseRead(pushStreamId);

  // Increment the sequence no to account for the new transport-like stream
  incrementSeqNo();

  pushIdToStreamId_.emplace(pushId, pushStreamId);
  streamIdToPushId_.emplace(pushStreamId, pushId);

  VLOG(4) << __func__ << " assigned lookup from pushID=" << pushId
          << " to streamID=" << pushStreamId;

  // We have successfully read the push id. Notify the testing callbacks
  if (serverPushLifecycleCb_) {
    serverPushLifecycleCb_->onNascentPushStream(pushStreamId, pushId, eom);
  }

  // If the transaction for the incoming push stream has been created
  // already, bind the new stream to the transaction
  auto ingressPushStream = findIngressPushStreamByPushId(pushId);

  if (ingressPushStream) {
    auto bound =
        tryBindIngressStreamToTxn(pushStreamId, pushId, ingressPushStream);
    VLOG(4) << __func__ << " bound=" << bound << " pushID=" << pushId
            << " pushStreamID=" << pushStreamId << " to txn ";
  }
}

void HQUpstreamSession::HQIngressPushStream::bindTo(quic::StreamId streamId) {
  // Ensure the nascent push stream is in correct state
  // and that its push id matches this stream's push id
  DCHECK(txn_.getAssocTxnId().has_value());
  VLOG(4) << __func__ << " Binding streamID=" << streamId
          << " to txn=" << txn_.getID();
#if DEBUG
  // will throw bad-cast
  HQUpstreamSession& session = dynamic_cast<HQUpstreamSession&>(session_);
#else
  HQUpstreamSession& session = static_cast<HQUpstreamSession&>(session_);
#endif
  // Initialize this stream's codec with the id of the transport stream
  auto codec = session.createCodec(streamId);
  initCodec(std::move(codec), __func__);
  DCHECK_EQ(*codecStreamId_, streamId);

  // Now that the codec is initialized, set the stream ID
  // of the push stream
  setIngressStreamId(streamId);
  DCHECK_EQ(getIngressStreamId(), streamId);

  // Enable ingress on this stream. Read callback for the stream's
  // id will be transferred to the HQSession
  initIngress(__func__);

  // Re-enable reads
  session.resumeReadsForPushStream(streamId);

  // Notify testing callbacks that a full push transaction
  // has been successfully initialized
  if (session.serverPushLifecycleCb_) {
    session.serverPushLifecycleCb_->onPushedTxn(&txn_,
                                                streamId,
                                                getPushId(),
                                                txn_.getAssocTxnId().value(),
                                                false /* eof */);
  }
}

// This can only be unbound in that it has not received a stream ID yet
void HQUpstreamSession::eraseUnboundStream(HQStreamTransportBase* hqStream) {
  auto hqPushIngressStream = dynamic_cast<HQIngressPushStream*>(hqStream);
  CHECK(hqPushIngressStream)
      << "Only HQIngressPushStream streams are allowed to be non-bound";
  // This is what makes it unbound, it also cannot be in the map
  DCHECK(!hqStream->hasIngressStreamId());
  auto pushId = hqPushIngressStream->getPushId();
  DCHECK(pushIdToStreamId_.find(pushId) == pushIdToStreamId_.end());
  ingressPushStreams_.erase(pushId);
}

void HQUpstreamSession::cleanupUnboundPushStreams(
    std::vector<quic::StreamId>& streamsToCleanup) {
  for (auto& it : streamIdToPushId_) {
    auto streamId = it.first;
    auto pushId = it.second;
    if (!ingressPushStreams_.count(pushId)) {
      streamsToCleanup.push_back(streamId);
    }
  }
}
} // namespace proxygen
