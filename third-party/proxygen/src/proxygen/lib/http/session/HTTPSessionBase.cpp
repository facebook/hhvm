/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPSessionBase.h>

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/session/ByteEventTracker.h>
#include <proxygen/lib/http/session/HTTPSessionController.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>

using folly::SocketAddress;
using wangle::TransportInfo;

namespace proxygen {
std::atomic<uint32_t> HTTPSessionBase::kDefaultReadBufLimit{65536};
uint32_t HTTPSessionBase::maxReadBufferSize_ = 4000;
uint32_t HTTPSessionBase::egressBodySizeLimit_ = 4096;
uint32_t HTTPSessionBase::kDefaultWriteBufLimit = 65536;

HTTPSessionBase::HTTPSessionBase(const SocketAddress& localAddr,
                                 const SocketAddress& peerAddr,
                                 HTTPSessionController* controller,
                                 const TransportInfo& tinfo,
                                 InfoCallback* infoCallback,
                                 std::unique_ptr<HTTPCodec> codec,
                                 const WheelTimerInstance& wheelTimer,
                                 HTTPCodec::StreamID rootNodeId)
    : infoCallback_(infoCallback),
      transportInfo_(tinfo),
      codec_(std::move(codec)),
      txnEgressQueue_(isHTTP2CodecProtocol(codec_->getProtocol())
                          ? WheelTimerInstance(wheelTimer)
                          : WheelTimerInstance(),
                      rootNodeId),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      controller_(controller),
      h2PrioritiesEnabled_(true),
      exHeadersEnabled_(false) {

  // If we receive IPv4-mapped IPv6 addresses, convert them to IPv4.
  localAddr_.tryConvertToIPv4();
  peerAddr_.tryConvertToIPv4();
}

HTTPSessionBase::~HTTPSessionBase() {
  if (sessionStats_) {
    sessionStats_->recordPendingBufferedWriteBytes(-1 *
                                                   (int64_t)pendingWriteSize_);
    sessionStats_->recordPendingBufferedReadBytes(-1 *
                                                  (int64_t)pendingReadSize_);
  }
}

void HTTPSessionBase::setSessionStats(HTTPSessionStats* stats) {
  if (sessionStats_ != stats && sessionStats_ != nullptr) {
    sessionStats_->recordPendingBufferedWriteBytes(-1 *
                                                   (int64_t)pendingWriteSize_);
    sessionStats_->recordPendingBufferedReadBytes(-1 *
                                                  (int64_t)pendingReadSize_);
  }
  sessionStats_ = stats;
  if (sessionStats_) {
    sessionStats_->recordPendingBufferedWriteBytes(pendingWriteSize_);
    sessionStats_->recordPendingBufferedReadBytes(pendingReadSize_);
  }
}

void HTTPSessionBase::setControlMessageRateLimitParams(
    uint32_t maxControlMsgsPerInterval,
    uint32_t maxDirectErrorHandlingPerInterval,
    uint32_t maxHeadersPerInterval,
    std::chrono::milliseconds controlMsgIntervalDuration,
    std::chrono::milliseconds directErrorHandlingIntervalDuration,
    std::chrono::milliseconds headersIntervalDuration) {

  if (maxControlMsgsPerInterval < kMaxControlMsgsPerIntervalLowerBound) {
    XLOG_EVERY_MS(WARNING, 60000)
        << "Invalid maxControlMsgsPerInterval: " << maxControlMsgsPerInterval;
    maxControlMsgsPerInterval = kMaxControlMsgsPerIntervalLowerBound;
  }

  if (maxDirectErrorHandlingPerInterval <
      kMaxDirectErrorHandlingPerIntervalLowerBound) {
    XLOG_EVERY_MS(WARNING, 60000)
        << "Invalid maxDirectErrorHandlingPerInterval: "
        << maxDirectErrorHandlingPerInterval;
    maxDirectErrorHandlingPerInterval =
        kMaxDirectErrorHandlingPerIntervalLowerBound;
  }

  if (maxHeadersPerInterval < kMaxHeadersPerIntervalLowerBound) {
    XLOG_EVERY_MS(WARNING, 60000)
        << "Invalid maxHeadersPerInterval: " << maxHeadersPerInterval;
    maxHeadersPerInterval = kMaxHeadersPerIntervalLowerBound;
  }

  if (controlMessageRateLimitFilter_) {
    controlMessageRateLimitFilter_->setParams(
        maxControlMsgsPerInterval,
        maxDirectErrorHandlingPerInterval,
        maxHeadersPerInterval,
        controlMsgIntervalDuration,
        directErrorHandlingIntervalDuration,
        headersIntervalDuration);
  }
}

void HTTPSessionBase::runDestroyCallbacks() {
  if (infoCallback_) {
    infoCallback_->onDestroy(*this);
  }
  if (controller_) {
    controller_->detachSession(this);
    controller_ = nullptr;
  }
}

void HTTPSessionBase::onCodecChanged() {
  if (controller_) {
    controller_->onSessionCodecChange(this);
  }

  initCodecHeaderIndexingStrategy();
}

void HTTPSessionBase::initCodecHeaderIndexingStrategy() {
  if (controller_) {
    setHeaderIndexingStrategy(controller_->getHeaderIndexingStrategy());
  }
}

bool HTTPSessionBase::onBodyImpl(std::unique_ptr<folly::IOBuf> chain,
                                 size_t length,
                                 uint16_t padding,
                                 HTTPTransaction* txn) {
  DestructorGuard dg(this);
  auto oldSize = pendingReadSize_;
  CHECK_LE(pendingReadSize_,
           std::numeric_limits<uint32_t>::max() - length - padding);
  pendingReadSize_ += length + padding;
  if (httpSessionActivityTracker_) {
    httpSessionActivityTracker_->onIngressBody(length + padding);
  }
  if (sessionStats_) {
    sessionStats_->recordPendingBufferedReadBytes(length + padding);
  }
  txn->onIngressBody(std::move(chain), padding);
  if (oldSize < pendingReadSize_) {
    // Transaction must have buffered something and not called
    // notifyBodyProcessed() on it.
    VLOG(4) << *this << " Enqueued ingress. Ingress buffer uses "
            << pendingReadSize_ << " of " << readBufLimit_ << " bytes.";
    if (ingressLimitExceeded() && oldSize <= readBufLimit_) {
      if (infoCallback_) {
        infoCallback_->onIngressLimitExceeded(*this);
      }
      return true;
    }
  }
  return false;
}

bool HTTPSessionBase::notifyBodyProcessed(uint32_t bytes) {
  CHECK_GE(pendingReadSize_, bytes);
  auto oldSize = pendingReadSize_;
  pendingReadSize_ -= bytes;
  if (sessionStats_) {
    sessionStats_->recordPendingBufferedReadBytes(-1 * (int64_t)bytes);
  }

  VLOG(4) << *this << " Dequeued " << bytes << " bytes of ingress. "
          << "Ingress buffer uses " << pendingReadSize_ << " of "
          << readBufLimit_ << " bytes.";
  if (oldSize > readBufLimit_ && pendingReadSize_ <= readBufLimit_) {
    return true;
  }
  return false;
}

bool HTTPSessionBase::notifyEgressBodyBuffered(int64_t bytes, bool update) {
  pendingWriteSizeDelta_ += bytes;
  VLOG(4) << __func__ << " pwsd=" << pendingWriteSizeDelta_;
  // any net change requires us to update pause/resume state in the
  // loop callback
  if (pendingWriteSizeDelta_ >= 0 && update) {
    // pause inline, resume in loop
    updateWriteBufSize(0);
    return false;
  }
  return true;
}

void HTTPSessionBase::updateWriteBufSize(int64_t delta) {
  // This is the sum of body bytes buffered within transactions_ and in
  // the sock_'s write buffer.
  delta += pendingWriteSizeDelta_;
  pendingWriteSizeDelta_ = 0;
  DCHECK(delta >= 0 || uint64_t(-delta) <= pendingWriteSize_);
  if (sessionStats_) {
    sessionStats_->recordPendingBufferedWriteBytes(delta);
  }
  pendingWriteSize_ += delta;
}

void HTTPSessionBase::updatePendingWrites() {
  if (pendingWriteSizeDelta_) {
    updateWriteBufSize(0);
  }
}

void HTTPSessionBase::handleErrorDirectly(HTTPTransaction* txn,
                                          const HTTPException& error) {
  VLOG(4) << *this << " creating direct error handler";
  DCHECK(txn);
  auto handler = getParseErrorHandler(txn, error);
  if (!handler) {
    txn->sendAbort();
    return;
  }
  txn->setHandler(handler);
  if (infoCallback_) {
    infoCallback_->onIngressError(*this, error.getProxygenError());
  }
  txn->onError(error);
}

HTTPTransaction::Handler* HTTPSessionBase::getParseErrorHandler(
    HTTPTransaction* txn, const HTTPException& error) {
  // we encounter an error before we finish reading the ingress headers.
  if (codec_->getTransportDirection() == TransportDirection::UPSTREAM) {
    // do not return the parse error handler for upstreams, since all we
    // can do in that direction is abort.
    return nullptr;
  }
  return controller_->getParseErrorHandler(txn, error, getLocalAddress());
}

void HTTPSessionBase::enableExHeadersSettings() noexcept {
  HTTPSettings* settings = codec_->getEgressSettings();
  if (settings) {
    settings->setSetting(SettingsId::ENABLE_EX_HEADERS, 1);
    exHeadersEnabled_ = true;
  }
}

void HTTPSessionBase::attachToSessionController() {
  auto controllerPtr = getController();
  if (controllerPtr) {
    controllerPtr->attachSession(this);
  }
}

void HTTPSessionBase::informSessionControllerTransportReady() {
  auto controllerPtr = getController();
  if (controllerPtr) {
    controllerPtr->onTransportReady(this);
  }
}

void HTTPSessionBase::handleLastByteEvents(ByteEventTracker* byteEventTracker,
                                           HTTPTransaction* txn,
                                           size_t encodedSize,
                                           size_t byteOffset,
                                           bool piggybacked) {
  // TODO: sort out the TransportCallback for all the EOM handling cases.
  //  Current code has the same behavior as before when there wasn't commonEom.
  //  The issue here is onEgressBodyLastByte can be called twice, depending on
  //  the encodedSize. E.g., when codec actually write to buffer in sendEOM.
  if (!txn->testAndSetFirstByteSent()) {
    txn->onEgressBodyFirstByte();
  }
  if (!piggybacked) {
    txn->onEgressBodyLastByte();
  }
  // in case encodedSize == 0 we won't get TTLBA which is acceptable
  // noting the fact that we don't have a response body
  if (byteEventTracker && (encodedSize > 0)) {
    byteEventTracker->addLastByteEvent(txn, byteOffset);
  }
}

} // namespace proxygen
