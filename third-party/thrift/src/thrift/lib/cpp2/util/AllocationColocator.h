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
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <string_view>
#include <type_traits>
#include <utility>

#include <folly/CPortability.h>
#include <folly/lang/SafeAssert.h>

/**
 * AllocationColocator provides a safe(-ish) abstraction for creating multiple
 * objects in one malloc, taking into account size and alignment requirements.
 *
 * AllocationColocator keeps dynamically-sized objects contiguous in memory for
 * improved cache locality, avoiding page faults, and avoiding repeated malloc
 * overhead.
 *
 * Example usage:
 *   struct RootObject {
 *     int* i;
 *     int* arr;
 *     std::string_view str;
 *   };
 *   AllocationColocator<RootObject> alloc;
 *   auto obj = alloc.allocate(
 *       [i = alloc.object<int>(),
 *        arr = alloc.array<int>(2),
 *        str = alloc.string(5)](auto make) mutable -> RootObject {
 *         RootObject obj;
 *         obj.i = make(std::move(i), 10);
 *         obj.arr = make(std::move(arr));
 *         obj.arr[0] = 42;
 *         obj.arr[1] = 24;
 *         obj.str = make(std::move(str), "hello");
 *         return obj;
 *       });
 *   EXPECT_EQ(*obj->i, 10);
 *   EXPECT_EQ(*obj->arr[0], 42);
 *   EXPECT_EQ(*obj->arr[1], 24);
 *   EXPECT_EQ(obj->str, "hello");
 *
 * This roughly resembles the following structure:
 *
 *   struct RootObject {
 *     int* i = &__c.i;
 *     int* arr = __c.arr;
 *     std::string_view str = {__c.str, 5};
 *
 *     struct Colocated {
 *       int i;
 *       int arr[2];
 *       const char str[6];
 *     } __c;
 *   };
 *
 * However, a struct like this cannot be used if the size of `arr` and `str` are
 * only known at runtime. AllocationColocator solves this problem.
 *
 * The API is used in two phases: planning & allocation.
 *
 * Every AllocationColocator has a "root" object which is signified by the
 * primary template argument `RootObject`. The root object is expected to hold
 * and manage pointers to colocated memory.
 *
 * During the planning phase, the caller may call any of the reservation
 * methods:
 *   - object<T>() — reserves space for a single object of size T with proper
 *                   alignment (alignof(T))
 *   - array<T>(N) — reserves space for N objects of type T with proper
 *                   alignment (alignof(T))
 *   - string(N)   — reserves space for exactly N+1 bytes to store a
 *                   null-terminated string
 * Each of these methods returns a "Locator" object which can be used later to
 * access the object after allocation.
 *
 * The allocation phase is initiated by calling `allocate(F)`. `F` denotes a
 * functional object that receives one argument: a `Builder` object.
 * The `Builder` object is used to construct the colocated objects described
 * by locators. In other words, the `Builder` converts a locator and
 * initialization arguments for a type `T` into a concrete object `T` and
 * returns a pointer to it.
 * The intended usage pattern is that the objects created via the `Builder` are
 * stored as fields in `RootObject` so that they are automatically cleaned up
 * when `RootObject` is destroyed.
 *
 * For any Locator type, the `Builder` can be used to retrieve uninitialized
 * memory by caling `uninitialized(Locator)`.
 *
 * To avoid initialization complexity, `array<T>(N)` only provides uninitialized
 * access. This makes dealing with non-trivial typed arrays very cumbersome so
 * only trivially constructible and destructible types are allowed. The backdoor
 * for managing arrays of complex objects is to use `uninitialized` and manually
 * manage the memory.
 *
 * `object<T>` produces T* for trivially destructible types. For other types, it
 * returns a AllocationColocator<>::Ptr<T> which will call the destructor but
 * won't free the underlying memory. This pattern works nicely if `RootObject`
 * stores this smart pointer as a data member.
 *
 * It may not always be possible to store pointers to colocated objects, for
 * example, to preserve memory. For such cases, the colocated objects can be
 * retrieved with a cursor API called `UnsafeCursor`.
 *
 *   auto cursor = AllocationColocator<RootObject>::unsafeCursor(rootPtr);
 *   cursor.object<int>();
 *   cursor.array<int>(2);
 *   cursor.string(5);
 *
 * The cursor API is unsafe because:
 *   - The locator abstraction, which provides type-safe access, is bypassed.
 *     Thus the cursor API must be called in the exact order in which the
 *     allocation was reserved.
 *   - The retrieved value is always a raw pointer and lifetime must be
 *     managed explicitly.
 *
 * Ideally, the cursor API should be avoided, especially when used with
 * non-trivial types.
 *
 * A few important details to keep in mind when using this API:
 *   - The reservation methods reserve space in the order in which they are
 *     called.
 *   - Locator objects can only be used to access objects once.
 *   - array<T> is only allowed if T is trivially constructible and
 *     destructible. This is not a fundamental limitation but it simplifies the
 *     access API.
 *   - allocate() returns a smart pointer (AllocationColocator<Root>::Ptr) which
 *     will call operator delete[] correctly.
 *   - strings are always null-terminated.
 *   - The cursor API can be used for dynamic access to colocated objects.
 */
namespace apache::thrift::util {

namespace detail {
/**
 * Aligns `value` to the specified alignment. If `value` were a pointer to a
 * memory buffer, then the return value would represent the closest locator in
 * memory where an object with alignment of `alignment` could be created.
 *
 * For example:
 *   align(15, 8) = 16
 *   align(16, 8) = 16
 *   align(17, 8) = 24
 *
 * `alignment` must be a non-negative power of two, which is true for alignof(T)
 * for any T.
 */
template <typename T>
constexpr T align(T value, std::align_val_t alignment) {
  const auto mask = T(alignment) - 1;
  return T((value + mask) & ~mask);
}

struct LocatorBase {
  std::ptrdiff_t offset;
  explicit LocatorBase(std::ptrdiff_t offsetValue) : offset(offsetValue) {}

  LocatorBase(const LocatorBase&) = delete;
  LocatorBase& operator=(const LocatorBase&) = delete;
  LocatorBase(LocatorBase&& other) noexcept = default;
  LocatorBase& operator=(LocatorBase&& other) noexcept = default;
};

template <typename T>
static constexpr bool IsValidColocatedArrayType =
    std::is_trivially_constructible_v<T>&& std::is_trivially_destructible_v<T>;

template <bool kIsConst>
class UnsafeCursorBase {
 private:
  template <typename T>
  using MaybeAddConst = std::conditional_t<kIsConst, const T, T>;
  using MaybeConstByte = MaybeAddConst<std::byte>;

 public:
  explicit UnsafeCursorBase(MaybeConstByte* buffer) : buffer_(buffer) {}
  UnsafeCursorBase(const UnsafeCursorBase&) = default;
  UnsafeCursorBase& operator=(const UnsafeCursorBase&) = default;
  UnsafeCursorBase(UnsafeCursorBase&&) = default;
  UnsafeCursorBase& operator=(UnsafeCursorBase&&) = default;

  template <typename T>
  MaybeAddConst<T>* array(std::size_t count) noexcept {
    buffer_ = reinterpret_cast<MaybeConstByte*>(
        detail::align(std::uintptr_t(buffer_), std::align_val_t(alignof(T))));
    auto value = reinterpret_cast<MaybeAddConst<T>*>(buffer_);
    buffer_ += sizeof(T) * count;
    return value;
  }

  template <typename T>
  MaybeAddConst<T>* object() noexcept {
    return array<T>(1);
  }

  std::string_view string(std::size_t length) noexcept {
    return {array<MaybeAddConst<char>>(length + 1), length};
  }

 private:
  MaybeConstByte* buffer_;
};

// For testing access only
class AllocationColocatorInternals;

} // namespace detail

template <typename Root = void>
class AllocationColocator;

template <>
class AllocationColocator<void> {
 public:
  template <typename T>
  struct ObjectLocator : public detail::LocatorBase {
    using detail::LocatorBase::LocatorBase;
  };

  template <typename T>
  struct ArrayLocator : public detail::LocatorBase {
    using detail::LocatorBase::LocatorBase;
  };

  struct StringLocator : public detail::LocatorBase {
    StringLocator(std::ptrdiff_t offset, std::size_t lengthValue)
        : LocatorBase(offset), length(lengthValue) {}
    std::size_t length;
  };

  template <typename T>
  struct Deleter {
    void operator()(T* pointer) const {
      if (pointer) {
        pointer->~T();
        // pointer will be free'd as part of freeing Root
      }
    }
  };
  template <typename T>
  using Ptr = std::unique_ptr<T, Deleter<T>>;

  class Builder {
   public:
    template <
        typename Locator,
        typename =
            std::enable_if_t<std::is_base_of_v<detail::LocatorBase, Locator>>>
    std::byte* uninitialized(Locator locator) const noexcept {
      return buffer_ + locator.offset;
    }

    template <
        typename T,
        typename = std::enable_if_t<detail::IsValidColocatedArrayType<T>>>
    T* array(ArrayLocator<T> locator) const noexcept {
      return reinterpret_cast<T*>(this->uninitialized(std::move(locator)));
    }

    template <
        typename T,
        typename... Args,
        typename = std::enable_if_t<std::is_trivially_destructible_v<T>>>
    T* object(ObjectLocator<T>&& locator, Args&&... args) const
        noexcept(noexcept(std::is_nothrow_constructible_v<T, Args...>)) {
      return new (this->uninitialized(std::move(locator)))
          T(std::forward<Args>(args)...);
    }

    template <
        typename T,
        typename... Args,
        typename = std::enable_if_t<!std::is_trivially_destructible_v<T>>>
    AllocationColocator<>::Ptr<T> object(
        ObjectLocator<T>&& locator, Args&&... args) const
        noexcept(noexcept(std::is_nothrow_constructible_v<T, Args...>)) {
      T* value = new (this->uninitialized(std::move(locator)))
          T(std::forward<Args>(args)...);
      return AllocationColocator<>::Ptr<T>(value, {});
    }

    std::string_view string(
        StringLocator locator, std::string_view value) const noexcept {
      FOLLY_SAFE_CHECK(
          value.size() <= locator.length,
          "String value length exceeds requested buffer length");
      char* str = static_cast<char*>(std::memcpy(
          this->uninitialized(std::move(locator)), value.data(), value.size()));
      str[value.size()] = '\0';
      return {str, value.size()};
    }

    template <typename T, typename... Args>
    decltype(auto) operator()(ObjectLocator<T>&& locator, Args&&... args) const
        noexcept(noexcept(std::declval<Builder>().object(
            std::move(locator), std::forward<Args>(args)...))) {
      return this->object(std::move(locator), std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    decltype(auto) operator()(ArrayLocator<T>&& locator) const noexcept {
      return this->array(std::move(locator));
    }

    decltype(auto) operator()(
        StringLocator locator, std::string_view value) const noexcept {
      return this->string(std::move(locator), value);
    }

   private:
    template <typename Root>
    friend class AllocationColocator;

    explicit Builder(std::byte* buffer) : buffer_(buffer) {}
    std::byte* const buffer_;
  };

  class UnsafeCursor : private detail::UnsafeCursorBase</* kIsConst */ false> {
   private:
    using Base = detail::UnsafeCursorBase<false>;

   public:
    using Base::array;
    using Base::Base;
    using Base::object;
    using Base::string;
  };

  class ConstUnsafeCursor
      : private detail::UnsafeCursorBase</* kIsConst */ true> {
   private:
    using Base = detail::UnsafeCursorBase<true>;

   public:
    using Base::array;
    using Base::Base;
    using Base::object;
    using Base::string;
  };
};

template <typename Root>
class AllocationColocator {
 public:
  template <typename T>
  using ObjectLocator = AllocationColocator<>::ObjectLocator<T>;
  template <typename T>
  using ArrayLocator = AllocationColocator<>::ArrayLocator<T>;
  using StringLocator = AllocationColocator<>::StringLocator;
  using Builder = AllocationColocator<>::Builder;

 private:
  template <typename T>
  FOLLY_ALWAYS_INLINE ArrayLocator<T> arrayImpl(std::size_t count) noexcept {
    // If AllocationColocator needed to support alignment > default new, then we
    // would have to use the sized allocation functions (introduced in C++17).
    // Consequently, we would also have to use the corresponding sized
    // de-allocation function which means that we would have to store the size
    // and alignment in the deleter of the returned std::unique_ptr.
    //
    // The trade-off we are making is that objects with such alignment
    // requirements are rare, and in return we get a stateless deleter.
    static_assert(
        alignof(T) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__,
        "AllocationColocator does not support alignment greater than unaligned operator new()");
    bytes_ = detail::align(bytes_, std::align_val_t(alignof(T)));
    auto offset = std::ptrdiff_t(bytes_);
    bytes_ += sizeof(T) * count;
    return ArrayLocator<T>(offset);
  }

 public:
  template <
      typename T,
      typename = std::enable_if_t<detail::IsValidColocatedArrayType<T>>>
  ArrayLocator<T> array(std::size_t count) noexcept {
    return arrayImpl<T>(count);
  }

  template <typename T>
  ObjectLocator<T> object() noexcept {
    return ObjectLocator<T>(arrayImpl<T>(1).offset);
  }

  StringLocator string(std::size_t length) noexcept {
    return StringLocator(array<char>(length + 1).offset, length);
  }

  struct Deleter {
    void operator()(Root* pointer) const {
      if (pointer) {
        pointer->~Root();
        delete[] reinterpret_cast<std::byte*>(pointer);
      }
    }
  };
  using Ptr = std::unique_ptr<Root, Deleter>;
  static_assert(sizeof(Ptr) == sizeof(Root*));

  template <typename F>
  Ptr allocate(F&& build) const {
    auto buffer = new std::byte[bytes_];
    FOLLY_SAFE_DCHECK(
        (std::uintptr_t(buffer) % __STDCPP_DEFAULT_NEW_ALIGNMENT__) == 0,
        "Allocated buffer is under-aligned");

    try {
      Root* value = new (buffer) Root(build(Builder(buffer)));
      return Ptr(value, Deleter());
    } catch (...) {
      delete[] buffer;
      throw;
    }
  }

  static AllocationColocator<>::UnsafeCursor unsafeCursor(Root* root) {
    auto colocationBegin = reinterpret_cast<std::byte*>(root + 1);
    return AllocationColocator<>::UnsafeCursor(colocationBegin);
  }
  static AllocationColocator<>::ConstUnsafeCursor unsafeCursor(
      const Root* root) {
    auto colocationBegin = reinterpret_cast<const std::byte*>(root + 1);
    return AllocationColocator<>::ConstUnsafeCursor(colocationBegin);
  }
  static AllocationColocator<>::UnsafeCursor unsafeCursor(const Ptr& root) {
    return unsafeCursor(root.get());
  }

 private:
  std::size_t bytes_ = sizeof(Root);

  friend class detail::AllocationColocatorInternals;
};

namespace detail {
class AllocationColocatorInternals {
 public:
  template <typename Root>
  static std::size_t getNumBytesForAllocation(
      const AllocationColocator<Root>& alloc) noexcept {
    return alloc.bytes_;
  }
};
} // namespace detail

} // namespace apache::thrift::util
