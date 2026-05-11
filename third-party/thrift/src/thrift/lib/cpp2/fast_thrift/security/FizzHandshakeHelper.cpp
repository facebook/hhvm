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

#include <thrift/lib/cpp2/fast_thrift/security/FizzHandshakeHelper.h>

#include <folly/io/async/AsyncSocketException.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/security/PendingHandshakes.h>

namespace apache::thrift::fast_thrift::security {

FizzHandshakeHelper::FizzHandshakeHelper(
    folly::AsyncSocket::UniquePtr socket,
    std::shared_ptr<const fizz::server::FizzServerContext> context,
    std::chrono::milliseconds timeout,
    PendingHandshakes& pending,
    Callback callback)
    : folly::AsyncTimeout(socket->getEventBase()),
      fizzServer_(new fizz::server::AsyncFizzServer(
          folly::AsyncTransport::UniquePtr(socket.release()), context)),
      evb_(fizzServer_->getEventBase()),
      pending_(pending),
      callback_(std::move(callback)),
      timeout_(timeout) {}

FizzHandshakeHelper::~FizzHandshakeHelper() {
  cancelTimeout();
  // Suppress any synchronous fizzHandshakeError that closeNow may fire
  // during member destruction.
  done_ = true;
  if (fizzServer_) {
    fizzServer_->closeNow();
  }
}

void FizzHandshakeHelper::start() {
  scheduleTimeout(timeout_);
  fizzServer_->accept(this);
}

void FizzHandshakeHelper::fizzHandshakeSuccess(
    fizz::server::AsyncFizzServer* /*transport*/) noexcept {
  DestructorGuard guard(this);
  cancelTimeout();
  finish(
      folly::AsyncTransport::UniquePtr(fizzServer_.release()),
      folly::exception_wrapper());
}

void FizzHandshakeHelper::fizzHandshakeError(
    fizz::server::AsyncFizzServer* /*transport*/,
    folly::exception_wrapper ex) noexcept {
  DestructorGuard guard(this);
  cancelTimeout();
  XLOG(DBG3) << "Fizz handshake failed: " << ex.what();
  finish(nullptr, std::move(ex));
}

void FizzHandshakeHelper::fizzHandshakeAttemptFallback(
    fizz::server::AttemptVersionFallback /*fallback*/) {
  // fast_thrift does not support TLS<1.3 fallback. Treat as a handshake
  // error and close the connection.
  DestructorGuard guard(this);
  cancelTimeout();
  finish(
      nullptr,
      folly::make_exception_wrapper<folly::AsyncSocketException>(
          folly::AsyncSocketException::SSL_ERROR,
          "TLS fallback to legacy protocol not supported"));
}

void FizzHandshakeHelper::timeoutExpired() noexcept {
  DestructorGuard guard(this);
  XLOG(DBG3) << "Fizz handshake timed out";
  // Deliver our timeout error first; closeNow may synchronously fire
  // fizzHandshakeError, but finish() short-circuits via done_.
  finish(
      nullptr,
      folly::make_exception_wrapper<folly::AsyncSocketException>(
          folly::AsyncSocketException::TIMED_OUT, "TLS handshake timed out"));
  if (fizzServer_) {
    fizzServer_->closeNow();
  }
}

void FizzHandshakeHelper::cancel() {
  DestructorGuard guard(this);
  // Synchronous-terminal-callback contract (mirrors wangle's
  // AcceptorHandshakeHelper::dropConnection): closeNow drives fizz to fire
  // fizzHandshakeError on this same stack, which routes through finish() and
  // synchronously removes us from PendingHandshakes.
  if (fizzServer_) {
    fizzServer_->closeNow();
  }
  // Belt-and-suspenders: if fizz didn't fire the callback synchronously
  // (e.g. handshake already settled but the terminal callback hasn't been
  // delivered), drive finish() directly so the caller's contract holds.
  if (!done_) {
    finish(
        nullptr,
        folly::make_exception_wrapper<folly::AsyncSocketException>(
            folly::AsyncSocketException::END_OF_FILE,
            "TLS handshake cancelled by server shutdown"));
  }
}

void FizzHandshakeHelper::finish(
    folly::AsyncTransport::UniquePtr transport,
    folly::exception_wrapper ex) noexcept {
  if (done_) {
    return;
  }
  done_ = true;
  if (callback_) {
    callback_(std::move(transport), std::move(ex));
  }
  // Inline self-removal. Every entry point that lands here first takes a
  // DestructorGuard on `this`, so DelayedDestruction defers the actual
  // delete (triggered by HelperPtr's deleter) until that guard pops.
  pending_.remove(this);
}

} // namespace apache::thrift::fast_thrift::security
