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
#include <sys/mman.h>

#include <util/timer.h>
#include <runtime/vm/bytecode.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/translator/writelease.h>

namespace HPHP { namespace VM { namespace Transl {
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
  return (pthread_t)(~((int64)tid));
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
  int64 expire = m_hintExpire;
  int64 expireDiff = expire - Timer::GetCurrentTimeMicros();
  if (!blocking && (m_held ||
                    (expireDiff > 0 && m_owner != pthread_self()))) {
    return false;
  }
  checkRank(RankWriteLease);
  if (0 == (blocking ?
            pthread_mutex_lock(&m_lock) :
            pthread_mutex_trylock(&m_lock))) {
    TRACE(4, "thr%lx: acquired lease, called by %p,%p\n",
          pthread_self(), __builtin_return_address(0),
          __builtin_return_address(1));
    if (debug) {
      pushRank(RankWriteLease);
      if (expire != 0 && m_owner != pthread_self()) {
        m_hintGrabbed++;
        TRACE(3, "thr%lx acquired hinted lease: expired %lldus ago\n",
              pthread_self(), -expireDiff);
      } else if (expire != 0 && m_owner == pthread_self()) {
        m_hintKept++;
      }
      Translator::Get()->unprotectCode();
    }

    m_owner = pthread_self();
    m_hintExpire = 0;
    m_held = true;
    return true;
  }
  if (blocking) {
    TRACE(3, "thr%lx: failed to acquired lease in blocking mode\n",
          pthread_self());
  }
  return false;
}

void Lease::drop(int64 hintExpireDelay) {
  ASSERT(amOwner());
  TRACE(4, "thr%lx: dropping lease, called by %p,%p\n",
        pthread_self(), __builtin_return_address(0),
        __builtin_return_address(1));
  if (debug) {
    popRank(RankWriteLease);
    Translator::Get()->protectCode();
  }
  m_hintExpire = hintExpireDelay > 0 ?
    Timer::GetCurrentTimeMicros() + hintExpireDelay : 0;
  m_held = false;
  pthread_mutex_unlock(&m_lock);
}

LeaseHolderBase::LeaseHolderBase(Lease& l, LeaseAcquire acquire,
                                                bool blocking)
  : m_lease(l), m_haveLock(false), m_acquired(false) {
  ASSERT(IMPLIES(blocking, acquire == ACQUIRE));
  if (!m_lease.amOwner() && acquire == ACQUIRE) {
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
  ASSERT(!m_acquired);
  ASSERT(m_haveLock == m_lease.amOwner());
  if (m_haveLock) {
    return true;
  }
  return m_haveLock = m_acquired = m_lease.acquire();
}

}}}
