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
#ifndef incl_HPHP_WRITELEASE_H_
#define incl_HPHP_WRITELEASE_H_

#include "hphp/util/assertions.h"
#include "hphp/util/compilation-flags.h"
#include "hphp/runtime/base/runtime-option.h"

#include <pthread.h>

namespace HPHP {

struct Func;

namespace jit {

/*
 * The write Lease guards write access to the translation caches, srcDB, and
 * TransDB. The term "lease" is meant to indicate that the right of ownership
 * is conferred for a long, variable time: often the entire length of a
 * request. If a request is not actively translating, it will perform a "hinted
 * drop" of the lease: the lease is unlocked but all calls to acquire(false)
 * from other threads will fail for a short period of time.
 */

struct LeaseHolderBase;
struct Lease {
  friend struct LeaseHolderBase;

  Lease();
  ~Lease();

  /*
   * Returns true iff the Lease is locked and the current thread is the owner.
   */
  bool amOwner() const;

  int64_t hintKept()    const { return m_hintKept; }
  int64_t hintGrabbed() const { return m_hintGrabbed; }

  static bool mayLock(bool f);

private:
  // acquire: also returns true if we are already the writer.
  bool acquire(bool blocking = false);
  void drop(int64_t hintExpireDelay = 0);
  void gremlinUnlockImpl();

  // Since there's no portable, universally invalid pthread_t, explicitly
  // represent the held <-> unheld state machine with m_held.
  pthread_t         m_owner;
  std::atomic<bool> m_held{false};
  pthread_mutex_t   m_lock;

  // Timestamp for when a hinted lease drop (see block comment above) will
  // expire.
  int64_t m_hintExpire{0};

  // Statistics about how many times the write lease is picked back up by the
  // thread that did a hinted drop.
  int64_t m_hintKept{0};
  int64_t m_hintGrabbed{0};
};

enum class LeaseAcquire {
  ACQUIRE,
  BLOCKING
};

struct LeaseHolderBase {
  ~LeaseHolderBase();

  /*
   * Attempt a non-blocking acquisition of the write lease if not already held.
   *
   * Returns true iff this thread already owned it or was able to acquire it.
   */
  bool acquire();

  /*
   * Perform a blocking acquire of the write lease. Will never fail, but may
   * take a long time to return.
   */
  void acquireBlocking();

  /*
   * Returns true iff the thread owning this LeaseHolder may proceed with the
   * unsynchronized first phase of translation.
   */
  bool canTranslate() const {
    switch (m_state) {
      case LockLevel::None:
        return false;
      case LockLevel::Translate:
      case LockLevel::Write:
        return true;
    }
    not_reached();
  }

  /*
   * Returns true iff the thread owning this LeaseHolder may write to the
   * translation cache.
   */
  bool canWrite() const {
    switch (m_state) {
      case LockLevel::None:
      case LockLevel::Translate:
        return false;
      case LockLevel::Write:
        return true;
    }
    not_reached();
  }

 protected:
  LeaseHolderBase(Lease& l, LeaseAcquire acquire, const Func* f = nullptr);

 private:
  enum class LockLevel {
    None,
    Translate,
    Write,
  };

  Lease& m_lease;
  LockLevel m_state{LockLevel::None};
  bool m_acquired{false};
  const Func* m_func;
  bool m_acquiredFunc{false};
};

struct LeaseHolder : public LeaseHolderBase {
  explicit LeaseHolder(Lease& l)
    : LeaseHolderBase(l,
                      RuntimeOption::EvalJitRequireWriteLease ?
                      LeaseAcquire::BLOCKING : LeaseAcquire::ACQUIRE)
  {}
  explicit LeaseHolder(Lease& l, const Func* func)
    : LeaseHolderBase(l, LeaseAcquire::ACQUIRE, func)
  {}
};

struct BlockingLeaseHolder : public LeaseHolderBase {
  explicit BlockingLeaseHolder(Lease& l)
    : LeaseHolderBase(l, LeaseAcquire::BLOCKING) {}
};

}} // HPHP::jit

#endif /* incl_HPHP_WRITELEASE_H_ */
