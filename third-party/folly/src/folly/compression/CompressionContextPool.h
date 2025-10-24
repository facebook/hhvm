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

#include <memory>
#include <optional>

#include <folly/Memory.h>
#include <folly/Synchronized.h>

/**
 * Temporary implementation detail:
 *
 * The compression context pool singletons are actually used during static
 * initialization time by other modules, so those singletons need to avoid
 * SIOF issues. They do that by being marked `constinit`, so that they have
 * already been constructed before static initialization / program execution
 * begins. That means that the context pools need `constexpr` constructors.
 * `std::vector` is supposed to have a `constexpr` constructor in C++20, but
 * we don't seem to have it (everywhere).
 *
 * So while we can't rely on `constexpr std::vector::vector()`, we wrap the
 * vector with a `std::optional`. When we can `constexpr` construct the vector,
 * we do so. Otherwise, we construct it as empty and have to add checks to
 * populate it later.
 *
 * When folly is only being built on platforms that have this, we can remove
 * the `std::optional` shim etc.
 */
#ifndef FOLLY_COMPRESSION_HAS_CONSTEXPR_VECTOR
#if defined(__cpp_lib_constexpr_vector) && \
    __cpp_lib_constexpr_vector >= 201907L && !defined(_MSC_VER)
#define FOLLY_COMPRESSION_HAS_CONSTEXPR_VECTOR 1
#else
#define FOLLY_COMPRESSION_HAS_CONSTEXPR_VECTOR 0
#endif
#endif

namespace folly {
namespace compression {

struct CompressionContextPoolDefaultCallback {
  void operator()() const {}
};

constexpr size_t COMPRESSION_CONTEXT_POOL_CALLBACK_INTERVAL = 1024;

/**
 * This implementation is slow under contention. Except under uncontended
 * scenarios, you shouldn't use it directly. You likely want to use the
 * CompressionCoreLocalContextPool instead, which, behind the fast cache slots,
 * is backed by this implementation.
 */
template <
    typename T,
    typename Creator,
    typename Deleter,
    typename Resetter,
    typename Sizeof,
    typename Callback = CompressionContextPoolDefaultCallback>
class CompressionContextPool {
 private:
  using InternalRef = std::unique_ptr<T, Deleter>;

  class ReturnToPoolDeleter {
   public:
    using Pool =
        CompressionContextPool<T, Creator, Deleter, Resetter, Sizeof, Callback>;

    explicit ReturnToPoolDeleter(Pool* pool) : pool_(pool) { DCHECK(pool); }

    void operator()(T* t) {
      InternalRef ptr(t, pool_->deleter_);
      pool_->add(std::move(ptr));
    }

   private:
    Pool* pool_;
  };

 public:
  using Object = T;
  using Ref = std::unique_ptr<T, ReturnToPoolDeleter>;

  constexpr explicit CompressionContextPool(
      Creator creator = Creator(),
      Deleter deleter = Deleter(),
      Resetter resetter = Resetter(),
      Sizeof size_of = Sizeof(),
      Callback callback = Callback())
      : creator_(std::move(creator)),
        deleter_(std::move(deleter)),
        resetter_(std::move(resetter)),
        size_of_(std::move(size_of)),
        callback_(std::move(callback)),
        state_(),
        created_(0) {}

  Ref get() {
    bool do_cb = false;
    Ref ref{nullptr, get_deleter()};
    {
      auto lock = state_.wlock();
#if !FOLLY_COMPRESSION_HAS_CONSTEXPR_VECTOR
      if (!lock->stack_) {
        lock->stack_.emplace();
      }
#endif
      auto& stack = *lock->stack_;
      if (!stack.empty()) {
        auto ptr = std::move(stack.back());
        stack.pop_back();
        do_cb = (lock->callback_counter_++ %
                 COMPRESSION_CONTEXT_POOL_CALLBACK_INTERVAL) == 0;
        if (!ptr) {
          throw_exception<std::logic_error>(
              "A nullptr snuck into our context pool!?!?");
        }
        ref = Ref(ptr.release(), get_deleter());
      }
    }
    if (do_cb) {
      callback_();
    }
    if (!ref) {
      ref = create();
    }
    return ref;
  }

  size_t created_count() const { return created_.load(); }

  size_t size() {
    auto lock = state_.rlock();
#if !FOLLY_COMPRESSION_HAS_CONSTEXPR_VECTOR
    if (!lock->stack_) {
      return 0;
    }
#endif
    auto& stack = *lock->stack_;
    return stack.size();
  }

  ReturnToPoolDeleter get_deleter() { return ReturnToPoolDeleter(this); }

  const Resetter& get_resetter() { return resetter_; }

  void flush_deep() {
    flush_shallow();
    // no backing stack, so deep == shallow
  }

  void flush_shallow() {
    auto lock = state_.wlock();
#if !FOLLY_COMPRESSION_HAS_CONSTEXPR_VECTOR
    if (!lock->stack_) {
      return;
    }
#endif
    auto& stack = *lock->stack_;
    stack.resize(0);
  }

 private:
  void add(InternalRef ptr) {
    DCHECK(ptr);
    resetter_(ptr.get());
    auto lock = state_.wlock();
    // add() can only be called when we get a ref we created back, so the
    // stack must already have been initialized. So we don't need to check.
    auto& stack = *lock->stack_;
    stack.push_back(std::move(ptr));
  }

  Ref create() {
    T* t = creator_();
    if (t == nullptr) {
      throw_exception<std::bad_alloc>();
    }
    created_++;
    return Ref(t, get_deleter());
  }

  const Creator creator_;
  const Deleter deleter_;
  const Resetter resetter_;
  const Sizeof size_of_;
  const Callback callback_;

  struct SyncState {
    explicit constexpr SyncState()
        :
#if FOLLY_COMPRESSION_HAS_CONSTEXPR_VECTOR
          stack_(std::in_place, std::vector<InternalRef>{}),
#else
          stack_(),
#endif
          callback_counter_(0) {
    }

    std::optional<std::vector<InternalRef>> stack_;
    size_t callback_counter_;
  };

  folly::Synchronized<SyncState> state_;

  std::atomic<size_t> created_;
};
} // namespace compression
} // namespace folly
