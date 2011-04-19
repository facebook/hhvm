/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "logger.h"
#include "base.h"
#include "stack_trace.h"
#include "process.h"
#include "exception.h"
#include "util.h"
#include "log_aggregator.h"
#include "text_color.h"
#include <util/atomic.h>
#include <runtime/base/runtime_option.h>

using namespace std;

#define IMPLEMENT_LOGLEVEL(LOGLEVEL, err)                               \
  void Logger::LOGLEVEL(const char *fmt, ...) {                         \
    if (LogLevel < Log ## LOGLEVEL) return;                             \
    va_list ap; va_start(ap, fmt); Log(err, fmt, ap); va_end(ap);       \
  }                                                                     \
  void Logger::LOGLEVEL(const std::string &msg) {                       \
    if (LogLevel < Log ## LOGLEVEL) return;                             \
    Log(err, msg, NULL);                                                \
  }                                                                     \
  void Logger::Raw ## LOGLEVEL(const std::string &msg) {                \
    if (LogLevel < Log ## LOGLEVEL) return;                             \
    Log(err, msg, NULL, false);                                         \
  }                                                                     \

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_LOGLEVEL(Error,   true);
IMPLEMENT_LOGLEVEL(Warning, true);
IMPLEMENT_LOGLEVEL(Info,    false);
IMPLEMENT_LOGLEVEL(Verbose, false);

///////////////////////////////////////////////////////////////////////////////

bool Logger::UseLogAggregator = false;
bool Logger::UseLogFile = true;
bool Logger::UseCronolog = true;
int Logger::DropCacheChunkSize = (1 << 20);
FILE *Logger::Output = NULL;
Cronolog Logger::cronOutput;
Logger::LogLevelType Logger::LogLevel = LogInfo;
int Logger::bytesWritten = 0;
int Logger::prevBytesWritten = 0;
bool Logger::LogHeader = false;
bool Logger::LogNativeStackTrace = true;
std::string Logger::ExtraHeader;
int Logger::MaxMessagesPerRequest = -1;
IMPLEMENT_THREAD_LOCAL(Logger::ThreadData, Logger::s_threadData);

Logger *Logger::s_logger = new Logger();

void Logger::Log(bool err, const char *fmt, va_list ap) {
  if (!UseLogAggregator && !UseLogFile) return;

  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  Log(err, msg, NULL);
}

void Logger::LogEscapeMore(bool err, const char *fmt, va_list ap) {
  if (!UseLogAggregator && !UseLogFile) return;

  string msg;
  Util::string_vsnprintf(msg, fmt, ap);
  Log(err, msg, NULL, true, true);
}

void Logger::Log(bool err, const char *type, const Exception &e,
                 const char *file /* = NULL */, int line /* = 0 */) {
  s_logger->log(err, type, e, file, line);
}

void Logger::log(bool err, const char *type, const Exception &e,
                 const char *file /* = NULL */, int line /* = 0 */) {
  if (!UseLogAggregator && !UseLogFile) return;

  std::string msg = type;
  msg += e.getMessage();
  if (file && file[0]) {
    ostringstream os;
    os << " in " << file << " on line " << line;
    msg += os.str();
  }
  Log(err, msg, &e.getStackTrace());
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

void Logger::Log(bool err, const std::string &msg,
                 const StackTrace *stackTrace,
                 bool escape /* = true */, bool escapeMore /* = false */) {
  s_logger->log(err, msg, stackTrace, escape, escapeMore);
}

void Logger::log(bool err, const std::string &msg,
                 const StackTrace *stackTrace,
                 bool escape /* = true */, bool escapeMore /* = false */) {
  ASSERT(!escapeMore || escape);
  ThreadData *threadData = s_threadData.get();
  if (++threadData->message > MaxMessagesPerRequest &&
      MaxMessagesPerRequest >= 0) {
    return;
  }

  boost::shared_ptr<StackTrace> deleter;
  if (stackTrace == NULL) {
    deleter = boost::shared_ptr<StackTrace>(new StackTrace());
    stackTrace = deleter.get();
  }

  if (UseLogAggregator) {
    LogAggregator::TheLogAggregator.log(*stackTrace, msg);
  }
  FILE *stdf = err ? stderr : stdout;
  if (UseLogFile) {
    FILE *f;
    if (UseCronolog) {
      f = cronOutput.getOutputFile();
      if (!f) f = stdf;
    } else {
      f = Output ? Output : stdf;
    }
    string header, sheader;
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
    int bytes;
    if (f == stdf && Util::s_stderr_color) {
      bytes =
        fprintf(f, "%s%s%s%s%s",
                Util::s_stderr_color, sheader.c_str(), msg.c_str(), ending,
                ANSI_COLOR_END);
    } else {
      bytes = fprintf(f, "%s%s%s", sheader.c_str(), escaped, ending);
    }
    atomic_add(bytesWritten, bytes);
    FILE *tf = threadData->log;
    if (tf) {
      threadData->bytesWritten +=
        fprintf(tf, "%s%s%s", header.c_str(), escaped, ending);
      fflush(tf);
      checkDropCache(threadData->bytesWritten,
                     threadData->prevBytesWritten,
                     tf);
    }
    if (threadData->hook) {
      threadData->hook(header.c_str(), msg.c_str(), ending,
                       threadData->hookData);
    }
    if (escape) {
      free((void*)escaped);
    }

    fflush(f);
    if (UseCronolog || (Output && RuntimeOption::LogFile[0] != '|')) {
      checkDropCache(bytesWritten, prevBytesWritten, f);
    }
  }
}

std::string Logger::GetHeader() {
  static std::string host = Process::GetHostName();
  static pid_t pid = Process::GetProcessId();

  time_t now = time(NULL);
  char snow[64];
  ctime_r(&now, snow);
  // Eliminate trailing newilne from ctime_r.
  snow[24] = '\0';

  char header[128];
  ThreadData *threadData = s_threadData.get();
  snprintf(header, sizeof(header), "[%s] [hphp] [%lld:%llx:%d:%06d%s] ",
           snow,
           (unsigned long long)pid,
           (unsigned long long)Process::GetThreadId(),
           threadData->request, threadData->message,
           ExtraHeader.c_str());
  return header;
}

char *Logger::EscapeString(const std::string &msg) {
  char *new_str = (char *)malloc((msg.size() << 2) + 1);
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
      default: target += sprintf(target, "x%02x", (unsigned char)c);
      }
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

bool Logger::checkDropCache(int &bytesWritten, int &prevBytesWritten,
                            FILE *f) {
  if (bytesWritten - prevBytesWritten > Logger::DropCacheChunkSize) {
    Util::drop_cache(f, (off_t)bytesWritten);
    prevBytesWritten = bytesWritten;
    return true;
  }
  return false;
}

bool Logger::SetThreadLog(const char *file) {
  return (s_threadData->log = fopen(file, "a")) != NULL;
}
void Logger::ClearThreadLog() {
  ThreadData *threadData = s_threadData.get();
  if (threadData->log) {
    fclose(threadData->log);
  }
  threadData->log = NULL;
}

void Logger::SetThreadHook(PFUNC_LOG func, void *data) {
  ThreadData *threadData = s_threadData.get();
  threadData->hook = func;
  threadData->hookData = data;
}

void Logger::SetNewOutput(FILE *output) {
  Logger::UseCronolog = false;
  ThreadData *threadData = s_threadData.get();
  if (threadData->log) {
    fclose(threadData->log);
    threadData->log = output;
  } else {
    if (Output) fclose(Output);
    Output = output;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
