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

#include <atomic>

#include <folly/portability/Asm.h>

namespace folly {

//  asymmetric_thread_fence_light
//
//  A lightweight fence interacts with a heavyweight fence as if both fences
//  were normal std::atomic_thread_fence fences.
//
//  A lightweight fence does not interact with another lightweight fence, with a
//  normal fence, with a signal fence, or directly with an atomic operation in
//  another thread.
//
//  https://en.cppreference.com/w/cpp/atomic/atomic_thread_fence.html
//  https://en.cppreference.com/w/cpp/atomic/atomic_signal_fence.html
//
//  mimic: std::experimental::asymmetric_thread_fence_light, p1202r4
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1202r4.pdf
struct asymmetric_thread_fence_light_fn {
  FOLLY_ALWAYS_INLINE void operator()(std::memory_order order) const noexcept {
    if (kIsLinux) {
      asm_volatile_memory();
    } else {
      std::atomic_thread_fence(order);
    }
  }
};
inline constexpr asymmetric_thread_fence_light_fn
    asymmetric_thread_fence_light{};

//  asymmetric_thread_fence_heavy
//
//  A heavyweight fence interacts with a heavyweight, lightweight, or normal
//  fence as if both fences were normal std::atomic_thread_fence fences, and
//  interacts directly with an atomic operation as if it were a normal fence.
//
//  A heavyweight fence interacts with a signal fence as if both fences were
//  signal fences, just as a normal fence would do.
//
//  https://en.cppreference.com/w/cpp/atomic/atomic_thread_fence.html
//  https://en.cppreference.com/w/cpp/atomic/atomic_signal_fence.html
//
//  mimic: std::experimental::asymmetric_thread_fence_heavy, p1202r4
//  http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1202r4.pdf
struct asymmetric_thread_fence_heavy_fn {
  FOLLY_ALWAYS_INLINE void operator()(std::memory_order order) const noexcept {
    if (kIsLinux) {
      impl_(order);
    } else {
      std::atomic_thread_fence(order);
    }
  }

 private:
  static void impl_(std::memory_order) noexcept;
};
inline constexpr asymmetric_thread_fence_heavy_fn
    asymmetric_thread_fence_heavy{};

} // namespace folly
