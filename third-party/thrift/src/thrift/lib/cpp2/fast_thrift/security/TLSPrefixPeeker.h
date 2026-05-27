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

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp2/fast_thrift/security/HandshakeTimeout.h>

namespace apache::thrift::fast_thrift::security {

/**
 * Peeks a fixed-size prefix from a freshly-accepted socket and reports
 * whether it looks like a TLS ClientHello (via
 * apache::thrift::TLSHelper::looksLikeTLS; requires kTLSPeekBytes = 9
 * bytes, matching wangle's TLSPlaintextPeekingCallback).
 *
 * Peeked bytes are returned via the callback alongside the TLS/not-TLS
 * bit; the caller is responsible for replaying them via
 * folly::AsyncSocket::setPreReceivedData before any subsequent reader
 * observes the socket.
 *
 * Held by the owner (typically TLSClassifier) via a unique_ptr with a
 * DelayedDestruction::Destructor deleter; finish() fires onTerminal(this)
 * synchronously so the owner can drop its unique_ptr.
 * DelayedDestruction defers the actual delete behind any live
 * DestructorGuard.
 */
class TLSPrefixPeeker : private folly::AsyncReader::ReadCallback,
                        private folly::AsyncTimeout,
                        public folly::DelayedDestruction {
 public:
  // Owning pointer with the DelayedDestruction-aware deleter. Matches the
  // folly convention (e.g. AsyncSocket::UniquePtr).
  using UniquePtr =
      std::unique_ptr<TLSPrefixPeeker, folly::DelayedDestruction::Destructor>;

  // On the success path, socket is non-null, peekedBytes carries the bytes
  // already consumed off the wire (caller must replay), and looksLikeTLS
  // tells the caller which branch to take. On error, socket is null and
  // ex carries the reason; the underlying fd is already closed.
  using Callback = folly::Function<void(
      folly::AsyncSocket::UniquePtr socket,
      std::unique_ptr<folly::IOBuf> peekedBytes,
      bool looksLikeTLS,
      folly::exception_wrapper ex) noexcept>;

  // Fires exactly once on the EventBase thread, before the user Callback
  // (mirrors FizzHandshakeHelper's terminal ordering). Receives `this`;
  // owner uses it to drop its owning UniquePtr.
  using OnTerminal = folly::Function<void(TLSPrefixPeeker*) noexcept>;

  TLSPrefixPeeker(
      folly::AsyncSocket::UniquePtr socket,
      HandshakeTimeout timeout,
      OnTerminal onTerminal,
      Callback callback);

  TLSPrefixPeeker(const TLSPrefixPeeker&) = delete;
  TLSPrefixPeeker& operator=(const TLSPrefixPeeker&) = delete;
  TLSPrefixPeeker(TLSPrefixPeeker&&) = delete;
  TLSPrefixPeeker& operator=(TLSPrefixPeeker&&) = delete;

  // Begins the peek read. Must be called on the EventBase thread that owns
  // the socket.
  void start();

  // Synchronously aborts the peek, fires the callback with a cancellation
  // error, and invokes onTerminal(this). Mirrors FizzHandshakeHelper::cancel.
  void cancel();

 protected:
  ~TLSPrefixPeeker() override;

 private:
  // folly::AsyncReader::ReadCallback
  void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  void readDataAvailable(size_t len) noexcept override;
  void readEOF() noexcept override;
  void readErr(const folly::AsyncSocketException& ex) noexcept override;

  // folly::AsyncTimeout
  void timeoutExpired() noexcept override;

  void finish(bool looksLikeTLS, folly::exception_wrapper ex) noexcept;

  folly::AsyncSocket::UniquePtr socket_;
  std::vector<uint8_t> buf_;
  size_t bytesRead_{0};
  OnTerminal onTerminal_;
  Callback callback_;
  HandshakeTimeout timeout_;
  bool done_{false};
};

} // namespace apache::thrift::fast_thrift::security
