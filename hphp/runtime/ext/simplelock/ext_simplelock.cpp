/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/extension.h"

#include "hphp/util/hash-set.h"
#include "hphp/util/rds-local.h"

#include <folly/concurrency/ConcurrentHashMap.h>
#include <atomic>

namespace HPHP {

struct SimpleLockEvent;

namespace {

using LockAtom = std::atomic<SimpleLockEvent*>;
void unlock(LockAtom*);
SimpleLockEvent* const UNLOCKED = (SimpleLockEvent*)(-1);
folly::ConcurrentHashMapSIMD<std::string, LockAtom> s_locks;

RDS_LOCAL(hphp_fast_set<LockAtom*>, tl_heldLocks);

}

struct SimpleLockEvent : AsioExternalThreadEvent {
  ~SimpleLockEvent() override {
    if (m_held) unlock(m_held);
  }

  void acquire(LockAtom* lock) {
    m_held = lock;
    markAsFinished();
  }

  void unserialize(TypedValue& result) override {
    result = make_tv<KindOfNull>();
    tl_heldLocks->emplace(m_held);
    m_held = nullptr;
  }

  std::atomic<SimpleLockEvent*> m_next{nullptr};
private:
  std::atomic<LockAtom*> m_held{nullptr};
};

namespace {

LockAtom* get_lock(const std::string& name) {
  // ConcurrentHashMapSIMD guarantees reference stability across rehashes, so
  // long as we don't erase values from the map it should be safe to continue
  // accessing these references without holding a hazard pointer.
  {
    auto const it = s_locks.find(name);
    if (it != s_locks.end()) return const_cast<LockAtom*>(&it->second);
  }

  auto [it, ins] = s_locks.emplace(name, UNLOCKED);
  return const_cast<LockAtom*>(&it->second);
}

SimpleLockEvent* try_lock(LockAtom* lock) {
  auto expected = UNLOCKED;
  if (lock->compare_exchange_strong(expected, nullptr,
                                    std::memory_order_acq_rel)) {
    return nullptr;
  }

  auto ev = new SimpleLockEvent();
  do {
    ev->m_next = expected;
  } while (!lock->compare_exchange_weak(expected,
                                        expected != UNLOCKED ? ev : nullptr, 
                                        std::memory_order_acq_rel));

  if (expected != UNLOCKED) return ev;
  ev->abandon();
  return nullptr;
}

void unlock(LockAtom* lock) {
  SimpleLockEvent* expected = nullptr;
  auto next = UNLOCKED;
  while (!lock->compare_exchange_weak(expected, next,
                                      std::memory_order_acq_rel)) {
    next = expected ? expected->m_next.load(std::memory_order_relaxed)
                    : UNLOCKED;
  }
  if (expected) expected->acquire(lock);
}

Object HHVM_FUNCTION(lock_mutex, const String& name) {
  auto const l = get_lock(name.toCppString());
  if (auto ev = try_lock(l)) return Object{ev->getWaitHandle()};

  tl_heldLocks->emplace(l);
  return Object{c_StaticWaitHandle::CreateSucceeded(make_tv<KindOfNull>())};
}

bool HHVM_FUNCTION(try_lock_mutex, const String& name) {
  auto const l = get_lock(name.toCppString());
  auto exp = UNLOCKED;
  if (l->compare_exchange_strong(exp, nullptr, std::memory_order_acq_rel)) {
    tl_heldLocks->emplace(l);
    return true;
  }
  return false;
}

void HHVM_FUNCTION(unlock_mutex, const String& name) {
  auto const l = get_lock(name.toCppString());
  if (!tl_heldLocks->erase(l)) {
    SystemLib::throwInvalidOperationExceptionObject(
      "cannot release unheld lock"
    );
  }
  unlock(l);
}

bool HHVM_FUNCTION(is_held, const String& name) {
  auto const l = get_lock(name.toCppString());
  return l->load(std::memory_order_relaxed) != UNLOCKED;
}

struct SimpleLockExtension final : Extension {
  SimpleLockExtension() : Extension("simplelock", "1.0", "sandbox_infra") {}

  bool moduleEnabled() const override { return !Cfg::Repo::Authoritative; }

  void moduleRegisterNative() override {
    HHVM_NAMED_FE(HH\\SimpleLock\\lock, HHVM_FN(lock_mutex));
    HHVM_NAMED_FE(HH\\SimpleLock\\try_lock, HHVM_FN(try_lock_mutex));
    HHVM_NAMED_FE(HH\\SimpleLock\\is_held, HHVM_FN(is_held));
    HHVM_NAMED_FE_STR(
      "HH\\SimpleLock\\unlock", HHVM_FN(unlock_mutex), nativeFuncs()
    );
  }

  void requestShutdown() override {
    while (!tl_heldLocks->empty()) {
      hphp_fast_set<LockAtom*> locks;
      std::swap(*tl_heldLocks, locks);
      for (auto l : locks) unlock(l);
    }
  }

} s_simple_lock_extension;

}

}
