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

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

#include <folly/CPortability.h>
#include <folly/CppAttributes.h>
#include <folly/lang/SafeAssert.h>

namespace apache::thrift::util {

/**
 * IntrusiveSharedPtr is a smart pointer that provides shared ownership
 * semantics via reference counting. The "intrusive" in the name refers to the
 * fact that the "control block" is embedded in the pointed-to object itself.
 *
 * Access to this control block is performed by the `TAccess` template parameter
 * and must be provided by the user since its implementation is specific to the
 * user-provided type.
 *
 * The pointed-to object can be in one of two states:
 *   - Ref-counted — there is at least one IntrusiveSharedPtr owner. The last
 *     owner to will delete the pointed-to object when it is destroyed.
 *   - Unmanaged — ownership is manually managed by the user, as if there were
 *     no control block. In this state, we can be sure that there are exactly
 *     zero InstrusiveSharedPtr owners pointing to this object. This state can
 * be achieved by manually allocating the object (e.g. std::make_unique<T>()) or
 *     calling IntrusiveSharedPtr::release().
 *
 * In terms of capabilities, IntrusiveSharedPtr is more flexible than
 * std::unique_ptr but not as powerful as std::shared_ptr.
 *
 * For the most part, IntrusiveSharedPtr is API-compatible with std::unique_ptr.
 *   - IntrusiveSharedPtr can be constructed or assigned from a std::unique_ptr
 *   - get(), operator->, operator* return the stored pointer (or object)
 *
 * Non-const operations involving raw pointers are generally unsafe in
 * concurrent code with shared ownership. For API compatibility with
 * std::unique_ptr, the functionality of release() is provided. However, it has
 * been renamed to unsafeRelease(). Only reset(std::nullptr_t) is provided.
 * reset() with a non-null pointer is not safe and omitted from the API.
 *
 * For unsafeRelease(), the user must ensure that there is exactly one
 * IntrusiveSharedPtr owner to the pointed-to object when it is called. In
 * concurrent code, this might be impossible to ensure. This is why it's
 * considered unsafe.
 *
 * An alternative to unsafeRelease(), named leak(), is provided.
 * This API is similar to unsafeRelease() except that the ref-count is not
 * decremented, and thus unique ownership is not required. If the ref-count is
 * not eventually decremented, the pointed-to object will never be deleted
 * (memory leak). One way to do this is call fromLeaked() on the released
 * pointer.
 *
 * The embedded control block has some implications on the capabilities:
 *   - No std::weak_ptr equivalent — the control block's lifetime is confined by
 *     the pointed-to object, which means that weak references are
 *     unimplementable.
 *   - std::enable_shared_from_this — since the control block is always
 *     accessible from the pointed-to object, it's always possible to create a
 *     new IntrusiveSharedPtr from a raw pointer which joins the existing pool
 *     of owners.
 *
 * IntrusiveSharedPtr does not support custom deleters — there is only
 * std::default_delete. However, std::default_delete may be specialized to
 * achieve custom behavior. This is not a fundamental limitation of the design
 * and support may be added in the future.
 *
 * IntrusiveSharedPtr does not support std::shared_ptr's aliasing behavior. This
 * is not a fundamental limitation of the design and support may be added in the
 * future.
 *
 * IntrusiveSharedPtr does not support array types. For arrays, a struct
 * wrapping an array can be used to achieve the same effect. However,
 * functionality like operator[] will not be available. This is not a
 * fundamental limitation of the design and support may be added in the future.
 */
template <class T, class TAccess>
class IntrusiveSharedPtr {
  static_assert(
      !std::is_array_v<T>, "Arrays are not supported by IntrusiveSharedPtr");

 public:
  using element_type = T;
  using pointer = T*;

  IntrusiveSharedPtr() noexcept = default;
  /* implicit */ IntrusiveSharedPtr(std::nullptr_t) noexcept
      : IntrusiveSharedPtr() {}

  IntrusiveSharedPtr(const IntrusiveSharedPtr& other) noexcept
      : IntrusiveSharedPtr(other.ptr_) {}
  IntrusiveSharedPtr(IntrusiveSharedPtr&& other) noexcept
      : ptr_(std::exchange(other.ptr_, nullptr)) {}

  IntrusiveSharedPtr& operator=(const IntrusiveSharedPtr& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->resetImpl(other.ptr_);
    return *this;
  }
  IntrusiveSharedPtr& operator=(IntrusiveSharedPtr&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    this->reset();
    ptr_ = std::exchange(other.ptr_, nullptr);
    return *this;
  }
  IntrusiveSharedPtr& operator=(std::nullptr_t) noexcept {
    this->reset();
    return *this;
  }

  template <
      class U,
      class = std::enable_if_t<
          std::is_convertible_v<typename std::unique_ptr<U>::pointer, pointer>>>
  /* implicit */ IntrusiveSharedPtr(std::unique_ptr<U>&& other) noexcept
      : IntrusiveSharedPtr(other.release()) {}

  template <
      class U,
      class = std::enable_if_t<
          std::is_convertible_v<typename std::unique_ptr<U>::pointer, pointer>>>
  IntrusiveSharedPtr& operator=(std::unique_ptr<U>&& other) noexcept {
    this->resetImpl(other.release());
    return *this;
  }

  template <
      class U,
      class UAccess,
      class = std::enable_if_t<std::is_convertible_v<
          typename IntrusiveSharedPtr<U, UAccess>::pointer,
          pointer>>>
  /* implicit */ IntrusiveSharedPtr(
      const IntrusiveSharedPtr<U, UAccess>& other) noexcept
      : IntrusiveSharedPtr(other.ptr_) {}

  template <
      class U,
      class UAccess,
      class = std::enable_if_t<std::is_convertible_v<
          typename IntrusiveSharedPtr<U, UAccess>::pointer,
          pointer>>>
  /* implicit */ IntrusiveSharedPtr(
      IntrusiveSharedPtr<U, UAccess>&& other) noexcept
      : ptr_(std::exchange(other.ptr_, nullptr)) {}

  template <
      class U,
      class UAccess,
      class = std::enable_if_t<std::is_convertible_v<
          typename IntrusiveSharedPtr<U, UAccess>::pointer,
          pointer>>>
  IntrusiveSharedPtr& operator=(
      const IntrusiveSharedPtr<U, UAccess>& other) noexcept {
    this->resetImpl(other.ptr_);
    return *this;
  }

  template <
      class U,
      class UAccess,
      class = std::enable_if_t<std::is_convertible_v<
          typename IntrusiveSharedPtr<U, UAccess>::pointer,
          pointer>>>
  IntrusiveSharedPtr& operator=(
      IntrusiveSharedPtr<U, UAccess>&& other) noexcept {
    this->reset();
    ptr_ = std::exchange(other.ptr_, nullptr);
    return *this;
  }

  ~IntrusiveSharedPtr() noexcept { this->reset(); }

  void reset(std::nullptr_t = nullptr) noexcept { resetImpl(nullptr); }

  FOLLY_ALWAYS_INLINE pointer get() const noexcept { return ptr_; }
  FOLLY_ALWAYS_INLINE pointer operator->() const noexcept {
    FOLLY_SAFE_DCHECK(
        ptr_ != nullptr, "Tried to access empty InstrusiveSharedPtr");
    return ptr_;
  }
  FOLLY_ALWAYS_INLINE element_type& operator*() const noexcept {
    FOLLY_SAFE_DCHECK(
        ptr_ != nullptr, "Tried to access empty InstrusiveSharedPtr");
    return *ptr_;
  }
  FOLLY_ALWAYS_INLINE explicit operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  long use_count() const noexcept {
    return ptr_ == nullptr ? 0 : static_cast<long>(TAccess::useCount(*ptr_));
  }

  void swap(IntrusiveSharedPtr& other) noexcept { std::swap(ptr_, other.ptr_); }

  [[deprecated("release() is unsafe in concurrent code"), nodiscard]] pointer
  unsafeRelease() noexcept {
    if (ptr_ != nullptr) {
      auto refCount = TAccess::releaseRef(*ptr_);
      FOLLY_SAFE_CHECK(
          refCount == 0, "Tried to release non-unique InstrusiveSharedPtr");
      return std::exchange(ptr_, nullptr);
    }
    return nullptr;
  }

  pointer leak() && noexcept {
    if (ptr_ != nullptr) {
      return std::exchange(ptr_, nullptr);
    }
    return nullptr;
  }

  std::default_delete<T>& get_deleter() noexcept { return deleter_; }
  const std::default_delete<T>& get_deleter() const noexcept {
    return deleter_;
  }

  template <class... Args>
  static IntrusiveSharedPtr make(Args&&... args) {
    return IntrusiveSharedPtr(new T(std::forward<Args>(args)...));
  }

  struct UnsafelyFromRawPointer {};
  IntrusiveSharedPtr(UnsafelyFromRawPointer, pointer ptr) noexcept
      : IntrusiveSharedPtr(ptr) {}

  static IntrusiveSharedPtr fromLeaked(pointer ptr) noexcept {
    return IntrusiveSharedPtr(FromLeakedRawPointer{}, ptr);
  }

 private:
  explicit IntrusiveSharedPtr(pointer ptr) noexcept : ptr_(ptr) {
    if (ptr_ != nullptr) {
      TAccess::acquireRef(*ptr_);
    }
  }

  struct FromLeakedRawPointer {};
  IntrusiveSharedPtr(FromLeakedRawPointer, pointer ptr) noexcept : ptr_(ptr) {}

  void resetImpl(pointer ptr) noexcept {
    if (ptr_ == ptr) {
      return;
    }
    if (ptr_ != nullptr) {
      if (TAccess::releaseRef(*ptr_) == 0) {
        deleter_(ptr_);
      }
    }
    ptr_ = ptr;
    if (ptr_ != nullptr) {
      TAccess::acquireRef(*ptr_);
    }
  }

  template <class U, class UAccess>
  friend class IntrusiveSharedPtr;

  pointer ptr_{nullptr};
  [[FOLLY_ATTR_NO_UNIQUE_ADDRESS]] std::default_delete<T> deleter_;
};

struct BasicIntrusiveSharedPtrControlBlock {
 public:
  using RefCount = std::int32_t;

  void acquireRef() noexcept {
    auto old = refCount_.fetch_add(1, std::memory_order_relaxed);
    // We should never end up with negative ref count. However, it can be 0 in
    // the case that it's unmanaged.
    FOLLY_SAFE_DCHECK(
        old >= 0, "call to acquireRef() revealed that ref count is negative");
  }
  RefCount releaseRef() noexcept {
    auto old = refCount_.fetch_sub(1, std::memory_order_release);
    FOLLY_SAFE_DCHECK(
        old >= 1, "call to releaseRef() when ref count is already 0");
    return old - 1;
  }
  RefCount useCount() const noexcept {
    return refCount_.load(std::memory_order_relaxed);
  }

 private:
  std::atomic<RefCount> refCount_{0};
};

template <class T, class TAccess>
void swap(
    IntrusiveSharedPtr<T, TAccess>& lhs,
    IntrusiveSharedPtr<T, TAccess>& rhs) noexcept {
  lhs.swap(rhs);
}

template <class T, class TAccess, class U, class UAccess>
FOLLY_ALWAYS_INLINE bool operator==(
    const IntrusiveSharedPtr<T, TAccess>& lhs,
    const IntrusiveSharedPtr<U, UAccess>& rhs) noexcept {
  return lhs.get() == rhs.get();
}

template <class T, class TAccess, class U, class UAccess>
FOLLY_ALWAYS_INLINE bool operator!=(
    const IntrusiveSharedPtr<T, TAccess>& lhs,
    const IntrusiveSharedPtr<U, UAccess>& rhs) noexcept {
  return lhs.get() != rhs.get();
}

template <class T, class TAccess, class U, class UAccess>
FOLLY_ALWAYS_INLINE bool operator<(
    const IntrusiveSharedPtr<T, TAccess>& lhs,
    const IntrusiveSharedPtr<U, UAccess>& rhs) noexcept {
  return lhs.get() < rhs.get();
}

template <class T, class TAccess, class U, class UAccess>
FOLLY_ALWAYS_INLINE bool operator>(
    const IntrusiveSharedPtr<T, TAccess>& lhs,
    const IntrusiveSharedPtr<U, UAccess>& rhs) noexcept {
  return lhs.get() > rhs.get();
}

template <class T, class TAccess, class U, class UAccess>
FOLLY_ALWAYS_INLINE bool operator<=(
    const IntrusiveSharedPtr<T, TAccess>& lhs,
    const IntrusiveSharedPtr<U, UAccess>& rhs) noexcept {
  return lhs.get() <= rhs.get();
}

template <class T, class TAccess, class U, class UAccess>
FOLLY_ALWAYS_INLINE bool operator>=(
    const IntrusiveSharedPtr<T, TAccess>& lhs,
    const IntrusiveSharedPtr<U, UAccess>& rhs) noexcept {
  return lhs.get() >= rhs.get();
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator==(
    const IntrusiveSharedPtr<T, TAccess>& lhs, std::nullptr_t) noexcept {
  return lhs.get() == nullptr;
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator==(
    std::nullptr_t, const IntrusiveSharedPtr<T, TAccess>& rhs) noexcept {
  return nullptr == rhs.get();
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator!=(
    const IntrusiveSharedPtr<T, TAccess>& lhs, std::nullptr_t) noexcept {
  return lhs.get() != nullptr;
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator!=(
    std::nullptr_t, const IntrusiveSharedPtr<T, TAccess>& rhs) noexcept {
  return nullptr != rhs.get();
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator<(
    const IntrusiveSharedPtr<T, TAccess>& lhs, std::nullptr_t) noexcept {
  return lhs.get() < nullptr;
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator<(
    std::nullptr_t, const IntrusiveSharedPtr<T, TAccess>& rhs) noexcept {
  return nullptr < rhs.get();
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator>(
    const IntrusiveSharedPtr<T, TAccess>& lhs, std::nullptr_t) noexcept {
  return lhs.get() > nullptr;
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator>(
    std::nullptr_t, const IntrusiveSharedPtr<T, TAccess>& rhs) noexcept {
  return nullptr > rhs.get();
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator<=(
    const IntrusiveSharedPtr<T, TAccess>& lhs, std::nullptr_t) noexcept {
  return lhs.get() <= nullptr;
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator<=(
    std::nullptr_t, const IntrusiveSharedPtr<T, TAccess>& rhs) noexcept {
  return nullptr <= rhs.get();
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator>=(
    const IntrusiveSharedPtr<T, TAccess>& lhs, std::nullptr_t) noexcept {
  return lhs.get() >= nullptr;
}

template <class T, class TAccess>
FOLLY_ALWAYS_INLINE bool operator>=(
    std::nullptr_t, const IntrusiveSharedPtr<T, TAccess>& rhs) noexcept {
  return nullptr >= rhs.get();
}

} // namespace apache::thrift::util

namespace std {

template <class T, class TAccess>
struct hash<apache::thrift::util::IntrusiveSharedPtr<T, TAccess>> {
  std::size_t operator()(
      const apache::thrift::util::IntrusiveSharedPtr<T, TAccess>& ptr)
      const noexcept {
    return std::hash<T*>{}(ptr.get());
  }
};

} // namespace std
