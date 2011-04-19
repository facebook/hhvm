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

#ifndef __HPHP_FIBER_SAFE_H__
#define __HPHP_FIBER_SAFE_H__

#include <util/base.h>
#include <util/mutex.h>
#include <runtime/base/fiber_reference_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
/**
 * In general, fiber safety can be achieved by either working on fiber's own
 * copy or sharing the same object but with locking. We provide base classes
 * for these approaches:
 *
 *   1. To create a fiber's own object, most of time it's enough to create a
 *      completely new object. For examples, most extensions can have their
 *      own thread local statics re-initialized when a fiber is started. Their
 *      values can be created without copying anything from main thread's.
 *      In this case, nothing in this file is needed. ThreadLocal<T> takes
 *      care of it.
 *
 *   2. If an object needs to copy certain members, when creating fiber's own
 *      copy, derive a new class from FiberLocal. Implement fiberInit() and
 *      fiberExit() to marshal and unmarshal data members.
 *
 *   3. If an object has to be shared to work, derive it from FiberSafe class,
 *      mark it as shared object by calling incFiberCount() in fiberInit()
 *      and calling decFiberCount() in fiberExit(). Then protect class's
 *      methods with FiberLocks.
 */
///////////////////////////////////////////////////////////////////////////////

/**
 * Derive from this class to intercept fiber events to marshal/unmarshal
 * between fibers.
 */
class FiberLocal {
public:
  FiberLocal();
  virtual ~FiberLocal();

  /**
   * "src" can be safely casted to your own derived class. Note "this" is,
   *
   *    FiberThis->fiberInit(MainThis);
   *    MainThis->fiberExit(FiberThis);
   */
  virtual void fiberInit(FiberLocal *src, FiberReferenceMap &refMap) = 0;
  virtual void fiberExit(FiberLocal *src, FiberReferenceMap &refMap) = 0;
};

///////////////////////////////////////////////////////////////////////////////

#ifndef EXTRA_FIBER_LOCKING
#ifdef DEBUG
#define EXTRA_FIBER_LOCKING 1
#endif
#elif !DEBUG
#undef EXTRA_FIBER_LOCKING
#endif

/**
 * Derive from this class to share the same object between two or more fibers.
 */
class FiberSafe {
public:
  FiberSafe() : m_fiberCount(0) {}

  void incFiberCount() { ++m_fiberCount;}
  void decFiberCount() { --m_fiberCount;}

protected:
  friend class FiberReadLock;
  friend class FiberWriteLock;
  friend class FiberReadLockViolator;
  friend class FiberWriteLockViolator;
  int m_fiberCount; // how many fibers are sharing me
  mutable ReadWriteMutex m_fiberMutex;

  inline bool shouldLock() const {
    /*
     * As an optimization, avoid costly lock operations when no
     * fiber is present. However, since fibers are rarely present, we
     * don't get very good debugging coverage of FiberSafe synchronization
     * without them. So lock them regardless in debug builds.
     */
#if EXTRA_FIBER_LOCKING
    ASSERT(m_fiberCount >= 0);
    return true;
#else
    return m_fiberCount > 0;
#endif
  }
};

#define IMPLEMENT_FIBER_SAFE_COUNTABLE          \
  public:                                       \
  void incRefCount() const {                    \
    FiberLock lock(this);                       \
    Countable::incRefCount();                   \
  }                                             \
  int decRefCount() const {                     \
    FiberLock lock(this);                       \
    return Countable::decRefCount();            \
  }                                             \

///////////////////////////////////////////////////////////////////////////////

/**
 * A FiberSafe object can use these locks to protect its methods:
 *
 *    class MyClass : public FiberSafe {
 *    public:
 *      void foo() {
 *        FiberLock lock(this); // this is no-op when not running with fibers
 *      }
 *    };
 */
class FiberReadLock {
public:
  FiberReadLock(const FiberSafe *obj) : m_obj(obj) {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.acquireRead();
  }
  ~FiberReadLock() {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.release();
  }

private:
  const FiberSafe *m_obj;
};

class FiberWriteLock {
public:
  FiberWriteLock(const FiberSafe *obj) : m_obj(obj) {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.acquireWrite();
  }
  ~FiberWriteLock() {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.release();
  }

private:
  const FiberSafe *m_obj;
};

/*
 * Do not use this class (unless you have to).
 * It temporarily unlocks something that was locked with FiberReadLock
 */
class FiberReadLockViolator {
public:
  FiberReadLockViolator(const FiberSafe *obj) : m_obj(obj) {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.release();
  }
  ~FiberReadLockViolator() {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.acquireRead();
  }

private:
  const FiberSafe *m_obj;
};

/*
 * Do not use this class (unless you have to).
 * It temporarily unlocks something that was locked with FiberWriteLock
 */
class FiberWriteLockViolator {
public:
  FiberWriteLockViolator(const FiberSafe *obj) : m_obj(obj) {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.release();
  }
  ~FiberWriteLockViolator() {
    if (m_obj && m_obj->shouldLock()) m_obj->m_fiberMutex.acquireWrite();
  }

private:
  const FiberSafe *m_obj;
};


typedef FiberWriteLock FiberLock;

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_FIBER_SAFE_H__
