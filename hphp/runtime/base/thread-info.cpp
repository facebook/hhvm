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

#include <atomic>

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <signal.h>

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/code-coverage.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/util/lock.h"
#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "folly/String.h"

using std::map;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex s_thread_info_mutex;
static std::set<ThreadInfo*> s_thread_infos;

__thread char* ThreadInfo::t_stackbase = 0;

IMPLEMENT_THREAD_LOCAL_NO_CHECK(ThreadInfo, ThreadInfo::s_threadInfo);

static std::string ini_get_max_execution_time(void*) {
  int64_t timeout = ThreadInfo::s_threadInfo.getNoCheck()->
    m_reqInjectionData.getTimeout();
  return std::to_string(timeout);
}

static bool ini_on_update_max_execution_time(const String& value, void*) {
  int64_t limit = value.toInt64();
  ThreadInfo::s_threadInfo.getNoCheck()->
    m_reqInjectionData.setTimeout(limit);
  return true;
}

ThreadInfo::ThreadInfo()
    : m_stacklimit(0), m_executing(Idling) {
  assert(!t_stackbase);
  t_stackbase = static_cast<char*>(stack_top_ptr());

  m_mm = &MM();

  m_profiler = nullptr;
  m_pendingException = nullptr;
  m_coverage = new CodeCoverage();

  RDS::threadInit();
  onSessionInit();

  Lock lock(s_thread_info_mutex);
  s_thread_infos.insert(this);
}

ThreadInfo::~ThreadInfo() {
  t_stackbase = 0;

  Lock lock(s_thread_info_mutex);
  s_thread_infos.erase(this);
  delete m_coverage;
  RDS::threadExit();
}

bool ThreadInfo::valid(ThreadInfo* info) {
  Lock lock(s_thread_info_mutex);
  return s_thread_infos.find(info) != s_thread_infos.end();
}

void ThreadInfo::GetExecutionSamples(std::map<Executing, int> &counts) {
  Lock lock(s_thread_info_mutex);
  for (std::set<ThreadInfo*>::const_iterator iter = s_thread_infos.begin();
       iter != s_thread_infos.end(); ++iter) {
    ++counts[(*iter)->m_executing];
  }
}

void ThreadInfo::onSessionInit() {
  m_reqInjectionData.onSessionInit();

  // Take the address of the cached per-thread stackLimit, and use this to allow
  // some slack for (a) stack usage above the caller of reset() and (b) stack
  // usage after the position gets checked.
  // If we're not in a threaded environment, then s_stackSize will be
  // zero. Use getrlimit to figure out what the size of the stack is to
  // calculate an approximation of where the bottom of the stack should be.
  if (s_stackSize == 0) {
    struct rlimit rl;

    getrlimit(RLIMIT_STACK, &rl);
    m_stacklimit = t_stackbase - (rl.rlim_cur - StackSlack);
  } else {
    m_stacklimit = (char *)s_stackLimit + StackSlack;
    assert(uintptr_t(m_stacklimit) < s_stackLimit + s_stackSize);
  }
}

void ThreadInfo::clearPendingException() {
  m_reqInjectionData.clearPendingExceptionFlag();
  if (m_pendingException != nullptr) delete m_pendingException;
  m_pendingException = nullptr;
}

void ThreadInfo::setPendingException(Exception* e) {
  m_reqInjectionData.setPendingExceptionFlag();
  if (m_pendingException != nullptr) delete m_pendingException;
  m_pendingException = e;
}

void ThreadInfo::onSessionExit() {
  m_reqInjectionData.setTimeout(0);
  m_reqInjectionData.reset();
  RDS::requestExit();
}

RequestInjectionData::~RequestInjectionData() {
#ifndef __APPLE__
  if (m_hasTimer) {
    timer_delete(m_timer_id);
  }
#endif
}

void RequestInjectionData::threadInit() {
  // Language and Misc Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY, "expose_php",
                   &RuntimeOption::ExposeHPHP);

  // Resource Limits
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL, "memory_limit",
                   [this](const std::string& value, void* p) {
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
                   ini_get_stdstring,
                   &m_maxMemory);

  // Data Handling
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "arg_separator.output", "&",
                   &m_argSeparatorOutput);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "post_max_size",
                   ini_on_update_long,
                   [](void*) {
                     return std::to_string(VirtualHost::GetMaxPostSize());
                   },
                   &RuntimeOption::MaxPostSize);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "default_charset", RuntimeOption::DefaultCharsetName.c_str(),
                   &m_defaultCharset);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "always_populate_raw_post_data",
                   &RuntimeOption::AlwaysPopulateRawPostData);

  // Paths and Directories
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "include_path", getDefaultIncludePath().c_str(),
                   [this](const std::string& value, void* p) {
                     auto paths = f_explode(":", value);
                     m_include_paths.clear();
                     for (ArrayIter iter(paths); iter; ++iter) {
                       m_include_paths.push_back(
                         iter.second().toString().toCppString());
                     }
                     return true;
                   },
                   [this](void*) {
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
                   });
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "doc_root", &RuntimeOption::SourceRoot);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "open_basedir",
                   [](const std::string& value, void* p) {
                     RuntimeOption::AllowedDirectories.clear();
                     auto boom = f_explode(";", value).toCArrRef();
                     for (ArrayIter iter(boom); iter; ++iter) {
                       RuntimeOption::AllowedDirectories.push_back(
                         iter.second().toCStrRef().toCppString()
                       );
                     }
                     return true;
                   },
                   [](void*) {
                     std::string out = "";
                     for (auto& dir : RuntimeOption::AllowedDirectories) {
                       if (!dir.empty()) {
                         out += dir + ";";
                       }
                     }
                     return out;
                   });

  // FastCGI
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY,
                   "pid", &RuntimeOption::PidFile);

  // File Uploads
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "file_uploads", "true",
                   &RuntimeOption::EnableFileUploads);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "upload_tmp_dir", &RuntimeOption::UploadTmpDir);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "upload_max_filesize",
                   ini_on_update_long,
                   [](void*) {
                     int uploadMaxFilesize =
                       VirtualHost::GetUploadMaxFileSize() / (1 << 20);
                     return std::to_string(uploadMaxFilesize) + "M";
                   },
                   &RuntimeOption::UploadMaxFileSize);

  // Errors and Logging Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "error_reporting",
                   std::to_string(RuntimeOption::RuntimeErrorReportingLevel).
                     c_str(),
                   &m_errorReportingLevel);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "log_errors", "false",
                   [this](const std::string& value, void* p) {
                     bool on;
                     ini_on_update_bool(value, &on);
                     if (m_logErrors != on) {
                       m_logErrors = on;
                       if (m_logErrors) {
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
                   ini_get_bool,
                   &m_logErrors);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "error_log",
                   [this](const std::string& value, void* p) {
                     m_errorLog = value;
                     if (m_logErrors && !m_errorLog.empty()) {
                       FILE *output = fopen(m_errorLog.data(), "a");
                       if (output) {
                         Logger::SetNewOutput(output);
                       }
                     }
                     return true;
                   },
                   ini_get_stdstring,
                   &m_errorLog);

  // Filesystem and Streams Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "allow_url_fopen",
                   ini_on_update_fail, ini_get_static_string_1);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "default_socket_timeout",
                   std::to_string(RuntimeOption::SocketDefaultTimeout).c_str(),
                   &m_socketDefaultTimeout);

  // Info
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "max_execution_time",
                   ini_on_update_max_execution_time,
                   ini_get_max_execution_time);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ALL,
                   "maximum_execution_time",
                   ini_on_update_max_execution_time,
                   ini_get_max_execution_time);

  // HPHP specific
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.compiler_id",
                   ini_on_update_fail,
                   [](void*) {
                     return getHphpCompilerId();
                   });
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.compiler_version",
                   ini_on_update_fail,
                   [](void*) {
                     return getHphpCompilerVersion();
                   });
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hhvm.ext_zend_compat",
                   ini_on_update_fail, ini_get_bool,
                   &RuntimeOption::EnableZendCompat),
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.build_id",
                   ini_on_update_fail, ini_get_stdstring,
                   &RuntimeOption::BuildId);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "notice_frequency",
                   &RuntimeOption::NoticeFrequency);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "warning_frequency",
                   &RuntimeOption::WarningFrequency);
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

///////////////////////////////////////////////////////////////////////////////
}
