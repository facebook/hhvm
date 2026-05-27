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

#include <fizz/server/FizzServerContext.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/async/EventBase.h>
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
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

namespace apache::thrift::fast_thrift::connection::handler {

namespace fast_security = ::apache::thrift::fast_thrift::security;
namespace tls_pipeline = ::apache::thrift::fast_thrift::connection::security;
namespace tls_handler =
    ::apache::thrift::fast_thrift::connection::security::handler;

// Inner-pipeline handler tags. Local to ConnectionTLSHandler; not exposed
// to the outer pipeline.
HANDLER_TAG(tls_inner_classifier);
HANDLER_TAG(tls_inner_fizz_handshake);
HANDLER_TAG(tls_inner_stoptls_v1);

/**
 * Outer-pipeline handler that hides the entire TLS lifecycle behind a
 * single boundary. Owns an inner pipeline whose shape is fixed at
 * construction by sslPolicy + thriftParams:
 *
 *   DISABLED:    not installed in the outer pipeline (caller's choice).
 *   REQUIRED:    [FizzHandshakeHandler] (+ [StopTLSV1Handler] if useStopTLS)
 *   PERMITTED:   [TLSClassifier → FizzHandshakeHandler]
 *                              (+ [StopTLSV1Handler] if useStopTLS)
 *
 * Plays three roles:
 *   1. Middle handler in the outer (connection acceptance) pipeline. Takes
 *      ConnectionMessage in via onRead(Context&,...) and forwards to the
 *      inner pipeline via TLSPipelineMessage. Forwards writes through.
 *   2. Head endpoint of the inner pipeline. The TLS pipeline has no
 *      outbound traffic, so the head's onWrite is a sink. (Required by the
 *      pipeline framework's static head/tail contract; the alternative —
 *      a one-off empty TLSPipelineHead class — would be pure boilerplate.)
 *   3. Tail endpoint of the inner pipeline. Receives the final
 *      TLSPipelineMessage and bridges it back out by firing on the outer
 *      context as a ConnectionMessage.
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
  // install this handler at all in that case. fizzContext is required for
  // REQUIRED/PERMITTED; thriftParams is optional (null = no Thrift
  // extension negotiation, StopTLS V1 stage is omitted from the inner
  // pipeline). handshakeTimeout is the shared peek+handshake budget;
  // nullopt = unbounded. allocator is shared with the outer pipeline.
  ConnectionTLSHandler(
      folly::EventBase& evb,
      fast_security::SSLPolicy sslPolicy,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::optional<std::chrono::milliseconds> handshakeTimeout,
      channel_pipeline::SimpleBufferAllocator* allocator) {
    DCHECK(sslPolicy != fast_security::SSLPolicy::DISABLED)
        << "ConnectionTLSHandler should not be installed for DISABLED policy";
    DCHECK(fizzContext != nullptr) << "fizzContext is required when TLS is on";

    channel_pipeline::PipelineBuilder<
        ConnectionTLSHandler,
        ConnectionTLSHandler,
        channel_pipeline::SimpleBufferAllocator>
        builder;
    builder.setEventBase(&evb).setHead(this).setTail(this).setAllocator(
        allocator);

    if (sslPolicy == fast_security::SSLPolicy::PERMITTED) {
      builder.template addNextDuplex<tls_handler::TLSClassifier>(
          tls_inner_classifier_tag, handshakeTimeout);
    }
    builder.template addNextDuplex<tls_handler::FizzHandshakeHandler>(
        tls_inner_fizz_handshake_tag,
        std::move(fizzContext),
        thriftParams,
        handshakeTimeout);
    if (thriftParams && thriftParams->getUseStopTLS()) {
      builder.template addNextDuplex<tls_handler::StopTLSV1Handler>(
          tls_inner_stoptls_v1_tag);
    }

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
      // Outer pipeline tore down. Transport is dropped here; this is the
      // race window between outer handlerRemoved and inner-helper terminal
      // callbacks. Inner helpers are cancelled on onPipelineInactive, so
      // this should be rare.
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
