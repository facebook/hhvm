/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <cstdint>

#include <folly/Likely.h>
#include <folly/fibers/FiberManager.h>

namespace facebook::mcrouter::detail {

/**
 * Registry for thread/fiber local variables.
 */
class FlsRegistry {
 public:
  using FlsHandle = std::size_t;
  using Constructor = void (*)(void* location);
  using Destructor = void (*)(void* location);

  /**
   * @return pointer to the local storage at the given offset
   */
  static void* get(FlsHandle offset) noexcept {
    auto& storage = localStorage();
    if (UNLIKELY(storage == nullptr)) {
      allocate();
    }
    assert(isSafe(offset));
    return static_cast<char*>(storage) + offset;
  }

  /**
   * Free a set of local variables. Destructors are called on each
   * variable if present.
   */
  static void freeLocalStorage(void* opaque_locals) noexcept;

  /**
   * Allocate and construct all local variables for this thread or
   * fiber.  Does nothing if this thread's or fiber's locals are
   * already allocated.
   */
  static void allocate();

  /**
   * Register a new fiber local.
   *
   * @param object_size Size of the object type to register.
   * @param object_align Alignment of the object type to register.
   * @param constructor Function to initialize the variable.
   * @param destructor Function to deinitialize the variable.
   * @return handle for the new fiber local.
   */
  static FlsHandle registerFls(
      std::size_t object_size,
      std::size_t object_align,
      Constructor constructor,
      Destructor destructor);

 private:
  FlsRegistry(); // not constructible

  /**
   * @return true if the given offset into localStorage is safe
   */
  static bool isSafe(FlsHandle offset);

  static void*& localStorage();
};

struct FlsWrapper {
  // Fibers library will inherit the local data from parent fiber, we
  // don't want this.
  FlsWrapper() : ptr(nullptr) {}
  FlsWrapper(const FlsWrapper& /*cpy*/) : ptr(nullptr) {}
  FlsWrapper& operator=(const FlsWrapper& other) = delete;

  ~FlsWrapper() {
    void* tmp_ptr = ptr;
    // We need to clear the ptr to prevent use-after-free during
    // exit. Due to undefined global destruction order across
    // translation units, we could end up calling the destructor on
    // the thread local FlsWrapper, while destroying another type
    // later on the same thread might try to access this ptr
    ptr = nullptr;
    FlsRegistry::freeLocalStorage(tmp_ptr);
  }

  void* ptr;
};

inline void*& FlsRegistry::localStorage() {
  return folly::fibers::local<detail::FlsWrapper>().ptr;
}

} // namespace facebook::mcrouter::detail
