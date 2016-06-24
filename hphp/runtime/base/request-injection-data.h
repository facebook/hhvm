/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/async-flow-stepper.h"
#include "hphp/runtime/vm/pc-filter.h"

#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdlib>
#include <stack>
#include <string>
#include <vector>

#ifdef __APPLE__
# include <dispatch/dispatch.h>
#elif defined(_MSC_VER)
# include <agents.h>
# include <ppltasks.h>
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct RequestInjectionData;

//////////////////////////////////////////////////////////////////////

struct RequestTimer {
  friend struct RequestInjectionData;

#if defined(__APPLE__) || defined(_MSC_VER)
  RequestTimer(RequestInjectionData*);
#else
  RequestTimer(RequestInjectionData*, clockid_t);
#endif

  ~RequestTimer();

  void setTimeout(int seconds);
  void onTimeout();
  int getRemainingTime() const;

private:
  RequestInjectionData* m_reqInjectionData;
  int m_timeoutSeconds{0};

#if defined(__APPLE__)
  void cancelTimerSource();
  dispatch_source_t m_timerSource{nullptr};
  dispatch_group_t m_timerGroup;
#elif defined(_MSC_VER)
  concurrency::task_completion_event<void>* m_tce{nullptr};
#else
  clockid_t m_clockType;
  timer_t m_timerId;

  /* Whether we've created our timer yet. */
  bool m_hasTimer{false};

  /* Set true when we activate a timer, cleared when the signal handler runs. */
  std::atomic<bool> m_timerActive{false};
#endif
};

//////////////////////////////////////////////////////////////////////

/*
 * General-purpose bag of data and options for a request.
 *
 * This class contains a lot of debugger data that must be accessed via getter
 * and setter methods.  Updating data like the "debugger attached" field
 * dynamically affects whether the request will use the JIT or not.
 *
 * Refrain from adding more fields to this class if you can help it.  It's a
 * better idea to add request local data to whatever extension you're working
 * on.
 */
struct RequestInjectionData {
  /* The state of the step out command. */
  enum class StepOutState : int8_t {
    /* Command is inactive. */
    None,
    /* Waiting for the corresponding function to exit. */
    Stepping,
    /* We have stepped out and will break on the next valid opcode. */
    Out,
  };

  RequestInjectionData()
#if defined(__APPLE__) || defined(_MSC_VER)
    : m_timer(this)
    , m_cpuTimer(this)
#else
    : m_timer(this, CLOCK_REALTIME)
    , m_cpuTimer(this, CLOCK_THREAD_CPUTIME_ID)
#endif
    {}

  ~RequestInjectionData() = default;

  static constexpr uint32_t debuggerReadOnlyOffset() {
    return offsetof(RequestInjectionData, m_debuggerAttached);
  }

  void reset();

  void onSessionInit();

  void threadInit();

  int getTimeout() const;
  void setTimeout(int seconds);

  int getCPUTimeout() const;
  void setCPUTimeout(int seconds);

  int getRemainingTime() const;
  int getRemainingCPUTime() const;

  void resetTimer(int seconds = 0);
  void resetCPUTimer(int seconds = 0);

  void onTimeout(RequestTimer*);

  /*
   * Intended to be used by threads other than the current thread.  To get
   * surprise flags for the current thread, use stackLimitAndSurprise() instead.
   */
  void clearFlag(SurpriseFlag);
  void setFlag(SurpriseFlag);

  /*
   * Whether the JIT is enabled.
   */
  bool getJit() const;
  void updateJit();

  /*
   * Whether coverage is being collected.
   */
  bool getCoverage() const;
  void setCoverage(bool);

  /*
   * Whether there is a debugger attached to the request.  Controlled by
   * DebuggerHook.  This field gets read directly by JIT'd code.
   */
  bool getDebuggerAttached();
  void setDebuggerAttached(bool);

  /*
   * Returns true if the debugger should force interrupts due to any of the
   * debugger interrupt conditions being true.
   */
  bool getDebuggerForceIntr() const;

  /*
   * Indicating we should force interrupts for debuggers.  This is intended to
   * be used by debuggers for forcing onOpcode events.
   */
  bool getDebuggerIntr() const;
  void setDebuggerIntr(bool);

  /*
   * Whether the debugger is running a "step in" command.
   */
  bool getDebuggerStepIn() const;
  void setDebuggerStepIn(bool);

  /*
   * Whether the debugger is running a "next" command.
   */
  bool getDebuggerNext() const;
  void setDebuggerNext(bool);

  /*
   * Whether the debugger is running a "step out" command, and where it is in
   * the process.
   */
  StepOutState getDebuggerStepOut() const;
  void setDebuggerStepOut(StepOutState);

  /*
   * The stack depth registered by the debugger's most recent flow command.
   * (e.g. step, next, etc.)
   */
  int getDebuggerFlowDepth() const;
  void setDebuggerFlowDepth(int);

  /*
   * Set to a line number if the request has hit a line breakpoint on the line,
   * and hasn't left that line yet.  This tracks a single line per stack frame.
   * If there's no tracked line, then it is treated as line number -1.
   */
  int getActiveLineBreak() const;
  void clearActiveLineBreak();
  void setActiveLineBreak(int);

  /*
   * Adds a slot to the active line stack upon entering or leaving a function.
   */
  void popActiveLineBreak();
  void pushActiveLineBreak();

  /*
   * Uses the active line break stack to compute the size of the stack when in
   * debug mode.
   */
  size_t getDebuggerStackDepth() const;

  /* Getters and setters for user settable INI settings. */

  const std::string& getDefaultMimeType() const;

  std::string getDefaultIncludePath();
  const std::vector<std::string>& getIncludePaths() const;

  int64_t getErrorReportingLevel();
  void setErrorReportingLevel(int64_t);

  int64_t getMemoryLimitNumeric() const;
  void setMemoryLimit(folly::StringPiece);

  const std::string& getVariablesOrder() const;
  void setVariablesOrder(const std::string&);

  const std::string& getRequestOrder() const;
  void setRequestOrder(const std::string&);

  int64_t getSocketDefaultTimeout() const;

  const std::string& getUserAgent() const;
  void setUserAgent(const std::string&);

  const std::string& getTimeZone() const;
  void setTimeZone(const std::string&);

  bool setAllowedDirectories(const std::string& value);

  const std::vector<std::string>& getAllowedDirectoriesProcessed() const;

  bool hasSafeFileAccess() const;
  bool hasTrackErrors() const;
  bool hasHtmlErrors() const;

private:
  RequestTimer m_timer;
  RequestTimer m_cpuTimer;

  bool m_debuggerAttached{false};
  bool m_coverage{false};
  bool m_jit{false};
  bool m_debuggerIntr{false};

  bool m_debuggerStepIn{false};
  bool m_debuggerNext{false};
  StepOutState m_debuggerStepOut{StepOutState::None};

public:
  PCFilter m_breakPointFilter;
  PCFilter m_flowFilter;
  PCFilter m_lineBreakPointFilter;
  PCFilter m_callBreakPointFilter;
  PCFilter m_retBreakPointFilter;
  // Only allow one async stepper at a time.
  AsyncFlowStepper m_asyncStepper;

private:
  int m_debuggerFlowDepth{0};

  /* INI settings. */
  bool m_logErrors{false};
  bool m_trackErrors{false};
  bool m_htmlErrors{false};
  bool m_safeFileAccess{false};

  /* Pointer to surprise flags stored in RDS. */
  std::atomic<size_t>* m_sflagsAndStkPtr{nullptr};

  std::stack<int> m_activeLineBreaks;

  /* Things corresponding to user settable INI settings. */

  std::string m_maxMemory;
  std::string m_argSeparatorOutput;
  std::string m_argSeparatorInput;
  std::string m_variablesOrder;
  std::string m_requestOrder;
  std::string m_defaultCharset;
  std::string m_defaultMimeType;
  std::string m_brotliEnabled;
  std::string m_brotliChunkedEnabled;
  std::string m_gzipCompressionLevel = "-1";
  std::string m_gzipCompression;
  std::string m_errorLog;
  std::string m_userAgent;
  std::string m_timezone;
  std::vector<std::string> m_include_paths;
  struct AllowedDirectoriesInfo {
    AllowedDirectoriesInfo(std::vector<std::string>&& v,
                           std::string&& s) :
        vec(std::move(v)), string(std::move(s)) {}
    std::vector<std::string> vec;
    std::string string;
  };
  std::unique_ptr<AllowedDirectoriesInfo> m_allowedDirectoriesInfo;
  int64_t m_errorReportingLevel;
  int64_t m_socketDefaultTimeout;
  int64_t m_maxMemoryNumeric;
  int64_t m_zendAssertions;
  int64_t m_brotliLgWindowSize;
  int64_t m_brotliQuality;

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

#define incl_HPHP_REQUEST_INJECTION_DATA_INL_H_
#include "hphp/runtime/base/request-injection-data-inl.h"
#undef incl_HPHP_REQUEST_INJECTION_DATA_INL_H_

#endif
