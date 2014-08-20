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

#include <cinttypes>
#include <cstdlib>
#include <vector>
#include <atomic>
#include <string>
#include <stack>
#include <cassert>
#include <cstddef>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct RequestInjectionData {
  static const ssize_t MemExceededFlag      = 1 << 0;
  static const ssize_t TimedOutFlag         = 1 << 1;
  static const ssize_t SignaledFlag         = 1 << 2;
  static const ssize_t EventHookFlag        = 1 << 3;
  static const ssize_t PendingExceptionFlag = 1 << 4;
  static const ssize_t InterceptFlag        = 1 << 5;
  static const ssize_t XenonSignalFlag      = 1 << 6;
  static const ssize_t AsyncEventHookFlag   = 1 << 7;
  // Set by the debugger to break out of loops in translated code.
  static const ssize_t DebuggerSignalFlag   = 1 << 8;
  // Set by the debugger hook handler to force function entry/exit events
  static const ssize_t DebuggerHookFlag     = 1 << 9;
  static const ssize_t LastFlag             = DebuggerHookFlag;
  // flags that shouldn't be cleared by fetchAndClearFlags, because:
  // fetchAndClearFlags is only supposed to touch flags related to PHP-visible
  // signals/exceptions and resource limits
  static const ssize_t StickyFlags = RequestInjectionData::AsyncEventHookFlag |
                                     RequestInjectionData::DebuggerHookFlag |
                                     RequestInjectionData::EventHookFlag |
                                     RequestInjectionData::InterceptFlag |
                                     RequestInjectionData::XenonSignalFlag;

  RequestInjectionData()
      : cflagsPtr(nullptr),
        m_timeoutSeconds(0), // no timeout by default
        m_hasTimer(false),
        m_timerActive(false),
        m_debuggerAttached(false),
        m_debuggerIntr(false),
        m_debuggerStepIn(false),
        m_debuggerStepOut(false),
        m_debuggerNext(false),
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
  timer_t m_timer_id;      // id of our timer
#endif
  int m_timeoutSeconds;    // how many seconds to timeout
  bool m_hasTimer;         // Whether we've created our timer yet
  std::atomic<bool> m_timerActive;
                           // Set true when we activate a timer,
                           // cleared when the signal handler runs
  bool m_debuggerAttached; // whether there is a debugger attached.
  bool m_debuggerIntr;     // indicating we should force interrupt for debugger
  bool m_debuggerStepIn;   // These indicate whether or not the debugger is
  bool m_debuggerStepOut;  // currently stepping in, stepping out, or stepping
  bool m_debuggerNext;     // to the next line
  bool m_coverage;         // is coverage being collected
  bool m_jit;              // is the jit enabled

  // When the PC is currently over a line that has been registered for a line
  // break, the top element is the line. Otherwise the top element is -1.
  // On function entry we push -1, on function exit, we pop.
  std::stack<int> m_activeLineBreaks;

  // Things corresponding to user setable INI settings
  std::string m_maxMemory;
  std::string m_argSeparatorOutput;
  std::string m_argSeparatorInput;
  std::string m_defaultCharset;
  std::string m_defaultMimeType;
  std::vector<std::string> m_include_paths;
  int64_t m_errorReportingLevel;
  bool m_logErrors;
  std::string m_errorLog;
  bool m_trackErrors;
  int64_t m_socketDefaultTimeout;
  std::string m_userAgent;
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
  bool getDebuggerAttached() { return m_debuggerAttached; }
  // Should only be set by DebuggerHookHandler::attach
  void setDebuggerAttached(bool d) {
    m_debuggerAttached = d;
    updateJit();
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
  bool getDebuggerStepOut() const { return m_debuggerStepOut; }
  void setDebuggerStepOut(bool d) {
    m_debuggerStepOut = d;
    updateJit();
  }
  bool getDebuggerNext() const { return m_debuggerNext; }
  void setDebuggerNext(bool d) {
    m_debuggerNext = d;
    updateJit();
  }
  int getActiveLineBreak() const {
    return m_activeLineBreaks.size() == 0 ? -1 : m_activeLineBreaks.top();
  }
  void setActiveLineBreak(int line) {
    m_activeLineBreaks.top() = line;
    updateJit();
  }
  void popActiveLineBreak() {
    m_activeLineBreaks.pop();
    updateJit();
  }
  void pushActiveLineBreak(int line) {
    m_activeLineBreaks.push(line);
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
  std::string getUserAgent() { return m_userAgent; }
  void setUserAgent(std::string userAgent) { m_userAgent = userAgent; }
  std::vector<std::string> getAllowedDirectories() const {
    return m_allowedDirectories;
  }
  bool hasSafeFileAccess() const { return m_safeFileAccess; }
  bool hasTrackErrors() { return m_trackErrors; }

  std::stack<void *> interrupts;   // CmdInterrupts this thread's handling

  void reset();

  void setMemExceededFlag();
  void setTimedOutFlag();
  void clearTimedOutFlag();
  void setSignaledFlag();
  void setAsyncEventHookFlag();
  void clearAsyncEventHookFlag();
  void setDebuggerHookFlag();
  void clearDebuggerHookFlag();
  void setEventHookFlag();
  void clearEventHookFlag();
  void setPendingExceptionFlag();
  void clearPendingExceptionFlag();
  void setInterceptFlag();
  void clearInterceptFlag();
  void setXenonSignalFlag();
  void clearXenonSignalFlag();
  void setDebuggerSignalFlag();
  ssize_t fetchAndClearFlags();

  inline bool checkXenonSignalFlag() {
    return getConditionFlags()->load() & RequestInjectionData::XenonSignalFlag;
  }

  void onSessionInit();
};

//////////////////////////////////////////////////////////////////////

}

#endif
