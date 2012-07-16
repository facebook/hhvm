/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/base/types.h>
#include <runtime/base/hphp_system.h>
#include <runtime/base/memory/smart_allocator.h>
#include <util/lock.h>
#include <util/alloc.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex s_thread_info_mutex;
static std::set<ThreadInfo*> s_thread_infos;

IMPLEMENT_THREAD_LOCAL_NO_CHECK_HOT(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo() : m_executing(Idling) {
  m_mm = MemoryManager::TheMemoryManager().getNoCheck();

  m_profiler = NULL;
  m_pendingException = false;

  onSessionInit();

  Lock lock(s_thread_info_mutex);
  s_thread_infos.insert(this);
}

ThreadInfo::~ThreadInfo() {
  Lock lock(s_thread_info_mutex);
  s_thread_infos.erase(this);
}

void ThreadInfo::GetExecutionSamples(std::map<Executing, int> &counts) {
  Lock lock(s_thread_info_mutex);
  for (std::set<ThreadInfo*>::const_iterator iter = s_thread_infos.begin();
       iter != s_thread_infos.end(); ++iter) {
    ++counts[(*iter)->m_executing];
  }
}

void ThreadInfo::onSessionInit() {
  char marker;
  m_top = NULL;
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
    if (LIKELY(rl.rlim_cur != -1)) {
      m_stacklimit = (char *)&marker - (rl.rlim_cur - StackSlack);
    } else {
      // RLIMIT_STACK might get lost after fork or exec on linux
      // let's just calculate in the old way with default thread stack size
      pthread_attr_t info;
      size_t m_stacksize;
      pthread_attr_init(&info);
      pthread_attr_getstacksize(&info, &m_stacksize);
      pthread_attr_destroy(&info);

      m_stacklimit = (char *)&marker - (m_stacksize - StackSlack);
  }
  } else {
    m_stacklimit = (char *)Util::s_stackLimit + StackSlack;
    ASSERT(uintptr_t(m_stacklimit) < (Util::s_stackLimit + Util::s_stackSize));
  }
}

void ThreadInfo::clearPendingException() {
  if (m_pendingException) {
    m_pendingException = false;
    m_exceptionMsg.clear();
    m_exceptionStack.reset();
  }
}

void ThreadInfo::onSessionExit() {
  m_reqInjectionData.reset();
}

void RequestInjectionData::onSessionInit() {
  reset();
  started = time(0);
}

void RequestInjectionData::reset() {
  __sync_fetch_and_and(&conditionFlags, 0);
  debugger    = false;
  while (!interrupts.empty()) interrupts.pop();
}

void RequestInjectionData::setMemExceededFlag() {
  __sync_fetch_and_or(&conditionFlags, RequestInjectionData::MemExceededFlag);
}

void RequestInjectionData::setTimedOutFlag() {
  __sync_fetch_and_or(&conditionFlags, RequestInjectionData::TimedOutFlag);
}

void RequestInjectionData::setSignaledFlag() {
  __sync_fetch_and_or(&conditionFlags, RequestInjectionData::SignaledFlag);
}

ssize_t RequestInjectionData::fetchAndClearFlags() {
  ssize_t flags;
  for (;;) {
    flags = conditionFlags;
    if (__sync_bool_compare_and_swap(&conditionFlags, flags, 0)) {
      break;
    }
  }
  return flags;
}

///////////////////////////////////////////////////////////////////////////////
}
