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
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/util/process.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include <sys/mman.h>

namespace HPHP { namespace JIT {
TRACE_SET_MOD(txlease);

bool
Lease::amOwner() const {
  return m_held && m_owner == pthread_self();
}

/*
 * Multi-threaded scenarios for lease interleaving are hard to tease out.
 * The "gremlin" is a pseudo-thread that sometimes steals our lease while
 * we're running in the TC.
 *
 * The gremlin's pthread id is the bitwise negation of our own. Yes,
 * hypothetical wacky pthread implementations can break this. It's
 * DEBUG-only, folks.
 */
static inline pthread_t gremlinize_threadid(pthread_t tid) {
  return (pthread_t)(~((int64_t)tid));
}

void Lease::gremlinLock() {
  if (amOwner()) {
    TRACE(2, "Lease: gremlinLock dropping lease\n");
    drop();
  }
  pthread_mutex_lock(&m_lock);
  TRACE(2, "Lease: gremlin grabbed lock\n ");
  m_held = true;
  m_owner = gremlinize_threadid(pthread_self());
}

void
Lease::gremlinUnlockImpl() {
  if (m_held && m_owner == gremlinize_threadid(pthread_self())) {
    TRACE(2, "Lease: gremlin dropping lock\n ");
    pthread_mutex_unlock(&m_lock);
    m_held = false;
  }
}

// acquire: also returns true if we are already the writer.
bool Lease::acquire(bool blocking /* = false */ ) {
  if (amOwner()) {
    return true;
  }
  int64_t expire = m_hintExpire;
  int64_t expireDiff = expire - Timer::GetCurrentTimeMicros();
  if (!blocking && (m_held ||
                    (expireDiff > 0 && m_owner != pthread_self()))) {
    return false;
  }
  checkRank(RankWriteLease);
  if (0 == (blocking ?
            pthread_mutex_lock(&m_lock) :
            pthread_mutex_trylock(&m_lock))) {
    TRACE(4, "thr%" PRIx64 ": acquired lease, called by %p,%p\n",
          Process::GetThreadIdForTrace(), __builtin_return_address(0),
          __builtin_return_address(1));
    if (debug) {
      pushRank(RankWriteLease);
      if (expire != 0 && m_owner != pthread_self()) {
        m_hintGrabbed++;
        TRACE(3,
              "thr%" PRIx64 ": acquired hinted lease"
              ", expired %" PRId64 "us ago\n",
              Process::GetThreadIdForTrace(), -expireDiff);
      } else if (expire != 0 && m_owner == pthread_self()) {
        m_hintKept++;
      }
      mcg->code.unprotect();
    }

    m_owner = pthread_self();
    m_hintExpire = 0;
    m_held = true;
    return true;
  }
  if (blocking) {
    TRACE(3, "thr%" PRIx64 ": failed to acquired lease in blocking mode\n",
          Process::GetThreadIdForTrace());
  }
  return false;
}

void Lease::drop(int64_t hintExpireDelay) {
  assert(amOwner());
  TRACE(4, "thr%" PRIx64 ": dropping lease, called by %p,%p\n",
        Process::GetThreadIdForTrace(), __builtin_return_address(0),
        __builtin_return_address(1));
  if (debug) {
    popRank(RankWriteLease);
    mcg->code.protect();
  }
  m_hintExpire = hintExpireDelay > 0 ?
    Timer::GetCurrentTimeMicros() + hintExpireDelay : 0;
  m_held = false;
  pthread_mutex_unlock(&m_lock);
}

LeaseHolderBase::LeaseHolderBase(Lease& l, LeaseAcquire acquire)
    : m_lease(l), m_haveLock(false), m_acquired(false) {
  if (!m_lease.amOwner() && (acquire == LeaseAcquire::ACQUIRE ||
                             acquire == LeaseAcquire::BLOCKING)) {
    bool blocking = (acquire == LeaseAcquire::BLOCKING);
    m_acquired = m_lease.acquire(blocking);
  }
  m_haveLock = m_lease.amOwner();
}

LeaseHolderBase::~LeaseHolderBase() {
  if (m_acquired && m_lease.amOwner()) {
    m_lease.drop(Lease::kStandardHintExpireInterval);
  }
}

bool LeaseHolderBase::acquire() {
  assert(!m_acquired);
  assert(m_haveLock == m_lease.amOwner());
  if (m_haveLock) {
    return true;
  }
  return m_haveLock = m_acquired = m_lease.acquire();
}

}}
