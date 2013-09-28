/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace {
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
// locks. If this becomes a perf issue, it's
void futex_wait(std::atomic<int>* value, int expected) {
}

void futex_wake(std::atomic<int>* value, int nwake) {
}
#endif
}

// A lock the size of a 4 byte int
// TODO:
// (1) This could be made 1 byte (or even 2 bits). We'd have to pass an aligned
//     address to futex wait and read the rest of the 4 bytes, making them part
//     of the expected value.
// (2) For performance, we could add spinning.
// (3) un-inline the slow path
class SmallLock {
public:

  SmallLock() : lock_data(0) {}

  // this is roughly based on http://www.akkadia.org/drepper/futex.pdf
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
}
#endif
