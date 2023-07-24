/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fmt/ranges.h>
#include <folly/Synchronized.h>
#include <folly/portability/Windows.h> // For timeval. Replace this.

#include "watchman/PubSub.h"
#include "watchman/watchman_preprocessor.h"
#include "watchman/watchman_string.h"

namespace watchman {

enum LogLevel { ABORT = -2, FATAL = -1, OFF = 0, ERR = 1, DBG = 2 };

const w_string& logLevelToLabel(LogLevel level);
LogLevel logLabelToLevel(const w_string& label);

class Log {
 public:
  std::shared_ptr<Publisher::Subscriber> subscribe(
      LogLevel level,
      Publisher::Notifier notify) {
    return levelToPub(level).subscribe(notify);
  }

  static char* currentTimeString(char* buf, size_t bufsize);
  static char* timeString(char* buf, size_t bufsize, timeval tv);
  static const char* getThreadName();
  static const char* setThreadName(std::string&& name);

  void setStdErrLoggingLevel(LogLevel level);

  // Build a string and log it
  template <typename... Args>
  void log(LogLevel level, Args&&... args) {
    auto& pub = levelToPub(level);

    // Avoid building the string if there are no subscribers
    if (!pub.hasSubscribers()) {
      return;
    }

    char timebuf[64];

    auto payload = json_object(
        {{"log",
          typed_string_to_json(w_string::build(
              currentTimeString(timebuf, sizeof(timebuf)),
              ": [",
              getThreadName(),
              "] ",
              std::forward<Args>(args)...))},
         {"unilateral", json_true()},
         {"level", typed_string_to_json(logLevelToLabel(level))}});

    pub.enqueue(std::move(payload));
  }

  // Format a string and log it
  template <typename... Args>
  void logf(LogLevel level, fmt::string_view format_str, Args&&... args) {
    auto& pub = levelToPub(level);

    // Avoid building the string if there are no subscribers
    if (!pub.hasSubscribers()) {
      return;
    }

    char timebuf[64];

    auto message =
        fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...);
    auto payload = json_object(
        {{"log",
          typed_string_to_json(w_string::build(
              currentTimeString(timebuf, sizeof(timebuf)),
              ": [",
              getThreadName(),
              "] ",
              std::move(message)))},
         {"unilateral", json_true()},
         {"level", typed_string_to_json(logLevelToLabel(level))}});

    pub.enqueue(std::move(payload));
  }

  Log();

 private:
  std::shared_ptr<Publisher> errorPub_;
  std::shared_ptr<Publisher> debugPub_;

  struct Subscribers {
    std::shared_ptr<Publisher::Subscriber> errorSub_;
    std::shared_ptr<Publisher::Subscriber> debugSub_;
  };
  // The lock on the subscribers exists for 2 reasons:
  // 1. The standard reason: preventing multiple threads from clobbering over
  //    each other or reading garbage from the subscribers. This lock prevents
  //    multiple clients from clobbering the subscribers.
  // 2. Only one thread may print to standard error at a given time. This avoids
  //    the output logs from becoming scrambled. This lock is acquired before
  //    writing to stderr.
  folly::Synchronized<Subscribers, std::mutex> subscribers_;

  inline Publisher& levelToPub(LogLevel level) {
    return level == DBG ? *debugPub_ : *errorPub_;
  }

  void doLogToStdErr();
};

// Get the logger singleton
Log& getLog();

template <typename... Args>
void log(LogLevel level, Args&&... args) {
  getLog().log(level, std::forward<Args>(args)...);
}

template <typename... Args>
void logf(LogLevel level, fmt::string_view format_str, Args&&... args) {
  getLog().logf(level, format_str, std::forward<Args>(args)...);
}

// Log only to stderr. This bypasses Logging / folly::Synchronized which
// might deadlock during exit.
template <typename... Args>
void logf_stderr(fmt::string_view format_str, Args&&... args) {
  auto msg = fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...);
  ignore_result(write(STDERR_FILENO, msg.data(), msg.size()));
}

#ifdef _WIN32
LONG WINAPI exception_filter(LPEXCEPTION_POINTERS excep);
#endif

} // namespace watchman

template <typename... Args>
const char* w_set_thread_name(const Args&... args) {
  auto name =
      fmt::to_string(fmt::join(std::make_tuple<const Args&...>(args...), ""));
  return watchman::Log::setThreadName(std::move(name));
}

#define w_check(e, ...)                          \
  if (!(e)) {                                    \
    watchman::logf(                              \
        watchman::ERR,                           \
        "{}:{} failed assertion `{}'\n",         \
        __FILE__,                                \
        __LINE__,                                \
        #e);                                     \
    watchman::log(watchman::ABORT, __VA_ARGS__); \
  }

// Similar to assert(), but uses W_LOG_FATAL to log the stack trace
// before giving up the ghost
#ifdef NDEBUG
#define w_assert(e, ...) ((void)0)
#else
#define w_assert(e, ...) w_check(e, __VA_ARGS__)
#endif
