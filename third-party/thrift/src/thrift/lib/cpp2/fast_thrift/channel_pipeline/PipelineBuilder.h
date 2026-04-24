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
      "and Tail to satisfy TailEndpointHandler (onRead + onException)");
  static_assert(
      BufferAllocator<Allocator>,
      "Allocator must satisfy BufferAllocator concept");

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
        static_cast<void*>(allocator_)));

    wireHeadHandler(pipeline.get());
    wireTailHandler(pipeline.get());

    pipeline->allocateFn_ = [](void* alloc, size_t size) noexcept -> BytesPtr {
      return static_cast<Allocator*>(alloc)->allocate(size);
    };

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
                                  TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<HeadHandler*>(h)->onWrite(std::move(msg));
    };
    pipeline->headOnReadReadyFn_ = [](void* h) noexcept {
      static_cast<HeadHandler*>(h)->onReadReady();
    };

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
                                 TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<TailHandler*>(h)->onRead(std::move(msg));
    };
    pipeline->tailOnExceptionFn_ = [](void* h,
                                      folly::exception_wrapper&& e) noexcept {
      static_cast<TailHandler*>(h)->onException(std::move(e));
    };
    pipeline->tailOnWriteReadyFn_ = [](void* t) noexcept {
      static_cast<TailHandler*>(t)->onWriteReady();
    };

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
  std::vector<detail::HandlerNode> handlers_;
};

} // namespace apache::thrift::fast_thrift::channel_pipeline
