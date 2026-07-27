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

#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

namespace coro {
class ContextHandle;
class ReadAwaitable;
class WriteAwaitable;
class ExceptionAwaitable;
} // namespace coro

/**
 * A one-shot handle that resumes a handler context on its EventBase.
 *
 * The handle may move through asynchronous work on any thread. Calling a fire
 * method consumes the handle and enqueues the context operation on the
 * pipeline's EventBase. The pipeline guard is released after that operation
 * completes on the EventBase.
 *
 * A live handle must either be consumed from any thread or destroyed on its
 * EventBase. A moved-from handle may be destroyed on any thread.
 */
class ContextHandle final {
 public:
  explicit ContextHandle(detail::ContextImpl& context) noexcept
      : context_(&context), pipelineGuard_(context.pipeline()) {}

  ContextHandle(const ContextHandle&) = delete;
  ContextHandle& operator=(const ContextHandle&) = delete;
  ContextHandle& operator=(ContextHandle&&) = delete;

  ContextHandle(ContextHandle&& other) noexcept
      : context_(std::exchange(other.context_, nullptr)),
        pipelineGuard_(std::move(other.pipelineGuard_)) {}

  ~ContextHandle() {
    CHECK(context_ == nullptr || context_->eventBase()->isInEventBaseThread())
        << "A live ContextHandle can only be destroyed on its pipeline's "
           "EventBase thread";
  }

  void fireRead(TypeErasedBox&& message) && noexcept {
    std::move(*this).runOnEventBase(
        [message = std::move(message)](
            detail::ContextImpl& context, bool closed) mutable {
          if (!closed) {
            (void)context.fireRead(std::move(message));
          }
        });
  }

  void fireWrite(TypeErasedBox&& message) && noexcept {
    std::move(*this).runOnEventBase(
        [message = std::move(message)](
            detail::ContextImpl& context, bool closed) mutable {
          if (!closed) {
            (void)context.fireWrite(std::move(message));
          }
        });
  }

  void fireException(folly::exception_wrapper&& exception) && noexcept {
    std::move(*this).runOnEventBase(
        [exception = std::move(exception)](
            detail::ContextImpl& context, bool closed) mutable {
          if (!closed) {
            context.fireException(std::move(exception));
          }
        });
  }

 private:
  template <typename Func>
  bool runInlineIfOnEventBase(Func&& func) && noexcept {
    if (!context_->eventBase()->inRunningEventBaseThread()) {
      return false;
    }

    auto handle = std::move(*this);
    func(*handle.context_, handle.context_->pipeline()->isClosed());
    return true;
  }

  template <typename Func>
  void runOnEventBase(Func&& func) && noexcept {
    context_->eventBase()->runInEventBaseThread(
        [handle = std::move(*this),
         func = std::forward<Func>(func)]() mutable noexcept {
          func(*handle.context_, handle.context_->pipeline()->isClosed());
        });
  }

  detail::ContextImpl* context_;
  folly::DelayedDestruction::DestructorGuard pipelineGuard_;

  friend class coro::ContextHandle;
  friend class coro::ReadAwaitable;
  friend class coro::WriteAwaitable;
  friend class coro::ExceptionAwaitable;
};

static_assert(!std::is_copy_constructible_v<ContextHandle>);
static_assert(!std::is_copy_assignable_v<ContextHandle>);
static_assert(std::is_nothrow_move_constructible_v<ContextHandle>);
static_assert(!std::is_move_assignable_v<ContextHandle>);

} // namespace apache::thrift::fast_thrift::channel_pipeline
