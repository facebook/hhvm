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

#ifndef __TIMER_H__
#define __TIMER_H__

#include "base.h"

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

public:
  Timer(Type type, const char *name = NULL);
  ~Timer();

  const char *getName() const;
  int64 getMicroSeconds() const;
  void report() const;

private:
  Type m_type;
  std::string m_name;
  int64 m_start;

  int64 measure() const;
};

class SlowTimer {
public:
  SlowTimer(int64 msThreshold, const char *location, const char *info);
  ~SlowTimer();

  int64 getTime() const;

private:
  Timer m_timer;
  int64 m_msThreshold;
  std::string m_location;
  std::string m_info;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __TIMER_H__
