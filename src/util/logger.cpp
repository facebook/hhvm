/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include "log_aggregator.h"

using namespace std;

#define IMPLEMENT_LOGLEVEL(LOGLEVEL)                                   \
  void Logger::LOGLEVEL(const char *fmt, ...) {                        \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    va_list ap; va_start(ap, fmt); Log(fmt, ap); va_end(ap);           \
  }                                                                    \
  void Logger::LOGLEVEL(const std::string &msg) {                      \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    Log(msg, NULL);                                                    \
  }                                                                    \
  void Logger::Raw ## LOGLEVEL(const std::string &msg) {               \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    Log(msg, NULL, false);                                             \
  }                                                                    \

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_LOGLEVEL(Error);
IMPLEMENT_LOGLEVEL(Warning);
IMPLEMENT_LOGLEVEL(Info);
IMPLEMENT_LOGLEVEL(Verbose);

///////////////////////////////////////////////////////////////////////////////

bool Logger::UseLogAggregator = false;
bool Logger::UseLogFile = true;
FILE *Logger::Output = NULL;
Logger::LogLevelType Logger::LogLevel = LogInfo;
bool Logger::LogHeader = false;
bool Logger::LogNativeStackTrace = true;
std::string Logger::ExtraHeader;
int Logger::MaxMessagesPerRequest = -1;
IMPLEMENT_THREAD_LOCAL(Logger::ThreadData, Logger::s_threadData);

Logger *Logger::s_logger = new Logger();

void Logger::Printf(std::string &msg, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  VSNPrintf(msg, fmt, ap);
  va_end(ap);
}

void Logger::VSNPrintf(std::string &msg, const char *fmt, va_list ap) {
  int i = 0;
  for (int len = 1024; msg.empty(); len <<= 1) {
    va_list v;
    va_copy(v, ap);

    char *buf = (char*)malloc(len);
    if (vsnprintf(buf, len, fmt, v) < len) {
      msg = buf;
    }
    free(buf);

    va_end(v);
    if (++i > 10) break;
  }
}

void Logger::Log(const char *fmt, va_list ap) {
  if (!UseLogAggregator && !UseLogFile) return;

  string msg;
  VSNPrintf(msg, fmt, ap);
  Log(msg, NULL);
}

void Logger::Log(const char *type, const Exception &e,
                 const char *file /* = NULL */, int line /* = 0 */) {
  s_logger->log(type, e, file, line);
}

void Logger::log(const char *type, const Exception &e,
                 const char *file /* = NULL */, int line /* = 0 */) {
  if (!UseLogAggregator && !UseLogFile) return;

  std::string msg = type;
  msg += e.getMessage();
  if (file && file[0]) {
    ostringstream os;
    os << " in " << file << " on line " << line;
    msg += os.str();
  }
  Log(msg, &e.getStackTrace());
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

void Logger::Log(const std::string &msg, const StackTrace *stackTrace,
                 bool escape /* = true */) {
  s_logger->log(msg, stackTrace, escape);
}

void Logger::log(const std::string &msg, const StackTrace *stackTrace,
                 bool escape /* = true */) {
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
  if (UseLogFile) {
    FILE *f = Output ? Output : stdout;

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
    fprintf(f, "%s%s\n", sheader.c_str(), escaped);
    FILE *tf = threadData->log;
    if (tf) {
      fprintf(tf, "%s%s\n", header.c_str(), escaped);
      fflush(tf);
    }
    if (escape) {
      free((void*)escaped);
    }

    fflush(f);
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
       source < end; source++) {
    char c = *source;
    if ((unsigned char) c < 32 || (unsigned char) c > 126) {
      *target++ = '\\';
      switch (c) {
      case '\n': *target++ = 'n'; break;
      case '\t': *target++ = 't'; break;
      case '\r': *target++ = 'r'; break;
      case '\a': *target++ = 'a'; break;
      case '\v': *target++ = 'v'; break;
      case '\b': *target++ = 'b'; break;
      case '\f': *target++ = 'f'; break;
      default: target += sprintf(target, "x%02X", (unsigned char)c);
      }
    } else {
      *target++ = c;
    }
  }
  *target = 0;
  return new_str;
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

///////////////////////////////////////////////////////////////////////////////
}
