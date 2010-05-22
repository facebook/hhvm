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
  m_allocators.resize(wrappers.size());
  for (map<int, ObjectAllocatorWrapper *>::iterator it = wrappers.begin();
       it != wrappers.end(); it++) {
    m_allocators[it->first] = it->second->get();
  }

  m_top = NULL;
  m_stackdepth = 0;
  m_profiler = NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
