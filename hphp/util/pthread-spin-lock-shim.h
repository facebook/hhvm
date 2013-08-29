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
#ifndef PTHREAD_SPIN_LOCK_SHIM
#define PTHREAD_SPIN_LOCK_SHIM

#include <errno.h>

typedef int pthread_spinlock_t;

inline int pthread_spin_init(pthread_spinlock_t *lock, int pshared) {
  __asm__ __volatile__ ("" ::: "memory");
  *lock = 0;
  return 0;
}

inline int pthread_spin_destroy(pthread_spinlock_t *lock) {
  return 0;
}

inline int pthread_spin_lock(pthread_spinlock_t *lock) {
  while (1) {
    int i;
    for (i=0; i < 10000; i++) {
      if (__sync_bool_compare_and_swap(lock, 0, 1)) {
        return 0;
      }
    }
    sched_yield();
  }
}

inline int pthread_spin_trylock(pthread_spinlock_t *lock) {
  if (__sync_bool_compare_and_swap(lock, 0, 1)) {
    return 0;
  }
  return EBUSY;
}

inline int pthread_spin_unlock(pthread_spinlock_t *lock) {
  __asm__ __volatile__ ("" ::: "memory");
  *lock = 0;
  return 0;
}

#endif
