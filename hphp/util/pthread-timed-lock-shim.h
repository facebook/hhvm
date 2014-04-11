/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef PTHREAD_TIMED_LOCK_SHIM
#define PTHREAD_TIMED_LOCK_SHIM

#define MUTEX_LOCK 1
#define WRITE_LOCK 2
#define READ_LOCK  3

inline int _pthread_try_lock(void* lock, timespec* timeout, int type) {
  switch (type) {
    case MUTEX_LOCK:
	  return pthread_mutex_trylock((pthread_mutex_t*)lock);
	case WRITE_LOCK:
	  return pthread_rwlock_trywrlock((pthread_rwlock_t*)lock);
	case READ_LOCK:
	  return pthread_rwlock_tryrdlock((pthread_rwlock_t*)lock);
  }
  return 0;
}

int _pthread_timedlock(void* lock, timespec* timeout, int type) {
  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 10000000;

  int ret;
  while ((ret = _pthread_try_lock(lock, timeout, type)) == EBUSY) {
    timeval now;
    gettimeofday(&now, NULL);
    if (now.tv_sec >= timeout->tv_sec &&
       (now.tv_usec * 1000) >= timeout->tv_nsec) {
      return ETIMEDOUT;
    }
    nanosleep(&ts, NULL);
  }
  return ret;
}

int pthread_mutex_timedlock(pthread_mutex_t* mutex, timespec* timeout) {
  return _pthread_timedlock((void*)mutex, timeout, MUTEX_LOCK);
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t* lock, timespec* timeout) {
  return _pthread_timedlock((void*)lock, timeout, WRITE_LOCK);
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t* lock, timespec* timeout) {
  return _pthread_timedlock((void*)lock, timeout, READ_LOCK);
}

#endif
