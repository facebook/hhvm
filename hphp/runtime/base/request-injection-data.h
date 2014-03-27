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
#ifndef incl_HPHP_REQUEST_INJECTION_DATA_H_
#define incl_HPHP_REQUEST_INJECTION_DATA_H_

#include <cinttypes>
#include <cstdlib>
#include <vector>
#include <atomic>
#include <string>
#include <stack>
#include <cassert>
#include <stddef.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct RequestInjectionData {
  static const ssize_t MemExceededFlag      = 1 << 0;
  static const ssize_t TimedOutFlag         = 1 << 1;
  static const ssize_t SignaledFlag         = 1 << 2;
  static const ssize_t EventHookFlag        = 1 << 3;
  static const ssize_t PendingExceptionFlag = 1 << 4;
  static const ssize_t InterceptFlag        = 1 << 5;
  // Set by the debugger to break out of loops in translated code.
  static const ssize_t DebuggerSignalFlag   = 1 << 6;
  static const ssize_t LastFlag             = DebuggerSignalFlag;

  RequestInjectionData()
      : cflagsPtr(nullptr),
        m_timeoutSeconds(0), // no timeout by default
        m_hasTimer(false),
        m_timerActive(false),
        m_debugger(false),
        m_debuggerIntr(false),
        m_coverage(false),
        m_jit(false) {
    threadInit();
  }

  ~RequestInjectionData();

  void threadInit();

  inline std::atomic<ssize_t>* getConditionFlags() {
    assert(cflagsPtr);
    return cflagsPtr;
  }

  std::atomic<ssize_t>* cflagsPtr;  // this points to the real condition flags,
                                    // somewhere in the thread's targetcache

 private:
#ifndef __APPLE__
  timer_t m_timer_id;    // id of our timer
#endif
  int m_timeoutSeconds;  // how many seconds to timeout
  bool m_hasTimer;       // Whether we've created our timer yet
  std::atomic<bool> m_timerActive;
                         // Set true when we activate a timer,
                         // cleared when the signal handler runs
  bool m_debugger;       // whether there is a DebuggerProxy attached to me
  bool m_debuggerIntr;   // indicating we should force interrupt for debugger
  bool m_coverage;       // is coverage being collected
  bool m_jit;            // is the jit enabled

  // Things corresponding to user setable INI settings
  std::string m_maxMemory;
  std::string m_argSeparatorOutput;
  std::string m_defaultCharset;
  std::string m_defaultMimeType;
  std::vector<std::string> m_include_paths;
  int64_t m_errorReportingLevel;
  bool m_logErrors;
  std::string m_errorLog;
  int64_t m_socketDefaultTimeout;
  std::vector<std::string> m_allowedDirectories;
  bool m_safeFileAccess;

 public:
  std::string getDefaultMimeType() { return m_defaultMimeType; }
  int getTimeout() const { return m_timeoutSeconds; }
  void setTimeout(int seconds);
  int getRemainingTime() const;
  void resetTimer(int seconds = 0);
  void onTimeout();
  bool getJit() const { return m_jit; }
  bool getDebugger() const { return m_debugger; }
  void setDebugger(bool d) {
    m_debugger = d;
    updateJit();
  }
  static constexpr uint32_t debuggerReadOnlyOffset() {
    return offsetof(RequestInjectionData, m_debugger);
  }
  bool getDebuggerIntr() const { return m_debuggerIntr; }
  void setDebuggerIntr(bool d) {
    m_debuggerIntr = d;
    updateJit();
  }
  bool getCoverage() const { return m_coverage; }
  void setCoverage(bool flag) {
    m_coverage = flag;
    updateJit();
  }
  void updateJit();


  // getters for user setable INI settings
  std::vector<std::string> getIncludePaths() { return m_include_paths; }
  std::string getDefaultIncludePath();
  int64_t getErrorReportingLevel() { return m_errorReportingLevel; }
  void setErrorReportingLevel(int level) { m_errorReportingLevel = level; }
  int64_t getSocketDefaultTimeout() const { return m_socketDefaultTimeout; }
  std::vector<std::string> getAllowedDirectories() const {
    return m_allowedDirectories;
  }
  bool hasSafeFileAccess() const { return m_safeFileAccess; }

  std::stack<void *> interrupts;   // CmdInterrupts this thread's handling

  void reset();

  void setMemExceededFlag();
  void setTimedOutFlag();
  void clearTimedOutFlag();
  void setSignaledFlag();
  void setEventHookFlag();
  void clearEventHookFlag();
  void setPendingExceptionFlag();
  void clearPendingExceptionFlag();
  void setInterceptFlag();
  void clearInterceptFlag();
  void setDebuggerSignalFlag();
  ssize_t fetchAndClearFlags();

  void onSessionInit();
};

//////////////////////////////////////////////////////////////////////

}

#endif
