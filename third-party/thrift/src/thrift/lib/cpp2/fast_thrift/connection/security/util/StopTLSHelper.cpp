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

#include <thrift/lib/cpp2/fast_thrift/connection/security/util/StopTLSHelper.h>

#include <chrono>

#include <folly/io/async/AsyncSocketException.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/security/SSLUtil.h>

namespace apache::thrift::fast_thrift::connection::security::util {

StopTLSHelper::StopTLSHelper(
    fizz::server::AsyncFizzServer::UniquePtr fizzServer,
    OnTerminal onTerminal,
    Callback callback)
    : fizzServer_(std::move(fizzServer)),
      onTerminal_(std::move(onTerminal)),
      callback_(std::move(callback)) {}

StopTLSHelper::~StopTLSHelper() {
  // Suppress any synchronous stopTLSError that closeNow may fire during
  // member destruction.
  done_ = true;
  if (fizzServer_) {
    fizzServer_->closeNow();
  }
}

void StopTLSHelper::start() {
  // Guard against synchronous self-destruction via finish() →
  // onTerminal_(this) → destroy(); without a guard, `this` could be deleted
  // while we are still on the stack of start().
  DestructorGuard guard(this);

  XLOG(DBG3) << "Beginning StopTLS V1 negotiation";
  stopTlsFrame_.reset(new apache::thrift::AsyncStopTLS(*this));
  // 0ms = no internal timeout. Matches legacy FizzPeeker.cpp convention;
  // shutdown lifetime is managed by the owner via cancel().
  stopTlsFrame_->start(
      fizzServer_.get(),
      apache::thrift::AsyncStopTLS::Role::Server,
      std::chrono::milliseconds{0});
}

void StopTLSHelper::stopTLSSuccess(std::unique_ptr<folly::IOBuf> postTLSData) {
  // finish() invokes onTerminal_(this), which typically drops the owner's
  // unique_ptr and calls destroy(); without a guard, that delete fires while
  // we are still on this stack. See header comment on finish().
  DestructorGuard guard(this);
  // Re-wrap the same FD as a plaintext socket and push back any residual
  // bytes the peer sent immediately after their close_notify.
  auto plaintext = apache::thrift::toFDSocket(
      fizzServer_.get(), apache::thrift::kSecurityProtocolStopTLS);
  if (postTLSData) {
    plaintext->setPreReceivedData(std::move(postTLSData));
  }
  plaintext->cacheAddresses();
  fizzServer_.reset();
  finish(
      folly::AsyncTransport::UniquePtr(plaintext.release()),
      folly::exception_wrapper());
}

void StopTLSHelper::stopTLSError(const folly::exception_wrapper& ex) {
  DestructorGuard guard(this);
  XLOG(DBG3) << "StopTLS failed: " << ex.what();
  finish(nullptr, ex);
}

void StopTLSHelper::cancel() {
  DestructorGuard guard(this);
  // Synchronous-terminal-callback contract (mirrors FizzHandshakeHelper::
  // cancel): closeNow drives fizz to fire readErr → stopTLSError on this
  // same stack, which routes through finish() and synchronously fires
  // onTerminal_(this).
  if (fizzServer_) {
    fizzServer_->closeNow();
  }
  // Belt-and-suspenders: if fizz didn't fire the callback synchronously
  // (e.g. start() wasn't called yet, or the terminal callback hasn't been
  // delivered), drive finish() directly so the caller's contract holds.
  if (!done_) {
    finish(
        nullptr,
        folly::make_exception_wrapper<folly::AsyncSocketException>(
            folly::AsyncSocketException::END_OF_FILE,
            "StopTLS cancelled by server shutdown"));
  }
}

void StopTLSHelper::finish(
    folly::AsyncTransport::UniquePtr transport,
    folly::exception_wrapper ex) noexcept {
  if (done_) {
    return;
  }
  done_ = true;
  if (callback_) {
    callback_(std::move(transport), std::move(ex));
  }
  // User callback first, then signal the owner. Every entry point that lands
  // here first takes a DestructorGuard on `this`, so DelayedDestruction
  // defers the actual delete (triggered by the owner's unique_ptr deleter)
  // until that guard pops.
  if (onTerminal_) {
    onTerminal_(this);
  }
}

} // namespace apache::thrift::fast_thrift::connection::security::util
