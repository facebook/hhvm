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

#include <atomic>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <signal.h>
#include <limits>
#include <map>
#include <set>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "folly/String.h"

using std::map;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex s_thread_info_mutex;
static std::set<ThreadInfo*> s_thread_infos;

__thread char* ThreadInfo::t_stackbase = 0;

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo()
    : m_stacklimit(0), m_executing(Idling) {
  assert(!t_stackbase);
  t_stackbase = static_cast<char*>(stack_top_ptr());

  m_mm = &MM();

  m_profiler = nullptr;
  m_pendingException = nullptr;
  m_coverage = new CodeCoverage();

  RDS::threadInit();
  onSessionInit();

  Lock lock(s_thread_info_mutex);
  s_thread_infos.insert(this);
}

ThreadInfo::~ThreadInfo() {
  t_stackbase = 0;

  Lock lock(s_thread_info_mutex);
  s_thread_infos.erase(this);
  delete m_coverage;
  RDS::threadExit();
}

bool ThreadInfo::valid(ThreadInfo* info) {
  Lock lock(s_thread_info_mutex);
  return s_thread_infos.find(info) != s_thread_infos.end();
}

void ThreadInfo::GetExecutionSamples(std::map<Executing, int> &counts) {
  Lock lock(s_thread_info_mutex);
  for (std::set<ThreadInfo*>::const_iterator iter = s_thread_infos.begin();
       iter != s_thread_infos.end(); ++iter) {
    ++counts[(*iter)->m_executing];
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
  m_reqInjectionData.clearPendingExceptionFlag();
  if (m_pendingException != nullptr) delete m_pendingException;
  m_pendingException = nullptr;
}

void ThreadInfo::setPendingException(Exception* e) {
  m_reqInjectionData.setPendingExceptionFlag();
  if (m_pendingException != nullptr) delete m_pendingException;
  m_pendingException = e;
}

void ThreadInfo::onSessionExit() {
  // Clear any timeout handlers to they don't fire when the request has already
  // been destroyed
  m_reqInjectionData.setTimeout(0);

  m_reqInjectionData.reset();
  RDS::requestExit();
}

//////////////////////////////////////////////////////////////////////

void throw_infinite_recursion_exception() {
  if (!RuntimeOption::NoInfiniteRecursionDetection) {
    // Reset profiler otherwise it might recurse further causing segfault
    DECLARE_THREAD_INFO
    info->m_profiler = nullptr;
    throw UncatchableException("infinite recursion detected");
  }
}

ssize_t check_request_surprise(ThreadInfo* info) {
  auto& p = info->m_reqInjectionData;
  bool do_timedout, do_memExceeded, do_signaled;

  ssize_t flags = p.fetchAndClearFlags();
  do_timedout = (flags & RequestInjectionData::TimedOutFlag) &&
    !p.getDebugger();
  do_memExceeded = (flags & RequestInjectionData::MemExceededFlag);
  do_signaled = (flags & RequestInjectionData::SignaledFlag);

  // Start with any pending exception that might be on the thread.
  Exception* pendingException = info->m_pendingException;
  info->m_pendingException = nullptr;

  if (do_timedout && !pendingException) {
    pendingException = generate_request_timeout_exception();
  }
  if (do_memExceeded && !pendingException) {
    pendingException = generate_memory_exceeded_exception();
  }
  if (do_signaled) {
    extern bool f_pcntl_signal_dispatch();
    f_pcntl_signal_dispatch();
  }

  if (pendingException) {
    pendingException->throwException();
  }
  return flags;
}

//////////////////////////////////////////////////////////////////////

}
