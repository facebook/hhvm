/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/treadmill.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/process.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"

#include <folly/portability/SysMman.h>

namespace HPHP { namespace jit {
TRACE_SET_MOD(txlease);

namespace {
/*
 * A Lease is a roughly a mutex with the ability to perform a "hinted drop":
 * the Lease is unlocked but all calls to acquire(false) from threads other
 * than the last owner will fail for a short period of time. This is used to
 * help provide better code locality in the translation cache.
 */
struct Lease {
  Lease();
  ~Lease();

  /*
   * Returns true iff the Lease is locked and the current thread is the owner.
   */
  bool amOwner() const;

  /*
   * Returns true iff the current thread would be able to acquire the lease now
   * (or already owns it).
   */
  bool couldAcquire() const;

  // acquire: also returns true if we are already the writer.
  bool acquire(bool blocking = false);

  /*
   * Drop this lease with an expiration hint in microseconds.
   */
  void drop(int64_t hintExpireDelay = 0);

private:
  // Since there's no portable, universally invalid pthread_t, explicitly
  // represent the held <-> unheld state machine with m_held.
  pthread_t         m_owner;
  std::atomic<bool> m_held{false};
  pthread_mutex_t   m_lock;

  // Timestamp for when a hinted lease drop (see block comment above) will
  // expire.
  int64_t m_hintExpire{0};
};

static __thread bool threadCanAcquire = true;

AtomicVector<int64_t> s_funcOwners{0, Treadmill::kInvalidRequestIdx};
static InitFiniNode s_funcOwnersReinit([]{
  UnsafeReinitEmptyAtomicVector(
    s_funcOwners, RuntimeOption::EvalFuncCountHint);
}, InitFiniNode::When::PostRuntimeOptions, "s_funcOwners reinit");

std::atomic<int> s_jittingThreads{0};

Lease s_globalLease;
Lease s_liveLease;
Lease s_optimizeLease;

Lease::Lease() {
  pthread_mutex_init(&m_lock, nullptr);
}

Lease::~Lease() {
  always_assert(!amOwner());
  pthread_mutex_destroy(&m_lock);
}

bool Lease::amOwner() const {
  return m_held.load(std::memory_order_acquire) &&
    pthread_equal(m_owner, pthread_self());
}

// acquire: also returns true if we are already the writer.
bool Lease::acquire(bool blocking /* = false */ ) {
  if (amOwner()) return true;
  if (!threadCanAcquire && !blocking) return false;

  auto const expireDiff = m_hintExpire - Timer::GetCurrentTimeMicros();
  if (!blocking && (m_held.load(std::memory_order_acquire) ||
                    (expireDiff > 0 && m_owner != pthread_self()))) {
    return false;
  }

  checkRank(RankWriteLease);
  auto const locked = blocking ? pthread_mutex_lock(&m_lock)
                               : pthread_mutex_trylock(&m_lock);
  if (locked == 0) {
    if (debug) pushRank(RankWriteLease);

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
  if (debug) popRank(RankWriteLease);

  m_hintExpire = hintExpireDelay > 0 ?
    Timer::GetCurrentTimeMicros() + hintExpireDelay : 0;
  m_held.store(false, std::memory_order_release);
  pthread_mutex_unlock(&m_lock);
}

bool Lease::couldAcquire() const {
  auto const self = pthread_self();
  if (m_held.load(std::memory_order_acquire)) return m_owner == self;

  return m_owner == self || Timer::GetCurrentTimeMicros() > m_hintExpire;
}

LockLevel lockLevel(TransKind k) {
  if (RuntimeOption::EvalJitConcurrently == 0) return LockLevel::Global;

  switch (k) {
    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Invalid:
      break;
    case TransKind::ProfPrologue:
    case TransKind::Profile:
      return LockLevel::Func;
    case TransKind::OptPrologue:
    case TransKind::Optimize:
      return RuntimeOption::EvalJitConcurrently >= 2 ? LockLevel::Func
                                                     : LockLevel::Kind;
    case TransKind::LivePrologue:
    case TransKind::Live:
      return RuntimeOption::EvalJitConcurrently >= 3 ? LockLevel::Func
                                                     : LockLevel::Kind;
  }
  always_assert(false);
}

Lease& kindLease(TransKind k) {
  switch (k) {
    case TransKind::Anchor:
    case TransKind::Interp:
    case TransKind::Invalid:
    case TransKind::ProfPrologue:
    case TransKind::Profile:
      break;
    case TransKind::OptPrologue:
    case TransKind::Optimize:
      return s_optimizeLease;
    case TransKind::LivePrologue:
    case TransKind::Live:
      return s_liveLease;
  }
  always_assert(false);
}
}

void setMayAcquireLease(bool f) {
  threadCanAcquire = f;
}

bool couldAcquireOptimizeLease(const Func* func) {
  switch (lockLevel(TransKind::Optimize)) {
    case LockLevel::None:
      break;
    case LockLevel::Func: {
      auto const funcId = func->getFuncId();
      s_funcOwners.ensureSize(funcId + 1);
      auto const owner = s_funcOwners[funcId].load(std::memory_order_relaxed);
      auto const self = Treadmill::requestIdx();
      return owner == self || owner == Treadmill::kInvalidRequestIdx;
    }
    case LockLevel::Kind:
      return s_optimizeLease.couldAcquire();
    case LockLevel::Global:
      return s_globalLease.couldAcquire();
  }
  always_assert(false);
}

LeaseHolder::LeaseHolder(const Func* func, TransKind kind, bool isWorker)
  : m_func{RuntimeOption::EvalJitConcurrently > 0 ? func : nullptr}
{
  if (!threadCanAcquire) return;
  assertx(func || RuntimeOption::EvalJitConcurrently == 0);
  auto const level = m_func ? lockLevel(kind) : LockLevel::Global;

  if (level == LockLevel::Global && !s_globalLease.amOwner()) {
    auto const blocking = RuntimeOption::EvalJitRequireWriteLease &&
      RuntimeOption::EvalJitConcurrently == 0;
    if (!(m_acquiredGlobal = s_globalLease.acquire(blocking))) return;
  }

  SCOPE_EXIT { if (m_level == LockLevel::None) dropLocks(); };

  if (level == LockLevel::Kind && !acquireKind(kind)) return;

  if (m_func) {
    auto const funcId = m_func->getFuncId();
    s_funcOwners.ensureSize(funcId + 1);
    auto& owner = s_funcOwners[funcId];
    auto oldOwner = owner.load(std::memory_order_relaxed);
    auto const self = Treadmill::requestIdx();

    if (oldOwner == self) {
      // We already have the lock on this Func.
    } else if (oldOwner != Treadmill::kInvalidRequestIdx) {
      // Already owned by another thread.
      return;
    } else {
      // Unowned. Try to grab it. Only non-worker threads with LockLevel::Func
      // count towards the Eval.JitThreads limit.
      if (!isWorker && level == LockLevel::Func) {
        auto threads = s_jittingThreads.load(std::memory_order_relaxed);
        if (threads >= RuntimeOption::EvalJitThreads) return;

        threads = s_jittingThreads.fetch_add(1, std::memory_order_relaxed);
        m_acquiredThread = true;
        if (threads >= RuntimeOption::EvalJitThreads) return;
      }

      assertx(oldOwner == Treadmill::kInvalidRequestIdx);
      if (!owner.compare_exchange_strong(oldOwner, self,
                                         std::memory_order_acq_rel)) {
        return;
      }
      m_acquiredFunc = true;
    }
  }

  // If we made it this far, we acquired all the locks we need to translate.
  m_level = level;
}

LeaseHolder::~LeaseHolder() {
  dropLocks();
}

bool LeaseHolder::acquireKind(TransKind kind) {
  auto& lease = kindLease(kind);
  if (lease.amOwner()) return true;

  if (lease.acquire()) {
    m_acquiredKind = kind;
    return true;
  }

  return false;
}

void LeaseHolder::dropLocks() {
  if (m_acquiredThread) {
    s_jittingThreads.fetch_sub(1, std::memory_order_relaxed);
    m_acquiredThread = false;
  }

  if (m_acquiredFunc) {
    auto& owner = s_funcOwners[m_func->getFuncId()];
    owner.store(Treadmill::kInvalidRequestIdx, std::memory_order_release);
    m_acquiredFunc = false;
  }

  if (m_acquiredKind != TransKind::Invalid) {
    kindLease(m_acquiredKind).drop(RuntimeOption::EvalJitWriteLeaseExpiration);
    m_acquiredKind = TransKind::Invalid;
  }

  if (m_acquiredGlobal) {
    assertx(s_globalLease.amOwner());
    s_globalLease.drop(RuntimeOption::EvalJitWriteLeaseExpiration);
    m_acquiredGlobal = false;
  }
}

bool LeaseHolder::checkKind(TransKind kind) {
  assertx(m_level != LockLevel::None);

  auto const level = lockLevel(kind);
  if (level == m_level) return true;

  if (level == LockLevel::Kind && !acquireKind(kind)) return false;

  if (level == LockLevel::Global &&
      !s_globalLease.amOwner() &&
      !(m_acquiredGlobal = s_globalLease.acquire())) {
    return false;
  }

  m_level = level;
  return true;
}

}}
