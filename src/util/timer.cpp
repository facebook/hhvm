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

#include "timer.h"
#include "logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Timer::Timer(Type type, const char *name /* = NULL */) : m_type(type) {
  if (name) {
    m_name = name;
    Logger::Info("%s...", name);
  }
  m_start = measure();
}

Timer::~Timer() {
  if (!m_name.empty()) {
    report();
  }
}

int64 Timer::getMicroSeconds() const {
  return measure() - m_start;
}

void Timer::report() const {
  int64 ms = getMicroSeconds();
  int seconds = ms / 1000000;
  Logger::Info("%s took %d'%02d\" (%d ms) %s", m_name.c_str(),
               seconds / 60, seconds % 60, ms/1000, getName());
}

static int64 to_usec(const timeval& tv) {
  return (int64(tv.tv_sec) * 1000000) + tv.tv_usec;
}

int64 Timer::measure() const {
  if (m_type == WallTime) {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return to_usec(tv);
  }

  struct rusage ru;
  memset(&ru, 0, sizeof(ru));
  getrusage(RUSAGE_SELF, &ru);

  switch (m_type) {
  case SystemCPU: return to_usec(ru.ru_stime);
  case UserCPU:   return to_usec(ru.ru_utime);
  case TotalCPU:  return to_usec(ru.ru_stime) + to_usec(ru.ru_utime);
  default: ASSERT(false);
  }
  return 0;
}

const char *Timer::getName() const {
  switch (m_type) {
  case WallTime:  return "wall time";
  case SystemCPU: return "system cpu";
  case UserCPU:   return "user cpu";
  case TotalCPU:  return "total cpu";
  default: ASSERT(false);
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////

SlowTimer::SlowTimer(int64 msThreshold, const char *location, const char *info)
  : m_timer(Timer::WallTime), m_msThreshold(msThreshold) {
  if (location) m_location = location;
  if (info) m_info = info;
}

SlowTimer::~SlowTimer() {
  int64 msec = getTime();
  if (msec >= m_msThreshold) {
    Logger::Error("SlowTimer [%dms] at %s: %s",
                  msec, m_location.c_str(), m_info.c_str());
  }
}

int64 SlowTimer::getTime() const {
  return m_timer.getMicroSeconds() / 1000;
}

///////////////////////////////////////////////////////////////////////////////
}
