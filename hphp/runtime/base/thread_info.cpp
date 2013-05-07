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
#include <runtime/base/code_coverage.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/vm/translator/targetcache.h>
#include <util/lock.h>
#include <util/alloc.h>

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

  VM::Transl::TargetCache::threadInit();
  onSessionInit();

  Lock lock(s_thread_info_mutex);
  s_thread_infos.insert(this);
}

ThreadInfo::~ThreadInfo() {
  t_stackbase = 0;

  Lock lock(s_thread_info_mutex);
  s_thread_infos.erase(this);
  delete m_coverage;
  VM::Transl::TargetCache::threadExit();
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
  m_reqInjectionData.reset();
  VM::Transl::TargetCache::requestExit();
}

void RequestInjectionData::onSessionInit() {
  VM::Transl::TargetCache::requestInit();
  cflagsPtr = VM::Transl::TargetCache::conditionFlagsPtr();
  reset();
  started = time(0);
}

void RequestInjectionData::reset() {
  __sync_fetch_and_and(getConditionFlags(), 0);
  coverage = RuntimeOption::RecordCodeCoverage;
  debugger = false;
  debuggerIntr = false;
  while (!interrupts.empty()) interrupts.pop();
}

void RequestInjectionData::setMemExceededFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::MemExceededFlag);
}

void RequestInjectionData::setTimedOutFlag() {
  __sync_fetch_and_or(getConditionFlags(),
                      RequestInjectionData::TimedOutFlag);
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

ssize_t RequestInjectionData::fetchAndClearFlags() {
  return __sync_fetch_and_and(getConditionFlags(),
                              (RequestInjectionData::EventHookFlag |
                               RequestInjectionData::InterceptFlag));
}

///////////////////////////////////////////////////////////////////////////////
}
