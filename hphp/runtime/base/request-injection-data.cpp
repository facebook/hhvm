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

#include "hphp/runtime/base/request-injection-data.h"

#include <atomic>
#include <cinttypes>
#include <string>
#include <limits>
#include <filesystem>

#include <signal.h>

#include <folly/Random.h>
#include <folly/portability/SysTime.h>

#include "hphp/util/logger.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/configs/jit.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/asio/ext_waitable-wait-handle.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void RequestTimer::onTimeout() {
  m_reqInjectionData->onTimeout(this);
}

RequestTimer::RequestTimer(RequestInjectionData* data, clockid_t clockType)
    : m_reqInjectionData(data)
    , m_clockType(clockType)
    , m_hasTimer(false)
    , m_timerActive(false)
{}

RequestTimer::~RequestTimer() {
  if (m_hasTimer) {
    timer_delete(m_timerId);
  }
}

/*
 * NB: this function must be nothrow when `seconds' is zero.  RPCRequestHandler
 * makes use of this.
 */
void RequestTimer::setTimeout(int seconds) {
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
    if (timer_create(m_clockType, &sev, &m_timerId)) {
      raise_error("Failed to set timeout: %s", folly::errnoStr(errno).c_str());
    }
    m_hasTimer = true;
  }

  /*
   * There is a potential race here. Callers want to assume that
   * if they cancel the timeout (seconds = 0), they *won't* get
   * a signal after they call this (although they may get a signal
   * during the call).
   * So we need to clear the timeout, wait (if necessary) for a
   * pending signal to be handled, and then set the new timeout
   */
  itimerspec ts = {};
  itimerspec old;
  timer_settime(m_timerId, 0, &ts, &old);
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
    timer_settime(m_timerId, 0, &ts, nullptr);
  } else {
    m_timerActive.store(false, std::memory_order_relaxed);
  }
}

int RequestTimer::getRemainingTime() const {
  if (m_hasTimer) {
    itimerspec ts;
    if (!timer_gettime(m_timerId, &ts)) {
      int remaining = ts.it_value.tv_sec;
      return remaining > 1 ? remaining : 1;
    }
  }
  return m_timeoutSeconds;
}

//////////////////////////////////////////////////////////////////////

bool RequestInjectionData::setAllowedDirectories(const std::string& value) {
  // Backwards compat with ;
  // but moving forward should use PATH_SEPARATOR
  std::vector<std::string> boom;
  if (value.find(";") != std::string::npos) {
    folly::split(';', value, boom, true);
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
    IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                     "max_execution_time", setAndGet);
    IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                     "maximum_execution_time", setAndGet);
    IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                     "hhvm.max_wall_time", setAndGetWall);
    IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                     "hhvm.max_cpu_time", setAndGetCPU);
  }

  // Resource Limits
  std::string mem_def = std::to_string(RuntimeOption::RequestMemoryMaxBytes);
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request, "memory_limit",
                   mem_def.c_str(),
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       setMemoryLimit(value);
                       return true;
                     },
                     nullptr,
                     &m_maxMemory));

  // Data Handling
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "default_charset", Cfg::Server::DefaultCharsetName.c_str(),
                   &m_defaultCharset);
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "default_mimetype", "text/html",
                   &m_defaultMimeType);

  // Paths and Directories
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
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
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
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
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "error_reporting",
                   std::to_string(RuntimeOption::RuntimeErrorReportingLevel)
                    .c_str(),
                   &m_errorReportingLevel);
  IniSetting::Bind(
    IniSetting::CORE,
    IniSetting::Mode::Request,
    "html_errors",
    IniSetting::SetAndGet<bool>(
      [&] (const bool& on) {
        m_htmlErrors = on;
        return true;
      },
      [&] () { return m_htmlErrors; },
      &m_htmlErrors
    )
  );

  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "log_errors",
                   IniSetting::SetAndGet<bool>(
                     [this](const bool& on) {
                       if (m_logErrors != on) {
                         if (on) {
                           if (!m_errorLog.empty()) {
                             Logger::SetThreadLog(m_errorLog.data(), true);
                           }
                         } else {
                           Logger::ClearThreadLog();
                         }
                       }
                       return true;
                     },
                     nullptr,
                     &m_logErrors
                   ));
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "error_log",
                   IniSetting::SetAndGet<std::string>(
                     [this](const std::string& value) {
                       if (m_logErrors && !value.empty()) {
                         Logger::SetThreadLog(value.data(), true);
                       }
                       return true;
                     },
                     nullptr,
                     &m_errorLog
                   ));

  // Filesystem and Streams Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "user_agent", "", &m_userAgent);
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "default_socket_timeout",
                   std::to_string(RuntimeOption::SocketDefaultTimeout).c_str(),
                   &m_socketDefaultTimeout);

  // Response handling.
  // TODO(T5601927): output_compression supports int values where the value
  // represents the output buffer size. Also need to add a
  // zlib.output_handler ini setting as well.
  // http://php.net/zlib.configuration.php
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "zlib.output_compression", &m_gzipCompression);
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "zlib.output_compression_level", &m_gzipCompressionLevel);

  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "brotli.chunked_compression", &m_brotliChunkedEnabled);
  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "brotli.compression", &m_brotliEnabled);
  IniSetting::Bind(
      IniSetting::CORE,
      IniSetting::Mode::Request,
      "brotli.compression_quality",
      std::to_string(RuntimeOption::BrotliCompressionQuality).c_str(),
      &m_brotliQuality);
  IniSetting::Bind(
      IniSetting::CORE,
      IniSetting::Mode::Request,
      "brotli.compression_lgwin",
      std::to_string(RuntimeOption::BrotliCompressionLgWindowSize).c_str(),
      &m_brotliLgWindowSize);

  IniSetting::Bind(IniSetting::CORE, IniSetting::Mode::Request,
                   "zstd.compression", &m_zstdEnabled);
  IniSetting::Bind(
      IniSetting::CORE,
      IniSetting::Mode::Request,
      "zstd.compression_level",
      std::to_string(RuntimeOption::ZstdCompressionLevel).c_str(),
      &m_zstdLevel);
  IniSetting::Bind(
      IniSetting::CORE,
      IniSetting::Mode::Request,
      "zstd.checksum_rate",
      std::to_string(RuntimeOption::ZstdChecksumRate).c_str(),
      &m_zstdChecksumRate);
  IniSetting::Bind(
      IniSetting::CORE,
      IniSetting::Mode::Request,
      "zstd.window_log",
      std::to_string(RuntimeOption::ZstdWindowLog).c_str(),
      &m_zstdWindowLog);
  IniSetting::Bind(
      IniSetting::CORE,
      IniSetting::Mode::Request,
      "zstd.target_block_size",
      std::to_string(RuntimeOption::ZstdTargetBlockSize).c_str(),
      &m_zstdTargetBlockSize);

  // TODO(20427335): Get rid of this illogical onSessionInit() call.
  onSessionInit();
}

std::string RequestInjectionData::getDefaultIncludePath() {
  std::string result;
  folly::join(":", RuntimeOption::IncludeSearchPaths, result);
  return result;
}

void RequestInjectionData::onSessionInit() {
  static auto open_basedir_val = []() -> Optional<std::string> {
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
  m_logFunctionCalls = RuntimeOption::EvalFunctionCallSampleRate > 0 &&
    folly::Random::rand32(
      RuntimeOption::EvalFunctionCallSampleRate
    ) == 0;
  reset();
}

void RequestInjectionData::onTimeout(RequestTimer* timer) {
  if (timer == &m_timer) {
    triggerTimeout(TimeoutTime);
    m_timer.m_timerActive.store(false, std::memory_order_relaxed);
  } else if (timer == &m_cpuTimer) {
    triggerTimeout(TimeoutCPUTime);
    m_cpuTimer.m_timerActive.store(false, std::memory_order_relaxed);
  } else if (timer == &m_userTimeoutTimer) {
    triggerTimeout(TimeoutSoft);
    m_userTimeoutTimer.m_timerActive.store(false, std::memory_order_relaxed);
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

void RequestInjectionData::setUserTimeout(int seconds) {
  if (seconds == 0) {
    m_userTimeoutTimer.m_timerActive.store(false, std::memory_order_relaxed);
  }

  m_userTimeoutTimer.setTimeout(seconds);
}

void RequestInjectionData::invokeUserTimeoutCallback(c_WaitableWaitHandle* wh) {
  clearTimeoutFlag(TimeoutSoft);
  if (!g_context->m_timeThresholdCallback.isNull()) {
    VMRegAnchor _;
    try {
      auto args = make_vec_array(Object { wh });
      vm_call_user_func(g_context->m_timeThresholdCallback, args);
    } catch (Object& ex) {
      raise_error("Uncaught exception escaping pre timeout callback: %s",
                  throwable_to_string(ex.get()).data());
    }
  }
}

void RequestInjectionData::triggerTimeout(TimeoutKindFlag kind) {
  // Add the flags. The surprise handling queries those in a certain order
  m_timeoutFlags.fetch_or(kind);
  setFlag(TimedOutFlag);
}

bool RequestInjectionData::checkTimeoutKind(TimeoutKindFlag kind) {
  return m_timeoutFlags.load() & kind;
}

/*
 * Clear the specific flag. If new timeout flags are 0, remove the surprise.
 */
void RequestInjectionData::clearTimeoutFlag(TimeoutKindFlag kind) {
  if (m_timeoutFlags.fetch_and(~kind) == kind) {
    clearFlag(TimedOutFlag);
  }
}

int RequestInjectionData::getRemainingTime() const {
  return m_timer.getRemainingTime();
}

int RequestInjectionData::getRemainingCPUTime() const {
  return m_cpuTimer.getRemainingTime();
}

int RequestInjectionData::getUserTimeoutRemainingTime() const {
  return m_userTimeoutTimer.getRemainingTime();
}

// Called on fatal error, PSP and hphp_invoke
void RequestInjectionData::resetTimers(int time_sec, int cputime_sec) {
  resetTimer(time_sec);
  resetCPUTimer(cputime_sec);

  // Keep the pre-timeout timer the same
  resetUserTimeoutTimer(0);
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
  clearTimeoutFlag(TimeoutTime);
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
  clearTimeoutFlag(TimeoutCPUTime);
}

void RequestInjectionData::resetUserTimeoutTimer(int seconds /* = 0 */) {
  if (seconds == 0) {
    seconds = getUserTimeout();
  } else if (seconds < 0) {
    if (!getUserTimeout()) return;
    seconds = -seconds;
    if (seconds < getUserTimeoutRemainingTime()) return;
  }
  setUserTimeout(seconds);
  clearTimeoutFlag(TimeoutSoft);
}

void RequestInjectionData::reset() {
  m_sflagsAndStkPtr->fetch_and(kSurpriseFlagStackMask);
  m_timeoutFlags.fetch_and(TimeoutNone);
  m_hostOutOfMemory.store(false, std::memory_order_relaxed);
  m_OOMAbort = false;
  m_coverage = RuntimeOption::RecordCodeCoverage;
  m_jittingDisabled = false;
  m_debuggerAttached = false;
  m_vsdebugDisablesJit = false;
  m_debuggerIntr = false;
  m_debuggerStepIn = false;
  m_debuggerStepOut = StepOutState::None;
  m_debuggerNext = false;
  m_suppressClassConversionWarnings = false;

  clearPCFilters();
  while (!m_activeLineBreaks.empty()) {
    m_activeLineBreaks.pop();
  }
  updateJit();
  while (!interrupts.empty()) interrupts.pop();
}

void RequestInjectionData::updateJit() {
  m_jit = Cfg::Jit::Enabled &&
    !(Cfg::Jit::DisabledByHphpd && m_debuggerAttached) &&
    !m_coverage &&
    (rl_typeProfileLocals.isNull() || !isForcedToInterpret()) &&
    !m_vsdebugDisablesJit;
}

void RequestInjectionData::clearFlag(SurpriseFlag flag) {
  assertx(flag >= 1ull << 48);
  m_sflagsAndStkPtr->fetch_and(~flag);
}

void RequestInjectionData::setFlag(SurpriseFlag flag) {
  assertx(flag >= 1ull << 48);
  m_sflagsAndStkPtr->fetch_or(flag);
}

void RequestInjectionData::sendSignal(int signum) {
  if (signum <= 0 || signum >= Process::kNSig) {
    Logger::Warning("%d is not a valid signal", signum);
    return;
  }
  const unsigned index = signum / 64;
  const unsigned offset = signum % 64;
  const uint64_t mask = 1ull << offset;
  m_signalMask[index].fetch_or(mask, std::memory_order_release);
  setFlag(SignaledFlag);
}

int RequestInjectionData::getAndClearNextPendingSignal() {
  // We cannot look at the surprise flag because it may have already been
  // cleared in handle_request_surprise().
  for (unsigned i = 0; i < m_signalMask.size(); ++i) {
    auto& chunk = m_signalMask[i];
    if (auto value = chunk.load(std::memory_order_acquire)) {
      unsigned index = folly::findFirstSet(value);
      assertx(index);
      --index;             // folly::findFirstSet() returns 1-64 instead of 0-63
      // Clear the bit.
      chunk.fetch_and(~(1ull << index), std::memory_order_relaxed);
      return i * 64 + index;
    }
  }
  return 0;                             // no pending signal
}

void RequestInjectionData::setMemoryLimit(folly::StringPiece limit) {
  int64_t newInt = strtoll(limit.begin(), nullptr, 10);
  if (newInt <= 0) {
    newInt = std::numeric_limits<int64_t>::max();
    m_maxMemory = std::to_string(newInt);
  } else {
    m_maxMemory = limit.str();
    newInt = convert_bytes_to_long(limit);
    if (newInt <= 0) {
      newInt = std::numeric_limits<int64_t>::max();
    }
  }
  tl_heap->setMemoryLimit(newInt);
  m_maxMemoryNumeric = newInt;
}

}
