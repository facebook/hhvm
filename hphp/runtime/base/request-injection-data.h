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

#pragma once

#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/vm/async-flow-stepper.h"
#include "hphp/runtime/vm/pc-filter.h"
#include "hphp/util/process.h"

#include <array>
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

namespace VSDEBUG {
struct DebuggerRequestInfo;
}

//////////////////////////////////////////////////////////////////////

struct RequestTimer {
  friend struct RequestInjectionData;

  RequestTimer(RequestInjectionData*, clockid_t);

  ~RequestTimer();

  void setTimeout(int seconds);
  void onTimeout();
  int getRemainingTime() const;

private:
  RequestInjectionData* m_reqInjectionData;
  int m_timeoutSeconds{0};

  clockid_t m_clockType;
  timer_t m_timerId;
  TYPE_SCAN_IGNORE_FIELD(m_timerId); // timer_t is void*

  /* Whether we've created our timer yet. */
  bool m_hasTimer{false};

  /* Set true when we activate a timer, cleared when the signal handler runs. */
  std::atomic<bool> m_timerActive{false};
};

//////////////////////////////////////////////////////////////////////

enum TimeoutKindFlag : uint8_t {
  TimeoutNone          = 0,
  TimeoutTime          = 1ull << 1,
  TimeoutCPUTime       = 1ull << 2,
  TimeoutSoft          = 1ull << 3
};

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
    : m_timer(this, CLOCK_REALTIME)
    , m_cpuTimer(this, CLOCK_THREAD_CPUTIME_ID)
    , m_userTimeoutTimer(this, CLOCK_REALTIME)
    {}

  ~RequestInjectionData() = default;

  void reset();

  void onSessionInit();

  void threadInit();

  int getTimeout() const;
  void setTimeout(int seconds);
  void triggerTimeout(TimeoutKindFlag kind);
  bool checkTimeoutKind(TimeoutKindFlag kind);
  void clearTimeoutFlag(TimeoutKindFlag kind);

  /*
   * Sets/Returns the amount of seconds until user time based callback is fired
   */
  int getUserTimeout() const;
  void setUserTimeout(int seconds);
  /*
   * Triggers the user time based callback
   */
  void invokeUserTimeoutCallback(c_WaitableWaitHandle* wh = nullptr);

  int getCPUTimeout() const;
  void setCPUTimeout(int seconds);

  int getRemainingTime() const;
  int getRemainingCPUTime() const;
  int getUserTimeoutRemainingTime() const;

  void resetTimers(int time_sec = 0, int cputime_sec = 0);

  void onTimeout(RequestTimer*);

  /*
   * Intended to be used by threads other than the current thread.  To get
   * surprise flags for the current thread, use stackLimitAndSurprise() instead.
   */
  void clearFlag(SurpriseFlag);
  void setFlag(SurpriseFlag);

  /*
   * Deliver a "POSIX signal" through the SignaledFlag.
   */
  void sendSignal(int signum);
  /*
   * Get the next pending signal for this request, and clear the corresponding
   * bit to avoid reentering in the signal handler.  When there is no pending
   * signal, return 0.
   */
  int getAndClearNextPendingSignal();

  /*
   * Flags for rquest-level OOM killer.  The `m_hostOutOfMemory` flag is set on
   * all requests when host is low in memory, which triggers a memory check upon
   * checking surprise flags.  The `m_OOMAbort` is set when we decide to kill
   * the request.
   */
  void setHostOOMFlag() {
    m_hostOutOfMemory.store(true, std::memory_order_release);
    setFlag(MemExceededFlag);
  }
  void clearHostOOMFlag() {
    clearFlag(MemExceededFlag);
    m_hostOutOfMemory.store(false, std::memory_order_relaxed);
  }
  bool hostOOMFlag() const {
    return m_hostOutOfMemory.load(std::memory_order_acquire);
  }
  void setRequestOOMAbort() {
    m_OOMAbort = true;
  }
  bool shouldOOMAbort() const {
    return m_OOMAbort;
  }
  /*
   * Flags for request memory exceeded.  To differentiate from the above host
   * level OOMs.
   */
  void setRequestOOMFlag() { m_requestOOM = true; }
  void clearRequestOOMFlag() { m_requestOOM = false; }
  bool requestOOMFlag() const { return m_requestOOM; }


  /*
   * Whether the JIT is enabled.
   */
  bool getJit() const;
  void updateJit();

  /*
   * Whether jitting is disabled.
   *
   * This is distinct from getJit(), in that it allows us to _run_ jitted code,
   * but not to _compile_ it.  Also, the restriction applies only to jitting
   * new code; optimizing retranslations is not affected.
   */
  bool isJittingDisabled() const;
  void setJittingDisabled(bool);

  /*
   * Whether the JIT is performing function folding.
   */
  bool getJitFolding() const;
  void setJitFolding(bool);

  /*
   * Whether to suppress the emission of Class to String conversion warnings.
   */
  bool getSuppressClassConversionNotices() const;
  void setSuppressClassConversionNotices(bool);

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

  void setVSDebugDisablesJit(bool);
  bool getVSDebugDisablesJit() const { return m_vsdebugDisablesJit; }

  /*
   * Returns true if the debugger should force interrupts due to any of the
   * debugger interrupt conditions being true.
   */
  bool getDebuggerForceIntr() const;
  void setDebuggerIntr(bool);

  /*
   * Whether the debugger is running any step command.
   */
  bool getDebuggerStepIntr() const;

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

  VSDEBUG::DebuggerRequestInfo* getDebuggerRequestInfo() const {
    return m_debuggerRI.load(std::memory_order_acquire);
  }
  void setDebuggerRequestInfo(VSDEBUG::DebuggerRequestInfo* debuggerRI) {
    m_debuggerRI.store(debuggerRI, std::memory_order_release);
  }

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

  /*
   * Clear all breakpoint and flow filters.
   */
  void clearPCFilters();

  /* Getters and setters for user settable INI settings. */

  const std::string& getDefaultMimeType() const;

  std::string getDefaultIncludePath();
  const std::vector<std::string>& getIncludePaths() const;

  int64_t getErrorReportingLevel();
  void setErrorReportingLevel(int64_t);

  int64_t getMemoryLimitNumeric() const;
  void setMemoryLimit(folly::StringPiece);

  int64_t getSocketDefaultTimeout() const;

  const std::string& getUserAgent() const;
  void setUserAgent(const std::string&);

  const std::string& getTimezone() const;
  void setTimezone(const std::string&);

  bool setAllowedDirectories(const std::string& value);

  const std::vector<std::string>& getAllowedDirectoriesProcessed() const;

  // When safe file access is enabled only whitelisted by setAllowedDirectories
  // may be modified
  void setSafeFileAccess(bool b);
  bool hasSafeFileAccess() const;
  bool hasHtmlErrors() const;

  bool logFunctionCalls() const;

private:
  void resetTimer(int seconds = 0);
  void resetCPUTimer(int seconds = 0);
  void resetUserTimeoutTimer(int seconds = 0);
  RequestTimer m_timer;
  RequestTimer m_cpuTimer;
  RequestTimer m_userTimeoutTimer;

  bool m_debuggerAttached{false};
  bool m_vsdebugDisablesJit{false};
  bool m_coverage{false};
  bool m_jit{false};
  bool m_jittingDisabled{false};
  bool m_jitFolding{false};
  bool m_debuggerIntr{false};

  bool m_suppressClassConversionWarnings{false};

  bool m_debuggerStepIn{false};
  bool m_debuggerNext{false};
  StepOutState m_debuggerStepOut{StepOutState::None};

  std::atomic<VSDEBUG::DebuggerRequestInfo*> m_debuggerRI{nullptr};

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
  bool m_logFunctionCalls{false};

  static constexpr size_t kSigMaskWords = (Process::kNSig + 63) / 64;
  std::array<std::atomic<uint64_t>, kSigMaskWords> m_signalMask{};

  /*
   * `m_hostOutOfMemory` is a flag used together with MemExceededFlag, to
   * indicate whether the host is running low on memory.  Note that the presence
   * of this flag doesn't necessarily lead to the request being aborted.  A
   * request is only affected when it satisfies some other criteria, e.g., when
   * it uses more memory than RequestMemoryOOMKillBytes.  If we do decide to
   * abort the request, `m_OOMAbort` is set.
   */
  std::atomic<bool> m_hostOutOfMemory{false};
  bool m_OOMAbort{false};

  /* To differentiate request OOMs from host OOMs */
  bool m_requestOOM{false};

  /* Pointer to surprise flags stored in RDS. */
  std::atomic<size_t>* m_sflagsAndStkPtr{nullptr};

  std::stack<int> m_activeLineBreaks;

  /* Things corresponding to user settable INI settings. */

  std::string m_maxMemory;
  std::string m_defaultCharset;
  std::string m_defaultMimeType;
  std::string m_brotliEnabled;
  std::string m_brotliChunkedEnabled;
  std::string m_zstdEnabled;
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
  int64_t m_brotliLgWindowSize;
  int64_t m_brotliQuality;
  int64_t m_zstdLevel;
  int64_t m_zstdChecksumRate;
  int64_t m_zstdWindowLog;
  int64_t m_zstdTargetBlockSize;

  /*
   * Instead of using several surprise flags, we can track the timeout info
   * in its own array of flags. This allows us to define different kind of
   * of timeouts.
   */
  std::atomic<uint8_t> m_timeoutFlags;

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
