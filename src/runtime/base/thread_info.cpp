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

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex s_thread_info_mutex;
static std::set<ThreadInfo*> s_thread_infos;

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo() : m_executing(Idling) {
  m_mm = MemoryManager::TheMemoryManager().getNoCheck();

  m_profiler = NULL;
  m_pendingException = false;

  // get the default thread stack size once
  pthread_attr_t info;
  pthread_attr_init(&info);
  pthread_attr_getstacksize(&info, &m_stacksize);
  pthread_attr_destroy(&info);

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

  // We assume that this will be called reasonably low in the call stack.
  // Taking the address of marker gives us a location in this stack frame;
  // then, use that to calculate where the bottom of the stack should be,
  // allowing some slack for (a) stack usage above the caller of reset() and
  // (b) stack usage after the position gets checked.
  m_stacklimit = &marker - (m_stacksize - StackSlack);
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
  memExceeded = false;
  timedout    = false;
  signaled    = false;
  surprised   = false;
  debugger    = false;
  while (!interrupts.empty()) interrupts.pop();
}

///////////////////////////////////////////////////////////////////////////////
}
