/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/sink/HTTPTunnelSink.h>
#include <utility>

namespace proxygen {

constexpr uint16_t kMinReadSize = 1460;
constexpr uint16_t kMaxReadSize = 4000;
constexpr uint8_t kMaxOutstandingWrites = 1;

void HTTPTunnelSink::detachAndAbortIfIncomplete(
    std::unique_ptr<HTTPSink> self) {
  sock_->setReadCB(nullptr);
  handler_ = nullptr;
  // If we haven't seen either EOM, sock is still active so close it
  if (!egressEOMSeen_ && !ingressEOMRead_) {
    sock_->closeWithReset();
  }
  XCHECK(self.get() == this);
  if (outstandingWrites_ > 0) {
    destroyOnWriteComplete_ = true;
    void(self.release());
  }
}

void HTTPTunnelSink::sendBody(std::unique_ptr<folly::IOBuf> body) {
  DestructorCheck::Safety safety(*this);
  resetIdleTimeout();
  ++outstandingWrites_;
  sock_->writeChain(this, std::move(body));
  if (safety.destroyed()) {
    return;
  }
  if (outstandingWrites_ >= kMaxOutstandingWrites && !handlerEgressPaused_) {
    handlerEgressPaused_ = true;
    handler_->onEgressPaused();
  }
}

void HTTPTunnelSink::sendEOM() {
  sock_->shutdownWrite();
  egressEOMSeen_ = true;
  if (ingressEOMRead_) {
    handler_->detachTransaction();
  }
}

bool HTTPTunnelSink::isEgressEOMSeen() {
  return egressEOMSeen_;
}

void HTTPTunnelSink::sendAbort() {
  sock_->closeWithReset();
  handler_->detachTransaction();
}

void HTTPTunnelSink::getCurrentTransportInfo(
    wangle::TransportInfo* tinfo) const {
  auto sock = sock_->getUnderlyingTransport<folly::AsyncSocket>();
  if (sock) {
    tinfo->initWithSocket(sock);
#if defined(__linux__) || defined(__FreeBSD__)
    tinfo->readTcpCongestionControl(sock);
    tinfo->readMaxPacingRate(sock);
#endif // defined(__linux__) || defined(__FreeBSD__)
    tinfo->totalBytes = sock->getRawBytesWritten();
  }
}

void HTTPTunnelSink::pauseIngress() {
  sock_->setReadCB(nullptr);
}

void HTTPTunnelSink::resumeIngress() {
  sock_->setReadCB(this);
}

[[nodiscard]] bool HTTPTunnelSink::isIngressPaused() const {
  return sock_->getReadCallback() == nullptr;
}

[[nodiscard]] bool HTTPTunnelSink::isEgressPaused() const {
  return outstandingWrites_ >= kMaxOutstandingWrites;
}

void HTTPTunnelSink::timeoutExpired() noexcept {
  XLOG(DBG4) << "Closing socket now";
  sock_->closeNow();
  if (handler_) {
    DestructorCheck::Safety safety(*this);
    handler_->onError(HTTPException(
        HTTPException::Direction::INGRESS_AND_EGRESS, "Idle timeout expired"));
    if (!safety.destroyed() && handler_) {
      handler_->detachTransaction();
    }
  }
  idleTimeout_ = std::chrono::milliseconds(0);
}

void HTTPTunnelSink::setIdleTimeout(std::chrono::milliseconds timeout) {
  if (timeout.count() != 0) {
    idleTimeout_ = timeout;
    resetIdleTimeout();
  }
}

// ReadCallback methods
void HTTPTunnelSink::getReadBuffer(void** buf, size_t* bufSize) {
  std::pair<void*, uint32_t> readSpace =
      readBuf_.preallocate(kMinReadSize, kMaxReadSize);
  *buf = readSpace.first;
  *bufSize = readSpace.second;
}

void HTTPTunnelSink::readDataAvailable(size_t readSize) noexcept {
  resetIdleTimeout();
  readBuf_.postallocate(readSize);
  while (!readBuf_.empty()) {
    // Skip any 0 length buffers. Since readBuf_ is not empty, we are
    // guaranteed to find a non-empty buffer.
    while (readBuf_.front()->length() == 0) {
      readBuf_.pop_front();
    }
    handler_->onBody(readBuf_.pop_front());
  }
}

void HTTPTunnelSink::readEOF() noexcept {
  DestructorCheck::Safety safety(*this);
  ingressEOMRead_ = true;
  handler_->onEOM();
  if (!safety.destroyed() && egressEOMSeen_ && handler_) {
    handler_->detachTransaction();
  }
}

void HTTPTunnelSink::readErr(const folly::AsyncSocketException& err) noexcept {
  DestructorCheck::Safety safety(*this);
  handler_->onError(
      HTTPException(HTTPException::Direction::INGRESS_AND_EGRESS, err.what()));
  if (!safety.destroyed() && handler_) {
    handler_->detachTransaction();
  }
}

// Returns true if this sink is destroyed
bool HTTPTunnelSink::writeComplete() {
  outstandingWrites_--;
  if (outstandingWrites_ == 0 && destroyOnWriteComplete_) {
    delete this;
    return true;
  }

  return false;
}

// WriteCallback methods
void HTTPTunnelSink::writeSuccess() noexcept {
  bool destroyed = writeComplete();
  if (!destroyed) {
    // If we drop below the max outstanding writes, resume egress
    if (outstandingWrites_ < kMaxOutstandingWrites && handlerEgressPaused_ &&
        handler_) {
      handler_->onEgressResumed();
      handlerEgressPaused_ = false;
    }
    resetIdleTimeout();
  }
}

void HTTPTunnelSink::writeErr(size_t,
                              const folly::AsyncSocketException& err) noexcept {
  bool destroyed = writeComplete();
  if (!destroyed && handler_) {
    DestructorCheck::Safety safety(*this);
    handler_->onError(HTTPException(
        HTTPException::Direction::INGRESS_AND_EGRESS, err.what()));
    if (!safety.destroyed() && handler_) {
      handler_->detachTransaction();
    }
  }
}

} // namespace proxygen
