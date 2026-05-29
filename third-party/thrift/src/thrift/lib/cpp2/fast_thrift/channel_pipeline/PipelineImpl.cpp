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

#include <folly/CppAttributes.h>

#if defined(__GNUC__) || defined(__clang__)
#define PIPELINE_HOT_PATH __attribute__((flatten))
#else
#define PIPELINE_HOT_PATH
#endif

namespace {

using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl;

Result terminalRead(
    void* pipeline, ContextImpl& /*ctx*/, TypeErasedBox&& msg) noexcept {
  return static_cast<PipelineImpl*>(pipeline)->fireReadToTerminal(
      std::move(msg));
}

Result terminalWrite(
    void* pipeline, ContextImpl& /*ctx*/, TypeErasedBox&& msg) noexcept {
  return static_cast<PipelineImpl*>(pipeline)->fireWriteToTerminal(
      std::move(msg));
}

void terminalException(
    void* pipeline,
    ContextImpl& /*ctx*/,
    folly::exception_wrapper&& e) noexcept {
  static_cast<PipelineImpl*>(pipeline)->fireExceptionToTerminal(std::move(e));
}

} // namespace

namespace apache::thrift::fast_thrift::channel_pipeline {

PipelineImpl::~PipelineImpl() = default;

PipelineImpl::PipelineImpl(
    folly::EventBase* eventBase,
    std::vector<detail::HandlerNode> handlers,
    void* head,
    void* tail,
    void* allocator,
    HeadToTailOp headToTailOp) noexcept
    : eventBase_(eventBase),
      handlers_(std::move(handlers)),
      head_(head),
      tail_(tail),
      allocator_(allocator),
      headToTailOp_(headToTailOp) {
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
  }

  // Second pass: wire up cached dispatch pointers.
  // Each context caches direct function pointers to the next/prev handler,
  // eliminating the per-hop round-trip through PipelineImpl on the hot path.
  //
  // Step direction for read and write chains.
  // Write (default): reads flow 0→N→terminal, writes flow N→0→terminal
  // Read:            reads flow N→0→terminal, writes flow 0→N→terminal
  const int readStep = (headToTailOp_ == HeadToTailOp::Write) ? 1 : -1;
  const int writeStep = -readStep;

  for (size_t i = 0; i < N; ++i) {
    auto& ctx = contexts_[i];

    // Read/exception direction
    const int readNextIdx = static_cast<int>(i) + readStep;
    if (readNextIdx >= 0 && readNextIdx < static_cast<int>(N)) {
      ctx.nextReadFn_ = handlers_[readNextIdx].onReadFn;
      ctx.nextExceptionFn_ = handlers_[readNextIdx].onExceptionFn;
      ctx.nextHandler_ = handlers_[readNextIdx].handlerPtr;
      ctx.nextCtx_ = &contexts_[readNextIdx];
    } else {
      ctx.nextReadFn_ = &terminalRead;
      ctx.nextExceptionFn_ = &terminalException;
      ctx.nextHandler_ = this;
      ctx.nextCtx_ = &ctx; // unused by terminal functions
    }

    // Write direction
    const int writeNextIdx = static_cast<int>(i) + writeStep;
    if (writeNextIdx >= 0 && writeNextIdx < static_cast<int>(N)) {
      ctx.prevWriteFn_ = handlers_[writeNextIdx].onWriteFn;
      ctx.prevHandler_ = handlers_[writeNextIdx].handlerPtr;
      ctx.prevCtx_ = &contexts_[writeNextIdx];
    } else {
      ctx.prevWriteFn_ = &terminalWrite;
      ctx.prevHandler_ = this;
      ctx.prevCtx_ = &ctx; // unused by terminal function
    }
  }

  // Cache entry-point dispatch pointers for fireRead/fireWrite.
  // Eliminates vector indexing on the hot path entry.
  if (!handlers_.empty()) {
    const size_t readEntry = (headToTailOp_ == HeadToTailOp::Write) ? 0 : N - 1;
    const size_t writeEntry =
        (headToTailOp_ == HeadToTailOp::Write) ? N - 1 : 0;

    firstReadFn_ = handlers_[readEntry].onReadFn;
    firstExceptionFn_ = handlers_[readEntry].onExceptionFn;
    firstHandler_ = handlers_[readEntry].handlerPtr;
    firstCtx_ = &contexts_[readEntry];

    lastWriteFn_ = handlers_[writeEntry].onWriteFn;
    lastHandler_ = handlers_[writeEntry].handlerPtr;
    lastCtx_ = &contexts_[writeEntry];
  }
}

void PipelineImpl::callHandlerAdded() noexcept {
  DestructorGuard dg(this);
  for (size_t i = 0; i < handlers_.size(); ++i) {
    DCHECK(handlers_[i].handlerAddedFn);
    handlers_[i].handlerAddedFn(handlers_[i].handlerPtr, contexts_[i]);
  }
}

void PipelineImpl::callHandlerRemovedImpl() noexcept {
  // Call in reverse order (LIFO)
  for (size_t i = handlers_.size(); i > 0; --i) {
    size_t idx = i - 1;
    DCHECK(handlers_[idx].handlerRemovedFn);
    handlers_[idx].handlerRemovedFn(handlers_[idx].handlerPtr, contexts_[idx]);
  }
}

void PipelineImpl::callHandlerRemoved() noexcept {
  DestructorGuard dg(this);
  callHandlerRemovedImpl();
}

void PipelineImpl::activate() noexcept {
  DestructorGuard dg(this);
  if (closed_) {
    return;
  }
  for (size_t i = 0; i < handlers_.size(); ++i) {
    auto& handler = handlers_[i];
    DCHECK(handler.onPipelineActivatedFn);
    handler.onPipelineActivatedFn(handler.handlerPtr, contexts_[i]);
  }
  if (headLifecycleHook_ && headLifecycleHook_->onActivated) {
    headLifecycleHook_->onActivated(headLifecycleHook_->self);
  }
  if (tailLifecycleHook_ && tailLifecycleHook_->onActivated) {
    tailLifecycleHook_->onActivated(tailLifecycleHook_->self);
  }
}

void PipelineImpl::deactivate() noexcept {
  DestructorGuard dg(this);
  if (closed_) {
    return;
  }
  if (headLifecycleHook_ && headLifecycleHook_->onDeactivated) {
    headLifecycleHook_->onDeactivated(headLifecycleHook_->self);
  }
  if (tailLifecycleHook_ && tailLifecycleHook_->onDeactivated) {
    tailLifecycleHook_->onDeactivated(tailLifecycleHook_->self);
  }
  if (!handlers_.empty()) {
    deactivateFromIndex(handlers_.size() - 1);
  }
}

void PipelineImpl::deactivateFromIndex(size_t index) noexcept {
  for (size_t i = index + 1; i > 0; --i) {
    size_t idx = i - 1;
    auto& handler = handlers_[idx];
    DCHECK(handler.onPipelineDeactivatedFn);
    handler.onPipelineDeactivatedFn(handler.handlerPtr, contexts_[idx]);
  }
}

PIPELINE_HOT_PATH Result PipelineImpl::fireRead(TypeErasedBox&& msg) noexcept {
  if (FOLLY_UNLIKELY(closed_ || !firstReadFn_)) {
    return fireReadToTerminal(std::move(msg));
  }
  return firstReadFn_(firstHandler_, *firstCtx_, std::move(msg));
}

PIPELINE_HOT_PATH Result PipelineImpl::fireWrite(TypeErasedBox&& msg) noexcept {
  if (FOLLY_UNLIKELY(closed_ || !lastWriteFn_)) {
    return fireWriteToTerminal(std::move(msg));
  }
  return lastWriteFn_(lastHandler_, *lastCtx_, std::move(msg));
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
  if (closed_) {
    return;
  }
  closed_ = true;

  // Clear write ready list - handlers should not receive callbacks after close
  writeReadyList_.clear();

  callHandlerRemoved();
}

PIPELINE_HOT_PATH Result
PipelineImpl::fireReadFromIndex(size_t index, TypeErasedBox&& msg) noexcept {
  if (index >= handlers_.size()) {
    return fireReadToTerminal(std::move(msg));
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
    fireExceptionToTerminal(std::move(e));
    return;
  }
  auto& handler = handlers_[index];
  DCHECK(handler.onExceptionFn);
  handler.onExceptionFn(handler.handlerPtr, contexts_[index], std::move(e));
}

Result PipelineImpl::fireReadToTerminal(TypeErasedBox&& msg) noexcept {
  if (!readTerminalOnMessageFn_ || !readTerminal_) {
    return Result::Error;
  }
  return readTerminalOnMessageFn_(readTerminal_, std::move(msg));
}

Result PipelineImpl::fireWriteToTerminal(TypeErasedBox&& msg) noexcept {
  if (!writeTerminalOnMessageFn_ || !writeTerminal_) {
    return Result::Error;
  }
  return writeTerminalOnMessageFn_(writeTerminal_, std::move(msg));
}

void PipelineImpl::fireExceptionToTerminal(
    folly::exception_wrapper&& e) noexcept {
  if (!readTerminalOnExceptionFn_ || !readTerminal_) {
    return;
  }
  readTerminalOnExceptionFn_(readTerminal_, std::move(e));
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
  if (closed_) {
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
    handlers_[i].onWriteReadyFn(handlers_[i].handlerPtr, contexts_[i]);
  }
}

} // namespace apache::thrift::fast_thrift::channel_pipeline
