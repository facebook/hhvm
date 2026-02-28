/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <folly/Range.h>

#include "mcrouter/lib/fbi/cpp/util.h"

namespace facebook {
namespace memcache {

/**
 * Ref is a reference counting wrapper with value semantics.
 *
 * Copy ctor/assignment is disabled.
 * Instead, copies must be obtained explicitly:
 *   Ref a;
 *   Ref b = a.clone();
 * This is done to emphasize creation of new references in code and force
 * move semantics as the default.
 */
template <class T, class RefPolicy>
class Ref {
  static_assert(
      noexcept(RefPolicy::increfOrNull),
      "RefPolicy::increfOrNull should be noexcept");
  static_assert(
      noexcept(RefPolicy::decref),
      "RefPolicy::decref should be noexcept");

 public:
  /**
   * Construct an empty Ref.
   */
  Ref() = default;

  /**
   * Moves in the provided pointer (no reference count changes).
   */
  static Ref moveRef(T* ref) noexcept {
    return Ref(ref);
  }

  /**
   * Clones the reference (bumps the reference count)
   */
  static Ref cloneRef(T* ref) noexcept {
    return Ref(RefPolicy::increfOrNull(ref));
  }

  Ref(Ref&& from) noexcept : ref_(from.ref_) {
    from.ref_ = nullptr;
  }

  template <typename M, typename D>
  /* implicit */ Ref(std::unique_ptr<M, D>&& from) noexcept
      : ref_(from.release()) {
    static_assert(
        std::is_same<D, typename RefPolicy::Deleter>::value,
        "unique_ptr deleter is not compatible with RefPolicy");
  }

  Ref& operator=(Ref&& from) noexcept {
    if (this != &from) {
      RefPolicy::decref(ref_);
      ref_ = from.ref_;
      from.ref_ = nullptr;
    }
    return *this;
  }

  template <typename M, typename D>
  Ref& operator=(std::unique_ptr<M, D>&& from) noexcept {
    static_assert(
        std::is_same<D, typename RefPolicy::Deleter>::value,
        "unique_ptr deleter is not compatible with RefPolicy");

    RefPolicy::decref(ref_);
    ref_ = from.release();
    return *this;
  }

  /**
   * Explicitly obtains a new reference to the managed object.
   */
  Ref clone() const noexcept {
    return Ref(RefPolicy::increfOrNull(ref_));
  }

  Ref(const Ref& other) = delete;
  Ref& operator=(const Ref& other) = delete;

  /**
   * Access to the managed object
   */
  T* operator->() const noexcept {
    return ref_;
  }
  T* get() const noexcept {
    return ref_;
  }
  T& operator*() const noexcept {
    return *ref_;
  }

  /**
   * Releases the managed object
   *
   * @return pointer to the managed object; the caller
   *         is responsible for managing reference count
   *         after the call.
   */
  T* release() noexcept {
    auto t = ref_;
    ref_ = nullptr;
    return t;
  }

  /**
   * reset the managed object
   */
  void reset() noexcept {
    if (ref_) {
      RefPolicy::decref(ref_);
      ref_ = nullptr;
    }
  }

  ~Ref() noexcept {
    RefPolicy::decref(ref_);
  }

 private:
  T* ref_{nullptr};

  explicit Ref(T* ref) noexcept : ref_(ref) {}
};
} // namespace memcache
} // namespace facebook
