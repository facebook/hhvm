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
#include <optional>
#include <utility>

#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/FizzServerContext.h>
#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/container/F14Map.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzHandshakeHelper.h>
#include <thrift/lib/cpp2/fast_thrift/security/HandshakeTimeout.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>

namespace apache::thrift::fast_thrift::connection::security::handler {

/**
 * Inner-pipeline middle handler that runs the Fizz TLS handshake on each
 * accepted socket. Wraps FizzHandshakeHelper; tracks in-flight helpers in
 * inFlight_ and cancels them on onPipelineInactive. Upgrades the
 * TLSPipelineMessage transport from AsyncSocket → AsyncFizzServer and
 * populates `extension` so downstream stages (StopTLSV1Handler etc.) can
 * query the negotiated parameters. Handshake failures are logged at DBG3
 * and the connection is dropped — message is absorbed.
 */
class FizzHandshakeHandler {
 public:
  // handshakeTimeout is the per-connection deadline shared with any upstream
  // peek phase. std::nullopt = unbounded. A fresh HandshakeTimeout is
  // constructed per connection in onRead so each one captures its own
  // absolute deadline at handshake start.
  FizzHandshakeHandler(
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::optional<std::chrono::milliseconds> handshakeTimeout) noexcept
      : fizzContext_(std::move(fizzContext)),
        thriftParams_(std::move(thriftParams)),
        handshakeTimeout_(handshakeTimeout) {}

  ~FizzHandshakeHandler() = default;
  FizzHandshakeHandler(const FizzHandshakeHandler&) = delete;
  FizzHandshakeHandler& operator=(const FizzHandshakeHandler&) = delete;
  FizzHandshakeHandler(FizzHandshakeHandler&&) = delete;
  FizzHandshakeHandler& operator=(FizzHandshakeHandler&&) = delete;

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
    auto incoming = msg.take<TLSPipelineMessage>();

    // We expect a plain AsyncSocket at this point. If the transport is
    // already negotiated (some other handler upstream wrapped it), forward
    // unchanged — TLS is not our concern for already-secured transports.
    auto* asyncSocket =
        dynamic_cast<folly::AsyncSocket*>(incoming.transport.get());
    if (FOLLY_UNLIKELY(asyncSocket == nullptr)) {
      return ctx.fireRead(channel_pipeline::erase_and_box(std::move(incoming)));
    }

    (void)incoming.transport.release();
    folly::AsyncSocket::UniquePtr socket(asyncSocket);
    auto clientAddr = std::move(incoming.clientAddr);

    apache::thrift::fast_thrift::security::FizzHandshakeHelper::UniquePtr
        helper(new apache::thrift::fast_thrift::security::FizzHandshakeHelper(
            std::move(socket),
            fizzContext_,
            thriftParams_,
            apache::thrift::fast_thrift::security::HandshakeTimeout{
                handshakeTimeout_},
            [this](
                apache::thrift::fast_thrift::security::FizzHandshakeHelper*
                    h) noexcept { inFlight_.erase(h); },
            [this, clientAddr](
                fizz::server::AsyncFizzServer::UniquePtr fizzServer,
                std::shared_ptr<apache::thrift::ThriftParametersServerExtension>
                    extension,
                const folly::exception_wrapper& ex) noexcept {
              onHandshakeComplete(
                  std::move(fizzServer), std::move(extension), ex, clientAddr);
            }));
    auto* raw = helper.get();
    inFlight_.emplace(raw, std::move(helper));
    raw->start();

    // Message absorbed; downstream handlers see it again only on success.
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
    // Drain into a local map so each helper's synchronous self-remove via
    // onTerminal (inFlight_.erase) doesn't perturb iteration.
    auto drained = std::move(inFlight_);
    for (auto& [_, helper] : drained) {
      folly::DelayedDestruction::DestructorGuard guard(helper.get());
      helper->cancel();
    }
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  void onHandshakeComplete(
      fizz::server::AsyncFizzServer::UniquePtr fizzServer,
      std::shared_ptr<apache::thrift::ThriftParametersServerExtension>
          extension,
      const folly::exception_wrapper& ex,
      const folly::SocketAddress& clientAddr) noexcept {
    if (ex || !fizzServer) {
      XLOG(DBG3) << "TLS handshake failed for " << clientAddr.describe() << ": "
                 << (ex ? ex.what().toStdString() : std::string("null"));
      return;
    }
    if (FOLLY_UNLIKELY(!ctx_)) {
      // handlerRemoved fired before this callback (pipeline tearing down).
      return;
    }
    TLSPipelineMessage upgraded{
        .transport = folly::AsyncTransport::UniquePtr(fizzServer.release()),
        .clientAddr = clientAddr,
        .extension = std::move(extension),
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
  std::optional<std::chrono::milliseconds> handshakeTimeout_;
  folly::F14FastMap<
      apache::thrift::fast_thrift::security::FizzHandshakeHelper*,
      apache::thrift::fast_thrift::security::FizzHandshakeHelper::UniquePtr>
      inFlight_;
  channel_pipeline::detail::ContextImpl* ctx_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::security::handler
