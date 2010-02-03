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

#ifndef __LOCK_H__
#define __LOCK_H__

#include "mutex.h"
#include "synchronizable.h"
#include "logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Lock instrumentation for mutex stats.
 */
class LockProfiler {
public:
  typedef void (*PFUNC_PROFILE)(const std::string &stack, int64 elapsed_us);
  static PFUNC_PROFILE s_pfunc_profile;
  static bool s_profile;
  static int s_profile_sampling;

  LockProfiler(bool profile);
  ~LockProfiler();

private:
  bool m_profiling;
  timespec m_lockTime;
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Just a helper class that automatically unlocks a mutex when it goes out of
 * scope.
 *
 * {
 *   Lock lock(mutex);
 *   // inside lock
 * } // unlock here
 */
class Lock {
public:
  Lock(Mutex &mutex, bool profile = true)
    : m_profiler(profile), m_mutex(mutex) {
    m_mutex.lock();
  }
  Lock(Synchronizable *obj, bool profile = true)
    : m_profiler(profile), m_mutex(obj->getMutex()) {
    m_mutex.lock();
  }
  ~Lock() {
    m_mutex.unlock();
  }

private:
  LockProfiler m_profiler;
  Mutex &m_mutex;
};

///////////////////////////////////////////////////////////////////////////////

class ReadLock {
public:
  ReadLock(ReadWriteMutex &mutex, bool profile = true)
    : m_profiler(profile), m_mutex(mutex) {
    m_mutex.acquireRead();
  }

  ~ReadLock() {
    m_mutex.release();
  }

private:
  LockProfiler m_profiler;
  ReadWriteMutex &m_mutex;
};

class WriteLock {
public:
  WriteLock(ReadWriteMutex &mutex, bool profile = true)
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

#endif  // __LOCK_H__
