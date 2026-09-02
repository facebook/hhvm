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

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <folly/Demangle.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/async/EventBase.h>

#include <fmt/core.h>
#include <folly/ExceptionString.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftBackpressureExtension.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftConnectionExtension.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftExtension.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/ThriftPipelineHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/SetupMessages.h>

namespace apache::thrift::fast_thrift::thrift::server {

namespace extension_detail {

template <bool Subscribe, auto Ev>
using SubscriptionIf = std::conditional_t<
    Subscribe,
    channel_pipeline::Subscriptions<Ev>,
    channel_pipeline::Subscriptions<>>;

template <typename A, typename B>
struct ConcatSubscriptionsImpl;
template <auto... A, auto... B>
struct ConcatSubscriptionsImpl<
    channel_pipeline::Subscriptions<A...>,
    channel_pipeline::Subscriptions<B...>> {
  using type = channel_pipeline::Subscriptions<A..., B...>;
};
template <typename A, typename B>
using ConcatSubscriptions = typename ConcatSubscriptionsImpl<A, B>::type;

/**
 * The connection events H's adapter listens to. The two after-the-fact points
 * are events; onConnectionAttempted is a message on the read path. An H that
 * implements only onConnectionClosed still subscribes to SetupComplete,
 * because that is where the established latch gating it is set. An H that
 * hooks no connection point is linked into no event list.
 */
template <typename H>
using ConnectionSubscriptions = ConcatSubscriptions<
    ConcatSubscriptions<
        SubscriptionIf<
            ThriftConnectionExtensionHandler<H>,
            ThriftServerEventType::SetupComplete>,
        SubscriptionIf<
            HasConnectionClosedCallback<H>,
            ThriftServerEventType::ConnectionClosed>>,
    SubscriptionIf<
        ThriftBackpressureExtensionHandler<H>,
        ThriftServerEventType::WriteComplete>>;

/**
 * Per-connection state an admission-control extension needs and nothing else
 * pays for: the resumer's control block, the pipeline to wake, and the latch
 * itself. Empty for every other extension, so the adapter is the same size it
 * was.
 */
struct BackpressureStateEnabled {
  std::shared_ptr<backpressure_detail::ResumeControl> resumeControl_;
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  bool paused_{false};
};
struct BackpressureStateDisabled {};

template <typename H>
using BackpressureState = std::conditional_t<
    ThriftBackpressureExtensionHandler<H>,
    BackpressureStateEnabled,
    BackpressureStateDisabled>;

/**
 * H's demangled type name, for attributing a refusal to the extension that
 * made it. Computed once per H and returned by reference, so the event can
 * hold a view rather than a copy.
 */
template <typename H>
const std::string& extensionName() {
  static const std::string kName =
      folly::demangle(typeid(H).name()).toStdString();
  return kName;
}

} // namespace extension_detail

/**
 * Adapts a user extension handler H into a duplex channel_pipeline handler.
 * This is the only place that touches the raw TypeErasedBox and pipeline
 * context — the extension itself only ever sees a ThriftRequestView /
 * ThriftRequestMutator, so it cannot retain, replace, or manually forward a
 * message.
 *
 * Forwarding / rejection contract:
 *   - H::onRequest returns RequestVerdict::proceed() → the request is
 *     forwarded toward the tail (fireRead).
 *   - H::onRequest returns RequestVerdict::reject(cause) → the request is
 *     dropped and an application error carrying the cause's name/message is
 *     emitted back down the outbound path (fireWrite), echoing the request's
 *     streamId. As the error travels out, extensions that already ran see their
 *     onResponse/onWrite in reverse order — the pipeline provides that
 * ordering.
 *
 * Response side: if H additionally implements onResponse, it is invoked (via a
 * view or mutator) on every outbound response before it is forwarded.
 *
 * Lifetime: one instance per connection (constructed by the pipeline handler
 * factory); H is owned by value and its constructor arguments are forwarded
 * from addThriftExtension. An H declaring a ConnState additionally receives
 * that connection's shared state as its first constructor argument, resolved
 * once here so the extension never handles the store itself.
 */
template <typename H>
class ThriftExtensionPipelineHandler {
  static_assert(
      ThriftExtensionHandler<H> || HasResponseCallback<H> ||
          ThriftConnectionExtensionHandler<H> ||
          ThriftBackpressureExtensionHandler<H>,
      "ThriftExtensionPipelineHandler<H>: H must implement at least one "
      "extension callback — onRequest taking either const ThriftRequestView& "
      "[read-only] or ThriftRequestMutator& [read/write] and returning "
      "RequestVerdict; onResponse over the response view or mutator; one of "
      "onConnectionAttempted / onConnectionAnswering / "
      "onConnectionEstablished / onConnectionClosed; or onEgressDrained for "
      "per-connection admission control. Every callback is noexcept.");

  static_assert(
      !ThriftBackpressureExtensionHandler<H> ||
          HasBackpressureAttachedCallback<H>,
      "ThriftExtensionPipelineHandler<H>: an extension declaring "
      "onEgressDrained must also declare onBackpressureAttached(ReadResumer). "
      "Pausing is only undone by the resumer, so an extension that can pause "
      "without holding one would stall its connection permanently.");

 public:
  template <typename... Args>
  explicit ThriftExtensionPipelineHandler(
      ExtensionStateStore& store, Args&&... args)
      : handler_(makeHandler(store, std::forward<Args>(args)...)) {}

  channel_pipeline::Result onRead(
      ThriftPipelineHandlerContext& ctx,
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& request = msg.get<ThriftServerRequestMessage>();

    // Lifecycle messages never reach the request callbacks, whatever H
    // implements: to a request extension the setup exchange is not a request,
    // and handing it one would run onRequest against a connection.
    if (FOLLY_UNLIKELY(request.payload.is<ThriftConnectionSetupPayload>())) {
      auto& setup = *request.payload.get<ThriftConnectionSetupPayload>().setup;
      // The setup message is the only place the connection is offered, so it
      // is latched here for every family that presents it later — not just
      // connection extensions.
      connContext_ = setup.connContext;
      if constexpr (ThriftConnectionExtensionHandler<H>) {
        return onConnectionAttempted(ctx, std::move(msg), setup);
      } else {
        return ctx.fireRead(std::move(msg));
      }
    }
    const channel_pipeline::Result result = [&]() noexcept {
      if constexpr (!ThriftExtensionHandler<H>) {
        return ctx.fireRead(std::move(msg));
      } else {
        return onRequest(ctx, std::move(msg));
      }
    }();
    if constexpr (ThriftBackpressureExtensionHandler<H>) {
      // Reported upward rather than acted on here: Backpressure travels back
      // to the transport, which is what stops reading the socket. An Error is
      // terminal and is never masked.
      if (FOLLY_UNLIKELY(
              bp_.paused_ && result != channel_pipeline::Result::Error)) {
        return channel_pipeline::Result::Backpressure;
      }
    }
    return result;
  }

  static constexpr extension_detail::ConnectionSubscriptions<H>
      kSubscribedEvents{};

  /**
   * The two lifecycle points that fire after the exchange they report on:
   * SetupComplete once the answer is on the wire, ConnectionClosed during
   * teardown. Only onConnectionAttempted stays a message, because it is the
   * one point that still has an answer to shape.
   */
  void onEvent(
      ThriftPipelineHandlerContext& /*ctx*/,
      ThriftServerEventType ev,
      const channel_pipeline::TypeErasedBox& evt) noexcept {
    if constexpr (ThriftBackpressureExtensionHandler<H>) {
      if (ev == ThriftServerEventType::WriteComplete) {
        onWriteComplete(evt.get<ThriftServerWriteCompleteEvent>());
        return;
      }
    }
    if constexpr (ThriftConnectionExtensionHandler<H>) {
      if (ev == ThriftServerEventType::SetupComplete) {
        onSetupComplete(*evt.get<ThriftServerSetupCompleteEvent*>());
        return;
      }
    }
    if (ev != ThriftServerEventType::ConnectionClosed) {
      return;
    }
    if constexpr (HasConnectionClosedCallback<H>) {
      // Rebuilt from the context latched during setup: the event carries no
      // payload, and the connection-context handler still owns the context
      // while the pipeline tears down.
      if (established_ && connContext_ != nullptr) {
        const ThriftConnectionView view(*connContext_);
        handler_.onConnectionClosed(view);
      }
    }
  }

 private:
  // A refusal stops the message here and answers outbound, exactly as a
  // rejected request does. Forwarding otherwise lets the next extension
  // contribute, and finally lets ThriftServerSetupHandler answer.
  channel_pipeline::Result onConnectionAttempted(
      ThriftPipelineHandlerContext& ctx,
      channel_pipeline::TypeErasedBox&& msg,
      ConnectionSetupData& setup) noexcept
    requires ThriftConnectionExtensionHandler<H>
  {
    if constexpr (HasConnectionAttemptedCallback<H>) {
      // View form first: a mutator binds to a const view parameter, so an
      // extension that only reads is not handed write access it did not ask
      // for.
      ConnectionVerdict verdict = [&]() noexcept {
        if constexpr (HasConnectionAttemptedViewCallback<H>) {
          const ThriftSetupConnectionView view(setup);
          return handler_.onConnectionAttempted(view);
        } else {
          ThriftSetupConnectionMutator mutator(setup);
          return handler_.onConnectionAttempted(mutator);
        }
      }();
      if (FOLLY_UNLIKELY(verdict.isRejected())) {
        return refuse(ctx, "onConnectionAttempted", std::move(verdict));
      }
    }
    return ctx.fireRead(std::move(msg));
  }

  // The setup answer is on the wire. Nothing is forwarded and nothing can be
  // contributed here — a refusal at this point reaches the client as an error
  // frame following the response, which the rocket setup handler emits once
  // this returns.
  void onSetupComplete(ThriftServerSetupCompleteEvent& event) noexcept
    requires ThriftConnectionExtensionHandler<H>
  {
    // Latched even when H has no established callback, so an H that only
    // implements onConnectionClosed can still tell a connection that reached
    // the established point from one that died during setup.
    established_ = true;
    if constexpr (HasConnectionEstablishedCallback<H>) {
      if (connContext_ == nullptr) {
        return;
      }
      const ThriftConnectionView view(*connContext_);
      auto verdict = handler_.onConnectionEstablished(view);
      if (FOLLY_UNLIKELY(verdict.isRejected()) && !event.reject.has_value()) {
        event.reject = SetupRejection{
            .code =
                apache::thrift::fast_thrift::frame::ErrorCode::REJECTED_SETUP,
            .reason =
                rejectionReason("onConnectionEstablished", std::move(verdict))};
      }
    }
  }

  // Names the refusing extension and its cause, so an operator can tell which
  // one closed the connection without server-side logs.
  static std::string rejectionReason(
      std::string_view callback, ConnectionVerdict verdict) noexcept {
    return fmt::format(
        "ThriftExtension::{} rejected the connection:\n[{}] {}",
        callback,
        extension_detail::extensionName<H>(),
        folly::exceptionStr(std::move(verdict).cause()).toStdString());
  }

  channel_pipeline::Result refuse(
      ThriftPipelineHandlerContext& ctx,
      std::string_view callback,
      ConnectionVerdict verdict) noexcept {
    (void)ctx.fireWrite(
        channel_pipeline::erase_and_box(makeSetupRejectionMessage(
            apache::thrift::fast_thrift::frame::ErrorCode::REJECTED_SETUP,
            rejectionReason(callback, std::move(verdict)))));
    // Non-Success is how a refusal travels back to the transport adapter: it
    // stops the setup exchange there, so no established point is announced and
    // the connection is torn down.
    return channel_pipeline::Result::Error;
  }

  // Egress has gone idle, so the server has caught up on what it owed this
  // client — the point at which the extension reassesses the connection. An
  // errored completion also empties the write queue, but says nothing about a
  // connection that is already going away, so it is not offered as quiescence;
  // teardown reaches the extension through onPipelineInactive instead.
  //
  void onWriteComplete(const ThriftServerWriteCompleteEvent& event) noexcept
    requires ThriftBackpressureExtensionHandler<H>
  {
    if (!event.quiesced ||
        event.status !=
            apache::thrift::fast_thrift::transport::WriteCompletionStatus::
                Success) {
      return;
    }
    if (FOLLY_UNLIKELY(connContext_ == nullptr)) {
      return;
    }
    const ThriftConnectionView view(*connContext_);
    handler_.onEgressDrained(view);
  }

  // Reached only through a ReadResumer, which is documented to run on the
  // connection's EventBase. That is what lets bp_ stay a plain struct: every
  // reader and writer of it — this, onRead, handlerAdded, detachResumer — is
  // that one thread. The check is the contract, not a defence against a race
  // it could not fix anyway.
  void resumeReads() noexcept
    requires ThriftBackpressureExtensionHandler<H>
  {
    DCHECK(
        bp_.pipeline_ == nullptr ||
        bp_.pipeline_->eventBase()->isInEventBaseThread())
        << "ReadResumer::resume() must be called on the connection's EventBase";
    if (!bp_.paused_) {
      return;
    }
    bp_.paused_ = false;
    if (FOLLY_LIKELY(bp_.pipeline_ != nullptr)) {
      bp_.pipeline_->onReadReady();
    }
  }

  // Severs every resumer handed out: the extension may hold one past the
  // connection, and resuming a pipeline that is going away must do nothing.
  void detachResumer() noexcept
    requires ThriftBackpressureExtensionHandler<H>
  {
    bp_.paused_ = false;
    bp_.pipeline_ = nullptr;
    if (bp_.resumeControl_ != nullptr) {
      bp_.resumeControl_->owner = nullptr;
    }
  }

  channel_pipeline::Result onRequest(
      ThriftPipelineHandlerContext& ctx,
      channel_pipeline::TypeErasedBox&& msg) noexcept
    requires ThriftExtensionHandler<H>
  {
    auto& request = msg.get<ThriftServerRequestMessage>();
    // Capture before any potential reject drops the inbound message: the
    // synthesized response must echo this streamId to correlate on the client.
    const uint32_t streamId = request.streamId;

    // View form first: a mutator binds to a const view parameter, so testing
    // the view form first hands each extension exactly the access it declared.
    const RequestVerdict verdict = [&]() noexcept {
      if constexpr (HasRequestViewCallback<H>) {
        const ThriftRequestView view(request);
        return handler_.onRequest(view);
      } else {
        ThriftRequestMutator mutator(request);
        return handler_.onRequest(mutator);
      }
    }();

    if (!verdict.isRejected()) {
      if (FOLLY_UNLIKELY(verdict.appliesBackpressure())) {
        // Latch it before forwarding: onRead reads this on the way back out
        // and reports Backpressure for this very message, so the socket stops
        // being read without a further request having to arrive first.
        if constexpr (ThriftBackpressureExtensionHandler<H>) {
          bp_.paused_ = true;
        } else {
          DCHECK(false)
              << "RequestVerdict::backpressure() from an extension that "
                 "declares no onBackpressureAttached — nothing could resume it";
        }
      }
      return ctx.fireRead(std::move(msg));
    }
    auto response = makeUnknownExceptionMessage(streamId, verdict.cause());
    return ctx.fireWrite(channel_pipeline::erase_and_box(std::move(response)));
  }

 public:
  channel_pipeline::Result onWrite(
      ThriftPipelineHandlerContext& ctx,
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    if constexpr (HasConnectionAnsweringCallback<H>) {
      auto& outbound = msg.get<ThriftServerResponseMessage>();
      if (FOLLY_UNLIKELY(
              outbound.payload.template is<ThriftSetupResponsePayload>())) {
        // The answer is assembled but not yet serialized: this is the one
        // window in which an extension can put something in front of the
        // client.
        auto& setupResponse =
            outbound.payload.template get<ThriftSetupResponsePayload>()
                .response;
        if (FOLLY_LIKELY(setupResponse != nullptr)) {
          ThriftSetupResponseMutator mutator(*setupResponse);
          handler_.onConnectionAnswering(mutator);
        }
        return ctx.fireWrite(std::move(msg));
      }
    }
    if constexpr (HasResponseCallback<H>) {
      auto& response = msg.get<ThriftServerResponseMessage>();
      if constexpr (HasResponseViewCallback<H>) {
        const ThriftResponseView view(response);
        handler_.onResponse(view);
      } else {
        ThriftResponseMutator mutator(response);
        handler_.onResponse(mutator);
      }
    }
    return ctx.fireWrite(std::move(msg));
  }

  void onException(
      ThriftPipelineHandlerContext& ctx,
      folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onPipelineActive(ThriftPipelineHandlerContext&) noexcept {}

  void onPipelineInactive(ThriftPipelineHandlerContext&) noexcept {
    if constexpr (ThriftBackpressureExtensionHandler<H>) {
      detachResumer();
    }
  }

  void onReadReady(ThriftPipelineHandlerContext&) noexcept {}
  void onWriteReady(ThriftPipelineHandlerContext&) noexcept {}

  void handlerAdded(ThriftPipelineHandlerContext& ctx) noexcept {
    if constexpr (ThriftBackpressureExtensionHandler<H>) {
      bp_.pipeline_ = ctx.pipeline();
      bp_.resumeControl_ =
          std::make_shared<backpressure_detail::ResumeControl>();
      bp_.resumeControl_->owner = this;
      bp_.resumeControl_->resumeFn = +[](void* owner) noexcept {
        static_cast<ThriftExtensionPipelineHandler*>(owner)->resumeReads();
      };
      handler_.onBackpressureAttached(
          ReadResumer(bp_.resumeControl_, ctx.eventBase()));
    }
  }

  void handlerRemoved(ThriftPipelineHandlerContext&) noexcept {
    if constexpr (ThriftBackpressureExtensionHandler<H>) {
      detachResumer();
    }
  }

 private:
  // Returned as a prvalue so H is built directly into handler_ — H needs no
  // move constructor, and its shared state is resolved exactly once per
  // connection, before any callback can run.
  template <typename... Args>
  static H makeHandler(ExtensionStateStore& store, Args&&... args) {
    if constexpr (HasConnState<H>) {
      static_assert(
          std::is_default_constructible_v<typename H::ConnState>,
          "H::ConnState must be default-constructible: the connection's state "
          "is created by whichever extension names it first, so there is no "
          "one extension whose arguments could construct it.");
      return H(
          store.getOrCreate<typename H::ConnState>(),
          std::forward<Args>(args)...);
    } else {
      return H(std::forward<Args>(args)...);
    }
  }

  H handler_;
  [[no_unique_address]] extension_detail::BackpressureState<H> bp_;
  // Latched from the setup messages so the payload-less ConnectionClosed can
  // still present the connection. Non-owning: the connection-context handler
  // outlives this pipeline's teardown.
  const ThriftConnContext* connContext_{nullptr};
  // Whether this connection reached ConnectionEstablished. Gates
  // onConnectionClosed so it never fires for a connection that died mid-setup.
  bool established_{false};
};

/**
 * Build a factory that splices extension H into a connection's thrift pipeline.
 *
 * The peer of makeThriftPipelineHandlerFactory for the extension API: it wraps
 * H in the adapter and threads the connection's ExtensionStateStore into it, so
 * an H declaring a ConnState is handed that state at construction. `args` are
 * copied and forwarded to every per-connection instance, after the state.
 */
template <typename H, typename... Args>
ThriftPipelineHandlerFactory makeThriftExtensionHandlerFactory(
    channel_pipeline::HandlerId id, Args... args) {
  using Adapter = ThriftExtensionPipelineHandler<H>;
  // The adapter is framework-owned, so this can only fire if the adapter itself
  // stops being a pipeline handler — the same named diagnostic the native path
  // gets from makeThriftPipelineHandlerFactory, rather than a failure deep
  // inside makeHandlerNode.
  static_assert(
      channel_pipeline::InboundHandler<Adapter, ThriftPipelineHandlerContext> ||
          channel_pipeline::
              OutboundHandler<Adapter, ThriftPipelineHandlerContext> ||
          channel_pipeline::
              DuplexHandler<Adapter, ThriftPipelineHandlerContext>,
      "ThriftExtensionPipelineHandler<H> must satisfy the Inbound, Outbound, or "
      "Duplex handler concept over ThriftPipelineHandlerContext");
  return [id, args...](ExtensionStateStore& store) {
    return channel_pipeline::detail::
        makeHandlerNode<Adapter, ThriftServerEventType>(
            id, std::make_unique<Adapter>(store, args...));
  };
}

} // namespace apache::thrift::fast_thrift::thrift::server
