/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/request-injection-data.h"

#include <atomic>
#include <cinttypes>
#include <string>
#include <limits>

#include <sys/time.h>
#include <signal.h>

#include <boost/filesystem.hpp>
#include <folly/Optional.h>

#include "hphp/util/logger.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

RequestTimer::RequestTimer(RequestInjectionData* data, clockid_t clockType)
    : m_reqInjectionData(data)
#if !defined(__APPLE__) && !defined(_MSC_VER)
    , m_clockType(clockType)
    , m_hasTimer(false)
#endif
    , m_timeoutSeconds(0)  // no timeout by default
    , m_timerActive(false)
{}

RequestTimer::~RequestTimer() {
#if !defined(__APPLE__) && !defined(_MSC_VER)
  if (m_hasTimer) {
    timer_delete(m_timer_id);
  }
#endif
}

/*
 * NB: this function must be nothrow when `seconds' is zero.  RPCRequestHandler
 * makes use of this.
 */
void RequestTimer::setTimeout(int seconds) {
#if !defined(__APPLE__) && !defined(_MSC_VER)
  m_timeoutSeconds = seconds > 0 ? seconds : 0;
  if (!m_hasTimer) {
    if (!m_timeoutSeconds) {
      // we don't have a timer, and we don't have a timeout
      return;
    }
    sigevent sev;
    memset(&sev, 0, sizeof(sev));
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGVTALRM;
    sev.sigev_value.sival_ptr = this;
    if (timer_create(m_clockType, &sev, &m_timer_id)) {
      raise_error("Failed to set timeout: %s", folly::errnoStr(errno).c_str());
    }
    m_hasTimer = true;
  }

  /*
   * There is a potential race here. Callers want to assume that
   * if they cancel the timeout (seconds = 0), they *wont* get
   * a signal after they call this (although they may get a signal
   * during the call).
   * So we need to clear the timeout, wait (if necessary) for a
   * pending signal to be handled, and then set the new timeout
   */
  itimerspec ts = {};
  itimerspec old;
  timer_settime(m_timer_id, 0, &ts, &old);
  if (!old.it_value.tv_sec && !old.it_value.tv_nsec) {
    // the timer has gone off...
    if (m_timerActive.load(std::memory_order_acquire)) {
      // but m_timerActive is still set, so we haven't processed
      // the signal yet.
      // spin until its done.
      while (m_timerActive.load(std::memory_order_relaxed)) {
      }
    }
  }
  if (m_timeoutSeconds) {
    m_timerActive.store(true, std::memory_order_relaxed);
    ts.it_value.tv_sec = m_timeoutSeconds;
    timer_settime(m_timer_id, 0, &ts, nullptr);
  } else {
    m_timerActive.store(false, std::memory_order_relaxed);
  }
#endif
}

void RequestTimer::onTimeout() {
  m_reqInjectionData->onTimeout(this);
}

int RequestTimer::getRemainingTime() const {
#if !defined(__APPLE__) && !defined(_MSC_VER)
  if (m_hasTimer) {
    itimerspec ts;
    if (!timer_gettime(m_timer_id, &ts)) {
      int remaining = ts.it_value.tv_sec;
      return remaining > 1 ? remaining : 1;
    }
  }
#endif
  return m_timeoutSeconds;
}

//////////////////////////////////////////////////////////////////////

bool RequestInjectionData::setAllowedDirectories(const std::string& value) {
  // Backwards compat with ;
  // but moving forward should use PATH_SEPARATOR
  std::vector<std::string> boom;
  if (value.find(";") != std::string::npos) {
    folly::split(";", value, boom, true);
    m_open_basedir_separator = ";";
  } else {
    m_open_basedir_separator =
      s_PATH_SEPARATOR.toCppString();
    folly::split(m_open_basedir_separator, value, boom,
                 true);
  }

  if (boom.empty() && m_safeFileAccess) return false;
  for (auto& path : boom) {
    // Canonicalise the path
    if (!path.empty() &&
        File::TranslatePathKeepRelative(path).empty()) {
      return false;
    }

    if (path == ".") {
      path = g_context->getCwd().toCppString();
    }
  }
  m_safeFileAccess = !boom.empty();
  std::string dirs;
  folly::join(m_open_basedir_separator, boom, dirs);
  VirtualHost::SortAllowedDirectories(boom);
  m_allowedDirectoriesInfo.reset(
    new AllowedDirectoriesInfo(std::move(boom), std::move(dirs)));
  return true;
}

const std::vector<std::string>&
RequestInjectionData::getAllowedDirectoriesProcessed() const {
  return m_allowedDirectoriesInfo ?
    m_allowedDirectoriesInfo->vec : VirtualHost::GetAllowedDirectories();
}

void RequestInjectionData::threadInit() {
  // phpinfo
  {
    auto setAndGetWall = IniSetting::SetAndGet<int64_t>(
      [this](const int64_t &limit) {
        setTimeout(limit);
        return true;
      },
      [this] { return getTimeout(); }
    );
    auto setAndGetCPU = IniSetting::SetAndGet<int64_t>(
      [this](const int64_t &limit) {
        setCPUTimeout(limit);
        return true;
      },
      [this] { return getCPUTimeout(); }
    );
    auto setAndGet = RuntimeOption::TimeoutsUseWallTime
      ? setAndGetWall : setAndGetCPU;
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                     "max_execution_time", setAndGet);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                     "maximum_execution_time", setAndGet);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                     "hhvm.max_wall_time", setAndGetWall);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                     "hhvm.max_cpu_time", setAndGetCPU);
  }

  // Resource Limits
  std::string mem_def = std::to_string(RuntimeOption::RequestMemoryMaxBytes);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL, "memory_limit",
                   mem_def.c_str(),
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       setMemoryLimit(value);
                       return true;
                     },
                     nullptr
                   ), &m_maxMemory);

  // Data Handling
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "arg_separator.output", "&",
                   &m_argSeparatorOutput);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "arg_separator.input", "&",
                   &m_argSeparatorInput);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "variables_order", "EGPCS",
                   &m_variablesOrder);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "request_order", "",
                   &m_requestOrder);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "default_charset", RuntimeOption::DefaultCharsetName.c_str(),
                   &m_defaultCharset);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "default_mimetype", "text/html",
                   &m_defaultMimeType);

  // Paths and Directories
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "include_path", getDefaultIncludePath().c_str(),
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       m_include_paths.clear();
                       int pos = value.find(':');
                       if (pos < 0) {
                         m_include_paths.push_back(value);
                       } else {
                         int pos0 = 0;
                         do {
                           // Check for stream wrapper
                           if (value.length() > pos + 2 &&
                               value[pos + 1] == '/' &&
                               value[pos + 2] == '/') {
                             // .:// or ..:// is not stream wrapper
                             if (((pos - pos0) >= 1 && value[pos - 1] != '.') ||
                                 ((pos - pos0) >= 2 && value[pos - 2] != '.') ||
                                 (pos - pos0) > 2) {
                               pos += 3;
                               continue;
                             }
                           }
                           m_include_paths.push_back(
                             value.substr(pos0, pos - pos0));
                           pos++;
                           pos0 = pos;
                         } while ((pos = value.find(':', pos)) >= 0);

                         if (pos0 <= value.length()) {
                           m_include_paths.push_back(
                             value.substr(pos0));
                         }
                       }
                       return true;
                     },
                     [this]() {
                       std::string ret;
                       folly::join(":", m_include_paths, ret);
                       return ret;
                     }
                   ));

  // Paths and Directories
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "open_basedir",
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       return setAllowedDirectories(value);
                     },
                     [this]() -> std::string {
                       if (!hasSafeFileAccess()) {
                         return "";
                       }
                       if (m_allowedDirectoriesInfo) {
                         return m_allowedDirectoriesInfo->string;
                       }
                       std::string ret;
                       folly::join(m_open_basedir_separator,
                                   getAllowedDirectoriesProcessed(),
                                   ret);
                       return ret;
                     }
                   ));

  // Errors and Logging Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "error_reporting",
                   std::to_string(RuntimeOption::RuntimeErrorReportingLevel)
                    .c_str(),
                   &m_errorReportingLevel);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "track_errors", "0",
                   &m_trackErrors);
  IniSetting::Bind(
    IniSetting::CORE,
    IniSetting::PHP_INI_ALL,
    "html_errors",
    IniSetting::SetAndGet<bool>(
      [&] (const bool& on) {
        m_htmlErrors = on;
        return true;
      },
      [&] () { return m_htmlErrors; }
    ),
    &m_htmlErrors
  );

  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "log_errors",
                   IniSetting::SetAndGet<bool>(
                     [this](const bool& on) {
                       if (m_logErrors != on) {
                         if (on) {
                           if (!m_errorLog.empty()) {
                             FILE *output = fopen(m_errorLog.data(), "a");
                             if (output) {
                               Logger::SetNewOutput(output);
                             }
                           }
                         } else {
                           Logger::SetNewOutput(nullptr);
                         }
                       }
                       return true;
                     },
                     nullptr
                   ),
                   &m_logErrors);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "error_log",
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       if (m_logErrors && !m_errorLog.empty()) {
                         FILE *output = fopen(m_errorLog.data(), "a");
                         if (output) {
                           Logger::SetNewOutput(output);
                         }
                       }
                       return true;
                     },
                     nullptr
                   ), &m_errorLog);

  // Filesystem and Streams Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "user_agent", "", &m_userAgent);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "default_socket_timeout",
                   std::to_string(RuntimeOption::SocketDefaultTimeout).c_str(),
                   &m_socketDefaultTimeout);

  // Response handling.
  // TODO(T5601927): output_compression supports int values where the value
  // represents the output buffer size. Also need to add a
  // zlib.output_handler ini setting as well.
  // http://docs.hhvm.com/zlib.configuration.php
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "zlib.output_compression", &m_gzipCompression);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "zlib.output_compression_level", &m_gzipCompressionLevel);
}

std::string RequestInjectionData::getDefaultIncludePath() {
  std::string result;
  folly::join(":", RuntimeOption::IncludeSearchPaths, result);
  return result;
}

void RequestInjectionData::onSessionInit() {
  static auto open_basedir_val = []() -> folly::Optional<std::string> {
    Variant v;
    if (IniSetting::GetSystem("open_basedir", v)) {
      return { v.toString().toCppString() };
    }
    return {};
  }();

  rds::requestInit();
  m_sflagsAndStkPtr = &rds::header()->stackLimitAndSurprise;
  m_allowedDirectoriesInfo.reset();
  m_open_basedir_separator = s_PATH_SEPARATOR.toCppString();
  m_safeFileAccess = RuntimeOption::SafeFileAccess;
  if (open_basedir_val) {
    setAllowedDirectories(*open_basedir_val);
  }
  reset();
}

void RequestInjectionData::onTimeout(RequestTimer* timer) {
  if (timer == &m_timer) {
    setFlag(TimedOutFlag);
    m_timer.m_timerActive.store(false, std::memory_order_relaxed);
  } else if (timer == &m_cpuTimer) {
    setFlag(CPUTimedOutFlag);
    m_cpuTimer.m_timerActive.store(false, std::memory_order_relaxed);
  } else {
    always_assert(false && "Unknown timer fired");
  }
}

void RequestInjectionData::setTimeout(int seconds) {
  m_timer.setTimeout(seconds);
}

void RequestInjectionData::setCPUTimeout(int seconds) {
  m_cpuTimer.setTimeout(seconds);
}

int RequestInjectionData::getRemainingTime() const {
  return m_timer.getRemainingTime();
}

int RequestInjectionData::getRemainingCPUTime() const {
  return m_cpuTimer.getRemainingTime();
}

/*
 * If seconds == 0, reset the timeout to the last one set
 * If seconds  < 0, set the timeout to -seconds if there's less than
 *                  -seconds remaining.
 * If seconds  > 0, set the timeout to seconds.
 */
void RequestInjectionData::resetTimer(int seconds /* = 0 */) {
  if (seconds == 0) {
    seconds = getTimeout();
  } else if (seconds < 0) {
    if (!getTimeout()) return;
    seconds = -seconds;
    if (seconds < getRemainingTime()) return;
  }
  setTimeout(seconds);
  clearFlag(TimedOutFlag);
}

void RequestInjectionData::resetCPUTimer(int seconds /* = 0 */) {
  if (seconds == 0) {
    seconds = getCPUTimeout();
  } else if (seconds < 0) {
    if (!getCPUTimeout()) return;
    seconds = -seconds;
    if (seconds < getRemainingCPUTime()) return;
  }
  setCPUTimeout(seconds);
  clearFlag(CPUTimedOutFlag);
}

void RequestInjectionData::reset() {
  m_sflagsAndStkPtr->fetch_and(kSurpriseFlagStackMask);
  m_coverage = RuntimeOption::RecordCodeCoverage;
  m_debuggerAttached = false;
  m_debuggerIntr = false;
  m_debuggerStepIn = false;
  m_debuggerStepOut = StepOutState::NONE;
  m_debuggerNext = false;
  m_breakPointFilter.clear();
  m_flowFilter.clear();
  m_lineBreakPointFilter.clear();
  m_callBreakPointFilter.clear();
  m_retBreakPointFilter.clear();
  while (!m_activeLineBreaks.empty()) {
    m_activeLineBreaks.pop();
  }
  updateJit();
  while (!interrupts.empty()) interrupts.pop();
}

void RequestInjectionData::updateJit() {
  m_jit = RuntimeOption::EvalJit &&
    !(RuntimeOption::EvalJitDisabledByHphpd && m_debuggerAttached) &&
    !m_coverage &&
    isStandardRequest() &&
    !getDebuggerForceIntr();
}

void RequestInjectionData::clearFlag(SurpriseFlag flag) {
  assert(flag >= 1ull << 48);
  m_sflagsAndStkPtr->fetch_and(~flag);
}

void RequestInjectionData::setFlag(SurpriseFlag flag) {
  assert(flag >= 1ull << 48);
  m_sflagsAndStkPtr->fetch_or(flag);
}

void RequestInjectionData::setMemoryLimit(std::string limit) {
  int64_t newInt = strtoll(limit.c_str(), nullptr, 10);
  if (newInt <= 0) {
   newInt = std::numeric_limits<int64_t>::max();
   m_maxMemory = std::to_string(newInt);
  } else {
   m_maxMemory = limit;
   newInt = convert_bytes_to_long(limit);
   if (newInt <= 0) {
     newInt = std::numeric_limits<int64_t>::max();
   }
  }
  MM().getStatsNoRefresh().maxBytes = newInt;
  m_maxMemoryNumeric = newInt;
}
}
