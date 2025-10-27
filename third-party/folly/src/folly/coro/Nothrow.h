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

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h> // FOLLY_HAS_COROUTINES
#include <folly/coro/ViaIfAsync.h>

// For DCHECK, `ViaIfAsync.h` depends on it already.
#include <glog/logging.h>

#if FOLLY_HAS_COROUTINES

namespace folly::coro {

namespace detail {

template <typename T>
class [[FOLLY_ATTR_CLANG_CORO_AWAIT_ELIDABLE]] NothrowAwaitable;

// The `!noexcept_awaitable_v` constraint stops `co_nothrow()` from wrapping
// `value_or_error_or_stopped`, `co_awaitTry`, `AsNoexcept`, etc.
//
// Rationale: Instead, we could do:
//   - (not very useful) Nothing -- modeling the behavior that the exception
//     was already captured by the inner type.
//   - (highly unexpected / buggy) Prevent the inner type from capturing the
//     exception, and force its propagation up-stack.
// This was banned because real-world users didn't uniformly expect one of the
// behaviors over the other, and nobody really **needs** this to work.
template <typename T>
class [[FOLLY_ATTR_CLANG_CORO_AWAIT_ELIDABLE]]
NothrowAwaitable : public CommutativeWrapperAwaitable<NothrowAwaitable, T> {
 public:
  using CommutativeWrapperAwaitable<NothrowAwaitable, T>::
      CommutativeWrapperAwaitable;

  template <
      typename T2 = T,
      std::enable_if_t<!noexcept_awaitable_v<T>, int> = 0>
  T2&& unwrap() {
    return std::move(this->inner_);
  }
};

template <typename>
class ValueOrError;
template <typename>
class ValueOrErrorImpl;

// Mixin supporting `co_nothrow()` and `value_or_error()` for tasks & generators
class BypassExceptionThrowing {
 private:
  enum class BypassMode : uint8_t {
    // Default state for `co_await task()`.  The corresponding
    // `await_transform` calls must call `maybeActivate()`, which will either
    // promote `REQUESTED` to `ACTIVE`, or reset any `ACTIVE` state from a
    // prior suspension point back to `INACTIVE`.
    INACTIVE,
    // State after `await_transform` with a `co_nothrow` awaitable.
    ACTIVE,
    // State before `await_transform` with a `co_nothrow` awaitable.
    REQUESTED,
    // State for `value_or_error()` only allowing for nothrow-propagation of
    // `OperationCancelled` to the parent.
    ONLY_WHEN_OPERATION_CANCELLED,
  } bypassMode_{BypassMode::INACTIVE};

 protected:
  // This write interface is protected to alert future authors -- this is a
  // TIGHTLY COUPLED DETAIL.  See the analogous note in `BasePromise`.
  template <typename>
  friend class BasePromise;

  template <typename Awaitable>
  void maybeActivate() {
    if (is_instantiation_of_v<ValueOrErrorImpl, Awaitable>) {
      DCHECK(bypassMode_ == BypassMode::ONLY_WHEN_OPERATION_CANCELLED);
    } else {
      // Awaitable should've been unwrapped before getting here.
      static_assert(
          !is_instantiation_of_v<NothrowAwaitable, Awaitable> &&
          !is_instantiation_of_v<ValueOrError, Awaitable>);
      bypassMode_ = bypassMode_ == BypassMode::REQUESTED
          ? BypassMode::ACTIVE
          : BypassMode::INACTIVE;
    }
  }

  // Implements `co_nothrow` -- this gets called only from the matching
  // `await_transform(NothrowAwaitable<Awaitable>)`.  The subsequent
  // `await_transform(Awaitable)` maps `REQUESTED` to `ACTIVE.
  template <typename Awaitable>
  void requestDueToNothrow() {
    // `co_nothrow` is incompatible with noexcept-awaitables, doc above.
    static_assert(!noexcept_awaitable_v<Awaitable>);
    bypassMode_ = BypassMode::REQUESTED;
  }

  void requestDueToValueOrError() {
    bypassMode_ = BypassMode::ONLY_WHEN_OPERATION_CANCELLED;
  }

 public: // Otherwise we'd also need to friend `AsyncGenerator`, etc
  bool shouldBypassFor(exception_wrapper& ex) {
    return (bypassMode_ == BypassMode::ACTIVE) ||
        (bypassMode_ == BypassMode::ONLY_WHEN_OPERATION_CANCELLED &&
         // Today, this incurs RTTI cost, but once we migrate coros to use
         // `rich_exception_ptr`, this will often be RTTI-free.
         ex.get_exception<folly::OperationCancelled>());
  }
};

} // namespace detail

template <
    typename Awaitable,
    std::enable_if_t<
        !noexcept_awaitable_v<Awaitable> && // Comment on `NothrowAwaitable`
            !folly::ext::must_use_immediately_v<Awaitable>,
        int> = 0>
detail::NothrowAwaitable<remove_cvref_t<Awaitable>> co_nothrow(
    [[FOLLY_ATTR_CLANG_CORO_AWAIT_ELIDABLE_ARGUMENT]] Awaitable&& awaitable) {
  return detail::NothrowAwaitable<remove_cvref_t<Awaitable>>{
      static_cast<Awaitable&&>(awaitable)};
}
template <
    typename Awaitable,
    std::enable_if_t<
        !noexcept_awaitable_v<Awaitable> && // Comment on `NothrowAwaitable`
            folly::ext::must_use_immediately_v<Awaitable>,
        int> = 0>
detail::NothrowAwaitable<remove_cvref_t<Awaitable>> co_nothrow(
    [[FOLLY_ATTR_CLANG_CORO_AWAIT_ELIDABLE_ARGUMENT]] Awaitable awaitable) {
  return detail::NothrowAwaitable<remove_cvref_t<Awaitable>>{
      folly::ext::must_use_immediately_unsafe_mover(std::move(awaitable))()};
}

} // namespace folly::coro

#endif // FOLLY_HAS_COROUTINES
