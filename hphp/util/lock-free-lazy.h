/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <atomic>
#include <climits>
#include <type_traits>

#include <folly/Likely.h>
#include <folly/ScopeGuard.h>

#include "hphp/util/assertions.h"
#include "hphp/util/lock-free-ptr-wrapper.h"
#include "hphp/util/smalllocks.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Lazily calculate a value, safely dealing with concurrency, without
 * needing locking.
 *
 * LockFreeLazy<T> wraps a value of type T. The value is not initially
 * calculated, and will only be calculated upon the first call to
 * get(). Once calculated, the value is immutable. The value can be
 * put back into its un-calculated state by calling reset().
 *
 * We use a similar implementation as LockFreePtrWrapper. An atomic
 * variable guards the state of the lazy value. If the state is set to
 * "Present", the value has been calculated already and it can be
 * read. If the state is "Updating", another thread is calculating the
 * value (and this thread must wait). Otherwise this thread should
 * calculate the value (by atomically setting the state to Updating
 * and then calculating it).
 *
 * The advantage of this scheme is that in the common case of the
 * value already being calculated, there is no need to take a lock.
 */

template <typename T>
struct LockFreeLazy {
  LockFreeLazy() : m_state{State::Empty} {}

  LockFreeLazy(const LockFreeLazy&) = delete;
  LockFreeLazy(LockFreeLazy&&) = delete;
  LockFreeLazy& operator=(const LockFreeLazy&) = delete;
  LockFreeLazy& operator=(LockFreeLazy&&) = delete;

  ~LockFreeLazy();

  /*
   * Return a const reference to the contained value, if present. If
   * not, call the given callable and set the contained value to the
   * return value. This call may block if another thread is
   * calculating the value. Concurrent calls to get() are
   * allowed. Only one thread will calculate the value. However no
   * concurrent calls to reset() are allowed at the same time as
   * calling get(). The returned reference will remain valid as long
   * as this LockFreeLazy is alive, and reset() has not been called.
   */
  template <typename F> const T& get(const F&);

  /*
   * Return a const reference to the contained value. The value is
   * assumed to be present. Behavior is undefined if not. Only useful
   * if present() has already returned true and you know there's no
   * concurrent reset() calls.
   */
  const T& rawGet();

  /*
   * Check (without blocking) whether there's a contained value. This
   * must be used carefully, as concurrent get() or reset() calls can
   * change this result at any instant.
   */
  bool present() const;

  /*
   * Destroy the contained value (if any). After this calling this,
   * any subsequent calls to get() will re-calculate the value. This
   * call may block if another thread is destroying the
   * value. Concurrent calls to reset() are allowed. Only one thread
   * will destroy the value. However no concurrent calls to get() are
   * allowed at the same time as calling reset(). This invalidates any
   * references previously returned from get(). Returns true if this
   * call successfully resets, false otherwise.
   */
  bool reset();

private:
  typename std::aligned_storage<sizeof(T), alignof(T)>::type m_storage;
  enum class State : uint32_t {
    Present,
    Updating,
    Empty
  };
  std::atomic<State> m_state;
};

//////////////////////////////////////////////////////////////////////

template<typename T> LockFreeLazy<T>::~LockFreeLazy() {
  // Destroy the value, if it's present
  auto const current = m_state.load(std::memory_order_acquire);
  assertx(current != State::Updating);
  if (current != State::Present) return;
  std::launder(reinterpret_cast<T*>(&m_storage))->~T();
}

template<typename T> bool LockFreeLazy<T>::present() const {
  return m_state.load(std::memory_order_acquire) == State::Present;
}

template<typename T> template<typename F>
const T& LockFreeLazy<T>::get(const F& f) {
  while (true) {
    // Check the state:
    auto current = m_state.load(std::memory_order_acquire);
    if (LIKELY(current == State::Present)) {
      // The value is already calculated. Since it cannot change once
      // calculated, we can return a reference to it without any
      // locking. reset() can destroy the value, but this is not
      // allowed to be called concurrently with get().
      return *std::launder(reinterpret_cast<T*>(&m_storage));
    } else if (current == State::Updating) {
      // Another thread is updating it. Wait on the state
      // variable. The other thread will wake us when they're
      // done. Then we'll repeat this loop and try again.
      futex_wait(
        reinterpret_cast<std::atomic<uint32_t>*>(&m_state),
        static_cast<uint32_t>(current)
      );
      // Otherwise it's not present. We should try to update it. Do a
      // compare and swap to atomically claim the state and mark it
      // that we're updating the value.
    } else if (m_state.compare_exchange_weak(current,
                                             State::Updating,
                                             std::memory_order_relaxed)) {
      // We updated the state, so we're responsible now for updating
      // the value.
      SCOPE_FAIL{
        // If we throw while constructing the value, revert back to
        // the empty state.
        m_state.store(State::Empty, std::memory_order_release);
        futex_wake(reinterpret_cast<std::atomic<uint32_t>*>(&m_state), INT_MAX);
      };
      // Construct the value using the callable, update the state
      // variable, and wake any waiters.
      new(&m_storage) T(f());
      m_state.store(State::Present, std::memory_order_release);
      futex_wake(reinterpret_cast<std::atomic<uint32_t>*>(&m_state), INT_MAX);
      return *std::launder(reinterpret_cast<T*>(&m_storage));
    }
    // If the compare and swap fails, another thread may be updating
    // it now. Loop back to the top and try again.
  }
}

template<typename T> const T& LockFreeLazy<T>::rawGet() {
  assertx(present());
  return *std::launder(reinterpret_cast<T*>(&m_storage));
}

template<typename T> bool LockFreeLazy<T>::reset() {
  // This logic is very similar to get(), except the meaning of Empty
  // and Present are effectively reversed.
  while (true) {
    auto current = m_state.load(std::memory_order_acquire);
    if (current == State::Empty) {
      return false;
    } else if (current == State::Updating) {
      futex_wait(
        reinterpret_cast<std::atomic<uint32_t>*>(&m_state),
        static_cast<uint32_t>(current)
      );
    } else if (m_state.compare_exchange_weak(current,
                                             State::Updating,
                                             std::memory_order_relaxed)) {
      std::launder(reinterpret_cast<T*>(&m_storage))->~T();
      m_state.store(State::Empty, std::memory_order_release);
      futex_wake(reinterpret_cast<std::atomic<uint32_t>*>(&m_state), INT_MAX);
      return true;
    }
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * LockFreeLazyPtr is like LockFreeLazy, but space optimized for the
 * case where you're wrapping a pointer (to a heap allocated
 * value). Nullptr is used to indicate "not present", so that must not
 * be a valid value.
 *
 * If Delete is true, the pointer will be freed (via delete) when the
 * value is cleared. If false, it will not be.
 */

template <typename T, bool Delete = true>
struct LockFreeLazyPtr {
  LockFreeLazyPtr() : m_ptr{nullptr} {}

  LockFreeLazyPtr(const LockFreeLazyPtr&) = delete;
  LockFreeLazyPtr(LockFreeLazyPtr&&) = delete;
  LockFreeLazyPtr& operator=(const LockFreeLazyPtr&) = delete;
  LockFreeLazyPtr& operator=(LockFreeLazyPtr&&) = delete;

  ~LockFreeLazyPtr();

  /*
   * Return a const reference to the value the pointer points to, if
   * present. If not, call the given callable to obtain the pointer
   * and store it This call may block if another thread is calculating
   * the pointer. Concurrent calls to get() are allowed. Only one
   * thread will calculate the pointer. However no concurrent calls to
   * reset() are allowed at the same time as calling get(). The
   * returned reference will remain valid as long as this
   * LockFreeLazyPtr is alive, and reset() has not been called.
   *
   * The callable must return a non-null pointer which can be freed
   * via delete.
   */
  template <typename F> const T& get(const F&);

  /*
   * Check (without blocking) whether there's a contained value. This
   * must be used carefully, as concurrent get() or reset() calls can
   * change this result at any instant.
   */
  bool present() const;

  /*
   * Delete the value pointed to by the contained pointer (if any) and
   * set the pointer to nullptr. After this calling this, any
   * subsequent calls to get() will re-calculate the value. This call
   * may block if another thread is destroying the value. Concurrent
   * calls to reset() are allowed. Only one thread will destroy the
   * value. However no concurrent calls to get() are allowed at the
   * same time as calling reset(). This invalidates any references
   * previously returned from get(). Returns true if this call
   * successfully resets, false otherwise.
   */
  bool reset();
private:
  LockFreePtrWrapper<const T*> m_ptr;
};

template <typename T> using LockFreeLazyPtrNoDelete = LockFreeLazyPtr<T, false>;

//////////////////////////////////////////////////////////////////////

template<typename T, bool D> LockFreeLazyPtr<T, D>::~LockFreeLazyPtr() {
  if (!D) return;
  if (auto const p = m_ptr.copy()) delete p;
}

template<typename T, bool D> template<typename F>
const T& LockFreeLazyPtr<T, D>::get(const F& f) {
  if (auto const p = m_ptr.copy()) return *p;
  auto lock = m_ptr.lock_for_update();
  if (auto const p = m_ptr.copy()) return *p;
  const T* newPtr = f();
  assertx(newPtr);
  lock.update(std::move(newPtr));
  return *m_ptr.copy();
}

template<typename T, bool D> bool LockFreeLazyPtr<T, D>::present() const {
  return m_ptr.copy() != nullptr;
}

template<typename T, bool D> bool LockFreeLazyPtr<T, D>::reset() {
  if (auto const p = m_ptr.copy(); !p) return false;
  auto lock = m_ptr.lock_for_update();
  auto const p = m_ptr.copy();
  if (!p) return false;
  if (D) delete p;
  lock.update(nullptr);
  return true;
}

//////////////////////////////////////////////////////////////////////

}
