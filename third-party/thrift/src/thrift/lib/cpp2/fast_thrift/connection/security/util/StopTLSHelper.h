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

#include <memory>

#include <fizz/server/AsyncFizzServer.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp2/security/AsyncStopTLS.h>

namespace apache::thrift::fast_thrift::connection::security::util {

/**
 * Drives a single StopTLS V1 exchange on an AsyncFizzServer whose handshake
 * has already completed and that negotiated StopTLS V1. On success, produces
 * a plaintext folly::AsyncSocketTransport with peer cert info preserved.
 *
 * Lifetime / cancellation contract mirrors FizzHandshakeHelper:
 *   - Constructed on the EventBase thread that owns the fizz transport. Held
 *     by the owner (typically StopTLSV1Handler) via a unique_ptr with a
 *     folly::DelayedDestruction::Destructor deleter.
 *   - start() kicks off the StopTLS exchange.
 *   - On success/error, invokes the user Callback exactly once and then
 *     synchronously invokes onTerminal(this) so the owner can drop its
 *     unique_ptr. The deleter triggers destroy(); DelayedDestruction defers
 *     the actual delete until the surrounding DestructorGuard pops.
 *   - cancel() drives the same path synchronously during shutdown by closing
 *     the AsyncFizzServer; fizz routes through readErr → stopTLSError →
 *     finish(). The user Callback receives a cancellation exception_wrapper.
 */
class StopTLSHelper : private apache::thrift::AsyncStopTLS::Callback,
                      public folly::DelayedDestruction {
 public:
  using UniquePtr =
      std::unique_ptr<StopTLSHelper, folly::DelayedDestruction::Destructor>;

  // On success: plaintext transport (fd re-wrapped, post-TLS data replayed).
  // On error: nullptr + reason.
  using Callback = folly::Function<void(
      folly::AsyncTransport::UniquePtr, folly::exception_wrapper) noexcept>;

  // Fires exactly once after the user Callback, on the EventBase thread.
  // Receives `this`; owner uses it to drop its owning UniquePtr.
  using OnTerminal = folly::Function<void(StopTLSHelper*) noexcept>;

  StopTLSHelper(
      fizz::server::AsyncFizzServer::UniquePtr fizzServer,
      OnTerminal onTerminal,
      Callback callback);

  StopTLSHelper(const StopTLSHelper&) = delete;
  StopTLSHelper& operator=(const StopTLSHelper&) = delete;
  StopTLSHelper(StopTLSHelper&&) = delete;
  StopTLSHelper& operator=(StopTLSHelper&&) = delete;

  // Begins the StopTLS exchange. Must be called on the EventBase thread that
  // owns the fizz transport.
  void start();

  // Synchronously aborts. Closes the AsyncFizzServer, which routes through
  // readErr → stopTLSError → finish() and fires onTerminal(this).
  void cancel();

 protected:
  // DelayedDestruction requires a non-public destructor; deletion happens via
  // destroy() through the unique_ptr deleter.
  ~StopTLSHelper() override;

 private:
  // apache::thrift::AsyncStopTLS::Callback
  void stopTLSSuccess(std::unique_ptr<folly::IOBuf> postTLSData) override;
  void stopTLSError(const folly::exception_wrapper& ex) override;

  // Invokes the user callback (once), then synchronously invokes
  // onTerminal(this) so the owner can drop its unique_ptr. All callers must
  // hold a DestructorGuard on `this` because onTerminal typically destroys
  // `this` via the unique_ptr deleter.
  void finish(
      folly::AsyncTransport::UniquePtr transport,
      folly::exception_wrapper ex) noexcept;

  fizz::server::AsyncFizzServer::UniquePtr fizzServer_;
  apache::thrift::AsyncStopTLS::UniquePtr stopTlsFrame_;
  OnTerminal onTerminal_;
  Callback callback_;
  bool done_{false};
};

} // namespace apache::thrift::fast_thrift::connection::security::util
