/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <mcrouter/lib/FiberLocalInternal.h>

namespace facebook::mcrouter {

/**
 * Like thread_local, but fiber compatible.  In a fiber context, every
 * active fiber has a distinct variable instance per (T, Tag) tuple.
 * Outside of a fiber, this behaves like a regular thread_local.
 *
 * T must be no-throw constructible.
 */
template <typename T, typename Tag>
class FiberLocal {
 public:
  static_assert(
      std::is_nothrow_constructible<T>::value,
      "T must be nothrow constructible");

  /**
   * @return R/W reference to this thread's or fiber's local instance.
   */
  static T& ref() {
    return *static_cast<T*>(detail::FlsRegistry::get(handle_));
  }

 private:
  FiberLocal() = delete; // not constructible

  static detail::FlsRegistry::FlsHandle handle_;

  static void construct(void* location) noexcept {
    assert(location != nullptr);
    assert(reinterpret_cast<uintptr_t>(location) % alignof(T) == 0);
    new (location) T();
  }

  static void destruct(void* location) noexcept {
    static_cast<T*>(location)->~T();
  }
};

template <typename T, typename Tag>
detail::FlsRegistry::FlsHandle FiberLocal<T, Tag>::handle_ =
    detail::FlsRegistry::registerFls(
        sizeof(T),
        alignof(T),
        &FiberLocal<T, Tag>::construct,
        &FiberLocal<T, Tag>::destruct);

/**
 * Pass FiberLocalType() to the FiberManager constructor
 */
using FiberLocalType = folly::fibers::LocalType<detail::FlsWrapper>;

} // namespace facebook::mcrouter
