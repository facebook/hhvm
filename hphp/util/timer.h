/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TIMER_H_
#define incl_HPHP_TIMER_H_

#include "hphp/util/base.h"
#include "hphp/util/compatibility.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Timing execution of block of codes.
 */
class Timer {
public:
  enum Type {
    WallTime,
    SystemCPU,
    UserCPU,
    TotalCPU,
  };
  enum ReportType {
    Log,
    Stderr,
    Trace,
  };

public:
  explicit Timer(Type type, const char *name = nullptr, ReportType r = Log);
  ~Timer();

  static void GetRealtimeTime(timespec &sp);
  static void GetMonotonicTime(timespec &sp);
  static int64_t GetCurrentTimeMicros();
  const char *getName() const;
  int64_t getMicroSeconds() const;
  void report() const;

private:
  Type m_type;
  ReportType m_report;
  std::string m_name;
  int64_t m_start;

  int64_t measure() const;
};

class SlowTimer {
public:
  SlowTimer(int64_t msThreshold, const char *location, const char *info);
  ~SlowTimer();

  int64_t getTime() const;

private:
  Timer m_timer;
  int64_t m_msThreshold;
  std::string m_location;
  std::string m_info;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TIMER_H_
