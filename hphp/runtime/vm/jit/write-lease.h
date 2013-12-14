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
#ifndef incl_HPHP_WRITELEASE_H_
#define incl_HPHP_WRITELEASE_H_

#include "hphp/util/base.h"
#include "hphp/runtime/base/runtime-option.h"

#include <pthread.h>

namespace HPHP { namespace JIT {

/*
 * The write Lease guards write access to the translation caches,
 * srcDB, and TransDB. The term "lease" is meant to indicate that
 * the right of ownership is conferred for a long, variable time:
 * often the entire length of a request. If a request is not
 * actively translating, it will perform a "hinted drop" of the lease:
 * the lease is unlocked but all calls to acquire(false) from other
 * threads will fail for a short period of time.
 */

struct Lease {
  static const int64_t kStandardHintExpireInterval = 750;
  pthread_t       m_owner;
  pthread_mutex_t m_lock;
  // m_held: since there's no portable, universally invalid pthread_t,
  // explicitly represent the held <-> unheld state machine.
  volatile bool   m_held;
  int64_t           m_hintExpire;
  int64_t           m_hintKept;
  int64_t           m_hintGrabbed;

  Lease() : m_held(false), m_hintExpire(0), m_hintKept(0), m_hintGrabbed(0) {
    pthread_mutex_init(&m_lock, nullptr);
  }
  ~Lease() {
    if (m_held && m_owner == pthread_self()) {
      // Can happen, e.g., in exception scenarios.
      pthread_mutex_unlock(&m_lock);
    }
    pthread_mutex_destroy(&m_lock);
  }
  bool amOwner() const;
  // acquire: also returns true if we are already the writer.
  bool acquire(bool blocking = false);
  void drop(int64_t hintExpireDelay = 0);

  /*
   * A malevolent entity sometimes takes the write lease out from under us
   * for debugging purposes.
   */
  void gremlinLock();
  void gremlinUnlock() {
    if (debug) { gremlinUnlockImpl(); }
  }

private:
  void gremlinUnlockImpl();
};

enum class LeaseAcquire {
  ACQUIRE,
  NO_ACQUIRE,
  BLOCKING
};

struct LeaseHolderBase {
  protected:
    LeaseHolderBase(Lease& l, LeaseAcquire acquire);

  public:
    ~LeaseHolderBase();
    explicit operator bool() const { return m_haveLock; }
    bool acquire();

  private:
    Lease& m_lease;
    bool m_haveLock;
    bool m_acquired;
};
struct LeaseHolder : public LeaseHolderBase {
  explicit LeaseHolder(Lease& l, LeaseAcquire acquire)
    : LeaseHolderBase(l, acquire) {}
  explicit LeaseHolder(Lease& l)
    : LeaseHolderBase(l,
                      RuntimeOption::EvalJitRequireWriteLease ?
                      LeaseAcquire::BLOCKING : LeaseAcquire::ACQUIRE)
    {}
};
struct BlockingLeaseHolder : public LeaseHolderBase {
  explicit BlockingLeaseHolder(Lease& l)
    : LeaseHolderBase(l, LeaseAcquire::BLOCKING) {}
};

}} // HPHP::JIT

#endif /* incl_HPHP_WRITELEASE_H_ */
