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
#include <utility>

#include <fizz/server/FizzServerContext.h>
#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzHandshakeHelper.h>
#include <thrift/lib/cpp2/fast_thrift/security/PendingHandshakes.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

namespace apache::thrift::fast_thrift::connection::handler {

// Middle handler that runs the Fizz TLS handshake on each accepted socket.
// Absorbs the inbound ConnectionMessage, starts an async handshake via
// PendingHandshakes + FizzHandshakeHelper, and re-fires the upgraded
// ConnectionMessage (transport = AsyncFizzServer) on success. Handshake
// failures are logged and the connection is dropped — matches the prior
// behavior of ConnectionHandler::connectionAccepted.
class FizzHandshakeHandler {
 public:
  FizzHandshakeHandler(
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::chrono::milliseconds handshakeTimeout) noexcept
      : fizzContext_(std::move(fizzContext)),
        thriftParams_(std::move(thriftParams)),
        handshakeTimeout_(handshakeTimeout) {}

  FizzHandshakeHandler(const FizzHandshakeHandler&) = delete;
  FizzHandshakeHandler& operator=(const FizzHandshakeHandler&) = delete;
  FizzHandshakeHandler(FizzHandshakeHandler&&) = delete;
  FizzHandshakeHandler& operator=(FizzHandshakeHandler&&) = delete;
  ~FizzHandshakeHandler() = default;

  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    ctx_ = &ctx;
  }

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    ctx_ = nullptr;
  }

  // === Inbound ===

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto incoming = msg.take<ConnectionMessage>();

    // We expect a plain AsyncSocket at this point. If the transport is
    // already negotiated (some other handler upstream wrapped it), forward
    // unchanged — TLS is not our concern for already-secured transports.
    auto* asyncSocket =
        dynamic_cast<folly::AsyncSocket*>(incoming.transport.get());
    if (FOLLY_UNLIKELY(asyncSocket == nullptr)) {
      return ctx.fireRead(channel_pipeline::erase_and_box(std::move(incoming)));
    }

    // Re-wrap the raw socket pointer in the type FizzHandshakeHelper
    // expects. Release from the AsyncTransport::UniquePtr and re-own in
    // an AsyncSocket::UniquePtr; both deleters are DelayedDestruction::
    // Destructor, so the lifetime contract is preserved.
    (void)incoming.transport.release();
    folly::AsyncSocket::UniquePtr socket(asyncSocket);

    auto clientAddr = std::move(incoming.clientAddr);

    security::PendingHandshakes::HelperPtr helper(
        new security::FizzHandshakeHelper(
            std::move(socket),
            fizzContext_,
            thriftParams_,
            handshakeTimeout_,
            pendingHandshakes_,
            [this, clientAddr](
                folly::AsyncTransport::UniquePtr negotiated,
                const folly::exception_wrapper& ex) noexcept {
              onHandshakeComplete(std::move(negotiated), ex, clientAddr);
            }));
    auto* raw = helper.get();
    pendingHandshakes_.add(std::move(helper));
    raw->start();

    // Message absorbed; downstream handlers will see it again only if the
    // handshake succeeds.
    return channel_pipeline::Result::Success;
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  // === Outbound (passthrough) ===

  template <typename Context>
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    // Drop any in-flight handshakes; their callbacks fire with a cancellation
    // exception and the helpers self-remove from pending_.
    pendingHandshakes_.cancelAll();
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  void onHandshakeComplete(
      folly::AsyncTransport::UniquePtr negotiated,
      const folly::exception_wrapper& ex,
      const folly::SocketAddress& clientAddr) noexcept {
    if (ex || !negotiated) {
      XLOG(DBG3) << "TLS handshake failed for " << clientAddr.describe() << ": "
                 << (ex ? ex.what().toStdString() : std::string("null"));
      return;
    }
    if (FOLLY_UNLIKELY(!ctx_)) {
      // handlerRemoved fired before this callback (pipeline tearing down).
      // The negotiated transport will be destroyed when this lambda returns.
      return;
    }
    ConnectionMessage upgraded{
        .transport = std::move(negotiated),
        .clientAddr = clientAddr,
    };
    auto result =
        ctx_->fireRead(channel_pipeline::erase_and_box(std::move(upgraded)));
    switch (result) {
      case channel_pipeline::Result::Success:
        return;
      case channel_pipeline::Result::Backpressure:
        XLOG_EVERY_MS(WARN, 1000)
            << "Downstream backpressure after TLS handshake from "
            << clientAddr.describe();
        return;
      case channel_pipeline::Result::Error:
        XLOG(WARN) << "Downstream rejected post-TLS connection from "
                   << clientAddr.describe();
        return;
    }
  }

  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext_;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams_;
  std::chrono::milliseconds handshakeTimeout_;
  security::PendingHandshakes pendingHandshakes_;
  channel_pipeline::detail::ContextImpl* ctx_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::handler
