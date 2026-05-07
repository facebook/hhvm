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

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/code-view.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/service-data.h"

namespace HPHP::jit {

TRACE_SET_MOD(mcg)

CodeCacheViews::CodeCacheViews(int maxViews)
  : m_maxViews{maxViews} {
  if (!Cfg::Jit::EnableConcurrentCodeViews) return;
  auto const ss = CodeCache::sectionSizes(TransKind::Anchor, nullptr);
  // Anchor views only emit to cold and data. Alias main/cold to frozen
  // to avoid wasting TC space on unused sections.
  auto* cold = tc::code().releaseCold(ss.cold);
  auto* data = tc::code().releaseData(ss.data);
  CodeCache::View newView(*cold, *cold, *cold, *data, false, false);
  m_views.insert(kAnchorTid, newView);
  m_numSlots.fetch_add(1);
}

bool CodeCacheViews::reserveSlot() {
  auto cur = m_numSlots.load(std::memory_order_acquire);
  while (cur < maxViews()) {
    if (m_numSlots.compare_exchange_weak(cur, cur + 1, std::memory_order_acq_rel)) {
      return true;
    }
  }
  return false;
}

bool CodeCacheViews::doTryAcquire(pthread_t tid) {
  if (!Cfg::Jit::EnableConcurrentCodeViews) return true;
  auto now = std::chrono::steady_clock::now();
  auto nowMs = SlotState::toMs(now);

  // Fast path: re-activate our own slot via CAS.
  while (true) {
    auto it = m_slots.find(tid);
    if (it == m_slots.end()) break;
    auto state = it->second;
    SlotState next{state.activeCount + 1u, nowMs};
    if (m_slots.assign_if_equal(tid, state, next)) return true;
  }

  // Try to create a new slot before attempting to steal.
  // This should ideally happen before stealing.
  if (reserveSlot()) {
    // It's okay to delete this because if tid's view was stolen,
    // the stealer got a copy of its view before deleting the slot.
    m_views.erase(tid);  
    always_assert(m_slots.insert(tid, SlotState{1, nowMs}).second);
    return true;
  }

  // At capacity. Try to steal a slot.
  auto last = m_lastStealTime.load(std::memory_order_relaxed);
  if (now < last + std::chrono::milliseconds(Cfg::CodeCache::StealIntervalMs) ||
      !m_lastStealTime.compare_exchange_strong(
          last, now, std::memory_order_relaxed)) {
    return false;
  }

  for (auto it = m_slots.begin(); it != m_slots.end(); ++it) {
    auto threadId = it->first;
    if (isFull(threadId)) continue;

    auto state = it->second;
    if (state.activeCount > 0) continue;
    if (nowMs < state.deadlineMs) continue;

    auto vit = m_views.find(threadId);
    // Only one thread will win this CAS.
    if (!m_slots.erase_if_equal(threadId, state)) continue;

    // Take victim's view if it exists; stealer will create a fresh one
    // in view() if not. Victim erases its own stale view next time it
    // acquires a slot.
    if (vit == m_views.end()) {
      m_views.erase(tid);
    } else {
      m_views.insert_or_assign(tid, vit->second);
    }
    always_assert(m_slots.insert(tid, SlotState{1, nowMs}).second);
    return true;
  }

  return false;
}

void CodeCacheViews::acquire(pthread_t tid) {
  while (!doTryAcquire(tid)) {
    std::unique_lock lk{m_cvMutex};
    m_cv.wait_for(lk, std::chrono::milliseconds(1));
  }
}

bool CodeCacheViews::tryAcquire(pthread_t tid) {
  return doTryAcquire(tid);
}

void CodeCacheViews::setFull(pthread_t tid) {
  m_threadsFull.insert(tid, true);
}

bool CodeCacheViews::isFull(pthread_t tid) {
  return m_threadsFull.find(tid) != m_threadsFull.end();
}

int CodeCacheViews::numThreadsFull() {
  return m_threadsFull.size();
}

CodeCache::View CodeCacheViews::view(TransKind kind, const tc::TransRange* sizes, pthread_t tid,
                                     bool threadAffinity, bool codeLocked) {
  if (!threadAffinity) return tc::code().view(kind, sizes);

  assertx(tid == kAnchorTid || [&] {
    auto it = m_slots.find(tid);
    return it != m_slots.end() && it->second.activeCount > 0;
  }());

  auto it = m_views.find(tid);

  if (it != m_views.end()) {
    auto const ss = CodeCache::sectionSizes(kind, sizes);
    auto view = it->second;
    bool updated = false;

    if (!view.main().canEmit(ss.main)) {
      auto codeLock = tc::lockCode(!codeLocked);
      view.setMain(tc::code().releaseMain(ss.main));
      updated = true;
    }

    if (!view.cold().canEmit(ss.cold)) {
      auto codeLock = tc::lockCode(!codeLocked);
      view.setCold(tc::code().releaseCold(ss.cold));
      updated = true;
    }

    if (!view.frozen().canEmit(ss.frozen)) {
      auto codeLock = tc::lockCode(!codeLocked);
      view.setFrozen(tc::code().releaseFrozen(ss.frozen));
      updated = true;
    }

    if (!view.data().canEmit(ss.data)) {
      auto codeLock = tc::lockCode(!codeLocked);
      view.setData(tc::code().releaseData(ss.data));
      updated = true;
    }

    if (updated) {
      m_views.insert_or_assign(tid, view);
    }

    return view;
  }

  auto codeLock = tc::lockCode(!codeLocked);
  auto newView = tc::code().view(kind, sizes, true);
  m_views.insert_or_assign(tid, newView);
  return newView;
}

void CodeCacheViews::release(pthread_t tid, std::chrono::milliseconds dropHint) {
  if (!Cfg::Jit::EnableConcurrentCodeViews) {
    return;
  }

  while (true) {
    auto it = m_slots.find(tid);
    assertx(it != m_slots.end() && it->second.activeCount > 0);
    auto state = it->second;
    if (state.activeCount == 1) {
      auto deadlineMs = SlotState::toMs(std::chrono::steady_clock::now() + dropHint);
      SlotState next{0, deadlineMs};
      if (m_slots.assign_if_equal(tid, state, next)) {
        m_cv.notify_one();
        return;
      }
    } else {
      SlotState next{state.activeCount - 1u, state.deadlineMs};
      if (m_slots.assign_if_equal(tid, state, next)) return;
    }
  }
}

ViewHolder::ViewHolder(pthread_t threadId, bool blocking,
                       std::chrono::milliseconds dropHint)
  : m_dropHint(dropHint) {
  if (blocking) {
    tc::codeViews().acquire(threadId);
    m_tid = threadId;
  } else if (tc::codeViews().tryAcquire(threadId)) {
    m_tid = threadId;
  }
}

ViewHolder::~ViewHolder() {
  if (m_tid) {
    tc::codeViews().release(m_tid.value(), m_dropHint);
  }
}

bool ViewHolder::isValid() const {
  return m_tid.has_value();
}

int CodeCacheViews::numViews() const {
  return m_numSlots.load(std::memory_order_acquire);
}

int CodeCacheViews::maxViews() const {
  return m_maxViews.load(std::memory_order_acquire) + 1; // 1 more for Anchor
}

namespace tc { extern CodeCacheViews* g_views; }

namespace {
ServiceData::CounterCallback s_viewCounters(
  [](ServiceData::CounterMap& counters) {
    if (!tc::g_views) return;
    counters["jit.code_views.count"] = tc::g_views->numViews();
  },
  "jit.code_views."
);
}

}
