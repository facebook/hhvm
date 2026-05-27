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
#include <folly/io/async/EventBase.h>
#include <folly/observer/Observer.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/FizzHandshakeHandler.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/StopTLSV1Handler.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/TLSClassifier.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/TLSConfigHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>

namespace apache::thrift::fast_thrift::connection::handler {

namespace fast_security = ::apache::thrift::fast_thrift::security;
namespace tls_pipeline = ::apache::thrift::fast_thrift::connection::security;
namespace tls_handler =
    ::apache::thrift::fast_thrift::connection::security::handler;

// Inner-pipeline handler tags. Local to ConnectionTLSHandler; not exposed
// to the outer pipeline.
HANDLER_TAG(tls_inner_config);
HANDLER_TAG(tls_inner_classifier);
HANDLER_TAG(tls_inner_fizz_handshake);
HANDLER_TAG(tls_inner_stoptls_v1);

/**
 * Outer-pipeline handler that hides the entire TLS lifecycle behind a
 * single boundary. Owns an inner pipeline whose shape is fixed at
 * construction by sslPolicy:
 *
 *   DISABLED:  not installed in the outer pipeline (caller's choice).
 *   REQUIRED:  TLSConfigHandler → FizzHandshakeHandler → StopTLSV1Handler
 *   PERMITTED: TLSConfigHandler → TLSClassifier
 *                              → FizzHandshakeHandler → StopTLSV1Handler
 *
 * StopTLSV1Handler is always present in the inner pipeline. It's a
 * passthrough for connections that didn't negotiate StopTLS V1, so the
 * extra stage is cheap; this also keeps the pipeline shape stable across
 * hot-reload changes to whether StopTLS is enabled.
 *
 * Plays three roles on a single class via overloaded methods:
 *   1. Middle handler in the outer (connection acceptance) pipeline. Takes
 *      ConnectionMessage in via onRead(Context&,...) and forwards to the
 *      inner pipeline as TLSPipelineMessage. Forwards writes through.
 *   2. Head endpoint of the inner pipeline. The TLS pipeline has no
 *      outbound traffic, so the head's onWrite is a sink.
 *   3. Tail endpoint of the inner pipeline. Receives the final
 *      TLSPipelineMessage and bridges it back out by firing on the outer
 *      context as a ConnectionMessage.
 *
 * Hot-reload of TLS state is pull-based: the Observer passed at
 * construction is threaded into the inner pipeline and snapshotted there
 * per accepted connection.
 *
 * Lifecycle:
 *   - Inner pipeline is built once in the constructor; never null after.
 *   - onPipelineActive / onPipelineInactive on the outer middle handler
 *     cascade activate() / deactivate() into the inner pipeline so the
 *     inner handlers can drain their in-flight peek/handshake/StopTLS
 *     helpers on shutdown.
 */
class ConnectionTLSHandler {
 public:
  // sslPolicy must not be DISABLED — caller (ConnectionHandler) doesn't
  // install this handler at all in that case. tlsParamsObserver supplies
  // fizz context + thrift extension params + handshake timeout per accept.
  // allocator is shared with the outer pipeline.
  ConnectionTLSHandler(
      folly::EventBase& evb,
      fast_security::SSLPolicy sslPolicy,
      folly::observer::Observer<std::shared_ptr<const fast_security::TLSParams>>
          tlsParamsObserver,
      channel_pipeline::SimpleBufferAllocator* allocator) {
    DCHECK(sslPolicy != fast_security::SSLPolicy::DISABLED)
        << "ConnectionTLSHandler should not be installed for DISABLED policy";

    channel_pipeline::PipelineBuilder<
        ConnectionTLSHandler,
        ConnectionTLSHandler,
        channel_pipeline::SimpleBufferAllocator>
        builder;
    builder.setEventBase(&evb).setHead(this).setTail(this).setAllocator(
        allocator);

    // TLSConfigHandler stamps the current TLSParams snapshot onto every
    // accepted message; downstream handlers read it via msg.tlsParams and
    // don't carry the Observer themselves.
    builder.template addNextDuplex<tls_handler::TLSConfigHandler>(
        tls_inner_config_tag, std::move(tlsParamsObserver));
    if (sslPolicy == fast_security::SSLPolicy::PERMITTED) {
      builder.template addNextDuplex<tls_handler::TLSClassifier>(
          tls_inner_classifier_tag);
    }
    builder.template addNextDuplex<tls_handler::FizzHandshakeHandler>(
        tls_inner_fizz_handshake_tag);
    builder.template addNextDuplex<tls_handler::StopTLSV1Handler>(
        tls_inner_stoptls_v1_tag);

    innerPipeline_ = builder.build();
  }

  ~ConnectionTLSHandler() = default;
  ConnectionTLSHandler(const ConnectionTLSHandler&) = delete;
  ConnectionTLSHandler& operator=(const ConnectionTLSHandler&) = delete;
  ConnectionTLSHandler(ConnectionTLSHandler&&) = delete;
  ConnectionTLSHandler& operator=(ConnectionTLSHandler&&) = delete;

  // === Outer pipeline: middle handler (Context overloads) ===

  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    outerCtx_ = &ctx;
  }

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    outerCtx_ = nullptr;
  }

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& /*ctx*/, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto incoming = msg.take<ConnectionMessage>();
    tls_pipeline::TLSPipelineMessage tlsMsg{
        .transport = std::move(incoming.transport),
        .clientAddr = std::move(incoming.clientAddr),
        .tlsParams = nullptr,
        .extension = nullptr,
    };
    return innerPipeline_->fireRead(
        channel_pipeline::erase_and_box(std::move(tlsMsg)));
  }

  template <typename Context>
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {
    innerPipeline_->activate();
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    innerPipeline_->deactivate();
  }

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === Inner pipeline: tail endpoint (no Context) ===

  // Final TLSPipelineMessage from the inner pipeline → ConnectionMessage on
  // the outer pipeline.
  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto m = msg.take<tls_pipeline::TLSPipelineMessage>();
    if (FOLLY_UNLIKELY(!outerCtx_)) {
      return channel_pipeline::Result::Success;
    }
    ConnectionMessage out{
        .transport = std::move(m.transport),
        .clientAddr = std::move(m.clientAddr),
    };
    return outerCtx_->fireRead(channel_pipeline::erase_and_box(std::move(out)));
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    if (outerCtx_) {
      outerCtx_->fireException(std::move(e));
    }
  }

  // === Inner pipeline: head endpoint (no Context) ===

  // TLS pipeline has no outbound traffic — discard.
  channel_pipeline::Result onWrite(
      channel_pipeline::TypeErasedBox&& /*msg*/) noexcept {
    return channel_pipeline::Result::Success;
  }

  // === Endpoint lifecycle methods (no Context, shared by head + tail) ===

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onReadReady() noexcept {}
  void onWriteReady() noexcept {}

 private:
  channel_pipeline::PipelineImpl::Ptr innerPipeline_;
  channel_pipeline::detail::ContextImpl* outerCtx_{nullptr};
};

} // namespace apache::thrift::fast_thrift::connection::handler
