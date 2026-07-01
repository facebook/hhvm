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
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/observer/Observer.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>

namespace apache::thrift::fast_thrift::connection::security::handler {

namespace fast_security = ::apache::thrift::fast_thrift::security;

/**
 * Pipeline handler that snapshots a TLSParams Observer per message on the
 * work (write) path and stamps the result onto msg.tlsParams before
 * forwarding. The read path is passthrough. No in-flight state, nothing to
 * drain.
 *
 * Hot-reload comes for free: a setValue on the Observer's source is
 * picked up by the next snapshot; an in-flight message holds the old
 * snapshot via the captured shared_ptr.
 */
class TLSConfigHandler {
 public:
  explicit TLSConfigHandler(
      folly::observer::Observer<std::shared_ptr<const fast_security::TLSParams>>
          tlsParamsObserver) noexcept
      : tlsParamsObserver_(std::move(tlsParamsObserver)) {}

  ~TLSConfigHandler() = default;
  TLSConfigHandler(const TLSConfigHandler&) = delete;
  TLSConfigHandler& operator=(const TLSConfigHandler&) = delete;
  TLSConfigHandler(TLSConfigHandler&&) = delete;
  TLSConfigHandler& operator=(TLSConfigHandler&&) = delete;

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === Outbound (work path) ===

  template <typename Context>
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto incoming = msg.take<TLSRequestMessage>();
    incoming.tlsParams = *tlsParamsObserver_.getSnapshot();
    return ctx.fireWrite(channel_pipeline::erase_and_box(std::move(incoming)));
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
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  folly::observer::Observer<std::shared_ptr<const fast_security::TLSParams>>
      tlsParamsObserver_;
};

} // namespace apache::thrift::fast_thrift::connection::security::handler
