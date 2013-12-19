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
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"
#include "folly/String.h"

#include <atomic>
#include <sys/mman.h>

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
  RDS::requestExit();
}

RequestInjectionData::~RequestInjectionData() {
#ifndef __APPLE__
  if (m_hasTimer) {
    timer_delete(m_timer_id);
  }
#endif
}

void RequestInjectionData::onSessionInit() {
  RDS::requestInit();
  cflagsPtr = &RDS::header()->conditionFlags;
  reset();
}

void RequestInjectionData::onTimeout() {
  setTimedOutFlag();
  m_timerActive.store(false, std::memory_order_relaxed);
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
    auto const& clockType =
      RuntimeOption::TimeoutsUseWallTime ? CLOCK_REALTIME :
                                           CLOCK_THREAD_CPUTIME_ID;
    if (timer_create(clockType, &sev, &m_timer_id)) {
      raise_error("Failed to set timeout: %s", folly::errnoStr(errno).c_str());
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

/*
 * If seconds == 0, reset the timeout to the last one set
 * If seconds  < 0, set the timeout to -seconds if there's less than
 *                  -seconds remaining.
 * If seconds  > 0, set the timeout to seconds.
 */
void RequestInjectionData::resetTimer(int seconds /* = 0 */) {
  auto data = &ThreadInfo::s_threadInfo->m_reqInjectionData;
  if (seconds == 0) {
    seconds = data->getTimeout();
  } else if (seconds < 0) {
    if (!data->getTimeout()) return;
    seconds = -seconds;
    if (seconds < data->getRemainingTime()) return;
  }
  data->setTimeout(seconds);
  data->clearTimedOutFlag();
}

void RequestInjectionData::reset() {
  getConditionFlags()->store(0);
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
  getConditionFlags()->fetch_or(RequestInjectionData::MemExceededFlag);
}

void RequestInjectionData::setTimedOutFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::TimedOutFlag);
}

void RequestInjectionData::clearTimedOutFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::TimedOutFlag);
}

void RequestInjectionData::setSignaledFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::SignaledFlag);
}

void RequestInjectionData::setEventHookFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::EventHookFlag);
}

void RequestInjectionData::clearEventHookFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::EventHookFlag);
}

void RequestInjectionData::setPendingExceptionFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::PendingExceptionFlag);
}

void RequestInjectionData::clearPendingExceptionFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::PendingExceptionFlag);
}

void RequestInjectionData::setInterceptFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::InterceptFlag);
}

void RequestInjectionData::clearInterceptFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::InterceptFlag);
}

void RequestInjectionData::setDebuggerSignalFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::DebuggerSignalFlag);
}

ssize_t RequestInjectionData::fetchAndClearFlags() {
  return getConditionFlags()->fetch_and(RequestInjectionData::EventHookFlag |
                                        RequestInjectionData::InterceptFlag);
}

///////////////////////////////////////////////////////////////////////////////
}
