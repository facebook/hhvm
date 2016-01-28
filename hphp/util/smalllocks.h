/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_SMALLLOCKS_H_
#define incl_HPHP_SMALLLOCKS_H_

#include <atomic>
#include <unistd.h>
#include <iostream>
#ifdef __linux__
#include <syscall.h>
#include <linux/futex.h>
#include <sys/time.h>
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

#ifdef __linux__

inline int futex(int* uaddr, int op, int val, const timespec* timeout,
                 int* uaddr2, int val3) noexcept {
  return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}

inline void futex_wait(std::atomic<int>* value, int expected) {
  futex(reinterpret_cast<int*>(value), FUTEX_WAIT_PRIVATE, expected,
        nullptr, nullptr, 0);
}

inline void futex_wake(std::atomic<int>* value, int nwake) {
  futex(reinterpret_cast<int*>(value), FUTEX_WAKE_PRIVATE, nwake, nullptr,
        nullptr, 0);
}

#else

// On non-linux OSs we do nothing for futexes. They essentially turn into spin
// locks. If this becomes a perf issue, it's <space intentionally left blank>
inline void futex_wait(std::atomic<int>* value, int expected) {
}

inline void futex_wake(std::atomic<int>* value, int nwake) {
}

#endif

//////////////////////////////////////////////////////////////////////

/*
 * A lock the size of a 4 byte int, using futex_wait when it needs to block.
 *
 * This structure is a standard layout class so it can be put in unions without
 * declaring custom union constructors.  Zeroing its storage is guaranteed to
 * put it in the unlocked state, and unlocking it is guaranteed to put it back
 * to all bits zero.
 *
 * This is roughly based on http://www.akkadia.org/drepper/futex.pdf.
 */
struct SmallLock {
  void lock() {
    int c = 0;
    if (lock_data.compare_exchange_strong(c, 1, std::memory_order_acquire)) {
      return;
    }

    if (c != 2) {
      c = lock_data.exchange(2, std::memory_order_acquire);
    }
    while (c != 0) {
      futex_wait(&lock_data, 2);
      c = lock_data.exchange(2, std::memory_order_acquire);
    }
  }

  void unlock() {
    // Differs from "futexes are tricky" because std::atomic can't generate
    // a dec instruction and test the flags.
    if (lock_data.exchange(0, std::memory_order_release) != 1) {
      futex_wake(&lock_data, 1);
    }
  }

private:
  std::atomic<int> lock_data;
};

//////////////////////////////////////////////////////////////////////

}

#endif
