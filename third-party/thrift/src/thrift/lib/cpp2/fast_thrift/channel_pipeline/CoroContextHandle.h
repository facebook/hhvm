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

#include <folly/CancellationToken.h>
#include <folly/Executor.h>
#include <folly/Try.h>
#include <folly/coro/Coroutine.h>
#include <folly/coro/Traits.h>
#include <folly/coro/WithAsyncStack.h>
#include <folly/io/async/Request.h>
#include <folly/lang/CustomizationPoint.h>
#include <folly/tracing/AsyncStack.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/ContextHandle.h>

namespace apache::thrift::fast_thrift::channel_pipeline::coro {

namespace detail {

template <typename Promise>
struct AsyncFrameCapture {
  folly::AsyncStackFrame* frame{nullptr};
  folly::AsyncStackRoot* root{nullptr};

  static AsyncFrameCapture capture(folly::coro::coroutine_handle<Promise> h) {
    AsyncFrameCapture cap;
    if constexpr (!std::is_void_v<Promise>) {
      if constexpr (requires { h.promise().getAsyncFrame(); }) {
        cap.frame = &h.promise().getAsyncFrame();
        cap.root = cap.frame->getStackRoot();
        folly::deactivateAsyncStackFrame(*cap.frame);
      }
    }
    return cap;
  }

  void resumeWithContext(
      folly::coro::coroutine_handle<Promise> h,
      std::shared_ptr<folly::RequestContext> savedCtx) const noexcept {
    folly::RequestContextScopeGuard guard(std::move(savedCtx));
    if (frame) {
      folly::resumeCoroutineWithNewAsyncStackRoot(h, *frame);
    } else {
      h.resume();
    }
  }

  void restoreOnThrow() const noexcept {
    if (frame && root) {
      folly::activateAsyncStackFrame(*root, *frame);
    }
  }
};

} // namespace detail

class ReadAwaitable final {
 public:
  ReadAwaitable(
      channel_pipeline::ContextHandle handle, TypeErasedBox message) noexcept
      : handle_(std::move(handle)), message_(std::move(message)) {}

  /** Preserve the awaiting Task's executor after EventBase completion. */
  ReadAwaitable&& viaIfAsync(
      folly::Executor::KeepAlive<> executor) && noexcept {
    executor_ = std::move(executor);
    return std::move(*this);
  }

  bool await_ready() noexcept {
    return std::move(handle_).runInlineIfOnEventBase(
        [this](channel_pipeline::detail::ContextImpl& context, bool closed) {
          result_ =
              closed ? Result::Error : context.fireRead(std::move(message_));
        });
  }

  template <typename Promise>
  bool await_suspend(folly::coro::coroutine_handle<Promise> h) noexcept {
    auto cap = detail::AsyncFrameCapture<Promise>::capture(h);
    auto savedCtx = folly::RequestContext::saveContext();
    std::move(handle_).runOnEventBase(
        [this, h, cap, savedCtx = std::move(savedCtx)](
            channel_pipeline::detail::ContextImpl& ctxImpl,
            bool closed) mutable noexcept {
          result_ =
              closed ? Result::Error : ctxImpl.fireRead(std::move(message_));
          auto* eb = ctxImpl.eventBase();
          if (executor_ && executor_.get() == eb) {
            cap.resumeWithContext(h, std::move(savedCtx));
            return;
          }
          if (executor_) {
            try {
              executor_->add(
                  [h, cap, savedCtx = std::move(savedCtx)]() mutable noexcept {
                    cap.resumeWithContext(h, std::move(savedCtx));
                  });
              return;
            } catch (...) {
              cap.restoreOnThrow();
            }
          }
          cap.resumeWithContext(h, std::move(savedCtx));
        });
    return true;
  }

  Result await_resume() const noexcept { return result_; }
  folly::Try<Result> await_resume_try() const noexcept {
    return folly::Try<Result>(result_);
  }

  // Trusted awaitable: we handle async-stack + cancellation ourselves,
  // so folly::coro::Task doesn't wrap us in ViaIfAsync/WithAsyncStack.
  friend ReadAwaitable tag_invoke(
      folly::cpo_t<folly::coro::co_withAsyncStack>,
      ReadAwaitable&& self) noexcept {
    return std::move(self);
  }
  friend ReadAwaitable co_withCancellation(
      const folly::CancellationToken&, ReadAwaitable&& self) noexcept {
    return std::move(self);
  }
  friend ReadAwaitable co_withCancellation(
      folly::CancellationToken&&, ReadAwaitable&& self) noexcept {
    return std::move(self);
  }

 private:
  channel_pipeline::ContextHandle handle_;
  TypeErasedBox message_;
  folly::Executor::KeepAlive<> executor_;
  Result result_{Result::Error};
};

class WriteAwaitable final {
 public:
  WriteAwaitable(
      channel_pipeline::ContextHandle handle, TypeErasedBox message) noexcept
      : handle_(std::move(handle)), message_(std::move(message)) {}

  WriteAwaitable&& viaIfAsync(
      folly::Executor::KeepAlive<> executor) && noexcept {
    executor_ = std::move(executor);
    return std::move(*this);
  }

  bool await_ready() noexcept {
    return std::move(handle_).runInlineIfOnEventBase(
        [this](channel_pipeline::detail::ContextImpl& context, bool closed) {
          result_ =
              closed ? Result::Error : context.fireWrite(std::move(message_));
        });
  }

  template <typename Promise>
  bool await_suspend(folly::coro::coroutine_handle<Promise> h) noexcept {
    auto cap = detail::AsyncFrameCapture<Promise>::capture(h);
    auto savedCtx = folly::RequestContext::saveContext();
    std::move(handle_).runOnEventBase(
        [this, h, cap, savedCtx = std::move(savedCtx)](
            channel_pipeline::detail::ContextImpl& ctxImpl,
            bool closed) mutable noexcept {
          result_ =
              closed ? Result::Error : ctxImpl.fireWrite(std::move(message_));
          auto* eb = ctxImpl.eventBase();
          if (executor_ && executor_.get() == eb) {
            cap.resumeWithContext(h, std::move(savedCtx));
            return;
          }
          if (executor_) {
            try {
              executor_->add(
                  [h, cap, savedCtx = std::move(savedCtx)]() mutable noexcept {
                    cap.resumeWithContext(h, std::move(savedCtx));
                  });
              return;
            } catch (...) {
              cap.restoreOnThrow();
            }
          }
          cap.resumeWithContext(h, std::move(savedCtx));
        });
    return true;
  }

  Result await_resume() const noexcept { return result_; }
  folly::Try<Result> await_resume_try() const noexcept {
    return folly::Try<Result>(result_);
  }

  friend WriteAwaitable tag_invoke(
      folly::cpo_t<folly::coro::co_withAsyncStack>,
      WriteAwaitable&& self) noexcept {
    return std::move(self);
  }
  friend WriteAwaitable co_withCancellation(
      const folly::CancellationToken&, WriteAwaitable&& self) noexcept {
    return std::move(self);
  }
  friend WriteAwaitable co_withCancellation(
      folly::CancellationToken&&, WriteAwaitable&& self) noexcept {
    return std::move(self);
  }

 private:
  channel_pipeline::ContextHandle handle_;
  TypeErasedBox message_;
  folly::Executor::KeepAlive<> executor_;
  Result result_{Result::Error};
};

class ExceptionAwaitable final {
 public:
  ExceptionAwaitable(
      channel_pipeline::ContextHandle handle,
      folly::exception_wrapper exception) noexcept
      : handle_(std::move(handle)), exception_(std::move(exception)) {}

  ExceptionAwaitable&& viaIfAsync(
      folly::Executor::KeepAlive<> executor) && noexcept {
    executor_ = std::move(executor);
    return std::move(*this);
  }

  bool await_ready() noexcept {
    return std::move(handle_).runInlineIfOnEventBase(
        [this](channel_pipeline::detail::ContextImpl& context, bool closed) {
          if (!closed) {
            context.fireException(std::move(exception_));
          }
        });
  }

  template <typename Promise>
  bool await_suspend(folly::coro::coroutine_handle<Promise> h) noexcept {
    auto cap = detail::AsyncFrameCapture<Promise>::capture(h);
    auto savedCtx = folly::RequestContext::saveContext();
    std::move(handle_).runOnEventBase(
        [this, h, cap, savedCtx = std::move(savedCtx)](
            channel_pipeline::detail::ContextImpl& ctxImpl,
            bool closed) mutable noexcept {
          if (!closed) {
            ctxImpl.fireException(std::move(exception_));
          }
          auto* eb = ctxImpl.eventBase();
          if (executor_ && executor_.get() == eb) {
            cap.resumeWithContext(h, std::move(savedCtx));
            return;
          }
          if (executor_) {
            try {
              executor_->add(
                  [h, cap, savedCtx = std::move(savedCtx)]() mutable noexcept {
                    cap.resumeWithContext(h, std::move(savedCtx));
                  });
              return;
            } catch (...) {
              cap.restoreOnThrow();
            }
          }
          cap.resumeWithContext(h, std::move(savedCtx));
        });
    return true;
  }

  void await_resume() const noexcept {}
  folly::Try<void> await_resume_try() const noexcept {
    return folly::Try<void>();
  }

  friend ExceptionAwaitable tag_invoke(
      folly::cpo_t<folly::coro::co_withAsyncStack>,
      ExceptionAwaitable&& self) noexcept {
    return std::move(self);
  }
  friend ExceptionAwaitable co_withCancellation(
      const folly::CancellationToken&, ExceptionAwaitable&& self) noexcept {
    return std::move(self);
  }
  friend ExceptionAwaitable co_withCancellation(
      folly::CancellationToken&&, ExceptionAwaitable&& self) noexcept {
    return std::move(self);
  }

 private:
  channel_pipeline::ContextHandle handle_;
  folly::exception_wrapper exception_;
  folly::Executor::KeepAlive<> executor_;
};

/** Allocation-free coroutine adapter for channel_pipeline ContextHandle. */
class ContextHandle final {
 public:
  explicit ContextHandle(
      channel_pipeline::detail::ContextImpl& context) noexcept
      : handle_(context) {}

  explicit ContextHandle(channel_pipeline::ContextHandle handle) noexcept
      : handle_(std::move(handle)) {}

  ContextHandle(const ContextHandle&) = delete;
  ContextHandle& operator=(const ContextHandle&) = delete;
  ContextHandle& operator=(ContextHandle&&) = delete;
  ContextHandle(ContextHandle&&) noexcept = default;

  ReadAwaitable co_fireRead(TypeErasedBox message) && noexcept {
    return ReadAwaitable{std::move(handle_), std::move(message)};
  }

  WriteAwaitable co_fireWrite(TypeErasedBox message) && noexcept {
    return WriteAwaitable{std::move(handle_), std::move(message)};
  }

  ExceptionAwaitable co_fireException(
      folly::exception_wrapper exception) && noexcept {
    return ExceptionAwaitable{std::move(handle_), std::move(exception)};
  }

 private:
  channel_pipeline::ContextHandle handle_;
};

static_assert(!std::is_copy_constructible_v<ContextHandle>);
static_assert(!std::is_copy_assignable_v<ContextHandle>);
static_assert(std::is_nothrow_move_constructible_v<ContextHandle>);
static_assert(!std::is_move_assignable_v<ContextHandle>);

} // namespace apache::thrift::fast_thrift::channel_pipeline::coro
