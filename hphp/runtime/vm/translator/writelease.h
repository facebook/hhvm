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
#ifndef _WRITELEASE_H_
#define _WRITELEASE_H_

#include <pthread.h>

namespace HPHP { namespace VM { namespace Transl {

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
  static const int64 kStandardHintExpireInterval = 750;
  pthread_t       m_owner;
  pthread_mutex_t m_lock;
  // m_held: since there's no portable, universally invalid pthread_t,
  // explicitly represent the held <-> unheld state machine.
  volatile bool   m_held;
  int64           m_hintExpire;
  int64           m_hintKept;
  int64           m_hintGrabbed;

  Lease() : m_held(false), m_hintExpire(0), m_hintKept(0), m_hintGrabbed(0) {
    pthread_mutex_init(&m_lock, NULL);
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
  void drop(int64 hintExpireDelay = 0);

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

enum LeaseAcquire {
  ACQUIRE,
  NO_ACQUIRE,
};

struct LeaseHolderBase {
  protected:
    LeaseHolderBase(Lease& l, LeaseAcquire acquire, bool blocking);

  public:
    ~LeaseHolderBase();
    operator bool() const { return m_haveLock; }
    bool acquire();

  private:
    Lease& m_lease;
    bool m_haveLock;
    bool m_acquired;
};
struct LeaseHolder : public LeaseHolderBase {
  LeaseHolder(Lease& l, LeaseAcquire acquire = ACQUIRE)
    : LeaseHolderBase(l, acquire, false) {}
};
struct BlockingLeaseHolder : public LeaseHolderBase {
  BlockingLeaseHolder(Lease& l)
    : LeaseHolderBase(l, ACQUIRE, true) {}
};

}}} // HPHP::VM::Transl

#endif /* _WRITELEASE_H_ */
