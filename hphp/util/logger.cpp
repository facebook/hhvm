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
ServiceData::ExportedCounter* Logger::s_errorBytes =
    ServiceData::createCounter("errorlog_bytes");

IMPLEMENT_THREAD_LOCAL(Logger::ThreadData, Logger::s_threadData);

std::map<std::string, Logger*> Logger::s_loggers = {
  {Logger::DEFAULT, new Logger()},
};

void Logger::Log(LogLevelType level, const char *type, const Exception &e,
                 const char *file /* = NULL */, int line /* = 0 */) {
  if (!IsEnabled()) return;
  auto msg = type + e.getMessage();
  if (file && file[0]) {
    msg += folly::sformat(" in {} on line {}", file, line);
  }
  LogImpl(level, msg, nullptr);
}

void Logger::OnNewRequest() {
  ThreadData *threadData = s_threadData.get();
  ++threadData->request;
  threadData->message = 0;
}

void Logger::ResetRequestCount() {
  ThreadData *threadData = s_threadData.get();
  threadData->request = 0;
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
  for (auto& l : s_loggers) {
    auto& logger = l.second;
    if (logger) {
      auto growth = logger->log(level, msg, stackTrace, escape, escapeMore);
      s_errorLines->addValue(growth.first);
      s_errorBytes->addValue(growth.second);
    }
  }
}

void Logger::SetStandardOut(const std::string &name, FILE *file) {
  auto it = s_loggers.find(name);
  if (it != s_loggers.end()) {
    auto& logger = it->second;
    logger->m_standardOut = file;
  }
}

void Logger::FlushAll() {
  for (auto& l : s_loggers) {
    auto& logger = l.second;
    if (logger) {
      auto growth = logger->flush();
      s_errorLines->addValue(growth.first);
      s_errorBytes->addValue(growth.second);
    }
  }
}

void Logger::SetBatchSize(size_t bsize) {
  for (auto& l : s_loggers) {
    auto& logger = l.second;
    logger->setBatchSize(bsize);
  }
}

void Logger::SetFlushTimeout(std::chrono::milliseconds timeoutMs) {
  for (auto& l : s_loggers) {
    auto& logger = l.second;
    logger->setFlushTimeout(timeoutMs);
  }
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

std::pair<int, int> Logger::log(LogLevelType level, const std::string &msg,
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
  return std::make_pair(1, bytes);
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
  snprintf(header, sizeof(header), "[%s] [hphp] [%lld:%llx:%d:%06d%s] ",
           snow,
           (unsigned long long)s_pid,
           (unsigned long long)Process::GetThreadId(),
           threadData->request,
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

void Logger::SetTheLogger(const std::string &name, Logger* newLogger) {
  auto& logger = s_loggers[name];
  if (logger != nullptr) delete logger;
  if (newLogger) {
    logger = newLogger;
  } else {
    s_loggers.erase(name);
  }
}

void Logger::UnlimitThreadMessages() {
  ThreadData *threadData = s_threadData.get();
  threadData->message = -1;
}

Cronolog *Logger::CronoOutput(const std::string &name) {
  auto it = s_loggers.find(name);
  if (it != s_loggers.end()) {
    auto& logger = it->second;
    return &logger->m_cronOutput;
  }
  return nullptr;
}

void Logger::SetOutput(const std::string &name, FILE *output, bool isPipe) {
  auto it = s_loggers.find(name);
  if (it != s_loggers.end()) {
    auto& logger = it->second;
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
  const auto it = s_loggers.find(name);
  if (it != s_loggers.end()) {
    const auto& logger = it->second;
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
