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

#include "hphp/runtime/vm/type-profile.h"

#include <atomic>
#include <cstdint>
#include <queue>
#include <utility>

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/util/atomic-vector.h"

namespace HPHP {

TRACE_SET_MOD(typeProfile);

//////////////////////////////////////////////////////////////////////

void profileInit() {
}

/*
 * Warmup/profiling.
 *
 * In cli mode, we only record samples if we're in recording to replay later.
 *
 * In server mode, we exclude warmup document requests from profiling, then
 * record samples for EvalJitProfileInterpRequests standard requests.
 */

RequestKind __thread requestKind = RequestKind::Warmup;
static bool warmingUp;
static int64_t numRequests;
bool __thread standardRequest = true;
static std::atomic<bool> singleJitLock;
static std::atomic<int> singleJitRequests;
static std::atomic<int> relocateRequests;

void setRelocateRequests(int32_t n) {
  relocateRequests.store(n);
}

namespace {
AtomicVector<uint32_t> s_func_counters{kFuncCountHint, 0};
}

void profileWarmupStart() {
  warmingUp = true;
}

void profileWarmupEnd() {
  warmingUp = false;
}

typedef std::pair<const Func*, uint32_t> FuncHotness;
static bool comp(const FuncHotness& a, const FuncHotness& b) {
  return a.second > b.second;
}

/*
 * Set AttrHot on hot functions. Sort all functions by their profile count, and
 * set AttrHot to the top Eval.HotFuncCount functions.
 */
static Mutex syncLock;
void profileSetHotFuncAttr() {
  static bool synced = false;
  if (LIKELY(synced)) return;

  Lock lock(syncLock);
  if (synced) return;

  /*
   * s_treadmill forces any Funcs that are being destroyed to go through a
   * treadmill pass, to make sure we won't try to dereference something that's
   * being pulled out from under us.
   */
  Func::s_treadmill = true;
  SCOPE_EXIT {
    Func::s_treadmill = false;
  };

  if (RuntimeOption::EvalHotFuncCount) {
    std::priority_queue<FuncHotness,
                        std::vector<FuncHotness>,
                        bool(*)(const FuncHotness& a, const FuncHotness& b)>
      queue(comp);

    Func::getFuncVec().foreach([&](const Func* f) {
      if (!f) return;
      auto const profCounter = [&]() -> uint32_t {
        auto const id = f->getFuncId();
        if (id < s_func_counters.size()) {
          return s_func_counters[id].load(std::memory_order_relaxed);
        }
        return 0;
      }();
      auto fh = FuncHotness(f, profCounter);
      if (queue.size() >= RuntimeOption::EvalHotFuncCount) {
        if (!comp(fh, queue.top())) return;
        queue.pop();
      }
      queue.push(fh);
    });

    while (queue.size()) {
      auto f = queue.top().first;
      queue.pop();
      const_cast<Func*>(f)->setAttrs(f->attrs() | AttrHot);
    }
  }

  // We won't need the counters anymore.  But there might be requests in flight
  // that still thought they were profiling, so we need to clear it on the
  // treadmill.
  Treadmill::enqueue([&] {
    s_func_counters.~AtomicVector<uint32_t>();
    new (&s_func_counters) AtomicVector<uint32_t>{0, 0};
  });

  synced = true;
}

void profileIncrementFuncCounter(const Func* f) {
  s_func_counters.ensureSize(f->getFuncId() + 1);
  s_func_counters[f->getFuncId()].fetch_add(1, std::memory_order_relaxed);
}

int64_t requestCount() {
  return numRequests;
}

static inline bool doneProfiling() {
  return (numRequests >= RuntimeOption::EvalJitProfileInterpRequests) ||
    (RuntimeOption::ClientExecutionMode() &&
     !RuntimeOption::EvalJitProfileRecord);
}

static inline RequestKind getRequestKind() {
  if (warmingUp) return RequestKind::Warmup;
  if (doneProfiling()) return RequestKind::Standard;
  if (RuntimeOption::ServerExecutionMode() ||
      RuntimeOption::EvalJitProfileRecord) return RequestKind::Profile;
  return RequestKind::Standard;
}

void profileRequestStart() {
  requestKind = getRequestKind();

  bool okToJit = requestKind == RequestKind::Standard;
  if (okToJit) {
    jit::Lease::mayLock(true);
    if (singleJitRequests < RuntimeOption::EvalNumSingleJitRequests) {
      bool flag = false;
      if (!singleJitLock.compare_exchange_strong(flag, true)) {
        jit::Lease::mayLock(false);
      }
    }
  }
  if (standardRequest != okToJit) {
    standardRequest = okToJit;
    if (!ThreadInfo::s_threadInfo.isNull()) {
      ThreadInfo::s_threadInfo->m_reqInjectionData.updateJit();
    }
  }

  if (standardRequest && relocateRequests > 0 && !--relocateRequests) {
    jit::liveRelocate(true);
  }
}

void profileRequestEnd() {
  if (warmingUp) return;
  numRequests++; // racy RMW; ok to miss a rare few.
  if (standardRequest &&
      singleJitRequests < RuntimeOption::EvalNumSingleJitRequests &&
      jit::Lease::mayLock(true)) {
    assert(singleJitLock);
    ++singleJitRequests;
    singleJitLock = false;
    if (RuntimeOption::ServerExecutionMode()) {
      Logger::Warning("Finished singleJitRequest %d", singleJitRequests.load());
    }
  }
}

}
