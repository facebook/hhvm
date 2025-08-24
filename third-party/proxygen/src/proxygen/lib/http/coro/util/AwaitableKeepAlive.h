/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>
#include <folly/logging/xlog.h>

namespace proxygen::coro::detail {

// forward decls
template <class T>
class EnableAwaitableKeepAlive;

template <class T>
class KeepAlivePtr;

/**
 * HTTPCoroSession enables KeepAlive semantics; destruction of the session is
 * owned by HTTPCoroSession itself, however it allows users to block destruction
 * until all acquired KeepAlives are released.
 *
 * KeepAliveState is internal to both EnableAwaitableKeepAlive (owns the state)
 * and KeepAlivePtr (holds a pointer to state); Consequently all members are
 * private with EnableAwaitableKeepAlive & KeepAlivePtr declared as friend
 * classes
 */
template <class T>
struct KeepAliveState {
 private:
  friend class EnableAwaitableKeepAlive<T>;
  friend class KeepAlivePtr<T>;

  void keepAliveAcquired() {
    uint64_t prevCount = refCount_.fetch_add(1, std::memory_order_relaxed);
    // 0->1 refcount is a logical uaf; abort
    XCHECK_GE(prevCount, 1ull) << "use after free";
  }

  void keepAliveReleased() {
    uint64_t prevCount = refCount_.fetch_sub(1, std::memory_order_relaxed);
    XCHECK_GE(prevCount, 1ull) << "underflow";
    if (prevCount == 1) {
      refsBaton_.post();
    }
  }

  uint64_t getRefCount() {
    return refCount_.load();
  }

  T& ref() {
    return ref_;
  }
  const T& ref() const {
    return ref_;
  }

  explicit KeepAliveState(T& ref) : ref_(ref) {
  }
  ~KeepAliveState() = default;
  /**
   * Note that `refCount_` is initially 1 here. This is effectively an implicit
   * KeepAlive token automatically acquired on construction.
   * EnableAwaitableKeepAlive::zeroRefs releases this token; this mechanism
   * enables us to treat a 0->1 refcount increase as a logical use after free.
   */
  T& ref_;
  std::atomic_uint64_t refCount_{1};
  folly::coro::Baton refsBaton_;
};

template <class T>
class KeepAlivePtr {
 public:
  KeepAlivePtr() noexcept = default;

  // move constructor, no ref count change
  KeepAlivePtr(KeepAlivePtr&& other) noexcept
      : keepAliveState_(other.keepAliveState_) {
    other.keepAliveState_ = nullptr;
  }

  KeepAlivePtr(const KeepAlivePtr& other) noexcept
      : keepAliveState_(other.keepAliveState_) {
    if (keepAliveState_) {
      keepAliveState_->keepAliveAcquired();
    }
  }

  KeepAlivePtr& operator=(KeepAlivePtr&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    reset();
    keepAliveState_ = std::exchange(other.keepAliveState_, nullptr);
    return *this;
  }

  KeepAlivePtr& operator=(const KeepAlivePtr& other) noexcept {
    if (this == &other) {
      return *this;
    }
    reset();
    if ((keepAliveState_ = other.keepAliveState_)) {
      keepAliveState_->keepAliveAcquired();
    }
    return *this;
  }

  ~KeepAlivePtr() noexcept {
    // decrement ref count
    reset();
  }

  // getters
  T* get() {
    return keepAliveState_ ? &keepAliveState_->ref() : nullptr;
  }
  T* operator->() {
    return get();
  }
  // const getters
  const T* get() const {
    return keepAliveState_ ? &keepAliveState_->ref() : nullptr;
  }
  const T* operator->() const {
    return get();
  }

  operator bool() const {
    return get() != nullptr;
  }

  // reset
  void reset() {
    if (auto* ka = std::exchange(keepAliveState_, nullptr)) {
      ka->keepAliveReleased();
    }
  }

 private:
  friend class EnableAwaitableKeepAlive<T>;
  explicit KeepAlivePtr(KeepAliveState<T>* keepAliveState)
      : keepAliveState_(CHECK_NOTNULL(keepAliveState)) {
    // increment refcount
    keepAliveState_->keepAliveAcquired();
  }

  KeepAliveState<T>* keepAliveState_{nullptr};
};

template <class T>
class EnableAwaitableKeepAlive {
 public:
  KeepAlivePtr<T> acquireKeepAlive() {
    return KeepAlivePtr<T>{&keepAliveState_};
  }

  uint64_t numKeepAlives() {
    return keepAliveState_.getRefCount();
  }

 protected:
  /**
   * TODO(@damlaj):
   *
   * This no_sanitize(vptr) attribute is here to prevent ubsan builds from
   * raising false positive errors; see S452115
   */
#if defined(__clang__)
  __attribute__((no_sanitize("vptr")))
#endif
  EnableAwaitableKeepAlive()
      : keepAliveState_{*static_cast<T*>(this)} {
  }
  virtual ~EnableAwaitableKeepAlive() = default;

  folly::coro::Task<void> zeroRefs() {
    // static_assert must be inside function definition otherwise incomplete
    // type error
    static_assert(
        std::is_base_of_v</*Base=*/EnableAwaitableKeepAlive<T>, /*Derived=*/T>);

    // we release the implicit KeepAlive token first
    keepAliveState_.keepAliveReleased();

    co_await keepAliveState_.refsBaton_;
  }

 private:
  KeepAliveState<T> keepAliveState_{nullptr};
};

} // namespace proxygen::coro::detail
