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
#include "hphp/util/logger.h"

#include "hphp/util/assertions.h"
#include "hphp/util/exception.h"
#include "hphp/util/process.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/string-vsnprintf.h"
#include "hphp/util/text-color.h"

#include <folly/portability/Syslog.h>
#include <folly/portability/Unistd.h>

#include <typeinfo>

#define IMPLEMENT_LOGLEVEL(LOGLEVEL)                                    \
  void Logger::LOGLEVEL(const char *fmt, ...) {                         \
    if (LogLevel < Log ## LOGLEVEL) return;                             \
    if (!IsEnabled()) return;                                           \
    std::string msg;                                                    \
    va_list ap; va_start(ap, fmt);                                      \
    string_vsnprintf(msg, fmt, ap);                                     \
    va_end(ap);                                                         \
    LogImpl(Log ## LOGLEVEL, msg, nullptr);                             \
  }                                                                     \
  void Logger::LOGLEVEL(const std::string &msg) {                       \
    if (LogLevel < Log ## LOGLEVEL) return;                             \
    LogImpl(Log ## LOGLEVEL, msg, nullptr);                             \
  }

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_LOGLEVEL(Error);
IMPLEMENT_LOGLEVEL(Warning);
IMPLEMENT_LOGLEVEL(Info);
IMPLEMENT_LOGLEVEL(Verbose);

///////////////////////////////////////////////////////////////////////////////

constexpr const char* Logger::DEFAULT;

bool Logger::AlwaysEscapeLog = true;
bool Logger::UseSyslog = false;
bool Logger::UseLogFile = true;
bool Logger::UseRequestLog = false;
bool Logger::UseCronolog = false;
Logger::LogLevelType Logger::LogLevel = LogInfo;
bool Logger::LogHeader = false;
bool Logger::LogNativeStackTrace = true;
std::string Logger::ExtraHeader;
int Logger::MaxMessagesPerRequest = -1;
bool Logger::Escape = true;
pid_t Logger::s_pid;
ServiceData::ExportedCounter* Logger::s_errorLines =
    ServiceData::createCounter("errorlog_lines");
// ideally this should be "errorlog_serialized_bytes" but this was
// added long ago and many tools rely on it.
ServiceData::ExportedCounter* Logger::s_errorSerializedBytes =
    ServiceData::createCounter("errorlog_bytes");
ServiceData::ExportedCounter* Logger::s_errorCompressedBytes =
    ServiceData::createCounter("errorlog_bytes_compressed");
THREAD_LOCAL(Logger::ThreadData, Logger::s_threadData);

Logger::GlobalLoggers::GlobalLoggers() {
  m_loggers.insert({Logger::DEFAULT, std::make_unique<Logger>()});
}

Logger* Logger::getLogger(const std::string& name) {
  auto loggers = Logger::getLoggers();
  if (!loggers) return nullptr;
  auto it = loggers->m_loggers.find(name);
  if (it == loggers->m_loggers.end()) {
    return nullptr;
  }
  return it->second.get();
}


std::optional<Logger*> Logger::GlobalLoggers::get(const std::string& name) {
  auto it = m_loggers.find(name);
  if (it == m_loggers.end()) {
    return std::nullopt;
  } else {
    return it->second.get();
  }
}

std::shared_ptr<Logger::GlobalLoggers> Logger::getLoggers() {
  // We use a meyers singleton to track the global singleton
  // GlobalLoggers. Since we want to tell when the main GlobalLoggers has been
  // destroyed and we can't guarantee that ~shared_ptr will clear itself on
  // destruction we use a separate bool.
  //
  // Can't use folly::Singleton because we don't actually call folly::init()!
  struct DestructTracker {
    ~DestructTracker() { m_destroyed = true; }
    explicit DestructTracker(bool& destroyed)
      : m_destroyed(destroyed),
        m_loggers{std::make_shared<Logger::GlobalLoggers>()} {
    }
    bool& m_destroyed;
    std::shared_ptr<Logger::GlobalLoggers> m_loggers;
  };
  static bool s_destroyed;
  static DestructTracker s_loggers(s_destroyed);
  if (s_destroyed) {
    return nullptr;
  }
  return s_loggers.m_loggers;
}

void Logger::Log(LogLevelType level, const char* type, const Exception& e,
                 const char *file /* = NULL */, int line /* = 0 */) {
  if (!IsEnabled()) return;
  auto msg = type + e.getMessage();
  if (file && file[0]) {
    msg += folly::sformat(" in {} on line {}", file, line);
  }
  LogImpl(level, msg, nullptr);
}

void Logger::OnNewRequest(int64_t requestId) {
  ThreadData *threadData = s_threadData.get();
  threadData->requestId = requestId;
  threadData->message = 0;
}

void Logger::LogImpl(LogLevelType level, const std::string &msg,
                     const StackTrace *stackTrace,
                     bool escape /* = false */, bool escapeMore /* = false */) {

  ThreadData *threadData = s_threadData.get();
  if (threadData->message != -1 &&
      ++threadData->message > MaxMessagesPerRequest &&
      MaxMessagesPerRequest >= 0) {
    return;
  }
  Logger::forEachLogger([&](Logger& logger) {
    auto growth = logger.log(level, msg, stackTrace, escape, escapeMore);
    s_errorLines->addValue(growth.lines);
    s_errorSerializedBytes->addValue(growth.serializedBytes);
    s_errorCompressedBytes->addValue(growth.compressedBytes);
  });
}

void Logger::SetStandardOut(const std::string &name, FILE *file) {
  Logger::withLoggers([&](auto& loggers) {
    if (auto logger = loggers.get(name)) {
      logger.value()->m_standardOut = file;
    }
  });
}

void Logger::FlushAll() {
  Logger::forEachLogger([&](Logger& logger) {
    auto growth = logger.flush();
    s_errorLines->addValue(growth.lines);
    s_errorSerializedBytes->addValue(growth.serializedBytes);
    s_errorCompressedBytes->addValue(growth.compressedBytes);
  });
}

void Logger::SetBatchSize(size_t bsize) {
  Logger::forEachLogger([&](Logger& logger) {
    logger.setBatchSize(bsize);
  });
}

void Logger::SetFlushTimeout(std::chrono::milliseconds timeoutMs) {
  Logger::forEachLogger([&](Logger& logger) {
    logger.setFlushTimeout(timeoutMs);
  });
}

int Logger::GetSyslogLevel(LogLevelType level) {
  switch (level) {
  case LogError:   return LOG_ERR;
  case LogWarning: return LOG_WARNING;
  case LogInfo:    return LOG_INFO;
  case LogVerbose: return LOG_DEBUG;
  default:         return LOG_NOTICE;
  }
}

LogGrowth Logger::log(LogLevelType level, const std::string &msg,
                      const StackTrace *stackTrace,
                      bool escape /* = false */,
                      bool escapeMore /* = false */) {
  if (Logger::AlwaysEscapeLog && Logger::Escape) {
    escape = true;
  }
  assertx(!escapeMore || escape); // escape must be enabled to escapeMore

  std::unique_ptr<StackTrace> deleter;
  if (LogNativeStackTrace && stackTrace == nullptr) {
    deleter.reset(new StackTrace());
    stackTrace = deleter.get();
  }

  if (UseSyslog) {
    syslog(GetSyslogLevel(level), "%s", msg.c_str());
  }
  int bytes = 0;
  if (UseLogFile) {
    ThreadData *threadData = s_threadData.get();
    FILE* tf = threadData->log;
    FILE* f = output();
    std::string header, sheader;
    if (LogHeader) {
      header = GetHeader();
      if (LogNativeStackTrace) {
        sheader = header + "[" + stackTrace->hexEncode(5) + "] ";
      } else {
        sheader = header;
      }
    }
    const char *escaped = escape ? EscapeString(msg) : msg.c_str();
    const char *ending = escapeMore ? "\\n" : "\n";
    if (f == m_standardOut && s_stderr_color) {
      bytes =
        fprintf(f, "%s%s%s%s%s",
                s_stderr_color, sheader.c_str(), msg.c_str(), ending,
                ANSI_COLOR_END);
    } else {
      bytes = fprintf(f, "%s%s%s", sheader.c_str(), escaped, ending);
    }
    if (tf && tf != f) {
      int threadBytes =
        fprintf(tf, "%s%s%s", header.c_str(), escaped, ending);
      fflush(tf);
      threadData->flusher.recordWriteAndMaybeDropCaches(tf, threadBytes);
    }
    if (threadData->hook) {
      (*threadData->hook)(header.c_str(), msg.c_str(), ending);
    }
    if (escape) {
      free((void*)escaped);
    }
    fflush(f);
    if (UseCronolog || (m_output && !m_isPipeOutput)) {
      m_flusher.recordWriteAndMaybeDropCaches(f, bytes);
    }
  }
  return LogGrowth(1, static_cast<uint64_t>(bytes), static_cast<uint64_t>(bytes));
}

void Logger::ResetPid() {
  s_pid = getpid();
}

std::string Logger::GetHeader() {
  static std::string host = Process::GetHostName();

  time_t now = time(nullptr);
  char snow[64];
  ctime_r(&now, snow);
  // Eliminate trailing newline from ctime_r.
  snow[24] = '\0';

  char header[128];
  ThreadData *threadData = s_threadData.get();
  snprintf(header, sizeof(header), "[%s] [hphp] [%lld:%llx:%lld:%06d%s] ",
           snow,
           (unsigned long long)s_pid,
           (unsigned long long)Process::GetThreadId(),
           (long long)threadData->requestId,
           (threadData->message == -1 ? 0 : threadData->message),
           ExtraHeader.c_str());
  return header;
}

char *Logger::EscapeString(const std::string &msg) {
  auto new_size = msg.size() * 4 + 1;
  char *new_str = (char *)malloc(new_size);
  const char *source;
  const char *end;
  char *target;
  for (source = msg.c_str(), end = source + msg.size(), target = new_str;
       source < end && *source; source++) {
    char c = *source;
    if ((unsigned char) c < 32 || (unsigned char) c > 126) {
      *target++ = '\\';
      switch (c) {
      case '\n': *target++ = 'n'; break;
      case '\t': *target++ = 't'; break;
      case '\r': *target++ = 'r'; break;
      case '\v': *target++ = 'v'; break;
      case '\b': *target++ = 'b'; break;
      default: {
        auto avail = new_size - (target - new_str);
        target += snprintf(target, avail, "x%02x", (unsigned char)c);
        assertx(target < new_str + new_size); // we allocated 4x space
      }}
    } else if (c == '\\') {
      *target++ = c;
      *target++ = c;
    } else {
      *target++ = c;
    }
  }
  *target = 0;
  return new_str;
}

bool Logger::SetThreadLog(const char *file, bool threadOnly) {
  if (auto log = fopen(file, "a")) {
    ClearThreadLog();
    s_threadData->log = log;
    s_threadData->threadLogOnly = threadOnly;
    return true;
  }
  return false;
}
void Logger::ClearThreadLog() {
  ThreadData *threadData = s_threadData.get();
  if (threadData->log) {
    fclose(threadData->log);
    threadData->log = nullptr;
  }
}

void Logger::SetThreadHook(LoggerHook* hook) {
  s_threadData.get()->hook = hook;
}

void Logger::SetTheLogger(const std::string &name, std::unique_ptr<Logger> newLogger) {
  withLoggers([&](GlobalLoggers& loggers) {
    if (!newLogger) {
      loggers.m_loggers.erase(name);
    } else {
      loggers.m_loggers.insert_or_assign(name, std::move(newLogger));
    }
  });
}

bool Logger::IsDefaultLogger(const std::string &name) {
  if (auto logger = Logger::getLogger(name)) {
    return typeid(*logger) == typeid(Logger);
  }
  // This is weird - but seems to be what the original code was doing.
  return true;
}

void Logger::UnlimitThreadMessages() {
  ThreadData *threadData = s_threadData.get();
  threadData->message = -1;
}

Cronolog *Logger::CronoOutput(const std::string &name) {
  if (auto logger = Logger::getLogger(name)) {
    return &logger->m_cronOutput;
  } else {
    return nullptr;
  }
}

void Logger::SetOutput(const std::string &name, FILE *output, bool isPipe) {
  if (auto logger = Logger::getLogger(name)) {
    if (logger->m_output && logger->m_output != output) {
      if (logger->m_isPipeOutput) {
        pclose(logger->m_output);
      } else {
        fclose(logger->m_output);
      }
    }
    logger->m_output = output;
    logger->m_isPipeOutput = isPipe;
  }
}

std::pair<FILE*, bool> Logger::GetOutput(const std::string &name) {
  if (auto logger = Logger::getLogger(name)) {
    return std::make_pair(logger->m_output, logger->m_isPipeOutput);
  }
  return std::make_pair(nullptr, false);
}

FILE* Logger::output() {
  ThreadData *threadData = s_threadData.get();
  if (threadData->log && threadData->threadLogOnly) {
    return threadData->log;
  }
  FILE* cronOut = m_cronOutput.getOutputFile();
  return cronOut != nullptr ? cronOut :
         m_output != nullptr ? m_output : m_standardOut;
}

///////////////////////////////////////////////////////////////////////////////
}
