/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <assert.h>
#include <pthread.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Recursive mutex.
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
  Mutex(const Mutex &); // suppress
  Mutex &operator=(const Mutex &); // suppress

  pthread_mutexattr_t m_mutexattr;
  pthread_mutex_t m_mutex;
};

///////////////////////////////////////////////////////////////////////////////

class SpinLock {
public:
  SpinLock() {
    pthread_spin_init(&m_spinlock, 0);
  }
  ~SpinLock() {
    pthread_spin_destroy(&m_spinlock);
  }

  void lock() {
    pthread_spin_lock(&m_spinlock);
  }
  void unlock() {
    pthread_spin_unlock(&m_spinlock);
  }

  pthread_spinlock_t &getRaw() { return m_spinlock;}

private:
  SpinLock(const SpinLock &); // suppress
  SpinLock &operator=(const SpinLock &); // suppress

  pthread_spinlock_t m_spinlock;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Read-write lock wrapper.
 */
class ReadWriteMutex {
#ifdef DEBUG
/* 
 * We have a track record of self-deadlocking on these, and our pthread
 * implementation tends to do crazy things when a rwlock is double-wlocked,
 * so check and assert early in debug builds.
 */
  static const pthread_t InvalidThread = (pthread_t)0;
  pthread_t m_writeOwner;
#endif

  void invalidateWriteOwner() {
#ifdef DEBUG
    m_writeOwner = InvalidThread;
#endif
  }

  void recordWriteAcquire() {
#ifdef DEBUG
    assert(m_writeOwner == InvalidThread);
    m_writeOwner = pthread_self();
#endif
  }

  void assertNotWriteOwner() {
#ifdef DEBUG
    assert(m_writeOwner != pthread_self());
#endif
  }

  void assertNotWriteOwned() {
#ifdef DEBUG
    assert(m_writeOwner == InvalidThread);
#endif
  }

public:
  ReadWriteMutex() {
    invalidateWriteOwner();
    pthread_rwlock_init(&m_rwlock, NULL);
  }

  ~ReadWriteMutex() {
    assertNotWriteOwned();
    pthread_rwlock_destroy(&m_rwlock);
  }

  void acquireRead() {
    /*
     * Atomically downgrading a write lock to a read lock is not part of the
     * pthreads standard. See task #528421. 
     */
    assertNotWriteOwner();
    pthread_rwlock_rdlock(&m_rwlock);
    /*
     * Again, see task #528421.
     */
    assertNotWriteOwned();
  }

  void acquireWrite() {
    assertNotWriteOwner();
    pthread_rwlock_wrlock(&m_rwlock);
    assertNotWriteOwned();
    recordWriteAcquire();
  }

  bool attemptRead() { return !pthread_rwlock_tryrdlock(&m_rwlock); }
  bool attemptWrite() { return !pthread_rwlock_trywrlock(&m_rwlock); }
  void release() {
#ifdef DEBUG
    if (m_writeOwner == pthread_self()) {
      invalidateWriteOwner();
    }
#endif
    pthread_rwlock_unlock(&m_rwlock);
  }

private:
  ReadWriteMutex(const ReadWriteMutex &); // suppress
  ReadWriteMutex &operator=(const ReadWriteMutex &); // suppress

  pthread_rwlock_t m_rwlock;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __MUTEX_H__
