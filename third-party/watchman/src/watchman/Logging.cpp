/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/Logging.h"

#include <folly/ScopeGuard.h>
#include <folly/ThreadLocal.h>
#include <folly/experimental/symbolizer/Symbolizer.h>
#include <folly/portability/SysTime.h>
#include <folly/system/ThreadName.h>

#include "watchman/portability/Backtrace.h"

#include <fmt/core.h>
#include <array>
#include <limits>
#include <optional>
#include <sstream>

#ifdef __APPLE__
#include <pthread.h>
#endif

using namespace watchman;

static folly::ThreadLocal<std::optional<std::string>> threadName;

namespace {
template <typename String>
void write_stderr(const String& str) {
  w_string_piece piece = str;
  ignore_result(::write(STDERR_FILENO, piece.data(), piece.size()));
}

template <typename String, typename... Strings>
void write_stderr(const String& str, Strings&&... strings) {
  write_stderr(str);
  write_stderr(strings...);
}
} // namespace

static void log_stack_trace() {
  using namespace folly::symbolizer;
#if FOLLY_HAVE_ELF && FOLLY_HAVE_DWARF
  static FastStackTracePrinter printer{
      std::make_unique<FDSymbolizePrinter>(STDERR_FILENO)};
#else
  // stack-allocated to avoid thread safety issues.
  SafeStackTracePrinter printer;
#endif
  write_stderr("Fatal error detected at:\n");
  printer.printStackTrace(true);
}

namespace watchman {

namespace {
struct levelMaps {
  // Actually a map of LogLevel, w_string, but it is relatively high friction
  // to define the hasher for an enum key :-p
  std::unordered_map<int, w_string> levelToLabel;
  std::unordered_map<w_string, LogLevel> labelToLevel;

  levelMaps()
      : levelToLabel{
            {ABORT, "abort"},
            {FATAL, "fatal"},
            {ERR, "error"},
            {OFF, "off"},
            {DBG, "debug"}} {
    // Create the reverse map
    for (auto& it : levelToLabel) {
      labelToLevel.insert(
          std::make_pair(it.second, static_cast<LogLevel>(it.first)));
    }
  }
};

// Meyers singleton for holding the log level maps
levelMaps& getLevelMaps() {
  static levelMaps maps;
  return maps;
}

} // namespace

const w_string& logLevelToLabel(LogLevel level) {
  return getLevelMaps().levelToLabel.at(static_cast<int>(level));
}

LogLevel logLabelToLevel(const w_string& label) {
  return getLevelMaps().labelToLevel.at(label);
}

Log::Log()
    : errorPub_(std::make_shared<Publisher>()),
      debugPub_(std::make_shared<Publisher>()) {
  setStdErrLoggingLevel(ERR);
}

Log& getLog() {
  static Log log;
  return log;
}

char* Log::timeString(char* buf, size_t bufsize, timeval tv) {
  struct tm tm;
#ifdef _WIN32
  time_t seconds = (time_t)tv.tv_sec;
  tm = *localtime(&seconds);
#else
  localtime_r(&tv.tv_sec, &tm);
#endif

  char timebuf[64];
  strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%S", &tm);
  snprintf(buf, bufsize, "%s,%03d", timebuf, (int)tv.tv_usec / 1000);
  return buf;
}

char* Log::currentTimeString(char* buf, size_t bufsize) {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return timeString(buf, bufsize, tv);
}

namespace {
// The C++ standard does not require that globals are all initialized on the
// same thread, but that's a safe assumption in practice.
std::thread::id mainThreadId = std::this_thread::get_id();
} // namespace

const char* Log::setThreadName(std::string&& name) {
  if (mainThreadId != std::this_thread::get_id()) {
    // pthread_setname_np on the main thread sets the name of the watchman
    // process, preventing `pkill watchman` from crashing. We still want to set
    // our local thread name for the purposes of Watchman's log messages.
    folly::setThreadName(name);
  }

  threadName->emplace(name);
  return threadName->value().c_str();
}

const char* Log::getThreadName() {
  if (!threadName->has_value()) {
    auto name = folly::getCurrentThreadName();
    if (name.hasValue()) {
      threadName->emplace(name.value());
    } else {
      std::stringstream ss;
      ss << std::this_thread::get_id();
      threadName->emplace(ss.str());
    }
  }
  return threadName->value().c_str();
}

void Log::setStdErrLoggingLevel(LogLevel level) {
  auto notify = [this]() { doLogToStdErr(); };
  auto subs = subscribers_.lock();
  auto& debugSub = subs->debugSub_;
  auto& errorSub = subs->errorSub_;
  switch (level) {
    case OFF:
      errorSub.reset();
      debugSub.reset();
      return;
    case DBG:
      if (!debugSub) {
        debugSub = debugPub_->subscribe(notify);
      }
      if (!errorSub) {
        errorSub = errorPub_->subscribe(notify);
      }
      return;
    default:
      debugSub.reset();
      if (!errorSub) {
        errorSub = errorPub_->subscribe(notify);
      }
      return;
  }
}

void Log::doLogToStdErr() {
  std::vector<std::shared_ptr<const watchman::Publisher::Item>> items;

  {
    auto subs = subscribers_.lock();
    getPending(items, subs->errorSub_, subs->debugSub_);
  }

  bool doFatal = false;
  bool doAbort = false;
  static w_string kFatal("fatal");
  static w_string kAbort("abort");

  for (auto& item : items) {
    auto& log = json_to_w_string(item->payload.get("log"));
    ignore_result(::write(STDERR_FILENO, log.data(), log.size()));

    auto level = json_to_w_string(item->payload.get("level"));
    if (level == kFatal) {
      doFatal = true;
    } else if (level == kAbort) {
      doAbort = true;
    }
  }

  if (doFatal || doAbort) {
    log_stack_trace();
    if (doAbort) {
      abort();
    } else {
      _exit(1);
    }
  }
}

#ifdef _WIN32
static constexpr size_t kMaxFrames = 64;

LONG WINAPI exception_filter(LPEXCEPTION_POINTERS excep) {
  std::array<void*, kMaxFrames> array;
  size_t size;
  char** strings;
  size_t i;
  char timebuf[64];

  size = backtrace_from_exception(excep, array.data(), array.size());
  strings = backtrace_symbols(array.data(), size);

  write_stderr(
      watchman::Log::currentTimeString(timebuf, sizeof(timebuf)),
      ": [",
      watchman::Log::getThreadName(),
      "] Unhandled win32 exception code=",
      fmt::to_string(excep->ExceptionRecord->ExceptionCode),
      ".  Fatal error detected at:\n");

  for (i = 0; i < size; i++) {
    write_stderr(strings[i], "\n");
  }
  free(strings);

  write_stderr("the stack trace for the exception filter call is:\n");
  size = backtrace(array.data(), array.size());
  strings = backtrace_symbols(array.data(), size);
  for (i = 0; i < size; i++) {
    write_stderr(strings[i], "\n");
  }
  free(strings);

  // Terminate the process.
  // msvcrt abort() ultimately calls exit(3), so we shortcut that.
  // Ideally we'd just exit() or ExitProcess() and be done, but it
  // is documented as possible (or even likely!) that deadlock
  // is possible, so we use TerminateProcess() to force ourselves
  // to terminate.
  TerminateProcess(GetCurrentProcess(), 3);
  // However, TerminateProcess() is asynchronous and we will continue
  // running here.  Let's also try exiting normally and see which
  // approach wins!
  exit(3);
  return EXCEPTION_CONTINUE_SEARCH;
}
#endif

} // namespace watchman
