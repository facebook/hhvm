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
#include "hphp/runtime/base/request-injection-data.h"

#include <atomic>
#include <cinttypes>
#include <string>
#include <limits>

#include <sys/time.h>
#include <signal.h>

#include "hphp/util/logger.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/ext_string.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

const StaticString s_dot(".");

//////////////////////////////////////////////////////////////////////

RequestInjectionData::~RequestInjectionData() {
#ifndef __APPLE__
  if (m_hasTimer) {
    timer_delete(m_timer_id);
  }
#endif
}

void RequestInjectionData::threadInit() {
  // phpinfo
  {
    auto setAndGet = IniSetting::SetAndGet<int64_t>(
      [this](const int64_t &limit) {
        setTimeout(limit);
        return true;
      },
      [this] { return getTimeout(); }
    );
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                     "max_execution_time", setAndGet);
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                     "maximum_execution_time", setAndGet);
  }

  // Resource Limits
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL, "memory_limit",
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       int64_t newInt = strtoll(value.c_str(), nullptr, 10);
                       if (newInt <= 0) {
                         newInt = std::numeric_limits<int64_t>::max();
                         m_maxMemory = std::to_string(newInt);
                       } else {
                         m_maxMemory = value;
                         newInt = convert_bytes_to_long(value);
                         if (newInt <= 0) {
                           newInt = std::numeric_limits<int64_t>::max();
                         }
                       }
                       MM().getStatsNoRefresh().maxBytes = newInt;
                       return true;
                     },
                     nullptr
                   ), &m_maxMemory);

  // Data Handling
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "arg_separator.output", "&",
                   &m_argSeparatorOutput);
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
                       auto paths = f_explode(":", value);
                       m_include_paths.clear();
                       for (ArrayIter iter(paths); iter; ++iter) {
                         m_include_paths.push_back(
                           iter.second().toString().toCppString());
                       }
                       return true;
                     },
                     [this]() {
                       std::string ret;
                       bool first = true;
                       for (auto &path : m_include_paths) {
                         if (first) {
                           first = false;
                         } else {
                           ret += ":";
                         }
                         ret += path;
                       }
                       return ret;
                     }
                   ));

  // Paths and Directories
  m_allowedDirectories = RuntimeOption::AllowedDirectories;
  m_safeFileAccess = RuntimeOption::SafeFileAccess;
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "open_basedir",
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       auto boom = f_explode(";", value).toCArrRef();

                       std::vector<std::string> directories;
                       directories.reserve(boom.size());
                       for (ArrayIter iter(boom); iter; ++iter) {
                         const auto& path = iter.second().toString();

                         // Canonicalise the path
                         if (!path.empty() &&
                             File::TranslatePathKeepRelative(path).empty()) {
                           return false;
                         }

                         if (path.equal(s_dot)) {
                           auto cwd = g_context->getCwd().toCppString();
                           directories.push_back(cwd);
                         } else {
                           directories.push_back(path.toCppString());
                         }
                       }
                       m_allowedDirectories = directories;
                       m_safeFileAccess = !boom.empty();
                       return true;
                     },
                     [this]() -> std::string {
                       if (!hasSafeFileAccess()) {
                         return "";
                       }

                       std::string out;
                       for (auto& directory: getAllowedDirectories()) {
                         if (!directory.empty()) {
                           out += directory + ";";
                         }
                       }

                       // Remove the trailing ;
                       if (!out.empty()) {
                         out.erase(std::end(out) - 1, std::end(out));
                       }
                       return out;
                     }
                   ));

  // Errors and Logging Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "error_reporting",
                   std::to_string(RuntimeOption::RuntimeErrorReportingLevel)
                    .c_str(),
                   &m_errorReportingLevel);
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
                   "default_socket_timeout",
                   std::to_string(RuntimeOption::SocketDefaultTimeout).c_str(),
                   &m_socketDefaultTimeout);
}


std::string RequestInjectionData::getDefaultIncludePath() {
  auto include_paths = Array::Create();
  for (unsigned int i = 0; i < RuntimeOption::IncludeSearchPaths.size(); ++i) {
    include_paths.append(String(RuntimeOption::IncludeSearchPaths[i]));
  }
  return f_implode(":", include_paths).toCppString();
}

void RequestInjectionData::onSessionInit() {
  RDS::requestInit();
  cflagsPtr = &RDS::header()->conditionFlags;
  reset();
}

void RequestInjectionData::onTimeout() {
  setTimedOutFlag();
  m_timerActive.store(false, std::memory_order_relaxed);
}

void RequestInjectionData::setTimeout(int seconds) {
#ifndef __APPLE__
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
    auto const& clockType =
      RuntimeOption::TimeoutsUseWallTime ? CLOCK_REALTIME :
                                           CLOCK_THREAD_CPUTIME_ID;
    if (timer_create(clockType, &sev, &m_timer_id)) {
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

int RequestInjectionData::getRemainingTime() const {
#ifndef __APPLE__
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

/*
 * If seconds == 0, reset the timeout to the last one set
 * If seconds  < 0, set the timeout to -seconds if there's less than
 *                  -seconds remaining.
 * If seconds  > 0, set the timeout to seconds.
 */
void RequestInjectionData::resetTimer(int seconds /* = 0 */) {
  auto data = &ThreadInfo::s_threadInfo->m_reqInjectionData;
  if (seconds == 0) {
    seconds = data->getTimeout();
  } else if (seconds < 0) {
    if (!data->getTimeout()) return;
    seconds = -seconds;
    if (seconds < data->getRemainingTime()) return;
  }
  data->setTimeout(seconds);
  data->clearTimedOutFlag();
}

void RequestInjectionData::reset() {
  getConditionFlags()->store(0);
  m_coverage = RuntimeOption::RecordCodeCoverage;
  m_debugger = false;
  m_debuggerIntr = false;
  updateJit();
  while (!interrupts.empty()) interrupts.pop();
}

void RequestInjectionData::updateJit() {
  m_jit = RuntimeOption::EvalJit &&
    !(RuntimeOption::EvalJitDisabledByHphpd && m_debugger) &&
    !m_debuggerIntr &&
    !m_coverage &&
    !shouldProfile();
}

void RequestInjectionData::setMemExceededFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::MemExceededFlag);
}

void RequestInjectionData::setTimedOutFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::TimedOutFlag);
}

void RequestInjectionData::clearTimedOutFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::TimedOutFlag);
}

void RequestInjectionData::setSignaledFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::SignaledFlag);
}

void RequestInjectionData::setEventHookFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::EventHookFlag);
}

void RequestInjectionData::clearEventHookFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::EventHookFlag);
}

void RequestInjectionData::setPendingExceptionFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::PendingExceptionFlag);
}

void RequestInjectionData::clearPendingExceptionFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::PendingExceptionFlag);
}

void RequestInjectionData::setInterceptFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::InterceptFlag);
}

void RequestInjectionData::clearInterceptFlag() {
  getConditionFlags()->fetch_and(~RequestInjectionData::InterceptFlag);
}

void RequestInjectionData::setDebuggerSignalFlag() {
  getConditionFlags()->fetch_or(RequestInjectionData::DebuggerSignalFlag);
}

ssize_t RequestInjectionData::fetchAndClearFlags() {
  return getConditionFlags()->fetch_and(RequestInjectionData::EventHookFlag |
                                        RequestInjectionData::InterceptFlag);
}

}
