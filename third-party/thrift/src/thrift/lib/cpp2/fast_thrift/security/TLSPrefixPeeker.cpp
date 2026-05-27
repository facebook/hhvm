/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/fast_thrift/security/TLSPrefixPeeker.h>

#include <folly/io/IOBuf.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/server/peeking/TLSHelper.h>

namespace apache::thrift::fast_thrift::security {

TLSPrefixPeeker::TLSPrefixPeeker(
    folly::AsyncSocket::UniquePtr socket,
    HandshakeTimeout timeout,
    OnTerminal onTerminal,
    Callback callback)
    : folly::AsyncTimeout(socket->getEventBase()),
      socket_(std::move(socket)),
      buf_(apache::thrift::kTLSPeekBytes),
      onTerminal_(std::move(onTerminal)),
      callback_(std::move(callback)),
      timeout_(timeout) {}

TLSPrefixPeeker::~TLSPrefixPeeker() {
  cancelTimeout();
  // Suppress any synchronous read callback closeNow may fire during member
  // destruction — finish() short-circuits via done_.
  done_ = true;
  if (socket_) {
    socket_->setReadCB(nullptr);
    socket_->closeNow();
  }
}

void TLSPrefixPeeker::start() {
  // Hold a guard because the fail-fast branch synchronously calls
  // finish() → onTerminal_(this) → destroy(); without a guard,
  // `this` could be deleted while we are still on the stack of start().
  DestructorGuard guard(this);

  if (timeout_.hasExpired()) {
    XLOG(DBG3) << "TLS peek: handshake budget already exhausted at start";
    finish(
        /*looksLikeTLS=*/false,
        folly::make_exception_wrapper<folly::AsyncSocketException>(
            folly::AsyncSocketException::TIMED_OUT,
            "handshake budget already exhausted before TLS peek"));
    return;
  }
  // No-op when timeout_ has no deadline (unbounded) or when the deadline
  // has just passed in the microseconds since hasExpired() returned false.
  // The latter race window means "no timer enforcement" — acceptable; the
  // hasExpired() check above is the primary fail-fast mechanism.
  timeout_.schedule(*this);
  socket_->setReadCB(this);
}

void TLSPrefixPeeker::cancel() {
  DestructorGuard guard(this);
  finish(
      /*looksLikeTLS=*/false,
      folly::make_exception_wrapper<folly::AsyncSocketException>(
          folly::AsyncSocketException::END_OF_FILE,
          "TLS peek cancelled by server shutdown"));
}

void TLSPrefixPeeker::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  *bufReturn = buf_.data() + bytesRead_;
  *lenReturn = buf_.size() - bytesRead_;
}

void TLSPrefixPeeker::readDataAvailable(size_t len) noexcept {
  DestructorGuard guard(this);
  bytesRead_ += len;
  if (bytesRead_ < buf_.size()) {
    return;
  }
  bool isTls = apache::thrift::TLSHelper::looksLikeTLS(buf_);
  finish(isTls, /*ex=*/{});
}

void TLSPrefixPeeker::readEOF() noexcept {
  DestructorGuard guard(this);
  XLOG(DBG3) << "TLS peek: peer closed before " << buf_.size()
             << " bytes received";
  finish(
      /*looksLikeTLS=*/false,
      folly::make_exception_wrapper<folly::AsyncSocketException>(
          folly::AsyncSocketException::END_OF_FILE,
          "peer closed before TLS peek completed"));
}

void TLSPrefixPeeker::readErr(const folly::AsyncSocketException& ex) noexcept {
  DestructorGuard guard(this);
  XLOG(DBG3) << "TLS peek read error: " << ex.what();
  finish(
      /*looksLikeTLS=*/false,
      folly::make_exception_wrapper<folly::AsyncSocketException>(ex));
}

void TLSPrefixPeeker::timeoutExpired() noexcept {
  DestructorGuard guard(this);
  XLOG(DBG3) << "TLS peek timed out after " << bytesRead_ << "/" << buf_.size()
             << " bytes";
  finish(
      /*looksLikeTLS=*/false,
      folly::make_exception_wrapper<folly::AsyncSocketException>(
          folly::AsyncSocketException::TIMED_OUT, "TLS peek timed out"));
}

void TLSPrefixPeeker::finish(bool isTls, folly::exception_wrapper ex) noexcept {
  if (done_) {
    return;
  }
  done_ = true;
  cancelTimeout();
  if (socket_) {
    socket_->setReadCB(nullptr);
  }

  folly::AsyncSocket::UniquePtr s;
  std::unique_ptr<folly::IOBuf> peeked;
  if (!ex) {
    s = std::move(socket_);
    peeked = folly::IOBuf::copyBuffer(buf_.data(), bytesRead_);
  } else if (socket_) {
    socket_->closeNow();
    socket_.reset();
  }

  auto cb = std::move(callback_);
  // Signal owner first (mirrors current ordering: owner drops the
  // unique_ptr → destroy() is deferred behind the caller's DestructorGuard,
  // so the user callback still runs safely on this stack).
  if (onTerminal_) {
    onTerminal_(this);
  }
  if (cb) {
    cb(std::move(s), std::move(peeked), isTls, std::move(ex));
  }
}

} // namespace apache::thrift::fast_thrift::security
