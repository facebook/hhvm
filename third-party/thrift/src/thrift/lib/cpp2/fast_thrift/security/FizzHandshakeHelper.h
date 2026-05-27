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
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/security/HandshakeTimeout.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache::thrift::fast_thrift::security {

/**
 * Drives a single TLS handshake on an accepted socket using fizz. Pure
 * handshake only — post-handshake StopTLS is a separate concern handled by
 * `connection::security::util::StopTLSHelper`. Callers that want StopTLS
 * inspect the surfaced `ThriftParametersServerExtension` for
 * `getNegotiatedStopTLS*()` and chain `StopTLSHelper` themselves.
 *
 * Lifetime:
 *   - Constructed on the EventBase thread that owns the socket. Held by the
 *     owner (typically the FizzHandshakeHandler) via a unique_ptr with a
 *     folly::DelayedDestruction::Destructor deleter.
 *   - start() kicks off the handshake.
 *   - On success/error/timeout, invokes the user callback exactly once and
 *     then synchronously invokes onTerminal(this) so the owner can drop its
 *     unique_ptr. The deleter triggers destroy(); DelayedDestruction defers
 *     the actual delete until the surrounding DestructorGuard pops.
 *   - cancel() drives the same path synchronously during shutdown by closing
 *     the AsyncFizzServer; fizz routes through fizzHandshakeError → finish().
 *     The user callback receives a cancellation exception_wrapper.
 */
class FizzHandshakeHelper
    : private fizz::server::AsyncFizzServer::HandshakeCallback,
      private folly::AsyncTimeout,
      public folly::DelayedDestruction {
 public:
  // Owning pointer with the DelayedDestruction-aware deleter. Matches the
  // folly convention (e.g. AsyncSocket::UniquePtr).
  using UniquePtr = std::
      unique_ptr<FizzHandshakeHelper, folly::DelayedDestruction::Destructor>;

  // Invoked on terminal handshake state.
  // On success: `transport` is the negotiated AsyncFizzServer; `extension`
  // is non-null iff the helper was constructed with thriftParams and carries
  // the negotiation results (StopTLS V1/V2, TTLSTunnel, compression, PSP).
  // On error: `transport` is null and `ex` carries the reason.
  using Callback = folly::Function<void(
      fizz::server::AsyncFizzServer::UniquePtr,
      std::shared_ptr<apache::thrift::ThriftParametersServerExtension>,
      folly::exception_wrapper) noexcept>;

  // Fires exactly once after the user Callback, on the EventBase thread.
  // Receives `this`; owner uses it to drop its owning UniquePtr.
  using OnTerminal = folly::Function<void(FizzHandshakeHelper*) noexcept>;

  FizzHandshakeHelper(
      folly::AsyncSocket::UniquePtr socket,
      std::shared_ptr<const fizz::server::FizzServerContext> context,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      HandshakeTimeout timeout,
      OnTerminal onTerminal,
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
  // and fire onTerminal(this). Mirrors wangle's AcceptorHandshakeHelper::
  // dropConnection contract.
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

  // folly::AsyncTimeout
  void timeoutExpired() noexcept override;

  // Invokes the user callback (once), then synchronously invokes
  // onTerminal(this) so the owner can drop its unique_ptr. All callers must
  // hold a DestructorGuard on `this` because onTerminal typically destroys
  // `this` via the unique_ptr deleter.
  void finish(
      fizz::server::AsyncFizzServer::UniquePtr transport,
      folly::exception_wrapper ex) noexcept;

  fizz::server::AsyncFizzServer::UniquePtr fizzServer_;
  std::shared_ptr<apache::thrift::ThriftParametersServerExtension> extension_;
  folly::EventBase* evb_;
  OnTerminal onTerminal_;
  Callback callback_;
  HandshakeTimeout timeout_;
  bool done_{false};
};

} // namespace apache::thrift::fast_thrift::security
