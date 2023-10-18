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

#include "hphp/runtime/vm/treadmill.h"

#include <list>
#include <atomic>
#include <vector>
#include <memory>
#include <algorithm>

#include <pthread.h>
#include <stdio.h>
#include <signal.h>

#include <folly/portability/SysTime.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/trace.h"
#include "hphp/util/rank.h"
#include "hphp/util/service-data.h"

namespace HPHP {  namespace Treadmill {

TRACE_SET_MOD(treadmill);

/*
 * We assign local, unique indexes to each thread, with hopes that
 * they are densely packed.
 *
 * The plan here is that each thread starts with s_thisThreadIdx as
 * kInvalidRequestIdx.  And the first time a thread starts using the Treadmill
 * it allocates a new thread id from s_nextThreadIdx with fetch_add.
 */
std::atomic<int64_t> g_nextThreadIdx{0};
RDS_LOCAL_NO_CHECK(int64_t, rl_thisRequestIdx){kInvalidRequestIdx};

namespace {

//////////////////////////////////////////////////////////////////////

const int64_t ONE_SEC_IN_MICROSEC = 1000000;

struct TreadmillRequestInfo {
  GenCount  startTime;
  pthread_t pthreadId;
  RequestInfo* requestInfo;
  SessionKind sessionKind;
};

pthread_mutex_t s_genLock = PTHREAD_MUTEX_INITIALIZER;
std::vector<TreadmillRequestInfo> s_inflightRequests;
GenCount s_latestCount = 0;
std::atomic<GenCount> s_oldestRequestInFlight(0);

//////////////////////////////////////////////////////////////////////

/*
 * The next 2 functions should be used to manage the generation count/time
 * in the treadmill for both the requests and the work items.
 * The pattern is to call getTime() outside of the lock and correctTime()
 * while holding the lock.
 * That pattern guarantees a monotonically increasing counter.
 * The resolution being microseconds should give us all the room we need
 * to accommodate requests and work items at any conceivable rate and
 * correctTime() should give us correct behavior at any granularity of
 * gettimeofday().
 */

/*
 * Return the current time in microseconds.
 * Usually called outside of the lock.
 */
GenCount getTime() {
  struct timeval time;
  gettimeofday(&time, nullptr);
  return time.tv_sec * ONE_SEC_IN_MICROSEC + time.tv_usec;
}

/*
 * Return a monotonically increasing time given the last time recorded.
 * This must be called while holding the lock.
 */
GenCount correctTime(GenCount time) {
  s_latestCount = time <= s_latestCount ? s_latestCount + 1 : time;
  return s_latestCount;
}

struct GenCountGuard {
  GenCountGuard() {
    checkRank(RankTreadmill);
    pthread_mutex_lock(&s_genLock);
    pushRank(RankTreadmill);
  }

  struct Locked {};
  GenCountGuard(Locked) { pushRank(RankTreadmill); }

  ~GenCountGuard() {
    popRank(RankTreadmill);
    pthread_mutex_unlock(&s_genLock);
  }
};

//////////////////////////////////////////////////////////////////////

char const* getSessionKindName(SessionKind value) {
  switch(value) {
    case SessionKind::None: return "None";
    case SessionKind::DebuggerClient: return "DebuggerClient";
    case SessionKind::PreloadRepo: return "PreloadRepo";
    case SessionKind::Watchman: return "Watchman";
    case SessionKind::Vsdebug: return "VSDebug";
    case SessionKind::FactsWorker: return "FactsWorker";
    case SessionKind::CLIServer: return "CLIServer";
    case SessionKind::AdminPort: return "AdminRequest";
    case SessionKind::HttpRequest: return "HttpRequest";
    case SessionKind::RpcRequest: return "RpcRequest";
    case SessionKind::TranslateWorker: return "TranslateWorker";
    case SessionKind::Retranslate: return "Retranslate";
    case SessionKind::RetranslateAll: return "RetranslateAll";
    case SessionKind::ProfData: return "ProfData";
    case SessionKind::UnitTests: return "UnitTests";
    case SessionKind::CompileRepo: return "CompileRepo";
    case SessionKind::HHBBC: return "HHBBC";
    case SessionKind::CompilerEmit: return "CompilerEmit";
    case SessionKind::CompilerAnalysis: return "CompilerAnalysis";
    case SessionKind::CLISession: return "CLISession";
    case SessionKind::UnitReaper: return "UnitReaper";
  }
  return "";
}

//////////////////////////////////////////////////////////////////////

/*
 * Get the ID of the thread to abort in case the treadmill gets stuck for too
 * long.  In general, this is the oldest thread, but special treatment is used
 * for retranslate-all.  When the main retranslate-all thread gets stuck, it's
 * often because it's waiting for one of the JIT worker threads, which do the
 * bulk of the JITing.  In such cases, we want to abort the oldest JIT worker,
 * to capture its backtrace, instead of the main retranslate-all thread.
 */
TreadmillRequestInfo getRequestToAbort() {
  int64_t oldestStart = s_oldestRequestInFlight.load(std::memory_order_relaxed);
  const TreadmillRequestInfo* oldest = nullptr;
  const TreadmillRequestInfo* oldestWorker = nullptr;
  for (auto const& req : s_inflightRequests) {
    if (req.startTime == oldestStart) oldest = &req;
    if (req.sessionKind == SessionKind::TranslateWorker &&
        (oldestWorker == nullptr || req.startTime < oldestWorker->startTime)) {
      oldestWorker = &req;
    }
  }
  always_assert(oldest != nullptr);
  if (oldest->sessionKind != SessionKind::RetranslateAll) {
    return *oldest;
  }
  if (oldestWorker != nullptr) {
    return *oldestWorker;
  }
  return *oldest;
}

void checkOldest(Optional<GenCountGuard>& guard) {
  int64_t limit =
    RuntimeOption::MaxRequestAgeFactor * RuntimeOption::RequestTimeoutSeconds;
  if (debug || !limit) return;

  auto const ageOldest = getAgeOldestRequest();
  if (ageOldest <= limit) return;

  auto const request = getRequestToAbort();
  auto const msg = folly::sformat(
    "Oldest request ({}, {}, {}) has been running for {} "
    "seconds. Aborting the server.",
    request.requestInfo ? request.requestInfo->m_id.toString() : "none",
    request.startTime,
    getSessionKindName(request.sessionKind),
    ageOldest
  );
  Logger::Error(msg);
  // Drop the lock since the SIGABRT we're about to send will try to
  // acquire it.
  guard.reset();
  pthread_kill(request.pthreadId, SIGABRT);
  // We're going to die, wait for it and don't bother proceeding.
  ::pause();
  always_assert(false);
}

void refreshStats() {
  static ServiceData::ExportedCounter* s_oldestRequestAgeStat =
    ServiceData::createCounter("treadmill.age");
  s_oldestRequestAgeStat->setValue(getAgeOldestRequest());
}

}

struct PendingTriggers : std::list<std::unique_ptr<WorkItem>> {
  ~PendingTriggers() {
    s_destroyed = true;
  }
  static bool s_destroyed;
};

static PendingTriggers s_tq;
bool PendingTriggers::s_destroyed = false;
void enqueueInternal(std::unique_ptr<WorkItem> gt) {
  if (PendingTriggers::s_destroyed) {
    return;
  }
  GenCount time = getTime();
  {
    GenCountGuard g;
    gt->m_gen = correctTime(time);
    s_tq.emplace_back(std::move(gt));
  }
}

void startRequest(SessionKind session_kind) {
  auto const requestIdx = Treadmill::requestIdx();

  GenCount startTime = getTime();
  {
    Optional<GenCountGuard> g;
    g.emplace();

    refreshStats();
    checkOldest(g);
    if (requestIdx >= s_inflightRequests.size()) {
      s_inflightRequests.resize(
        requestIdx + 1, {kIdleGenCount, 0, 0, SessionKind::None});
    } else {
      assertx(s_inflightRequests[requestIdx].startTime == kIdleGenCount);
    }
    s_inflightRequests[requestIdx].startTime = correctTime(startTime);
    s_inflightRequests[requestIdx].pthreadId = Process::GetThreadId();
    s_inflightRequests[requestIdx].requestInfo =
      RequestInfo::s_requestInfo.isNull() ? nullptr : &RI();
    s_inflightRequests[requestIdx].sessionKind = session_kind;
    FTRACE(1, "requestIdx {} pthreadId {} start @gen {}\n", requestIdx,
           s_inflightRequests[requestIdx].pthreadId,
           s_inflightRequests[requestIdx].startTime);
    if (s_oldestRequestInFlight.load(std::memory_order_relaxed) == 0) {
      s_oldestRequestInFlight = s_inflightRequests[requestIdx].startTime;
    }
    if (!RequestInfo::s_requestInfo.isNull()) {
      RI().changeGlobalGCStatus(RequestInfo::Idle,
                                RequestInfo::OnRequestWithNoPendingExecution);
    }
  }
}

void finishRequest() {
  auto const requestIdx = Treadmill::requestIdx();
  assertx(requestIdx != -1);
  FTRACE(1, "tid {} finish\n", requestIdx);
  std::vector<std::unique_ptr<WorkItem>> toFire;
  {
    GenCountGuard g;
    assertx(s_inflightRequests[requestIdx].startTime != kIdleGenCount);
    GenCount finishedRequest = s_inflightRequests[requestIdx].startTime;
    s_inflightRequests[requestIdx].startTime = kIdleGenCount;

    // After finishing a request, check to see if we've allowed any triggers
    // to fire and update the time of the oldest request in flight.
    // However if the request just finished is not the current oldest we
    // don't need to check anything as there cannot be any WorkItem to run.
    if (s_oldestRequestInFlight.load(std::memory_order_relaxed) ==
        finishedRequest) {
      GenCount limit = s_latestCount + 1;
      for (auto& val : s_inflightRequests) {
        if (val.startTime != kIdleGenCount && val.startTime < limit) {
          limit = val.startTime;
        }
      }
      // update "oldest in flight" or kill it if there are no running requests
      s_oldestRequestInFlight = limit == s_latestCount + 1 ? 0 : limit;

      // collect WorkItem to run
      auto it = s_tq.begin();
      auto end = s_tq.end();
      while (it != end) {
        TRACE(2, "considering delendum %d\n", int((*it)->m_gen));
        if ((*it)->m_gen >= limit) {
          TRACE(2, "not unreachable! %d\n", int((*it)->m_gen));
          break;
        }
        toFire.emplace_back(std::move(*it));
        it = s_tq.erase(it);
      }
    }
    constexpr int limit = 100;
    if (!RequestInfo::s_requestInfo.isNull()) {
      // If somehow we excessed the limit, GlobalGCTrigger will stay on
      // "Triggering" stage forever. No more global GC can be triggered.
      // But it should have no effect on APC GC -- The data will be freed
      // by treadmill's calling
      int i;
      for (i = 0; i < limit; ++i) {
        if (RI().changeGlobalGCStatus(
              RequestInfo::OnRequestWithPendingExecution,
              RequestInfo::Idle)) {
          // Call globalGCTrigger to Run the pending execution
          // TODO(20074509)
          FTRACE(2, "treadmill executes pending global GC callbacks\n");
          break;
        }
        if (RI().changeGlobalGCStatus(
              RequestInfo::OnRequestWithNoPendingExecution,
              RequestInfo::Idle)) {
          break;
        }
      }
      assertx(i < limit);
      if (i == limit) {
        Logger::Warning("Treadmill fails to set global GC status into Idle");
      }
    }
  }
  for (unsigned i = 0; i < toFire.size(); ++i) {
    toFire[i]->run();
  }
}

int64_t getOldestRequestGenCount() {
  return s_oldestRequestInFlight.load(std::memory_order_relaxed);
}

int64_t getAgeOldestRequest() {
  int64_t start = s_oldestRequestInFlight.load(std::memory_order_relaxed);
  if (start == 0) return 0; // no request in flight
  int64_t time = getTime() - start;
  return time / ONE_SEC_IN_MICROSEC;
}

int64_t getRequestGenCount() {
  auto const requestIdx = Treadmill::requestIdx();
  assertx(requestIdx != -1);
  return s_inflightRequests[requestIdx].startTime;
}

SessionKind sessionKind() {
  if (*rl_thisRequestIdx == kInvalidRequestIdx) return SessionKind::None;
  return s_inflightRequests[*rl_thisRequestIdx].sessionKind;
}

void deferredFree(void* p) {
  enqueue([p] { free(p); });
}

std::string dumpTreadmillInfo(bool forCrash) {
  std::string out;
  Optional<GenCountGuard> g;

  if (!forCrash) {
    g.emplace();
  } else {
    // Make an attempt to grab the lock even if we're dumping for a
    // crash. Only wait one second and then give up. If we don't grab
    // the lock, then proceed anyways and hope for the best.
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    ++timeout.tv_sec;
    if (pthread_mutex_timedlock(&s_genLock, &timeout) != 0) {
      out += "Attempting to dump treadmill without acquiring GenCountGuard\n";
    } else {
      g.emplace(GenCountGuard::Locked{});
    }
  }

  int64_t oldestStart =
    s_oldestRequestInFlight.load(std::memory_order_relaxed);

  folly::format(
      &out,
      "OldestStartTime: {}\n",
      oldestStart
  );

  folly::format(
      &out,
      "InflightRequestsSize: {}\n",
      s_inflightRequests.size()
  );

  for (auto& req : s_inflightRequests) {
    if (req.startTime != kIdleGenCount) {
      folly::format(
          &out,
          "{} {} {} {}{}\n",
          req.pthreadId,
          req.requestInfo ? req.requestInfo->m_id.toString() : "none",
          req.startTime,
          getSessionKindName(req.sessionKind),
          req.startTime == oldestStart ? " OLDEST" : ""
      );
    }
  }
  return out;
}

}}
