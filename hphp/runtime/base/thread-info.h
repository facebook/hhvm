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
#ifndef incl_HPHP_THREAD_INFO_H_
#define incl_HPHP_THREAD_INFO_H_

#include <cinttypes>
#include <map>

#include "hphp/util/portability.h"
#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/request-injection-data.h"

namespace HPHP {

struct MemoryManager;
struct Profiler;
struct CodeCoverage;

//////////////////////////////////////////////////////////////////////

struct ThreadInfo {
  enum Executing {
    Idling,
    RuntimeFunctions,
    ExtensionFunctions,
    UserFunctions,
    NetworkIO,
  };

  static void GetExecutionSamples(std::map<Executing, int> &counts);
  static DECLARE_THREAD_LOCAL_NO_CHECK(ThreadInfo, s_threadInfo);

  RequestInjectionData m_reqInjectionData;

  // For infinite recursion detection.  m_stacklimit is the lowest
  // address the stack can grow to.
  char *m_stacklimit;

  // Either null, or populated by initialization of ThreadInfo as an
  // approximation of the highest address of the current thread's
  // stack.
  static __thread char* t_stackbase;

  // This is the amount of "slack" in stack usage checks - if the
  // stack pointer gets within this distance from the end (minus
  // overhead), throw an infinite recursion exception.
  static const int StackSlack = 1024 * 1024;

  MemoryManager* m_mm;

  // This pointer is set by ProfilerFactory
  Profiler *m_profiler;
  CodeCoverage *m_coverage;

  Executing m_executing;

  // A C++ exception which will be thrown by the next surprise check.
  Exception* m_pendingException;

  ThreadInfo();
  ~ThreadInfo();

  void onSessionInit();
  void onSessionExit();
  void setPendingException(Exception* e);
  void clearPendingException();

  static bool valid(ThreadInfo* info);
};

//////////////////////////////////////////////////////////////////////

inline void* stack_top_ptr() {
  DECLARE_STACK_POINTER(sp);
  return sp;
}

inline bool stack_in_bounds(ThreadInfo*& info) {
  return stack_top_ptr() >= info->m_stacklimit;
}

// The ThreadInfo pointer itself must be from the current stack frame.
inline void check_recursion(ThreadInfo*& info) {
  extern void throw_infinite_recursion_exception();
  if (!stack_in_bounds(info)) {
    throw_infinite_recursion_exception();
  }
}

ssize_t check_request_surprise(ThreadInfo *info);

//////////////////////////////////////////////////////////////////////

}

#endif
