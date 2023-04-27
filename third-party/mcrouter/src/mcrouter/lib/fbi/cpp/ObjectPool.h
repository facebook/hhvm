/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cassert>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <type_traits>
#include <vector>

#include <glog/logging.h>

namespace facebook {
namespace memcache {

constexpr size_t kInfinity = std::numeric_limits<size_t>::max();

/// ObjectPool is an efficient way to allocate small objects of
/// type T. Internally it uses a free list of pointers to objects
/// allocated via Allocator. The free list is used to satisfy the
/// allocation request first. If the free list is empty it resorts
/// to using the Allocator, which is by default std::allocator<T>,
/// to allocate the object. This speeds up allocation in most cases
/// by avoiding the need to use malloc(new)/free(delete).
///
/// ObjectPool is compatible with custom allocators as long as they
/// define the properties in std::allocator_traits.
///
/// Note: ObjectPool is not thread-safe
template <typename T, typename Allocator = std::allocator<T>>
class ObjectPool {
 public:
  /// Create an object pool with the given max capacity for the free list
  /// If maxCapacity == std::numeric_limits<size_t>::max() then the capacity
  /// of the free list is unbounded
  explicit ObjectPool(size_t maxCapacity = kInfinity)
      : maxCapacity_(maxCapacity) {}

  ObjectPool(const ObjectPool&) = delete;
  ObjectPool& operator=(const ObjectPool&) = delete;

  /// Allocate an object
  /// @param    args        Arguments to forward to T's constructor
  /// @return               Pointer to object on success.
  ///
  /// @throws               any exception thrown while constructing T,
  ///                       any exception thrown by the allocator while
  ///                       allocating the object
  template <typename... Args>
  T* alloc(Args&&... args) {
    // First check the free list
    auto* obj = getFromFreeList();
    if (obj == nullptr) {
      obj = std::allocator_traits<Allocator>::allocate(allocator_, 1);
    }

    if (obj == nullptr) {
      throw std::bad_alloc();
    }

    try {
      std::allocator_traits<Allocator>::construct(
          allocator_, obj, std::forward<Args>(args)...);
      return obj;
    } catch (...) {
      // If we failed while constructing just add the object
      // back to the free list
      addToFreeList(obj);
      throw; // re-throw
    }

    assert(false);
    return nullptr;
  }

  /// Frees the object previously allocated by alloc, after
  /// invoking the destructor. If obj == nullptr it's a NOOP.
  ///
  /// @param  obj          pointer to object of type T
  ///
  /// NOTE:
  /// 1) If the object wasn't allocated by alloc, the function
  /// doesn't provide any guarantees.
  /// 2) The function guarantess noexcept if the destructor doesn't
  /// throw.
  void free(T* obj) {
    if (obj == nullptr) {
      // nothing to do
      return;
    }
    std::allocator_traits<Allocator>::destroy(allocator_, obj);
    addToFreeList(obj);
  }

  ~ObjectPool() {
    for (auto* p : freeList_) {
      std::allocator_traits<Allocator>::deallocate(allocator_, p, 1);
    }
  }

 private:
  class RecacheDeleter {
   public:
    explicit RecacheDeleter(ObjectPool<T>& pool) : pool_(pool) {}

    RecacheDeleter(const RecacheDeleter&) = default;
    RecacheDeleter& operator=(const RecacheDeleter&) = default;
    RecacheDeleter(RecacheDeleter&&) = default;
    RecacheDeleter& operator=(RecacheDeleter&&) = default;

    void operator()(T* p) {
      pool_.free(p);
    }

   private:
    ObjectPool<T>& pool_;
  };

 public:
  using UniquePtr = std::unique_ptr<T, RecacheDeleter>;

  /**
   * Same as alloc(), but returns a unique_ptr with custom deleter that will
   * automatically release memory back to the pool on destruction.
   */
  template <class... Args>
  UniquePtr make(Args&&... args) {
    return takeOwnership(alloc(std::forward<Args>(args)...));
  }

  /**
   * Convert a raw T* to a UniquePtr that will automatically release memory back
   * to the pool on destruction. The caller must ensure that it is appropriate
   * and safe to deallocate p via this->free().
   */
  UniquePtr takeOwnership(T* p) {
    return UniquePtr(p, RecacheDeleter(*this));
  }

 private:
  // Return an object from the free list. The caller must invoke the
  // constructor
  //
  // @return               Pointer to the memory location large enough
  //                       to hold the object, nullptr on error
  T* getFromFreeList() noexcept {
    if (freeList_.empty()) {
      return nullptr;
    }
    auto* obj = freeList_.back();
    freeList_.pop_back();
    return obj;
  }

  // Adds the given object to the freeList_. It is the callers
  // reponsibility to destruct the object *before* calling addToFreeList.
  //
  // @param obj         pointer to object of type T
  void addToFreeList(T* obj) noexcept {
    if (obj == nullptr) {
      return;
    }

    if (maxCapacity_ == kInfinity || freeList_.size() < maxCapacity_) {
      try {
        freeList_.push_back(obj);
        return;
      } catch (const std::exception& e) {
        LOG(ERROR) << "Failed while adding to free list - %s" << e.what();
      } catch (...) {
        LOG(ERROR) << "Unknown exception while adding to free list";
      }
    }

    // We couldn't add to free list so just deallocate
    std::allocator_traits<Allocator>::deallocate(allocator_, obj, 1);
    return;
  }

  std::vector<
      T*,
      typename std::allocator_traits<Allocator>::template rebind_alloc<T*>>
      freeList_; // List of free objects

  Allocator allocator_; // Allocator used to allocate objects

  // Maximum number of objects that can be stored within the pool
  const size_t maxCapacity_;
};

/// ThreadSafeObjectPool, as the name suggests, is a thread safe version of
/// ObjectPool. It is based on ObjectPool but in addition it uses std::mutex
/// to guarantee thread-safety.
template <typename T, typename Allocator = std::allocator<T>>
class ThreadSafeObjectPool {
 public:
  /// Create a thread-safe object pool with the give maxCapacity
  /// If maxCapacity == std::numeric_limits<size_t>::max() then capacity
  /// is infinity
  explicit ThreadSafeObjectPool(size_t maxCapacity) : objectPool(maxCapacity) {}

  /// Allocate an object
  /// @param    args        Arguments to forward to T's constructor
  /// @return               Pointer to object on success.
  ///
  /// @throws               any exception thrown while constructing T,
  ///                       any exception thrown by the allocator while
  ///                       allocating the object
  template <typename... Args>
  T* alloc(Args&&... args) {
    std::lock_guard<std::mutex> lg(mtx);
    return objectPool.alloc(std::forward<Args>(args)...);
  }

  /// Frees the object previously allocated by alloc, after
  /// invoking the destructor. If obj == nullptr it's a NOOP.
  ///
  /// @param  obj          pointer to object of type T
  ///
  /// NOTE:
  /// 1) If the object wasn't allocated by alloc, the function
  /// doesn't provide any guarantees.
  /// 2) The function guarantess noexcept if the destructor doesn't
  /// throw.
  void free(T* obj) {
    std::lock_guard<std::mutex> lg(mtx);
    return objectPool.free(obj);
  }

 private:
  ObjectPool<T, Allocator> objectPool; // Object pool used for allocations
  std::mutex mtx; // Mutex for mutual exclusion
};
} // namespace memcache
} // namespace facebook
