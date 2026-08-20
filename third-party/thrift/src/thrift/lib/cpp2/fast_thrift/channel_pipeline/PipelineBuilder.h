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

#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/EndpointAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/HandlerNode.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/TypedContext.h>

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline {

namespace detail {
// Shared tag for PipelineBuilder's private rebind constructor. Defined at
// namespace scope (not nested) so every PipelineBuilder instantiation names the
// same type — addState constructs a sibling instantiation through it.
struct PipelineBuilderRebindTag {};
} // namespace detail

/**
 * PipelineBuilder provides a fluent API for constructing pipelines.
 *
 * Template parameters:
 * - HeadHandler: Type satisfying HeadEndpointHandler concept
 * - TailHandler: Type satisfying TailEndpointHandler concept
 * - Allocator: Type satisfying BufferAllocator concept
 *
 * The pipeline has a fixed flow direction:
 * - fireRead() propagates and exits at head's onRead()
 * - fireWrite() propagates and exits at tail's onWrite()
 *
 * Handlers are added in head-to-tail order using concept-checked methods:
 *   - addNextInbound<H>:  H must satisfy InboundHandler concept
 *   - addNextOutbound<H>: H must satisfy OutboundHandler concept
 *   - addNextDuplex<H>:   H must satisfy DuplexHandler concept
 *
 * Usage:
 *   HANDLER_TAG(codec);
 *   HANDLER_TAG(thrift);
 *
 *   auto pipeline = PipelineBuilder<MyApp, MyTransport,
 *     SimpleBufferAllocator>()
 *       .setEventBase(&evb)
 *       .setHead(&app)
 *       .setTail(&transport)
 *       .setAllocator(&allocator)
 *       .addNextDuplex<CodecHandler>(codec_tag)
 *       .addNextInbound<ThriftHandler>(thrift_tag)
 *       .build();
 */
template <
    typename HeadHandler,
    typename TailHandler,
    typename Allocator = SimpleBufferAllocator,
    typename EventEnumT = NoEvent,
    typename StateTuple = std::tuple<>>
class PipelineBuilder {
  static_assert(
      ValidEndpointPair<HeadHandler, TailHandler>,
      "Pipeline requires Head to satisfy HeadEndpointHandler (onWrite) "
      "and Tail to satisfy TailEndpointHandler (onRead + onException)");
  static_assert(
      BufferAllocator<Allocator>,
      "Allocator must satisfy BufferAllocator concept");
  static_assert(
      EventEnum<EventEnumT>,
      "EventEnum must be NoEvent (events disabled) or a uint32_t-backed enum "
      "class exposing a Count sentinel as its last value");

  // The context type handlers receive: bare ContextImpl for a stateless
  // pipeline, otherwise a TypedContext<StateTuple>. Handler concepts are
  // checked against this type so a handler that uses ctx.state<T>() is
  // validated against the compile-time-typed context.
  using Ctx = detail::ContextFor<StateTuple>;

  // Sibling instantiations (produced by addState) construct one another
  // through the private rebind constructor.
  template <typename, typename, typename, typename, typename>
  friend class PipelineBuilder;

 public:
  PipelineBuilder() = default;
  ~PipelineBuilder() = default;

  // Non-copyable, move-only
  PipelineBuilder(const PipelineBuilder&) = delete;
  PipelineBuilder& operator=(const PipelineBuilder&) = delete;
  PipelineBuilder(PipelineBuilder&&) noexcept = default;
  PipelineBuilder& operator=(PipelineBuilder&&) noexcept = default;

  /**
   * Set the EventBase this pipeline will run on.
   * Required before build().
   */
  PipelineBuilder& setEventBase(folly::EventBase* eventBase) noexcept {
    eventBase_ = eventBase;
    return *this;
  }

  /**
   * Set the head endpoint handler.
   * Required before build().
   */
  PipelineBuilder& setHead(HeadHandler* head) noexcept {
    headHandler_ = head;
    return *this;
  }

  /**
   * Set the tail endpoint handler.
   * Required before build().
   */
  PipelineBuilder& setTail(TailHandler* tail) noexcept {
    tailHandler_ = tail;
    return *this;
  }

  /**
   * Set the buffer allocator.
   * Required before build().
   */
  PipelineBuilder& setAllocator(Allocator* allocator) noexcept {
    allocator_ = allocator;
    return *this;
  }

  /**
   * Add the next inbound handler in head-to-tail order.
   *
   * @tparam H Handler type (must satisfy InboundHandler concept)
   * @param tag HandlerTag for compile-time ID
   * @param args Arguments forwarded to handler constructor
   * @return Reference to this builder for chaining
   */
  template <typename H, HandlerId Id, typename... Args>
  PipelineBuilder& addNextInbound(HandlerTag<Id> tag, Args&&... args) {
    static_assert(
        InboundHandler<H, Ctx>, "Handler must satisfy InboundHandler concept");
    return addHandler<H>(tag.id, std::forward<Args>(args)...);
  }

  /**
   * Add an existing inbound handler in head-to-tail order.
   *
   * @tparam H Handler type (must satisfy InboundHandler concept)
   * @param tag HandlerTag for compile-time ID
   * @param handler Unique pointer to handler instance
   * @return Reference to this builder for chaining
   */
  template <typename H, HandlerId Id>
  PipelineBuilder& addNextInbound(
      HandlerTag<Id> tag, std::unique_ptr<H> handler) {
    static_assert(
        InboundHandler<H, Ctx>, "Handler must satisfy InboundHandler concept");
    return addHandler<H>(tag.id, std::move(handler));
  }

  /**
   * Add the next outbound handler in head-to-tail order.
   *
   * @tparam H Handler type (must satisfy OutboundHandler concept)
   * @param tag HandlerTag for compile-time ID
   * @param args Arguments forwarded to handler constructor
   * @return Reference to this builder for chaining
   */
  template <typename H, HandlerId Id, typename... Args>
  PipelineBuilder& addNextOutbound(HandlerTag<Id> tag, Args&&... args) {
    static_assert(
        OutboundHandler<H, Ctx>,
        "Handler must satisfy OutboundHandler concept");
    return addHandler<H>(tag.id, std::forward<Args>(args)...);
  }

  /**
   * Add an existing outbound handler in head-to-tail order.
   *
   * @tparam H Handler type (must satisfy OutboundHandler concept)
   * @param tag HandlerTag for compile-time ID
   * @param handler Unique pointer to handler instance
   * @return Reference to this builder for chaining
   */
  template <typename H, HandlerId Id>
  PipelineBuilder& addNextOutbound(
      HandlerTag<Id> tag, std::unique_ptr<H> handler) {
    static_assert(
        OutboundHandler<H, Ctx>,
        "Handler must satisfy OutboundHandler concept");
    return addHandler<H>(tag.id, std::move(handler));
  }

  /**
   * Add the next duplex handler in head-to-tail order.
   *
   * @tparam H Handler type (must satisfy DuplexHandler concept)
   * @param tag HandlerTag for compile-time ID
   * @param args Arguments forwarded to handler constructor
   * @return Reference to this builder for chaining
   */
  template <typename H, HandlerId Id, typename... Args>
  PipelineBuilder& addNextDuplex(HandlerTag<Id> tag, Args&&... args) {
    static_assert(
        DuplexHandler<H, Ctx>, "Handler must satisfy DuplexHandler concept");
    return addHandler<H>(tag.id, std::forward<Args>(args)...);
  }

  /**
   * Add an existing duplex handler in head-to-tail order.
   *
   * @tparam H Handler type (must satisfy DuplexHandler concept)
   * @param tag HandlerTag for compile-time ID
   * @param handler Unique pointer to handler instance
   * @return Reference to this builder for chaining
   */
  template <typename H, HandlerId Id>
  PipelineBuilder& addNextDuplex(
      HandlerTag<Id> tag, std::unique_ptr<H> handler) {
    static_assert(
        DuplexHandler<H, Ctx>, "Handler must satisfy DuplexHandler concept");
    return addHandler<H>(tag.id, std::move(handler));
  }

  /**
   * Register a pipeline-level state object of type T, accessible by all
   * handlers via ctx.state<T>(). The object is constructed in place from
   * `args` and lives at pipeline scope (per-connection). Multiple distinct
   * state types can be registered by chaining addState calls; each is a
   * separate slot keyed by its type. Registering the same type twice is a
   * usage error (ctx.state<T>() would be ambiguous).
   *
   * Returns a rebound builder whose type carries the accumulated state types,
   * so the handler context becomes TypedContext<std::tuple<...>>.
   *
   * All state must be registered before any handler is added: every handler is
   * wired against the final registered state set, so interleaving addState with
   * addNext* is rejected (throws). Register state up front, then add handlers.
   *
   * `T` must be move-constructible — the registered object is moved into the
   * pipeline-owned allocation at build().
   *
   * Because it extends the state type list, addState rebinds the builder: it
   * returns a *new* builder of the extended type and moves-from this one. Use
   * it fluently on the builder expression
   * (`PipelineBuilder<...>().addState<A>().addState<B>()...`); the pre-addState
   * builder is left moved-from and must not be reused.
   *
   * @param args Constructor arguments forwarded to T
   * @return A builder carrying the extended state type list
   */
  template <typename T, typename... Args>
  auto addState(Args&&... args) {
    static_assert(
        std::is_move_constructible_v<T>,
        "addState<T>(): T must be move-constructible — the state object is "
        "moved into the pipeline at build()");
    static_assert(
        !detail::TupleHasType<StateTuple, T>::value,
        "addState<T>(): T is already registered — each pipeline-level state "
        "type may be registered at most once (ctx.state<T>() would be "
        "ambiguous)");
    if (!handlers_.empty()) {
      throw std::runtime_error(
          "PipelineBuilder: addState<T>() must be called before any handler; "
          "register all pipeline-level state before addNext*");
    }
    using NewStateTuple = decltype(std::tuple_cat(
        std::declval<StateTuple&&>(), std::declval<std::tuple<T>>()));
    return PipelineBuilder<
        HeadHandler,
        TailHandler,
        Allocator,
        EventEnumT,
        NewStateTuple>(
        detail::PipelineBuilderRebindTag{},
        eventBase_,
        headHandler_,
        tailHandler_,
        allocator_,
        std::move(handlers_),
        std::tuple_cat(
            std::move(stateTuple_),
            std::tuple<T>(T(std::forward<Args>(args)...))));
  }

  /**
   * Append an already-constructed, type-erased handler node in head-to-tail
   * order.
   *
   * Unlike addNext{Inbound,Outbound,Duplex}, the concrete handler type is not
   * known at this call site — the node was produced by
   * detail::makeHandlerNode<H, EventEnumT> where H was in scope (e.g. a
   * config-time handler registry that erased H into a factory). The node is
   * placed at the current tail-most position, so nodes appended later sit
   * closer to the tail. Callers are responsible for building the node with the
   * same EventEnumT as this builder so event subscriptions link correctly.
   *
   * @param node A HandlerNode produced by detail::makeHandlerNode
   * @return Reference to this builder for chaining
   */
  PipelineBuilder& addErasedHandler(detail::HandlerNode node) {
    handlers_.push_back(std::move(node));
    return *this;
  }

  /**
   * Build the pipeline.
   *
   * Validates all required components are set, creates the PipelineImpl,
   * initializes contexts, and calls lifecycle methods.
   *
   * @return Unique pointer to the constructed pipeline
   * @throws std::runtime_error if required components are missing
   */
  PipelineImpl::Ptr build() {
    validateRequired();

    auto pipeline = PipelineImpl::Ptr(new PipelineImpl(
        eventBase_,
        std::move(handlers_),
        static_cast<void*>(headHandler_),
        static_cast<void*>(tailHandler_),
        static_cast<void*>(allocator_),
        kEventCount<EventEnumT>));

    wireHeadHandler(pipeline.get());
    wireTailHandler(pipeline.get());

    pipeline->allocateFn_ = [](void* alloc, size_t size) noexcept -> BytesPtr {
      return static_cast<Allocator*>(alloc)->allocate(size);
    };
    pipeline->copyBufferFn_ =
        [](void* alloc, const void* data, size_t size) noexcept -> BytesPtr {
      return static_cast<Allocator*>(alloc)->copyBuffer(data, size);
    };

    // Link per-event subscriber lists now that endpoints are wired (endpoints
    // subscribe too). No-op when events are disabled.
    if constexpr (kEventsEnabled<EventEnumT>) {
      pipeline->linkEventLists();
    }

    // Wire pipeline-level state. A stateless pipeline (empty tuple) does none
    // of this: it allocates nothing, leaves each context's state pointer at its
    // null default, and passes the bare ContextImpl through unchanged.
    if constexpr (std::tuple_size_v<StateTuple> > 0) {
      // Move the accumulated state tuple into a pipeline-owned, type-erased
      // allocation and propagate the pointer to all handler contexts.
      pipeline->pipelineState_ = new StateTuple(std::move(stateTuple_));
      pipeline->pipelineStateDeleter_ = [](void* p) noexcept {
        delete static_cast<StateTuple*>(p);
      };
      pipeline->propagatePipelineState();

      // Construct each context's persistent typed view now that contexts are
      // finalized (the same stability point nextCtx_/prevCtx_ rely on).
      // Handlers then receive a stable TypedContext<StateTuple>& in every
      // callback, including lifecycle.
      detail::initTypedView<StateTuple>(pipeline->headCtx_);
      for (auto& ctx : pipeline->contexts_) {
        detail::initTypedView<StateTuple>(ctx);
      }
      detail::initTypedView<StateTuple>(pipeline->tailCtx_);
    }

    pipeline->callHandlerAdded();
    return pipeline;
  }

 private:
  void validateRequired() {
    if (!eventBase_) {
      throw std::runtime_error("PipelineBuilder: EventBase is required");
    }
    if (!headHandler_) {
      throw std::runtime_error("PipelineBuilder: Head handler is required");
    }
    if (!tailHandler_) {
      throw std::runtime_error("PipelineBuilder: Tail handler is required");
    }
    if (!allocator_) {
      throw std::runtime_error("PipelineBuilder: Allocator is required");
    }
  }

  void wireHeadHandler(PipelineImpl* pipeline) {
    pipeline->headOnWriteFn_ = [](void* h,
                                  detail::ContextImpl& ctx,
                                  TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<HeadHandler*>(h)->onWrite(ctx, std::move(msg));
    };
    pipeline->headOnReadReadyFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->onReadReady();
    };

    // User-event subscription: the head endpoint opts in by declaring
    // kSubscribedEvents and implementing `onEvent(E, const TypeErasedBox&)`.
    // linkEventLists() then links one hook per subscribed event.
    if constexpr (
        kEventsEnabled<EventEnumT> && EndpointEventSubscriber<HeadHandler>) {
      pipeline->headSubscriptions_ =
          detail::kHandlerSubscriptions<HeadHandler, /*Endpoint=*/true>.data();
      pipeline->headSubscriptionCount_ =
          detail::kHandlerSubscriptions<HeadHandler, /*Endpoint=*/true>.size();
    }

    // Lifecycle methods
    pipeline->headOnPipelineActiveFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->onPipelineActive();
    };
    pipeline->headOnPipelineInactiveFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->onPipelineInactive();
    };
    pipeline->headHandlerAddedFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->handlerAdded();
    };
    pipeline->headHandlerRemovedFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->handlerRemoved();
    };
  }

  void wireTailHandler(PipelineImpl* pipeline) {
    pipeline->tailOnReadFn_ = [](void* h,
                                 detail::ContextImpl& ctx,
                                 TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<TailHandler*>(h)->onRead(ctx, std::move(msg));
    };
    pipeline->tailOnExceptionFn_ = [](void* h,
                                      folly::exception_wrapper&& e) noexcept {
      static_cast<TailHandler*>(h)->onException(std::move(e));
    };
    pipeline->tailOnWriteReadyFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->onWriteReady();
    };

    // User-event subscription: the tail endpoint opts in by declaring
    // kSubscribedEvents and implementing `onEvent(E, const TypeErasedBox&)`.
    // linkEventLists() then links one hook per subscribed event.
    if constexpr (
        kEventsEnabled<EventEnumT> && EndpointEventSubscriber<TailHandler>) {
      pipeline->tailSubscriptions_ =
          detail::kHandlerSubscriptions<TailHandler, /*Endpoint=*/true>.data();
      pipeline->tailSubscriptionCount_ =
          detail::kHandlerSubscriptions<TailHandler, /*Endpoint=*/true>.size();
    }

    // Lifecycle methods
    pipeline->tailOnPipelineActiveFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->onPipelineActive();
    };
    pipeline->tailOnPipelineInactiveFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->onPipelineInactive();
    };
    pipeline->tailHandlerAddedFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->handlerAdded();
    };
    pipeline->tailHandlerRemovedFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->handlerRemoved();
    };
  }

  template <typename H, typename... Args>
  PipelineBuilder& addHandler(HandlerId id, Args&&... args) {
    auto handler = std::make_unique<H>(std::forward<Args>(args)...);
    handlers_.push_back(
        detail::makeHandlerNode<H, EventEnumT, StateTuple>(
            id, std::move(handler)));
    return *this;
  }

  template <typename H>
  PipelineBuilder& addHandler(HandlerId id, std::unique_ptr<H> handler) {
    handlers_.push_back(
        detail::makeHandlerNode<H, EventEnumT, StateTuple>(
            id, std::move(handler)));
    return *this;
  }

  // Private rebind constructor used by addState to transfer the in-progress
  // build state into a sibling instantiation with an extended state type list.
  PipelineBuilder(
      detail::PipelineBuilderRebindTag,
      folly::EventBase* eventBase,
      HeadHandler* headHandler,
      TailHandler* tailHandler,
      Allocator* allocator,
      std::vector<detail::HandlerNode>&& handlers,
      StateTuple&&
          stateTuple) noexcept(std::is_nothrow_move_constructible_v<StateTuple>)
      : eventBase_(eventBase),
        headHandler_(headHandler),
        tailHandler_(tailHandler),
        allocator_(allocator),
        handlers_(std::move(handlers)),
        stateTuple_(std::move(stateTuple)) {}

  folly::EventBase* eventBase_{nullptr};
  HeadHandler* headHandler_{nullptr};
  TailHandler* tailHandler_{nullptr};
  Allocator* allocator_{nullptr};
  std::vector<detail::HandlerNode> handlers_;
  StateTuple stateTuple_{};
};

} // namespace apache::thrift::fast_thrift::channel_pipeline
