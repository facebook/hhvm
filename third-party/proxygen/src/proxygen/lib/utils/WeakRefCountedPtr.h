/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <glog/logging.h>

namespace proxygen {

/**
 * WeakRefCountedPtr allows an object to handout pointers to itself that:
 *   - Can extend the target object's lifetime, if the object chooses to do so.
 *   - Are aware of whether the target object has been destroyed.
 *
 * WeakRefCountedPtr is not thread-safe for performance reasons.
 *
 * Comparison to other approaches:
 *   - shared_ptr<T> is too strong, while weak_ptr<T> is too weak:
 *      - shared_ptr<T> would require an object to remain alive
 *      - weak_ptr<T> does not delay destruction
 *      - WeakRefCountedPtr is a happy medium; an object is aware of whether
 *        WeakRefCountedPtr exist that point to it and can choose to delay its
 *        own destruction, or can safely destroy.
 *   - Because WeakRefCountedPtr is not thread-safe, it avoids using atomics
 *     that would be required for shared_ptr and weak_ptr.
 *
 * Enabling for a type T:
 *   1. Derive from EnableWeakRefCountedPtr<T>.
 *   2. Delay destruction if (numWeakRefCountedPtrs() != 0).
 *   3. Implement onWeakRefCountedPtrDestroy, called in ~WeakRefCountedPtr.
 *      If the object delayed destruction due to outstanding WeakRefCountedPtr,
 *      it should check if (numWeakRefCountedPtrs() == 0). If this condition is
 *      met, then no WeakRefCountedPtr remain and destruction can proceed.
 *   4. An object is not required to delay its destruction if WeakRefCountedPtr
 *      still exist (2); if it is destroyed while they still exist, the
 *      consumers will be able to determine this.
 *
 * Getting and using a WeakRefCountedPtr<T>:
 *   1. Call T::getWeakRefCountedPtr<T> to get a WeakRefCountedPtr<T>.
 *      In the case of derived classes, T must have an ancestor B that derived
 *      from EnableWeakRefCountedPtr<B>.
 *   2. On each use of the WeakRefCountedPtr, check if target still exists by:
 *        (a) Call get() and check if T* returned != nullptr, OR
 *        (b) Check if the WeakRefCountedPtr object evaluates to true.
 *   3. If target still exists, can use get() or -> to get pointer to target.
 *   4. Target pointer should not be stored.
 */

template <class T1, class T2>
class WeakRefCountedPtr;

template <class T>
struct WeakRefCountedPtrState {
  T* ptr{nullptr};
  uint64_t count{0};
};

/**
 * Derive from EnableWeakRefCountedPtr to enable WeakRefCountedPtr for class.
 */
template <class T>
class EnableWeakRefCountedPtr {
 public:
  EnableWeakRefCountedPtr() : state_(nullptr) {
    static_assert(std::is_base_of<EnableWeakRefCountedPtr<T>, T>::value,
                  "T must derive from EnableWeakRefCountedPtr");
  }

  /**
   * Destructor.
   *
   * If any WeakRefCountedPtr are pointing to this object, any future calls to
   * WeakRefCountedPtr::get will return nullptr after the destructor executes.
   */
  virtual ~EnableWeakRefCountedPtr() {
    if (state_) {
      state_->ptr = nullptr;
      if (state_->count == 0) {
        delete state_;
      }
    }
  }

  /**
   * Returns a WeakRefCountedPtr<T>.
   *
   * T must either have EnableWeakRefCountedPtr<T> as a base class or must have
   * some ancestor class B that derives from EnableWeakRefCountedPtr<B>.
   *
   * To check if the WeakRefCountedPtr's target still exists, you can either:
   *   (a) Call WeakRefCountedPtr<T>::get() and check if T* returned != nullptr.
   *   (b) Check if WeakRefCountedPtr<T> evaluates to true.
   *
   * If target object still exists, you can use get() or -> to get the pointer.
   */
  template <typename T2 = T>
  WeakRefCountedPtr<T, T2> getWeakRefCountedPtr() {
    if (!state_) {
      // state not setup yet, do it now
      state_ = new WeakRefCountedPtrState<T>();
      state_->ptr = static_cast<T*>(this);
    }
    return WeakRefCountedPtr<T, T2>(state_);
  }

  /**
   * Returns number of WeakRefCountedPtr pointing to this object.
   */
  uint64_t numWeakRefCountedPtrs() const {
    if (!state_) {
      return 0;
    }
    return state_->count;
  }

 protected:
  /**
   * Called when a WeakRefCountedPtr is created.
   *
   * Default implementation is a no-op.
   */
  virtual void onWeakRefCountedPtrCreate() {
  }

  /**
   * Called when a WeakRefCountedPtr is destroyed.
   *
   * If the object has delayed destruction due to outstanding WeakRefCountedPtr,
   * implementaton should check if (numWeakRefCountedPtrs() == 0). If condition
   * is met, then no WeakRefCountedPtr remain and destruction can proceed.
   */
  virtual void onWeakRefCountedPtrDestroy() = 0;

  // WeakRefCountedPtr must be a friend so that it can call
  // onWeakRefCountedPtrDestroy()
  //
  // We do not care about its precise template arguments.
  template <typename T1, typename T2>
  friend class WeakRefCountedPtr;

 private:
  WeakRefCountedPtrState<T>* state_{nullptr};
};

/**
 * WeakRefCountedPtr pointing to object of type T2, which derives from T1.
 *
 * To check if the WeakRefCountedPtr's target still exists, you can either:
 *   (a) Call WeakRefCountedPtr<T>::get() and check if T* returned != nullptr.
 *   (b) Check if WeakRefCountedPtr<T> evaluates to true.
 *
 * If target object still exists, you can use get() or -> to get the pointer.
 */
template <class T1, class T2 = T1>
class WeakRefCountedPtr {
 public:
  WeakRefCountedPtr() : state_(nullptr) {
    static_assert(std::is_base_of<T1, T2>::value, "T2 must be derived from T1");
  }

  WeakRefCountedPtr(const WeakRefCountedPtr& rhs) {
    initPtr(rhs.state_);
  }

  WeakRefCountedPtr(WeakRefCountedPtr&& rhs) {
    state_ = rhs.state_;
    rhs.state_ = nullptr;
  }

  WeakRefCountedPtr& operator=(const WeakRefCountedPtr& rhs) {
    destroyPtr();
    initPtr(rhs.state_);
    return *this;
  }

  WeakRefCountedPtr& operator=(WeakRefCountedPtr&& rhs) {
    destroyPtr();
    state_ = rhs.state_;
    rhs.state_ = nullptr;
    return *this;
  }

  ~WeakRefCountedPtr() {
    destroyPtr();
  }

  T2* get() const {
    if (!state_) {
      return nullptr;
    }
    return static_cast<T2*>(state_->ptr);
  }

  T2* operator->() const {
    return get();
  }

  operator bool() const {
    return (get());
  }

  void reset() {
    destroyPtr();
  }

 private:
  WeakRefCountedPtr(WeakRefCountedPtrState<T1>* state) {
    initPtr(state);
  }

  /**
   * Initializes the WeakRefCountedPtr.
   *   - Increments reference count.
   */
  void initPtr(WeakRefCountedPtrState<T1>* state) {
    CHECK(!state_);
    state_ = state;
    if (state_) {
      state_->count++;
      if (state_->ptr) {
        state_->ptr->onWeakRefCountedPtrCreate();
      }
      CHECK_GE(state_->count, 1); // sanity if state_->count is ever unsigned
    }
  }

  /**
   * Destroys the WeakRefCountedPtr.
   *   - Decrements reference count.
   *   - Calls onWeakRefCountedPtrDestroy on target object if it still exists.
   *   - Cleans up state object if target destroyed and no other ptrs remain.
   */
  void destroyPtr() {
    if (!state_) {
      return;
    }
    CHECK_GE(state_->count, 1);
    state_->count--;
    if (state_->ptr) {
      state_->ptr->onWeakRefCountedPtrDestroy();
    } else if (!state_->count) {
      delete state_;
    }
    state_ = nullptr;
  }

  WeakRefCountedPtrState<T1>* state_{nullptr};
  friend EnableWeakRefCountedPtr<T1>;
};

} // namespace proxygen
