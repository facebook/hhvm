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
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/util/process.h"
#include "hphp/util/timer.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include <sys/mman.h>

namespace HPHP { namespace jit {
TRACE_SET_MOD(txlease);

namespace {
__thread bool threadCanAcquire = true;

using FuncMap = tbb::concurrent_hash_map<const Func*, pthread_t>;
FuncMap s_funcOwners;
}

Lease::Lease() {
  pthread_mutex_init(&m_lock, nullptr);
}

Lease::~Lease() {
  if (amOwner()) {
    // Can happen, e.g., in exception scenarios.
    pthread_mutex_unlock(&m_lock);
  }
  pthread_mutex_destroy(&m_lock);
}

bool Lease::amOwner() const {
  return m_held.load(std::memory_order_acquire) &&
    pthread_equal(m_owner, pthread_self());
}

bool Lease::mayLock(bool f) {
  std::swap(threadCanAcquire, f);
  return f;
}

// acquire: also returns true if we are already the writer.
bool Lease::acquire(bool blocking /* = false */ ) {
  if (amOwner()) {
    return true;
  }
  if (!threadCanAcquire && !blocking) {
    return false;
  }
  int64_t expire = m_hintExpire;
  int64_t expireDiff = expire - Timer::GetCurrentTimeMicros();
  if (!blocking && (m_held.load(std::memory_order_acquire) ||
                    (expireDiff > 0 && m_owner != pthread_self()))) {
    return false;
  }

  checkRank(RankWriteLease);
  auto const locked = blocking ? pthread_mutex_lock(&m_lock)
                               : pthread_mutex_trylock(&m_lock);
  if (locked == 0) {
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
    }

    m_owner = pthread_self();
    m_hintExpire = 0;
    m_held.store(true, std::memory_order_release);
    return true;
  }

  always_assert(!blocking && "Failed to acquire write lease in blocking mode");
  return false;
}

void Lease::drop(int64_t hintExpireDelay) {
  assertx(amOwner());
  TRACE(4, "thr%" PRIx64 ": dropping lease, called by %p,%p\n",
        Process::GetThreadIdForTrace(), __builtin_return_address(0),
        __builtin_return_address(1));
  if (debug) {
    popRank(RankWriteLease);
  }
  m_hintExpire = hintExpireDelay > 0 ?
    Timer::GetCurrentTimeMicros() + hintExpireDelay : 0;
  m_held.store(false, std::memory_order_release);
  pthread_mutex_unlock(&m_lock);
}

LeaseHolderBase::LeaseHolderBase(Lease& l,
                                 LeaseAcquire acquire,
                                 const Func* func)
  : m_lease(l)
  , m_func{RuntimeOption::EvalJitConcurrently ? func : nullptr}
{
  if (m_func) {
    FuncMap::const_accessor acc;
    auto const self = pthread_self();
    if (s_funcOwners.insert(acc, std::make_pair(m_func, self))) {
      m_acquiredFunc = true;
    } else if (!pthread_equal(acc->second, self)) {
      return;
    }
  }

  if (!m_lease.amOwner()) {
    if (m_func) {
      m_state = LockLevel::Translate;
      return;
    }

    auto const blocking = acquire == LeaseAcquire::BLOCKING;
    m_acquired = m_lease.acquire(blocking);
  }

  if (m_lease.amOwner()) {
    m_state = LockLevel::Write;
  }
}

LeaseHolderBase::~LeaseHolderBase() {
  if (m_acquired && m_lease.amOwner()) {
    m_lease.drop(Lease::kStandardHintExpireInterval);
  }

  if (m_acquiredFunc) {
    auto const removed = s_funcOwners.erase(m_func);
    always_assert(removed);
  }
}

void LeaseHolderBase::acquireBlocking() {
  assertx(!canWrite());

  if (m_lease.amOwner()) {
    m_state = LockLevel::Write;
    return;
  }

  m_acquired = m_lease.acquire(true);
  m_state = LockLevel::Write;
}

}}
