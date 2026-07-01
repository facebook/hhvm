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
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/TLSConnectionAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/TLSFinalizer.h>
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
 * Outer-pipeline (connection acceptance) middle handler that hides the entire
 * TLS lifecycle behind a single boundary. It owns the TLS pipeline and drives
 * it, but is not itself a node in it.
 *
 * The TLS pipeline does its work on the write path: an accepted transport is
 * submitted to the tail adapter as a write that flows through the stages and
 * converges at the head, which returns the resolved transport on the read path
 * back to the adapter. The pipeline shape is fixed at construction by
 * sslPolicy (build order is head→tail, i.e. reverse of the work order):
 *
 *   DISABLED:  not installed in the outer pipeline (caller's choice).
 *   REQUIRED:  TLSFinalizer → StopTLSV1Handler → FizzHandshakeHandler
 *                           → TLSConfigHandler → TLSConnectionAdapter
 *   PERMITTED: TLSFinalizer → StopTLSV1Handler → FizzHandshakeHandler
 *                           → TLSClassifier → TLSConfigHandler
 *                           → TLSConnectionAdapter
 *
 * StopTLSV1Handler is always present. It's a passthrough for connections that
 * didn't negotiate StopTLS V1, so the extra stage is cheap; this also keeps
 * the pipeline shape stable across hot-reload changes to whether StopTLS is
 * enabled.
 *
 * On an accepted ConnectionMessage, onRead converts to a TLSRequestMessage
 * and submits it to the tail adapter. When the adapter receives the resolved
 * transport back, it invokes the handoff registered here, which fires it onto
 * the outer pipeline as a ConnectionMessage. Exceptions raised inside the TLS
 * pipeline are routed back here the same way and re-fired onto the outer
 * pipeline so its teardown still observes inner failures.
 *
 * Hot-reload of TLS state is pull-based: the Observer passed at construction
 * is threaded into the pipeline and snapshotted there per accepted connection.
 *
 * Lifecycle:
 *   - The pipeline and its endpoints are built once in the constructor.
 *   - onPipelineActive / onPipelineInactive cascade activate() / deactivate()
 *     into the pipeline so handlers can drain in-flight peek/handshake/StopTLS
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
        tls_handler::TLSFinalizer,
        tls_handler::TLSConnectionAdapter,
        channel_pipeline::SimpleBufferAllocator>
        builder;
    builder.setEventBase(&evb).setHead(&head_).setTail(&tail_).setAllocator(
        allocator);

    // Build order is head→tail, the reverse of the work (write) order. Work
    // flows TLSConfigHandler → [TLSClassifier] → FizzHandshakeHandler →
    // StopTLSV1Handler → TLSFinalizer.
    builder.template addNextDuplex<tls_handler::StopTLSV1Handler>(
        tls_inner_stoptls_v1_tag);
    builder.template addNextDuplex<tls_handler::FizzHandshakeHandler>(
        tls_inner_fizz_handshake_tag);
    if (sslPolicy == fast_security::SSLPolicy::PERMITTED) {
      builder.template addNextDuplex<tls_handler::TLSClassifier>(
          tls_inner_classifier_tag);
    }
    // TLSConfigHandler stamps the current TLSParams snapshot onto every
    // accepted message; downstream stages read it via msg.tlsParams and
    // don't carry the Observer themselves.
    builder.template addNextDuplex<tls_handler::TLSConfigHandler>(
        tls_inner_config_tag, std::move(tlsParamsObserver));

    innerPipeline_ = builder.build();
    head_.setPipeline(innerPipeline_.get());
    tail_.setPipeline(innerPipeline_.get());
    tail_.setOwner(
        this,
        [](void* self,
           folly::AsyncTransport::UniquePtr transport,
           folly::SocketAddress clientAddr) noexcept {
          return static_cast<ConnectionTLSHandler*>(self)->onResolved(
              std::move(transport), std::move(clientAddr));
        },
        [](void* self, folly::exception_wrapper&& e) noexcept {
          static_cast<ConnectionTLSHandler*>(self)->onInnerException(
              std::move(e));
        });
  }

  ~ConnectionTLSHandler() = default;
  ConnectionTLSHandler(const ConnectionTLSHandler&) = delete;
  ConnectionTLSHandler& operator=(const ConnectionTLSHandler&) = delete;
  ConnectionTLSHandler(ConnectionTLSHandler&&) = delete;
  ConnectionTLSHandler& operator=(ConnectionTLSHandler&&) = delete;

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
    tls_pipeline::TLSRequestMessage request{
        .transport = std::move(incoming.transport),
        .clientAddr = std::move(incoming.clientAddr),
        .tlsParams = nullptr,
        .extension = nullptr,
    };
    return tail_.submit(std::move(request));
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

 private:
  // Handoff registered on the tail adapter: the resolved transport returns
  // here and is fired onto the outer pipeline as a ConnectionMessage.
  channel_pipeline::Result onResolved(
      folly::AsyncTransport::UniquePtr transport,
      folly::SocketAddress clientAddr) noexcept {
    if (FOLLY_UNLIKELY(!outerCtx_)) {
      return channel_pipeline::Result::Success;
    }
    ConnectionMessage out{
        .transport = std::move(transport),
        .clientAddr = std::move(clientAddr),
    };
    return outerCtx_->fireRead(channel_pipeline::erase_and_box(std::move(out)));
  }

  // Routed here by the tail adapter when the TLS pipeline raises an exception.
  // Re-fire onto the outer pipeline so its teardown observes inner failures.
  void onInnerException(folly::exception_wrapper&& e) noexcept {
    if (outerCtx_) {
      outerCtx_->fireException(std::move(e));
    }
  }

  tls_handler::TLSFinalizer head_;
  tls_handler::TLSConnectionAdapter tail_;
  channel_pipeline::detail::ContextImpl* outerCtx_{nullptr};
  // Declared last so it is destroyed first: PipelineImpl teardown calls
  // handlerRemoved on head_/tail_, which must still be alive.
  channel_pipeline::PipelineImpl::Ptr innerPipeline_;
};

} // namespace apache::thrift::fast_thrift::connection::handler
