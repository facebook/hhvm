/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/vm/pc-filter.h"

#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdlib>
#include <stack>
#include <string>
#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct RequestInjectionData;

//////////////////////////////////////////////////////////////////////

struct RequestTimer {
  friend class RequestInjectionData;

  RequestTimer(RequestInjectionData*, clockid_t);
  ~RequestTimer();

  void setTimeout(int seconds);
  void onTimeout();
  int getRemainingTime() const;

private:
  RequestInjectionData* m_reqInjectionData;
  clockid_t m_clockType;
#ifndef __APPLE__
  timer_t m_timer_id;      // id of our timer
#endif
  int m_timeoutSeconds;    // how many seconds to timeout
  bool m_hasTimer;         // Whether we've created our timer yet
  std::atomic<bool> m_timerActive;
                           // Set true when we activate a timer,
                           // cleared when the signal handler runs
};

//////////////////////////////////////////////////////////////////////

#ifdef OUT
# undef OUT
#endif
struct RequestInjectionData {
  /* The state of the step out command. */
  enum class StepOutState : int8_t {
    NONE,     // Command is inactive
    STEPPING, // Waiting for the corresponding function to exit
    OUT       // We have stepped out and will break on the next valid opcode
  };

  RequestInjectionData()
#ifdef __APPLE__
    // OS X doesn't have CLOCK_THREAD_CPUTIME_ID... but it also doesn't have an
    // implementation of POSIX timers at all, so all of RequestTimer is ifdef'd
    // out anyways. Just pass dummy values.
     : m_timer(this, 0)
     , m_cpuTimer(this, 0)
#else
      : m_timer(this, CLOCK_REALTIME)
      , m_cpuTimer(this, CLOCK_THREAD_CPUTIME_ID)
#endif
    {}

  ~RequestInjectionData() = default;

  void threadInit();

  std::string getDefaultMimeType() { return m_defaultMimeType; }
  int getTimeout() const { return m_timer.m_timeoutSeconds; }
  int getCPUTimeout() const { return m_cpuTimer.m_timeoutSeconds; }
  void setTimeout(int seconds);
  void setCPUTimeout(int seconds);
  int getRemainingTime() const;
  int getRemainingCPUTime() const;
  void resetTimer(int seconds = 0);
  void resetCPUTimer(int seconds = 0);
  void onTimeout(RequestTimer*);
  bool getJit() const { return m_jit; }
  void updateJit();

  bool getCoverage() const { return m_coverage; }
  void setCoverage(bool flag) {
    m_coverage = flag;
    updateJit();
  }

  bool getDebuggerAttached() { return m_debuggerAttached; }
  // Should only be set by DebuggerHookHandler::attach
  void setDebuggerAttached(bool d) {
    m_debuggerAttached = d;
    updateJit();
  }

  // Uses the active line break stack to compute the size of the stack when
  // in debug mode
  int getDebuggerStackDepth() const { return m_activeLineBreaks.size(); }

  // Returns true if the debugger should force interrupts due to any of the
  // debugger interrupt conditions being true
  bool getDebuggerForceIntr() const {
    return
      m_debuggerIntr ||
      // Force interrupts when over an active line
      getActiveLineBreak() != -1 ||
      // Interrupts forced while stepping in
      m_debuggerStepIn ||
      // step out forces interrupts after we have exited the function
      m_debuggerStepOut == StepOutState::OUT;
  }

  static constexpr uint32_t debuggerReadOnlyOffset() {
    return offsetof(RequestInjectionData, m_debuggerAttached);
  }

  bool getDebuggerIntr() const { return m_debuggerIntr; }
  void setDebuggerIntr(bool d) {
    m_debuggerIntr = d;
    updateJit();
  }

  bool getDebuggerStepIn() const { return m_debuggerStepIn; }
  void setDebuggerStepIn(bool d) {
    m_debuggerStepIn = d;
    updateJit();
  }

  StepOutState getDebuggerStepOut() const { return m_debuggerStepOut; }
  void setDebuggerStepOut(StepOutState state) {
    m_debuggerStepOut = state;
    updateJit();
  }

  bool getDebuggerNext() const { return m_debuggerNext; }
  void setDebuggerNext(bool d) {
    m_debuggerNext = d;
    updateJit();
  }

  int getDebuggerFlowDepth() const { return m_debuggerFlowDepth; }
  void setDebuggerFlowDepth(int depth) {
    m_debuggerFlowDepth = depth;
  }

  int getActiveLineBreak() const {
    return m_activeLineBreaks.size() == 0 ? -1 : m_activeLineBreaks.top();
  }
  void setActiveLineBreak(int line) {
    if (m_activeLineBreaks.size()) {
      m_activeLineBreaks.top() = line;
      updateJit();
    }
  }
  void popActiveLineBreak() {
    if (m_activeLineBreaks.size()) {
      m_activeLineBreaks.pop();
      updateJit();
    }
  }
  void pushActiveLineBreak(int line) {
    m_activeLineBreaks.push(line);
    updateJit();
  }

  /* Getters for user settable INI settings. */
  const std::vector<std::string>& getIncludePaths() const {
    return m_include_paths;
  }

  std::string getDefaultIncludePath();
  int64_t getErrorReportingLevel() { return m_errorReportingLevel; }
  void setErrorReportingLevel(int level) { m_errorReportingLevel = level; }

  const std::string& getVariablesOrder() const {
    return m_variablesOrder;
  }

  void setVariablesOrder(std::string variablesOrder) {
    m_variablesOrder = variablesOrder;
  }

  std::string getRequestOrder() const { return m_requestOrder; }
  void setRequestOrder(std::string requestOrder) {
    m_requestOrder = requestOrder;
  }
  int64_t getSocketDefaultTimeout() const { return m_socketDefaultTimeout; }
  std::string getUserAgent() { return m_userAgent; }
  void setUserAgent(std::string userAgent) { m_userAgent = userAgent; }
  std::vector<std::string> getAllowedDirectories() const {
    return m_allowedDirectories;
  }
  bool hasSafeFileAccess() const { return m_safeFileAccess; }
  bool hasTrackErrors() const { return m_trackErrors; }
  bool hasHtmlErrors() const { return m_htmlErrors; }

  void reset();

  void onSessionInit();

  /*
   * Intended to be used by other threads other than the current thread.  To get
   * surprise flags for the current thread, use rds::surpriseFlags() instead.
   */
  void clearFlag(SurpriseFlag);
  void setFlag(SurpriseFlag);

private:
  RequestTimer m_timer;
  RequestTimer m_cpuTimer;
  bool m_debuggerAttached{false}; // whether there is a debugger attached.
  bool m_coverage{false};         // is coverage being collected
  bool m_jit{false};              // is the jit enabled

  /*
   * Indicating we should force interrupts for debuggers.  This is intended to
   * be used by debuggers for forcing onOpcode events. It shouldn't be modified
   * internally.
   */
  bool m_debuggerIntr{false};

  /* Whether the commands step out or next are active. */
  bool m_debuggerStepIn{false};
  bool m_debuggerNext{false};

  /* The actual step out state. */
  StepOutState m_debuggerStepOut{StepOutState::NONE};

public:
  PCFilter m_breakPointFilter;
  PCFilter m_flowFilter;
  PCFilter m_lineBreakPointFilter;
  PCFilter m_callBreakPointFilter;
  PCFilter m_retBreakPointFilter;

private:
  /* The stack depth used by step out and next. */
  int m_debuggerFlowDepth;

  /* INI settings. */
  bool m_logErrors;
  bool m_trackErrors;
  bool m_htmlErrors{false};
  bool m_safeFileAccess;

  /* Pointer to surprise flags stored in RDS. */
  std::atomic<size_t>* m_sflagsAndStkPtr{nullptr};

  /*
   * When the PC is currently over a line that has been registered for a line
   * break, the top element is the line.  Otherwise the top element is -1.  On
   * function entry we push -1, on function exit, we pop.
   */
  std::stack<int> m_activeLineBreaks;

  /* Things corresponding to user settable INI settings. */
  std::string m_maxMemory;
  std::string m_argSeparatorOutput;
  std::string m_argSeparatorInput;
  std::string m_variablesOrder;
  std::string m_requestOrder;
  std::string m_defaultCharset;
  std::string m_defaultMimeType;
  std::string m_gzipCompressionLevel = "-1";
  std::string m_gzipCompression;
  std::string m_errorLog;
  std::string m_userAgent;
  std::vector<std::string> m_include_paths;
  std::vector<std::string> m_allowedDirectories;
  int64_t m_errorReportingLevel;
  int64_t m_socketDefaultTimeout;

  /*
   * Keep track of the open_basedir_separator that may be used so we can
   * have backwards compatibility with our current ;.
   * This is a simple fix with the caveat that we don't mix the characters
   * in an ini file or ini_set().
   * Moving forward we should just use s_PATH_SEPARATOR and support only that
   */
  std::string m_open_basedir_separator;

 public:
  /* CmdInterrupts this thread is handling. */
  std::stack<void*> interrupts;
};

//////////////////////////////////////////////////////////////////////

}

#endif
