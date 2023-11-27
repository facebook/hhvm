/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPTransaction.h>

#include <algorithm>
#include <folly/Conv.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/tracing/ScopedTraceSection.h>
#include <glog/logging.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/http/RFC2616.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>
#include <sstream>

using folly::IOBuf;
using std::unique_ptr;

namespace proxygen {

namespace {
const int64_t kApproximateMTU = 1400;
const std::chrono::seconds kRateLimitMaxDelay(10);
const uint64_t kMaxBufferPerTxn = 65536;
const uint64_t kMaxWTIngressBuf = 65536;

using namespace proxygen;
HTTPException stateMachineError(HTTPException::Direction dir, std::string msg) {
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS, msg);
  // Sadly, ProxygenErrorEnum is maxed out at 6 bits, so I cannot add
  // kErrorEgressStateTransition.  Instead, set to ingress and record the
  // exception direction in the 'errno' field of the exception.
  ex.setProxygenError(kErrorIngressStateTransition);
  ex.setCodecStatusCode(ErrorCode::INTERNAL_ERROR);
  ex.setErrno(uint32_t(dir));
  return ex;
}
} // namespace

#define INVARIANT_RETURN(X, Y)                                            \
  if (!(X)) {                                                             \
    invariantViolation(                                                   \
        HTTPException(HTTPException::Direction::INGRESS_AND_EGRESS, #X)); \
    return Y;                                                             \
  }

#define INVARIANT(X)                                                      \
  if (!(X)) {                                                             \
    invariantViolation(                                                   \
        HTTPException(HTTPException::Direction::INGRESS_AND_EGRESS, #X)); \
    return;                                                               \
  }

uint64_t HTTPTransaction::egressBufferLimit_ = kMaxBufferPerTxn;

HTTPTransaction::HTTPTransaction(
    TransportDirection direction,
    HTTPCodec::StreamID id,
    uint32_t seqNo,
    Transport& transport,
    HTTP2PriorityQueueBase& egressQueue,
    folly::HHWheelTimer* timer,
    const folly::Optional<std::chrono::milliseconds>& defaultIdleTimeout,
    HTTPSessionStats* stats,
    bool useFlowControl,
    uint32_t receiveInitialWindowSize,
    uint32_t sendInitialWindowSize,
    http2::PriorityUpdate priority,
    folly::Optional<HTTPCodec::StreamID> assocId,
    folly::Optional<HTTPCodec::ExAttributes> exAttributes,
    bool setIngressTimeoutAfterEom)
    : deferredEgressBody_(folly::IOBufQueue::cacheChainLength()),
      direction_(direction),
      id_(id),
      seqNo_(seqNo),
      transport_(transport),
      stats_(stats),
      recvWindow_(receiveInitialWindowSize),
      sendWindow_(sendInitialWindowSize),
      egressQueue_(egressQueue),
      assocStreamId_(assocId),
      priority_(priority),
      ingressPaused_(false),
      egressPaused_(false),
      flowControlPaused_(false),
      handlerEgressPaused_(false),
      egressRateLimited_(false),
      useFlowControl_(useFlowControl),
      aborted_(false),
      deleting_(false),
      firstByteSent_(false),
      firstHeaderByteSent_(false),
      inResume_(false),
      isCountedTowardsStreamLimit_(false),
      ingressErrorSeen_(false),
      priorityFallback_(false),
      headRequest_(false),
      enableLastByteFlushedTracking_(false),
      wtConnectStream_(false),
      egressHeadersDelivered_(false),
      has1xxResponse_(false),
      isDelegated_(false),
      idleTimeout_(defaultIdleTimeout),
      timer_(timer),
      setIngressTimeoutAfterEom_(setIngressTimeoutAfterEom) {
  if (assocStreamId_) {
    if (isUpstream()) {
      egressState_ = HTTPTransactionEgressSM::State::SendingDone;
    } else {
      ingressState_ = HTTPTransactionIngressSM::State::ReceivingDone;
    }
  }

  if (exAttributes) {
    exAttributes_ = exAttributes;
    if (exAttributes_->unidirectional) {
      if (isRemoteInitiated()) {
        egressState_ = HTTPTransactionEgressSM::State::SendingDone;
      } else {
        ingressState_ = HTTPTransactionIngressSM::State::ReceivingDone;
      }
    }
  }

  updateReadTimeout();
  if (stats_) {
    stats_->recordTransactionOpened();
  }

  if (direction_ == TransportDirection::DOWNSTREAM || !isPushed()) {
    queueHandle_ =
        egressQueue_.addTransaction(id_, priority, this, false, &insertDepth_);
  }
  if (priority.streamDependency != egressQueue_.getRootId() &&
      insertDepth_ == 1) {
    priorityFallback_ = true;
  }

  currentDepth_ = insertDepth_;
}

void HTTPTransaction::onDelayedDestroy(bool delayed) {
  if (!isEgressComplete() || !isIngressComplete() || isEnqueued() ||
      pendingByteEvents_ > 0 || deleting_) {
    return;
  }
  VLOG(4) << "destroying transaction " << *this;
  deleting_ = true;
  // These loops are dicey for possible erasure from callbacks
  for (auto ingressStreamIt = wtIngressStreams_.begin();
       ingressStreamIt != wtIngressStreams_.end();) {
    auto id = ingressStreamIt->first;
    auto& stream = ingressStreamIt->second;
    ingressStreamIt++;
    // Deliver an error to the application if needed
    if (stream.open()) {
      VLOG(4) << "aborting WT ingress id=" << id;
      stream.error(WebTransport::kInternalError);
      transport_.stopReadingWebTransportIngress(id,
                                                WebTransport::kInternalError);
      // TODO: does the spec say how to handle this at the transport?  Eg: the
      // peer must RESET any open write streams.
    } else {
      VLOG(4) << "WT ingress already complete for id=" << id;
    }
  }
  wtIngressStreams_.clear();
  for (auto egressStreamIt = wtEgressStreams_.begin();
       egressStreamIt != wtEgressStreams_.end();) {
    auto id = egressStreamIt->first;
    auto& stream = egressStreamIt->second;
    egressStreamIt++;
    // Deliver an error to the application
    stream.onStopSending(WebTransport::kInternalError);
    // The handler may have run and reset this stream, removing it from
    // wtEgressStreams_, otherwise we have to reset it.
    if (wtEgressStreams_.find(id) != wtEgressStreams_.end()) {
      resetWebTransportEgress(id,
                              /*TODO: errorCode=*/WebTransport::kInternalError);
    }
  }
  wtEgressStreams_.clear();
  if (handler_) {
    // TODO: call onWebTransportSessionClose?
    handler_->detachTransaction();
    handler_ = nullptr;
  }
  transportCallback_ = nullptr;
  const auto bytesBuffered = recvWindow_.getOutstanding();
  if (bytesBuffered) {
    transport_.notifyIngressBodyProcessed(bytesBuffered);
  }
  transport_.detach(this);
  (void)delayed; // prevent unused variable warnings
}

HTTPTransaction::~HTTPTransaction() {
  // Cancel transaction timeout if still scheduled.
  if (isScheduled()) {
    cancelTimeout();
  }

  if (stats_) {
    stats_->recordTransactionClosed();
  }
  if (isEnqueued()) {
    dequeue();
  }
  // TODO: handle the case where the priority node hangs out longer than
  // the transaction
  if (queueHandle_) {
    egressQueue_.removeTransaction(queueHandle_);
  }
}

void HTTPTransaction::reset(bool useFlowControl,
                            uint32_t receiveInitialWindowSize,
                            uint32_t receiveStreamWindowSize,
                            uint32_t sendInitialWindowSize) {
  useFlowControl_ = useFlowControl;
  recvWindow_.setCapacity(receiveInitialWindowSize);
  setReceiveWindow(receiveStreamWindowSize);
  sendWindow_.setCapacity(sendInitialWindowSize);
}

void HTTPTransaction::onIngressHeadersComplete(
    std::unique_ptr<HTTPMessage> msg) {
  DestructorGuard g(this);
  msg->setSeqNo(seqNo_);
  if (isUpstream() && !isPushed() && msg->isResponse()) {
    lastResponseStatus_ = msg->getStatusCode();
  }
  bool nonFinalPushHeaders = isPushed() && msg->isRequest();
  if (!validateIngressStateTransition(
          msg->isFinal() && !nonFinalPushHeaders
              ? HTTPTransactionIngressSM::Event::onFinalHeaders
              : HTTPTransactionIngressSM::Event::onNonFinalHeaders)) {
    return;
  }
  if (msg->isRequest()) {
    headRequest_ = (msg->getMethod() == HTTPMethod::HEAD);
    wtConnectStream_ = WebTransport::isConnectMessage(*msg);
  }

  if ((msg->isRequest() && msg->getMethod() != HTTPMethod::CONNECT) ||
      (msg->isResponse() && !headRequest_ &&
       !RFC2616::responseBodyMustBeEmpty(msg->getStatusCode()))) {
    // CONNECT payload has no defined semantics
    const auto& contentLen =
        msg->getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH);
    if (!contentLen.empty()) {
      try {
        expectedIngressContentLengthRemaining_ =
            folly::to<uint64_t>(contentLen);
      } catch (const folly::ConversionError& ex) {
        LOG(ERROR) << "Invalid content-length: " << contentLen
                   << ", ex=" << ex.what() << " " << *this;
      }
      if (expectedIngressContentLengthRemaining_) {
        expectedIngressContentLength_ =
            expectedIngressContentLengthRemaining_.value();
      }
    }
  }
  if (transportCallback_) {
    transportCallback_->headerBytesReceived(msg->getIngressHeaderSize());
  }
  updateIngressCompressionInfo(transport_.getCodec().getCompressionInfo());
  if (mustQueueIngress()) {
    checkCreateDeferredIngress();
    deferredIngress_->emplace(
        id_, HTTPEvent::Type::HEADERS_COMPLETE, std::move(msg));
    VLOG(4) << "Queued ingress event of type "
            << HTTPEvent::Type::HEADERS_COMPLETE << " " << *this;
  } else {
    processIngressHeadersComplete(std::move(msg));
  }
}

void HTTPTransaction::processIngressHeadersComplete(
    std::unique_ptr<HTTPMessage> msg) {
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  refreshTimeout();
  if (handler_ && !isIngressComplete()) {
    handler_->onHeadersComplete(std::move(msg));
  }
}

bool HTTPTransaction::updateContentLengthRemaining(size_t len) {
  if (expectedIngressContentLengthRemaining_.has_value()) {
    if (expectedIngressContentLengthRemaining_.value() >= len) {
      expectedIngressContentLengthRemaining_ =
          expectedIngressContentLengthRemaining_.value() - len;
    } else {
      auto errorMsg = folly::to<std::string>(
          "Content-Length/body mismatch onIngressBody: received=",
          len,
          " expecting no more than ",
          expectedIngressContentLengthRemaining_.value());
      LOG(ERROR) << errorMsg << " " << *this;
      if (handler_) {
        HTTPException ex(HTTPException::Direction::INGRESS, errorMsg);
        ex.setProxygenError(kErrorParseBody);
        onError(ex);
      }
      return false;
    }
  }
  return true;
}

void HTTPTransaction::onIngressBody(unique_ptr<IOBuf> chain, uint16_t padding) {
  FOLLY_SCOPED_TRACE_SECTION("HTTPTransaction - onIngressBody");
  DestructorGuard g(this);
  if (isIngressEOMSeen()) {
    std::stringstream ss;
    // Use stringstream to invoke operator << for this
    ss << "onIngressBody after ingress closed " << *this;
    VLOG(4) << ss.str();
    abortAndDeliverError(ErrorCode::STREAM_CLOSED, ss.str());
    return;
  }
  auto len = chain->computeChainDataLength();
  if (len == 0) {
    return;
  }
  if (!validateIngressStateTransition(
          HTTPTransactionIngressSM::Event::onBody)) {
    return;
  }
  if (!updateContentLengthRemaining(len)) {
    return;
  }

  if (transportCallback_) {
    transportCallback_->bodyBytesReceived(len);
  }
  // register the bytes in the receive window
  if (!recvWindow_.reserve(len + padding, useFlowControl_)) {
    std::stringstream ss;
    // Use stringstream to invoke operator << for this
    ss << "recvWindow_.reserve failed with len=" << len
       << " padding=" << padding << " capacity=" << recvWindow_.getCapacity()
       << " outstanding=" << recvWindow_.getOutstanding() << " " << *this;
    LOG(ERROR) << ss.str();
    abortAndDeliverError(ErrorCode::FLOW_CONTROL_ERROR, ss.str());
    return;
  } else {
    INVARIANT(recvWindow_.free(padding));
    recvToAck_ += padding;
  }
  if (mustQueueIngress()) {
    checkCreateDeferredIngress();
    deferredIngress_->emplace(id_, HTTPEvent::Type::BODY, std::move(chain));
    VLOG(4) << "Queued ingress event of type " << HTTPEvent::Type::BODY
            << " size=" << len << " " << *this;
  } else {
    INVARIANT(recvWindow_.free(len));
    processIngressBody(std::move(chain), len);
  }
}

void HTTPTransaction::processIngressBody(unique_ptr<IOBuf> chain, size_t len) {
  FOLLY_SCOPED_TRACE_SECTION("HTTPTransaction - processIngressBody");
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  refreshTimeout();
  transport_.notifyIngressBodyProcessed(len);
  auto chainLen = chain->computeChainDataLength();
  if (handler_) {
    if (!isIngressComplete()) {
      handler_->onBodyWithOffset(ingressBodyOffset_, std::move(chain));
    }

    if (useFlowControl_ && !isIngressEOMSeen()) {
      recvToAck_ += len;
      if (recvToAck_ > 0) {
        uint32_t divisor = 2;
        if (transport_.isDraining()) {
          // only send window updates for draining transports when window is
          // closed
          divisor = 1;
        }
        if (uint32_t(recvToAck_) >= (recvWindow_.getCapacity() / divisor)) {
          flushWindowUpdate();
        }
      }
    } // else don't care about window updates
  }
  ingressBodyOffset_ += chainLen;
}

void HTTPTransaction::onIngressChunkHeader(size_t length) {
  if (!validateIngressStateTransition(
          HTTPTransactionIngressSM::Event::onChunkHeader)) {
    return;
  }
  if (mustQueueIngress()) {
    checkCreateDeferredIngress();
    deferredIngress_->emplace(id_, HTTPEvent::Type::CHUNK_HEADER, length);
    VLOG(4) << "Queued ingress event of type " << HTTPEvent::Type::CHUNK_HEADER
            << " size=" << length << " " << *this;
  } else {
    processIngressChunkHeader(length);
  }
}

void HTTPTransaction::processIngressChunkHeader(size_t length) {
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  refreshTimeout();
  if (handler_ && !isIngressComplete()) {
    handler_->onChunkHeader(length);
  }
}

void HTTPTransaction::onIngressChunkComplete() {
  if (!validateIngressStateTransition(
          HTTPTransactionIngressSM::Event::onChunkComplete)) {
    return;
  }
  if (mustQueueIngress()) {
    checkCreateDeferredIngress();
    deferredIngress_->emplace(id_, HTTPEvent::Type::CHUNK_COMPLETE);
    VLOG(4) << "Queued ingress event of type "
            << HTTPEvent::Type::CHUNK_COMPLETE << " " << *this;
  } else {
    processIngressChunkComplete();
  }
}

void HTTPTransaction::processIngressChunkComplete() {
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  refreshTimeout();
  if (handler_ && !isIngressComplete()) {
    handler_->onChunkComplete();
  }
}

void HTTPTransaction::onIngressTrailers(unique_ptr<HTTPHeaders> trailers) {
  if (!validateIngressStateTransition(
          HTTPTransactionIngressSM::Event::onTrailers)) {
    return;
  }
  if (mustQueueIngress()) {
    checkCreateDeferredIngress();
    deferredIngress_->emplace(
        id_, HTTPEvent::Type::TRAILERS_COMPLETE, std::move(trailers));
    VLOG(4) << "Queued ingress event of type "
            << HTTPEvent::Type::TRAILERS_COMPLETE << " " << *this;
  } else {
    processIngressTrailers(std::move(trailers));
  }
}

void HTTPTransaction::processIngressTrailers(unique_ptr<HTTPHeaders> trailers) {
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  refreshTimeout();
  if (handler_ && !isIngressComplete()) {
    handler_->onTrailers(std::move(trailers));
  }
}

void HTTPTransaction::onIngressUpgrade(UpgradeProtocol protocol) {
  if (!validateIngressStateTransition(
          HTTPTransactionIngressSM::Event::onUpgrade)) {
    return;
  }
  if (mustQueueIngress()) {
    checkCreateDeferredIngress();
    deferredIngress_->emplace(id_, HTTPEvent::Type::UPGRADE, protocol);
    VLOG(4) << "Queued ingress event of type " << HTTPEvent::Type::UPGRADE
            << " " << *this;
  } else {
    processIngressUpgrade(protocol);
  }
}

void HTTPTransaction::processIngressUpgrade(UpgradeProtocol protocol) {
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  if (handler_ && !isIngressComplete()) {
    handler_->onUpgrade(protocol);
  }
}

void HTTPTransaction::onIngressEOM() {
  if (isIngressEOMSeen()) {
    // This can happen when HTTPSession calls onIngressEOF()
    std::stringstream ss;
    // Use stringstream to invoke operator << for this
    ss << "onIngressEOM after ingress closed " << *this;
    VLOG(4) << ss.str();
    abortAndDeliverError(ErrorCode::STREAM_CLOSED, ss.str());
    return;
  }
  if (expectedIngressContentLengthRemaining_.has_value() &&
      expectedIngressContentLengthRemaining_.value() > 0) {
    auto errorMsg = folly::to<std::string>(
        "Content-Length/body mismatch onIngressEOM: expecting another ",
        expectedIngressContentLengthRemaining_.value());
    LOG(ERROR) << errorMsg << " " << *this;
    if (handler_) {
      HTTPException ex(HTTPException::Direction::INGRESS, errorMsg);
      ex.setProxygenError(kErrorParseBody);
      onError(ex);
    }
    return;
  }

  if (!validateIngressStateTransition(HTTPTransactionIngressSM::Event::onEOM)) {
    return;
  }
  // We need to update the read timeout here.  We're not likely to be
  // expecting any more ingress, and the timer should be cancelled
  // immediately.  If we are expecting more, this will reset the timer.
  updateReadTimeout();
  if (mustQueueIngress()) {
    checkCreateDeferredIngress();
    deferredIngress_->emplace(id_, HTTPEvent::Type::MESSAGE_COMPLETE);
    VLOG(4) << "Queued ingress event of type "
            << HTTPEvent::Type::MESSAGE_COMPLETE << " " << *this;
  } else {
    processIngressEOM();
  }
}

void HTTPTransaction::processIngressEOM() {
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  VLOG(4) << "ingress EOM on " << *this;
  const bool wasComplete = isIngressComplete();
  if (!validateIngressStateTransition(
          HTTPTransactionIngressSM::Event::eomFlushed)) {
    return;
  }
  if (handler_) {
    if (!wasComplete) {
      handler_->onEOM();
    }
  } else {
    markEgressComplete();
  }
  updateReadTimeout();
}

bool HTTPTransaction::isExpectingWindowUpdate() const {
  return egressState_ != HTTPTransactionEgressSM::State::SendingDone &&
         useFlowControl_ && sendWindow_.getSize() <= 0;
}

bool HTTPTransaction::isExpectingIngress() const {
  bool upstreamSendingDone = true;
  if (setIngressTimeoutAfterEom_) {
    upstreamSendingDone = isDownstream() || isEgressComplete();
  }
  return isExpectingWindowUpdate() ||
         (!ingressPaused_ && !isIngressEOMSeen() && upstreamSendingDone);
}

void HTTPTransaction::updateReadTimeout() {
  if (isExpectingIngress()) {
    refreshTimeout();
  } else {
    cancelTimeout();
  }
}

void HTTPTransaction::markIngressComplete() {
  VLOG(4) << "Marking ingress complete on " << *this;
  ingressState_ = HTTPTransactionIngressSM::State::ReceivingDone;
  deferredIngress_.reset();
  cancelTimeout();
}

void HTTPTransaction::markEgressComplete() {
  VLOG(4) << "Marking egress complete on " << *this;
  auto pendingBytes = getOutstandingEgressBodyBytes();
  if (pendingBytes) {
    int64_t deferredEgressBodyBytes = folly::to<int64_t>(pendingBytes);
    transport_.notifyEgressBodyBuffered(-deferredEgressBodyBytes);
  }
  deferredEgressBody_.move();
  deferredBufferMeta_.length = 0;
  if (isEnqueued()) {
    dequeue();
  }
  egressState_ = HTTPTransactionEgressSM::State::SendingDone;
}

bool HTTPTransaction::validateIngressStateTransition(
    HTTPTransactionIngressSM::Event event) {
  DestructorGuard g(this);

  if (!HTTPTransactionIngressSM::transit(ingressState_, event)) {
    std::stringstream ss;
    // Use stringstream to invoke operator << for state machine
    ss << "Invalid ingress state transition, state=" << ingressState_
       << ", event=" << event << ", streamID=" << id_;
    auto ex = stateMachineError(HTTPException::Direction::INGRESS, ss.str());
    // This will invoke sendAbort() and also inform the handler of the
    // error and detach the handler.
    onError(ex);
    return false;
  }
  return true;
}

bool HTTPTransaction::validateEgressStateTransition(
    HTTPTransactionEgressSM::Event event) {
  DestructorGuard g(this);

  if (!HTTPTransactionEgressSM::transit(egressState_, event)) {
    std::stringstream ss;
    // Use stringstream to invoke operator << for state machine
    ss << "Invalid egress state transition, state=" << egressState_
       << ", event=" << event << ", streamID=" << id_;
    LOG(ERROR) << ss.str() << " " << *this;
    invariantViolation(
        stateMachineError(HTTPException::Direction::EGRESS, ss.str()));
    return false;
  }
  return true;
}

void HTTPTransaction::invariantViolation(HTTPException ex) {
  LOG(ERROR) << "invariantViolation msg=" << ex.what()
             << " aborted_=" << uint32_t(aborted_) << " " << *this;
  sendAbort();
  if (handler_) {
    handler_->onInvariantViolation(ex);
  } else {
    LOG(FATAL) << "Invariant violation with no handler; ex=" << ex.what();
  }
}

void HTTPTransaction::abortAndDeliverError(ErrorCode codecError,
                                           const std::string& msg) {
  HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS, msg);
  ex.setCodecStatusCode(codecError);
  // onError will call sendAbort if there is a codec status code and no
  // proxygen error code.  It will *also* notify the handler.
  onError(ex);
}

void HTTPTransaction::onError(const HTTPException& error) {
  DestructorGuard g(this);

  const bool wasAborted = aborted_; // see comment below
  const bool wasEgressComplete = isEgressComplete();
  const bool wasIngressComplete = isIngressComplete();
  bool notify = (handler_);
  HTTPException::Direction direction = error.getDirection();

  if (direction == HTTPException::Direction::INGRESS && isIngressEOMSeen() &&
      isExpectingWindowUpdate()) {
    // we got an ingress error, we've seen the entire message, but we're
    // expecting more (window updates).  These aren't coming, convert to
    // INGRESS_AND_EGRESS
    VLOG(4) << "Converting ingress error to ingress+egress due to"
               " flow control, and aborting "
            << *this;
    direction = HTTPException::Direction::INGRESS_AND_EGRESS;
    sendAbort(ErrorCode::FLOW_CONTROL_ERROR);
  }

  if (error.getProxygenError() == kErrorStreamAbort) {
    DCHECK(error.getDirection() ==
           HTTPException::Direction::INGRESS_AND_EGRESS);
    aborted_ = true;
  } else if (error.hasCodecStatusCode()) {
    DCHECK(error.getDirection() ==
           HTTPException::Direction::INGRESS_AND_EGRESS);
    sendAbort(error.getCodecStatusCode());
  }

  switch (direction) {
    case HTTPException::Direction::INGRESS_AND_EGRESS:
      markEgressComplete();
      markIngressComplete();
      if (wasEgressComplete && wasIngressComplete &&
          // We mark egress complete before we get acknowledgement of the
          // write segment finishing successfully.
          // TODO: instead of using DestructorGuard hacks to keep txn around,
          // use an explicit callback function and set egress complete after
          // last byte flushes (or egress error occurs), see #3912823
          (error.getProxygenError() != kErrorWriteTimeout || wasAborted)) {
        notify = false;
      }
      break;
    case HTTPException::Direction::EGRESS:
      markEgressComplete();
      if (!wasEgressComplete && isIngressEOMSeen() && ingressErrorSeen_) {
        // we've already seen an ingress error but we ignored it, hoping the
        // handler would resume and read our queued EOM.  Now both sides are
        // dead and we need to kill this transaction.
        markIngressComplete();
      }
      if (wasEgressComplete &&
          !shouldNotifyExTxnError(HTTPException::Direction::EGRESS)) {
        notify = false;
      }
      break;
    case HTTPException::Direction::INGRESS:
      if (isIngressEOMSeen() &&
          !shouldNotifyExTxnError(HTTPException::Direction::INGRESS)) {
        // Not an error, for now
        ingressErrorSeen_ = true;
        return;
      }
      markIngressComplete();
      if (wasIngressComplete &&
          !shouldNotifyExTxnError(HTTPException::Direction::INGRESS)) {
        notify = false;
      }
      break;
  }
  if (notify && handler_) {
    // mark egress complete may result in handler detaching
    handler_->onError(error);
  }
}

void HTTPTransaction::onGoaway(ErrorCode code) {
  DestructorGuard g(this);
  VLOG(4) << "received GOAWAY notification on " << *this;
  // This callback can be received at any time and does not affect this
  // transaction's ingress or egress state machines. If it would have
  // affected this transaction's state, we would have received onError()
  // instead.
  if (handler_) {
    handler_->onGoaway(code);
  }
}

void HTTPTransaction::onIngressTimeout() {
  DestructorGuard g(this);
  VLOG(4) << "ingress timeout on " << *this;
  pauseIngress();
  bool windowUpdateTimeout = !isEgressComplete() && isExpectingWindowUpdate();
  if (handler_) {
    if (windowUpdateTimeout) {
      HTTPException ex(
          HTTPException::Direction::INGRESS_AND_EGRESS,
          folly::to<std::string>("ingress timeout, streamID=", id_));
      ex.setProxygenError(kErrorWriteTimeout);
      // This is a protocol error
      ex.setCodecStatusCode(ErrorCode::PROTOCOL_ERROR);
      onError(ex);
    } else {
      HTTPException ex(
          HTTPException::Direction::INGRESS,
          folly::to<std::string>("ingress timeout, streamID=", id_));
      ex.setProxygenError(kErrorTimeout);
      onError(ex);
    }
  } else {
    markIngressComplete();
    markEgressComplete();
  }
}

void HTTPTransaction::onIngressWindowUpdate(const uint32_t amount) {
  if (!useFlowControl_) {
    return;
  }
  DestructorGuard g(this);
  VLOG(4) << "Remote side ack'd " << amount << " bytes " << *this;
  updateReadTimeout();
  if (sendWindow_.free(amount)) {
    notifyTransportPendingEgress();
  } else {
    std::stringstream ss;
    // Use stringstream to invoke operator << for this
    ss << "sendWindow_.free failed with amount=" << amount
       << " capacity=" << sendWindow_.getCapacity()
       << " outstanding=" << sendWindow_.getOutstanding() << " " << *this;
    LOG(ERROR) << ss.str();
    abortAndDeliverError(ErrorCode::FLOW_CONTROL_ERROR, ss.str());
  }
}

void HTTPTransaction::onIngressSetSendWindow(const uint32_t newWindowSize) {
  if (!useFlowControl_) {
    return;
  }
  updateReadTimeout();
  if (sendWindow_.setCapacity(newWindowSize)) {
    notifyTransportPendingEgress();
  } else {
    std::stringstream ss;
    // Use stringstream to invoke operator << for this
    ss << "sendWindow_.setCapacity failed with newWindowSize=" << newWindowSize
       << " capacity=" << sendWindow_.getCapacity()
       << " outstanding=" << sendWindow_.getOutstanding() << " " << *this;
    LOG(ERROR) << ss.str();
    abortAndDeliverError(ErrorCode::FLOW_CONTROL_ERROR, ss.str());
  }
}

void HTTPTransaction::onEgressTimeout() {
  DestructorGuard g(this);
  VLOG(4) << "egress timeout on " << *this;
  if (handler_) {
    HTTPException ex(HTTPException::Direction::EGRESS,
                     folly::to<std::string>("egress timeout, streamID=", id_));
    ex.setProxygenError(kErrorTimeout);
    onError(ex);
  } else {
    markEgressComplete();
  }
}

void HTTPTransaction::onEgressHeaderFirstByte() {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->firstHeaderByteFlushed();
  }
}

void HTTPTransaction::onEgressBodyFirstByte() {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->firstByteFlushed();
  }
}

void HTTPTransaction::onEgressBodyLastByte() {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->lastByteFlushed();
  }
}

void HTTPTransaction::onEgressTrackedByte() {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->trackedByteFlushed();
  }
}

void HTTPTransaction::onEgressLastByteAck(std::chrono::milliseconds latency) {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->lastByteAcked(latency);
  }
}

void HTTPTransaction::onLastEgressHeaderByteAcked() {
  FOLLY_SCOPED_TRACE_SECTION("HTTPTransaction - onLastEgressHeaderByteAcked");
  egressHeadersDelivered_ = true;
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->lastEgressHeaderByteAcked();
  }
}

void HTTPTransaction::onEgressBodyBytesAcked(uint64_t bodyOffset) {
  FOLLY_SCOPED_TRACE_SECTION("HTTPTransaction - onEgressBodyBytesAcked");
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->bodyBytesDelivered(bodyOffset);
  }
}

void HTTPTransaction::onEgressBodyBytesTx(uint64_t bodyOffset) {
  FOLLY_SCOPED_TRACE_SECTION("HTTPTransaction - onEgressBodyBytesTx");
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->bodyBytesTx(bodyOffset);
  }
}

void HTTPTransaction::onEgressBodyDeliveryCanceled(uint64_t bodyOffset) {
  FOLLY_SCOPED_TRACE_SECTION("HTTPTransaction - onEgressBodyDeliveryCanceled");
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->bodyBytesDeliveryCancelled(bodyOffset);
  }
}

void HTTPTransaction::onEgressTrackedByteEventTX(const ByteEvent& event) {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->trackedByteEventTX(event);
  }
}

void HTTPTransaction::onEgressTrackedByteEventAck(const ByteEvent& event) {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->trackedByteEventAck(event);
  }
}

void HTTPTransaction::onEgressTransportAppRateLimited() {
  DestructorGuard g(this);
  if (transportCallback_) {
    transportCallback_->transportAppRateLimited();
  }
}

void HTTPTransaction::sendHeadersWithOptionalEOM(const HTTPMessage& headers,
                                                 bool eom) {
  if (!validateEgressStateTransition(
          HTTPTransactionEgressSM::Event::sendHeaders)) {
    return;
  }
  DCHECK(!isEgressComplete());
  if (!headers.isRequest() && !isPushed()) {
    lastResponseStatus_ = headers.getStatusCode();
  }
  if (headers.isRequest()) {
    headRequest_ = (headers.getMethod() == HTTPMethod::HEAD);
    wtConnectStream_ = WebTransport::isConnectMessage(headers);
  } else {
    has1xxResponse_ = headers.is1xxResponse();
  }

  if (headers.isResponse() && !headRequest_) {
    const auto& contentLen =
        headers.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH);
    if (!contentLen.empty()) {
      try {
        expectedResponseLength_ = folly::to<uint64_t>(contentLen);
      } catch (const folly::ConversionError& ex) {
        LOG(ERROR) << "Invalid content-length: " << contentLen
                   << ", ex=" << ex.what() << " " << *this;
      }
    }
  }
  HTTPHeaderSize size;
  transport_.sendHeaders(this, headers, &size, eom);
  if (transportCallback_) {
    transportCallback_->headerBytesGenerated(size);
  }
  updateEgressCompressionInfo(transport_.getCodec().getCompressionInfo());
  if (eom) {
    if (!validateEgressStateTransition(
            HTTPTransactionEgressSM::Event::sendEOM)) {
      return;
    }
    // trailers are supported in this case:
    // trailers are for chunked encoding-transfer of a body
    if (transportCallback_) {
      transportCallback_->bodyBytesGenerated(0);
    }
    if (!validateEgressStateTransition(
            HTTPTransactionEgressSM::Event::eomFlushed)) {
      return;
    }
    updateReadTimeout();
  }
  flushWindowUpdate();
}

bool HTTPTransaction::delegatedTransactionChecks(
    const HTTPMessage& headers) noexcept {
  if (!delegatedTransactionChecks()) {
    return false;
  }
  // DSR txn is for downstream response only
  INVARIANT_RETURN(!headers.isRequest(), false);

  const auto& contentLen =
      headers.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH);
  if (contentLen.empty()) {
    LOG(ERROR) << "Delegate response must include CL header. txn=" << *this;
    return false;
  }
  expectedResponseLength_ = folly::to<uint64_t>(contentLen);
  return true;
}

bool HTTPTransaction::delegatedTransactionChecks() noexcept {
  // TODO: We should examine if it's possible to actually support 1xx resp.
  // For now, just bail from such case.
  if (has1xxResponse_ || headRequest_ || isPushed()) {
    LOG(ERROR) << "This transaction cannot be delegated";
    return false;
  }
  if (direction_ == TransportDirection::UPSTREAM) {
    LOG(ERROR) << "Upstream transaction cannot be delegated";
    return false;
  }
  auto codecProtocol = transport_.getCodec().getProtocol();
  if (codecProtocol != CodecProtocol::HTTP_3 &&
      codecProtocol != CodecProtocol::HQ) {
    LOG(ERROR) << "Only H3 and HQ can be delegated";
    return false;
  }
  // Mixed body types is not supported
  INVARIANT_RETURN(deferredEgressBody_.empty(), false);
  INVARIANT_RETURN(!useFlowControl_, false);
  INVARIANT_RETURN(!isEgressComplete(), false);
  INVARIANT_RETURN(
      egressState_ != HTTPTransactionEgressSM::State::ChunkHeaderSent, false);
  return true;
}

bool HTTPTransaction::sendHeadersWithDelegate(
    const HTTPMessage& headers, std::unique_ptr<DSRRequestSender> sender) {
  auto okToDelegate = delegatedTransactionChecks(headers);
  if (!okToDelegate) {
    LOG(ERROR) << "This transaction can not use delegated body sending. txn="
               << *this;
    return false;
  }
  isDelegated_ = true;
  if (!validateEgressStateTransition(
          HTTPTransactionEgressSM::Event::sendHeaders)) {
    return false;
  }

  lastResponseStatus_ = headers.getStatusCode();
  HTTPHeaderSize size;
  size_t dataFrameHeaderSize = 0;
  if (!transport_.sendHeadersWithDelegate(this,
                                          headers,
                                          &size,
                                          &dataFrameHeaderSize,
                                          *expectedResponseLength_,
                                          std::move(sender))) {
    return false;
  }
  if (transportCallback_) {
    transportCallback_->headerBytesGenerated(size);
    transportCallback_->bodyBytesGenerated(dataFrameHeaderSize);
  }
  updateEgressCompressionInfo(transport_.getCodec().getCompressionInfo());
  return true;
}

void HTTPTransaction::sendHeadersWithEOM(const HTTPMessage& header) {
  sendHeadersWithOptionalEOM(header, true);
}

void HTTPTransaction::sendHeaders(const HTTPMessage& header) {
  sendHeadersWithOptionalEOM(header, false);
}

void HTTPTransaction::sendBody(std::unique_ptr<folly::IOBuf> body) {
  DestructorGuard guard(this);
  bool chunking =
      ((egressState_ == HTTPTransactionEgressSM::State::ChunkHeaderSent) &&
       !transport_.getCodec().supportsParallelRequests()); // see
                                                           // sendChunkHeader

  INVARIANT(deferredBufferMeta_.length == 0);
  if (!validateEgressStateTransition(
          HTTPTransactionEgressSM::Event::sendBody)) {
    return;
  }

  if (body) {
    size_t bodyLen = body->computeChainDataLength();
    actualResponseLength_ = actualResponseLength_.value() + bodyLen;

    if (chunking) {
      // Note, this check doesn't account for cases where sendBody is called
      // multiple times for a single chunk, and the total length exceeds the
      // header.
      DCHECK(!chunkHeaders_.empty());
      DCHECK_LE(bodyLen, chunkHeaders_.back().length)
          << "Sent body longer than chunk header ";
    }
    deferredEgressBody_.append(std::move(body));
    transport_.notifyEgressBodyBuffered(bodyLen);
  }
  notifyTransportPendingEgress();
}

bool HTTPTransaction::addBufferMeta() noexcept {
  DestructorGuard guard(this);
  if (!validateEgressStateTransition(
          HTTPTransactionEgressSM::Event::sendBody)) {
    return false;
  }
  INVARIANT_RETURN(!deferredBufferMeta_.length, false);
  INVARIANT_RETURN(!actualResponseLength_ || !*actualResponseLength_, false);
  auto bufferMetaLen = *expectedResponseLength_;
  deferredBufferMeta_.length = bufferMetaLen;
  actualResponseLength_ = bufferMetaLen;
  transport_.notifyEgressBodyBuffered(bufferMetaLen);

  notifyTransportPendingEgress();
  return true;
}

bool HTTPTransaction::onWriteReady(const uint32_t maxEgress, double ratio) {
  DestructorGuard g(this);
  DCHECK(isEnqueued());
  cumulativeRatio_ += ratio;
  egressCalls_++;
  sendDeferredBody(maxEgress);
  return isEnqueued();
}

// Send up to maxEgress body bytes, including pendingEOM if appropriate
size_t HTTPTransaction::sendDeferredBody(uint32_t maxEgress) {
  const int32_t windowAvailable = sendWindow_.getSize();
  const uint32_t sendWindow =
      useFlowControl_
          ? std::min<uint32_t>(maxEgress,
                               windowAvailable > 0 ? windowAvailable : 0)
          : maxEgress;

  // We shouldn't be called if we have no pending body/EOM, egress is paused, or
  // the send window is closed
  const size_t bytesLeft = getOutstandingEgressBodyBytes();
  INVARIANT_RETURN((bytesLeft > 0 || isEgressEOMQueued()) && sendWindow > 0, 0);

  size_t canSend = std::min<size_t>(sendWindow, bytesLeft);
  if (maybeDelayForRateLimit()) {
    // Timeout will call notifyTransportPendingEgress again
    return 0;
  }

  size_t nbytes = 0;
  bool willSendEOM = false;

  if (chunkHeaders_.empty()) {
    if (deferredEgressBody_.chainLength() > 0) {
      INVARIANT_RETURN(deferredBufferMeta_.length == 0, 0);
      std::unique_ptr<IOBuf> body = deferredEgressBody_.split(canSend);
      nbytes = sendBodyNow(std::move(body), canSend, hasPendingEOM());
    }
    if (deferredBufferMeta_.length > 0) {
      INVARIANT_RETURN(delegatedTransactionChecks(), 0);
      nbytes += sendDeferredBufferMeta(canSend);
    }
  } else {
    size_t curLen = 0;
    // This body is expliticly chunked
    while (!chunkHeaders_.empty() && canSend > 0) {
      Chunk& chunk = chunkHeaders_.front();
      if (!chunk.headerSent) {
        nbytes += transport_.sendChunkHeader(this, chunk.length);
        chunk.headerSent = true;
      }
      curLen = std::min<size_t>(chunk.length, canSend);
      std::unique_ptr<folly::IOBuf> cur = deferredEgressBody_.split(curLen);
      VLOG(4) << "sending " << curLen << " fin=false";
      nbytes += sendBodyNow(std::move(cur), curLen, false);
      canSend -= curLen;
      chunk.length -= curLen;
      if (chunk.length == 0) {
        nbytes += transport_.sendChunkTerminator(this);
        chunkHeaders_.pop_front();
      } else {
        DCHECK_EQ(canSend, 0);
      }
    }
    willSendEOM = hasPendingEOM();
  }
  // Send any queued eom
  if (willSendEOM) {
    nbytes += sendEOMNow();
  }

  // Update the handler's pause state
  notifyTransportPendingEgress();

  if (transportCallback_) {
    transportCallback_->bodyBytesGenerated(nbytes);
  }
  return nbytes;
}

size_t HTTPTransaction::sendDeferredBufferMeta(uint32_t maxEgress) {
  auto bufferMeta = deferredBufferMeta_.split(maxEgress);
  INVARIANT_RETURN(bufferMeta.length > 0, 0);
  auto okToDelegate = delegatedTransactionChecks();
  if (!okToDelegate) {
    VLOG(2) << "Cannot send deferred buffer meta due to "
               "delegatedTransactionChecks. txn="
            << *this;
    return 0;
  }
  auto sendEom = hasPendingEOM();
  VLOG(4) << "DSR transaction sending " << bufferMeta.length
          << " bytes of body. eom=" << ((sendEom) ? "yes" : "no") << " "
          << *this;

  size_t nbytes = 0;
  transport_.notifyEgressBodyBuffered(-static_cast<int64_t>(bufferMeta.length));
  if (sendEom) {
    if (!validateEgressStateTransition(
            HTTPTransactionEgressSM::Event::eomFlushed)) {
      return 0;
    }
  }
  updateReadTimeout();
  nbytes = transport_.sendBody(this, bufferMeta, sendEom);
  bodyBytesEgressed_ += bufferMeta.length;
  for (auto it = egressBodyOffsetsToTrack_.begin();
       it != egressBodyOffsetsToTrack_.end() && it->first < bodyBytesEgressed_;
       it = egressBodyOffsetsToTrack_.begin()) {
    transport_.trackEgressBodyOffset(it->first, it->second);
    egressBodyOffsetsToTrack_.erase(it);
  }
  if (egressLimitBytesPerMs_ > 0) {
    numLimitedBytesEgressed_ += nbytes;
  }
  return nbytes;
}

bool HTTPTransaction::maybeDelayForRateLimit() {
  if (egressLimitBytesPerMs_ <= 0) {
    // No rate limiting
    return false;
  }

  if (numLimitedBytesEgressed_ == 0) {
    // If we haven't egressed any bytes yet, don't delay.
    return false;
  }

  int64_t limitedDurationMs =
      (int64_t)millisecondsBetween(getCurrentTime(), startRateLimit_).count();

  // Algebra!  Try to figure out the next time send where we'll
  // be allowed to send at least 1 full packet's worth.  The
  // formula we're using is:
  //   (bytesSoFar + packetSize) / (timeSoFar + delay) == targetRateLimit
  std::chrono::milliseconds requiredDelay(
      (((int64_t)numLimitedBytesEgressed_ + kApproximateMTU) -
       ((int64_t)egressLimitBytesPerMs_ * limitedDurationMs)) /
      (int64_t)egressLimitBytesPerMs_);

  if (requiredDelay.count() <= 0) {
    // No delay required
    return false;
  }

  if (requiredDelay > kRateLimitMaxDelay) {
    // The delay should never be this long
    VLOG(4) << "ratelim: Required delay too long (" << requiredDelay.count()
            << "ms), ignoring";
    return false;
  }

  // Delay required

  egressRateLimited_ = true;

  if (timer_) {
    timer_->scheduleTimeout(&rateLimitCallback_, requiredDelay);
  }

  notifyTransportPendingEgress();
  return true;
}

void HTTPTransaction::rateLimitTimeoutExpired() {
  egressRateLimited_ = false;
  notifyTransportPendingEgress();
}

size_t HTTPTransaction::sendEOMNow() {
  VLOG(4) << "egress EOM on " << *this;
  // TODO: with ByteEvent refactor, we will have to delay changing this
  // state until later
  if (!validateEgressStateTransition(
          HTTPTransactionEgressSM::Event::eomFlushed)) {
    return 0;
  }
  size_t nbytes = transport_.sendEOM(this, trailers_.get());
  trailers_.reset();
  updateReadTimeout();
  return nbytes;
}

size_t HTTPTransaction::sendBodyNow(std::unique_ptr<folly::IOBuf> body,
                                    size_t bodyLen,
                                    bool sendEom) {
  static const std::string noneStr = "None";
  DCHECK(body);
  DCHECK_GT(bodyLen, 0);
  size_t nbytes = 0;
  if (useFlowControl_) {
    // Because of how sendBodyNow is embedded in HTTPTransaction code flow,
    // calling INVARIANT here is not safe
    CHECK(sendWindow_.reserve(bodyLen));
  }
  VLOG(4) << "Sending " << bodyLen
          << " bytes of body. eom=" << ((sendEom) ? "yes" : "no")
          << " send_window is "
          << (useFlowControl_
                  ? folly::to<std::string>(
                        sendWindow_.getSize(), " / ", sendWindow_.getCapacity())
                  : noneStr)
          << " trailers=" << ((trailers_) ? "yes" : "no") << " " << *this;
  DCHECK_LT(bodyLen, std::numeric_limits<int64_t>::max());
  transport_.notifyEgressBodyBuffered(-static_cast<int64_t>(bodyLen));
  if (sendEom && !trailers_) {
    if (!validateEgressStateTransition(
            HTTPTransactionEgressSM::Event::eomFlushed)) {
      return 0;
    }
  } else if (ingressErrorSeen_ && isExpectingWindowUpdate()) {
    // I don't know how we got here but we're in trouble.  We need a window
    // update to continue but we've already seen an ingress error.
    HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                     folly::to<std::string>("window blocked with ingress error,"
                                            " streamID=",
                                            id_));
    ex.setProxygenError(kErrorEOF);
    ex.setCodecStatusCode(ErrorCode::FLOW_CONTROL_ERROR);
    onError(ex);
    return 0;
  }
  updateReadTimeout();
  nbytes = transport_.sendBody(this,
                               std::move(body),
                               sendEom && !trailers_,
                               enableLastByteFlushedTracking_);
  bodyBytesEgressed_ += bodyLen;
  for (auto it = egressBodyOffsetsToTrack_.begin();
       it != egressBodyOffsetsToTrack_.end() && it->first < bodyBytesEgressed_;
       it = egressBodyOffsetsToTrack_.begin()) {
    transport_.trackEgressBodyOffset(it->first, it->second);
    egressBodyOffsetsToTrack_.erase(it);
  }
  if (sendEom && trailers_) {
    nbytes += sendEOMNow();
  }
  if (egressLimitBytesPerMs_ > 0) {
    numLimitedBytesEgressed_ += nbytes;
  }
  return nbytes;
}

void HTTPTransaction::sendEOM() {
  DestructorGuard g(this);
  if (!validateEgressStateTransition(HTTPTransactionEgressSM::Event::sendEOM)) {
    return;
  }
  if (expectedResponseLength_ && actualResponseLength_ &&
      (*expectedResponseLength_ != *actualResponseLength_)) {
    if (stats_) {
      stats_->recordEgressContentLengthMismatches();
    }
    auto errorMsg = folly::to<std::string>(
        "Content-Length/body mismatch sendEOM: expected=",
        *expectedResponseLength_,
        ", actual= ",
        *actualResponseLength_);
    LOG(ERROR) << errorMsg << " " << *this;
  }

  if (getOutstandingEgressBodyBytes() == 0 && chunkHeaders_.empty()) {
    // there is nothing left to send, egress the EOM directly.  For SPDY
    // this will jump the txn queue
    if (!isEnqueued()) {
      size_t nbytes = sendEOMNow();
      transport_.notifyPendingEgress();
      if (transportCallback_) {
        transportCallback_->bodyBytesGenerated(nbytes);
      }
    } else {
      // If the txn is enqueued, sendDeferredBody()
      // should take care of sending the EOM.
      // This can happen for some uses of the egress queue
      VLOG(4) << "Queued egress EOM with no body"
              << "[egressState=" << egressState_ << ", "
              << "ingressState=" << ingressState_ << ", "
              << "egressPaused=" << egressPaused_ << ", "
              << "ingressPaused=" << ingressPaused_ << ", "
              << "aborted=" << aborted_ << ", "
              << "enqueued=" << isEnqueued() << ", "
              << "chainLength=" << deferredEgressBody_.chainLength() << ", "
              << "bufferMetaLen=" << deferredBufferMeta_.length << "]"
              << " on " << *this;
    }
  } else {
    VLOG(4) << "Queued egress EOM on " << *this;
    notifyTransportPendingEgress();
  }
}

void HTTPTransaction::sendAbort() {
  sendAbort(isUpstream() ? ErrorCode::CANCEL : ErrorCode::INTERNAL_ERROR);
}

void HTTPTransaction::sendAbort(ErrorCode statusCode) {
  DestructorGuard g(this);
  markIngressComplete();
  markEgressComplete();
  if (aborted_) {
    // This can happen in cases where the abort is sent before notifying the
    // handler, but its logic also wants to abort
    VLOG(4) << "skipping redundant abort";
    return;
  }
  VLOG(4) << "aborting transaction " << *this;
  aborted_ = true;
  size_t nbytes = transport_.sendAbort(this, statusCode);
  if (transportCallback_) {
    HTTPHeaderSize size;
    size.uncompressed = nbytes;
    transportCallback_->headerBytesGenerated(size);
  }
}

bool HTTPTransaction::trackEgressBodyOffset(uint64_t offset,
                                            ByteEvent::EventFlags flags) {
  if (transport_.getSessionType() != Transport::Type::QUIC) {
    // for now
    return false;
  }
  if (offset < bodyBytesEgressed_) {
    // we've egressed this byte already, ask transport to track it
    transport_.trackEgressBodyOffset(offset, flags);
  } else {
    egressBodyOffsetsToTrack_.emplace(offset, flags);
  }
  return true;
}

uint16_t HTTPTransaction::getDatagramSizeLimit() const noexcept {
  return transport_.getDatagramSizeLimit();
}

bool HTTPTransaction::sendDatagram(std::unique_ptr<folly::IOBuf> datagram) {
  if (!validateEgressStateTransition(
          HTTPTransactionEgressSM::Event::sendDatagram)) {
    return false;
  }

  auto size = datagram->computeChainDataLength();
  if (size > getDatagramSizeLimit()) {
    return false;
  }

  auto sent = transport_.sendDatagram(std::move(datagram));

  if (sent && transportCallback_) {
    transportCallback_->datagramBytesGenerated(size);
  }

  return sent;
}

folly::Expected<WebTransport::BidiStreamHandle, WebTransport::ErrorCode>
HTTPTransaction::newWebTransportBidiStream() {
  auto id = transport_.newWebTransportBidiStream();
  if (!id) {
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  auto ingressRes =
      wtIngressStreams_.emplace(std::piecewise_construct,
                                std::forward_as_tuple(*id),
                                std::forward_as_tuple(*this, *id));
  auto egressRes = wtEgressStreams_.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(*id),
                                            std::forward_as_tuple(*this, *id));
  return WebTransport::BidiStreamHandle(
      {&ingressRes.first->second, &egressRes.first->second});
}

folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
HTTPTransaction::newWebTransportUniStream() {
  auto id = transport_.newWebTransportUniStream();
  if (!id) {
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  auto res = wtEgressStreams_.emplace(std::piecewise_construct,
                                      std::forward_as_tuple(*id),
                                      std::forward_as_tuple(*this, *id));
  return &res.first->second;
}

folly::Expected<HTTPTransaction::Transport::FCState, WebTransport::ErrorCode>
HTTPTransaction::sendWebTransportStreamData(HTTPCodec::StreamID id,
                                            std::unique_ptr<folly::IOBuf> data,
                                            bool eof) {
  refreshTimeout();
  auto res = transport_.sendWebTransportStreamData(id, std::move(data), eof);
  if (eof || res.hasError()) {
    wtEgressStreams_.erase(id);
  }
  return res;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
HTTPTransaction::resetWebTransportEgress(HTTPCodec::StreamID id,
                                         uint32_t errorCode) {
  auto res = transport_.resetWebTransportEgress(id, errorCode);
  wtEgressStreams_.erase(id);
  return res;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
HTTPTransaction::stopReadingWebTransportIngress(HTTPCodec::StreamID id,
                                                uint32_t errorCode) {
  auto res = transport_.stopReadingWebTransportIngress(id, errorCode);
  wtIngressStreams_.erase(id);
  return res;
}

folly::Optional<HTTPTransaction::ConnectionToken>
HTTPTransaction::getConnectionToken() const noexcept {
  return transport_.getConnectionToken();
}

void HTTPTransaction::pauseIngress() {
  VLOG(4) << "pauseIngress request " << *this;
  DestructorGuard g(this);
  if (ingressPaused_) {
    VLOG(4) << "can't pause ingress; ingressPaused=" << ingressPaused_;
    return;
  }
  ingressPaused_ = true;
  cancelTimeout();
  transport_.pauseIngress(this);
}

void HTTPTransaction::resumeIngress() {
  VLOG(4) << "resumeIngress request " << *this;
  DestructorGuard g(this);
  if (!ingressPaused_ || isIngressComplete()) {
    VLOG(4) << "can't resume ingress, ingressPaused=" << ingressPaused_
            << ", ingressComplete=" << isIngressComplete()
            << ", inResume_=" << inResume_ << " " << *this;
    return;
  }
  ingressPaused_ = false;
  transport_.resumeIngress(this);
  if (inResume_) {
    VLOG(4) << "skipping recursive resume loop " << *this;
    return;
  }
  inResume_ = true;
  SCOPE_EXIT {
    updateReadTimeout();
    inResume_ = false;
  };

  if (deferredIngress_ && (maxDeferredIngress_ <= deferredIngress_->size())) {
    maxDeferredIngress_ = deferredIngress_->size();
  }

  // Process any deferred ingress callbacks
  // Note: we recheck the ingressPaused_ state because a callback
  // invoked by the resumeIngress() call above could have re-paused
  // the transaction.
  while (!ingressPaused_ && deferredIngress_ && !deferredIngress_->empty()) {
    HTTPEvent& callback(deferredIngress_->front());
    VLOG(5) << "Processing deferred ingress callback of type "
            << callback.getEvent() << " " << *this;
    SCOPE_EXIT {
      if (deferredIngress_) {
        deferredIngress_->pop();
      }
    };
    switch (callback.getEvent()) {
      case HTTPEvent::Type::MESSAGE_BEGIN:
        LOG(FATAL) << "unreachable";
        break;
      case HTTPEvent::Type::HEADERS_COMPLETE:
        processIngressHeadersComplete(callback.getHeaders());
        break;
      case HTTPEvent::Type::BODY: {
        unique_ptr<IOBuf> data = callback.getBody();
        auto len = data->computeChainDataLength();
        INVARIANT(recvWindow_.free(len));
        processIngressBody(std::move(data), len);
      } break;
      case HTTPEvent::Type::CHUNK_HEADER:
        processIngressChunkHeader(callback.getChunkLength());
        break;
      case HTTPEvent::Type::CHUNK_COMPLETE:
        processIngressChunkComplete();
        break;
      case HTTPEvent::Type::TRAILERS_COMPLETE:
        processIngressTrailers(callback.getTrailers());
        break;
      case HTTPEvent::Type::MESSAGE_COMPLETE:
        processIngressEOM();
        break;
      case HTTPEvent::Type::UPGRADE:
        processIngressUpgrade(callback.getUpgradeProtocol());
        break;
    }
  }
}

void HTTPTransaction::pauseEgress() {
  VLOG(4) << "asked to pause egress " << *this;
  DestructorGuard g(this);
  if (egressPaused_) {
    VLOG(4) << "egress already paused " << *this;
    return;
  }
  egressPaused_ = true;
  updateHandlerPauseState();
}

void HTTPTransaction::resumeEgress() {
  VLOG(4) << "asked to resume egress " << *this;
  DestructorGuard g(this);
  if (!egressPaused_) {
    VLOG(4) << "egress already not paused " << *this;
    return;
  }
  egressPaused_ = false;
  updateHandlerPauseState();
}

void HTTPTransaction::setEgressRateLimit(uint64_t bitsPerSecond) {
  egressLimitBytesPerMs_ = bitsPerSecond / 8000;
  if (bitsPerSecond > 0 && egressLimitBytesPerMs_ == 0) {
    VLOG(4) << "ratelim: Limit too low (" << bitsPerSecond << "), ignoring";
  }
  startRateLimit_ = getCurrentTime();
  numLimitedBytesEgressed_ = 0;
}

void HTTPTransaction::notifyTransportPendingEgress() {
  DestructorGuard guard(this);
  CHECK(queueHandle_);
  if (!egressRateLimited_ &&
      (getOutstandingEgressBodyBytes() > 0 || isEgressEOMQueued()) &&
      (!useFlowControl_ || sendWindow_.getSize() > 0)) {
    // Egress isn't paused, we have something to send, and flow
    // control isn't blocking us.
    if (!isEnqueued()) {
      // Insert into the queue and let the session know we've got something
      egressQueue_.signalPendingEgress(queueHandle_);
      transport_.notifyPendingEgress();
    }
  } else if (isEnqueued()) {
    // Nothing to send, or not allowed to send right now.
    egressQueue_.clearPendingEgress(queueHandle_);
  }
  updateHandlerPauseState();
}

void HTTPTransaction::updateHandlerPauseState() {
  if (isEgressEOMSeen()) {
    VLOG(4) << "transaction already egress complete, not updating pause state "
            << *this;
    return;
  }
  int64_t availWindow = sendWindow_.getSize() - getOutstandingEgressBodyBytes();
  // do not count transaction stalled if no more bytes to send,
  // i.e. when availWindow == 0
  if (useFlowControl_ && availWindow < 0 && !flowControlPaused_) {
    VLOG(4) << "transaction stalled by flow control txn=" << *this;
    if (stats_) {
      stats_->recordTransactionStalled();
    }
  }
  flowControlPaused_ = useFlowControl_ && availWindow <= 0;
  bool bufferFull = getOutstandingEgressBodyBytes() > egressBufferLimit_;
  bool handlerShouldBePaused =
      egressPaused_ || flowControlPaused_ || egressRateLimited_ || bufferFull;

  if (!egressPaused_ && bufferFull) {
    VLOG(4) << "Not resuming handler, buffer full, txn=" << *this;
  }

  if (handler_ && handlerShouldBePaused != handlerEgressPaused_) {
    if (handlerShouldBePaused) {
      if (canSendHeaders()) {
        VLOG(4) << "txn hasn't egressed headers, not updating pause state "
                << *this;
        return;
      }
      handlerEgressPaused_ = true;
      VLOG(4) << "egress paused txn=" << *this;
      handler_->onEgressPaused();
    } else {
      handlerEgressPaused_ = false;
      VLOG(4) << "egress resumed txn=" << *this;
      handler_->onEgressResumed();
    }
  }
}

void HTTPTransaction::updateIngressCompressionInfo(
    const CompressionInfo& tableInfo) {
  tableInfo_.ingress = tableInfo.ingress;
}

void HTTPTransaction::updateEgressCompressionInfo(
    const CompressionInfo& tableInfo) {
  tableInfo_.egress = tableInfo.egress;
}

const CompressionInfo& HTTPTransaction::getCompressionInfo() const {
  return tableInfo_;
}

bool HTTPTransaction::mustQueueIngress() const {
  return ingressPaused_ || (deferredIngress_ && !deferredIngress_->empty());
}

void HTTPTransaction::checkCreateDeferredIngress() {
  if (!deferredIngress_) {
    deferredIngress_ = std::make_unique<std::queue<HTTPEvent>>();
  }
}

bool HTTPTransaction::onPushedTransaction(HTTPTransaction* pushTxn) {
  DestructorGuard g(this);
  if (isDelegated_) {
    LOG(ERROR) << "Adding Pushed transaction on a delegated HTTPTransaction "
               << "is not supported. txn=" << *this;
    return false;
  }
  INVARIANT_RETURN(*pushTxn->assocStreamId_ == id_, false);
  if (!handler_) {
    VLOG(4) << "Cannot add a pushed txn to an unhandled txn";
    return false;
  }
  refreshTimeout();
  handler_->onPushedTransaction(pushTxn);
  if (!pushTxn->getHandler()) {
    VLOG(4) << "Failed to create a handler for push transaction";
    return false;
  }
  pushedTransactions_.insert(pushTxn->getID());
  return true;
}

bool HTTPTransaction::onExTransaction(HTTPTransaction* exTxn) {
  DestructorGuard g(this);
  if (isDelegated_) {
    LOG(ERROR) << "Adding ExTransaction on a delegated HTTPTransaction is "
               << "not supported. txn=" << *this;
    return false;
  }
  INVARIANT_RETURN(*(exTxn->getControlStream()) == id_, false);
  if (!handler_) {
    LOG(ERROR) << "Cannot add a exTxn to an unhandled txn";
    return false;
  }
  handler_->onExTransaction(exTxn);
  if (!exTxn->getHandler()) {
    LOG(ERROR) << "Failed to create a handler for ExTransaction";
    return false;
  }
  exTransactions_.insert(exTxn->getID());
  return true;
}

void HTTPTransaction::setIdleTimeout(std::chrono::milliseconds idleTimeout) {
  idleTimeout_ = idleTimeout;
  VLOG(4) << "HTTPTransaction: idle timeout is set to  "
          << std::chrono::duration_cast<std::chrono::milliseconds>(idleTimeout)
                 .count();
  updateReadTimeout();
}

void HTTPTransaction::describe(std::ostream& os) const {
  transport_.describe(os);
  os << ", streamID=" << id_;
}

/*
 * TODO: when HTTPSession sends a SETTINGS frame indicating a
 * different initial window, it should call this function on all its
 * transactions.
 */
void HTTPTransaction::setReceiveWindow(uint32_t capacity) {
  // Depending on whether delta is positive or negative it will cause the
  // window to either increase or decrease.
  if (!useFlowControl_) {
    return;
  }
  int32_t delta = capacity - recvWindow_.getCapacity();
  if (delta < 0) {
    // For now, we're disallowing shrinking the window, since it can lead
    // to FLOW_CONTROL_ERRORs if there is data in flight.
    VLOG(4) << "Refusing to shrink the recv window";
    return;
  }
  if (!recvWindow_.setCapacity(capacity)) {
    return;
  }
  recvToAck_ += delta;
  flushWindowUpdate();
}

void HTTPTransaction::flushWindowUpdate() {
  if (recvToAck_ > 0 && useFlowControl_ && !isIngressEOMSeen() &&
      (direction_ == TransportDirection::DOWNSTREAM ||
       egressState_ != HTTPTransactionEgressSM::State::Start ||
       ingressState_ != HTTPTransactionIngressSM::State::Start)) {
    // Down egress upstream window updates until after headers
    VLOG(4) << "recv_window is " << recvWindow_.getSize() << " / "
            << recvWindow_.getCapacity() << " after acking " << recvToAck_
            << " " << *this;
    transport_.sendWindowUpdate(this, recvToAck_);
    recvToAck_ = 0;
  }
}

int32_t HTTPTransaction::getRecvToAck() const {
  return recvToAck_;
}

std::ostream& operator<<(std::ostream& os, const HTTPTransaction& txn) {
  txn.describe(os);
  return os;
}

void HTTPTransaction::updateAndSendPriority(int8_t newPriority) {
  newPriority = HTTPMessage::normalizePriority(newPriority);
  INVARIANT(newPriority >= 0);
  priority_.streamDependency =
      transport_.getCodec().mapPriorityToDependency(newPriority);
  if (queueHandle_) {
    queueHandle_ = egressQueue_.updatePriority(queueHandle_, priority_);
  }
  transport_.sendPriority(this, priority_);
}

void HTTPTransaction::updateAndSendPriority(
    const http2::PriorityUpdate& newPriority) {
  onPriorityUpdate(newPriority);
  transport_.sendPriority(this, priority_);
}

void HTTPTransaction::updateAndSendPriority(HTTPPriority pri) {
  pri.urgency = HTTPMessage::normalizePriority((int8_t)pri.urgency);
  // Note we no longer want to play with the egressQueue_ with the new API.
  transport_.changePriority(this, pri);
}

void HTTPTransaction::onPriorityUpdate(const http2::PriorityUpdate& priority) {
  if (!queueHandle_) {
    LOG(ERROR) << "Received priority update on ingress only transaction";
    return;
  }
  priority_ = priority;
  queueHandle_ =
      egressQueue_.updatePriority(queueHandle_, priority_, &currentDepth_);
  if (priority_.streamDependency != egressQueue_.getRootId() &&
      currentDepth_ == 1) {
    priorityFallback_ = true;
  }
}

void HTTPTransaction::checkIfEgressRateLimitedByUpstream() {
  if (transportCallback_ && !isEgressEOMQueued() &&
      getOutstandingEgressBodyBytes() == 0) {
    transportCallback_->egressBufferEmpty();
  }
}

void HTTPTransaction::onDatagram(
    std::unique_ptr<folly::IOBuf> datagram) noexcept {
  DestructorGuard g(this);
  if (aborted_) {
    return;
  }
  VLOG(4) << "datagram received on " << *this;
  if (!validateIngressStateTransition(
          HTTPTransactionIngressSM::Event::onDatagram)) {
    return;
  }
  refreshTimeout();
  auto size = datagram->computeChainDataLength();

  if (transportCallback_) {
    transportCallback_->datagramBytesReceived(size);
  }

  if (handler_ && !isIngressComplete()) {
    handler_->onDatagram(std::move(datagram));
  }
}

void HTTPTransaction::onWebTransportBidiStream(HTTPCodec::StreamID id) {
  if (!handler_) {
    transport_.resetWebTransportEgress(id, WebTransport::kInternalError);
    transport_.stopReadingWebTransportIngress(id, WebTransport::kInternalError);
    return;
  }
  refreshTimeout();
  auto ingRes = wtIngressStreams_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(id),
                                          std::forward_as_tuple(*this, id));

  auto egRes = wtEgressStreams_.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(id),
                                        std::forward_as_tuple(*this, id));
  handler_->onWebTransportBidiStream(
      id,
      WebTransport::BidiStreamHandle(
          {&ingRes.first->second, &egRes.first->second}));
}

void HTTPTransaction::onWebTransportUniStream(HTTPCodec::StreamID id) {
  if (!handler_) {
    LOG(ERROR) << "Handler not set";
    transport_.stopReadingWebTransportIngress(id, WebTransport::kInternalError);
    return;
  }
  refreshTimeout();
  auto ingRes = wtIngressStreams_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(id),
                                          std::forward_as_tuple(*this, id));

  handler_->onWebTransportUniStream(id, &ingRes.first->second);
}

folly::Expected<folly::SemiFuture<folly::Unit>, WebTransport::ErrorCode>
HTTPTransaction::TxnStreamWriteHandle::writeStreamData(
    std::unique_ptr<folly::IOBuf> data, bool fin) {
  CHECK(!writePromise_) << "Wait for previous write to complete";
  if (stopSendingErrorCode_) {
    return folly::makeSemiFuture<folly::Unit>(
        folly::make_exception_wrapper<WebTransport::Exception>(
            *stopSendingErrorCode_));
  }
  auto fcState = txn_.sendWebTransportStreamData(id_, std::move(data), fin);
  if (fcState.hasError()) {
    return folly::makeUnexpected(fcState.error());
  }
  if (*fcState == Transport::FCState::UNBLOCKED) {
    return folly::makeSemiFuture(folly::unit);
  } else {
    auto contract = folly::makePromiseContract<folly::Unit>();
    writePromise_.emplace(std::move(contract.first));
    return std::move(contract.second);
  }
}

void HTTPTransaction::TxnStreamWriteHandle::onStopSending(uint32_t errorCode) {
  auto token = cancellationSource_.getToken();
  if (writePromise_) {
    writePromise_->setException(WebTransport::Exception(errorCode));
    writePromise_.reset();
  } else if (!stopSendingErrorCode_) {
    stopSendingErrorCode_ = errorCode;
    cancellationSource_.requestCancellation();
  }
}

void HTTPTransaction::TxnStreamWriteHandle::onEgressReady() {
  if (writePromise_) {
    writePromise_->setValue();
    writePromise_.reset();
  }
}

HTTPTransaction::Transport::FCState
HTTPTransaction::TxnStreamReadHandle::dataAvailable(
    std::unique_ptr<folly::IOBuf> data, bool eof) {
  VLOG(4)
      << "dataAvailable buflen=" << (data ? data->computeChainDataLength() : 0)
      << " eof=" << uint64_t(eof);
  if (readPromise_) {
    readPromise_->setValue(WebTransport::StreamData({std::move(data), eof}));
    readPromise_.reset();
    if (eof) {
      txn_.wtIngressStreams_.erase(getID());
      return Transport::FCState::UNBLOCKED;
    }
  } else {
    buf_.append(std::move(data));
    eof_ = eof;
  }
  VLOG(4) << "dataAvailable buflen=" << buf_.chainLength();
  return (eof || buf_.chainLength() < kMaxWTIngressBuf)
             ? Transport::FCState::UNBLOCKED
             : Transport::FCState::BLOCKED;
}

void HTTPTransaction::TxnStreamReadHandle::error(uint32_t error) {
  cancellationSource_.requestCancellation();
  if (readPromise_) {
    readPromise_->setException(WebTransport::Exception(error));
    readPromise_.reset();
    txn_.wtIngressStreams_.erase(getID());
  } else {
    error_ = error;
  }
}

void HTTPTransaction::onWebTransportStreamIngress(
    HTTPCodec::StreamID id, std::unique_ptr<folly::IOBuf> data, bool eof) {
  refreshTimeout();
  auto ingressStreamIt = wtIngressStreams_.find(id);
  CHECK(ingressStreamIt != wtIngressStreams_.end());
  auto fcState = ingressStreamIt->second.dataAvailable(std::move(data), eof);
  if (fcState == Transport::FCState::BLOCKED) {
    transport_.pauseWebTransportIngress(id);
  }
}

void HTTPTransaction::onWebTransportStreamError(HTTPCodec::StreamID id,
                                                uint32_t errorCode) {
  auto ingressStreamIt = wtIngressStreams_.find(id);
  CHECK(ingressStreamIt != wtIngressStreams_.end()) << id;
  ingressStreamIt->second.error(errorCode);
}

bool HTTPTransaction::onWebTransportStopSending(HTTPCodec::StreamID id,
                                                uint32_t errorCode) {
  auto egressStreamIt = wtEgressStreams_.find(id);
  if (egressStreamIt != wtEgressStreams_.end()) {
    egressStreamIt->second.onStopSending(errorCode);
    // Think hard if we have to reset the stream here...
    return true;
  }
  return false;
}

void HTTPTransaction::onWebTransportEgressReady(HTTPCodec::StreamID id) {
  auto wtStream = wtEgressStreams_.find(id);
  CHECK(wtStream != wtEgressStreams_.end());
  wtStream->second.onEgressReady();
}

folly::SemiFuture<WebTransport::StreamData>
HTTPTransaction::TxnStreamReadHandle::readStreamData() {
  CHECK(!readPromise_) << "One read at a time";
  if (error_) {
    auto ex = folly::make_exception_wrapper<WebTransport::Exception>(*error_);
    txn_.wtIngressStreams_.erase(getID());
    return folly::makeSemiFuture<WebTransport::StreamData>(std::move(ex));
  } else if (buf_.empty() && !eof_) {
    auto contract = folly::makePromiseContract<WebTransport::StreamData>();
    readPromise_.emplace(std::move(contract.first));
    return std::move(contract.second);
  } else {
    auto bufLen = buf_.chainLength();
    WebTransport::StreamData streamData({buf_.move(), eof_});
    if (eof_) {
      txn_.wtIngressStreams_.erase(getID());
    } else if (bufLen >= kMaxWTIngressBuf) {
      txn_.transport_.resumeWebTransportIngress(getID());
    }
    return folly::makeFuture(std::move(streamData));
  }
}

} // namespace proxygen
