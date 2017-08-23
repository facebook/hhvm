/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <functional>

#include "hphp/util/thread-local.h"
#include "hphp/runtime/base/request-injection-data.h"
#include "hphp/runtime/base/surprise-flags.h"

namespace HPHP {

struct MemoryManager;
struct Profiler;
struct CodeCoverage;
struct DebuggerHook;
struct c_WaitableWaitHandle;

//////////////////////////////////////////////////////////////////////

struct ThreadInfo {
  enum Executing {
    Idling,
    RuntimeFunctions,
    ExtensionFunctions,
    UserFunctions,
    NetworkIO,
  };
  /*
   * Set to Idle when Treadmill::finishRequest().
   * Set to OnRequestWithNoPendingExecution when Treadmill::startRequest()
   * If SetPendingGCForAllOnRequestThread() get called,
   * set to OnRequestWithPendingExecution for each on-request thread
   */
  enum GlobalGCStatus {
    Idle,
    OnRequestWithNoPendingExecution,
    OnRequestWithPendingExecution,
  };

  static void GetExecutionSamples(std::map<Executing, int>& counts);
  static void ExecutePerThread(std::function<void(ThreadInfo*)> f);
  /*
   * Only on-request threads should set up PendingGCFlag
   * Returns number of on-request threads
   */
  static int SetPendingGCForAllOnRequestThread();
  static DECLARE_THREAD_LOCAL_NO_CHECK(ThreadInfo, s_threadInfo);

  /*
   * This is the amount of "slack" in stack usage checks - if the stack pointer
   * gets within this distance from the end (minus overhead), throw an infinite
   * recursion exception.
   */
  static constexpr int StackSlack = 1024 * 1024;

  /*
   * Since this is often used as a static global, we want to do anything that
   * might try to access ThreadInfo::s_threadInfo here instead of in the
   * constructor.
   */
  void init();

  void onSessionInit();
  void onSessionExit();

  /*
   * Change m_globalGCStatus as atomic operation
   * Returns true if m_globalGCStatus == from when user call this, and atomic
   * change it
   */
  bool changeGlobalGCStatus(GlobalGCStatus from, GlobalGCStatus to);

  /*
   * Setting the pending exception.
   */
  void setPendingException(Exception*);

  static bool valid(ThreadInfo*);

  ThreadInfo();
  ~ThreadInfo();

  ////////////////////////////////////////////////////////////////////

  RequestInjectionData m_reqInjectionData;

  /* This pointer is set by ProfilerFactory. */
  Profiler* m_profiler{nullptr};

  CodeCoverage* m_coverage{nullptr};

  /* Set by DebuggerHook::attach(). */
  DebuggerHook* m_debuggerHook{nullptr};

  /* A C++ exception which will be thrown by the next surprise check. */
  Exception* m_pendingException{nullptr};

  Executing m_executing{Idling};

private:
  std::atomic<GlobalGCStatus> m_globalGCStatus{Idle};
};

//////////////////////////////////////////////////////////////////////

/*
 * Access to the running thread's ThreadInfo and RequestInjectionData.
 */

inline ThreadInfo& TI() {
  return *ThreadInfo::s_threadInfo;
}

inline RequestInjectionData& RID() {
  return TI().m_reqInjectionData;
}

//////////////////////////////////////////////////////////////////////

void raise_infinite_recursion_error();

inline void* stack_top_ptr() {
  char marker;

  // gcc warns about directly returning pointers to local variables.
  auto to_trick_gcc = static_cast<void*>(&marker);
  return to_trick_gcc;
}

inline bool stack_in_bounds() {
  return uintptr_t(stack_top_ptr()) >= s_stackLimit + ThreadInfo::StackSlack;
}

/*
 * Raises an error when infinite recursion is detected.
 *
 * It's recommended to use check_recursion_throw() instead of this, as raising
 * an error will use much more stack than throwing an exception, making this
 * have a higher chance of blowing out what little stack the thread has left.
 */
inline void check_recursion_error() {
  if (LIKELY(stack_in_bounds())) return;
  raise_infinite_recursion_error();
}

/* Throws exception when infinite recursion is detected. */
inline void check_recursion_throw() {
  if (LIKELY(stack_in_bounds())) return;
  throw Exception("Maximum stack size reached");
}

size_t handle_request_surprise(c_WaitableWaitHandle* wh = nullptr,
                               size_t mask = kSurpriseFlagMask);

/*
 * Check and handle non-safepoint surprise conditions.
 *
 * The intended use case for this helper is for instrumenting resource exceeded
 * checks within an unbounded operation implemented in C++, like array
 * unserialization or JSON decoding.
 */
inline void check_non_safepoint_surprise() {
  if (UNLIKELY(getSurpriseFlag(NonSafepointFlags))) {
    handle_request_surprise(nullptr, NonSafepointFlags);
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif
