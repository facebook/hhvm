/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/hphp_system.h"
#include "hphp/runtime/base/code_coverage.h"
#include "hphp/runtime/base/smart_allocator.h"
#include "hphp/runtime/vm/jit/target-cache.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"

#include <sys/mman.h>

using std::map;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex s_thread_info_mutex;
static std::set<ThreadInfo*> s_thread_infos;

__thread char* ThreadInfo::t_stackbase = 0;

IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo()
    : m_stacklimit(0), m_executing(Idling) {
  assert(!t_stackbase);
  t_stackbase = static_cast<char*>(stack_top_ptr());

  map<int, ObjectAllocatorBaseGetter> &wrappers =
    ObjectAllocatorCollector::getWrappers();
  m_allocators.resize(wrappers.rbegin()->first + 1);
  for (map<int, ObjectAllocatorBaseGetter>::iterator it = wrappers.begin();
       it != wrappers.end(); it++) {
    m_allocators[it->first] = it->second();
    assert(it->second() != nullptr);
  }

  m_mm = MemoryManager::TheMemoryManager();

  m_profiler = nullptr;
  m_pendingException = nullptr;
  m_coverage = new CodeCoverage();

  Transl::TargetCache::threadInit();
  onSessionInit();

  Lock lock(s_thread_info_mutex);
  s_thread_infos.insert(this);
}

ThreadInfo::~ThreadInfo() {
  t_stackbase = 0;

  Lock lock(s_thread_info_mutex);
  s_thread_infos.erase(this);
  delete m_coverage;
  Transl::TargetCache::threadExit();
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
  // If we're not in a threaded environment, then Util::s_stackSize will be
  // zero. Use getrlimit to figure out what the size of the stack is to
  // calculate an approximation of where the bottom of the stack should be.
  if (Util::s_stackSize == 0) {
    struct rlimit rl;

    getrlimit(RLIMIT_STACK, &rl);
    m_stacklimit = t_stackbase - (rl.rlim_cur - StackSlack);
  } else {
    m_stacklimit = (char *)Util::s_stackLimit + StackSlack;
    assert(uintptr_t(m_stacklimit) < (Util::s_stackLimit + Util::s_stackSize));
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
  m_reqInjectionData.setTimeout(0);
  m_reqInjectionData.reset();
  Transl::TargetCache::requestExit();
}

RequestInjectionData::~RequestInjectionData() {
#ifndef __APPLE__
  if (m_hasTimer) {
    timer_delete(m_timer_id);
  }
#endif
}

void RequestInjectionData::onSessionInit() {
  Transl::TargetCache::requestInit();
  cflagsPtr = Transl::TargetCache::conditionFlagsPtr();
  reset();
}

void RequestInjectionData::onTimeout() {
  setTimedOutFlag();
  if (surprisePage) {
    mprotect(surprisePage, sizeof(void*), PROT_NONE);
  }
  m_timerActive.store(false, std::memory_order_relaxed);
}

void RequestInjectionData::setSurprisePage(void* page) {
  if (page != surprisePage) {
    if (!page) {
      if (m_timerActive.load(std::memory_order_relaxed)) {
        setTimeout(0);
      }
    }
    assert(!m_timerActive.load(std::memory_order_relaxed));
    surprisePage = page;
  }
}

void RequestInjectionData::setTimeout(int seconds) {
#ifndef __APPLE__
  m_timeoutSeconds = seconds > 0 ? seconds : 0;
  if (!m_hasTimer) {
    if (!m_timeoutSeconds) {
      // we don't have a timer, and we don't have a timeout
      return;
    }
    sigevent sev;
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGVTALRM;
    sev.sigev_value.sival_ptr = this;
    if (timer_create(CLOCK_REALTIME, &sev, &m_timer_id)) {
      raise_error("Failed to set timeout: %s", strerror(errno));
    }
    m_hasTimer = true;
  }

  /*
   * There is a potential race here. Callers want to assume that
   * if they cancel the timeout (seconds = 0), they *wont* get
   * a signal after they call this (although they may get a signal
   * during the call).
   * So we need to clear the timeout, wait (if necessary) for a
   * pending signal to be handled, and then set the new timeout
   */
  itimerspec ts = {};
  itimerspec old;
  timer_settime(m_timer_id, 0, &ts, &old);
  if (!old.it_value.tv_sec && !old.it_value.tv_nsec) {
    // the timer has gone off...
    if (m_timerActive.load(std::memory_order_acquire)) {
      // but m_timerActive is still set, so we haven't processed
      // the signal yet.
      // spin until its done.
      while (m_timerActive.load(std::memory_order_relaxed)) {
      }
    }
  }
  if (m_timeoutSeconds) {
    m_timerActive.store(true, std::memory_order_relaxed);
    ts.it_value.tv_sec = m_timeoutSeconds;
    timer_settime(m_timer_id, 0, &ts, nullptr);
  } else {
    m_timerActive.store(false, std::memory_order_relaxed);
  }
#endif
}

int RequestInjectionData::getRemainingTime() const {
#ifndef __APPLE__
  if (m_hasTimer) {
    itimerspec ts;
    if (!timer_gettime(m_timer_id, &ts)) {
      int remaining = ts.it_value.tv_sec;
      return remaining > 1 ? remaining : 1;
    }
  }
#endif
  return m_timeoutSeconds;
}

void RequestInjectionData::resetTimer(int seconds /* = -1 */) {
  auto data = &ThreadInfo::s_threadInfo->m_reqInjectionData;
  if (seconds <= 0) seconds = data->getTimeout();
  data->setTimeout(seconds);
  data->clearTimedOutFlag();
}

void RequestInjectionData::reset() {
  __sync_fetch_and_and(getConditionFlags(), 0);
  m_coverage = RuntimeOption::RecordCodeCoverage;
  m_debugger = false;
  m_debuggerIntr = false;
  updateJit();
  while (!interrupts.empty()) interrupts.pop();
}

void RequestInjectionData::updateJit() {
  m_jit = RuntimeOption::EvalJit &&
    !(RuntimeOption::EvalJitDisabledByHphpd && m_debugger) &&
    !m_debuggerIntr &&
    !m_coverage &&
    !shouldProfile();
}

void RequestInjectionData::setMemExceededFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::MemExceededFlag);
}

void RequestInjectionData::setTimedOutFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::TimedOutFlag);
}

void RequestInjectionData::clearTimedOutFlag() {
  __sync_fetch_and_and(getConditionFlags(),
                       ~RequestInjectionData::TimedOutFlag);
}

void RequestInjectionData::setSignaledFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::SignaledFlag);
}

void RequestInjectionData::setEventHookFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::EventHookFlag);
}

void RequestInjectionData::clearEventHookFlag() {
  __sync_fetch_and_and(getConditionFlags(),
                       ~RequestInjectionData::EventHookFlag);
}

void RequestInjectionData::setPendingExceptionFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::PendingExceptionFlag);
}

void RequestInjectionData::clearPendingExceptionFlag() {
  __sync_fetch_and_and(getConditionFlags(),
                       ~RequestInjectionData::PendingExceptionFlag);
}

void RequestInjectionData::setInterceptFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::InterceptFlag);
}

void RequestInjectionData::clearInterceptFlag() {
  __sync_fetch_and_and(getConditionFlags(),
                       ~RequestInjectionData::InterceptFlag);
}

void RequestInjectionData::setDebuggerSignalFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::DebuggerSignalFlag);
}

ssize_t RequestInjectionData::fetchAndClearFlags() {
  return __sync_fetch_and_and(getConditionFlags(),
                              (RequestInjectionData::EventHookFlag |
                               RequestInjectionData::InterceptFlag));
}

///////////////////////////////////////////////////////////////////////////////
}
