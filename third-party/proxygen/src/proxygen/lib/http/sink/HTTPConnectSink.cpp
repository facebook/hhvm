/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/sink/HTTPConnectSink.h>
#include <utility>

namespace proxygen {

constexpr uint16_t kMinReadSize = 1460;
constexpr uint16_t kMaxReadSize = 4000;
constexpr uint8_t kMaxOutstandingWrites = 1;

void HTTPConnectSink::detachAndAbortIfIncomplete(
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

void HTTPConnectSink::sendBody(std::unique_ptr<folly::IOBuf> body) {
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

void HTTPConnectSink::sendEOM() {
  sock_->shutdownWrite();
  egressEOMSeen_ = true;
  if (ingressEOMRead_) {
    handler_->detachTransaction();
  }
}

bool HTTPConnectSink::isEgressEOMSeen() {
  return egressEOMSeen_;
}

void HTTPConnectSink::sendAbort() {
  sock_->closeWithReset();
  handler_->detachTransaction();
}

void HTTPConnectSink::getCurrentTransportInfo(
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

void HTTPConnectSink::pauseIngress() {
  sock_->setReadCB(nullptr);
}

void HTTPConnectSink::resumeIngress() {
  sock_->setReadCB(this);
}

[[nodiscard]] bool HTTPConnectSink::isIngressPaused() const {
  return sock_->getReadCallback() == nullptr;
}

[[nodiscard]] bool HTTPConnectSink::isEgressPaused() const {
  return outstandingWrites_ >= kMaxOutstandingWrites;
}

void HTTPConnectSink::timeoutExpired() noexcept {
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

void HTTPConnectSink::setIdleTimeout(std::chrono::milliseconds timeout) {
  if (timeout.count() != 0) {
    idleTimeout_ = timeout;
    resetIdleTimeout();
  }
}

// ReadCallback methods
void HTTPConnectSink::getReadBuffer(void** buf, size_t* bufSize) {
  std::pair<void*, uint32_t> readSpace =
      readBuf_.preallocate(kMinReadSize, kMaxReadSize);
  *buf = readSpace.first;
  *bufSize = readSpace.second;
}

void HTTPConnectSink::readDataAvailable(size_t readSize) noexcept {
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

void HTTPConnectSink::readEOF() noexcept {
  DestructorCheck::Safety safety(*this);
  ingressEOMRead_ = true;
  handler_->onEOM();
  if (!safety.destroyed() && egressEOMSeen_ && handler_) {
    handler_->detachTransaction();
  }
}

void HTTPConnectSink::readErr(const folly::AsyncSocketException& err) noexcept {
  DestructorCheck::Safety safety(*this);
  handler_->onError(
      HTTPException(HTTPException::Direction::INGRESS_AND_EGRESS, err.what()));
  if (!safety.destroyed() && handler_) {
    handler_->detachTransaction();
  }
}

// Returns true if this sink is destroyed
bool HTTPConnectSink::writeComplete() {
  outstandingWrites_--;
  if (outstandingWrites_ == 0 && destroyOnWriteComplete_) {
    delete this;
    return true;
  }

  return false;
}

// WriteCallback methods
void HTTPConnectSink::writeSuccess() noexcept {
  bool destroyed = writeComplete();
  if (!destroyed) {
    // If we drop below the max outstanding writes, resume egress
    if (outstandingWrites_ < kMaxOutstandingWrites && handlerEgressPaused_ &&
        handler_) {
      handlerEgressPaused_ = false;
      handler_->onEgressResumed();
    }
    resetIdleTimeout();
  }
}

void HTTPConnectSink::writeErr(
    size_t, const folly::AsyncSocketException& err) noexcept {
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
