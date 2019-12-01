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

#ifndef incl_LOCK_FREE_PTR_WRAPPER_H_
#define incl_LOCK_FREE_PTR_WRAPPER_H_

#include <atomic>
#include <type_traits>

#include "hphp/util/assertions.h"
#include "hphp/util/low-ptr.h"
#include "hphp/util/smalllocks.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Implements a wrapper around a pointer-like thing such that readers
 * can read the pointer without locking (and without writing memory at
 * all).
 *
 * A single writer will use a compare-and-swap followed by an exchange
 * to update the pointer; multiple concurrent writers will wait on a
 * futex.
 *
 * For this to work, T's behavior must be fully determined by its bit
 * pattern (ie it can't care about its address), so that after:
 *
 *  auto bits = *(uintptr_t*)&t;
 *  auto &t2 = *(T*)&bits;
 *
 * t and t2 can be used interchangeably.
 *
 * All pointer types, and CompactVector in particular, satisfy this.
 */
template<class T>
struct LockFreePtrWrapper {
  static_assert((sizeof(T) == sizeof(uintptr_t)) ||
                (sizeof(T) == sizeof(uint32_t)),
                "LockFreePtrWrapper operates on pointers, or "
                "classes that wrap pointers (with no other data)");
  using raw_type =
    typename std::conditional<sizeof(T) == 4, uint32_t, uintptr_t>::type;

  LockFreePtrWrapper() : val{} { assertx(notLocked()); }
  LockFreePtrWrapper(const T& v) : val{v} { assertx(notLocked()); }
  ~LockFreePtrWrapper() {
    assertx(notLocked());
    val.~T();
  }

  LockFreePtrWrapper(const LockFreePtrWrapper<T>&) = delete;
  LockFreePtrWrapper<T>& operator=(const LockFreePtrWrapper<T>&) = delete;

  /*
   * We can't return a T* to callers because a valid T doesn't always exist in
   * LockFreePtrWrapper. Holder is a convenience wrapper around a bitwise copy
   * of T that avoids calling T's constructor or destructor, giving nearly the
   * same effect as returning a T*.
   */
  struct Holder {
    friend struct LockFreePtrWrapper;
    auto get() { return getter(val, false); }
    auto operator->() { return get(); }
    Holder(const Holder& h) : bits{h.bits} { assertx(!(bits & ~kPtrMask)); }
    Holder(raw_type bits) : bits{bits} { assertx(!(bits & ~kPtrMask)); }
    Holder(T&& val) : val{std::move(val)} { assertx(!(bits & ~kPtrMask)); }
    ~Holder() {}
  private:
    union {
      raw_type bits;
      T val;
    };
    template<typename U>
    static const U* getter(const U& p, int f) { return &p; }
    template<typename U>
    static auto getter(const U& p, bool f) -> decltype(p.operator->(), p) {
      return p;
    }
  };

  // Get a bitwise copy of the current value
  auto get() const {
    return getImpl<T>();
  }

  auto operator->() const {
    return get();
  }

  T copy() const {
    return copyImpl<T>();
  }

  /*
   * Get an exclusive lock on the wrapped value. Other threads can
   * still read its current value via get() or copy(). After calling
   * this, you must unlock it either with update_and_unlock (if you
   * want to change the value), or unlock (if you don't).
   */
  void lock_for_update();
  /*
   * Unlock it.
   */
  void unlock();
  /*
   * Update the wrapped value, and return the old value. The old value
   * will typically need to be destroyed via a treadmill-like
   * mechanism, because other threads may have read the old value just
   * prior to the update (and still be using it).
   */
  T update_and_unlock(T&& v);

 protected:
  // Constructor that accepts raw bits, for use in the unsafe version.
  LockFreePtrWrapper(raw_type rawBits) : bits(rawBits) {
    assertx(notLocked());
  }
  raw_type raw() const { return bits.load(std::memory_order_relaxed); }
  const bool notLocked() {
    return !(low_bits.load(std::memory_order_relaxed) & ~kPtrMask);
  }

  template<typename U>
  typename std::enable_if<std::is_same<T,U>::value &&
                          std::is_pointer<U>::value,U>::type
  getImpl() const {
    return reinterpret_cast<T>(unlocked());
  }

  template<typename U>
  typename std::enable_if<std::is_same<T,U>::value && is_lowptr_v<U>,
                          typename lowptr_traits<U>::pointer>::type
  getImpl() const {
    auto p = unlocked();
    return reinterpret_cast<T*>(&p)->get();
  }

  template<typename U>
  typename std::enable_if<std::is_same<T,U>::value &&
                          !std::is_pointer<U>::value &&
                          !is_lowptr_v<U>, Holder>::type
  getImpl() const {
    return Holder { unlocked() };
  }

  template<typename U>
  typename std::enable_if<std::is_same<T,U>::value &&
                          (std::is_pointer<U>::value || is_lowptr_v<U>),
                          T>::type
  copyImpl() const {
    return get();
  }

  template<typename U>
  typename std::enable_if<std::is_same<T,U>::value &&
                          !std::is_pointer<U>::value &&
                          !is_lowptr_v<U>, T>::type
  copyImpl() const {
    // We need to force a copy, rather than a move from get().val. If you
    // change this, make sure you know what you're doing.
    auto const& x = get();
    return x.val;
  }

  raw_type unlock_helper(raw_type rep) {
    auto const c = bits.exchange(rep, std::memory_order_release);
    if (c & kLockWithWaitersBit) {
      futex_wake(&low_bits, 1);
    } else {
      assertx(c & kLockNoWaitersBit);
    }
    return c & kPtrMask;
  }
  raw_type unlocked() const {
    return bits.load(std::memory_order_acquire) & kPtrMask;
  }
  union {
    std::atomic<raw_type> bits;
    std::atomic<uint32_t> low_bits;
    T                     val;
  };
  static_assert(
    folly::kIsLittleEndian,
    "The low bits of low_bits must correspond to the low bits of bits"
  );
  static constexpr raw_type kPtrMask = static_cast<raw_type>(-4);
  static constexpr raw_type kLockNoWaitersBit = 1;
  static constexpr raw_type kLockWithWaitersBit = 2;
};

/*
 * The unsafe-version of LockFreePtrWrapper, provides copy constructor and
 * copy assignment operator, to be used when other mechanisms provide guarantee
 * that no concurrent access happens during the unsafe copying.
 *
 * Warning: a thorough idea on when and in which threads all accesses happen is
 * needed to safely use this.
 */
template<class T>
struct UnsafeLockFreePtrWrapper : LockFreePtrWrapper<T> {
  using LockFreePtrWrapper<T>::notLocked;

  UnsafeLockFreePtrWrapper() : LockFreePtrWrapper<T>() {}
  UnsafeLockFreePtrWrapper(const T& v) : LockFreePtrWrapper<T>(v) {}

  UnsafeLockFreePtrWrapper(const UnsafeLockFreePtrWrapper& o)
    : LockFreePtrWrapper<T>(o.raw()) {}
  auto const& operator=(const UnsafeLockFreePtrWrapper& o) {
    assertx(notLocked());
    LockFreePtrWrapper<T>::bits.store(o.raw(), std::memory_order_relaxed);
    assertx(notLocked());
    return *this;
  }
};

static_assert(!std::is_copy_constructible<LockFreePtrWrapper<int*>>::value, "");
static_assert(std::is_copy_constructible<UnsafeLockFreePtrWrapper<int*>>::
              value, "");

//////////////////////////////////////////////////////////////////////

template<class T>
void LockFreePtrWrapper<T>::lock_for_update() {
  auto lockBit = kLockNoWaitersBit;
  while (true) {
    auto c = raw() & kPtrMask;
    // writing is expected to be unusual, so start by assuming the low
    // two bits are clear, and attempt to set the appropriate low bit.
    if (bits.compare_exchange_weak(c, c + lockBit, std::memory_order_relaxed)) {
      return;
    }
    // We didn't get the lock, so someone else had it. c holds the
    // value we found there.
    if (c & kLockNoWaitersBit) {
      auto const desired = (c & kPtrMask) + kLockWithWaitersBit;
      if (!bits.compare_exchange_weak(c, desired, std::memory_order_relaxed)) {
        // We failed, so someone else got in before us. start over.
        continue;
      }
      // compare_exchange_weak only upates c when it fails, so set it
      // to the value we actually wrote to memory.
      c = desired;
    }
    assertx(!(c & kLockNoWaitersBit));
    if (c & kLockWithWaitersBit) {
      futex_wait(&low_bits, static_cast<uint32_t>(c));
      // If we were waiting on the futex, others might have been
      // waiting too (but we can't tell), so when we grab the lock, we
      // have to record that fact.
      lockBit = kLockWithWaitersBit;
    }
  }
}

template<class T>
void LockFreePtrWrapper<T>::unlock() {
  unlock_helper(raw() & kPtrMask);
}

template<class T>
T LockFreePtrWrapper<T>::update_and_unlock(T&& v) {
  Holder h{std::move(v)};

  h.bits = unlock_helper(h.bits);
  return std::move(h.val);
}

//////////////////////////////////////////////////////////////////////
}

#endif
