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

#include <folly/Demangle.h>

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace apache::thrift::fast_thrift::rocket {

#ifndef NDEBUG
/**
 * Exception thrown in debug mode when TypeErasedPtr::release_as<T>()
 * is called with a type that doesn't match the type the handle was
 * adopted with via from_unique_ptr<T>(...), or when called on a
 * non-owning handle constructed via borrow().
 *
 * Only thrown in debug builds. In release builds, type mismatches
 * result in undefined behavior (zero-overhead recovery).
 */
class TypeErasedPtrTypeMismatch : public std::runtime_error {
 public:
  TypeErasedPtrTypeMismatch(
      const std::type_info& requestedType, const std::type_info* actualType)
      : std::runtime_error(buildMessage(requestedType, actualType)),
        requestedType_(requestedType),
        actualType_(actualType) {}

  const std::type_info& requestedType() const noexcept {
    return requestedType_;
  }

  const std::type_info* actualType() const noexcept { return actualType_; }

 private:
  static std::string buildMessage(
      const std::type_info& requestedType, const std::type_info* actualType) {
    std::string msg = "TypeErasedPtr::release_as() type mismatch: requested '";
    msg += folly::demangle(requestedType.name()).toStdString();
    msg += "' but handle was constructed with '";
    if (actualType) {
      msg += folly::demangle(actualType->name()).toStdString();
    } else {
      msg += "<unknown — built via borrow() or with_custom_deleter()>";
    }
    msg += "'";
    return msg;
  }

  const std::type_info& requestedType_;
  const std::type_info* actualType_;
};
#endif // NDEBUG

/**
 * TypeErasedPtr - Move-only smart pointer over a type-erased object,
 * with optional ownership.
 *
 * The pipeline counterpart to TypeErasedBox: where TypeErasedBox erases
 * the type of a small inline value, TypeErasedPtr erases the type of an
 * indirected object. Use this to transport per-request contexts (or any
 * opaque pointer) through pipeline stages that don't know the concrete
 * type, while keeping ownership well-defined at construction time.
 *
 * Layout:
 *  - Release: two pointers (16 bytes) — same as
 *    `std::unique_ptr<T, void(*)(T*) noexcept>`.
 *  - Debug: one extra `type_info*` (24 bytes) for typed-recovery checks.
 *
 * Performance:
 *  - get()      : single load. Zero overhead vs raw `T*`.
 *  - destruction: one indirect call through the function-pointer deleter.
 *
 * Move-only. No vtable. No allocation beyond the user's underlying
 * smart-pointer construction.
 *
 * Construction is via factories (the public surface) — not the raw
 * (ptr, deleter) constructor. Most callers want the default-owning
 * pattern via `from_unique_ptr<T>(std::make_unique<T>(...))` and never
 * have to think about deleters.
 *
 * Factories:
 *  - `from_unique_ptr<T>(std::unique_ptr<T>)` — adopt ownership; default
 *    `delete` cleanup. The common case.
 *  - `borrow(ptr)`                            — non-owning view; cleanup
 *    is a no-op. Tests/benchmarks only.
 *  - `with_custom_deleter(ptr, deleter)`      — escape hatch for the
 *    rare case where cleanup needs to do extra work beyond `delete`
 *    (e.g., the rescue-on-drop pattern in AppAdapters that fires a
 *    pending callback before freeing the heap object).
 *
 * Future factories will be added as new backing smart-pointer kinds
 * appear (e.g., `from_evb_local<T>(EvbLocalPtr<T>)` for EvbBumpAllocator-
 * backed objects whose deleter calls only the destructor and leaves
 * memory reclaim to the arena).
 */
class TypeErasedPtr {
 public:
  using Deleter = void (*)(void*) noexcept;

  /// Empty handle: null pointer, no-op deleter, safe to destruct.
  TypeErasedPtr() noexcept = default;

  ~TypeErasedPtr() { reset(); }

  TypeErasedPtr(TypeErasedPtr&& o) noexcept
      : ptr_(std::exchange(o.ptr_, nullptr)),
        deleter_(std::exchange(o.deleter_, &noopDeleter))
#ifndef NDEBUG
        ,
        type_(std::exchange(o.type_, nullptr))
#endif
  {
  }

  TypeErasedPtr& operator=(TypeErasedPtr&& o) noexcept {
    if (this != &o) {
      reset();
      ptr_ = std::exchange(o.ptr_, nullptr);
      deleter_ = std::exchange(o.deleter_, &noopDeleter);
#ifndef NDEBUG
      type_ = std::exchange(o.type_, nullptr);
#endif
    }
    return *this;
  }

  TypeErasedPtr(const TypeErasedPtr&) = delete;
  TypeErasedPtr& operator=(const TypeErasedPtr&) = delete;

  /**
   * Release ownership; caller is responsible for cleanup of the returned
   * pointer using whatever scheme the original deleter expected.
   * After release(), the handle becomes empty.
   */
  void* release() noexcept {
    deleter_ = &noopDeleter;
#ifndef NDEBUG
    type_ = nullptr;
#endif
    return std::exchange(ptr_, nullptr);
  }

  /**
   * Release ownership and recover the concrete type. Returns a regular
   * `std::unique_ptr<T>` that uses the default `delete` for cleanup.
   *
   * Debug builds throw TypeErasedPtrTypeMismatch when the handle has a
   * recorded type (i.e., was built via `from_unique_ptr<T>()`) and that
   * type doesn't match T. Borrowed and custom-deleter handles carry no
   * recorded type — the caller is trusted to know whether `delete` is
   * the right cleanup for the underlying memory.
   */
  template <typename T>
  std::unique_ptr<T> release_as() {
#ifndef NDEBUG
    // Only flag actual type mismatches. Borrowed handles and ones built
    // via with_custom_deleter() carry no recorded type — caller is on
    // their own to ensure the recovered unique_ptr<T> can validly call
    // `delete` on the underlying memory.
    if (type_ != nullptr && *type_ != typeid(T)) {
      throw TypeErasedPtrTypeMismatch(typeid(T), type_);
    }
#endif
    return std::unique_ptr<T>{static_cast<T*>(release())};
  }

  /// Run the deleter on any owned pointer; leave handle empty.
  void reset() noexcept {
    if (ptr_ != nullptr) {
      deleter_(ptr_);
      ptr_ = nullptr;
    }
    deleter_ = &noopDeleter;
#ifndef NDEBUG
    type_ = nullptr;
#endif
  }

  /// Raw pointer access (does not transfer ownership). Single load —
  /// matches `T*` cost.
  void* get() const noexcept { return ptr_; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

#ifndef NDEBUG
  /// Type recorded at construction (only for handles built via
  /// `from_unique_ptr<T>(...)`). Returns nullptr otherwise.
  const std::type_info* storedType() const noexcept { return type_; }

  std::string typeName() const {
    if (type_ != nullptr) {
      return folly::demangle(type_->name()).toStdString();
    }
    return "<unknown>";
  }
#endif

 private:
  // Private — callers go through factories so the "do I need a custom
  // deleter?" question only comes up when they really do.
  TypeErasedPtr(
      void* ptr,
      Deleter deleter
#ifndef NDEBUG
      ,
      const std::type_info* type = nullptr
#endif
      ) noexcept
      : ptr_(ptr),
        deleter_(deleter)
#ifndef NDEBUG
        ,
        type_(type)
#endif
  {
  }

  static void noopDeleter(void*) noexcept {}

  template <typename T>
  static void defaultDeleter(void* p) noexcept {
    delete static_cast<T*>(p);
  }

  void* ptr_{nullptr};
  Deleter deleter_{&noopDeleter};

#ifndef NDEBUG
  const std::type_info* type_{nullptr};
#endif

  // Factories need access to the private constructor (and
  // defaultDeleter<T> for from_unique_ptr).
  template <typename T>
  friend TypeErasedPtr from_unique_ptr(std::unique_ptr<T> ptr) noexcept;
  friend TypeErasedPtr borrow(void* ptr) noexcept;
  friend TypeErasedPtr with_custom_deleter(void* ptr, Deleter deleter) noexcept;
};

// Verify zero-cost layout in release builds.
#ifdef NDEBUG
static_assert(
    sizeof(TypeErasedPtr) == 2 * sizeof(void*),
    "TypeErasedPtr release size unexpected — should be one ptr + one fn ptr");
#else
static_assert(
    sizeof(TypeErasedPtr) == 3 * sizeof(void*),
    "TypeErasedPtr debug size unexpected — should be release + one type_info*");
#endif

// ============================================================================
// Factories
// ============================================================================

/**
 * Adopt ownership from an existing `std::unique_ptr<T>` (default delete).
 * Records typeid(T) in debug builds so `release_as<T>()` can verify.
 *
 * An empty input produces an empty TypeErasedPtr — no deleter invocation
 * on destruction.
 *
 * For unique_ptrs with custom deleters (lambdas with state, allocator-
 * aware deleters, etc.), there's no general way to fold an arbitrary
 * deleter into a function pointer. Use `with_custom_deleter()` instead.
 */
template <typename T>
TypeErasedPtr from_unique_ptr(std::unique_ptr<T> ptr) noexcept {
  if (!ptr) {
    return TypeErasedPtr{};
  }
  return TypeErasedPtr{
      ptr.release(),
      &TypeErasedPtr::defaultDeleter<T>
#ifndef NDEBUG
      ,
      &typeid(T)
#endif
  };
}

/**
 * Non-owning view: wrap a raw pointer with a no-op deleter.
 *
 * For tests / benchmarks ONLY. Production code must own its objects via
 * `from_unique_ptr<T>()` or `with_custom_deleter()`.
 */
inline TypeErasedPtr borrow(void* ptr) noexcept {
  return TypeErasedPtr{ptr, &TypeErasedPtr::noopDeleter};
}

/**
 * Escape hatch for the rare case where cleanup must do work beyond
 * `delete` and a `from_*` factory doesn't fit. The deleter must be
 * noexcept and must not be nullptr. No type info is recorded;
 * `release_as<T>()` on the returned handle will throw in debug builds.
 */
inline TypeErasedPtr with_custom_deleter(
    void* ptr, TypeErasedPtr::Deleter deleter) noexcept {
  return TypeErasedPtr{ptr, deleter};
}

} // namespace apache::thrift::fast_thrift::rocket
