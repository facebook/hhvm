/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/connpool/SessionHolder.h>

#include <folly/Random.h>
#include <folly/io/async/AsyncSocket.h>
#include <glog/logging.h>

using folly::AsyncSocket;
using folly::SocketAddress;

namespace {
const double kJitterPct = 0.3;
}

namespace proxygen {

SessionHolder::SessionHolder(HTTPSessionBase* sess,
                             Callback* parent,
                             Stats* stats,
                             Endpoint endpoint)
    : session_(CHECK_NOTNULL(sess)),
      parent_(CHECK_NOTNULL(parent)),
      stats_(stats),
      jitter_(folly::Random::randDouble(-kJitterPct, kJitterPct)),
      endpoint_(std::move(endpoint)),
      originalSessionInfoCb_(sess->getInfoCallback()) {
  session_->setInfoCallback(this);
}

SessionHolder::~SessionHolder() {
  CHECK(state_ == ListState::DETACHED);
  CHECK(!listHook.is_linked());
  CHECK(!secondaryListHook.is_linked());
}

bool SessionHolder::isPoolable(const HTTPSessionBase* sess) {
  return !sess->isClosing() &&
         (sess->getNumOutgoingStreams() || sess->isReusable());
}

bool SessionHolder::shouldAgeOut(std::chrono::milliseconds maxAge) const {
  if (maxAge.count() <= 0) {
    return false;
  }
  double sessMaxAge = (1 + jitter_) * maxAge.count();
  auto age = millisecondsSince(session_->getSetupTransportInfo().acceptTime);
  return age >= std::chrono::milliseconds(int64_t(sessMaxAge));
}

const HTTPSessionBase& SessionHolder::getSession() const {
  return *session_;
}

HTTPTransaction* SessionHolder::newTransaction(
    HTTPTransaction::Handler* handler) {
  return session_->newTransaction(handler);
}

std::chrono::steady_clock::time_point SessionHolder::getLastUseTime() const {
  return lastUseTime_;
}

void SessionHolder::drain() {
  VLOG(4) << "draining holder=" << *this;
  if (state_ != ListState::DETACHED) {
    unlink();
  }
  if (stats_) {
    // The connection hasn't closed yet, but it will. We won't find out
    // about when it actually closes since we are setting the info
    // callback to nullptr.
    stats_->onConnectionClosed();
    if (session_->hasActiveTransactions()) {
      stats_->onConnectionDeactivated();
    }
  }
  session_->setInfoCallback(originalSessionInfoCb_);
  originalSessionInfoCb_ = nullptr;
  parent_->addDrainingSession(session_);
  session_->drain();
  delete this;
}

void SessionHolder::closeWithReset() {
  if (state_ != ListState::DETACHED) {
    unlink();
  }
  if (stats_) {
    // The connection hasn't closed yet, but it will. We won't find out
    // about when it actually closes since we are setting the info
    // callback to nullptr.
    stats_->onConnectionClosed();
    if (session_->hasActiveTransactions()) {
      stats_->onConnectionDeactivated();
    }
  }
  session_->setInfoCallback(originalSessionInfoCb_);
  originalSessionInfoCb_ = nullptr;
  session_->dropConnection();
  delete this;
}

void SessionHolder::unlink() {
  CHECK(parent_);
  CHECK(listHook.is_linked());

  switch (state_) {
    case ListState::IDLE:
      parent_->detachIdle(this);
      break;
    case ListState::PARTIAL:
      parent_->detachPartiallyFilled(this);
      break;
    case ListState::FULL:
      parent_->detachFilled(this);
      break;
    case ListState::DETACHED:
      LOG(FATAL) << "Inconsistentency between listHook.is_linked() and state_";
  }
  state_ = ListState::DETACHED;
}

void SessionHolder::link() {
  CHECK(state_ == ListState::DETACHED);
  if (!parent_) {
    return;
  }

  if (!isPoolable(session_)) {
    VLOG(4) << *this << " Not pooling session since it is not poolable";
    drain();
    return;
  }
  lastUseTime_ = std::chrono::steady_clock::now();
  auto curTxnCount = session_->getNumOutgoingStreams();
  if (!session_->supportsMoreTransactions()) {
    state_ = ListState::FULL;
    parent_->attachFilled(this);
  } else if (curTxnCount == 0 &&
             session_->isDetachable(/*checkSocket=*/false)) {
    state_ = ListState::IDLE;
    parent_->attachIdle(this);
  } else {
    state_ = ListState::PARTIAL;
    parent_->attachPartiallyFilled(this);
  }
}

void SessionHolder::onCreate(const HTTPSessionBase&) {
  LOG(FATAL) << "onCreate() should not be reachable.";
}

void SessionHolder::onIngressError(const HTTPSessionBase& session,
                                   ProxygenError error) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onIngressError(session, error);
  }
}

void SessionHolder::onRead(const HTTPSessionBase& session, size_t bytesRead) {
  onRead(session, bytesRead, folly::none);
}
void SessionHolder::onRead(const HTTPSessionBase& session,
                           size_t bytesRead,
                           folly::Optional<HTTPCodec::StreamID> streamId) {
  if (stats_) {
    stats_->onRead(bytesRead);
  }
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onRead(session, bytesRead, streamId);
  }
}

void SessionHolder::onWrite(const HTTPSessionBase& session,
                            size_t bytesWritten) {
  if (stats_) {
    stats_->onWrite(bytesWritten);
  }
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onWrite(session, bytesWritten);
  }
}

void SessionHolder::onRequestBegin(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onRequestBegin(session);
  }
}

void SessionHolder::onRequestEnd(const HTTPSessionBase& session,
                                 uint32_t maxIngressQueueSize) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onRequestEnd(session, maxIngressQueueSize);
  }
}

void SessionHolder::onActivateConnection(const HTTPSessionBase& session) {
  if (stats_) {
    stats_->onConnectionActivated();
  }
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onActivateConnection(session);
  }
}

void SessionHolder::onDeactivateConnection(const HTTPSessionBase& sess) {
  if (stats_) {
    stats_->onConnectionDeactivated();
  }
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onDeactivateConnection(sess);
  }
  handleTransactionDetached();
}

void SessionHolder::onDestroy(const HTTPSessionBase& session) {
  if (state_ != ListState::DETACHED) {
    unlink();
  }
  if (stats_) {
    stats_->onConnectionClosed();
  }
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onDestroy(session);
  }
  VLOG(4) << *this << " connection to server was destroyed";
  delete this;
}

void SessionHolder::onIngressMessage(const HTTPSessionBase& session,
                                     const HTTPMessage& msg) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onIngressMessage(session, msg);
  }
}

void SessionHolder::onIngressLimitExceeded(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onIngressLimitExceeded(session);
  }
}

void SessionHolder::onIngressPaused(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onIngressPaused(session);
  }
}

void SessionHolder::onTransactionAttached(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onTransactionAttached(session);
  }
}

void SessionHolder::onTransactionDetached(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onTransactionDetached(session);
  }
  handleTransactionDetached();
}

void SessionHolder::onPingReplySent(int64_t latency) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onPingReplySent(latency);
  }
}

void SessionHolder::onPingReplyReceived() {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onPingReplyReceived();
  }
}

void SessionHolder::onSettingsOutgoingStreamsFull(
    const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onSettingsOutgoingStreamsFull(session);
  }
  if (state_ != ListState::DETACHED && state_ != ListState::FULL) {
    unlink();
    link();
  }
}

void SessionHolder::onSettingsOutgoingStreamsNotFull(
    const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onSettingsOutgoingStreamsNotFull(session);
  }
  if (state_ != ListState::DETACHED && state_ == ListState::FULL) {
    unlink();
    link();
  }
}

void SessionHolder::onFlowControlWindowClosed(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onFlowControlWindowClosed(session);
  }
}

void SessionHolder::onEgressBuffered(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onEgressBuffered(session);
  }
}

void SessionHolder::onEgressBufferCleared(const HTTPSessionBase& session) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onEgressBufferCleared(session);
  }
}

void SessionHolder::onSettings(const HTTPSessionBase& sess,
                               const SettingsList& settings) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onSettings(sess, settings);
  }
}

void SessionHolder::onSettingsAck(const HTTPSessionBase& sess) {
  if (originalSessionInfoCb_) {
    originalSessionInfoCb_->onSettingsAck(sess);
  }
}

void SessionHolder::describe(std::ostream& os) const {
  const auto transport = session_->getTransport();
  if (!transport) {
    os << "(nullptr)";
    return;
  }
  const AsyncSocket* sock = transport->getUnderlyingTransport<AsyncSocket>();
  if (sock) {
    os << "fd=" << sock->getNetworkSocket().toFd();

    SocketAddress localAddr, serverAddr;
    try {
      sock->getLocalAddress(&localAddr);
      sock->getPeerAddress(&serverAddr);
    } catch (...) {
      // The socket might have been disconnected.
    }

    if (localAddr.isInitialized()) {
      os << ",local=" << localAddr;
    } else {
      os << ",lp=-1";
    }

    if (serverAddr.isInitialized()) {
      os << "," << serverAddr;
    } else {
      os << ",-";
    }
  } else {
    os << "fd=-1,lp=-1,-";
  }
  os << ",listState=" << uint32_t(state_);
}

void SessionHolder::handleTransactionDetached() {
  CHECK(state_ != ListState::DETACHED);
  unlink();
  link();
}

} // namespace proxygen
