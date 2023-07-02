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

#include "hphp/runtime/base/extended-logger.h"

#include <sstream>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/util/string-vsnprintf.h"

///////////////////////////////////////////////////////////////////////////////
#define IMPLEMENT_LOGLEVEL(LOGLEVEL)                                   \
  void ExtendedLogger::LOGLEVEL(const char *fmt, ...) {                \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    std::string msg;                                                   \
    va_list ap; va_start(ap, fmt);                                     \
    string_vsnprintf(msg, fmt, ap);                                    \
    va_end(ap);                                                        \
    if (!ExtendedLogger::EnabledByDefault) {                           \
      LogImpl(Log ## LOGLEVEL, msg);                                   \
    } else {                                                           \
      Logger::LogImpl(Log ## LOGLEVEL, msg, nullptr, true, true);      \
    }                                                                  \
  }                                                                    \
  void ExtendedLogger::LOGLEVEL(const std::string &msg) {              \
    if (LogLevel < Log ## LOGLEVEL) return;                            \
    if (!ExtendedLogger::EnabledByDefault) {                           \
      LogImpl(Log ## LOGLEVEL, msg);                                   \
    } else {                                                           \
      Logger::LogImpl(Log ## LOGLEVEL, msg, nullptr, true, true);      \
    }                                                                  \
  }

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

bool ExtendedLogger::EnabledByDefault = false;

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_LOGLEVEL(Error);
IMPLEMENT_LOGLEVEL(Warning);
IMPLEMENT_LOGLEVEL(Info);
IMPLEMENT_LOGLEVEL(Verbose);

///////////////////////////////////////////////////////////////////////////////

void ExtendedLogger::LogImpl(LogLevelType level, const std::string &msg) {
  assertx(!EnabledByDefault);
  ThreadData *threadData = s_threadData.get();
  if (threadData->message != -1 &&
      ++threadData->message > MaxMessagesPerRequest &&
      MaxMessagesPerRequest >= 0) {
    return;
  }
  Logger::forEachLogger([&](Logger& logger) {
    // we can only get here if there's no extended loggers (see assertion)
    // if this isn't enough of assurance we could check type at runtime,
    // but probably good enough for now
    Array bt;
    bool writeBt = false;
    auto* stFile = logger.fileForStackTrace();
    if (stFile) {
      bt = createBacktrace(BacktraceArgs());
      writeBt = !bt.empty();
    }
    // only escape more (the final new line) if we're writing the backtrace
    auto growth = logger.log(level, msg, nullptr, true, writeBt);
    if (writeBt) {
      // escape the BT too as well as the log
      auto stacktraceSize = PrintStackTrace(stFile, bt, true);
      growth.serializedBytes += stacktraceSize;
      growth.compressedBytes += stacktraceSize;
      FILE* tf = s_threadData.get()->log;
      if (tf && tf != stFile) {
        PrintStackTrace(tf, bt, true);
      }
    }
    s_errorLines->addValue(growth.lines);
    s_errorSerializedBytes->addValue(growth.serializedBytes);
    s_errorCompressedBytes->addValue(growth.compressedBytes);
  });
}

LogGrowth ExtendedLogger::log(LogLevelType level,
                                        const std::string &msg,
                                        const StackTrace *stackTrace,
                                        bool escape /* = true */,
                                        bool escapeMore /* = false */) {
  assertx(!escapeMore || escape); // escape must be enabled to escapeMore
  Array bt = createBacktrace(BacktraceArgs());
  if (bt.empty()) {
    return Logger::log(level, msg, stackTrace, escape, escapeMore);
  }
  // If we've got a BT and escaping, then we've got to escape more to fit BT.
  auto growth = Logger::log(level, msg, stackTrace, escape, escape);
  FILE* out = output();
  FILE* tf = s_threadData.get()->log;
  if (out) {
    auto stackTraceBytes = PrintStackTrace(out, bt, escape);
    growth.compressedBytes += stackTraceBytes;
    growth.serializedBytes += stackTraceBytes;
  }
  if (tf && tf != out) {
    PrintStackTrace(tf, bt, escape);
  }
  return growth;
}

const StaticString
  s_class("class"),
  s_function("function"),
  s_file("file"),
  s_type("type"),
  s_line("line");

int ExtendedLogger::PrintStackTrace(FILE *f, const Array& stackTrace,
                                    bool escape /* = false */) {
  if (stackTrace.isNull() || !f) return 0;
  int i = 0;
  int bytes = 0;
  for (ArrayIter it(stackTrace); it; ++it, ++i) {
    if (i > 0) {
      bytes += fprintf(f, "%s", escape ? "\\n" : "\n");
    }
    Array frame = it.second().toArray();
    bytes += fprintf(f, "    #%d ", i);
    if (frame.exists(s_function)) {
      if (frame.exists(s_class)) {
        bytes += fprintf(f, "%s%s%s(), called ",
                         frame[s_class].toString().c_str(),
                         frame[s_type].toString().c_str(),
                         frame[s_function].toString().c_str());
      } else {
        bytes += fprintf(f, "%s(), called ",
                         frame[s_function].toString().c_str());
      }
    }
   bytes +=  fprintf(f, "at [%s:%" PRId64 "]",
                     frame[s_file].toString().c_str(),
                     frame[s_line].toInt64());
  }
  bytes += fprintf(f, "\n");
  fflush(f);
  return bytes;
}

std::string ExtendedLogger::StringOfFrame(const Array& frame, int i, bool escape) {
  std::ostringstream ss;

  if (i > 0) {
    ss << (escape ? "\\n" : "\n");
  }
  ss << "    #" << i << " ";
  if (frame.exists(s_function)) {
    if (frame.exists(s_class)) {
      ss << frame[s_class].toString().c_str()
         << frame[s_type].toString().c_str()
         << frame[s_function].toString().c_str()
         << "(), called ";
    } else {
      ss << frame[s_function].toString().c_str()
         << "(), called ";
    }
  }
  ss << "at [" << frame[s_file].toString().c_str()
     << ":" << frame[s_line].toInt64() << "]";

  return ss.str();
}

std::string ExtendedLogger::StringOfStackTrace(const Array& stackTrace) {
  std::string buf;
  int i = 0;
  for (ArrayIter it(stackTrace); it; ++it, ++i) {
    buf += StringOfFrame(it.second().toArray(), i);
  }
  buf += "\n";
  return buf;
}
///////////////////////////////////////////////////////////////////////////////
}
