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

#ifndef incl_HPHP_REQUEST_INJECTION_DATA_INL_H_
#error "Can only be included by request-injection-data.h"
#endif

namespace HPHP {
////////////////////////////////////////////////////////////////////////////////

inline int RequestInjectionData::getTimeout() const {
  return m_timer.m_timeoutSeconds;
}

inline int RequestInjectionData::getCPUTimeout() const {
  return m_cpuTimer.m_timeoutSeconds;
}

/*
 * Returns the amount of seconds until user callback is invoked
 */
inline int RequestInjectionData::getUserTimeout() const {
  return m_userTimeoutTimer.m_timeoutSeconds;
}

inline bool RequestInjectionData::getJit() const {
  return m_jit;
}

inline bool RequestInjectionData::isJittingDisabled() const {
  return m_jittingDisabled;
}

inline void RequestInjectionData::setJittingDisabled(bool flag) {
  m_jittingDisabled = flag;
}

inline bool RequestInjectionData::getJitFolding() const {
  return m_jitFolding;
}

inline void RequestInjectionData::setJitFolding(bool flag) {
  m_jitFolding = flag;
}

inline bool RequestInjectionData::getSuppressClassConversionNotices() const {
  return m_suppressClassConversionWarnings;
}

inline void RequestInjectionData::setSuppressClassConversionNotices(bool flag) {
  m_suppressClassConversionWarnings = flag;
}

inline bool RequestInjectionData::getCoverage() const {
  return m_coverage;
}

inline void RequestInjectionData::setCoverage(bool flag) {
  m_coverage = flag;
  updateJit();
}

inline bool RequestInjectionData::getDebuggerAttached() {
  return m_debuggerAttached;
}

inline void RequestInjectionData::setDebuggerAttached(bool flag) {
  m_debuggerAttached = flag;
  updateJit();
}

inline void RequestInjectionData::setVSDebugDisablesJit(bool flag) {
  m_vsdebugDisablesJit = flag;
  updateJit();
}

inline size_t RequestInjectionData::getDebuggerStackDepth() const {
  return m_activeLineBreaks.size();
}

inline bool RequestInjectionData::getDebuggerForceIntr() const {
  return
    m_debuggerIntr ||
    // Force interrupts when over an active line.
    getActiveLineBreak() != -1 ||
    // Interrupts forced while stepping in.
    m_debuggerStepIn ||
    // Step out forces interrupts after we have exited the function.
    m_debuggerStepOut == StepOutState::Out;
}

inline bool RequestInjectionData::getDebuggerStepIntr() const {
  return m_debuggerStepIn || m_debuggerStepOut == StepOutState::Out;
}

inline void RequestInjectionData::setDebuggerIntr(bool flag) {
  m_debuggerIntr = flag;
}

inline bool RequestInjectionData::getDebuggerStepIn() const {
  return m_debuggerStepIn;
}

inline void RequestInjectionData::setDebuggerStepIn(bool flag) {
  m_debuggerStepIn = flag;
}

inline RequestInjectionData::StepOutState
RequestInjectionData::getDebuggerStepOut() const {
  return m_debuggerStepOut;
}

inline void RequestInjectionData::setDebuggerStepOut(StepOutState state) {
  m_debuggerStepOut = state;
}

inline bool RequestInjectionData::getDebuggerNext() const {
  return m_debuggerNext;
}

inline void RequestInjectionData::setDebuggerNext(bool flag) {
  m_debuggerNext = flag;
}

inline int RequestInjectionData::getDebuggerFlowDepth() const {
  return m_debuggerFlowDepth;
}

inline void RequestInjectionData::setDebuggerFlowDepth(int depth) {
  m_debuggerFlowDepth = depth;
}

inline void RequestInjectionData::clearPCFilters() {
  m_breakPointFilter.clear();
  m_flowFilter.clear();
  m_lineBreakPointFilter.clear();
  m_callBreakPointFilter.clear();
  m_retBreakPointFilter.clear();
}

inline int RequestInjectionData::getActiveLineBreak() const {
  return m_activeLineBreaks.empty() ? -1 : m_activeLineBreaks.top();
}

inline void RequestInjectionData::clearActiveLineBreak() {
  if (m_activeLineBreaks.empty()) return;
  m_activeLineBreaks.top() = -1;
}

inline void RequestInjectionData::setActiveLineBreak(int line) {
  assertx(line != -1);
  if (m_activeLineBreaks.empty()) return;
  m_activeLineBreaks.top() = line;
}

inline void RequestInjectionData::popActiveLineBreak() {
  if (m_activeLineBreaks.empty()) return;
  m_activeLineBreaks.pop();
}

inline void RequestInjectionData::pushActiveLineBreak() {
  m_activeLineBreaks.push(-1);
}

inline const std::vector<std::string>&
RequestInjectionData::getIncludePaths() const {
  return m_include_paths;
}

inline const std::string& RequestInjectionData::getDefaultMimeType() const {
  return m_defaultMimeType;
}

inline int64_t RequestInjectionData::getErrorReportingLevel() {
  return m_errorReportingLevel;
}

inline void RequestInjectionData::setErrorReportingLevel(int64_t level) {
  m_errorReportingLevel = level;
}

inline int64_t RequestInjectionData::getMemoryLimitNumeric() const {
  return m_maxMemoryNumeric;
}

inline int64_t RequestInjectionData::getSocketDefaultTimeout() const {
  return m_socketDefaultTimeout;
}

inline const std::string& RequestInjectionData::getUserAgent() const {
  return m_userAgent;
}

inline void RequestInjectionData::setUserAgent(const std::string& agent) {
  m_userAgent = agent;
}

inline const std::string& RequestInjectionData::getTimezone() const {
  return m_timezone;
}

inline void RequestInjectionData::setTimezone(const std::string& tz) {
  m_timezone = tz;
}

inline void RequestInjectionData::setSafeFileAccess(bool b) {
  m_safeFileAccess = b;
}

inline bool RequestInjectionData::hasSafeFileAccess() const {
  return m_safeFileAccess;
}

inline bool RequestInjectionData::hasHtmlErrors() const {
  return m_htmlErrors;
}

inline bool RequestInjectionData::logFunctionCalls() const {
  return m_logFunctionCalls;
}
////////////////////////////////////////////////////////////////////////////////
}
