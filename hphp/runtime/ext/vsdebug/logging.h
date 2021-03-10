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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/util/async-func.h"

#include <string>
#include <condition_variable>

namespace HPHP {
namespace VSDEBUG {

// Log will be flushed approximately this often.
static constexpr int kLogFlushIntervalSec = 10;

struct VSDebugLogger final {
  static void Log(const char* level, const char* fmt, ...);
  static void LogFlush();
  static int InitializeLogging(const std::string& logFilePath);
  static void FinalizeLogging();
  static void TryRotateLogs();

  static void SetLogRotationEnabled(bool enabled);
  static bool GetLogRotationEnabled();

  static constexpr const char* LogLevelError = "ERROR";
  static constexpr const char* LogLevelWarning = "WARNING";
  static constexpr const char* LogLevelInfo = "INFO";
  static constexpr const char* LogLevelVerbose = "VERBOSE";

  VSDebugLogger() :
    m_loggerTaskThread(this, &VSDebugLogger::loggerMaintenanceTask) {
  }

  ~VSDebugLogger() {
    FinalizeLogging();
  }

private:

  void loggerMaintenanceTask();

  static bool s_loggerDestroyed;
  static int OpenLogFile();

  std::string m_logFilePath {""};
  FILE* m_logFile {nullptr};
  std::recursive_mutex m_lock;
  bool m_logRotationEnabled {false};
  bool m_taskStarted {false};

  bool m_terminate;
  AsyncFunc<VSDebugLogger> m_loggerTaskThread;
  std::condition_variable m_cond;
  std::mutex m_condLock;

  // Log will be rotated approximately this often.
  static constexpr int kLogFileMaxSizeBytes = 512 * 1024; // 0.5MB
  static constexpr int kLogFilesToRetain = 5;
};

}
}

