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

#include <memory>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline {

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
    typename Allocator = SimpleBufferAllocator>
class PipelineBuilder {
  static_assert(
      ValidEndpointPair<HeadHandler, TailHandler>,
      "Pipeline requires Head to satisfy HeadEndpointHandler (onWrite) "
      "or EndpointHandler, and Tail to satisfy "
      "TailEndpointHandler (onRead + onException) or EndpointHandler");
  static_assert(
      BufferAllocator<Allocator>,
      "Allocator must satisfy BufferAllocator concept");

  // Compile-time detection: which style is each endpoint?
  static constexpr bool kHeadIsNewStyle = HeadEndpointHandler<HeadHandler>;
  static constexpr bool kTailIsNewStyle = TailEndpointHandler<TailHandler>;
  static constexpr bool kUsingNewEndpoints = kHeadIsNewStyle && kTailIsNewStyle;
  static constexpr bool kUsingLegacyEndpoints =
      !kHeadIsNewStyle && !kTailIsNewStyle;
  static constexpr bool kUsingMixedEndpoints =
      !kUsingNewEndpoints && !kUsingLegacyEndpoints;

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  /**
   * Set the head-to-tail operation direction.
   * DEPRECATED: Only needed for legacy EndpointHandler implementations.
   * For new code, use HeadEndpointHandler and TailEndpointHandler concepts
   * and the direction will be inferred automatically.
   */
  [[deprecated(
      "Please use new HeadEndpointHandler/TailEndpointHandler version")]]
  PipelineBuilder& setHeadToTailOp(HeadToTailOp op) noexcept {
    headToTailOp_ = op;
    return *this;
  }
#pragma GCC diagnostic pop

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
        InboundHandler<H, detail::ContextImpl>,
        "Handler must satisfy InboundHandler concept");
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
        InboundHandler<H, detail::ContextImpl>,
        "Handler must satisfy InboundHandler concept");
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
        OutboundHandler<H, detail::ContextImpl>,
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
        OutboundHandler<H, detail::ContextImpl>,
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
        DuplexHandler<H, detail::ContextImpl>,
        "Handler must satisfy DuplexHandler concept");
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
        DuplexHandler<H, detail::ContextImpl>,
        "Handler must satisfy DuplexHandler concept");
    return addHandler<H>(tag.id, std::move(handler));
  }

  /**
   * Build the pipeline (new-style endpoints).
   *
   * Validates all required components are set, creates the PipelineImpl,
   * initializes contexts, and calls lifecycle methods.
   *
   * @return Unique pointer to the constructed pipeline
   * @throws std::runtime_error if required components are missing
   */
  PipelineImpl::Ptr build()
    requires kUsingNewEndpoints
  {
    validateRequired();

    // Use new constructor without direction - direction is fixed
    auto pipeline = PipelineImpl::Ptr(new PipelineImpl(
        eventBase_,
        std::move(handlers_),
        static_cast<void*>(headHandler_),
        static_cast<void*>(tailHandler_),
        static_cast<void*>(allocator_)));

    // Wire terminal function pointers (fixed direction)
    // Head handles writes (data exits), Tail handles reads (data enters)
    wireWriteTerminal(pipeline.get(), headHandler_);
    wireReadTerminal(pipeline.get(), tailHandler_);

    // Wire lifecycle function pointers
    wireEndpointLifecycle(pipeline.get());

    pipeline->allocateFn_ = [](void* alloc, size_t size) noexcept -> BytesPtr {
      return static_cast<Allocator*>(alloc)->allocate(size);
    };

    // Call handlerAdded for handlers and endpoints
    pipeline->callHandlerAdded();
    headHandler_->handlerAdded();
    tailHandler_->handlerAdded();

    return pipeline;
  }

  /**
   * Build the pipeline (legacy endpoints).
   *
   * @deprecated Use HeadEndpointHandler and TailEndpointHandler instead.
   */
  [[deprecated("Use HeadEndpointHandler/TailEndpointHandler concepts")]]
  PipelineImpl::Ptr build()
    requires(kUsingLegacyEndpoints)
  {
    validateRequired();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    auto pipeline = PipelineImpl::Ptr(new PipelineImpl(
        eventBase_,
        std::move(handlers_),
        static_cast<void*>(headHandler_),
        static_cast<void*>(tailHandler_),
        static_cast<void*>(allocator_),
        headToTailOp_));

    // Wire terminals based on runtime direction
    if (headToTailOp_ == HeadToTailOp::Write) {
      wireLegacyReadTerminal(pipeline.get(), headHandler_);
      wireLegacyWriteTerminal(pipeline.get(), tailHandler_);
    } else {
      wireLegacyReadTerminal(pipeline.get(), tailHandler_);
      wireLegacyWriteTerminal(pipeline.get(), headHandler_);
    }
#pragma GCC diagnostic pop

    pipeline->allocateFn_ = [](void* alloc, size_t size) noexcept -> BytesPtr {
      return static_cast<Allocator*>(alloc)->allocate(size);
    };

    pipeline->callHandlerAdded();

    return pipeline;
  }

  /**
   * Build the pipeline (mixed endpoints - one new-style, one legacy).
   *
   * Direction is fixed: reads exit at head, writes exit at tail.
   * Uses appropriate method (onRead/onMessage, onWrite/onMessage) based
   * on each endpoint's type.
   *
   * @deprecated Migrate all endpoints to
   * HeadEndpointHandler/TailEndpointHandler.
   */
  [[deprecated(
      "Migrate all endpoints to HeadEndpointHandler/TailEndpointHandler")]]
  PipelineImpl::Ptr build()
    requires(kUsingMixedEndpoints)
  {
    validateRequired();

    auto pipeline = PipelineImpl::Ptr(new PipelineImpl(
        eventBase_,
        std::move(handlers_),
        static_cast<void*>(headHandler_),
        static_cast<void*>(tailHandler_),
        static_cast<void*>(allocator_)));

    // Wire write terminal based on head's type
    if constexpr (kHeadIsNewStyle) {
      wireWriteTerminal(pipeline.get(), headHandler_);
    } else {
      wireLegacyWriteTerminal(pipeline.get(), headHandler_);
    }

    // Wire read terminal based on tail's type
    if constexpr (kTailIsNewStyle) {
      wireReadTerminal(pipeline.get(), tailHandler_);
    } else {
      wireLegacyReadTerminal(pipeline.get(), tailHandler_);
    }

    // Wire lifecycle for new-style endpoints only
    if constexpr (kHeadIsNewStyle) {
      wireHeadLifecycle(pipeline.get());
    }
    if constexpr (kTailIsNewStyle) {
      wireTailLifecycle(pipeline.get());
    }

    pipeline->allocateFn_ = [](void* alloc, size_t size) noexcept -> BytesPtr {
      return static_cast<Allocator*>(alloc)->allocate(size);
    };

    pipeline->callHandlerAdded();

    // Call handlerAdded for new-style endpoints
    if constexpr (kHeadIsNewStyle) {
      headHandler_->handlerAdded();
    }
    if constexpr (kTailIsNewStyle) {
      tailHandler_->handlerAdded();
    }

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

  // Wire write terminal for new-style HeadEndpointHandler
  void wireWriteTerminal(PipelineImpl* pipeline, HeadHandler* handler) {
    pipeline->writeTerminal_ = static_cast<void*>(handler);
    pipeline->writeTerminalOnMessageFn_ =
        [](void* h, TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<HeadHandler*>(h)->onWrite(std::move(msg));
    };
  }

  // Wire read terminal for new-style TailEndpointHandler
  void wireReadTerminal(PipelineImpl* pipeline, TailHandler* handler) {
    pipeline->readTerminal_ = static_cast<void*>(handler);
    pipeline->readTerminalOnMessageFn_ =
        [](void* h, TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<TailHandler*>(h)->onRead(std::move(msg));
    };
    pipeline->readTerminalOnExceptionFn_ =
        [](void* h, folly::exception_wrapper&& e) noexcept {
          static_cast<TailHandler*>(h)->onException(std::move(e));
        };
  }

  // Wire read terminal for legacy EndpointHandler (deprecated)
  template <typename Handler>
  void wireLegacyReadTerminal(PipelineImpl* pipeline, Handler* handler) {
    pipeline->readTerminal_ = static_cast<void*>(handler);
    pipeline->readTerminalOnMessageFn_ =
        [](void* h, TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<Handler*>(h)->onMessage(std::move(msg));
    };
    pipeline->readTerminalOnExceptionFn_ =
        [](void* h, folly::exception_wrapper&& e) noexcept {
          static_cast<Handler*>(h)->onException(std::move(e));
        };
  }

  // Wire write terminal for legacy EndpointHandler (deprecated)
  template <typename Handler>
  void wireLegacyWriteTerminal(PipelineImpl* pipeline, Handler* handler) {
    pipeline->writeTerminal_ = static_cast<void*>(handler);
    pipeline->writeTerminalOnMessageFn_ =
        [](void* h, TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<Handler*>(h)->onMessage(std::move(msg));
    };
  }

  void wireEndpointLifecycle(PipelineImpl* pipeline) {
    wireHeadLifecycle(pipeline);
    wireTailLifecycle(pipeline);
  }

  void wireHeadLifecycle(PipelineImpl* pipeline) {
    pipeline->headOnPipelineActiveFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->onPipelineActive();
    };
    pipeline->headOnPipelineInactiveFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->onPipelineInactive();
    };
    pipeline->headHandlerRemovedFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->handlerRemoved();
    };
  }

  void wireTailLifecycle(PipelineImpl* pipeline) {
    pipeline->tailOnPipelineActiveFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->onPipelineActive();
    };
    pipeline->tailOnPipelineInactiveFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->onPipelineInactive();
    };
    pipeline->tailHandlerRemovedFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->handlerRemoved();
    };
  }
#pragma GCC diagnostic pop

  template <typename H, typename... Args>
  PipelineBuilder& addHandler(HandlerId id, Args&&... args) {
    auto handler = std::make_unique<H>(std::forward<Args>(args)...);
    handlers_.push_back(detail::makeHandlerNode<H>(id, std::move(handler)));
    return *this;
  }

  template <typename H>
  PipelineBuilder& addHandler(HandlerId id, std::unique_ptr<H> handler) {
    handlers_.push_back(detail::makeHandlerNode<H>(id, std::move(handler)));
    return *this;
  }

  folly::EventBase* eventBase_{nullptr};
  HeadHandler* headHandler_{nullptr};
  TailHandler* tailHandler_{nullptr};
  Allocator* allocator_{nullptr};
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  HeadToTailOp headToTailOp_{HeadToTailOp::Write};
#pragma GCC diagnostic pop
  std::vector<detail::HandlerNode> handlers_;
};

} // namespace apache::thrift::fast_thrift::channel_pipeline
