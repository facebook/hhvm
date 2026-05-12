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
#include <memory>

#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/FizzServerContext.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/security/AsyncStopTLS.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache::thrift::fast_thrift::security {

class PendingHandshakes;

/**
 * Drives a single TLS handshake on an accepted socket using fizz.
 *
 * Lifetime:
 *   - Constructed on the EventBase thread that owns the socket. Held by
 *     PendingHandshakes via PendingHandshakes::HelperPtr (a unique_ptr with
 *     a folly::DelayedDestruction::Destructor deleter). Use that alias to
 *     construct, e.g. `PendingHandshakes::HelperPtr p(new
 * FizzHandshakeHelper(...));`.
 *   - start() kicks off the handshake; the helper lives in PendingHandshakes
 *     until completion.
 *   - On success/error/timeout, invokes the user callback exactly once and
 *     synchronously removes itself from PendingHandshakes. Removal triggers
 *     destroy() through the unique_ptr deleter; DelayedDestruction defers
 *     the actual delete until the surrounding DestructorGuard pops.
 *   - PendingHandshakes::cancelAll() drives the same path during shutdown
 *     by calling cancel() on each helper, which closes the AsyncFizzServer
 *     and routes through fizzHandshakeError → finish() synchronously. The
 *     user callback receives a cancellation exception_wrapper.
 */
class FizzHandshakeHelper
    : private fizz::server::AsyncFizzServer::HandshakeCallback,
      private apache::thrift::AsyncStopTLS::Callback,
      private folly::AsyncTimeout,
      public folly::DelayedDestruction {
 public:
  // Invoked with the negotiated transport on success, or `nullptr` plus the
  // failure reason on error. The transport may be a
  // fizz::server::AsyncFizzServer (encrypted), or — when StopTLS V1 was
  // negotiated — a plaintext folly::AsyncSocketTransport with peer cert info
  // preserved.
  using Callback = folly::Function<void(
      folly::AsyncTransport::UniquePtr, folly::exception_wrapper) noexcept>;

  FizzHandshakeHelper(
      folly::AsyncSocket::UniquePtr socket,
      std::shared_ptr<const fizz::server::FizzServerContext> context,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::chrono::milliseconds timeout,
      PendingHandshakes& pending,
      Callback callback);

  FizzHandshakeHelper(const FizzHandshakeHelper&) = delete;
  FizzHandshakeHelper& operator=(const FizzHandshakeHelper&) = delete;
  FizzHandshakeHelper(FizzHandshakeHelper&&) = delete;
  FizzHandshakeHelper& operator=(FizzHandshakeHelper&&) = delete;

  // Begins the handshake. Must be called on the EventBase thread.
  void start();

  // Synchronously aborts the handshake. Closes the underlying AsyncFizzServer,
  // which causes fizz to fire fizzHandshakeError on this same stack, routing
  // through finish() to invoke the user callback with a cancellation error
  // and remove the helper from PendingHandshakes. Mirrors wangle's
  // AcceptorHandshakeHelper::dropConnection contract: a terminal callback
  // must fire synchronously with respect to the cancel() call.
  void cancel();

 protected:
  // DelayedDestruction requires a non-public destructor; deletion happens via
  // destroy() through the unique_ptr deleter.
  ~FizzHandshakeHelper() override;

 private:
  // fizz::server::AsyncFizzServer::HandshakeCallback
  void fizzHandshakeSuccess(
      fizz::server::AsyncFizzServer* transport) noexcept override;
  void fizzHandshakeError(
      fizz::server::AsyncFizzServer* transport,
      folly::exception_wrapper ex) noexcept override;
  void fizzHandshakeAttemptFallback(
      fizz::server::AttemptVersionFallback fallback) override;

  // apache::thrift::AsyncStopTLS::Callback
  void stopTLSSuccess(std::unique_ptr<folly::IOBuf> postTLSData) override;
  void stopTLSError(const folly::exception_wrapper& ex) override;

  // folly::AsyncTimeout
  void timeoutExpired() noexcept override;

  // Invokes the user callback (once) and synchronously removes self from
  // PendingHandshakes. All callers must hold a DestructorGuard on `this`
  // because remove() drops the helper's HelperPtr, which calls destroy().
  void finish(
      folly::AsyncTransport::UniquePtr transport,
      folly::exception_wrapper ex) noexcept;

  fizz::server::AsyncFizzServer::UniquePtr fizzServer_;
  std::shared_ptr<apache::thrift::ThriftParametersServerExtension> extension_;
  apache::thrift::AsyncStopTLS::UniquePtr stopTlsFrame_;
  folly::EventBase* evb_;
  PendingHandshakes& pending_;
  Callback callback_;
  std::chrono::milliseconds timeout_;
  bool done_{false};
};

} // namespace apache::thrift::fast_thrift::security
