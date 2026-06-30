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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>

#include <memory>

#include <folly/CppAttributes.h>

#if defined(__GNUC__) || defined(__clang__)
#define PIPELINE_HOT_PATH __attribute__((flatten))
#else
#define PIPELINE_HOT_PATH
#endif

// After close(), handlers have been removed and the head/tail raw pointers
// may dangle (their owning structs can be destroyed without nulling our
// raw pointers). Refuse all dispatches.
#define RETURN_IF_CLOSED(...)                      \
  do {                                             \
    if (FOLLY_UNLIKELY(state_ == State::Closed)) { \
      return __VA_ARGS__;                          \
    }                                              \
  } while (0)

namespace {

using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl;

Result tailHandlerReadFn(
    void* pipeline, ContextImpl& /*ctx*/, TypeErasedBox&& msg) noexcept {
  return static_cast<PipelineImpl*>(pipeline)->fireReadToTailHandler(
      std::move(msg));
}

Result headHandlerWriteFn(
    void* pipeline, ContextImpl& /*ctx*/, TypeErasedBox&& msg) noexcept {
  return static_cast<PipelineImpl*>(pipeline)->fireWriteToHeadHandler(
      std::move(msg));
}

void tailHandlerExceptionFn(
    void* pipeline,
    ContextImpl& /*ctx*/,
    folly::exception_wrapper&& e) noexcept {
  static_cast<PipelineImpl*>(pipeline)->fireExceptionToTailHandler(
      std::move(e));
}

} // namespace

namespace apache::thrift::fast_thrift::channel_pipeline {

PipelineImpl::~PipelineImpl() = default;

PipelineImpl::PipelineImpl(
    folly::EventBase* eventBase,
    std::vector<detail::HandlerNode> handlers,
    void* headHandler,
    void* tailHandler,
    void* allocator,
    std::uint32_t eventCount) noexcept
    : eventBase_(eventBase),
      handlers_(std::move(handlers)),
      headHandler_(headHandler),
      tailHandler_(tailHandler),
      allocator_(allocator),
      eventListCount_(eventCount) {
  initializeContexts();
}

void PipelineImpl::initializeContexts() noexcept {
  const auto N = handlers_.size();
  contexts_.reserve(N);
  handlerMap_.reserve(N);

  // First pass: create all contexts
  for (size_t i = 0; i < N; ++i) {
    contexts_.emplace_back(
        this, eventBase_, allocator_, i, handlers_[i].handlerId);
    handlerMap_[handlers_[i].handlerId] = i;
    if (handlers_[i].writeReadyHook_) {
      handlers_[i].writeReadyHook_->handlerIndex = i;
    }
    if (handlers_[i].readReadyHook_) {
      handlers_[i].readReadyHook_->handlerIndex = i;
    }
  }

  // Second pass: wire up cached dispatch pointers.
  // Each context caches direct function pointers to the next/prev handler,
  // eliminating the per-hop round-trip through PipelineImpl on the hot path.
  //
  // Fixed direction: reads flow 0→N→terminal, writes flow N→0→terminal
  for (size_t i = 0; i < N; ++i) {
    auto& ctx = contexts_[i];

    // Read / exception: forward direction (i+1)
    if (i + 1 < N) {
      ctx.nextReadFn_ = handlers_[i + 1].onReadFn;
      ctx.nextExceptionFn_ = handlers_[i + 1].onExceptionFn;
      ctx.nextHandler_ = handlers_[i + 1].handlerPtr;
      ctx.nextCtx_ = &contexts_[i + 1];
    } else {
      ctx.nextReadFn_ = &tailHandlerReadFn;
      ctx.nextExceptionFn_ = &tailHandlerExceptionFn;
      ctx.nextHandler_ = this;
      ctx.nextCtx_ = &ctx; // unused by terminal functions
    }

    // Write: reverse direction (i-1)
    if (i > 0) {
      ctx.prevWriteFn_ = handlers_[i - 1].onWriteFn;
      ctx.prevHandler_ = handlers_[i - 1].handlerPtr;
      ctx.prevCtx_ = &contexts_[i - 1];
    } else {
      ctx.prevWriteFn_ = &headHandlerWriteFn;
      ctx.prevHandler_ = this;
      ctx.prevCtx_ = &ctx; // unused by terminal function
    }
  }

  // Cache entry-point dispatch pointers for fireRead/fireWrite. Eliminates
  // vector indexing on the hot path entry.
  if (!handlers_.empty()) {
    firstReadFn_ = handlers_[0].onReadFn;
    firstExceptionFn_ = handlers_[0].onExceptionFn;
    firstHandler_ = handlers_[0].handlerPtr;
    firstCtx_ = &contexts_[0];

    lastWriteFn_ = handlers_[N - 1].onWriteFn;
    lastHandler_ = handlers_[N - 1].handlerPtr;
    lastCtx_ = &contexts_[N - 1];
  }

  // Per-event hooks are linked later by linkEventLists(), after the builder has
  // wired the endpoints (endpoints subscribe too). See build().
}

void PipelineImpl::linkEventLists() noexcept {
  if (eventListCount_ == 0) {
    return;
  }
  eventLists_ = std::make_unique<EventList[]>(eventListCount_);

  // Links one hook per subscription for an endpoint. Endpoints take no context,
  // so the hook's ctx is null and the dispatch thunk ignores it. Each
  // subscription carries its own typed thunk.
  auto linkEndpoint = [this](
                          void* target,
                          const EventSubscription* subs,
                          std::size_t count,
                          std::unique_ptr<EventHook[]>& hooks) noexcept {
    if (!subs || count == 0) {
      return;
    }
    hooks = std::make_unique<EventHook[]>(count);
    for (std::size_t j = 0; j < count; ++j) {
      const EventSubscription& sub = subs[j];
      DCHECK_LT(sub.id, eventListCount_);
      auto& hook = hooks[j];
      hook.fn = sub.thunk;
      hook.target = target;
      hook.ctx = nullptr;
      eventLists_[sub.id].push_back(hook);
    }
  };

  // Link order within each per-event list: tail endpoint first, then internal
  // handlers tail→head (descending index), then head endpoint last.
  linkEndpoint(
      tailHandler_,
      tailSubscriptions_,
      tailSubscriptionCount_,
      tailEventHooks_);

  for (size_t i = handlers_.size(); i > 0; --i) {
    const size_t idx = i - 1;
    auto& node = handlers_[idx];
    if (!node.subscriptions || node.subscriptionCount == 0) {
      continue;
    }
    auto& ctx = contexts_[idx];
    ctx.eventHooks_ = std::make_unique<EventHook[]>(node.subscriptionCount);
    ctx.eventHookCount_ = static_cast<std::uint32_t>(node.subscriptionCount);
    for (std::size_t j = 0; j < node.subscriptionCount; ++j) {
      const EventSubscription& sub = node.subscriptions[j];
      DCHECK_LT(sub.id, eventListCount_);
      auto& hook = ctx.eventHooks_[j];
      hook.fn = sub.thunk;
      hook.target = node.handlerPtr;
      hook.ctx = &ctx;
      eventLists_[sub.id].push_back(hook);
    }
  }

  linkEndpoint(
      headHandler_,
      headSubscriptions_,
      headSubscriptionCount_,
      headEventHooks_);
}

void PipelineImpl::clearEventLists() noexcept {
  if (!eventLists_) {
    return;
  }
  for (std::uint32_t ev = 0; ev < eventListCount_; ++ev) {
    eventLists_[ev].clear();
  }
}

void PipelineImpl::callHandlerAdded() noexcept {
  DestructorGuard dg(this);
  headHandlerAddedFn_(headHandler_);
  for (size_t i = 0; i < handlers_.size(); ++i) {
    DCHECK(handlers_[i].handlerAddedFn);
    handlers_[i].handlerAddedFn(handlers_[i].handlerPtr, contexts_[i]);
  }
  tailHandlerAddedFn_(tailHandler_);
}

void PipelineImpl::callHandlerRemovedImpl() noexcept {
  // Call in reverse order (LIFO)
  tailHandlerRemovedFn_(tailHandler_);
  for (size_t i = handlers_.size(); i > 0; --i) {
    size_t idx = i - 1;
    DCHECK(handlers_[idx].handlerRemovedFn);
    handlers_[idx].handlerRemovedFn(handlers_[idx].handlerPtr, contexts_[idx]);
  }
  headHandlerRemovedFn_(headHandler_);
}

void PipelineImpl::callHandlerRemoved() noexcept {
  DestructorGuard dg(this);
  callHandlerRemovedImpl();
}

void PipelineImpl::activate() noexcept {
  DestructorGuard dg(this);
  if (state_ != State::Inactive) {
    return;
  }
  state_ = State::Active;

  headOnPipelineActiveFn_(headHandler_);
  for (size_t i = 0; i < handlers_.size(); ++i) {
    auto& handler = handlers_[i];
    DCHECK(handler.onPipelineActiveFn);
    handler.onPipelineActiveFn(handler.handlerPtr, contexts_[i]);
  }
  tailOnPipelineActiveFn_(tailHandler_);
}

void PipelineImpl::deactivate() noexcept {
  DestructorGuard dg(this);
  if (state_ != State::Active) {
    return;
  }
  state_ = State::Inactive;

  // Call in reverse order (LIFO)
  tailOnPipelineInactiveFn_(tailHandler_);
  if (!handlers_.empty()) {
    deactivateFromIndex(handlers_.size() - 1);
  }
  headOnPipelineInactiveFn_(headHandler_);
}

void PipelineImpl::deactivateFromIndex(size_t index) noexcept {
  for (size_t i = index + 1; i > 0; --i) {
    size_t idx = i - 1;
    auto& handler = handlers_[idx];
    DCHECK(handler.onPipelineInactiveFn);
    handler.onPipelineInactiveFn(handler.handlerPtr, contexts_[idx]);
  }
}

PIPELINE_HOT_PATH Result PipelineImpl::fireRead(TypeErasedBox&& msg) noexcept {
  RETURN_IF_CLOSED(Result::Error);
  if (FOLLY_UNLIKELY(!firstReadFn_)) {
    return fireReadToTailHandler(std::move(msg));
  }
  return firstReadFn_(firstHandler_, *firstCtx_, std::move(msg));
}

PIPELINE_HOT_PATH Result PipelineImpl::fireWrite(TypeErasedBox&& msg) noexcept {
  RETURN_IF_CLOSED(Result::Error);
  if (FOLLY_UNLIKELY(!lastWriteFn_)) {
    return fireWriteToHeadHandler(std::move(msg));
  }
  return lastWriteFn_(lastHandler_, *lastCtx_, std::move(msg));
}

PIPELINE_HOT_PATH void PipelineImpl::fireException(
    folly::exception_wrapper&& e) noexcept {
  RETURN_IF_CLOSED();
  if (FOLLY_UNLIKELY(!firstExceptionFn_)) {
    fireExceptionToTailHandler(std::move(e));
    return;
  }
  firstExceptionFn_(firstHandler_, *firstCtx_, std::move(e));
}

void PipelineImpl::fireEvent(
    std::uint32_t ev, TypeErasedBox&& eventMessage) noexcept {
  RETURN_IF_CLOSED();
  DestructorGuard dg(this);
  // Out-of-range covers the events-disabled (NoEvent) case too: no lists, so
  // any fire is a clean no-op.
  if (FOLLY_UNLIKELY(ev >= eventListCount_)) {
    return;
  }
  // Walk only the subscribers of this event type. Within the list the order is
  // tail endpoint → internal handlers tail→head → head endpoint (see
  // linkEventLists). The hook is self-contained, so handlers and endpoints
  // dispatch uniformly; ctx is null for endpoints.
  for (auto& hook : eventLists_[ev]) {
    hook.fn(hook.target, hook.ctx, ev, eventMessage);
  }
}

Result PipelineImpl::sendRead(
    HandlerId handlerId, TypeErasedBox&& msg) noexcept {
  DestructorGuard dg(this);
  size_t index = lookupHandler(handlerId);
  if (index >= handlers_.size()) {
    return Result::Error;
  }
  return fireReadFromIndex(index, std::move(msg));
}

Result PipelineImpl::sendWrite(
    HandlerId handlerId, TypeErasedBox&& msg) noexcept {
  DestructorGuard dg(this);
  size_t index = lookupHandler(handlerId);
  if (index >= handlers_.size()) {
    return Result::Error;
  }
  return fireWriteFromIndex(index, std::move(msg));
}

void PipelineImpl::sendException(
    HandlerId handlerId, folly::exception_wrapper&& e) noexcept {
  DestructorGuard dg(this);
  size_t index = lookupHandler(handlerId);
  DCHECK(index < handlers_.size());
  if (index < handlers_.size()) {
    fireExceptionFromIndex(index, std::move(e));
  }
}

detail::ContextImpl* FOLLY_NULLABLE
PipelineImpl::context(HandlerId handlerId) noexcept {
  size_t index = lookupHandler(handlerId);
  if (index >= handlers_.size()) {
    return nullptr;
  }
  return &contexts_[index];
}

void PipelineImpl::close() noexcept {
  DestructorGuard dg(this);
  if (state_ == State::Closed) {
    return;
  }
  state_ = State::Closed;

  // Clear ready lists - handlers should not receive callbacks after close
  writeReadyList_.clear();
  readReadyList_.clear();
  // Unlink per-event hooks before contexts_ / endpoint hook arrays (which own
  // the hooks) are destroyed; otherwise auto-unlink would touch a dead list
  // sentinel.
  clearEventLists();

  callHandlerRemoved();
}

PIPELINE_HOT_PATH Result
PipelineImpl::fireReadFromIndex(size_t index, TypeErasedBox&& msg) noexcept {
  if (index >= handlers_.size()) {
    return fireReadToTailHandler(std::move(msg));
  }
  auto& handler = handlers_[index];
  DCHECK(handler.onReadFn);
  return handler.onReadFn(handler.handlerPtr, contexts_[index], std::move(msg));
}

PIPELINE_HOT_PATH Result
PipelineImpl::fireWriteFromIndex(size_t index, TypeErasedBox&& msg) noexcept {
  auto& handler = handlers_[index];
  DCHECK(handler.onWriteFn);
  return handler.onWriteFn(
      handler.handlerPtr, contexts_[index], std::move(msg));
}

PIPELINE_HOT_PATH void PipelineImpl::fireExceptionFromIndex(
    size_t index, folly::exception_wrapper&& e) noexcept {
  if (index >= handlers_.size()) {
    fireExceptionToTailHandler(std::move(e));
    return;
  }
  auto& handler = handlers_[index];
  DCHECK(handler.onExceptionFn);
  handler.onExceptionFn(handler.handlerPtr, contexts_[index], std::move(e));
}

Result PipelineImpl::fireReadToTailHandler(TypeErasedBox&& msg) noexcept {
  if (!tailOnReadFn_ || !tailHandler_) {
    return Result::Error;
  }
  return tailOnReadFn_(tailHandler_, std::move(msg));
}

Result PipelineImpl::fireWriteToHeadHandler(TypeErasedBox&& msg) noexcept {
  if (!headOnWriteFn_ || !headHandler_) {
    return Result::Error;
  }
  return headOnWriteFn_(headHandler_, std::move(msg));
}

void PipelineImpl::fireExceptionToTailHandler(
    folly::exception_wrapper&& e) noexcept {
  if (!tailOnExceptionFn_ || !tailHandler_) {
    return;
  }
  tailOnExceptionFn_(tailHandler_, std::move(e));
}

BytesPtr PipelineImpl::allocate(size_t size) noexcept {
  if (!allocateFn_ || !allocator_) {
    // Fallback to simple allocation
    return folly::IOBuf::create(size);
  }
  return allocateFn_(allocator_, size);
}

size_t PipelineImpl::lookupHandler(HandlerId handlerId) const noexcept {
  auto it = handlerMap_.find(handlerId);
  return it != handlerMap_.end() ? it->second : handlers_.size();
}

PIPELINE_HOT_PATH void PipelineImpl::onWriteReady() noexcept {
  DestructorGuard dg(this);
  if (state_ == State::Closed) {
    return;
  }

  // Walk the intrusive list and notify each handler.
  // Use safe iteration since handlers may unlink themselves during callback.
  auto it = writeReadyList_.begin();
  while (it != writeReadyList_.end()) {
    auto& hook = *it;
    ++it; // Advance before callback in case handler unlinks

    // O(1) lookup using handlerIndex stored in hook
    size_t i = hook.handlerIndex;
    DCHECK(handlers_[i].onWriteReadyFn);
    DCHECK_LT(i, contexts_.size());
    handlers_[i].onWriteReadyFn(handlers_[i].handlerPtr, contexts_[i]);
  }

  // Notify the tail endpoint that writes can resume.
  DCHECK(tailOnWriteReadyFn_);
  tailOnWriteReadyFn_(tailHandler_);
}

void PipelineImpl::onReadReady() noexcept {
  DestructorGuard dg(this);
  if (state_ == State::Closed) {
    return;
  }

  // Walk the intrusive list and notify each handler.
  // Use safe iteration since handlers may unlink themselves during callback.
  auto it = readReadyList_.begin();
  while (it != readReadyList_.end()) {
    auto& hook = *it;
    ++it;

    size_t i = hook.handlerIndex;
    DCHECK(handlers_[i].onReadReadyFn);
    DCHECK_LT(i, contexts_.size());
    handlers_[i].onReadReadyFn(handlers_[i].handlerPtr, contexts_[i]);
  }

  // Notify the head endpoint that reads can resume.
  DCHECK(headOnReadReadyFn_);
  headOnReadReadyFn_(headHandler_);
}

} // namespace apache::thrift::fast_thrift::channel_pipeline
