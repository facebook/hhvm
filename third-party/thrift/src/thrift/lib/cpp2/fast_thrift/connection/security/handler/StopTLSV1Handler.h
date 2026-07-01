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

#include <utility>

#include <fizz/server/AsyncFizzServer.h>
#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/container/F14Map.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/util/StopTLSHelper.h>

namespace apache::thrift::fast_thrift::connection::security::handler {

/**
 * Inner-pipeline middle handler that performs StopTLS V1 downgrade on
 * connections whose handshake negotiated it. Stateless apart from its
 * in-flight helper map: the per-connection decision lives entirely on the
 * incoming TLSRequestMessage::extension.
 *
 * onWrite (work path):
 *   - `extension` null OR `getNegotiatedStopTLS()` false → passthrough.
 *   - Otherwise: take ownership of the AsyncFizzServer transport, spawn
 *     a StopTLSHelper, and on success refire the message with the
 *     downgraded plaintext transport (extension preserved so callers
 *     retain access to negotiated parameters).
 *
 * StopTLS failures are logged at DBG3 and the connection is dropped.
 */
class StopTLSV1Handler {
 public:
  StopTLSV1Handler() = default;
  ~StopTLSV1Handler() = default;

  StopTLSV1Handler(const StopTLSV1Handler&) = delete;
  StopTLSV1Handler& operator=(const StopTLSV1Handler&) = delete;
  StopTLSV1Handler(StopTLSV1Handler&&) = delete;
  StopTLSV1Handler& operator=(StopTLSV1Handler&&) = delete;

  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    ctx_ = &ctx;
  }

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    ctx_ = nullptr;
  }

  // === Outbound (work path) ===

  template <typename Context>
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto incoming = msg.take<TLSRequestMessage>();

    // No extension or StopTLS V1 not negotiated → passthrough.
    if (!incoming.extension || !incoming.extension->getNegotiatedStopTLS()) {
      return ctx.fireWrite(
          channel_pipeline::erase_and_box(std::move(incoming)));
    }

    // Defensive: extension says V1 but transport isn't fizz. Forward as-is
    // and let downstream reject if it must — we don't have a fizz session
    // to drive StopTLS against.
    auto* fizzPtr =
        dynamic_cast<fizz::server::AsyncFizzServer*>(incoming.transport.get());
    if (FOLLY_UNLIKELY(fizzPtr == nullptr)) {
      XLOG(DBG3)
          << "StopTLS V1 negotiated but transport is not AsyncFizzServer "
             "for "
          << incoming.clientAddr.describe();
      return ctx.fireWrite(
          channel_pipeline::erase_and_box(std::move(incoming)));
    }

    (void)incoming.transport.release();
    fizz::server::AsyncFizzServer::UniquePtr fizzServer(fizzPtr);
    auto clientAddr = std::move(incoming.clientAddr);
    auto extension = std::move(incoming.extension);

    util::StopTLSHelper::UniquePtr helper(new util::StopTLSHelper(
        std::move(fizzServer),
        [this](util::StopTLSHelper* h) noexcept { inFlight_.erase(h); },
        [this, clientAddr, extension](
            folly::AsyncTransport::UniquePtr plaintext,
            const folly::exception_wrapper& ex) noexcept {
          onStopTLSComplete(std::move(plaintext), ex, clientAddr, extension);
        }));
    auto* raw = helper.get();
    inFlight_.emplace(raw, std::move(helper));
    raw->start();

    // Message absorbed; downstream sees it again only on success.
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

  // === Inbound (passthrough) ===

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    auto drained = std::move(inFlight_);
    for (auto& [_, helper] : drained) {
      folly::DelayedDestruction::DestructorGuard guard(helper.get());
      helper->cancel();
    }
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  void onStopTLSComplete(
      folly::AsyncTransport::UniquePtr plaintext,
      const folly::exception_wrapper& ex,
      const folly::SocketAddress& clientAddr,
      std::shared_ptr<apache::thrift::ThriftParametersServerExtension>
          extension) noexcept {
    if (ex || !plaintext) {
      XLOG(DBG3) << "StopTLS V1 failed for " << clientAddr.describe() << ": "
                 << (ex ? ex.what().toStdString() : std::string("null"));
      return;
    }
    if (FOLLY_UNLIKELY(!ctx_)) {
      return;
    }
    TLSRequestMessage downgraded{
        .transport = std::move(plaintext),
        .clientAddr = clientAddr,
        .tlsParams = nullptr,
        .extension = std::move(extension),
    };
    auto result =
        ctx_->fireWrite(channel_pipeline::erase_and_box(std::move(downgraded)));
    switch (result) {
      case channel_pipeline::Result::Success:
        return;
      case channel_pipeline::Result::Backpressure:
        XLOG_EVERY_MS(WARN, 1000)
            << "Downstream backpressure after StopTLS V1 from "
            << clientAddr.describe();
        return;
      case channel_pipeline::Result::Error:
        XLOG(WARN) << "Downstream rejected post-StopTLS connection from "
                   << clientAddr.describe();
        return;
    }
  }

  folly::F14FastMap<util::StopTLSHelper*, util::StopTLSHelper::UniquePtr>
      inFlight_;
  channel_pipeline::detail::ContextImpl* ctx_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::security::handler
