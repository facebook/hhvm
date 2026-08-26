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

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/Likely.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/HHWheelTimer.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/RpcTransportStats.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ThriftRequestContext.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

/**
 * ThriftClientRequestTimeoutHandler - Enforces the per-request client timeout
 * (the RpcOptions timeout, carried as RequestRpcMetadata.clientTimeoutMs),
 * falling back to a connection-level default for requests that carry none.
 *
 * Without that default a request with no RpcOptions timeout is unbounded,
 * where a classic RocketClientChannel would fall back to its channel timeout.
 * The default is opt-in: leave it unset and such a request stays unbounded,
 * exactly as before.
 *
 * On expiry the caller is failed in-process with a TIMED_OUT
 * TTransportException; nothing is sent on the wire and the connection stays
 * healthy. A late server response for a timed-out request is dropped locally,
 * matching classic client behavior.
 *
 * Duplex handler: arms a timer on the outbound request and disarms it on the
 * matching inbound response. The timer lives on the per-request
 * ThriftRequestContext, which round-trips through the pipeline — so there is no
 * correlation map, and the timer is cancelled automatically if the context is
 * destroyed (connection teardown), keeping this handler stateless.
 */
class ThriftClientRequestTimeoutHandler {
 public:
  ThriftClientRequestTimeoutHandler() = default;

  // `defaultTimeout` bounds requests that carry no clientTimeoutMs of their
  // own. Non-positive leaves them unbounded.
  explicit ThriftClientRequestTimeoutHandler(
      std::chrono::milliseconds defaultTimeout) noexcept
      : defaultTimeout_(defaultTimeout) {}

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === OutboundHandler ===

  // Arm the per-request timeout, then forward. No-op when no timeout is set.
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<ThriftRequestMessage>();
    const std::chrono::milliseconds timeout = clientTimeout(request);
    if (timeout.count() > 0) {
      auto* rc =
          static_cast<ThriftRequestContext*>(request.requestContext.get());
      if (FOLLY_LIKELY(rc != nullptr)) {
        rc->timeout = std::make_unique<Timeout>(rc);
        ctx.eventBase()->timer().scheduleTimeout(rc->timeout.get(), timeout);
      }
    }
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  // === InboundHandler ===

  // Disarm the per-request timeout. If it already fired, drop this late
  // response so the tail is not invoked a second time.
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<ThriftResponseMessage>();
    auto* rc =
        static_cast<ThriftRequestContext*>(response.requestContext.get());
    if (rc != nullptr) {
      rc->timeout.reset();
      if (FOLLY_UNLIKELY(!rc->handler)) {
        return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
      }
    }
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

 private:
  // Timer callback owned by ThriftRequestContext. On expiry it fails the caller
  // with TIMED_OUT (mirroring ThriftClientAppAdapter's error delivery). It does
  // not touch ThriftRequestContext::timeout: a callback must not free itself
  // from within timeoutExpired(); the fired callback is reclaimed when the late
  // response arrives (onRead) or when the context is destroyed.
  struct Timeout : folly::HHWheelTimer::Callback {
    explicit Timeout(ThriftRequestContext* rc) noexcept : rc_(rc) {}

    void timeoutExpired() noexcept override {
      auto handler = std::move(rc_->handler);
      handler(
          folly::makeUnexpected(
              folly::make_exception_wrapper<
                  apache::thrift::transport::TTransportException>(
                  apache::thrift::transport::TTransportException::TIMED_OUT,
                  "client request timeout")),
          apache::thrift::RpcTransportStats{});
    }

    ThriftRequestContext* rc_;
  };

  // The per-request deadline wins wherever it is set; the connection default
  // covers the rest. Unparseable metadata falls back too -- a request whose
  // deadline cannot be read is the last one that should run unbounded.
  std::chrono::milliseconds clientTimeout(
      const ThriftRequestMessage& request) const noexcept {
    const auto* metadata = request.payload.getRequestRpcMetadata();
    if (metadata == nullptr) {
      return defaultTimeout_;
    }
    const std::chrono::milliseconds perRequest{
        metadata->clientTimeoutMs().value_or(0)};
    return perRequest.count() > 0 ? perRequest : defaultTimeout_;
  }

  std::chrono::milliseconds defaultTimeout_{0};
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        ThriftClientRequestTimeoutHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientRequestTimeoutHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::thrift::client::handler
