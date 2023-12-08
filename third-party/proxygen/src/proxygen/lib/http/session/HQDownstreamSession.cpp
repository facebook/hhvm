/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQDownstreamSession.h>

namespace proxygen {

void HQDownstreamSession::onTransportReady() noexcept {
  HQDownstreamSession::DestructorGuard dg(this);
  if (!onTransportReadyCommon()) {
    return;
  }
  if (infoCallback_) {
    infoCallback_->onTransportReady(*this);
  }
  transportReadyNotified_ = true;
}

void HQDownstreamSession::onFullHandshakeDone() noexcept {
  HQDownstreamSession::DestructorGuard dg(this);
  if (infoCallback_) {
    infoCallback_->onFullHandshakeCompletion(*this);
  }
}

void HQDownstreamSession::onAppRateLimited() noexcept {
  invokeOnEgressStreams(([](HQStreamTransportBase* stream) {
                          stream->txn_.onEgressTransportAppRateLimited();
                        }),
                        false /* includeDetached */);
}

void HQDownstreamSession::onConnectionSetupErrorHandler(
    quic::QuicError /* error */) noexcept {
  // Currently the users of this callback treat it like a connect error,
  // not a general connection error. Since we don't have proper separation
  // suppress the errors after onTransportReady has happened.
  if (infoCallback_ && !transportReadyNotified_) {
    infoCallback_->onConnectionError(*this);
  }
}

void HQDownstreamSession::setupOnHeadersComplete(HTTPTransaction* txn,
                                                 HTTPMessage* msg) {
  HTTPTransaction::Handler* handler =
      getController()->getRequestHandler(*txn, msg);
  CHECK(handler);
  txn->setHandler(handler);
  if (infoCallback_) {
    infoCallback_->onIngressMessage(*this, *msg);
  }
}

bool HQDownstreamSession::isDetachable(bool) const {
  LOG(FATAL) << __func__ << " is an upstream interface";
  return false;
}

void HQDownstreamSession::attachThreadLocals(
    folly::EventBase*,
    std::shared_ptr<const folly::SSLContext>,
    const WheelTimerInstance&,
    HTTPSessionStats*,
    FilterIteratorFn,
    HeaderCodec::Stats*,
    HTTPSessionController*) {
  LOG(FATAL) << __func__ << " is an upstream interface";
}

void HQDownstreamSession::detachThreadLocals(bool) {
  LOG(FATAL) << __func__ << " is an upstream interface";
}

bool HQDownstreamSession::pushAllowedByGoaway(hq::PushId pushId) {
  // TODO: This is always true since we ignore client sent GOAWAY right now
  return pushId < peerMinUnseenId_;
}

HQDownstreamSession::HQEgressPushStream* FOLLY_NULLABLE
HQDownstreamSession::createEgressPushStream(hq::PushId pushId,
                                            quic::StreamId streamId,
                                            quic::StreamId parentStreamId) {

  VLOG(4) << __func__ << "sess=" << *this << " pushId=" << pushId
          << " isClosing()=" << isClosing() << " streamId=" << streamId
          << " parentStreamId=" << parentStreamId;

  // Use version utils to ensure that the session is not in draining state
  if (!pushAllowedByGoaway(pushId)) {
    VLOG(3) << __func__ << " Not creating - session is draining"
            << " sess=" << *this << " pushId=" << pushId
            << " isClosing()=" << isClosing() << " streamId=" << streamId
            << " parentStreamId=" << parentStreamId;
    return nullptr;
  }

  auto codec = createCodec(streamId);

  auto matchPair = egressPushStreams_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(streamId),
      std::forward_as_tuple(
          *this,
          streamId,
          pushId,
          parentStreamId,
          getNumTxnServed(),
          std::move(codec),
          WheelTimerInstance(transactionsTimeout_, getEventBase())));
  incrementSeqNo();

  pushIdToStreamId_[pushId] = streamId;
  streamIdToPushId_[streamId] = pushId;

  CHECK(matchPair.second) << "Emplacement failed, despite earlier "
                             "existence check.";

  // Generate the stream preface
  matchPair.first->second.generateStreamPreface();

  // Generate the push id
  matchPair.first->second.generateStreamPushId();

  // Notify pending egress on the stream
  matchPair.first->second.notifyPendingEgress();

  // tracks max historical streams
  HTTPSessionBase::onNewOutgoingStream(getNumOutgoingStreams());

  return &matchPair.first->second;
}

// this is the creation of outgoing pushed transaction
HTTPTransaction* FOLLY_NULLABLE HQDownstreamSession::newPushedTransaction(
    HTTPCodec::StreamID parentRequestStreamId,
    HTTPTransaction::PushHandler* handler,
    ProxygenError* error) {

  if (isClosing()) {
    VLOG(3) << __func__ << " Not creating transaction - draining ";
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorTransportIsDraining);
    return nullptr;
  }

  auto parentRequestStream = findNonDetachedStream(parentRequestStreamId);
  if (!parentRequestStream) {
    VLOG(3) << __func__
            << " Not creating transaction - request stream StreamID="
            << parentRequestStreamId << " not found";
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorParentStreamNotExist);
    return nullptr;
  }

  // Allocate a new egress unidirectional stream from the socket
  // this method will throw exception on failure
  auto pushStreamId = sock_->createUnidirectionalStream();

  // Record the newly created push transaction in the session
  // NOTE: should be stored in the transaction
  // NOTE: should be cleaned up when the transaction is closed
  if (!pushStreamId) {
    VLOG(2) << __func__ << " failed to create new unidirectional stream";
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorCreatingStream);
    return nullptr;
  }

  auto pushId = createNewPushId();

  // Create the actual outgoing Egress Push stream.
  auto pushStream = createEgressPushStream(
      pushId, pushStreamId.value(), parentRequestStreamId);

  if (!pushStream) {
    LOG(ERROR) << "Creation of the push stream failed, pushID=" << pushId;
    SET_PROXYGEN_ERROR_IF(error, ProxygenError::kErrorCreatingStream);
    return nullptr;
  }

  VLOG(4) << "New pushed transaction: pushId=" << pushId
          << "; pushStreamId=" << pushStreamId.value()
          << "; assocStreamId=" << parentRequestStreamId;

  pushStream->txn_.setHandler(handler);
  return &pushStream->txn_;
}

HQSession::HQStreamTransportBase* HQDownstreamSession::findPushStream(
    quic::StreamId streamId) {
  return findEgressPushStream(streamId);
}

HQDownstreamSession::HQEgressPushStream* FOLLY_NULLABLE
HQDownstreamSession::findEgressPushStream(quic::StreamId streamId) {
  auto it = egressPushStreams_.find(streamId);
  if (it == egressPushStreams_.end()) {
    return nullptr;
  } else {
    auto pstream = &it->second;
    DCHECK(pstream->isUsing(streamId));
    return pstream;
  }
}

bool HQDownstreamSession::erasePushStream(quic::StreamId streamId) {
  auto pushIdIter = streamIdToPushId_.find(streamId);
  if (pushIdIter != streamIdToPushId_.end()) {
    auto pushId = pushIdIter->second;
    pushIdToStreamId_.erase(pushId);
    streamIdToPushId_.erase(pushIdIter);
  }
  return egressPushStreams_.erase(streamId);
}

uint32_t HQDownstreamSession::numberOfEgressPushStreams() const {
  return egressPushStreams_.size();
}

// Push-stream implementation of the "sendPushPromise"
// It invokes HQStreamTransport::sendPushPromise
// Since this method does not uses codecs directly, it should not
// set an active codec
void HQDownstreamSession::HQEgressPushStream::sendPushPromise(
    HTTPTransaction* txn,
    folly::Optional<hq::PushId> pushId,
    const HTTPMessage& headers,
    HTTPHeaderSize* size,
    bool includeEOM) {

  CHECK(txn) << "Must be invoked on a live transaction";
  CHECK(txn->getAssocTxnId())
      << "Must be invoked on a transaction with a parent";
  CHECK_EQ(txn_.getID(), txn->getID()) << " Transaction stream mismatch";
  CHECK(pushId == folly::none) << " The push id is stored in the egress stream,"
                               << " and should not be passed by the session";

  auto parentStreamId = txn->getAssocTxnId();
#if DEBUG
  HQDownstreamSession& session = dynamic_cast<HQDownstreamSession&>(session_);
#else
  HQDownstreamSession& session = static_cast<HQDownstreamSession&>(session_);
#endif
  auto parentStream = session.findNonDetachedStream(*parentStreamId);
  if (!parentStream) {
    session_.dropConnectionAsync(
        quic::QuicError(quic::TransportErrorCode::STREAM_STATE_ERROR,
                        "Send push promise on a stream without a parent"),
        kErrorConnection);
    return;
  }
  // Redirect to the parent transaction
  parentStream->sendPushPromise(txn, pushId_, headers, size, includeEOM);
}

size_t HQDownstreamSession::HQEgressPushStream::generateStreamPushId() {
  // reserve space for max quic interger len
  auto result = hq::writeStreamPreface(writeBuf_, pushId_);
  CHECK(!result.hasError())
      << __func__ << " QUIC integer encoding error value=" << pushId_;

  return *result;
}

// Return a new push id that can be used for outgoing transactions
hq::PushId HQDownstreamSession::createNewPushId() {
  auto newPushId = nextAvailablePushId_++;
  return newPushId;
}

folly::Optional<HTTPHeaders> HQDownstreamSession::getExtraHeaders(
    const HTTPMessage& headers, quic::StreamId streamId) {
  if (!sock_) {
    return folly::none;
  }
  if (headers.getHeaders().exists(HTTP_HEADER_PRIORITY)) {
    return folly::none;
  }
  auto priority = sock_->getStreamPriority(streamId);
  if (!priority) {
    return folly::none;
  }
  HTTPHeaders extraHeaders;
  extraHeaders.add(HTTP_HEADER_PRIORITY,
                   httpPriorityToString(
                       HTTPPriority(priority->level, priority->incremental)));
  return extraHeaders;
}

} // namespace proxygen
