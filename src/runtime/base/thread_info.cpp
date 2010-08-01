/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/memory/smart_allocator.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(ThreadInfo, ThreadInfo::s_threadInfo);

ThreadInfo::ThreadInfo() {
  map<int, ObjectAllocatorWrapper *> &wrappers =
    ObjectAllocatorCollector::getWrappers();
  m_allocators.resize(wrappers.rbegin()->first + 1);
  for (map<int, ObjectAllocatorWrapper *>::iterator it = wrappers.begin();
       it != wrappers.end(); it++) {
    m_allocators[it->first] = it->second->get();
  }

  m_profiler = NULL;

  // get the default thread stack size once
  pthread_attr_t info;
  pthread_attr_init(&info);
  pthread_attr_getstacksize(&info, &m_stacksize);
  pthread_attr_destroy(&info);

  onSessionInit();
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
  m_stacklimit = &marker - (m_stacksize - RecursionInjection::StackSlack);
}

void RequestInjectionData::onSessionInit() {
  started     = time(0);
  memExceeded = false;
  timedout    = false;
  signaled    = false;
  surprised   = false;
  debugger    = false;
}

///////////////////////////////////////////////////////////////////////////////
}
