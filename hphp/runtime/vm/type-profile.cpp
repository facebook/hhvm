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

#include "hphp/runtime/vm/type-profile.h"

#include <atomic>
#include <cstdint>
#include <queue>
#include <utility>

#include <tbb/concurrent_hash_map.h>

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/server/ext_server.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/runtime/vm/jit/relocation.h"
#include "hphp/runtime/vm/jit/tc.h"

#include "hphp/util/atomic-vector.h"
#include "hphp/util/boot-stats.h"
#include "hphp/util/struct-log.h"

namespace HPHP {

TRACE_SET_MOD(typeProfile);

//////////////////////////////////////////////////////////////////////

/*
 * Warmup/profiling.
 *
 * In cli mode, we only record samples if we're in recording to replay later.
 *
 * In server mode, we exclude warmup document requests from profiling, then
 * record samples for EvalJitProfileInterpRequests standard requests.
 */

RequestKind __thread requestKind = RequestKind::Warmup;
bool __thread standardRequest = true;

namespace {

bool warmingUp;
std::atomic<int64_t> numRequests;
std::atomic<bool> singleJitLock;
__thread bool acquiredSingleJit = false;
std::atomic<int> singleJitConcurrentCount;
__thread bool acquiredSingleJitConcurrent = false;
std::atomic<int> singleJitRequests;
std::atomic<int> relocateRequests;
__thread bool nonVMThread = false;

/*
 * RFH, or "requests served in first hour" is used as a performance metric that
 * is affected by warmup speed as well as steady-state performance. For every
 * element n in this list, we log a point along the RFH curve, which is the
 * total number of requests served when server uptime hits n seconds.
 */
const std::vector<int64_t> rfhBuckets = {
  30, 60, 90, 120, 150, 180, 210, 240, 270, 300,             // every 30s, to 5m
  360, 420, 480, 540, 600,                                   // every 1m, to 10m
  900, 1200, 1500, 1800, 2100, 2400, 2700, 3000, 3300, 3600, // every 5m, to 1h
  4500, 5400, 6300, 7200,                                    // every 15m, to 2h
  3 * 3600, 4 * 3600, 5 * 3600, 6 * 3600,                    // every 1h, to 6h
};
std::atomic<size_t> nextRFH{0};

}

ProfileNonVMThread::ProfileNonVMThread() {
  always_assert(!nonVMThread);
  nonVMThread = true;
}

ProfileNonVMThread::~ProfileNonVMThread() {
  nonVMThread = false;
}

void setRelocateRequests(int32_t n) {
  relocateRequests.store(n);
}

namespace {
AtomicVector<uint32_t> s_func_counters{0, 0};
static InitFiniNode s_func_counters_reinit([]{
  UnsafeReinitEmptyAtomicVector(
    s_func_counters, RuntimeOption::EvalFuncCountHint);
}, InitFiniNode::When::PostRuntimeOptions, "s_func_counters reinit");
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
  return numRequests.load(std::memory_order_relaxed);
}

int singleJitRequestCount() {
  return singleJitRequests.load(std::memory_order_relaxed);
}

static inline bool doneProfiling() {
  return requestCount() >= RuntimeOption::EvalJitProfileInterpRequests ||
    (!RuntimeOption::ServerExecutionMode() &&
     !RuntimeOption::EvalJitProfileRecord);
}

static inline RequestKind getRequestKind() {
  if (nonVMThread) return RequestKind::NonVM;
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
    jit::setMayAcquireLease(true);
    jit::setMayAcquireConcurrentLease(true);
    assertx(!acquiredSingleJit);
    assertx(!acquiredSingleJitConcurrent);

    if (singleJitRequests < RuntimeOption::EvalNumSingleJitRequests) {
      if (!singleJitLock.exchange(true, std::memory_order_relaxed)) {
        acquiredSingleJit = true;
      } else {
        jit::setMayAcquireLease(false);
      }

      if (RuntimeOption::EvalJitConcurrently > 0) {
        // The single jit lock is treated separately for translations that
        // happen concurrently: we still give threads permission to jit one
        // request at a time, but we allow up to Eval.JitThreads to have this
        // permission at once.
        auto threads = singleJitConcurrentCount.load(std::memory_order_relaxed);
        if (threads < RuntimeOption::EvalJitThreads &&
            singleJitConcurrentCount.compare_exchange_strong(
              threads, threads + 1, std::memory_order_relaxed)) {
          acquiredSingleJitConcurrent = true;
        } else {
          jit::setMayAcquireConcurrentLease(false);
        }
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
    jit::tc::liveRelocate(true);
  }
}

static void checkRFH(int64_t finished) {
  auto i = nextRFH.load(std::memory_order_relaxed);
  if (i == rfhBuckets.size() || !StructuredLog::enabled()) {
    return;
  }

  auto const uptime = f_server_uptime();
  if (uptime == -1) return;
  assertx(uptime >= 0);

  while (i < rfhBuckets.size() && uptime >= rfhBuckets[i]) {
    assertx(i == 0 || rfhBuckets[i - 1] < rfhBuckets[i]);
    if (!nextRFH.compare_exchange_strong(i, i + 1, std::memory_order_relaxed)) {
      // Someone else reported the sample at i. Try again with the current
      // value of nextRFH.
      continue;
    }

    // "bucket" and "uptime" will always be the same as long as the server
    // retires at least one request in each second of wall time.
    StructuredLogEntry cols;
    cols.setInt("requests", finished);
    cols.setInt("bucket", rfhBuckets[i]);
    cols.setInt("uptime", uptime);
    StructuredLog::log("hhvm_rfh", cols);

    ++i;
  }
}

void profileRequestEnd() {
  if (warmingUp || requestKind == RequestKind::NonVM) return;
  auto const finished = numRequests.fetch_add(1, std::memory_order_relaxed) + 1;
  static auto const requestSeries = ServiceData::createTimeSeries(
    "vm.requests",
    {ServiceData::StatsType::RATE, ServiceData::StatsType::SUM},
    {std::chrono::seconds(60), std::chrono::seconds(0)}
  );
  requestSeries->addValue(1);
  checkRFH(finished);

  if (acquiredSingleJit || acquiredSingleJitConcurrent) {
    ++singleJitRequests;

    if (acquiredSingleJit) {
      singleJitLock = false;
      acquiredSingleJit = false;
    }
    if (acquiredSingleJitConcurrent) {
      --singleJitConcurrentCount;
      acquiredSingleJitConcurrent = false;
    }

    if (RuntimeOption::ServerExecutionMode()) {
      Logger::Warning("Finished singleJitRequest %d", singleJitRequests.load());
    }
  }
}

}
