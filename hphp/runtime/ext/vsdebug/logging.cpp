/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <time.h>
#include <string>
#include "hphp/runtime/ext/vsdebug/logging.h"

namespace HPHP {
namespace VSDEBUG {

// Linkage for the log file pointer, this is a singleton for the extension.
FILE* VSDebugLogger::s_logFile {nullptr};

void VSDebugLogger::InitializeLogging(std::string& logFilePath) {
  if (logFilePath.empty()) {
    return;
  }

  // This is only expected to be invoked once, when the extension is loaded.
  assert(s_logFile == nullptr);

  // TODO: (Ericblue) Add logic for max file size, log file rotation, etc.
  const char* path = logFilePath.c_str();
  s_logFile = fopen(path, "a");
  if (s_logFile == nullptr) {
    return;
  }

  // Start with a visual delimiter so it's easy to see where
  // the session started.
  Log(VSDebugLogger::LogLevelInfo, "-------------------------------");
}

void VSDebugLogger::FinalizeLogging() {
  if (s_logFile == nullptr) {
    return;
  }

  Log(VSDebugLogger::LogLevelInfo, "VSDebugExtension shutting down.");
  Log(VSDebugLogger::LogLevelInfo, "-------------------------------");
  LogFlush();
  fclose(s_logFile);
  s_logFile = nullptr;
}

void VSDebugLogger::Log(const char* level, const char* fmt, ...) {
  if (s_logFile == nullptr) {
    return;
  }

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  fprintf(s_logFile,
    "[%d-%d-%d %d:%d:%d]",
    tm.tm_year + 1900,
    tm.tm_mon + 1,
    tm.tm_mday,
    tm.tm_hour,
    tm.tm_min,
    tm.tm_sec
  );
  fprintf(s_logFile, "[%s]\t", level);

  va_list args;
  va_start(args, fmt);
  vfprintf(s_logFile, fmt, args);
  va_end(args);

  fprintf(s_logFile, "\n");
}

void VSDebugLogger::LogFlush() {
  if (s_logFile == nullptr) {
    return;
  }

  fflush(s_logFile);
}

}
}
