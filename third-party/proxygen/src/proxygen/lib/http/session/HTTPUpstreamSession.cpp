/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPUpstreamSession.h>

#include <proxygen/lib/http/session/HTTPSessionController.h>

#include <folly/io/async/AsyncSSLSocket.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <wangle/acceptor/ConnectionManager.h>

namespace proxygen {

HTTPUpstreamSession::~HTTPUpstreamSession() {
}

bool HTTPUpstreamSession::isReplaySafe() const {
  return sock_ ? sock_->isReplaySafe() : false;
}

bool HTTPUpstreamSession::isReusable() const {
  VLOG(4) << "isReusable: " << *this
          << ", liveTransactions_=" << liveTransactions_
          << ", isClosing()=" << isClosing()
          << ", sock_->connecting()=" << sock_->connecting()
          << ", codec_->isReusable()=" << codec_->isReusable()
          << ", codec_->isBusy()=" << codec_->isBusy()
          << ", numActiveWrites_=" << numActiveWrites_
          << ", writeTimeout_.isScheduled()=" << writeTimeout_.isScheduled()
          << ", ingressError_=" << ingressError_
          << ", hasMoreWrites()=" << hasMoreWrites()
          << ", codec_->supportsParallelRequests()="
          << codec_->supportsParallelRequests();
  return !isClosing() && !sock_->connecting() && codec_->isReusable() &&
         !codec_->isBusy() && !ingressError_ &&
         (codec_->supportsParallelRequests() ||
          (
              // These conditions only apply to serial codec sessions
              !hasMoreWrites() && liveTransactions_ == 0 &&
              !writeTimeout_.isScheduled()));
}

bool HTTPUpstreamSession::isClosing() const {
  VLOG(5) << "isClosing: " << *this << ", sock_->good()=" << sock_->good()
          << ", draining_=" << draining_
          << ", readsShutdown()=" << readsShutdown()
          << ", writesShutdown()=" << writesShutdown()
          << ", writesDraining_=" << writesDraining_
          << ", resetAfterDrainingWrites_=" << resetAfterDrainingWrites_;
  return !sock_->good() || draining_ || readsShutdown() || writesShutdown() ||
         writesDraining_ || resetAfterDrainingWrites_;
}

void HTTPUpstreamSession::startNow() {
  // startNow in base class CHECKs this session has not started.
  HTTPSession::startNow();
  // Upstream specific:
  // create virtual priority nodes and send Priority frames to peer if necessary
  if (priorityMapFactory_) {
    priorityAdapter_ = priorityMapFactory_->createVirtualStreams(this);
    scheduleWrite();
  } else {
    // TODO/T17420249 Move this to the PriorityAdapter and remove it from the
    // codec.
    auto bytes = codec_->addPriorityNodes(
        txnEgressQueue_, writeBuf_, maxVirtualPriorityLevel_);
    if (bytes) {
      scheduleWrite();
    }
  }
}

HTTPTransaction* HTTPUpstreamSession::newTransaction(
    HTTPTransaction::Handler* handler) {
  auto txn = newTransactionWithError(handler);
  if (txn.hasError()) {
    return nullptr;
  }
  return txn.value();
}

folly::Expected<HTTPTransaction*, HTTPUpstreamSession::NewTransactionError>
HTTPUpstreamSession::newTransactionWithError(
    HTTPTransaction::Handler* handler) {
  if (!supportsMoreTransactions()) {
    // This session doesn't support any more parallel transactions
    return folly::makeUnexpected<NewTransactionError>(
        "Number of HTTP outgoing transactions reaches limit in the session");
  } else if (draining_) {
    return folly::makeUnexpected<NewTransactionError>("Connection is draining");
  }

  if (!started_) {
    startNow();
  }

  ProxygenError error;
  auto txn = createTransaction(codec_->createStream(),
                               HTTPCodec::NoStream,
                               HTTPCodec::NoExAttributes,
                               http2::DefaultPriority,
                               &error);

  if (!txn) {
    switch (error) {
      case ProxygenError::kErrorBadSocket:
        return folly::makeUnexpected<NewTransactionError>(
            "Socket connection is closing");
      case ProxygenError::kErrorDuplicatedStreamId:
        return folly::makeUnexpected<NewTransactionError>(
            "HTTP Stream ID already exists");
      default:
        return folly::makeUnexpected<NewTransactionError>(
            "Unknown error when creating HTTP transaction");
    }
  }

  DestructorGuard dg(this);
  txn->setHandler(CHECK_NOTNULL(handler));
  return txn;
}

HTTPTransaction::Handler* HTTPUpstreamSession::getTransactionTimeoutHandler(
    HTTPTransaction* /*txn*/) {
  // No special handler for upstream requests that time out
  return nullptr;
}

bool HTTPUpstreamSession::allTransactionsStarted() const {
  for (const auto& txn : transactions_) {
    if (!txn.second.isPushed() && !txn.second.isEgressStarted()) {
      return false;
    }
  }
  return true;
}

bool HTTPUpstreamSession::onNativeProtocolUpgrade(
    HTTPCodec::StreamID streamID,
    CodecProtocol protocol,
    const std::string& protocolString,
    HTTPMessage&) {

  VLOG(4) << *this << " onNativeProtocolUpgrade streamID=" << streamID
          << " protocol=" << protocolString;

  if (protocol != CodecProtocol::HTTP_2) {
    return false;
  }

  // Create the new Codec
  std::unique_ptr<HTTPCodec> codec =
      std::make_unique<HTTP2Codec>(TransportDirection::UPSTREAM);

  bool ret =
      onNativeProtocolUpgradeImpl(streamID, std::move(codec), protocolString);
  if (ret) {
    auto bytes = codec_->addPriorityNodes(
        txnEgressQueue_, writeBuf_, maxVirtualPriorityLevel_);
    if (bytes) {
      scheduleWrite();
    }
  }
  return ret;
}

void HTTPUpstreamSession::attachThreadLocals(
    folly::EventBase* eventBase,
    folly::SSLContextPtr sslContext,
    const WheelTimerInstance& wheelTimer,
    HTTPSessionStats* stats,
    FilterIteratorFn fn,
    HeaderCodec::Stats* headerCodecStats,
    HTTPSessionController* controller) {
  txnEgressQueue_.attachThreadLocals(wheelTimer);
  if (controlMessageRateLimitFilter_) {
    controlMessageRateLimitFilter_->attachThreadLocals(&eventBase->timer());
  }
  for (auto* rateLimitFilter : rateLimitFilters_) {
    if (rateLimitFilter) {
      rateLimitFilter->attachThreadLocals(&eventBase->timer());
    }
  }
  wheelTimer_ = wheelTimer;
  setController(controller);
  setSessionStats(stats);
  if (sock_) {
    sock_->attachEventBase(eventBase);
    maybeAttachSSLContext(sslContext);
  }
  codec_.foreach (fn);
  codec_->setHeaderCodecStats(headerCodecStats);
  resumeReadsImpl();
  rescheduleLoopCallbacks();
}

void HTTPUpstreamSession::maybeAttachSSLContext(
    folly::SSLContextPtr sslContext) const {
#ifndef NO_ASYNCSSLSOCKET
  auto sslSocket = sock_->getUnderlyingTransport<folly::AsyncSSLSocket>();
  if (sslSocket && sslContext) {
    sslSocket->attachSSLContext(sslContext);
  }
#endif
}

void HTTPUpstreamSession::detachThreadLocals(bool detachSSLContext) {
  CHECK(transactions_.empty());
  cancelLoopCallbacks();
  pauseReadsImpl();
  if (sock_) {
    if (detachSSLContext) {
      maybeDetachSSLContext();
    }
    sock_->detachEventBase();
  }
  txnEgressQueue_.detachThreadLocals();
  if (controlMessageRateLimitFilter_) {
    controlMessageRateLimitFilter_->detachThreadLocals();
  }
  for (auto* rateLimitFilter : rateLimitFilters_) {
    if (rateLimitFilter) {
      rateLimitFilter->detachThreadLocals();
    }
  }
  setController(nullptr);
  setSessionStats(nullptr);
  // The codec filters *shouldn't* be accessible while the socket is detached,
  // I hope
  codec_->setHeaderCodecStats(nullptr);
  auto cm = getConnectionManager();
  if (cm) {
    cm->removeConnection(this);
  }
}

void HTTPUpstreamSession::maybeDetachSSLContext() const {
#ifndef NO_ASYNCSSLSOCKET
  auto sslSocket = sock_->getUnderlyingTransport<folly::AsyncSSLSocket>();
  if (sslSocket) {
    sslSocket->detachSSLContext();
  }
#endif
}

} // namespace proxygen
