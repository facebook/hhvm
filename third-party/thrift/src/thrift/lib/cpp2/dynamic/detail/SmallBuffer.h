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

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>
#include <folly/CppAttributes.h>

namespace apache::thrift::dynamic::detail {

// Helper type to store polymorphic member without allocating.
// Supports both trivial and non-trivial types via type-erased operations.
// SupportsNonTrivial: if false, only trivially copyable/destructible types
// allowed (more efficient)
//                     if true, non-trivial types are supported via vtable
//                     (needed for MSVC)
template <
    size_t InlineSize,
    size_t Alignment = alignof(void*),
    bool SupportsNonTrivial = false>
struct SmallBuffer {
  // Access
  template <typename T>
  T& as() {
    checkInvariants<T>();
    return *reinterpret_cast<T*>(inline_);
  }
  template <typename T>
  const T& as() const {
    checkInvariants<T>();
    return *reinterpret_cast<const T*>(inline_);
  }

  // Lifecycle
  template <typename T, typename... Args>
  T& emplace(Args&&... args) {
    checkInvariants<T>();
    if constexpr (SupportsNonTrivial) {
      setOps<T>();
    }
    return *new (inline_) T(std::forward<Args>(args)...);
  }

  SmallBuffer() {
    if constexpr (SupportsNonTrivial) {
      ops_ = &kTrivialOps;
    }
  }

  SmallBuffer(SmallBuffer const& other) {
    if constexpr (SupportsNonTrivial) {
      ops_ = other.ops_;
      if (ops_->copy) {
        ops_->copy(inline_, other.inline_);
      } else {
        std::memcpy(inline_, other.inline_, InlineSize);
      }
    } else {
      std::memcpy(inline_, other.inline_, InlineSize);
    }
  }

  SmallBuffer& operator=(SmallBuffer const& other) {
    if (this != &other) {
      if constexpr (SupportsNonTrivial) {
        destroy();
        ops_ = other.ops_;
        if (ops_->copy) {
          ops_->copy(inline_, other.inline_);
        } else {
          std::memcpy(inline_, other.inline_, InlineSize);
        }
      } else {
        std::memcpy(inline_, other.inline_, InlineSize);
      }
    }
    return *this;
  }

  ~SmallBuffer() {
    if constexpr (SupportsNonTrivial) {
      destroy();
    }
  }

 private:
  struct Ops {
    void (*destroy)(void*);
    void (*copy)(void* dst, const void* src);
  };

  static constexpr Ops kTrivialOps{nullptr, nullptr};

  alignas(Alignment) std::byte inline_[InlineSize];
  [[FOLLY_ATTR_NO_UNIQUE_ADDRESS]]
  std::conditional_t<SupportsNonTrivial, const Ops*, std::nullptr_t> ops_{};

  void destroy() {
    if constexpr (SupportsNonTrivial) {
      if (ops_->destroy) {
        ops_->destroy(inline_);
      }
    }
  }

  template <typename T>
  void setOps() {
    if constexpr (SupportsNonTrivial) {
      if constexpr (
          std::is_trivially_destructible_v<T> &&
          std::is_trivially_copyable_v<T>) {
        ops_ = &kTrivialOps;
      } else {
        static constexpr Ops kOps = {
            // destroy
            [](void* p) { static_cast<T*>(p)->~T(); },
            // copy
            [](void* dst, const void* src) {
              new (dst) T(*static_cast<const T*>(src));
            }};
        ops_ = &kOps;
      }
    }
  }

  template <typename T>
  static constexpr void checkInvariants() {
    static_assert(sizeof(T) <= InlineSize);
    static_assert(alignof(T) <= Alignment);
    if constexpr (!SupportsNonTrivial) {
      static_assert(std::is_trivially_destructible_v<T>);
      static_assert(std::is_trivially_copyable_v<T>);
    }
  }
};

// F14 container iterators are small and trivial on linux/mac, but large and
// nontrivial on windows.
using F14IteratorBuffer = std::conditional_t<
    folly::kMscVer != 0,
    detail::SmallBuffer</*InlineSize=*/64,
                        /*Alignment=*/alignof(void*),
                        /*SupportsNonTrivial=*/true>,
    detail::SmallBuffer<16>>;

} // namespace apache::thrift::dynamic::detail
