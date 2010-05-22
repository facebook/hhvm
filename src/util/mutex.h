/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A mutex that's re-entrant, meaning it won't deadlock if you do this,
 *
 *   class A {
 *    public:
 *      void foo() {
 *        Lock lock(m_mutex);
 *        bar(); // if m_mutex is not re-entrant, this will deadlock
 *      }
 *
 *      void bar() {
 *        Lock lock(m_mutex);
 *        // ...
 *      }
 *
 *    private:
 *      Mutex m_mutex;
 *   };
 */
class Mutex {
public:
  Mutex(bool reentrant = true);
  ~Mutex() {
    pthread_mutex_destroy(&m_mutex);
    pthread_mutexattr_destroy(&m_mutexattr);
  }

  void lock() {
    pthread_mutex_lock(&m_mutex);
  }
  void unlock() {
    pthread_mutex_unlock(&m_mutex);
  }

  pthread_mutex_t &getRaw() { return m_mutex;}

private:
  pthread_mutexattr_t m_mutexattr;
  pthread_mutex_t m_mutex;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Read-write mutex for read-write locks.
 */
class ReadWriteMutex {
public:
  ReadWriteMutex() {
    pthread_rwlock_init(&m_rwlock, NULL);
  }
  ~ReadWriteMutex() {
    pthread_rwlock_destroy(&m_rwlock);
  }

  void acquireRead() { pthread_rwlock_rdlock(&m_rwlock); }
  void acquireWrite() { pthread_rwlock_wrlock(&m_rwlock); }
  bool attemptRead() { return !pthread_rwlock_tryrdlock(&m_rwlock); }
  bool attemptWrite() { return !pthread_rwlock_trywrlock(&m_rwlock); }
  void release() { pthread_rwlock_unlock(&m_rwlock); }

private:
  pthread_rwlock_t m_rwlock;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __MUTEX_H__
