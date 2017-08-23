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

#include "hphp/runtime/base/thread-info.h"

#include <map>
#include <set>

#include <folly/Format.h>

#include "hphp/util/alloc.h"
#include "hphp/util/lock.h"
#include "hphp/util/perf-event.h"

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/perf-mem-event.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/ext/process/ext_process.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
///////////////////////////////////////////////////////////////////////////////

/*
 * Set of all ThreadInfos for the running process.
 */
std::set<ThreadInfo*> s_thread_infos;
Mutex s_thread_info_mutex;

/*
 * Either null, or populated by initialization of ThreadInfo as an approximation
 * of the highest address of the current thread's stack.
 */
__thread char* t_stackbase = nullptr;

///////////////////////////////////////////////////////////////////////////////
}

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo() {
  assert(!t_stackbase);
  t_stackbase = static_cast<char*>(stack_top_ptr());

  m_coverage = new CodeCoverage();
}

ThreadInfo::~ThreadInfo() {
  assert(t_stackbase);
  t_stackbase = nullptr;

  Lock lock(s_thread_info_mutex);
  s_thread_infos.erase(this);
  delete m_coverage;
  rds::threadExit();
}

void ThreadInfo::init() {
  m_reqInjectionData.threadInit();
  rds::threadInit();
  onSessionInit();
  // TODO(20427335): Get rid of the illogical onSessionInit() call above.
  Lock lock(s_thread_info_mutex);
  s_thread_infos.insert(this);
}

bool ThreadInfo::valid(ThreadInfo* info) {
  Lock lock(s_thread_info_mutex);
  return s_thread_infos.find(info) != s_thread_infos.end();
}

void ThreadInfo::GetExecutionSamples(std::map<Executing, int>& counts) {
  Lock lock(s_thread_info_mutex);
  for (auto const info : s_thread_infos) {
    ++counts[info->m_executing];
  }
}

void ThreadInfo::ExecutePerThread(std::function<void(ThreadInfo*)> f) {
  Lock lock(s_thread_info_mutex);
  for (auto thread : s_thread_infos) {
    f(thread);
  }
}

int ThreadInfo::SetPendingGCForAllOnRequestThread() {
  int cnt = 0;
  ExecutePerThread( [&cnt](ThreadInfo* t) {
    if ( t->changeGlobalGCStatus(OnRequestWithNoPendingExecution,
                                 OnRequestWithPendingExecution)) {
      t->m_reqInjectionData.setFlag(PendingGCFlag);
      cnt++;
    }
  } );
  return cnt;
}

void ThreadInfo::onSessionInit() {
  m_reqInjectionData.onSessionInit();
}

bool ThreadInfo::changeGlobalGCStatus(GlobalGCStatus from, GlobalGCStatus to) {
  return m_globalGCStatus.compare_exchange_strong(from, to);
}

void ThreadInfo::setPendingException(Exception* e) {
  m_reqInjectionData.setFlag(PendingExceptionFlag);

  auto tmp = m_pendingException;
  m_pendingException = e;
  delete tmp;
}

void ThreadInfo::onSessionExit() {
  // Clear any timeout handlers to they don't fire when the request has already
  // been destroyed.
  m_reqInjectionData.setTimeout(0);
  m_reqInjectionData.setCPUTimeout(0);

  m_reqInjectionData.reset();

  if (auto tmp = m_pendingException) {
    m_pendingException = nullptr;
    // request memory has already been freed
    if (auto ee = dynamic_cast<ExtendedException*>(tmp)) {
      ee->leakBacktrace();
    }
    delete tmp;
  }

  rds::requestExit();
}

//////////////////////////////////////////////////////////////////////

void raise_infinite_recursion_error() {
  if (!RuntimeOption::NoInfiniteRecursionDetection) {
    // Reset profiler otherwise it might recurse further causing segfault.
    TI().m_profiler = nullptr;
    raise_error("infinite recursion detected");
  }
}

static Exception* generate_request_timeout_exception(c_WaitableWaitHandle* wh) {
  auto exceptionMsg = folly::sformat(
    !RuntimeOption::ServerExecutionMode() || is_cli_mode()
      ? "Maximum execution time of {} seconds exceeded"
      : "entire web request took longer than {} seconds and timed out",
    RID().getTimeout());
  auto exceptionStack = createBacktrace(BacktraceArgs()
                                        .fromWaitHandle(wh)
                                        .withSelf()
                                        .withThis()
                                        .withMetadata());
  return new RequestTimeoutException(exceptionMsg, exceptionStack);
}

static Exception* generate_request_cpu_timeout_exception(
  c_WaitableWaitHandle* wh
) {
  auto exceptionMsg = folly::sformat(
    "Maximum CPU time of {} seconds exceeded",
    RID().getCPUTimeout()
  );

  auto exceptionStack = createBacktrace(BacktraceArgs()
                                        .fromWaitHandle(wh)
                                        .withSelf()
                                        .withThis()
                                        .withMetadata());
  return new RequestCPUTimeoutException(exceptionMsg, exceptionStack);
}

static Exception* generate_memory_exceeded_exception(c_WaitableWaitHandle* wh) {
  auto exceptionStack = createBacktrace(BacktraceArgs()
                                        .fromWaitHandle(wh)
                                        .withSelf()
                                        .withThis()
                                        .withMetadata());
  return new RequestMemoryExceededException(
    "request has exceeded memory limit", exceptionStack);
}

// suppress certain callbacks when we're running a user error handler;
// to reduce the chances that a subsequent error occurs in the callback
// and obscures the effect that the first handler would have had.
static bool callbacksOk() {
  switch (g_context->getErrorState()) {
    case ExecutionContext::ErrorState::NoError:
    case ExecutionContext::ErrorState::ErrorRaised:
    case ExecutionContext::ErrorState::ErrorRaisedByUserHandler:
      return true;
    case ExecutionContext::ErrorState::ExecutingUserHandler:
      return false;
  }
  not_reached();
}

size_t handle_request_surprise(c_WaitableWaitHandle* wh, size_t mask) {
  NoHandleSurpriseScope::AssertNone(static_cast<SurpriseFlag>(mask));
  auto& info = TI();
  auto& p = info.m_reqInjectionData;

  auto flags = fetchAndClearSurpriseFlags() & mask;
  auto const debugging = p.getDebuggerAttached();

  // Start with any pending exception that might be on the thread.
  auto pendingException = info.m_pendingException;
  info.m_pendingException = nullptr;

  if (auto cbFlags =
      flags & (XenonSignalFlag | MemThresholdFlag | IntervalTimerFlag)) {
    if (!callbacksOk()) {
      setSurpriseFlag(static_cast<SurpriseFlag>(cbFlags));
      flags -= cbFlags;
    }
  }

  if ((flags & TimedOutFlag) && !debugging) {
    p.setCPUTimeout(0);  // Stop CPU timer so we won't time out twice.
    if (pendingException) {
      setSurpriseFlag(TimedOutFlag);
    } else {
      pendingException = generate_request_timeout_exception(wh);
    }
  } else if ((flags & CPUTimedOutFlag) && !debugging) {
    // Don't bother with the CPU timeout if we're already handling a wall
    // timeout.
    p.setTimeout(0);  // Stop wall timer so we won't time out twice.
    if (pendingException) {
      setSurpriseFlag(CPUTimedOutFlag);
    } else {
      pendingException = generate_request_cpu_timeout_exception(wh);
    }
  }
  if (flags & MemExceededFlag) {
    if (pendingException) {
      setSurpriseFlag(MemExceededFlag);
    } else {
      pendingException = generate_memory_exceeded_exception(wh);
    }
  }
  if (flags & PendingGCFlag) {
    if (StickyFlags & PendingGCFlag) {
      clearSurpriseFlag(PendingGCFlag);
    }
    if (MM().isGCEnabled()) {
      MM().collect("surprise");
    } else {
      MM().checkHeap("surprise");
    }
  }
  if (flags & SignaledFlag) {
    HHVM_FN(pcntl_signal_dispatch)();
  }

  if (flags & PendingPerfEventFlag) {
    if (StickyFlags & PendingPerfEventFlag) {
      clearSurpriseFlag(PendingPerfEventFlag);
    }
    perf_event_consume(record_perf_mem_event);
  }

  if (pendingException) {
    pendingException->throwException();
  }
  return flags;
}

}
