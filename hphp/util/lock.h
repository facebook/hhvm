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

#ifndef incl_HPHP_LOCK_H_
#define incl_HPHP_LOCK_H_

#include "hphp/util/mutex.h"
#include "hphp/util/synchronizable.h"
#include "hphp/util/synchronizable-multi.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Lock instrumentation for mutex stats.
 */
class LockProfiler {
public:
  typedef void (*PFUNC_PROFILE)(const std::string &stack, int64_t elapsed_us);
  static PFUNC_PROFILE s_pfunc_profile;
  static bool s_profile;
  static int s_profile_sampling;

  explicit LockProfiler(bool profile);
  ~LockProfiler();

private:
  bool m_profiling;
  timespec m_lockTime;
};

///////////////////////////////////////////////////////////////////////////////

template <typename MutexT>
class BaseConditionalLock {
public:
  BaseConditionalLock(MutexT &mutex, bool condition, bool profile = true)
    : m_profiler(profile), m_mutex(mutex), m_acquired(false) {
    if (condition) {
      m_mutex.lock(); // must not throw
      m_acquired = true;
    }
  }
  ~BaseConditionalLock() {
    if (m_acquired) {
      m_mutex.unlock(); // must not throw
    }
  }
private:
  LockProfiler m_profiler;
  MutexT&      m_mutex;
  bool         m_acquired;
};

class ConditionalLock : public BaseConditionalLock<Mutex> {
public:
  ConditionalLock(Mutex &mutex,
                  bool condition, bool profile = true)
    : BaseConditionalLock<Mutex>(mutex, condition, profile)
  {}
  ConditionalLock(Synchronizable *obj,
                  bool condition, bool profile = true)
    : BaseConditionalLock<Mutex>(obj->getMutex(), condition, profile)
  {}
  ConditionalLock(SynchronizableMulti *obj,
                  bool condition, bool profile = true)
    : BaseConditionalLock<Mutex>(obj->getMutex(), condition, profile)
  {}
};

/**
 * Just a helper class that automatically unlocks a mutex when it goes out of
 * scope.
 *
 * {
 *   Lock lock(mutex);
 *   // inside lock
 * } // unlock here
 */
class Lock : public ConditionalLock {
public:
  explicit Lock(Mutex &mutex, bool profile = true)
    : ConditionalLock(mutex, true, profile) {}
  explicit Lock(Synchronizable *obj, bool profile = true)
    : ConditionalLock(obj, true, profile) {}
  explicit Lock(SynchronizableMulti *obj, bool profile = true)
    : ConditionalLock(obj, true, profile) {}
};

class ScopedUnlock {
public:
  explicit ScopedUnlock(Mutex &mutex) : m_mutex(mutex) {
    m_mutex.unlock();
  }
  explicit ScopedUnlock(Synchronizable *obj) : m_mutex(obj->getMutex()) {
    m_mutex.unlock();
  }
  explicit ScopedUnlock(SynchronizableMulti *obj) : m_mutex(obj->getMutex()) {
    m_mutex.unlock();
  }

  ~ScopedUnlock() {
    m_mutex.lock();
  }

private:
  Mutex &m_mutex;
};

class SimpleConditionalLock : public BaseConditionalLock<SimpleMutex> {
public:
  SimpleConditionalLock(SimpleMutex &mutex,
                        bool condition, bool profile = true)
    : BaseConditionalLock<SimpleMutex>(mutex, condition, profile)
  {
    if (condition) {
      mutex.assertOwnedBySelf();
    }
  }
};

class SimpleLock : public SimpleConditionalLock {
public:
  explicit SimpleLock(SimpleMutex &mutex, bool profile = true)
    : SimpleConditionalLock(mutex, true, profile) {}
};

///////////////////////////////////////////////////////////////////////////////

class ConditionalReadLock {
public:
  ConditionalReadLock(ReadWriteMutex &mutex, bool condition,
                      bool profile = true)
    : m_profiler(profile), m_mutex(mutex), m_acquired(false) {
    if (condition) {
      m_mutex.acquireRead();
      m_acquired = true;
    }
  }

  ~ConditionalReadLock() {
    if (m_acquired) {
      m_mutex.release();
    }
  }

private:
  LockProfiler m_profiler;
  ReadWriteMutex &m_mutex;
  bool m_acquired;
};

class ReadLock : public ConditionalReadLock {
public:
  explicit ReadLock(ReadWriteMutex &mutex, bool profile = true)
    : ConditionalReadLock(mutex, true, profile) {}
};

class WriteLock {
public:
  explicit WriteLock(ReadWriteMutex &mutex, bool profile = true)
    : m_profiler(profile), m_mutex(mutex) {
    m_mutex.acquireWrite();
  }

  ~WriteLock() {
    m_mutex.release();
  }

private:
  LockProfiler m_profiler;
  ReadWriteMutex &m_mutex;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif  // incl_HPHP_LOCK_H_
