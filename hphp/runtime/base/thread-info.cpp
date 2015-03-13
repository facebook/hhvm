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

#include "hphp/runtime/base/thread-info.h"

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <signal.h>
#include <limits>
#include <map>
#include <set>

#include <folly/Format.h>

#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex s_thread_info_mutex;
static std::set<ThreadInfo*> s_thread_infos;

__thread char* ThreadInfo::t_stackbase = 0;

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo() {
  assert(!t_stackbase);
  t_stackbase = static_cast<char*>(stack_top_ptr());

  m_mm = &MM();
  m_coverage = new CodeCoverage();
}

ThreadInfo::~ThreadInfo() {
  t_stackbase = 0;

  Lock lock(s_thread_info_mutex);
  s_thread_infos.erase(this);
  delete m_coverage;
  rds::threadExit();
}

void ThreadInfo::init() {
  m_reqInjectionData.threadInit();
  rds::threadInit();
  onSessionInit();
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

void ThreadInfo::onSessionInit() {
  m_reqInjectionData.onSessionInit();

  // Take the address of the cached per-thread stackLimit, and use this to allow
  // some slack for (a) stack usage above the caller of reset() and (b) stack
  // usage after the position gets checked.
  // If we're not in a threaded environment, then s_stackSize will be
  // zero. Use getrlimit to figure out what the size of the stack is to
  // calculate an approximation of where the bottom of the stack should be.
  if (s_stackSize == 0) {
    struct rlimit rl;

    getrlimit(RLIMIT_STACK, &rl);
    m_stacklimit = t_stackbase - (rl.rlim_cur - StackSlack);
  } else {
    m_stacklimit = (char *)s_stackLimit + StackSlack;
    assert(uintptr_t(m_stacklimit) < s_stackLimit + s_stackSize);
  }
}

void ThreadInfo::clearPendingException() {
  m_reqInjectionData.clearFlag(PendingExceptionFlag);

  if (m_pendingException != nullptr) delete m_pendingException;
  m_pendingException = nullptr;
}

void ThreadInfo::setPendingException(Exception* e) {
  m_reqInjectionData.setFlag(PendingExceptionFlag);
  if (m_pendingException != nullptr) delete m_pendingException;
  m_pendingException = e;
}

void ThreadInfo::onSessionExit() {
  // Clear any timeout handlers to they don't fire when the request has already
  // been destroyed
  m_reqInjectionData.setTimeout(0);
  m_reqInjectionData.setCPUTimeout(0);

  m_reqInjectionData.reset();
  rds::requestExit();
}

//////////////////////////////////////////////////////////////////////

void raise_infinite_recursion_error() {
  if (!RuntimeOption::NoInfiniteRecursionDetection) {
    // Reset profiler otherwise it might recurse further causing segfault
    auto info = ThreadInfo::s_threadInfo.getNoCheck();
    info->m_profiler = nullptr;
    raise_error("infinite recursion detected");
  }
}

static Exception* generate_request_timeout_exception() {
  auto& data = ThreadInfo::s_threadInfo->m_reqInjectionData;

  auto exceptionMsg = folly::sformat(
    RuntimeOption::ClientExecutionMode()
      ? "Maximum execution time of {} seconds exceeded"
      : "entire web request took longer than {} seconds and timed out",
    data.getTimeout());
  auto exceptionStack = createBacktrace(BacktraceArgs()
                                        .withSelf()
                                        .withThis());
  return new RequestTimeoutException(exceptionMsg, exceptionStack);
}

static Exception* generate_request_cpu_timeout_exception() {
  auto& data = ThreadInfo::s_threadInfo->m_reqInjectionData;

  auto exceptionMsg =
    folly::sformat("Maximum CPU time of {} seconds exceeded",
                   data.getCPUTimeout());
  auto exceptionStack = createBacktrace(BacktraceArgs()
                                        .withSelf()
                                        .withThis());
  return new RequestCPUTimeoutException(exceptionMsg, exceptionStack);
}

static Exception* generate_memory_exceeded_exception() {
  auto exceptionStack = createBacktrace(BacktraceArgs()
                                        .withSelf()
                                        .withThis());
  return new RequestMemoryExceededException(
    "request has exceeded memory limit", exceptionStack);
}

ssize_t check_request_surprise() {
  auto info = ThreadInfo::s_threadInfo;
  auto& p = info->m_reqInjectionData;

  auto const flags = fetchAndClearSurpriseFlags();
  auto const do_timedout = (flags & TimedOutFlag) && !p.getDebuggerAttached();
  auto const do_memExceeded = flags & MemExceededFlag;
  auto const do_signaled = flags & SignaledFlag;
  auto const do_cpuTimedOut =
    (flags & CPUTimedOutFlag) && !p.getDebuggerAttached();

  // Start with any pending exception that might be on the thread.
  auto pendingException = info->m_pendingException;
  info->m_pendingException = nullptr;

  if (do_timedout) {
    p.setCPUTimeout(0);  // Stop CPU timer so we won't time out twice.
    if (pendingException) {
      setSurpriseFlag(TimedOutFlag);
    } else {
      pendingException = generate_request_timeout_exception();
    }
  }
  // Don't bother with the CPU timeout if we're already handling a wall timeout.
  if (do_cpuTimedOut && !do_timedout) {
    p.setTimeout(0);  // Stop wall timer so we won't time out twice.
    if (pendingException) {
      setSurpriseFlag(CPUTimedOutFlag);
    } else {
      pendingException = generate_request_cpu_timeout_exception();
    }
  }
  if (do_memExceeded) {
    if (pendingException) {
      setSurpriseFlag(MemExceededFlag);
    } else {
      pendingException = generate_memory_exceeded_exception();
    }
  }
  if (do_signaled) {
    extern bool HHVM_FN(pcntl_signal_dispatch)();
    HHVM_FN(pcntl_signal_dispatch)();
  }

  if (pendingException) {
    pendingException->throwException();
  }
  return flags;
}

ssize_t check_request_surprise_unlikely() {
  auto const flags = surpriseFlags().load();

  if (UNLIKELY(flags)) {
    check_request_surprise();
  }
  return flags;
}

//////////////////////////////////////////////////////////////////////

}
