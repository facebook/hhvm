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

#include <chrono>
#include <time.h>
#include <string>
#include <sys/stat.h>

#include "hphp/runtime/ext/vsdebug/logging.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"

namespace HPHP {
namespace VSDEBUG {

// Singleton logger.
static VSDebugLogger s_logger;
bool VSDebugLogger::s_loggerDestroyed {false};

int VSDebugLogger::InitializeLogging(const std::string& logFilePath) {
  std::unique_lock<std::recursive_mutex> lock(s_logger.m_lock);

  if (logFilePath.empty()) {
    return 0;
  }

  if (!s_logger.m_logFilePath.empty()) {
    LogFlush();
    if (s_logger.m_logFile != nullptr) {
      fclose(s_logger.m_logFile);
    }
    s_logger.m_logFile = nullptr;
  }

  s_logger.m_logFilePath = logFilePath;

  return OpenLogFile();
}

int VSDebugLogger::OpenLogFile() {
  const char* path = s_logger.m_logFilePath.c_str();
  s_logger.m_logFile = fopen(path, "a");
  if (s_logger.m_logFile == nullptr) {
    return errno;
  }

  // Start with a visual delimiter so it's easy to see where
  // the session started.
  Log(VSDebugLogger::LogLevelInfo, "-------------------------------");
  Log(VSDebugLogger::LogLevelInfo, "Created new log file.");
  Log(VSDebugLogger::LogLevelInfo,
      "Debugger version: %s",
      VSDEBUG_VERSION
  );

  return 0;
}

void VSDebugLogger::SetLogRotationEnabled(bool enabled) {
  std::unique_lock<std::recursive_mutex> lock(s_logger.m_lock);
  s_logger.m_logRotationEnabled = enabled;

  // Regardless of whether the client just connected or disconnected,
  // this is a good time to do a flush of the log.
  VSDebugLogger::LogFlush();

  if (enabled) {
    VSDebugLogger::TryRotateLogs();
  }

  if (!s_logger.m_taskStarted) {
    s_logger.m_loggerTaskThread.start();
    s_logger.m_taskStarted = true;
  }
}

bool VSDebugLogger::GetLogRotationEnabled() {
  std::unique_lock<std::recursive_mutex> lock(s_logger.m_lock);
  return s_logger.m_logRotationEnabled;
}

void VSDebugLogger::FinalizeLogging() {
  std::atomic_thread_fence(std::memory_order_acquire);
  if (s_loggerDestroyed) {
    return;
  }

  {
    std::unique_lock<std::mutex> condLock(s_logger.m_condLock);
    std::unique_lock<std::recursive_mutex> lock(s_logger.m_lock);
    s_loggerDestroyed = true;

    if (s_logger.m_logFile != nullptr) {
      Log(VSDebugLogger::LogLevelInfo, "VSDebugExtension shutting down.");
      Log(VSDebugLogger::LogLevelInfo, "-------------------------------");
      LogFlush();
      if (s_logger.m_logFile != nullptr) {
        fclose(s_logger.m_logFile);
      }
      s_logger.m_logFile = nullptr;
    }

    s_logger.m_terminate = true;
    s_logger.m_cond.notify_all();
    std::atomic_thread_fence(std::memory_order_release);
  }

  s_logger.m_loggerTaskThread.waitForEnd();
}

void VSDebugLogger::loggerMaintenanceTask() {
  // Periodically flush the log file stream, and rotate log files when needed.
  std::unique_lock<std::mutex> lock(s_logger.m_condLock);

  while (!m_terminate) {
    auto now = std::chrono::system_clock::now();
    auto dueTime = now + std::chrono::seconds(kLogFlushIntervalSec);
    if (m_cond.wait_until(lock, dueTime) == std::cv_status::timeout) {
      if (m_terminate) {
        break;
      }

      // Flush and rotate the logs if needed. We don't do any flushing or
      // log rotating if there's no debugger client connected.
      if (VSDebugLogger::GetLogRotationEnabled()) {
        VSDebugLogger::LogFlush();
        VSDebugLogger::TryRotateLogs();
      }
    }
  }
}

void VSDebugLogger::TryRotateLogs() {
  std::unique_lock<std::recursive_mutex> lock(s_logger.m_lock);

  if (s_logger.m_logFilePath.empty()) {
    return;
  }

  struct stat statBuffer;
  if (stat(s_logger.m_logFilePath.c_str(), &statBuffer) != 0) {
    return;
  }

  auto fileSizeBytes = statBuffer.st_size;
  if (fileSizeBytes < kLogFileMaxSizeBytes) {
    return;
  }

  Log(VSDebugLogger::LogLevelInfo, "Rotating log file.");
  LogFlush();
  fclose(s_logger.m_logFile);
  s_logger.m_logFile = nullptr;

  // Rotate the files.
  for (int i = kLogFilesToRetain - 1; i >= 0; i--) {
    // Try to rename the file if it exists, ignore return code.
    const std::string fromFile = i > 0
      ? s_logger.m_logFilePath + std::to_string(i)
      : s_logger.m_logFilePath;

    const std::string toFile = s_logger.m_logFilePath + std::to_string(i+1);
    std::remove(toFile.c_str());
    std::rename(fromFile.c_str(), toFile.c_str());
  }

  // Open a new log file.
  OpenLogFile();
}

void VSDebugLogger::Log(const char* level, const char* fmt, ...) {
  std::unique_lock<std::recursive_mutex> lock(s_logger.m_lock);

  if (s_logger.m_logFile == nullptr) {
    return;
  }

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  fprintf(s_logger.m_logFile,
    "[%" PRId64 "][%d-%d-%d %d:%d:%d]",
    (int64_t)getpid(),
    tm.tm_year + 1900,
    tm.tm_mon + 1,
    tm.tm_mday,
    tm.tm_hour,
    tm.tm_min,
    tm.tm_sec
  );
  fprintf(s_logger.m_logFile, "[%s]\t", level);

  va_list args;
  va_start(args, fmt);
  vfprintf(s_logger.m_logFile, fmt, args);
  va_end(args);

  fprintf(s_logger.m_logFile, "\n");
}

void VSDebugLogger::LogFlush() {
  std::unique_lock<std::recursive_mutex> lock(s_logger.m_lock);

  if (s_logger.m_logFile == nullptr) {
    return;
  }

  fflush(s_logger.m_logFile);
}

}
}
